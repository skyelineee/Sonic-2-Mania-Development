// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ForceSpin Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "ForceSpin.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ForceSpin);

void ForceSpin::Update()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (!player->CheckValidState())
            continue;

        Vector2 pivotPos = player->position;
        Vector2 pivotVel = player->velocity;

        Zone::RotateOnPivot(&pivotPos, &this->position, this->negAngle);
        Zone::RotateOnPivot(&pivotVel, &this->velocity, this->negAngle);

        if (abs(pivotPos.x - this->position.x) < 0x180000 && abs(pivotPos.y - this->position.y) < this->size << 19) {
            if (pivotPos.x >= this->position.x) {
                if (this->direction) {
                    if (player->state.Matches(&Player::State_TubeRoll) || player->state.Matches(&Player::State_TubeAirRoll)) {
                        player->state.Set(player->onGround ? &Player::State_Roll : &Player::State_Air);
                        player->nextGroundState.Set(nullptr);
                        player->nextAirState.Set(nullptr);
                    }
                }
                else {
                    SetPlayerState(player);
                }
            }
            else {
                if (!this->direction) {
                    if (player->state.Matches(&Player::State_TubeRoll) || player->state.Matches(&Player::State_TubeAirRoll)) {
                        player->state.Set(player->onGround ? &Player::State_Roll : &Player::State_Air);
                        player->nextGroundState.Set(nullptr);
                        player->nextAirState.Set(nullptr);
                    }
                }
                else {
                    SetPlayerState(player);
                }
            }
        }
    }

    this->visible = DebugMode::sVars->debugActive;
}
void ForceSpin::LateUpdate() {}
void ForceSpin::StaticUpdate() {}
void ForceSpin::Draw() { DrawSprites(); }

void ForceSpin::Create(void *data)
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->animator.frameID = 4;
    this->drawFX |= FX_FLIP;

    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = abs(this->size * Math::Sin256(this->angle) << 11) + 0x200000;
        this->updateRange.y = abs(this->size * Math::Cos256(this->angle) << 11) + 0x200000;
        this->visible       = false;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->negAngle      = (uint8) - (uint8)this->angle;
    }
}

void ForceSpin::StageLoad() { sVars->aniFrames.Load("Global/PlaneSwitch.bin", SCOPE_STAGE); }

void ForceSpin::DrawSprites()
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
}
void ForceSpin::SetPlayerState(Player *player)
{
    if (!player->state.Matches(&Player::State_TubeRoll) && !player->state.Matches(&Player::State_TubeAirRoll)) {
        if (player->animator.animationID != Player::ANI_JUMP) {
            Player::sVars->sfxRoll.Play();
            player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP, false, 0);
            if (!player->collisionMode && player->onGround)
                player->position.y += player->jumpOffset;
            player->pushing = 0;
        }

        player->state.Set(player->onGround ? &Player::State_TubeRoll : &Player::State_TubeAirRoll);
        player->nextGroundState.Set(nullptr);
        player->nextAirState.Set(nullptr);

        if (abs(player->groundVel) < 0x10000) {
            if (this->direction & FLIP_X)
                player->groundVel = -0x40000;
            else
                player->groundVel = 0x40000;
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void ForceSpin::EditorDraw() { DrawSprites(); }

void ForceSpin::EditorLoad()
{
    sVars->aniFrames.Load("Global/PlaneSwitch.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("U/S");
    RSDK_ENUM_VAR("S/U");
}
#endif

#if RETRO_REV0U
void ForceSpin::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(ForceSpin);

    sVars->aniFrames.Init();
}
#endif

void ForceSpin::Serialize()
{
    RSDK_EDITABLE_VAR(ForceSpin, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(ForceSpin, VAR_ENUM, size);
    RSDK_EDITABLE_VAR(ForceSpin, VAR_INT32, angle);
}

} // namespace GameLogic