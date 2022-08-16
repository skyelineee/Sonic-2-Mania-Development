// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: PlayerProbe Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "PlayerProbe.hpp"
#include "LogHelpers.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(PlayerProbe);

void PlayerProbe::Update()
{
    // Pretty much just an edit of the PlaneSwitch collision code
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 playerID   = player->Slot();

        Vector2 pivotPos = player->position;
        Vector2 pivotVel = player->velocity;

        Zone::RotateOnPivot(&pivotPos, &this->position, this->negAngle);
        Zone::RotateOnPivot(&pivotVel, &this->velocity, this->negAngle);

        if (abs(pivotPos.x - this->position.x) < 0x180000 && abs(pivotPos.y - this->position.y) < this->size << 19) {
            if (pivotPos.x >= this->position.x) {
                if (!this->direction) {
                    if (!((1 << playerID) & this->activePlayers))
                        Print(player);

                    this->activePlayers |= 1 << playerID;
                }
            }
            else {
                if (this->direction) {
                    if (!((1 << playerID) & this->activePlayers))
                        Print(player);

                    this->activePlayers |= 1 << playerID;
                }
            }
        }
        else {
            this->activePlayers &= ~(1 << playerID);
        }
    }

    this->visible = DebugMode::sVars->debugActive;
}
void PlayerProbe::LateUpdate() {}
void PlayerProbe::StaticUpdate() {}
void PlayerProbe::Draw() { DrawSprites(); }

void PlayerProbe::Create(void *data)
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    this->drawFX |= FX_FLIP;
    this->active           = ACTIVE_BOUNDS;
    this->animator.frameID = 4;

    this->updateRange.x = abs(this->size * Math::Sin256(this->angle) << 11) + 0x200000;
    this->updateRange.y = abs(this->size * Math::Cos256(this->angle) << 11) + 0x200000;
    this->visible       = false;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->activePlayers = 0;
    this->negAngle      = (uint8) - (this->angle & 0xFF);
}

void PlayerProbe::StageLoad() { sVars->aniFrames.Load("Global/PlaneSwitch.bin", SCOPE_STAGE); }

void PlayerProbe::Print(Player *player)
{
    if (!sceneInfo->inEditor) {
        LogHelpers::Print("====================");
        LogHelpers::Print("= Begin Probe      =");
        LogHelpers::Print("====================");

        if (this->direction)
            LogHelpers::Print("direction = S/U");
        else
            LogHelpers::Print("direction = U/S");

        LogHelpers::Print("angle = %i", this->angle);
        LogHelpers::Print("Cos256(angle) = %i", Math::Cos256(this->angle));
        LogHelpers::Print("Sin256(angle) = %i", Math::Sin256(this->angle));
        LogHelpers::Print("====================");

        if (player->direction)
            LogHelpers::Print("direction = FACING_LEFT");
        else
            LogHelpers::Print("direction = FACING_RIGHT");

        LogHelpers::Print("playerPtr->groundVel = %i", player->groundVel);
        LogHelpers::Print("playerPtr->angle = %i", player->angle);
        LogHelpers::Print("playerPtr->collisionMode = %i", player->collisionMode);
        LogHelpers::Print("playerPtr->onGround = %i", player->onGround);

        LogHelpers::Print("====================");
        LogHelpers::Print("= End Probe        =");
        LogHelpers::Print("====================");
    }
}

void PlayerProbe::DrawSprites()
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
        uint8 angle = -(uint8)(this->angle);
        if (this->direction)
            angle = -0x80 - (uint8)(this->angle);

        int32 x2 = this->position.x + 0x5000 * Math::Cos256(angle);
        int32 y2 = this->position.y + 0x5000 * Math::Sin256(angle);
        DrawArrow(this->position.x, this->position.y, x2, y2, !this->direction ? 0x00FFFF : 0xFF00FF);
    }
}

void PlayerProbe::DrawArrow(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color)
{
    int32 angle = Math::ATan2(x1 - x2, y1 - y2);

    Graphics::DrawLine(x1, y1, x2, y2, color, 0x7F, INK_ADD, false);
    Graphics::DrawLine(x2, y2, x2 + (Math::Cos256(angle + 12) << 12), (Math::Sin256(angle + 12) << 12) + y2, color, 0x7F, INK_ADD, false);
    Graphics::DrawLine(x2, y2, (Math::Cos256(angle - 12) << 12) + x2, (Math::Sin256(angle - 12) << 12) + y2, color, 0x7F, INK_ADD, false);
}

#if RETRO_INCLUDE_EDITOR
void PlayerProbe::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    this->drawFX |= FX_FLIP;
    this->animator.frameID = 4;

    this->negAngle = (uint8) - (this->angle & 0xFF);

    DrawSprites();
}

void PlayerProbe::EditorLoad()
{
    sVars->aniFrames.Load("Global/PlaneSwitch.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("U/S");
    RSDK_ENUM_VAR("S/U");
}
#endif

#if RETRO_REV0U
void PlayerProbe::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(PlayerProbe);

    sVars->aniFrames.Init();
}
#endif

void PlayerProbe::Serialize()
{
    RSDK_EDITABLE_VAR(PlayerProbe, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(PlayerProbe, VAR_ENUM, size);
    RSDK_EDITABLE_VAR(PlayerProbe, VAR_INT32, angle);
}

} // namespace GameLogic