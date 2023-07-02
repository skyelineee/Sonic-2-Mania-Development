// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIResPicker Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIResPicker.hpp"
#include "UIControl.hpp"
#include "UIWidgets.hpp"
#include "UIButton.hpp"
#include "Global/Localization.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIResPicker);

void UIResPicker::Update()
{
    this->touchPosSizeS.x   = this->size.x;
    this->touchPosSizeS.y   = this->size.y;
    this->touchPosOffsetS.y = 0;
    this->touchPosSizeS.x   = (this->touchPosSizeS.x + 3 * this->size.y) >> 1;
    this->touchPosSizeS.y   = this->size.y + 0x60000;
    this->touchPosOffsetS.x = -this->touchPosSizeS.x;

    if (this->selection != this->prevSelection) {
        UIResPicker::GetDisplayInfo(this);
        this->prevSelection = this->selection;
    }

    this->state.Run(this);

    UIControl *control = (UIControl *)this->parent;
    if (control && control->state.Matches(&UIButton::State_HandleButtonLeave))
        UIResPicker::SetChoiceInactive(this);

    this->visible = true;
    this->arrowAnimatorR.Process();
    this->arrowAnimatorL.Process();
}

void UIResPicker::LateUpdate() {}

void UIResPicker::StaticUpdate() {}

void UIResPicker::Draw()
{
    Vector2 drawPos;
    int32 width = (this->size.y + this->size.x) >> 16;

    drawPos = this->position;

    drawPos.x = this->buttonBounceOffset + this->position.x;
    UIWidgets::DrawParallelogram(drawPos.x, drawPos.y, width, (this->size.y >> 16), this->bgEdgeSize, 0x00, 0x00, 0x00);

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
        drawPos   = this->position;
        drawPos.x = this->buttonBounceOffset + this->position.x;

        switch (this->align) {
            case UIButton::ALIGN_LEFT: drawPos.x = -0x60000 - (this->size.x >> 1) + drawPos.x; break;

            default:
            case UIButton::ALIGN_CENTER: break;

            case UIButton::ALIGN_RIGHT: drawPos.x = drawPos.x + (this->size.x >> 1) - 0x60000; break;
        }

        int32 width = this->text.GetWidth(UIWidgets::sVars->fontFrames, 0, 0, this->text.length, 0);
        drawPos.x -= width << 15;
        this->textAnimator.DrawString(&drawPos, &this->text, 0, this->text.length, UIButton::ALIGN_LEFT, 0, nullptr, false);
    }
}

void UIResPicker::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->selection = -2;

        this->visible       = true;
        this->drawGroup     = 2;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x400000;
        this->bgEdgeSize    = this->size.y >> 16;
        this->size.y        = abs(this->size.y);

        this->textVisible = true;
        this->text.Init("", 0x100);
        this->processButtonCB.Set(&UIResPicker::ProcessButtonCB);
        this->touchCB.Set(&UIResPicker::ProcessTouchCB);

        this->arrowAnimatorR.SetAnimation(&UIWidgets::sVars->uiFrames, 0, true, 0);
        this->arrowAnimatorL.SetAnimation(&UIWidgets::sVars->uiFrames, 1, true, 0);
        this->textAnimator.SetAnimation(&UIWidgets::sVars->fontFrames, 0, true, 0);
    }
}

void UIResPicker::StageLoad() { sVars->aniFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE); }

void UIResPicker::GetDisplayInfo(UIResPicker *entity)
{
    if (SKU->platform == PLATFORM_PC || SKU->platform == PLATFORM_DEV) {
        char buffer[0x40];
        Graphics::GetDisplayInfo(&entity->selection, &entity->displayWidth, &entity->displayHeight, &entity->displayRefreshRate, buffer);

        if (strcmp(buffer, "DEFAULT") == 0)
            Localization::GetString(&entity->text, Localization::Default);
        else
            entity->text.Set(buffer);

        entity->text.SetSpriteString(UIWidgets::sVars->fontFrames, 0);
    }
}

void UIResPicker::ApplySettings()
{
    UIWidgets::sVars->sfxBleep.Play(false, 255);

    this->buttonBounceOffset   = 0;
    this->buttonBounceVelocity = -0x25000;

    UIResPicker::GetDisplayInfo(this);

    Graphics::SetVideoSetting(VIDEOSETTING_FSWIDTH, this->displayWidth);
    Graphics::SetVideoSetting(VIDEOSETTING_FSHEIGHT, this->displayHeight);
    if (!this->displayRefreshRate)
        Graphics::SetVideoSetting(VIDEOSETTING_REFRESHRATE, 60);
    else
        Graphics::SetVideoSetting(VIDEOSETTING_REFRESHRATE, this->displayRefreshRate);

    Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, true);
}

void UIResPicker::ProcessButtonCB()
{
    int32 selection = this->selection;

    if (UIControl::sVars->anyLeftPress)
        this->selection--;
    if (UIControl::sVars->anyRightPress)
        ++this->selection;

    if (selection != this->selection)
        UIResPicker::ApplySettings();
}

bool32 UIResPicker::ProcessTouchCB()
{
    StateMachine<UIResPicker> *callbacks[2];
    Vector2 touchStart[2];
    Vector2 touchEnd[2];

    callbacks[0]->Set(&UIResPicker::TouchedCB_Left);
    callbacks[1]->Set(&UIResPicker::TouchedCB_Right);

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

void UIResPicker::TouchedCB_Left()
{
    this->selection--;

    if (this->selection != this->selection - 1)
        UIResPicker::ApplySettings();
}

void UIResPicker::TouchedCB_Right()
{
    this->selection++;

    if (this->selection != this->selection + 1)
        UIResPicker::ApplySettings();
}

void UIResPicker::SetChoiceActive(UIResPicker *entity)
{
    if (entity) {
        entity->active  = ACTIVE_BOUNDS;
        entity->visible = true;

        entity->buttonBounceOffset   = 0;
        entity->buttonBounceVelocity = -0x25000;

        entity->isSelected = true;
        entity->state.Set(&UIResPicker::State_HandleButtonEnter);
    }
}

void UIResPicker::SetChoiceInactive(UIResPicker *entity)
{
    if (entity) {
        entity->buttonBounceOffset   = 0;
        entity->buttonBounceVelocity = 0;

        entity->isSelected = false;
        entity->state.Set(&UIResPicker::State_HandleButtonLeave);
    }
}

void UIResPicker::State_HandleButtonLeave()
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

void UIResPicker::State_HandleButtonEnter()
{
    this->buttonBounceVelocity += 0x4800;
    this->buttonBounceOffset += this->buttonBounceVelocity;

    if (this->buttonBounceOffset >= -0x20000 && this->buttonBounceVelocity > 0) {
        this->buttonBounceOffset   = 0;
        this->buttonBounceVelocity = 0;
    }
}

#if RETRO_INCLUDE_EDITOR
void UIResPicker::EditorDraw()
{
    int32 sizeY = this->size.y;

    this->selection     = -2;
    this->drawGroup     = 2;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x400000;
    this->bgEdgeSize    = this->size.y >> 16;
    this->size.y        = abs(this->size.y);

    if (UIWidgets::sVars) {
        this->arrowAnimatorR.SetAnimation(&UIWidgets::sVars->uiFrames, 0, true, 0);
        this->arrowAnimatorL.SetAnimation(&UIWidgets::sVars->uiFrames, 1, true, 0);
        this->textAnimator.SetAnimation(&UIWidgets::sVars->fontFrames, 0, true, 0);
    }

    this->isSelected = showGizmos();
    this->inkEffect  = this->disabled ? INK_BLEND : INK_NONE;

    UIResPicker::Draw();

    this->size.y = sizeY;
}

void UIResPicker::EditorLoad()
{
    RSDK_ACTIVE_VAR(sVars, align);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Center");
    RSDK_ENUM_VAR("Right");
}
#endif

void UIResPicker::Serialize()
{
    RSDK_EDITABLE_VAR(UIResPicker, VAR_BOOL, disabled);
    RSDK_EDITABLE_VAR(UIResPicker, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(UIResPicker, VAR_ENUM, align);
    RSDK_EDITABLE_VAR(UIResPicker, VAR_ENUM, arrowWidth);
}
} // namespace GameLogic