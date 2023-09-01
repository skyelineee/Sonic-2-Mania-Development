// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Flipper Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Flipper.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Flipper);

void Flipper::Update()
{
    this->animator.Process();

    if (this->animator.animationID == 1 && this->animator.frameID == this->animator.frameCount - 1)
        this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);

    if (this->direction == FLIP_NONE) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            int32 playerID = RSDKTable->GetEntitySlot(player);

            if (player->position.x >= this->position.x + 0x30000)
                this->hitbox.top = ((player->position.x - this->position.x - 0x30000) >> 17) - 20;
            else
                this->hitbox.top = -20;

            this->hitbox.bottom = this->hitbox.top + 16;

            if (player->CheckCollisionPlatform(this, &this->hitbox)) {
                this->activePlayers |= 1 << playerID;
                player->position.y += 0x80000;

                player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP, false, 0);

                if (player->jumpPress) {
                    for (int32 i = SLOT_PLAYER1; i < Player::sVars->playerCount; ++i) { // gets whichever player jumped so the actions are only done to it
                        if ((1 << i) & this->activePlayers) {
                            Player *playerPtr           = GameObject::Get<Player>(i);
                            int32 vel                   = (playerPtr->position.x - this->position.x) - 0x30000;
                            playerPtr->animator.speed   = 120;
                            playerPtr->state.Set(&Player::State_Air);
                            playerPtr->jumpAbilityState = 1;
                            playerPtr->onGround         = false;
                            playerPtr->velocity.x       = vel / 7;
                            playerPtr->velocity.y       = -0x90000 - (vel >> 3);
                        }
                    }

                    this->animator.SetAnimation(sVars->aniFrames, 1, false, 0);
                    sVars->sfxFlipper.Play(false, 255);
                }
                else {
                    player->tileCollisions = TILECOLLISION_DOWN;
                    player->state.Set(&Player::State_Air);

                    if (player->groundVel >= 0x10000) {
                        if (player->groundVel > 0x20000)
                            player->groundVel = 0x20000;

                        player->groundVel += 0x1000;
                    }
                    else {
                        player->groundVel = 0x10000;
                        player->groundVel += 0x1000;
                    }
                }
            }
            else {
                this->activePlayers &= ~(1 << playerID);
            }
        }
    }
    else {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            int32 playerID = RSDKTable->GetEntitySlot(player);

            if (player->position.x <= this->position.x - 0x30000)
                this->hitbox.top = ((this->position.x - player->position.x - 0x30000) >> 17) - 20;
            else
                this->hitbox.top = -20;

            this->hitbox.bottom = this->hitbox.top + 16;

            if (player->CheckCollisionPlatform(this, &this->hitbox)) {
                this->activePlayers |= 1 << playerID;
                player->position.y += 0x80000;
                player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP, false, 0);

                if (player->jumpPress) {
                    for (int32 i = SLOT_PLAYER1; i < Player::sVars->playerCount; ++i) {
                        if ((1 << i) & this->activePlayers) {
                            Player *playerPtr           = GameObject::Get<Player>(i);
                            int32 vel                   = (this->position.x - player->position.x) - 0x30000;
                            playerPtr->animator.speed   = 120;
                            playerPtr->state.Set(&Player::State_Air);
                            playerPtr->jumpAbilityState = 1;
                            playerPtr->onGround         = false;
                            playerPtr->velocity.x       = -(vel / 7);
                            playerPtr->velocity.y       = -0x90000 - (vel >> 3);
                        }
                    }

                    this->animator.SetAnimation(sVars->aniFrames, 1, false, 0);
                    sVars->sfxFlipper.Play(false, 255);
                }
                else {
                    player->tileCollisions = TILECOLLISION_DOWN;
                    player->state.Set(&Player::State_Air);
                    if (player->groundVel <= -0x10000) {
                        if (player->groundVel < -0x20000)
                            player->groundVel = -0x20000;

                        player->groundVel -= 0x1000;
                    }
                    else {
                        player->groundVel = -0x10000;
                        player->groundVel -= 0x1000;
                    }
                }
            }
            else {
                this->activePlayers &= ~(1 << playerID);
            }
        }
    }
}
void Flipper::LateUpdate() {}
void Flipper::StaticUpdate() {}
void Flipper::Draw() { this->animator.DrawSprite(nullptr, false); }

void Flipper::Create(void *data)
{
    this->drawFX = FX_FLIP;
    if (!sceneInfo->inEditor) {

        this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->hitbox.left   = sVars->hitbox.left;
        this->hitbox.right  = sVars->hitbox.right;
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    }
}

void Flipper::StageLoad()
{
	sVars->aniFrames.Load("CNZ/Flipper.bin", SCOPE_STAGE);

    sVars->hitbox.left  = -25;
    sVars->hitbox.right = 26;

    sVars->sfxFlipper.Get("Stage/Flipper.wav");

    DebugMode::AddObject(sVars->classID, &Flipper::DebugSpawn, &Flipper::DebugDraw);
}

void Flipper::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Flipper::DebugSpawn()
{
    GameObject::Create<Flipper>(nullptr, this->position.x, this->position.y);
}

#if RETRO_INCLUDE_EDITOR
void Flipper::EditorDraw() {}

void Flipper::EditorLoad()
{
	sVars->aniFrames.Load("CNZ/Flipper.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Right");
}
#endif

void Flipper::Serialize() { RSDK_EDITABLE_VAR(Flipper, VAR_UINT8, direction); }

} // namespace GameLogic