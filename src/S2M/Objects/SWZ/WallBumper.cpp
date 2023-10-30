// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: WallBumper Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "WallBumper.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(WallBumper);

void WallBumper::Update() { WallBumper::HandleInteractions(); }

void WallBumper::LateUpdate() {}

void WallBumper::StaticUpdate() {}

void WallBumper::Draw()
{
    Vector2 drawPos = this->position;
    if (this->type != WALLBUMPER_H) {
        this->direction ^= this->reverse;

        drawPos.x -= this->size << 20;
        for (int32 i = 0; i < this->size; ++i) {
            this->animator.DrawSprite(&drawPos, false);

            drawPos.x += 0x200000;
        }

        this->direction ^= this->reverse;
    }
    else {
        this->direction ^= FLIP_Y * this->reverse;

        drawPos.y -= this->size << 20;
        for (int32 i = 0; i < this->size; ++i) {
            this->animator.DrawSprite(&drawPos, false);

            drawPos.y += 0x200000;
        }

        this->direction ^= FLIP_Y * this->reverse;
    }
}

void WallBumper::Create(void *data)
{
    this->drawFX        = FX_FLIP;
    this->visible       = true;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x400000;
    this->updateRange.y = 0x400000;

    if (this->type == WALLBUMPER_H) {
        this->updateRange.y = (this->size + 4) << 20;

        this->hitbox.left   = 0;
        this->hitbox.top    = -16 - (16 * this->size);
        this->hitbox.right  = 8;
        this->hitbox.bottom = (16 * this->size) - 16;
    }
    else {
        this->direction *= FLIP_Y;
        this->updateRange.x = (this->size + 4) << 20;

        this->hitbox.left   = -16 - (16 * this->size);
        this->hitbox.top    = 0;
        this->hitbox.right  = (16 * this->size) - 16;
        this->hitbox.bottom = 8;
    }

    this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);
}

void WallBumper::StageLoad()
{
    sVars->aniFrames.Load("SWZ/WallBumper.bin", SCOPE_STAGE);

    sVars->sfxBouncer.Get("SWZ/Bouncer.wav");

    DebugMode::AddObject(sVars->classID, &WallBumper::DebugSpawn, &WallBumper::DebugDraw);
}

void WallBumper::DebugSpawn()
{
    GameObject::Create<WallBumper>(nullptr, this->position.x, this->position.y);
}

void WallBumper::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void WallBumper::HandleInteractions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->animator.animationID != Player::ANI_HURT && player->CheckBadnikTouch(this, &this->hitbox)) {
            if (this->type != WALLBUMPER_H) {
                if (this->direction) {
                    if (player->velocity.y < 0)
                        continue;

                    player->velocity.y = -0x80000;
                }
                else {
                    if (player->velocity.y > 0)
                        continue;

                    player->velocity.y = 0x80000;
                }

                if (this->reverse) {
                    player->velocity.x = -0x80000;
                    player->groundVel  = -0x80000;
                }
                else {
                    player->velocity.x = 0x80000;
                    player->groundVel  = 0x80000;
                }
            }
            else {
                if (this->direction) {
                    if (player->velocity.x < 0)
                        continue;

                    player->velocity.x = -0x80000;
                    player->groundVel  = -0x80000;
                }
                else {
                    if (player->velocity.x > 0)
                        continue;

                    player->velocity.x = 0x80000;
                    player->groundVel  = 0x80000;
                }

                player->velocity.y = this->reverse ? 0x80000 : -0x80000;
            }

            sVars->sfxBouncer.Play(false, 255);
            player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, false, 0);

            if (player->state.Matches(&Player::State_FlyCarried))
                GameObject::Get<Player>(SLOT_PLAYER2)->flyCarryTimer = 30;

            player->applyJumpCap   = false;
            player->onGround       = false;
            player->tileCollisions = TILECOLLISION_DOWN;
            player->onGround       = false;
            player->state.Set(&Player::State_Air);

            if (this->destructible) {
                this->Destroy();
                break;
            }
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void WallBumper::EditorDraw()
{
    if (!this->size)
        this->size = 1;

    int32 dir = this->direction;
    if (this->type == WALLBUMPER_H) {
        this->updateRange.y = (this->size + 4) << 20;
    }
    else {
        this->direction *= FLIP_Y;
        this->updateRange.x = (this->size + 4) << 20;
    }

    this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);

    this->inkEffect = this->destructible ? INK_BLEND : INK_NONE;

    WallBumper::Draw();

    this->direction = dir;
}

void WallBumper::EditorLoad()
{
    sVars->aniFrames.Load("SWZ/WallBumper.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Wall", WALLBUMPER_H);
    RSDK_ENUM_VAR("Ceiling/Floor", WALLBUMPER_V);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left/Top", FLIP_NONE);
    RSDK_ENUM_VAR("Right/Bottom", FLIP_X);
}
#endif

void WallBumper::Serialize()
{
    RSDK_EDITABLE_VAR(WallBumper, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(WallBumper, VAR_ENUM, size);
    RSDK_EDITABLE_VAR(WallBumper, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(WallBumper, VAR_BOOL, reverse);
    RSDK_EDITABLE_VAR(WallBumper, VAR_BOOL, destructible);
}


} // namespace GameLogic