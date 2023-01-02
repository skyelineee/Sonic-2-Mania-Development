// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIButton Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIButton.hpp"
#include "UIWidgets.hpp"
#include "UIControl.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIButton);

void UIButton::Update()
{
    this->touchPosSizeS.x   = this->size.x;
    this->touchPosOffsetS.x = 0;
    this->touchPosOffsetS.y = 0;
    this->touchPosSizeS.x += 3 * this->size.y;
    this->touchPosSizeS.y = this->size.y + 0x60000;

    if (!this->textFrames.Matches(UIWidgets::sVars->textFrames) || this->startListID != this->listID || this->startFrameID != this->frameID
        || this->isDisabled != this->disabled) {
        if (this->disabled)
            this->animator.SetAnimation(&UIWidgets::sVars->textFrames, 7, true, 0);
        else
            this->animator.SetAnimation(&UIWidgets::sVars->textFrames, this->listID, true, this->frameID);

        this->textFrames   = UIWidgets::sVars->textFrames;
        this->startListID  = this->listID;
        this->startFrameID = this->frameID;
        this->isDisabled   = this->disabled;
    }

    UIButton *choice = UIButton::GetChoicePtr(this, this->selection);
    if (choice)
        choice->visible = true;

    this->state.Run(this);

    UIControl *parent = (UIControl *)this->parent;
    if (parent && this->state.Matches(&UIButton::State_HandleButtonEnter)
        && (!parent->state.Matches(&UIControl::ProcessInputs) || parent->buttons[parent->buttonID] != this)) {
        this->isSelected = false;
        UIButton::ButtonLeaveCB();
    }
}
void UIButton::LateUpdate() {}
void UIButton::StaticUpdate() {}
void UIButton::Draw() {}

void UIButton::Create(void *data) {}

void UIButton::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void UIButton::EditorDraw() {}

void UIButton::EditorLoad() {}
#endif

void UIButton::Serialize() {}

} // namespace GameLogic