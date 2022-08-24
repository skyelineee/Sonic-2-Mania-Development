// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Decoration Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Decoration.hpp"
#include "Global/Zone.hpp"
#include "ScreenWrap.hpp"
#include "Water.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Decoration);

void Decoration::Update()
{
    this->animator.Process();

    this->rotation = (this->rotation + this->rotSpeed) & 0x1FF;
}
void Decoration::LateUpdate() {}
void Decoration::StaticUpdate() {}
void Decoration::Draw()
{
    uint8 startDir = this->direction;

    if (this->reverseX) {
        uint8 dirs[] = { FLIP_NONE, FLIP_Y, FLIP_X, FLIP_XY };

        int32 dir    = FLIP_XY - startDir;
        int32 newDir = dirs[startDir];
        if (startDir > FLIP_XY)
            dir = this->direction;
        if (startDir > FLIP_XY)
            newDir = this->direction;

        this->direction = this->reverseY ? dir : newDir;
    }

    DrawSprites();

    this->direction = startDir;

    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Decoration::Draw), true);
}

void Decoration::Create(void *data)
{
    this->drawFX = FX_FLIP;
    if (sceneInfo->inEditor) {
        this->animator.SetAnimation(sVars->aniFrames, this->type, true, this->subtype);
    }
    else {
        uint8 rangeTypes[] = { ACTIVE_NORMAL, ACTIVE_YBOUNDS, ACTIVE_XBOUNDS, ACTIVE_ALWAYS };
        if (this->rangeType < 4)
            this->active = rangeTypes[this->rangeType];

        this->visible = true;

        if (this->rotSpeed) {
            this->drawFX   = FX_ROTATE | FX_FLIP;
            this->rotation = this->startRot;
        }

        this->updateRange.x = ((this->repeatTimes.x * this->repeatSpacing.x) >> 16) + 0x800000;
        this->updateRange.y = ((this->repeatTimes.y * this->repeatSpacing.y) >> 16) + 0x800000;

        this->animator.SetAnimation(sVars->aniFrames, this->type, true, this->subtype);

        switch (this->animator.GetFrameID()) {
            case '1': this->drawGroup = Zone::sVars->objectDrawGroup[1]; break;
            case '2': this->drawGroup = Zone::sVars->playerDrawGroup[0] - 1; break;
            case '3': this->drawGroup = Zone::sVars->playerDrawGroup[1] - 1; break;
            case '4': this->drawGroup = Zone::sVars->playerDrawGroup[0] + 1; break;
            case '5': this->drawGroup = Zone::sVars->playerDrawGroup[1] + 1; break;
            case '6': this->drawGroup = Zone::sVars->objectDrawGroup[0] + 1; break;
            case '7': this->drawGroup = Zone::sVars->objectDrawGroup[1] - 2; break;
            case '8': this->drawGroup = Zone::sVars->objectDrawGroup[0] - 1; break;
            default: this->drawGroup = Zone::sVars->objectDrawGroup[0]; break;
        }

        if (ScreenWrap::CheckCompetitionWrap())
            this->active = ACTIVE_NORMAL;
    }
}

void Decoration::StageLoad()
{
    RSDK_DYNAMIC_PATH_ACTID("Decoration");

    sVars->aniFrames.Load(dynamicPath, SCOPE_STAGE);
}

void Decoration::DrawSprites()
{
    Vector2 drawPos, repeat;

    repeat.x  = this->repeatTimes.x >> 16;
    repeat.y  = this->repeatTimes.y >> 16;
    drawPos.x = this->position.x - ((repeat.x * this->repeatSpacing.x) >> 1);
    drawPos.y = this->position.y - ((repeat.y * this->repeatSpacing.y) >> 1);

    for (int32 y = 0; y < repeat.y + 1; ++y) {
        drawPos.x = this->position.x - (repeat.x * this->repeatSpacing.x >> 1);
        for (int32 x = 0; x < repeat.x + 1; ++x) {
            this->animator.DrawSprite(&drawPos, false);
            drawPos.x += this->repeatSpacing.x;
        }
        drawPos.y += this->repeatSpacing.y;
    }
}

#if RETRO_INCLUDE_EDITOR
void Decoration::EditorDraw()
{
    uint8 startDir = this->direction;

    if (this->reverseX) {
        uint8 dirs[] = { FLIP_NONE, FLIP_Y, FLIP_X, FLIP_XY };

        int32 dir = FLIP_XY - startDir;
        int32 newDir = dirs[startDir];
        if (startDir > FLIP_XY)
            dir = this->direction;
        if (startDir > FLIP_XY)
            newDir = this->direction;

        this->direction = this->reverseY ? dir : newDir;
    }

    this->animator.SetAnimation(sVars->aniFrames, this->type, true, this->subtype);
    DrawSprites();

    this->direction = startDir;
}

void Decoration::EditorLoad()
{
    RSDK_DYNAMIC_PATH_ACTID("Decoration");

    sVars->aniFrames.Load(dynamicPath, SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Normal");
    RSDK_ENUM_VAR("HFlip");
    RSDK_ENUM_VAR("VFlip");
    RSDK_ENUM_VAR("HVFlip");

    RSDK_ACTIVE_VAR(sVars, rangeType);
    RSDK_ENUM_VAR("XY");
    RSDK_ENUM_VAR("X");
    RSDK_ENUM_VAR("Y");
    RSDK_ENUM_VAR("Always");

    RSDK_ACTIVE_VAR(sVars, inkEffect);
    RSDK_ENUM_VAR("None");
    RSDK_ENUM_VAR("Blended");
    RSDK_ENUM_VAR("Alpha");
    RSDK_ENUM_VAR("Additive");
    RSDK_ENUM_VAR("Subtractive");
}
#endif

#if RETRO_REV0U
void Decoration::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Decoration);

    sVars->aniFrames.Init();
}
#endif

void Decoration::Serialize()
{
    RSDK_EDITABLE_VAR(Decoration, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(Decoration, VAR_UINT8, subtype);
    RSDK_EDITABLE_VAR(Decoration, VAR_INT32, rangeType);
    RSDK_EDITABLE_VAR(Decoration, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(Decoration, VAR_INT32, rotSpeed);
    RSDK_EDITABLE_VAR(Decoration, VAR_INT32, startRot);
    RSDK_EDITABLE_VAR(Decoration, VAR_VECTOR2, repeatTimes);
    RSDK_EDITABLE_VAR(Decoration, VAR_VECTOR2, repeatSpacing);
    RSDK_EDITABLE_VAR(Decoration, VAR_UINT8, inkEffect);
    RSDK_EDITABLE_VAR(Decoration, VAR_INT32, alpha);
    RSDK_EDITABLE_VAR(Decoration, VAR_BOOL, reverseX);
    RSDK_EDITABLE_VAR(Decoration, VAR_BOOL, reverseY);
}

} // namespace GameLogic