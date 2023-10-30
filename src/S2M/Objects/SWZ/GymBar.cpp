// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: GymBar Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "GymBar.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(GymBar);

void GymBar::Update() { GymBar::HandlePlayerInteractions(); }

void GymBar::LateUpdate() {}

void GymBar::StaticUpdate() {}

void GymBar::Draw()
{
    Vector2 drawPos = this->position;
    if (this->type != GYMBAR_HORIZONTAL) {
        drawPos.y              = this->position.y - (this->size << 18);
        this->animator.frameID = 0;
        this->animator.DrawSprite(&drawPos, false);

        this->animator.frameID = 1;
        for (int32 i = 0; i < this->size; ++i) {
            this->animator.DrawSprite(&drawPos, false);
            drawPos.y += 0x80000;
        }
    }
    else {
        drawPos.x              = this->position.x - (this->size << 18);
        this->animator.frameID = 0;
        this->animator.DrawSprite(&drawPos, false);

        this->animator.frameID = 1;
        for (int32 i = 0; i < this->size; ++i) {
            this->animator.DrawSprite(&drawPos, false);
            drawPos.x += 0x80000;
        }
    }

    this->animator.frameID = 2;
    this->animator.DrawSprite(&drawPos, false);
}

void GymBar::Create(void *data)
{
    this->drawFX        = FX_FLIP;
    this->visible       = true;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x400000;
    this->updateRange.y = 0x400000;

    if (this->type == GYMBAR_HORIZONTAL) {
        this->updateRange.x = (this->size << 18) + 0x400000;

        this->hitbox.left   = -4 * this->size;
        this->hitbox.top    = 0;
        this->hitbox.right  = 4 * this->size - 8;
        this->hitbox.bottom = 8;

        this->maxX = this->position.x - (this->size << 18) + 0x50000;
        this->minX = (this->size << 18) + this->position.x - 0xC0000;
    }
    else {
        this->updateRange.y = (this->size + 16) << 18;

        this->hitbox.left   = 0;
        this->hitbox.top    = (8 - (this->size << 2));
        this->hitbox.right  = 0;
        this->hitbox.bottom = (this->size << 2) - 16;
    }

    this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);
}

void GymBar::StageLoad()
{
    sVars->aniFrames.Load("SWZ/GymBar.bin", SCOPE_STAGE);

    sVars->sfxBumper.Get("Stage/Bumper3.wav");

    DebugMode::AddObject(sVars->classID, &GymBar::DebugSpawn, &GymBar::DebugDraw);
}

void GymBar::DebugSpawn() { GameObject::Create<GymBar>(nullptr, this->position.x, this->position.y); }

void GymBar::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void GymBar::HandlePlayerInteractions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        int32 playerID = RSDKTable->GetEntitySlot(player);
        if (player->animator.animationID != Player::ANI_HURT && !player->state.Matches(&Player::State_FlyCarried)) {
            if (player->CheckCollisionTouch(this, &this->hitbox)) {
                if (!this->playerTimers[playerID]) {
                    player->onGround = false;
                    if (this->type != GYMBAR_HORIZONTAL) {
                        if (abs(player->velocity.x) >= 0x40000) {
                            player->position.x = this->position.x;
                            player->direction  = FLIP_NONE;
                            if (player->velocity.x <= 0)
                                player->animator.SetAnimation(player->aniFrames, Player::ANI_POLE_SWING_V, false, 2);
                            else
                                player->animator.SetAnimation(player->aniFrames, Player::ANI_POLE_SWING_V, false, 9);
                            player->animator.speed   = 0xE0 + ((abs(player->velocity.x) - 0x40000) >> 12);
                            player->abilityValues[0] = 0;
                            player->abilityValues[1] = player->animator.frameID;
                            player->abilityValues[2] = player->velocity.x;
                            player->state.Set(&GymBar::PlayerState_SwingH);
                        }
                    }
                    else {
                        player->rotation   = 0;
                        player->position.x = CLAMP(player->position.x, this->maxX, this->minX);
                        player->position.y = this->position.y;

                        if (abs(player->velocity.y) < 0x40000 || this->noSwing) {
                            player->animator.SetAnimation(player->aniFrames, Player::ANI_POLE_SWING_H, false, 0);
                            player->animator.speed = 0;
                            player->abilityPtrs[0] = this;
                            player->state.Set(&GymBar::PlayerState_Hang);
                        }
                        else {
                            if (player->velocity.y <= 0)
                                player->animator.SetAnimation(player->aniFrames, Player::ANI_POLE_SWING_H, false, 2);
                            else
                                player->animator.SetAnimation(player->aniFrames, Player::ANI_POLE_SWING_H, false, 9);
                            player->animator.speed   = 0x100 + ((abs(player->velocity.y) - 0x40000) >> 12);
                            player->abilityValues[0] = 0;
                            player->abilityValues[1] = player->animator.frameID;
                            player->abilityValues[2] = player->velocity.y;
                            player->state.Set(&GymBar::PlayerState_SwingV);
                        }
                    }

                    if (player->state.Matches(&GymBar::PlayerState_Hang) || player->state.Matches(&GymBar::PlayerState_SwingH)
                        || player->state.Matches(&GymBar::PlayerState_SwingV)) {
                        this->playerTimers[playerID] = 16;
                        Player::sVars->sfxGrab.Play(false, 0xFF);
                        player->nextAirState.Set(nullptr);
                        player->nextGroundState.Set(nullptr);
                        player->stateGravity.Set(&Player::Gravity_False);
                        player->velocity.x      = 0;
                        player->velocity.y      = 0;
                        player->groundVel       = 0;
                        player->applyJumpCap    = false;
                    }
                }
            }
            else {
                if (this->playerTimers[playerID])
                    this->playerTimers[playerID]--;
            }
        }
    }
}

void GymBar::HandleSwingJump()
{
    Player *player = (Player *)this;

    if (!player->down)
        player->velocity.y = -0x50000;

    player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP, false, 0);

    if (player->characterID == ID_TAILS)
        player->animator.speed = 120;
    else
        player->animator.speed = ((abs(player->groundVel) * 0xF0) / 0x60000) + 0x30;

    if (player->animator.speed > 0xF0)
        player->animator.speed = 0xF0;

    player->applyJumpCap     = true;
    player->jumpAbilityState = 1;
    player->abilityPtrs[0]   = nullptr;
    player->abilityValues[0] = 0;
    player->abilityValues[1] = 0;
    player->abilityValues[3] = 0;

    player->state.Set(&Player::State_Air);
}

void GymBar::PlayerState_SwingV()
{
    Player *player = (Player *)this;

    if (player->jumpPress)
        GymBar::HandleSwingJump();

    int32 frame = player->abilityValues[2] <= 0 ? 4 : 11;

    if (player->animator.frameID == frame && player->abilityValues[1] != frame)
        ++player->abilityValues[0];

    player->abilityValues[1] = player->animator.frameID;
    if (player->abilityValues[0] >= 2) {
        if (player->abilityValues[2] <= 0)
            player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, false, 0);
        else
            player->animator.SetAnimation(player->aniFrames, Player::ANI_WALK, false, 0);

        player->velocity.y =
            player->abilityValues[2] + (player->abilityValues[2] >> ((abs(player->abilityValues[2]) >> 18) + (abs(player->abilityValues[2]) >> 20)));
        player->abilityPtrs[0]   = nullptr;
        player->abilityValues[0] = 0;
        player->abilityValues[1] = 0;
        player->abilityValues[2] = 0;

        player->state.Set(&Player::State_Air);
    }
}

void GymBar::PlayerState_Hang()
{
    Player *player = (Player *)this;

    player->animator.SetAnimation(player->aniFrames, Player::ANI_SHIMMY_IDLE, false, 0);

    if (player->jumpPress)
        GymBar::HandleSwingJump();
}

void GymBar::PlayerState_SwingH()
{
    Player *player = (Player *)this;

    if (player->jumpPress)
        GymBar::HandleSwingJump();

    int32 frame = player->abilityValues[2] <= 0 ? 4 : 11;

    if (player->animator.frameID == frame && player->abilityValues[1] != frame)
        ++player->abilityValues[0];

    player->abilityValues[1] = player->animator.frameID;
    if (player->abilityValues[0] >= 2) {
        player->animator.SetAnimation(player->aniFrames, Player::ANI_WALK, false, 0);

        player->velocity.x = player->abilityValues[2] + (player->abilityValues[2] >> (abs(player->abilityValues[2]) >> 18));
        player->groundVel  = player->velocity.x;
        player->direction  = abs(player->velocity.x >> 31);

        player->abilityPtrs[0]   = nullptr;
        player->abilityValues[0] = 0;
        player->abilityValues[1] = 0;
        player->abilityValues[2] = 0;

        player->state.Set(&Player::State_Air);
    }
}

#if RETRO_INCLUDE_EDITOR
void GymBar::EditorDraw()
{
    if (this->type == GYMBAR_HORIZONTAL) {
        this->updateRange.x = (this->size << 18) + 0x400000;
        this->maxX          = this->position.x - (this->size << 18) + 0x50000;
        this->minX          = (this->size << 18) + this->position.x - 0xC0000;
    }
    else {
        this->updateRange.y = (this->size + 16) << 18;
    }

    this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);

    GymBar::Draw();
}

void GymBar::EditorLoad()
{
    sVars->aniFrames.Load("SWZ/GymBar.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Horizontal", GYMBAR_HORIZONTAL);
    RSDK_ENUM_VAR("Vertical", GYMBAR_VERTICAL);
}
#endif

void GymBar::Serialize()
{
    RSDK_EDITABLE_VAR(GymBar, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(GymBar, VAR_ENUM, size);
    RSDK_EDITABLE_VAR(GymBar, VAR_BOOL, noSwing);
}
} // namespace GameLogic