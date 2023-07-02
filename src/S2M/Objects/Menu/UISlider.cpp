// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UISlider Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UISlider.hpp"
#include "UIWidgets.hpp"
#include "UIControl.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UISlider);

void UISlider::Update() 
{
    this->touchPosOffsetS.y = this->buttonBounceOffset;
    this->touchPosOffsetS.x = 0x7A0000 + this->buttonBounceOffset;

    this->state.Run(this);

    UIControl *control = (UIControl *)this->parent;

    int32 id = -1;
    for (int32 i = 0; i < control->buttonCount; ++i) {
        if (this == (UISlider *)control->buttons[i]) {
            id = i;
            break;
        }
    }

    if (this->state.Matches(&UISlider::State_HandleButtonEnter) && (!control->state.Matches(&UIControl::ProcessInputs) || control->buttonID != id)) {
        this->isSelected         = false;
        this->buttonBounceOffset = 0;

        if (this->isSelected) {
            this->isSelected = false;
            this->state.Set(&UISlider::State_HandleButtonLeave);
        }
    }
}

void UISlider::LateUpdate() {}
void UISlider::StaticUpdate() {}
void UISlider::Draw()
{
    UISlider::DrawSlider();
    UISlider::DrawFGShapes();
}

void UISlider::Create(void *data) 
{
    this->visible       = true;
    this->drawGroup     = 2;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x400000;

    this->touchPosOffsetS.y = 0;
    this->touchPosOffsetS.x = 0x7A0000;
    this->touchPosSizeS.x   = 0x940000;
    this->touchPosSizeS.y   = 0x180000;
    this->size.x            = 0x5A0000;
    this->size.y            = 0x180000;
    this->bgEdgeSize        = 24;

    this->processButtonCB.Set(&UISlider::ButtonPressCB);
    this->touchCB.Set(&UISlider::TouchCB);
    this->selectedCB.Set(nullptr);
    this->failCB.Set(nullptr);
    this->buttonEnterCB.Set(&UISlider::ButtonEnterCB);
    this->buttonLeaveCB.Set(&UISlider::ButtonLeaveCB);
    this->checkButtonEnterCB.Set(&UISlider::CheckButtonEnterCB);
    this->checkSelectedCB.Set(&UISlider::CheckSelectedCB);

    this->textVisible = true;
    this->sliderPos   = (UISLIDER_MAX - UISLIDER_MIN) / 2;
    this->textAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, this->listID, true, this->frameID);
}

void UISlider::StageLoad() { sVars->aniFrames.Load("UI/Slider.bin", SCOPE_STAGE); }

void UISlider::DrawFGShapes()
{
    Vector2 drawPos;
    int32 width = this->size.y + this->size.x;
    drawPos.x   = this->position.x + this->buttonBounceOffset;
    drawPos.y   = this->position.y;
    this->bgShapeAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
    this->bgShapeAnimator.DrawSprite(&drawPos, false);

    if (this->textVisible) {
        drawPos.x = this->position.x + this->buttonBounceOffset - TO_FIXED(18);
        drawPos.y = this->position.y;
        this->textAnimator.DrawSprite(&drawPos, false);
    }
}

void UISlider::DrawSlider()
{
    Vector2 drawPos;
    int32 drawX     = 0x7A0000 + this->position.x;
    int32 sliderPos = (((34048 * this->sliderPos) >> 2));
    int32 drawX2    = drawX - ((TO_FIXED(10) - sliderPos));

    drawPos.x       = this->position.x + this->buttonBounceOffset + TO_FIXED(156);
    drawPos.y       = this->position.y;
    this->sliderFrame = this->sliderPos / 64;
    this->sliderAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->sliderAnimator.DrawSprite(&drawPos, false);
    this->sliderAnimator.SetAnimation(sVars->aniFrames, 1, true, this->sliderFrame);
    this->sliderAnimator.DrawSprite(&drawPos, false);

    this->buttonAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    drawPos.x = drawX2 + (sliderPos >> 1) + this->buttonBounceOffset;
    drawPos.y = this->position.y;
 
    drawPos.x -= TO_FIXED(57);
    this->buttonAnimator.DrawSprite(&drawPos, false);

    if (this->isSelected) {
        drawPos.y = drawPos.y - TO_FIXED(24);
        UIWidgets::sVars->arrowDownAnimator.DrawSprite(&drawPos, false);
    }
}

void UISlider::ButtonPressCB()
{
    UIControl *parent = (UIControl *)this->parent;

    int32 rowID = 0;
    if (parent->rowCount && parent->columnCount)
        rowID = parent->buttonID / parent->columnCount;

    int32 columnID = 0;
    if (parent->columnCount)
        columnID = parent->buttonID % parent->columnCount;

    bool32 moveV = false;
    if (parent->rowCount > 1) {
        if (UIControl::sVars->anyUpPress) {
            --rowID;
            moveV = true;
        }

        if (UIControl::sVars->anyDownPress) {
            ++rowID;
            moveV = true;
        }
    }

    bool32 valueChanged = false;
    if (UIControl::sVars->anyLeftPress && this->sliderPos > UISLIDER_MIN) {
        this->sliderPos = (this->sliderPos & -UISLIDER_INCREMENT) - UISLIDER_INCREMENT;
        valueChanged    = true;
    }

    if (UIControl::sVars->anyRightPress && this->sliderPos < UISLIDER_MAX) {
        this->sliderPos = (this->sliderPos & -UISLIDER_INCREMENT) + UISLIDER_INCREMENT;
        valueChanged    = true;
    }

    if (valueChanged) {
        UIWidgets::sVars->sfxBleep.Play(false, 255);
        this->sliderChangedCB.Run(this);
    }

    if (moveV) {
        if (rowID < 0)
            rowID += parent->rowCount;

        if (rowID >= parent->rowCount)
            rowID -= parent->rowCount;

        int32 max = parent->buttonCount - 1;
        if (rowID * parent->columnCount + columnID < max)
            max = rowID * parent->columnCount + columnID;

        if (parent->buttonID != max) {
            parent->buttonID = max;
            if (this->isSelected) {
                this->isSelected = false;
                this->state.Set(&UISlider::State_HandleButtonLeave);
            }

            UIWidgets::sVars->sfxBleep.Play(false, 255);
        }
    }
    else {
        int32 id = -1;
        for (int32 i = 0; i < parent->buttonCount; ++i) {
            if (this == (UISlider *)parent->buttons[i]) {
                id = i;
                break;
            }
        }

        if (!this->isSelected && parent->buttonID == id && parent->state.Matches(&UIControl::ProcessInputs) && !parent->dialogHasFocus)
            UISlider::ButtonEnterCB();
    }
}

bool32 UISlider::TouchCB()
{
    bool32 touchPressed = false;
    if (touchInfo->count) {
        if (!UISlider::sVars->activeEntity || UISlider::sVars->activeEntity == (Entity *)this) {
            int32 sizeX = this->touchPosSizeS.x >> 1;
            int32 sizeY = this->touchPosSizeS.y >> 1;

            for (int32 i = 0; i < touchInfo->count; ++i) {
                int32 x = (screenInfo->position.x << 16) - ((touchInfo->x[i] * screenInfo->size.x) * -65536.0f);
                int32 y = (screenInfo->position.y << 16) - ((touchInfo->y[i] * screenInfo->size.y) * -65536.0f);

                int32 touchX = abs(this->touchPosOffsetS.x + this->position.x - x);
                int32 touchY = abs(this->touchPosOffsetS.y + this->position.y - y);
                if (!this->isTouchSelected && touchX < sizeX && touchY < sizeY) {
                    this->isTouchSelected = true;
                    UIWidgets::sVars->sfxBleep.Play(false, 255);
                }

                if (this->isTouchSelected) {
                    touchPressed           = true;
                    UISlider::sVars->activeEntity = (Entity *)this;

                    this->sliderPosTouch = MAX(x + sizeX - this->position.x - this->touchPosOffsetS.x, 0x70000);
                    if (this->touchPosSizeS.x - 0x70000 < this->sliderPosTouch)
                        this->sliderPosTouch = this->touchPosSizeS.x - 0x70000;

                    int32 sliderPos = 16
                                      * (MIN(((this->sliderPosTouch - 0x70000) >> 4 << 10) / (this->touchPosSizeS.x - 0xE0000) + 2, UISLIDER_MAX)
                                         & -(UISLIDER_INCREMENT / 0x10));
                    if (sliderPos != this->sliderPos) {
                        this->sliderPos = sliderPos;
                        this->sliderChangedCB.Run(this);
                    }
                }
            }

            this->touchPressed = touchPressed;
            return touchPressed;
        }
    }
    else {
        UISlider::sVars->activeEntity = nullptr;
    }

    if (this->touchPressed)
        UIWidgets::sVars->sfxBleep.Play(false, 255);

    this->isTouchSelected = false;
    this->touchPressed    = false;
    return false;
}

void UISlider::ButtonEnterCB()
{
    if (!this->isSelected) {
        this->isSelected           = true;
        this->buttonBounceOffset   = 0;
        this->buttonBounceVelocity = -0x25000;
        this->state.Set(&UISlider::State_HandleButtonEnter);
    }
}

void UISlider::ButtonLeaveCB()
{
    if (this->isSelected) {
        this->isSelected = false;
        this->state.Set(&UISlider::State_HandleButtonLeave);
    }
}

bool32 UISlider::CheckButtonEnterCB()
{
    return this->isSelected;
}

bool32 UISlider::CheckSelectedCB() { return false; }

void UISlider::State_HandleButtonLeave()
{
    if (this->buttonBounceOffset) {
        int32 offset = -(this->buttonBounceOffset / abs(this->buttonBounceOffset));
        this->buttonBounceOffset += offset << 16;

        if (offset < 0 && this->buttonBounceOffset < 0)
            this->buttonBounceOffset = 0;
        else if (offset > 0 && this->buttonBounceOffset > 0)
            this->buttonBounceOffset = 0;
    }
}

void UISlider::State_HandleButtonEnter()
{
    this->buttonBounceVelocity += 0x4800;
    this->buttonBounceOffset += this->buttonBounceVelocity;

    if (this->buttonBounceOffset >= -0x20000 && this->buttonBounceVelocity > 0) {
        this->buttonBounceOffset   = 0;
        this->buttonBounceVelocity = 0;
    }
}

#if RETRO_INCLUDE_EDITOR
void UISlider::EditorDraw() 
{
    this->inkEffect = this->disabled ? INK_BLEND : INK_NONE;
    UISlider::Draw();
}

void UISlider::EditorLoad() {}
#endif

void UISlider::Serialize()
{
    RSDK_EDITABLE_VAR(UISlider, VAR_BOOL, disabled);
    RSDK_EDITABLE_VAR(UISlider, VAR_ENUM, listID);
    RSDK_EDITABLE_VAR(UISlider, VAR_ENUM, frameID);
}

} // namespace GameLogic