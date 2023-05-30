// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UISubHeading Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UISubHeading.hpp"
#include "UIWidgets.hpp"
#include "UIButton.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UISubHeading);

void UISubHeading::Update() 
{
    if (!this->textFrames.Matches(&UIWidgets::sVars->textFrames) || this->storedListID != this->listID || this->storedFrameID != this->frameID) {
        this->animator.SetAnimation(&UIWidgets::sVars->textFrames, this->listID, true, this->frameID);
        this->textFrames    = UIWidgets::sVars->textFrames;
        this->storedListID  = this->listID;
        this->storedFrameID = this->frameID;
    }

    this->state.Run(this);
}

void UISubHeading::LateUpdate() {}
void UISubHeading::StaticUpdate() {}
void UISubHeading::Draw()
{
    Vector2 drawPos;

    int32 size = (this->size.x + this->size.y) >> 16;
    drawPos.x  = this->position.x;
    drawPos.y  = this->position.y;
    UIWidgets::DrawParallelogram(this->position.x, this->position.y, size, this->size.y >> 16, this->bgEdgeSize, 0x00, 0x00, 0x00);

    drawPos = this->position;
    switch (this->align) {
        case UIButton::ALIGN_LEFT: drawPos.x += -0x60000 - (this->size.x >> 1); break;

        default:
        case UIButton::ALIGN_CENTER: break;

        case UIButton::ALIGN_RIGHT:
            drawPos.x -= 0x60000;
            drawPos.x += this->size.x >> 1;
            break;
    }

    drawPos.x += this->offset;
    this->animator.DrawSprite(&drawPos, false);
}

void UISubHeading::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->offset <<= 16;
        this->visible       = true;
        this->drawGroup     = 2;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x400000;
        this->bgEdgeSize    = this->size.y >> 16;
        this->size.y        = abs(this->size.y);

        this->animator.SetAnimation(&UIWidgets::sVars->textFrames, this->listID, true, this->frameID);
        this->textFrames = UIWidgets::sVars->textFrames;
    }
}

void UISubHeading::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void UISubHeading::EditorDraw() 
{
    if (this->offset < 0x10000)
        this->offset <<= 16;

    this->drawGroup     = 2;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x400000;
    this->bgEdgeSize    = this->size.y >> 16;
    this->size.y        = abs(this->size.y);
    this->animator.SetAnimation(&UIWidgets::sVars->textFrames, this->listID, true, this->frameID);
    this->textFrames = UIWidgets::sVars->textFrames;

    UISubHeading::Draw();
}

void UISubHeading::EditorLoad()
{
    RSDK_ACTIVE_VAR(sVars, align);
    RSDK_ENUM_VAR("Left", UIButton::ALIGN_LEFT);
    RSDK_ENUM_VAR("Center", UIButton::ALIGN_CENTER);
    RSDK_ENUM_VAR("Right", UIButton::ALIGN_RIGHT);
}
#endif

void UISubHeading::Serialize()
{
    RSDK_EDITABLE_VAR(UISubHeading, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(UISubHeading, VAR_ENUM, listID);
    RSDK_EDITABLE_VAR(UISubHeading, VAR_ENUM, frameID);
    RSDK_EDITABLE_VAR(UISubHeading, VAR_ENUM, align);
    RSDK_EDITABLE_VAR(UISubHeading, VAR_ENUM, offset);
}

} // namespace GameLogic