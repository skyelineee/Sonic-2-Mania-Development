// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIWinSize Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIWinSize.hpp"
#include "UIControl.hpp"
#include "UIWidgets.hpp"
#include "UIButton.hpp"
#include "Global/Localization.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIWinSize);

void UIWinSize::Update()
{
    this->touchPosSizeS.x   = this->size.x;
    this->touchPosSizeS.y   = this->size.y;
    this->touchPosOffsetS.y = 0;
    this->touchPosSizeS.x   = (this->touchPosSizeS.x + 3 * this->size.y) >> 1;
    this->touchPosSizeS.y   = this->size.y + 0x60000;
    this->touchPosOffsetS.x = -this->touchPosSizeS.x;

    if (this->selection != this->prevSelection) {
        UIWinSize::SetupText(this);
        this->prevSelection = this->selection;
    }

    this->state.Run(this);

    UIControl *control = (UIControl *)this->parent;
    if (control && control->state.Matches(&UIButton::State_HandleButtonLeave))
        UIWinSize::SetChoiceInactive(this);

    this->visible = true;
}

void UIWinSize::LateUpdate() {}

void UIWinSize::StaticUpdate() {}

void UIWinSize::Draw()
{
    Vector2 drawPos;
    int32 width = (this->size.y + this->size.x) >> 16;

    drawPos = this->position;

    drawPos.x = this->buttonBounceOffset + this->position.x;
    UIWidgets::DrawParallelogram(drawPos.x, drawPos.y, width, this->size.y >> 16, this->bgEdgeSize, 0x00, 0x00, 0x00);

    if (this->arrowWidth > 0 && this->isSelected) {
        drawPos = this->position;
        drawPos.x -= this->arrowWidth << 15;
        drawPos.x += this->buttonBounceOffset;
        this->arrowAnimatorL.DrawSprite(&drawPos, false);

        drawPos = this->position;
        drawPos.x += this->arrowWidth << 15;
        drawPos.x += this->buttonBounceOffset;
        this->arrowAnimatorR.DrawSprite(&drawPos, false);
    }

    if (this->textVisible) {
        drawPos = this->position;
        drawPos.x = this->buttonBounceOffset + this->position.x;

        switch (this->align) {
            case UIButton::ALIGN_LEFT: drawPos.x = -0x60000 - (this->size.x >> 1) + drawPos.x; break;

            default:
            case UIButton::ALIGN_CENTER: break;

            case UIButton::ALIGN_RIGHT: drawPos.x = drawPos.x + (this->size.x >> 1) - 0x60000; break;
        }

        drawPos.x -= this->text.GetWidth(UIWidgets::sVars->fontFrames, 0, 0, this->text.length, 0) << 15;
        this->textAnimator.DrawString(&drawPos, &this->text, 0, this->text.length, UIButton::ALIGN_LEFT, 0, nullptr, false);
    }
}

void UIWinSize::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->visible       = true;
        this->drawGroup     = 2;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x400000;

        this->selection  = Graphics::GetVideoSetting(VIDEOSETTING_WINDOW_HEIGHT) / SCREEN_YSIZE;
        this->bgEdgeSize = this->size.y >> 16;
        this->size.y     = abs(this->size.y);

        this->textVisible = true;
        this->text.Init("", 0x100);
        this->processButtonCB.Set(&UIWinSize::ProcessButtonCB);
        this->touchCB.Set(&UIWinSize::ProcessTouchCB);

        this->arrowAnimatorL.SetAnimation(&UIWidgets::sVars->uiFrames, 2, true, 0);
        this->arrowAnimatorR.SetAnimation(&UIWidgets::sVars->uiFrames, 2, true, 1);
        this->textAnimator.SetAnimation(&UIWidgets::sVars->fontFrames, 0, true, 0);
    }
}

void UIWinSize::StageLoad() { UIWinSize::sVars->aniFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE); }

void UIWinSize::SetupText(UIWinSize *entityPtr)
{
    if (SKU->platform == PLATFORM_PC || SKU->platform == PLATFORM_DEV) {
        int32 height = 0;
        Graphics::GetWindowSize(nullptr, &height);

        this->maxScale = 5;
        if (this->selection < 1)
            this->selection = this->maxScale - 1;

        if (this->selection >= this->maxScale)
            this->selection = 1;

        char buffer[0x10];
        sprintf_s(buffer, (int32)sizeof(buffer), "%ix", this->selection);

        entityPtr->text.Set(buffer);
        if (Localization::sVars->language == LANGUAGE_TC) {
            for (int32 c = 0; c < entityPtr->text.length; ++c) {
                if (entityPtr->text.chars[c] == 'x')
                    entityPtr->text.chars[c] = 20493; // unicode character ID
            }
        }

        entityPtr->text.SetSpriteString(UIWidgets::sVars->fontFrames, 0);
    }
}

void UIWinSize::ApplySettings()
{
    UIWidgets::sVars->sfxBleep.Play(false, 255);

    this->buttonBounceVelocity = -0x20000;
    this->buttonBounceOffset   = -0x25000;

    UIWinSize::SetupText(this);

    Graphics::SetVideoSetting(VIDEOSETTING_WINDOW_WIDTH, this->selection * screenInfo->size.x);
    Graphics::SetVideoSetting(VIDEOSETTING_WINDOW_HEIGHT, this->selection * screenInfo->size.y);

    Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, true);
}

void UIWinSize::ProcessButtonCB()
{
    int32 selection = this->selection;

    if (UIControl::sVars->anyLeftPress)
        this->selection--;

    if (UIControl::sVars->anyRightPress)
        ++this->selection;

    if (selection != this->selection)
        UIWinSize::ApplySettings();
}

bool32 UIWinSize::ProcessTouchCB()
{
    StateMachine<UIWinSize> *callbacks[2] = {};
    Vector2 touchStart[2];
    Vector2 touchEnd[2];

    callbacks[0]->Set(&UIWinSize::TouchedCB_Left);
    callbacks[1]->Set(&UIWinSize::TouchedCB_Right);

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
                int32 x = (int32)((screenInfo->position.x << 16) - ((touchInfo->x[t] * screenInfo->size.x) * -65536.0f));
                int32 y = (int32)((screenInfo->position.y << 16) - ((touchInfo->y[t] * screenInfo->size.y) * -65536.0f));

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

    return this->touchPressed;
}

void UIWinSize::TouchedCB_Left()
{
    this->selection--;

    if (this->selection != this->selection - 1)
        UIWinSize::ApplySettings();
}

void UIWinSize::TouchedCB_Right()
{
    this->selection++;

    if (this->selection != this->selection + 1)
        UIWinSize::ApplySettings();
}

void UIWinSize::SetChoiceActive(UIWinSize *entity)
{
    if (entity) {
        entity->active  = ACTIVE_BOUNDS;
        entity->visible = true;

        entity->buttonBounceOffset   = 0;
        entity->buttonBounceVelocity = -0x25000;

        entity->isSelected = true;
        entity->state.Set(&UIWinSize::State_HandleButtonEnter);
    }
}

void UIWinSize::SetChoiceInactive(UIWinSize *entity)
{
    if (entity) {
        entity->buttonBounceOffset   = 0;
        entity->buttonBounceVelocity = 0;

        entity->isSelected = false;
        entity->state.Set(&UIWinSize::State_HandleButtonLeave);
    }
}

void UIWinSize::State_HandleButtonLeave()
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

void UIWinSize::State_HandleButtonEnter()
{
    this->buttonBounceVelocity += 0x4800;
    this->buttonBounceOffset += this->buttonBounceVelocity;

    if (this->buttonBounceOffset >= -0x20000 && this->buttonBounceVelocity > 0) {
        this->buttonBounceOffset   = 0;
        this->buttonBounceVelocity = 0;
    }
}

#if RETRO_INCLUDE_EDITOR
void UIWinSize::EditorDraw()
{
    int32 sizeY = this->size.y;

    this->visible       = true;
    this->drawGroup     = 2;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x400000;
    this->selection     = 1;
    this->bgEdgeSize    = this->size.y >> 16;
    this->size.y        = abs(this->size.y);

    if (UIWidgets::sVars) {
        this->arrowAnimatorL.SetAnimation(&UIWidgets::sVars->uiFrames, 2, true, 0);
        this->arrowAnimatorR.SetAnimation(&UIWidgets::sVars->uiFrames, 2, true, 1);
        this->textAnimator.SetAnimation(&UIWidgets::sVars->fontFrames, 0, true, 0);
    }

    this->isSelected = showGizmos();
    this->inkEffect  = this->disabled ? INK_BLEND : INK_NONE;

    UIWinSize::Draw();

    this->size.y = sizeY;
}

void UIWinSize::EditorLoad()
{
    sVars->aniFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, align);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Center");
    RSDK_ENUM_VAR("Right");
}
#endif

void UIWinSize::Serialize()
{
    RSDK_EDITABLE_VAR(UIWinSize, VAR_BOOL, disabled);
    RSDK_EDITABLE_VAR(UIWinSize, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(UIWinSize, VAR_ENUM, align);
    RSDK_EDITABLE_VAR(UIWinSize, VAR_ENUM, arrowWidth);
}
} // namespace GameLogic