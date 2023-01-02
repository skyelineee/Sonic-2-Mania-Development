// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIChoice Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIChoice.hpp"
#include "UIButton.hpp"
#include "UIWidgets.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIChoice);

void UIChoice::Update()
{
    UIButton *parent = (UIButton *)this->parent;
    bool32 disabled        = this->disabled;

    this->touchPosSizeS.x   = this->size.x;
    this->touchPosSizeS.y   = this->size.y;
    this->touchPosOffsetS.y = 0;
    this->touchPosSizeS.x   = (this->touchPosSizeS.x + 3 * this->size.y) >> 1;
    this->touchPosSizeS.y   = this->size.y + 0x60000;
    this->touchPosOffsetS.x = -(this->touchPosSizeS.x >> 1);

    if (parent && (disabled || parent->disabled))
        disabled = true;

    if (!this->aniFrames.Matches(&UIWidgets::sVars->textFrames) || this->isDisabled != disabled) {
        if (disabled)
            this->labelAnimator.SetAnimation(&UIWidgets::sVars->textFrames, 7, true, 0);
        else
            this->labelAnimator.SetAnimation(&UIWidgets::sVars->textFrames, this->listID, true, this->frameID);

        this->aniFrames  = UIWidgets::sVars->textFrames;
        this->isDisabled = disabled;
    }

    this->state.Run(this);

    if (parent && parent->state.Matches(&UIButton::State_HandleButtonLeave)) {
        this->textBounceOffset   = 0;
        this->buttonBounceOffset = 0;
        this->isSelected         = false;
        this->state.Set(&UIChoice::State_HandleButtonLeave);
    }
}
void UIChoice::LateUpdate() {}
void UIChoice::StaticUpdate() {}
void UIChoice::Draw()
{
    Vector2 drawPos;
    int32 size = (this->size.x + this->size.y) >> 16;

    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    drawPos.x -= this->buttonBounceOffset;
    drawPos.y -= this->buttonBounceOffset;

    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    drawPos.x += this->buttonBounceOffset;
    drawPos.y += this->buttonBounceOffset;

    UIButton *parent = (UIButton *)this->parent;
    if (this->arrowWidth > 0 && this->isSelected && !(this->disabled || parent->disabled)) {
        drawPos.x = this->position.x;
        drawPos.y = this->position.y;
        drawPos.y += this->textBounceOffset;
        drawPos.x -= this->arrowWidth << 15;
        drawPos.x += this->buttonBounceOffset;
        drawPos.y += this->buttonBounceOffset;
        this->leftArrowAnimator.DrawSprite(&drawPos, false);

        drawPos.x = this->position.x;
        drawPos.y = this->position.y;
        drawPos.y += this->textBounceOffset;
        drawPos.x += this->arrowWidth << 15;
        drawPos.x += this->buttonBounceOffset;
        drawPos.y += this->buttonBounceOffset;
        this->rightArrowAnimator.DrawSprite(&drawPos, false);
    }

    if (this->textVisible) {
        drawPos.x = this->position.x;
        drawPos.y = this->position.y;
        drawPos.y = this->textBounceOffset + this->position.y;
        drawPos.x = this->buttonBounceOffset + this->position.x;
        drawPos.y += this->buttonBounceOffset;

        switch (this->align) {
            case UIButton::ALIGN_LEFT: drawPos.x += -0x60000 - (this->size.x >> 1); break;

            default:
            case UIButton::ALIGN_CENTER: break;

            case UIButton::ALIGN_RIGHT: drawPos.x += (this->size.x >> 1) - 0x60000; break;
        }

        if (this->auxIcon)
            this->iconAnimator.DrawSprite(&drawPos, false);

        if (!this->noText)
            this->labelAnimator.DrawSprite(&drawPos, false);
    }
}

void UIChoice::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->visible       = true;
        this->drawGroup     = 2;
        this->active        = ACTIVE_NEVER;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x400000;
        this->bgEdgeSize    = this->size.y >> 16;
        this->size.y        = abs(this->size.y);
        this->textVisible   = true;

        this->touchCB.Set(&UIChoice::CheckTouch);

        this->aniFrames = UIWidgets::sVars->textFrames;
        this->labelAnimator.SetAnimation(&UIWidgets::sVars->textFrames, this->listID, true, this->frameID);
        this->iconAnimator.SetAnimation(&sVars->aniFrames, this->auxListID, true, this->auxFrameID);
        this->leftArrowAnimator.SetAnimation(&UIWidgets::sVars->uiFrames, 2, true, 0);
        this->rightArrowAnimator.SetAnimation(&UIWidgets::sVars->uiFrames, 2, true, 1);
    }
}

void UIChoice::StageLoad() { sVars->aniFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE); }

void UIChoice::SetChoiceActive(UIChoice *choice)
{
    if (choice) {
        UIButton *parent = (UIButton *)choice->parent;

        choice->active = ACTIVE_BOUNDS;
        if (!parent->disabled)
            choice->visible = true;

        choice->textBounceOffset     = 0;
        choice->buttonBounceOffset   = 0;
        choice->textBounceVelocity   = -0x20000;
        choice->buttonBounceVelocity = -0x20000;
        choice->isSelected           = true;
        choice->state.Set(&UIChoice::State_HandleButtonEnter);
    }
}

void UIChoice::SetChoiceInactive(UIChoice *choice)
{
    if (choice) {
        choice->textBounceOffset   = 0;
        choice->buttonBounceOffset = 0;
        choice->isSelected         = false;
        choice->state.Set(&UIChoice::State_HandleButtonLeave);
    }
}

void UIChoice::TouchedCB_Left()
{
    UIButton *parent = (UIButton *)this->parent;

    int32 selection = parent->selection;
    if (--selection < 0) {
        while (selection < 0) selection += parent->choiceCount;
    }
    UIButton *choice = UIButton::GetChoicePtr(parent, selection);

    while ((choice && choice->disabled) && selection != parent->selection) {
        if (--selection < 0) {
            while (selection < 0) selection += parent->choiceCount;
        }

        choice = UIButton::GetChoicePtr(parent, selection);
    }

    if (selection != parent->selection) {
        UIButton::SetChoiceSelectionWithCB(parent, selection);
        UIWidgets::sVars->sfxBleep.Play(false, 255);
    }
}

void UIChoice::TouchedCB_Right()
{
    UIButton *parent = (UIButton *)this->parent;

    int32 selection        = (parent->selection + 1) % parent->choiceCount;
    UIButton *choice = UIButton::GetChoicePtr(parent, selection);

    while ((choice && choice->disabled) && selection != parent->selection) {
        selection = (selection + 1) % parent->choiceCount;
        choice    = UIButton::GetChoicePtr(parent, selection);
    }

    if (selection != parent->selection) {
        UIButton::SetChoiceSelectionWithCB(parent, selection);
        UIWidgets::sVars->sfxBleep.Play(false, 255);
    }
}

bool32 UIChoice::CheckTouch()
{
    StateMachine<UIChoice> *callbacks[2];
    Vector2 touchStart[2];
    Vector2 touchEnd[2];

    callbacks[0]->Set(&UIChoice::TouchedCB_Left);
    callbacks[1]->Set(&UIChoice::TouchedCB_Right);

    touchStart[0].x = this->touchPosSizeS.x;
    touchStart[0].y = this->touchPosSizeS.y;
    touchStart[1].x = this->touchPosSizeS.x;
    touchStart[1].y = this->touchPosSizeS.y;

    touchEnd[0].x = this->touchPosOffsetS.x;
    touchEnd[0].y = this->touchPosOffsetS.y;
    touchEnd[1].x = -this->touchPosOffsetS.x;
    touchEnd[1].y = this->touchPosOffsetS.y;

    bool32 pressed = false;
    for (int32 i = 0; i < 2; ++i) {
        if (touchInfo->count) {
            int32 sizeX = touchStart[i].x >> 1;
            int32 sizeY = touchStart[i].y >> 1;

            for (int32 t = 0; t < touchInfo->count; ++t) {
                int32 x = (screenInfo->position.x << 16) - ((touchInfo->x[t] * screenInfo->size.x) * -65536.0f);
                int32 y = (screenInfo->position.y << 16) - ((touchInfo->y[t] * screenInfo->size.y) * -65536.0f);

                int32 touchX = abs(touchEnd[i].x + this->position.x - x);
                int32 touchY = abs(touchEnd[i].y + this->position.y - y);
                if (touchX < sizeX && touchY < sizeY) {
                    this->touchID = i;
                    pressed       = true;
                }
            }
        }
        else if (this->touchPressed && this->touchID == i && !this->disabled) {
#if RETRO_USE_MOD_LOADER
            callbacks[i]->Run(this);
#else
            callbacks[i]();
#endif
        }
    }

    this->touchPressed = pressed;

    return pressed;
}

void UIChoice::State_HandleButtonLeave()
{
    if (this->textBounceOffset) {
        int32 offset = -(this->textBounceOffset / abs(this->textBounceOffset));
        this->textBounceOffset += offset << 15;

        if (offset < 0 && this->textBounceOffset < 0)
            this->textBounceOffset = 0;
        else if (offset > 0 && this->textBounceOffset > 0)
            this->textBounceOffset = 0;
    }

    if (this->buttonBounceOffset) {
        int32 offset = -(this->buttonBounceOffset / abs(this->buttonBounceOffset));
        this->buttonBounceOffset += offset << 16;

        if (offset < 0 && this->buttonBounceOffset < 0)
            this->buttonBounceOffset = 0;
        else if (offset > 0 && this->buttonBounceOffset > 0)
            this->buttonBounceOffset = 0;
    }
}

void UIChoice::State_HandleButtonEnter()
{
    this->textBounceVelocity += 0x4000;
    this->textBounceOffset += this->textBounceVelocity;

    if (this->textBounceOffset >= 0 && this->textBounceVelocity > 0) {
        this->textBounceOffset   = 0;
        this->textBounceVelocity = 0;
    }

    this->buttonBounceVelocity += 0x4800;
    this->buttonBounceOffset += this->buttonBounceVelocity;

    if (this->buttonBounceOffset >= -0x20000 && this->buttonBounceVelocity > 0) {
        this->buttonBounceOffset   = -0x20000;
        this->buttonBounceVelocity = 0;
    }
}

#if RETRO_INCLUDE_EDITOR
void UIChoice::EditorDraw() {}

void UIChoice::EditorLoad() {}
#endif

void UIChoice::Serialize() {}

} // namespace GameLogic