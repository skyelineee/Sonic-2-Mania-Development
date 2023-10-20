// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: BallCannon Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "BallCannon.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(BallCannon);

void BallCannon::Update() { this->state.Run(this); }

void BallCannon::LateUpdate() {}

void BallCannon::StaticUpdate() {}

void BallCannon::Draw() { this->animator.DrawSprite(nullptr, false); }

void BallCannon::Create(void *data)
{
    this->drawGroup = Zone::sVars->playerDrawGroup[0];
    this->visible   = true;

    if (!sceneInfo->inEditor) {
        if (data) {
            this->visible       = true;
            this->drawFX        = FX_ROTATE;
            this->active        = ACTIVE_NORMAL;
            this->rotationSpeed = Math::Rand(-8, 8);

            this->animator.SetAnimation(sVars->aniFrames, 3, true, VOID_TO_INT(data));
            this->state.Set(&BallCannon::State_CorkDebris);
        }
        else {
            this->drawFX        = FX_ROTATE | FX_FLIP;
            this->active        = ACTIVE_BOUNDS;
            this->updateRange.x = 0x400000;
            this->updateRange.y = 0x400000;

            switch (this->type) {
                case BALLCANNON_CANNON:
                    if (this->angle >= 4)
                        this->direction = FLIP_X;

                    this->rotation = (this->angle + this->direction + 1) << 7;

                    switch (this->angle) {
                        case BALLCANNON_DIR_RIGHT_CW: // Right -> Down
                        case BALLCANNON_DIR_LEFT_CCW: // Left -> Down
                            this->velocity.y = 0x100000;
                            break;

                        case BALLCANNON_DIR_DOWN_CW: // Down -> Left
                        case BALLCANNON_DIR_UP_CCW:  // Up -> Left
                            this->velocity.x = -0x100000;
                            break;

                        case BALLCANNON_DIR_LEFT_CW:   // Left -> Up
                        case BALLCANNON_DIR_RIGHT_CCW: // Right -> Up
                            this->velocity.y = -0x100000;
                            break;

                        case BALLCANNON_DIR_UP_CW:    // Up -> Right
                        case BALLCANNON_DIR_DOWN_CCW: // Down -> Right
                            this->velocity.x = 0x100000;
                            break;

                        default: break;
                    }
                    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
                    this->state.Set(&BallCannon::State_Idle);
                    break;

                case BALLCANNON_CORKV:
                    this->animator.SetAnimation(sVars->aniFrames, 3, true, 0);
                    this->velocity.y = -0x80000;
                    this->state.Set(&BallCannon::State_CorkBlocked);
                    break;

                case BALLCANNON_CORKH:
                    this->animator.SetAnimation(sVars->aniFrames, 4, true, 0);
                    this->velocity.x = 0x80000;
                    this->state.Set(&BallCannon::State_CorkBlocked);
                    break;
            }
        }
    }
}

void BallCannon::StageLoad()
{
    sVars->aniFrames.Load("OOZ/BallCannon.bin", SCOPE_STAGE);

    sVars->hitboxCannon.top    = -4;
    sVars->hitboxCannon.left   = -4;
    sVars->hitboxCannon.right  = 4;
    sVars->hitboxCannon.bottom = 4;

    sVars->hitboxCorkBlock.top    = -16;
    sVars->hitboxCorkBlock.left   = -16;
    sVars->hitboxCorkBlock.right  = 16;
    sVars->hitboxCorkBlock.bottom = 16;

    sVars->hitboxCorkEntry.top    = -4;
    sVars->hitboxCorkEntry.left   = -8;
    sVars->hitboxCorkEntry.right  = 8;
    sVars->hitboxCorkEntry.bottom = 4;

    sVars->sfxLedgeBreak.Get("Stage/LedgeBreak.wav");
    sVars->sfxFire.Get("Stage/CannonFire.wav");
}

void BallCannon::CheckPlayerEntry()
{
    if (this->CheckOnScreen(nullptr)) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ALL_ENTITIES))
        {
            if (player->CheckValidState()) {
                int32 playerID = RSDKTable->GetEntitySlot(player);

                if (this->playerTimers[playerID]) {
                    this->playerTimers[playerID]--;
                }
                else {
                    if ((1 << playerID) & this->activePlayers) {
                        if (!player->state.Matches(&Player::State_Static))
                            this->activePlayers &= ~(1 << playerID);
                    }
                    else {
                        if (player->CheckCollisionTouch(this, &sVars->hitboxCannon)) {
                            Player::sVars->sfxRoll.Play(false, 0xFF);
                            player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP, false, 0);

                            player->position.x     = this->position.x;
                            player->position.y     = this->position.y;
                            player->velocity.x     = 0;
                            player->velocity.y     = 0;
                            player->tileCollisions = TILECOLLISION_NONE;
                            player->interaction    = false;
                            player->blinkTimer     = 0;
                            player->visible        = false;
                            player->state.Set(&Player::State_Static);
                            player->stateGravity.Set(&Player::Gravity_False);
                            this->activePlayers |= 1 << playerID;
                            this->active   = ACTIVE_NORMAL;
                            this->rotation = (this->angle + this->direction + 1) << 7;
                            this->drawFX   = FX_ROTATE;
                            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
                            this->state.Set(&BallCannon::State_Inserted);
                        }
                    }
                }
            }
        }
    }
    else {
        this->active   = ACTIVE_BOUNDS;
        this->rotation = (this->angle + this->direction + 1) << 7;
    }
}

void BallCannon::State_Idle() { BallCannon::CheckPlayerEntry(); }

void BallCannon::State_Inserted()
{
    BallCannon::CheckPlayerEntry();

    this->animator.Process();

    if (this->animator.frameID == this->animator.frameCount - 1) {
        this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->drawFX = FX_FLIP;
        this->state.Set(&BallCannon::State_Turning);
    }
}

void BallCannon::State_Turning()
{
    BallCannon::CheckPlayerEntry();

    this->animator.Process();

    if (this->animator.frameID == this->animator.frameCount - 1) {
        this->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
        this->drawFX   = FX_ROTATE;
        this->state.Set(&BallCannon::State_EjectPlayer);
        this->rotation = (this->angle - this->direction + 2) << 7;
    }
}

void BallCannon::State_EjectPlayer()
{
    BallCannon::CheckPlayerEntry();

    this->animator.Process();

    if (this->animator.frameID == this->animator.frameCount - 1) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ALL_ENTITIES))
        {
            if (player->CheckValidState()) {
                int32 playerID = RSDKTable->GetEntitySlot(player);

                if (((1 << playerID) & this->activePlayers)) {
                    sVars->sfxFire.Play(false, 0xFF);

                    player->velocity = this->velocity;
                    player->visible  = true;

                    if (this->exit) {
                        player->onGround       = false;
                        player->applyJumpCap   = false;
                        player->state.Set(&Player::State_Air);
                        player->tileCollisions = TILECOLLISION_DOWN;
                        player->interaction    = true;
                    }

                    this->activePlayers &= ~(1 << playerID);
                    this->playerTimers[playerID] = 15;
                }
            }
        }

        this->state.Set(&BallCannon::State_Idle);
    }
}

void BallCannon::State_CorkBlocked(void)
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        Animator animator;

        memcpy(&animator, &player->animator, sizeof(Animator));
        int32 storeX    = player->position.x;
        int32 storeY    = player->position.y;
        int32 storeVelX = player->velocity.x;
        int32 storeVelY = player->velocity.y;

        if (player->CheckCollisionBox(this, &sVars->hitboxCorkBlock) == C_TOP) {
            if (player->animator.animationID == Player::ANI_JUMP || player->state.Matches(&Player::State_DropDash)
            ) {
                if (storeVelY >= 0 && !player->groundedStore) {
                    for (int32 i = 0; i < 16; ++i) {
                        BallCannon *debris = GameObject::Create<BallCannon>(INT_TO_VOID((i & 3) + 1), this->position.x + sVars->corkDebrisOffset[(i * 2) + 0],
                                          this->position.y + sVars->corkDebrisVelocity[(i * 2) + 1]);
                        debris->velocity.x = sVars->corkDebrisVelocity[(i * 2) + 0];
                        debris->velocity.y = sVars->corkDebrisVelocity[(i * 2) + 1];
                    }

                    sVars->sfxLedgeBreak.Play(false, 0xFF);

                    memcpy(&player->animator, &animator, sizeof(Animator));
                    player->velocity.x = storeVelX;
                    player->velocity.y = storeVelY;
                    player->position.x = storeX;
                    player->position.y = storeY;
                    player->onGround   = false;
                    this->active       = ACTIVE_NORMAL;
                    this->visible      = false;
                    this->state.Set(&BallCannon::State_CorkOpened);

                    break;
                }
            }
        }
    }
}

void BallCannon::State_CorkOpened()
{
    if (this->CheckOnScreen(nullptr)) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            int32 playerID = RSDKTable->GetEntitySlot(player);

            if (this->playerTimers[playerID]) {
                this->playerTimers[playerID]--;
            }
            else {
                if (player->CheckCollisionTouch(this, &sVars->hitboxCorkEntry)) {
                    player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP, false, 0);
                    sVars->sfxFire.Play(false, 0xFF);

                    player->state.Set(&Player::State_Static);
                    player->stateGravity.Set(&Player::Gravity_False);
                    player->nextGroundState.Set(nullptr);
                    player->nextAirState.Set(nullptr);
                    player->position             = this->position;
                    player->velocity             = this->velocity;
                    player->tileCollisions       = TILECOLLISION_NONE;
                    player->interaction          = false;
                    player->onGround             = false;
                    this->playerTimers[playerID] = 15;
                }
            }
        }
    }
    else {
        this->visible = true;
        for (int32 i = 0; i < Player::sVars->playerCount; ++i) this->playerTimers[i] = 0;
        this->state.Set(&BallCannon::State_CorkBlocked);
    }
}

void BallCannon::State_CorkDebris()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x3800;

    this->rotation += this->rotationSpeed;

    if (!this->CheckOnScreen(&this->updateRange))
        this->Destroy();
}

#if RETRO_INCLUDE_EDITOR
void BallCannon::EditorDraw()
{
    this->drawFX        = FX_ROTATE | FX_FLIP;
    this->rotation      = 0;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x400000;
    this->updateRange.y = 0x400000;
    this->velocity.x    = 0;
    this->velocity.y    = 0;

    switch (this->type) {
        default: break;

        case BALLCANNON_CANNON:
            if (this->angle >= 4)
                this->direction = FLIP_X;

            this->rotation = (this->angle + this->direction + 1) << 7;

            switch (this->angle) {
                case BALLCANNON_DIR_RIGHT_CW: // Right -> Down
                case BALLCANNON_DIR_LEFT_CCW: // Left -> Down
                    this->velocity.y = 0x200000;
                    break;

                case BALLCANNON_DIR_DOWN_CW: // Down -> Left
                case BALLCANNON_DIR_UP_CCW:  // Up -> Left
                    this->velocity.x = -0x200000;
                    break;

                case BALLCANNON_DIR_LEFT_CW:   // Left -> Up
                case BALLCANNON_DIR_RIGHT_CCW: // Right -> Up
                    this->velocity.y = -0x200000;
                    break;

                case BALLCANNON_DIR_UP_CW:    // Up -> Right
                case BALLCANNON_DIR_DOWN_CCW: // Down -> Right
                    this->velocity.x = 0x200000;
                    break;

                default: break;
            }

            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            break;

        case BALLCANNON_CORKV: this->animator.SetAnimation(sVars->aniFrames, 3, true, 0); break;

        case BALLCANNON_CORKH: this->animator.SetAnimation(sVars->aniFrames, 4, true, 0); break;
    }

    BallCannon::Draw();

    if (showGizmos() && this->type == BALLCANNON_CANNON) {
        this->rotation = (this->angle - this->direction + 2) << 7;

        this->inkEffect = INK_BLEND;

        BallCannon::Draw();

        this->inkEffect = INK_NONE;

        // Draw the direction the player will be shot from (the names are a little confusing on their own)
        DrawHelpers::DrawArrow(this->position.x, this->position.y, this->position.x + this->velocity.x, this->position.y + this->velocity.y, 0x00FF00,
                              INK_NONE, 0xFF);
    }
}

#if RETRO_REV0U
void BallCannon::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(BallCannon);

    int32 corkDebrisOffset[] = { -0xC0000, -0xC0000, -0x40000, -0xC0000, 0x40000,  -0xC0000, 0xC0000, -0xC0000, -0xC0000, -0x40000, -0x40000,
                                 -0x40000, 0x40000,  -0x40000, 0xC0000,  -0x40000, -0xC0000, 0x40000, -0x40000, 0x40000,  0x40000,  0x40000,
                                  0xC0000,  0x40000,  -0xC0000, 0xC0000,  -0x40000, 0xC0000,  0x40000, 0xC0000,  0xC0000,  0xC0000 };
    memcpy(sVars->corkDebrisOffset, corkDebrisOffset, sizeof(corkDebrisOffset));

    int32 corkDebrisVelocity[] = { -0x40000, -0x40000, -0x20000, -0x40000, 0x20000,  -0x40000, 0x40000, -0x40000, -0x3C000, -0x20000, -0x1C000,
                              -0x20000, 0x1C000,  -0x20000, 0x3C000,  -0x20000, -0x38000, 0x20000, -0x18000, 0x20000,  0x18000,  0x20000,
                               0x38000,  0x20000,  -0x34000, 0x40000,  -0x14000, 0x40000,  0x14000, 0x40000,  0x34000,  0x40000 };
    memcpy(sVars->corkDebrisVelocity, corkDebrisVelocity, sizeof(corkDebrisVelocity));
}
#endif

void BallCannon::EditorLoad()
{
    sVars->aniFrames.Load("OOZ/BallCannon.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Cannon", BALLCANNON_CANNON);
    RSDK_ENUM_VAR("Cork V", BALLCANNON_CORKV);
    RSDK_ENUM_VAR("Cork H", BALLCANNON_CORKH);

    RSDK_ACTIVE_VAR(sVars, angle);
    RSDK_ENUM_VAR("Right (Rotates Clockwise)", BALLCANNON_DIR_RIGHT_CW);
    RSDK_ENUM_VAR("Down (Rotates Clockwise)", BALLCANNON_DIR_DOWN_CW);
    RSDK_ENUM_VAR("Left (Rotates Clockwise)", BALLCANNON_DIR_LEFT_CW);
    RSDK_ENUM_VAR("Up (Rotates Clockwise)", BALLCANNON_DIR_UP_CW);
    RSDK_ENUM_VAR("Down (Rotates Anti-Clockwise)", BALLCANNON_DIR_DOWN_CCW);
    RSDK_ENUM_VAR("Left (Rotates Anti-Clockwise)", BALLCANNON_DIR_LEFT_CCW);
    RSDK_ENUM_VAR("Up (Rotates Anti-Clockwise)", BALLCANNON_DIR_UP_CCW);
    RSDK_ENUM_VAR("Right (Rotates Anti-Clockwise)", BALLCANNON_DIR_RIGHT_CCW);
}
#endif

void BallCannon::Serialize()
{
    RSDK_EDITABLE_VAR(BallCannon, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(BallCannon, VAR_ENUM, angle);
    RSDK_EDITABLE_VAR(BallCannon, VAR_BOOL, exit);
}
} // namespace GameLogic