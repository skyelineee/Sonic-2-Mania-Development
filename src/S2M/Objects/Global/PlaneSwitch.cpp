// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: PlaneSwitch Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "PlaneSwitch.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "DebugMode.hpp"
#include "Common/ScreenWrap.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(PlaneSwitch);

void PlaneSwitch::Update()
{
    this->visible = DebugMode::sVars->debugActive;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        CheckCollisions(player, this->flags, this->size, true, Zone::sVars->playerDrawGroup[0], Zone::sVars->playerDrawGroup[1]);
    }

    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&PlaneSwitch::Update), true);
}
void PlaneSwitch::LateUpdate() {}
void PlaneSwitch::StaticUpdate() {}
void PlaneSwitch::Draw()
{
    DrawSprites();
    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&PlaneSwitch::Draw), true);
}

void PlaneSwitch::Create(void *data)
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    if (!sceneInfo->inEditor) {
        this->active = ACTIVE_BOUNDS;

        this->updateRange.x = abs(this->size * Math::Sin256(this->angle) << 11) + 0x200000;
        this->updateRange.y = abs(this->size * Math::Cos256(this->angle) << 11) + 0x200000;
        this->visible       = false;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->negAngle      = (uint8) - (this->angle & 0xFF);

        if (ScreenWrap::CheckCompetitionWrap())
            this->active = ACTIVE_NORMAL;
    }
}

void PlaneSwitch::StageLoad() { sVars->aniFrames.Load("Global/PlaneSwitch.bin", SCOPE_STAGE); }

void PlaneSwitch::DrawSprites()
{
    Vector2 drawPos;

    drawPos.x = this->position.x - 0x80000;
    drawPos.y = this->position.y - (this->size << 19);
    Zone::RotateOnPivot(&drawPos, &this->position, this->angle);

    this->animator.frameID = this->flags & 3;
    for (int32 i = 0; i < this->size; ++i) {
        this->animator.DrawSprite(&drawPos, false);
        drawPos.x += Math::Sin256(this->angle) << 12;
        drawPos.y += Math::Cos256(this->angle) << 12;
    }

    drawPos.x = this->position.x + 0x80000;
    drawPos.y = this->position.y - (this->size << 19);
    Zone::RotateOnPivot(&drawPos, &this->position, this->angle);

    this->animator.frameID = (this->flags >> 2) & 3;
    for (int32 i = 0; i < this->size; ++i) {
        this->animator.DrawSprite(&drawPos, false);
        drawPos.x += Math::Sin256(this->angle) << 12;
        drawPos.y += Math::Cos256(this->angle) << 12;
    }
}

// Custom function, but it allows me to majorly shrink any planeSwitch code
// it's based on all the repeated planeswitch code, just made more modular
// (and also it means you can add planeswitches to basically anything with ease :P)
void PlaneSwitch::CheckCollisions(RSDK::GameObject::Entity *other, int32 flags, int32 size, bool32 switchDrawOrder, uint8 low, uint8 high)
{
    Vector2 pivotPos = other->position;
    Vector2 pivotVel = other->velocity;

    Zone::RotateOnPivot(&pivotPos, &this->position, this->negAngle);
    Zone::RotateOnPivot(&pivotVel, &this->velocity, this->negAngle);

    if (this->onPath == PlaneSwitch::Either || (this->onPath == PlaneSwitch::GroundOnly && other->onGround)
        || (this->onPath == PlaneSwitch::AirOnly && !other->onGround)) {
        if (abs(pivotPos.x - this->position.x) < 0x180000 && abs(pivotPos.y - this->position.y) < size << 19) {
            if (pivotPos.x + pivotVel.x >= this->position.x) {
                other->collisionPlane = (flags >> 3) & 1; // collision plane bit
                if (switchDrawOrder) {
                    if (!(flags & 4)) // priority bit
                        other->drawGroup = low;
                    else
                        other->drawGroup = high;
                }
            }
            else {
                other->collisionPlane = (flags >> 1) & 1; // collision plane bit
                if (switchDrawOrder) {
                    if (!(flags & 1)) // priority bit
                        other->drawGroup = low;
                    else
                        other->drawGroup = high;
                }
            }
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void PlaneSwitch::EditorDraw()
{
    this->angle &= 0xFF;
    this->flags &= 0xF;
    if (!this->size)
        this->size = 1;

    DrawSprites();
}

void PlaneSwitch::EditorLoad()
{
    sVars->aniFrames.Load("Global/PlaneSwitch.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, flags);
    RSDK_ENUM_VAR("AL - AL");
    RSDK_ENUM_VAR("AH - AL");
    RSDK_ENUM_VAR("BL - AL");
    RSDK_ENUM_VAR("BH - AL");
    RSDK_ENUM_VAR("AL - AH");
    RSDK_ENUM_VAR("AH - AH");
    RSDK_ENUM_VAR("BL - AH");
    RSDK_ENUM_VAR("BH - AH");
    RSDK_ENUM_VAR("AL - BL");
    RSDK_ENUM_VAR("AH - BL");
    RSDK_ENUM_VAR("BL - BL");
    RSDK_ENUM_VAR("BH - BL");
    RSDK_ENUM_VAR("AL - BH");
    RSDK_ENUM_VAR("AH - BH");
    RSDK_ENUM_VAR("BL - BH");
    RSDK_ENUM_VAR("BH - BH");

    RSDK_ACTIVE_VAR(sVars, onPath);
    RSDK_ENUM_VAR("Either");
    RSDK_ENUM_VAR("Only On Ground");
    RSDK_ENUM_VAR("Only In Air");
}
#endif

#if RETRO_REV0U
void PlaneSwitch::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(PlaneSwitch);

    sVars->aniFrames.Init();
}
#endif

void PlaneSwitch::Serialize()
{
    RSDK_EDITABLE_VAR(PlaneSwitch, VAR_ENUM, flags);
    RSDK_EDITABLE_VAR(PlaneSwitch, VAR_ENUM, size);
    RSDK_EDITABLE_VAR(PlaneSwitch, VAR_INT32, angle);
    RSDK_EDITABLE_VAR(PlaneSwitch, VAR_UINT8, onPath);
}

} // namespace GameLogic