// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UICharButton Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UICharButton.hpp"
#include "UIControl.hpp"
#include "UIButton.hpp"
#include "UIWidgets.hpp"
#include "UITransition.hpp"
#include "Global/Announcer.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UICharButton);

void UICharButton::Update()
{
    this->touchPosSizeS.x   = 0x600000;
    this->touchPosSizeS.y   = 0x600000;
    this->touchPosOffsetS.x = 0;
    this->touchPosOffsetS.y = 0;

    this->state.Run(this);

    UIControl *parent = (UIControl *)this->parent;
    int32 id                = -1;
    for (int32 i = 0; i < parent->buttonCount; ++i) {
        if (this == (UICharButton *)parent->buttons[i]) {
            id = i;
            break;
        }
    }

    if (this->isSelected && (!parent->state.Matches(&UIControl::ProcessInputs) || parent->buttonID != id)) {
        this->isSelected = false;
        this->state.Set(&UICharButton::State_HandleButtonLeave);
    }
}

void UICharButton::LateUpdate() {}

void UICharButton::StaticUpdate() {}

void UICharButton::Draw() { UICharButton::DrawButton(); }

void UICharButton::Create(void *data)
{
    this->active        = ACTIVE_BOUNDS;
    this->drawGroup     = 2;
    this->visible       = true;
    this->drawFX        = FX_FLIP;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x300000;

    this->processButtonCB.Set(&UIButton::ProcessButtonCB_Scroll);
    this->touchCB.Set(&UIButton::ProcessTouchCB_Single);
    this->selectedCB.Set(&UICharButton::SelectedCB);
    this->failCB.Set(nullptr);
    this->buttonEnterCB.Set(&UICharButton::ButtonEnterCB);
    this->buttonLeaveCB.Set(&UICharButton::ButtonLeaveCB);
    this->checkButtonEnterCB.Set(&UICharButton::CheckButtonEnterCB);
    this->checkSelectedCB.Set(&UICharButton::CheckSelectedCB);
    this->state.Set(&UICharButton::State_HandleButtonLeave);
}

void UICharButton::StageLoad() {}

void UICharButton::DrawButton()
{
    Vector2 drawPos;
    
    if (this->isSelected) {
        drawPos.x = this->position.x;
        drawPos.y = this->buttonBounceOffset + this->position.y - TO_FIXED(64);
        UIWidgets::sVars->arrowDownAnimator.DrawSprite(&drawPos, false);
    }

    if (!this->state.Matches(&UICharButton::State_Selected) || !(this->timer & 2)) {
        int32 character = this->characterID;
        this->playerAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 22, true, character);

        drawPos.x = this->position.x;
        drawPos.y = this->buttonBounceOffset + this->position.y;
        this->playerAnimator.DrawSprite(&drawPos, false);
    }
}

void UICharButton::SelectedCB()
{
    this->timer           = 0;
    this->state.Set(&UICharButton::State_Selected);
    this->processButtonCB.Set(nullptr);

    UITransition::StartTransition(this->actionCB, 30);

    if (UIControl::GetUIControl())
        UIControl::GetUIControl()->selectionDisabled = true;

    ((UIControl *)this->parent)->backoutTimer = 30;

    UIWidgets::sVars->sfxAccept.Play(false, 255);
}

bool32 UICharButton::CheckButtonEnterCB()
{
    return this->state.Matches(&UICharButton::State_HandleButtonEnter);
}

bool32 UICharButton::CheckSelectedCB()
{
    return this->state.Matches(&UICharButton::State_Selected);
}

void UICharButton::ButtonEnterCB()
{
    if (!this->isSelected) {
        this->buttonBounceOffset   = 0;
        this->buttonBounceVelocity = -0x25000;

        this->isSelected = true;
        this->state.Set(&UICharButton::State_HandleButtonEnter);
    }
}

void UICharButton::ButtonLeaveCB()
{
    this->isSelected = false;
    this->state.Set(&UICharButton::State_HandleButtonLeave);
}

void UICharButton::State_HandleButtonLeave()
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

void UICharButton::State_HandleButtonEnter()
{
    this->buttonBounceOffset += this->buttonBounceVelocity;
    if (this->buttonBounceOffset >= -0x20000 && this->buttonBounceVelocity > 0) {
        this->buttonBounceOffset   = 0;
        this->buttonBounceVelocity = 0;
    }

    this->buttonBounceVelocity += 0x4800;
}

void UICharButton::State_Selected()
{
    UICharButton::State_HandleButtonEnter();

    if (this->timer >= 30) {
        this->isSelected      = false;
        this->timer           = 0;
        this->state.Set(&UICharButton::State_HandleButtonLeave);
        this->processButtonCB.Set(&UIButton::ProcessButtonCB_Scroll);
    }
    else {
        if (this->timer == 2) {
            switch (this->characterID) {
                case UICHARBUTTON_SONIC: Announcer::sVars->sfxSonic.Play(false, 255); break;
                case UICHARBUTTON_TAILS: Announcer::sVars->sfxTails.Play(false, 255); break;
                case UICHARBUTTON_KNUX: Announcer::sVars->sfxKnuckles.Play(false, 255); break;
                default: break;
            }
        }

        ++this->timer;
    }
}

#if RETRO_INCLUDE_EDITOR
void UICharButton::EditorDraw()
{
    this->inkEffect = this->disabled ? INK_BLEND : INK_NONE;

    UICharButton::Draw();
}

void UICharButton::EditorLoad()
{
    RSDK_ACTIVE_VAR(sVars, characterID);
    RSDK_ENUM_VAR("Sonic", UICHARBUTTON_SONIC);
    RSDK_ENUM_VAR("Tails", UICHARBUTTON_TAILS);
    RSDK_ENUM_VAR("Knuckles", UICHARBUTTON_KNUX);
}
#endif

void UICharButton::Serialize()
{
    RSDK_EDITABLE_VAR(UICharButton, VAR_BOOL, disabled);
    RSDK_EDITABLE_VAR(UICharButton, VAR_UINT8, characterID);
}

} // namespace GameLogic