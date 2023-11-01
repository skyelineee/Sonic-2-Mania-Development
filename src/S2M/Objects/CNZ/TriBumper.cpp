// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TriBumper Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "TriBumper.hpp"
#include "Global/Player.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TriBumper);

// this object also needs forceunsticks on the walls where its used, as if its not the player will stick to the wall tiles and it can get trapped
// inside the bumper, dont know what v4 did exactly to avoid this also i probably messed something up porting this and i dont really wanna redo it
// either so ill just work with it lol

void TriBumper::Update()
{
    // these are guesses on what the original "temp" variables are supposed to be, not 100% sure tho
    int32 playerVel       = 0;
    int32 playerPos       = 0;
    int32 playerHeightVel = 0;
    switch (this->type) {
        case SMALL_L:
            for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                this->player = currentPlayer;
                // BoxCollisionTest(C_TOUCH, this->entityPos, -32, -32, 32, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
                if (currentPlayer->CheckCollisionTouch(this, &sVars->smallHitbox)) {
                    playerVel = this->position.y;
                    playerVel -= currentPlayer->position.y;
                    playerVel = -playerVel;
                    if (playerVel >= 0x200000) {
                        currentPlayer->velocity.y = 0xA0000;
                        TriBumper::HandleBump();
                    }
                    else {
                        playerVel = this->position.x;
                        playerVel -= currentPlayer->position.x;
                        playerVel = -playerVel;
                        if (playerVel >= 0x200000) {
                            currentPlayer->velocity.x = 0xA0000;
                            TriBumper::HandleBump();
                        }
                        else {
                            playerVel = this->position.x;
                            playerVel -= currentPlayer->position.x;
                            if (playerVel >= 0x200000) {
                                playerVel = 0x200000;
                            }
                            playerVel += this->position.y;
                            playerVel -= 0x80000;
                            playerPos = currentPlayer->position.y;
                            playerPos += 0xE0000;
                            playerVel -= playerPos;
                            if (playerVel < 0) {
                                playerHeightVel = 32;
                                TriBumper::SetPlayerVelocity(playerPos, playerHeightVel);
                                TriBumper::HandleBump();
                            }
                        }
                    }
                }
            }
            break;

        case SMALL_R:
            for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                this->player = currentPlayer;
                // BoxCollisionTest(C_TOUCH, this->entityPos, -32, -32, 32, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
                if (currentPlayer->CheckCollisionTouch(this, &sVars->smallHitbox)) {
                    playerVel = this->position.y;
                    playerVel -= currentPlayer->position.y;
                    playerVel = -playerVel;
                    if (playerVel >= 0x200000) {
                        currentPlayer->velocity.y = 0xA0000;
                        TriBumper::HandleBump();
                    }
                    else {
                        playerVel = this->position.x;
                        playerVel -= currentPlayer->position.x;
                        if (playerVel >= 0x200000) {
                            currentPlayer->velocity.x = -0xA0000;
                        TriBumper:
                            HandleBump();
                        }
                        else {
                            playerVel = this->position.x;
                            playerVel -= currentPlayer->position.x;
                            playerVel = -playerVel;
                            if (playerVel >= 0x200000) {
                                playerVel = 0x200000;
                            }
                            playerVel += this->position.y;
                            playerVel -= 0x80000;
                            playerPos = currentPlayer->position.y;
                            playerPos += 0xE0000;
                            playerVel -= playerPos;
                            if (playerVel < 0) {
                                playerHeightVel = 96;
                                TriBumper::SetPlayerVelocity(playerPos, playerHeightVel);
                                TriBumper::HandleBump();
                            }
                        }
                    }
                }
            }
            break;

        case LARGE_U:
            for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                this->player = currentPlayer;
                // BoxCollisionTest(C_TOUCH, this->entityPos, -64, -8, 64, 8, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
                if (currentPlayer->CheckCollisionTouch(this, &sVars->largeUDHitbox)) {
                    playerVel = this->position.y;
                    playerVel -= currentPlayer->position.y;
                    playerVel = -playerVel;
                    if (playerVel >= 0x80000) {
                        currentPlayer->velocity.y = 0xA00;
                        TriBumper::HandleBump();
                    }
                    else {
                        playerVel = this->position.x;
                        playerVel -= currentPlayer->position.x;
                        if (playerVel >= 0x400000) {
                            currentPlayer->velocity.x = -0xA0000;
                            TriBumper::HandleBump();
                        }
                        else {
                            playerVel = -playerVel;
                            if (playerVel >= 0x400000) {
                                currentPlayer->velocity.x = 0xA0000;
                                TriBumper::HandleBump();
                            }
                            else {
                                if (playerVel < 0) {
                                    playerHeightVel = 56;
                                }
                                else {
                                    playerHeightVel = 72;
                                }
                                TriBumper::SetPlayerVelocity(playerPos, playerHeightVel);
                                TriBumper::HandleBump();
                            }
                        }
                    }
                }
            }
            break;

        case LARGE_D:
            for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                this->player = currentPlayer;
                // BoxCollisionTest(C_TOUCH, this->entityPos, -64, -8, 64, 8, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
                if (currentPlayer->CheckCollisionTouch(this, &sVars->largeUDHitbox)) {
                    playerVel = this->position.y;
                    playerVel -= currentPlayer->position.y;
                    if (playerVel >= 0x80000) {
                        currentPlayer->velocity.y = -0xA0000;
                        TriBumper::HandleBump();
                    }
                    else {
                        playerVel = this->position.x;
                        playerVel -= currentPlayer->position.x;
                        if (playerVel >= 0x400000) {
                            currentPlayer->velocity.x = -0xA0000;
                            TriBumper::HandleBump();
                        }
                        else {
                            playerVel = -playerVel;
                            if (playerVel >= 0x400000) {
                                currentPlayer->velocity.x = 0xA0000;
                                TriBumper::HandleBump();
                            }
                            else {
                                if (playerVel < 0) {
                                    playerHeightVel = 200;
                                }
                                else {
                                    playerHeightVel = 184;
                                }
                                TriBumper::SetPlayerVelocity(playerPos, playerHeightVel);
                                TriBumper::HandleBump();
                            }
                        }
                    }
                }
            }
            break;

        case LARGE_L:
            for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                this->player = currentPlayer;
                // BoxCollisionTest(C_TOUCH, this->entityPos, -8, -64, 8, 64, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
                if (currentPlayer->CheckCollisionTouch(this, &sVars->largeLRHitbox)) {
                    playerVel = this->position.x;
                    playerVel -= currentPlayer->position.x;
                    playerVel = -playerVel;
                    if (playerVel >= 0x80000) {
                        currentPlayer->velocity.x = 0xA0000;
                        TriBumper::HandleBump();
                    }
                    else {
                        playerVel = this->position.y;
                        playerVel -= currentPlayer->position.y;
                        if (playerVel >= 0x400000) {
                            currentPlayer->velocity.y = -0xA0000;
                            TriBumper::HandleBump();
                        }
                        else {
                            playerVel = -playerVel;
                            if (playerVel >= 0x400000) {
                                currentPlayer->velocity.y = 0xA0000;
                                TriBumper::HandleBump();
                            }
                            else {
                                if (playerVel < 0) {
                                    playerHeightVel = 8;
                                }
                                else {
                                    playerHeightVel = 248;
                                }
                                TriBumper::SetPlayerVelocity(playerPos, playerHeightVel);
                                TriBumper::HandleBump();
                            }
                        }
                    }
                }
            }
            break;

        case LARGE_R:
            for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                this->player = currentPlayer;
                // BoxCollisionTest(C_TOUCH, this->entityPos, -8, -64, 8, 64, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
                if (currentPlayer->CheckCollisionTouch(this, &sVars->largeLRHitbox)) {
                    playerVel = this->position.x;
                    playerVel -= currentPlayer->position.x;
                    if (playerVel >= 0x80000) {
                        currentPlayer->velocity.x = 0xA0000;
                        TriBumper::HandleBump();
                    }
                    else {
                        playerVel = this->position.y;
                        playerVel -= currentPlayer->position.y;
                        if (playerVel >= 0x400000) {
                            currentPlayer->velocity.y = -0xA0000;
                            TriBumper::HandleBump();
                        }
                        else {
                            playerVel = -playerVel;
                            if (playerVel >= 0x400000) {
                                currentPlayer->velocity.y = 0xA0000;
                                TriBumper::HandleBump();
                            }
                            else {
                                if (playerVel < 0) {
                                    playerHeightVel = 120;
                                }
                                else {
                                    playerHeightVel = 136;
                                }
                                TriBumper::SetPlayerVelocity(playerPos, playerHeightVel);
                                TriBumper::HandleBump();
                            }
                        }
                    }
                }
            }
            break;
    }
}
void TriBumper::LateUpdate() {}
void TriBumper::StaticUpdate() {}
void TriBumper::Draw() {}

void TriBumper::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->visible       = true;
        this->drawGroup     = 2;
    }
}

void TriBumper::StageLoad()
{
    sVars->sfxBumper.Get("Stage/CNZBumper.wav");

    sVars->smallHitbox.left   = -32;
    sVars->smallHitbox.top    = -32;
    sVars->smallHitbox.right  = 32;
    sVars->smallHitbox.bottom = 32;

    sVars->largeUDHitbox.left   = -64;
    sVars->largeUDHitbox.top    = -8;
    sVars->largeUDHitbox.right  = 64;
    sVars->largeUDHitbox.bottom = 8;

    sVars->largeLRHitbox.left   = -8;
    sVars->largeLRHitbox.top    = -64;
    sVars->largeLRHitbox.right  = 8;
    sVars->largeLRHitbox.bottom = 64;
}

void TriBumper::SetPlayerVelocity(int32 playerPos, int32 playerHeightVel)
{
    int32 playerVel = Math::ATan2(player->velocity.x, player->velocity.y);
    playerVel -= playerHeightVel;
    playerPos = abs(playerVel);
    playerVel = -playerVel;
    playerVel += 256;
    playerVel += playerHeightVel;
    playerVel &= 255;
    if (playerPos >= 56) {
        playerVel = playerHeightVel;
    }
    player->velocity.x = Math::Cos256(playerVel);
    player->velocity.x *= -0xA00;
    player->velocity.y = Math::Sin256(playerVel);
    player->velocity.y *= -0xA00;
}

void TriBumper::HandleBump()
{
    if (player->state.Matches(&Player::State_FlyCarried)) {
        if (player->sidekick) {
            player->flyCarryTimer = 30;
        }
    }

    if (player->animator.animationID != Player::ANI_GLIDE) {
        player->groundVel = player->velocity.x;
        player->timer     = 0;
    }

    player->tileCollisions = true;
    sVars->sfxBumper.Play(false, 255);
}

#if RETRO_INCLUDE_EDITOR
void TriBumper::EditorDraw() {}

void TriBumper::EditorLoad()
{
    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Facing Left");
    RSDK_ENUM_VAR("Facing Right");
    RSDK_ENUM_VAR("Facing Up");
    RSDK_ENUM_VAR("Facing Down");
    RSDK_ENUM_VAR("Facing Left");
    RSDK_ENUM_VAR("Facing Right");
}
#endif

void TriBumper::Serialize() { RSDK_EDITABLE_VAR(TriBumper, VAR_INT32, type); }

} // namespace GameLogic