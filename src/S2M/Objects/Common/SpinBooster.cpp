// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SpinBooster Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SpinBooster.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SpinBooster);

void SpinBooster::Update()
{
    int32 negAngle = -this->angle & 0xFF;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        int32 playerID = 1 << RSDKTable->GetEntitySlot(player);

        Vector2 pivotPos = player->position;
        Zone::RotateOnPivot(&pivotPos, &this->position, negAngle);

        if (abs(pivotPos.x - this->position.x) < TO_FIXED(24) && abs(pivotPos.y - this->position.y) < this->size << 19) {
            if (pivotPos.x >= this->position.x) {
                if (!(playerID & this->activePlayers)) {
                    SpinBooster::HandleForceRoll(player);
                    this->activePlayers |= playerID;
                }
            }
            else {
                // Exit Tube
                if ((playerID & this->activePlayers) && !this->forwardOnly) {
                    if (player->state.Matches(&Player::State_TubeRoll) || player->state.Matches(&Player::State_TubeAirRoll)) {
                        player->nextAirState.Set(nullptr);
                        player->nextGroundState.Set(nullptr);

                        if (!this->allowTubeInput)
                            player->controlLock = 0;

                        player->tileCollisions = TILECOLLISION_DOWN;
                        player->state.Set(player->onGround ? &Player::State_Roll : &Player::State_Air);
                    }
                }

                this->activePlayers &= ~playerID;
            }
        }
        else {
            if (pivotPos.x >= this->position.x)
                this->activePlayers |= playerID;
            else
                this->activePlayers &= ~playerID;
        }
    }

    this->visible = DebugMode::sVars->debugActive;
}

void SpinBooster::LateUpdate() {}

void SpinBooster::StaticUpdate() {}

void SpinBooster::Draw() { SpinBooster::DrawSprites(); }

void SpinBooster::Create(void *data)
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->drawFX |= FX_FLIP;
    this->animator.frameID = 4;
    this->activePlayers    = 0;

    if (sceneInfo->inEditor) {
        if (!this->boostPower)
            this->boostPower = 15;
    }
    else {
        switch (this->direction) {
            case FLIP_NONE: this->angle = 0x00; break;
            case FLIP_X: this->angle = 0x40; break;
            case FLIP_Y: this->angle = 0x80; break;
            case FLIP_XY: this->angle = 0xC0; break;
        }

        this->active = ACTIVE_BOUNDS;

        this->updateRange.x = this->size * abs(Math::Sin256(this->angle)) << 11;
        this->updateRange.x += 0x200000;

        this->updateRange.y = this->size * abs(Math::Cos256(this->angle)) << 11;
        this->updateRange.y += 0x200000;
        this->visible   = false;
        this->drawGroup = Zone::sVars->objectDrawGroup[0];
    }
}

void SpinBooster::StageLoad() { sVars->aniFrames .Load("Global/PlaneSwitch.bin", SCOPE_STAGE); }

int32 SpinBooster::GetRollDir(Player *player)
{
    switch (this->direction) {
        case FLIP_NONE:
            if (this->autoGrip != 5) {
                if ((this->autoGrip == 7 || this->autoGrip == 8) && player->up)
                    return CMODE_ROOF;

                return CMODE_FLOOR;
            }

            if (player->down)
                return CMODE_FLOOR;

            if (player->up)
                return CMODE_ROOF;

            return !this->bias ? CMODE_FLOOR : CMODE_ROOF;

        case FLIP_X:
            if (this->autoGrip != 5) {
                if (this->autoGrip == 7) {
                    if (!player->left)
                        return -1;

                    return CMODE_RWALL;
                }

                if (this->autoGrip == 6) {
                    if (!player->right)
                        return -1;

                    return CMODE_LWALL;
                }

                if (this->autoGrip != 8)
                    return -1;

                if (!player->left) {
                    if (!player->right)
                        return -1;

                    return CMODE_LWALL;
                }
                else {
                    return CMODE_RWALL;
                }
            }
            else {
                if (player->left)
                    return CMODE_RWALL;

                if (player->right)
                    return CMODE_LWALL;

                return !this->bias ? CMODE_LWALL : CMODE_RWALL;
            }

        case FLIP_Y:
            if (this->autoGrip != 5) {
                if ((this->autoGrip == 6 || this->autoGrip == 8) && player->up)
                    return CMODE_ROOF;

                return CMODE_FLOOR;
            }

            if (player->down)
                return CMODE_FLOOR;

            if (player->up)
                return CMODE_ROOF;

            return !this->bias ? CMODE_ROOF : CMODE_FLOOR;

        case FLIP_XY:
            if (this->autoGrip == 5) {
                return !player->left ? CMODE_LWALL : CMODE_RWALL;
            }
            else if (this->autoGrip == 6) {
                if (!player->left)
                    return -1;

                return CMODE_RWALL;
            }
            else {
                if (this->autoGrip == 7) {
                    if (!player->right)
                        return -1;

                    return CMODE_LWALL;
                }

                if (this->autoGrip != 8)
                    return -1;

                if (!player->left) {
                    if (!player->right)
                        return -1;

                    return CMODE_LWALL;
                }
                else {
                    return CMODE_RWALL;
                }
            }

        default: break;
    }

    return -1;
}
void SpinBooster::HandleRollDir(Player *player)
{
    if (this->autoGrip) {
        int8 cMode = this->autoGrip - 1;
        if (this->autoGrip >= 5)
            cMode = SpinBooster::GetRollDir(player);

        if (cMode < 0) {
            player->onGround = false;
            return;
        }

        int32 offsetX  = 0;
        int32 offsetY  = 0;
        int32 plrAngle = 0;
        int32 angle    = 0;

        switch (cMode) {
            case CMODE_FLOOR:
                plrAngle = 0x00;
                angle    = 0x40;
                break;

            case CMODE_LWALL:
                plrAngle = -0x40;
                angle    = 0x00;
                break;

            case CMODE_ROOF:
                plrAngle = -0x80;
                angle    = -0x40;
                break;

            case CMODE_RWALL:
                plrAngle = 0x40;
                angle    = -0x80;
                break;

            default:
                plrAngle = 0x00;
                angle    = 0x00;
                break;
        }

        Hitbox *playerHitbox = player->GetHitbox();
        switch (cMode) {
            case CMODE_FLOOR: offsetY = playerHitbox->bottom << 16; break;
            case CMODE_LWALL: offsetX = playerHitbox->right << 16; break;
            case CMODE_ROOF: offsetY = playerHitbox->top << 16; break;
            case CMODE_RWALL: offsetX = playerHitbox->left << 16; break;
            default: break;
        }

        player->position.x += Math::Cos256(angle) << 8;
        player->position.y += Math::Sin256(angle) << 8;
        player->TileGrip(Zone::sVars->collisionLayers, cMode, 0, offsetX, offsetY, 1);

        player->angle = plrAngle;
        if (player->onGround) {
            uint8 mode = 0;
            switch (player->collisionMode) {
                case CMODE_FLOOR: mode = CMODE_ROOF; break;
                case CMODE_LWALL: mode = CMODE_RWALL; break;
                case CMODE_ROOF: mode = CMODE_FLOOR; break;
                case CMODE_RWALL: mode = CMODE_LWALL; break;
            }

            if (cMode == mode)
                player->groundVel = -player->groundVel;
        }
        else {
            switch (cMode) {
                case CMODE_FLOOR: player->groundVel = player->velocity.x; break;
                case CMODE_LWALL: player->groundVel = -player->velocity.y; break;
                case CMODE_ROOF: player->groundVel = -player->velocity.x; break;
                case CMODE_RWALL: player->groundVel = player->velocity.y; break;
                default: break;
            }
        }

        player->collisionMode = cMode;
        player->onGround      = true;
    }
}
void SpinBooster::ApplyRollVelocity(Player *player)
{
    if (player->onGround) {
        int32 entAng = Math::Cos256(this->angle) + Math::Sin256(this->angle);
        int32 plrAng = Math::Cos256(player->angle) - Math::Sin256(player->angle);
        int32 power  = (this->boostPower << 15) * ((plrAng > 0) - (plrAng < 0)) * ((entAng > 0) - (entAng < 0));

        if (this->boostPower >= 0)
            player->groundVel += power;
        else
            player->groundVel = power;
    }
    else {
        int32 x = (0x80 * Math::Cos256(this->angle)) * this->boostPower;
        int32 y = (-0x80 * Math::Sin256(this->angle)) * this->boostPower;

        if (this->boostPower >= 0) {
            player->velocity.x += x;
            player->velocity.y += y;
        }
        else {
            player->velocity.x = x;
            player->velocity.y = y;
        }
    }

    if (this->boostPower < 0 && !this->forwardOnly) {
        if (player->state.Matches(&Player::State_TubeRoll) || player->state.Matches(&Player::State_TubeAirRoll)) {
            player->nextAirState.Set(nullptr);
            player->nextGroundState.Set(nullptr);

            if (!this->allowTubeInput)
                player->controlLock = 0;

            player->tileCollisions = TILECOLLISION_DOWN;
            player->state.Set(player->onGround ? &Player::State_Roll : &Player::State_Air);
        }
    }
}
void SpinBooster::DrawArrow(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color)
{
    int32 angle = Math::ATan2(x1 - x2, y1 - y2);

    Graphics::DrawLine(x1, y1, x2, y2, color, 0x7F, INK_ADD, false);
    Graphics::DrawLine(x2, y2, x2 + (Math::Cos256(angle + 12) << 12), y2 + (Math::Sin256(angle + 12) << 12), color, 0x7F, INK_ADD, false);
    Graphics::DrawLine(x2, y2, x2 + (Math::Cos256(angle - 12) << 12), y2 + (Math::Sin256(angle - 12) << 12), color, 0x7F, INK_ADD, false);
}
void SpinBooster::DrawSprites()
{
    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y - (this->size << 19);
    Zone::RotateOnPivot(&drawPos, &this->position, this->angle);

    for (int32 i = 0; i < this->size; ++i) {
        this->animator.DrawSprite(&drawPos, false);
        drawPos.x += Math::Sin256(this->angle) << 12;
        drawPos.y += Math::Cos256(this->angle) << 12;
    }

    if (sceneInfo->inEditor) {
        uint8 negAngle = -this->angle & 0xFF;
        int32 power    = this->boostPower;
        int32 x        = this->position.x;
        int32 y        = this->position.y;
        int32 clr      = ((power >> 31) & 0xFE0001) + 0xFFFF;
        if (!power)
            power = 1;

        int32 x2 = power * (Math::Cos256(negAngle) << 11) + this->position.x;
        int32 y2 = power * (Math::Sin256(negAngle) << 11) + this->position.y;
        SpinBooster::DrawArrow(x, y, x2, y2, clr);

        switch (this->autoGrip) {
            default: break;
            case 1:
            case 2:
            case 3:
            case 4: {
                int8 angle = 0;

                switch (this->autoGrip) {
                    default: break;
                    case 1: angle = 0x40; break;
                    case 2: angle = 0x00; break;
                    case 3: angle = 0xC0; break;
                    case 4: angle = 0x80; break;
                }

                x2 = (Math::Cos256(angle) << 14) + this->position.x;
                y2 = (Math::Sin256(angle) << 14) + this->position.y;
                SpinBooster::DrawArrow(x, y, x2, y2, 0x00FF00);
                break;
            }

            case 5:
                clr = !this->bias ? 0x00FF00 : 0xFFFF00;

                x2 = (Math::Cos256((negAngle + 64) & 0xFF) << 14) + this->position.x;
                y2 = (Math::Sin256((negAngle + 64) & 0xFF) << 14) + this->position.y;
                SpinBooster::DrawArrow(x, y, x2, y2, clr);

                x2 = (Math::Cos256((negAngle - 64) & 0xFF) << 14) + this->position.x;
                y2 = (Math::Sin256((negAngle - 64) & 0xFF) << 14) + this->position.y;
                SpinBooster::DrawArrow(x, y, x2, y2, clr);
                break;

            case 6:
                clr = 0xFFFF00;

                x2 = (Math::Cos256((negAngle + 64) & 0xFF) << 14) + this->position.x;
                y2 = (Math::Sin256((negAngle + 64) & 0xFF) << 14) + this->position.y;
                SpinBooster::DrawArrow(x, y, x2, y2, clr);

                x2 = (Math::Cos256(negAngle) << 14) + this->position.x;
                y2 = (Math::Sin256(negAngle) << 14) + this->position.y;
                SpinBooster::DrawArrow(x, y, x2, y2, 0x00FF00);
                break;

            case 0:
            case 7:
                clr = 0xFFFF00;

                x2 = (Math::Cos256((negAngle - 64) & 0xFF) << 14) + this->position.x;
                y2 = (Math::Sin256((negAngle - 64) & 0xFF) << 14) + this->position.y;
                SpinBooster::DrawArrow(x, y, x2, y2, clr);

                x2 = (Math::Cos256(negAngle) << 14) + this->position.x;
                y2 = (Math::Sin256(negAngle) << 14) + this->position.y;
                SpinBooster::DrawArrow(x, y, x2, y2, 0x00FF00);
                break;

            case 8:
                clr = 0xFFFF00;

                x2 = (Math::Cos256((negAngle + 64) & 0xFF) << 14) + this->position.x;
                y2 = (Math::Sin256((negAngle + 64) & 0xFF) << 14) + this->position.y;
                SpinBooster::DrawArrow(x, y, x2, y2, clr);

                x2 = (Math::Cos256((negAngle - 64) & 0xFF) << 14) + this->position.x;
                y2 = (Math::Sin256((negAngle - 64) & 0xFF) << 14) + this->position.y;
                SpinBooster::DrawArrow(x, y, x2, y2, clr);

                x2 = (Math::Cos256(negAngle) << 14) + this->position.x;
                y2 = (Math::Sin256(negAngle) << 14) + this->position.y;
                SpinBooster::DrawArrow(x, y, x2, y2, 0x00FF00);
                break;
        }
    }
}
void SpinBooster::HandleForceRoll(Player *player)
{
    player->tileCollisions = TILECOLLISION_DOWN;
    SpinBooster::HandleRollDir(player);

    if (player->state.Matches(&Player::State_TubeRoll) || player->state.Matches(&Player::State_TubeAirRoll)) {
        if (this->boostAlways)
            SpinBooster::ApplyRollVelocity(player);
    }
    else {
        if (this->playSound)
            Player::sVars->sfxRoll.Play(false, 0xFF);

        if (player->animator.animationID != Player::ANI_JUMP) {
            player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP, false, 0);
            if (!player->collisionMode && player->onGround)
                player->position.y += player->jumpOffset;
            player->pushing = false;
        }

        player->nextAirState.Set(nullptr);
        player->nextGroundState.Set(nullptr);

        if (!this->allowTubeInput)
            player->controlLock = 0xFFFF;

        player->state.Set(player->onGround ? &Player::State_TubeRoll : &Player::State_TubeAirRoll);

        if (abs(player->groundVel) < TO_FIXED(1))
            player->groundVel = (this->direction & FLIP_X) ? -TO_FIXED(4) : TO_FIXED(4);

        SpinBooster::ApplyRollVelocity(player);
    }
}

#if RETRO_INCLUDE_EDITOR
void SpinBooster::EditorDraw() { SpinBooster::DrawSprites(); }

void SpinBooster::EditorLoad()
{
    sVars->aniFrames.Load("Global/PlaneSwitch.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip");
    RSDK_ENUM_VAR("Flip X");
    RSDK_ENUM_VAR("Flip Y");
    RSDK_ENUM_VAR("Flip XY");

    RSDK_ACTIVE_VAR(sVars, bias);
    RSDK_ENUM_VAR("No Bias");
    RSDK_ENUM_VAR("Has Bias");
}
#endif

void SpinBooster::Serialize()
{
    RSDK_EDITABLE_VAR(SpinBooster, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_UINT8, autoGrip);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_UINT8, bias);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_ENUM, size);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_INT32, boostPower);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_BOOL, boostAlways);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_BOOL, forwardOnly);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_BOOL, playSound);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_BOOL, allowTubeInput);
}

} // namespace GameLogic