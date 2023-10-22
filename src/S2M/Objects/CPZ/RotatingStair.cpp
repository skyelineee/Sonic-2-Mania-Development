// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: RotatingStair Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "RotatingStair.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Helpers/DrawHelpers.hpp"
#include "Common/Platform.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(RotatingStair);

void RotatingStair::Update()
{ 
    Platform *platform = (Platform *)this;
    platform->Update();
}

void RotatingStair::LateUpdate() {}

void RotatingStair::StaticUpdate() {}

void RotatingStair::Draw() { this->animator.DrawSprite(&this->drawPos, false); }

void RotatingStair::Create(void *data)
{
    this->frameID   = 2;
    this->collision = Platform::C_Solid;
    this->speed     = 3;

    if (this->direction)
        this->amplitude.x = -this->amplitude.x;

    if (this->mode & 1)
        this->amplitude.x = -this->amplitude.x;

    int32 typeStore = this->mode;
    this->mode      = (RotatingStairModes)Platform::Linear;
    Platform *platform = (Platform *)this;
    platform->Create(nullptr);
    this->mode = typeStore;

    if (this->mode >= 4) {
        this->duration = 120 * this->speed;
        this->state.Set(&RotatingStair::State_Move_Intervals);
        this->interval = this->duration + 512;
    }
    else {
        this->state.Set(&RotatingStair::State_Move);
    }
}

void RotatingStair::StageLoad() { sVars->unused = 0; }

void RotatingStair::State_Move()
{
    int32 timer = Zone::sVars->timer + this->oscOff;
    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;

    int32 dir = 0;
    if (this->mode & 1)
        dir = this->mode - (((3 * timer) >> 9) & 3) - 2;
    else
        dir = ((3 * timer) >> 9) + this->mode;

    switch (dir & 3) {
        case FLIP_NONE: // right
            this->drawPos.x = this->centerPos.x + this->amplitude.x * Math::Cos1024(timer * this->speed);
            this->drawPos.y = this->centerPos.y + (this->amplitude.y << 10);
            break;

        case FLIP_X: // up
            this->drawPos.x = this->centerPos.x - (this->amplitude.x << 10);
            this->drawPos.y = this->centerPos.y - this->amplitude.y * Math::Cos1024(timer * this->speed);
            break;

        case FLIP_Y: // left
            this->drawPos.x = this->centerPos.x - this->amplitude.x * Math::Cos1024(timer * this->speed);
            this->drawPos.y = this->centerPos.y - (this->amplitude.y << 10);
            break;

        case FLIP_XY: // down
            this->drawPos.x = this->centerPos.x + (this->amplitude.x << 10);
            this->drawPos.y = this->centerPos.y + this->amplitude.y * Math::Cos1024(timer * this->speed);
            break;
    }

    this->velocity.x = drawX + this->drawPos.x;
    this->velocity.y = drawY + this->drawPos.y;
}

void RotatingStair::State_Move_Intervals(void)
{
    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;

    int32 angle = 0;
    if (this->speed * (Zone::sVars->timer + this->oscOff) % this->interval >= this->duration)
        angle = this->speed * (Zone::sVars->timer + this->oscOff) % this->interval - this->duration;

    int32 dir = 0;
    if (this->mode & 1)
        dir = this->mode - ((this->speed * (Zone::sVars->timer + this->oscOff) / this->interval) & 3) - 2;
    else
        dir = this->speed * (Zone::sVars->timer + this->oscOff) / this->interval + this->mode;

    switch (dir & 3) {
        case FLIP_NONE: // right
            this->drawPos.x = this->centerPos.x + this->amplitude.x * Math::Cos1024(angle);
            this->drawPos.y = this->centerPos.y + (this->amplitude.y << 10);
            break;

        case FLIP_X: // up
            this->drawPos.x = this->centerPos.x - (this->amplitude.x << 10);
            this->drawPos.y = this->centerPos.y - this->amplitude.y * Math::Cos1024(angle + 0x200);
            break;

        case FLIP_Y: // left
            this->drawPos.x = this->centerPos.x - this->amplitude.x * Math::Cos1024(angle);
            this->drawPos.y = this->centerPos.y - (this->amplitude.y << 10);
            break;

        case FLIP_XY: // down
            this->drawPos.x = this->centerPos.x + (this->amplitude.x << 10);
            this->drawPos.y = this->centerPos.y + this->amplitude.y * Math::Cos1024(angle + 0x200);
            break;
    }

    this->velocity.x = drawX + this->drawPos.x;
    this->velocity.y = drawY + this->drawPos.y;
}

#if RETRO_INCLUDE_EDITOR
void RotatingStair::EditorDraw()
{
    this->frameID   = 2;
    this->collision = Platform::C_Solid;
    this->speed     = 3;
    if (this->direction)
        this->amplitude.x = -this->amplitude.x;
    if (this->mode & 1)
        this->amplitude.x = -this->amplitude.x;

    int32 typeStore = this->mode;
    this->mode         = (RotatingStairModes)Platform::Linear;
    Platform *platform = (Platform *)this;
    platform->Create(nullptr);
    this->mode = typeStore;

    // this->drawPos = this->position;

    Vector2 amplitude = this->amplitude;

    if (this->direction)
        this->amplitude.x = -this->amplitude.x;
    if (this->mode & 1)
        this->amplitude.x = -this->amplitude.x;

    if (this->mode >= 4) {
        this->duration = 120 * this->speed;
        this->interval = this->duration + 512;
        RotatingStair::State_Move_Intervals();
    }
    else {
        RotatingStair::State_Move();
    }

    RotatingStair::Draw();

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        for (int32 s = sceneInfo->entitySlot + 1, i = 0; i < this->childCount; ++i) {
            //Entity *child = RSDK_GET_ENTITY_GEN(s + i);
            Entity *child = GameObject::Get(s + i);
            if (!child)
                continue;

            DrawHelpers::DrawArrow(this->drawPos.x, this->drawPos.y, child->position.x, child->position.y, 0xFFFF00, INK_NONE, 0xFF);
        }

        RSDK_DRAWING_OVERLAY(false);
    }

    this->amplitude = amplitude;
}

void RotatingStair::EditorLoad()
{

    RSDK_ACTIVE_VAR(sVars, mode);
    RSDK_ENUM_VAR("Start Left", ROTATINGSTAIR_LEFT);
    RSDK_ENUM_VAR("Start Down", ROTATINGSTAIR_DOWN);
    RSDK_ENUM_VAR("Start Right", ROTATINGSTAIR_RIGHT);
    RSDK_ENUM_VAR("Start Up", ROTATINGSTAIR_UP);
    RSDK_ENUM_VAR("Start Left (Use Intervals)", ROTATINGSTAIR_LEFT_INTERVAL);
    RSDK_ENUM_VAR("Start Down (Use Intervals)", ROTATINGSTAIR_DOWN_INTERVAL);
    RSDK_ENUM_VAR("Start Right (Use Intervals)", ROTATINGSTAIR_RIGHT_INTERVAL);
    RSDK_ENUM_VAR("Start Up (Use Intervals)", ROTATINGSTAIR_UP_INTERVAL);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip", FLIP_NONE);
    RSDK_ENUM_VAR("Flipped", FLIP_X);
}
#endif

void RotatingStair::Serialize()
{
    RSDK_EDITABLE_VAR(RotatingStair, VAR_ENUM, mode);
    RSDK_EDITABLE_VAR(RotatingStair, VAR_VECTOR2, amplitude);
    RSDK_EDITABLE_VAR(RotatingStair, VAR_ENUM, childCount);
    RSDK_EDITABLE_VAR(RotatingStair, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(RotatingStair, VAR_UINT16, oscOff);
}
} // namespace GameLogic