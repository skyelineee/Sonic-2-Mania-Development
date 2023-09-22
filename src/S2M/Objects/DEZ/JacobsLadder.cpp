// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: JacobsLadder Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "JacobsLadder.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(JacobsLadder);

void JacobsLadder::Update()
{
    bool32 showElectricity = this->showElectricity;

    this->electricFadeOut = false;
    this->showElectricity = false;
    this->direction       = this->flip ? FLIP_Y : FLIP_NONE;

    int32 timer = (this->intervalOffset + Zone::sVars->timer) % this->interval;

    if (timer < this->duration) {
        if (!showElectricity && this->onScreen == 1)
            sVars->sfxLadder.Play(false, 255);

        this->showElectricity = true;
    }
    else if (timer < this->duration + 12)
        this->electricFadeOut = true;

    if (this->electricFadeOut) {
        this->electricPos = this->height << 20;
        this->electricAnimator.SetAnimation(sVars->aniFrames, 3, true, 10);
    }

    if (this->showElectricity) {
        int32 storeX      = this->position.x;
        int32 storeY      = this->position.y;
        this->electricPos = timer * ((this->height << 20) / this->duration);
        this->position.y += (2 * this->flip - 1) * (this->electricPos + 0x300000);

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            int32 playerID = RSDKTable->GetEntitySlot(player);

            if (((1 << playerID) & this->activePlayers)) {
                if (player->state.Matches(&Player::State_Static) && player->shield == Player::Shield_Lightning) {
                    if (player->jumpPress) {
                        this->playerTimers[playerID] = 15;
                        this->activePlayers &= ~(1 << playerID);
                        player->jumpAbilityState = 1;
                        player->state.Set(&Player::State_Air);

                        sceneInfo->entity = (Entity *)player;
                        player->stateAbility.Run(player);
                        sceneInfo->entity = (Entity *)this;
                    }
                    else {
                        player->position.x = this->position.x + this->playerPositions[playerID].x;
                        player->position.y = this->position.y + this->playerPositions[playerID].y;
                    }
                }
                else {
                    this->activePlayers &= ~(1 << playerID);
                }
            }
            else {
                if (this->playerTimers[playerID]) {
                    this->playerTimers[playerID]--;
                }
                else {
                    if (player->CheckCollisionTouch(this, &this->hitbox)) {
                        if (player->shield == Player::Shield_Lightning) {
                            if (!player->state.Matches(& Player::State_Static)) {
                                player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP, false, 0);
                                player->state.Set(&Player::State_Static);
                                player->nextAirState.Set(nullptr);
                                player->nextGroundState.Set(nullptr);

                                this->playerPositions[playerID].x = player->position.x - this->position.x;
                                this->playerPositions[playerID].y = player->position.y - this->position.y;

                                player->velocity.x = 0;
                                player->velocity.y = 0;
                                player->groundVel  = 0;

                                this->activePlayers |= 1 << playerID;
                            }
                        }
                        else {
                            player->Hurt(this);
                        }
                    }
                }
            }
        }

        this->position.x = storeX;
        this->position.y = storeY;

        this->electricAnimator.Process();
    }
    else {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            int32 playerID = RSDKTable->GetEntitySlot(player);

            if ((1 << playerID) & this->activePlayers) {
                this->activePlayers &= ~(1 << playerID);

                if (player->state.Matches(&Player::State_Static)) {
                    player->velocity.x       = 0;
                    player->velocity.y       = 0;
                    player->groundVel        = 0;
                    player->jumpAbilityState = 1;
                    player->state.Set(&Player::State_Air);
                }
            }
        }
    }
}

void JacobsLadder::LateUpdate() {}

void JacobsLadder::StaticUpdate() {}

void JacobsLadder::Draw()
{
    this->drawFX    = FX_FLIP;
    this->inkEffect = INK_NONE;
    this->alpha     = 0xFF;

    // what a line of code tbh
    int32 direction = -(-(int32)this->flip != false) & 0xFFFFFF02;

    int32 yOff = 0x300000 * (2 * (this->flip != false) - 1);
    int32 side = 0;
    for (int32 i = 0; i < 2; ++i) {
        this->direction = direction ^ side;

        Vector2 drawPos;
        drawPos.x = this->position.x + ((2 * (side != false) - 1) << 21);
        drawPos.y = this->position.y;
        this->sideAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->sideAnimator.DrawSprite(&drawPos, false);

        drawPos.y += yOff;
        for (int32 y = 0; y < this->height; ++y) {
            this->sideAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
            this->sideAnimator.DrawSprite(&drawPos, false);
            drawPos.y += (2 * (this->flip != false) - 1) << 20;
        }

        this->sideAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
        this->sideAnimator.DrawSprite(&drawPos, false);
        side++;
    }

    this->direction = direction;
    if (this->showElectricity || this->electricFadeOut) {
        this->inkEffect = INK_ADD;
        if (this->electricFadeOut)
            this->alpha = 21 * (this->duration - (this->intervalOffset + Zone::sVars->timer) % this->interval) + 255;

        Vector2 drawPos;
        drawPos.x = this->position.x;
        drawPos.y = this->position.y + (2 * (this->flip != false) - 1) * (this->electricPos + 0x300000);
        this->electricAnimator.DrawSprite(&drawPos, false);
        this->electricAnimator.DrawSprite(&drawPos, false);
    }
}

void JacobsLadder::Create(void *data)
{
    this->active        = ACTIVE_BOUNDS;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->startPos      = this->position;
    this->visible       = true;
    this->updateRange.x = 0x800000;
    this->updateRange.y = (this->height + 8) << 20;
    this->drawFX        = FX_FLIP;
    this->direction     = this->flip ? FLIP_Y : FLIP_NONE;

    if (!this->interval)
        this->interval = 120;

    if (!this->duration)
        this->duration = 60;

    this->electricAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);

    this->hitbox.left   = -35;
    this->hitbox.top    = -4;
    this->hitbox.right  = 35;
    this->hitbox.bottom = 4;
}

void JacobsLadder::StageLoad()
{
    sVars->aniFrames.Load("DEZ/JacobsLadder.bin", SCOPE_STAGE);

    sVars->sfxLadder.Get("DEZ/JacobsLadder.wav");
}

#if RETRO_INCLUDE_EDITOR
void JacobsLadder::EditorDraw()
{
    JacobsLadder::Create(nullptr);
    JacobsLadder::Draw();
}

void JacobsLadder::EditorLoad() { sVars->aniFrames.Load("DEZ/JacobsLadder.bin", SCOPE_STAGE); }
#endif

void JacobsLadder::Serialize()
{
    RSDK_EDITABLE_VAR(JacobsLadder, VAR_UINT32, height);
    RSDK_EDITABLE_VAR(JacobsLadder, VAR_ENUM, interval);
    RSDK_EDITABLE_VAR(JacobsLadder, VAR_ENUM, intervalOffset);
    RSDK_EDITABLE_VAR(JacobsLadder, VAR_ENUM, duration);
    RSDK_EDITABLE_VAR(JacobsLadder, VAR_BOOL, flip);
}

} // namespace GameLogic