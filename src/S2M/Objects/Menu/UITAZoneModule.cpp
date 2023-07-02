// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UITAZoneModule Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UITAZoneModule.hpp"
#include "UIControl.hpp"
#include "UIButton.hpp"
#include "UIWidgets.hpp"
#include "UITransition.hpp"
#include "TimeAttackMenu.hpp"
#include "Helpers/MenuParam.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UITAZoneModule);

void UITAZoneModule::Update()
{
    UITAZoneModule::Setup();
    this->descriptionAnimator.SetAnimation(&UIWidgets::sVars->descFrames, this->descriptionListID, true, this->descriptionFrameID);

    this->active = ACTIVE_NORMAL;

    if (this->disabled != this->wasDisabled) {
        this->wasDisabled = this->disabled;
    }

    this->drawPos = this->position;

    this->state.Run(this);

    UIControl *parent = (UIControl *)this->parent;
    if (this->state.Matches(&UITAZoneModule::State_Selected)
        && (parent->buttons[parent->lastButtonID] != (UIButton *)this || !parent->state.Matches(&UIControl::ProcessInputs))) {
        this->currentlySelected = false;
        this->state.Set(&UITAZoneModule::State_NotSelected);
    }
}

void UITAZoneModule::LateUpdate() {}

void UITAZoneModule::StaticUpdate() {}

void UITAZoneModule::Draw()
{
    this->buttonZoneAnimator.frameID = this->zoneID;

    Vector2 drawPos;

    drawPos = this->position;
    drawPos.x = this->buttonBounceOffset + this->position.x;

    if (this->nameVisible) {
        MenuParam *param = MenuParam::GetMenuParam();

        this->buttonAnimator.frameID = param->characterID - 1;
        this->buttonZoneAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, param->characterID + 1, true, this->buttonZoneAnimator.frameID);
        if (!this->isSelected) {
            this->buttonAnimator.frameID = 3;
            this->buttonZoneAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, 5, true, this->buttonZoneAnimator.frameID);
        }

        this->buttonAnimator.DrawSprite(&drawPos, false);

        drawPos.x = this->buttonBounceOffset + this->position.x + TO_FIXED(68);
        drawPos.y = this->position.y + TO_FIXED(18);
        if (!this->isSelected) {
            drawPos.x = this->buttonBounceOffset + this->position.x + TO_FIXED(16);
            drawPos.y = this->position.y + TO_FIXED(7);
        }

        this->buttonZoneAnimator.DrawSprite(&drawPos, false);
    }

    this->descOffset.y = Math::Sin256(this->descAngle) << 10;
    this->descAngle    = (this->descAngle + 1) & 0xFF;

    Vector2 descDrawPos;

    descDrawPos.x = TO_FIXED(4);
    descDrawPos.y += TO_FIXED(2) + descOffset.y;

    UIControl *control = TimeAttackMenu::sVars->taZoneSelControl;

    if (control && control->active) {
        this->descriptionAnimator.DrawSprite(&descDrawPos, true);
    }
}

void UITAZoneModule::Create(void *data)
{
    this->startPos.x    = this->position.x;
    this->startPos.y    = this->position.y;
    this->active        = ACTIVE_BOUNDS;
    this->drawGroup     = 6;
    this->visible       = true;
    this->drawFX        = FX_FLIP;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x300000;

    this->buttonAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, 1, true, 0);
    this->buttonZoneAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, 5, true, 0);

    this->startDescriptionListID  = this->descriptionListID;
    this->startDescriptionFrameID = this->descriptionFrameID;

    this->processButtonCB.Set(&UITAZoneModule::ProcessButtonCB);
    this->state.Set(&UITAZoneModule::State_Init);
}

void UITAZoneModule::StageLoad()
{
    sVars->aniFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE);
}

void UITAZoneModule::Setup()
{
    this->touchPosOffsetS.x = 0;
    this->touchPosOffsetS.y = 0;

    this->checkButtonEnterCB.Set(&UITAZoneModule::CheckButtonEnterCB);
    this->checkSelectedCB.Set(&UITAZoneModule::CheckSelectedCB);

    if (this->processButtonCB.Matches(&UITAZoneModule::ProcessButtonCB)) {
        this->touchPosSizeS.x = 0x1380000;
        this->touchPosSizeS.y = 0x4E0000;

        this->touchCB.Set(&UIButton::ProcessTouchCB_Single);
        this->selectedCB.Set(&UITAZoneModule::SelectedCB);
        this->failCB.Set(&UITAZoneModule::FailCB);
        this->buttonEnterCB.Set(&UITAZoneModule::ButtonEnterCB);
        this->buttonLeaveCB.Set(&UITAZoneModule::ButtonLeaveCB);
    }
    else {
        this->touchPosSizeS.x = 0;
        this->touchPosSizeS.y = 0;
        this->touchPosCount   = 0;

        this->selectedCB.Set(nullptr);
        this->failCB.Set(nullptr);
        this->buttonEnterCB.Set(nullptr);
        this->buttonLeaveCB.Set(nullptr);
    }
}

void UITAZoneModule::ProcessButtonCB()
{
    UIControl *control = (UIControl *)this->parent;

    UIControl::SetTargetPos(control, this->position.x, this->position.y + TO_FIXED(48));

    switch (zoneID) {
        case 0: ;
        case 1: UIControl::SetTargetPos(control, this->position.x, this->position.y + TO_FIXED(84)); break;
    }

    if (!UIControl::isMoving(control)) {
        int32 rowID = 0;
        int32 colID = 0;

        if (control->rowCount && control->columnCount)
            rowID = control->buttonID / control->columnCount;

        if (control->columnCount)
            colID = control->buttonID % control->columnCount;

        bool32 changedSelection = false;
        if (control->rowCount > 1) {
            if (UIControl::sVars->anyUpPress) {
                --rowID;
                changedSelection = true;
            }

            if (UIControl::sVars->anyDownPress) {
                ++rowID;
                changedSelection = true;
            }
        }

        if (changedSelection) {
            if (control->noWrap) {
                int32 rowCount = control->rowCount;
                int32 colCount = control->columnCount;

                if (rowID < control->rowCount)
                    rowCount = rowID;

                if (rowCount >= 0) {
                    if (rowID >= control->rowCount)
                        rowID = control->rowCount;
                }
                else {
                    rowID = 0;
                }

                if (colID < control->columnCount)
                    colCount = colID;

                if (colCount >= 0) {
                    if (colID >= control->columnCount)
                        colID = control->columnCount;
                }
                else {
                    colID = 0;
                }
            }
            else {
                if (rowID < 0)
                    rowID += control->rowCount;

                if (rowID >= control->rowCount)
                    rowID -= control->rowCount;

                if (colID < 0)
                    colID += control->columnCount;

                if (colID >= control->columnCount)
                    colID -= control->columnCount;
            }

            int32 id = control->buttonCount - 1;
            if (colID + rowID * control->columnCount < id)
                id = colID + rowID * control->columnCount;

            if (control->buttonID != id) {
                control->buttonID = id;
                this->buttonLeaveCB.Run(this);
                UIWidgets::sVars->sfxBleep.Play(false, 0xFF);
            }
        }
        else {
            bool32 hasNoAction = true;
            if (UIControl::sVars->anyConfirmPress) {
                if (this->disabled) {
                    this->failCB.Run(this);
                }
                else {
                    hasNoAction = this->actionCB.Matches(nullptr);
                }
            }

            if (hasNoAction) {
                if (!this->currentlySelected) {
                    UIButton *button = (UIButton *)this;
                    if (control->buttonID == UIControl::GetButtonID(control, button) && control->state.Matches(&UIControl::ProcessInputs)
                        && !control->dialogHasFocus) {
                        this->buttonEnterCB.Run(this);
                    }
                }
            }
            else {
                this->selectedCB.Run(this);
            }
        }
    }
}

void UITAZoneModule::FailCB() { UIWidgets::sVars->sfxFail.Play(false, 255); }

void UITAZoneModule::SelectedCB()
{
    this->timer = 0;
    this->state.Set(&UITAZoneModule::State_HasBeenSelected);

    UIWidgets::sVars->sfxAccept.Play(false, 255);

    Palette *palette = (Palette *)this;
    palette->Copy(((this->zoneID % 12) >> 3) + 1, (32 * (this->zoneID % 12)), 224, 32);
}

bool32 UITAZoneModule::CheckButtonEnterCB()
{
    return this->state.Matches(&UITAZoneModule::State_Selected);
}

bool32 UITAZoneModule::CheckSelectedCB()
{
    return this->state.Matches(&UITAZoneModule::State_HasBeenSelected);
}

void UITAZoneModule::ButtonEnterCB()
{
    this->currentlySelected = true;
    this->buttonBounceOffset   = 0;
    this->buttonBounceVelocity = -0x25000;
    this->state.Set(&UITAZoneModule::State_Selected);

    Palette *palette = (Palette *)this;
    palette->Copy(((this->zoneID % 12) >> 3) + 1, (32 * (this->zoneID % 12)), 224, 32);
}

void UITAZoneModule::ButtonLeaveCB()
{
    this->currentlySelected = false;
    this->state.Set(&UITAZoneModule::State_NotSelected);
}

void UITAZoneModule::State_Init()
{
    this->state.Set(&UITAZoneModule::State_NotSelected);
    UITAZoneModule::State_NotSelected();
}

void UITAZoneModule::State_NotSelected()
{
    this->position.x = this->startPos.x;

    this->nameVisible = true;
    if (this->buttonBounceOffset) {
        int32 offset = -(this->buttonBounceOffset / abs(this->buttonBounceOffset));
        this->buttonBounceOffset += offset << 16;

        if (offset < 0 && this->buttonBounceOffset < 0)
            this->buttonBounceOffset = 0;
        else if (offset > 0 && this->buttonBounceOffset > 0)
            this->buttonBounceOffset = 0;
    }

    this->touchCB.Set(&UIButton::ProcessTouchCB_Single);
}

void UITAZoneModule::State_Selected()
{
    if (this->reverseVelocity == true) {
        this->buttonBounceOffset -= this->buttonBounceVelocity;
        if (this->buttonBounceOffset <= 0x20000 && this->buttonBounceVelocity > 0) {
            this->buttonBounceOffset   = 0;
            this->buttonBounceVelocity = 0;
        }
    }
    else {
        this->buttonBounceOffset += this->buttonBounceVelocity;
        if (this->buttonBounceOffset >= -0x20000 && this->buttonBounceVelocity > 0) {
            this->buttonBounceOffset   = 0;
            this->buttonBounceVelocity = 0;
        }
    }

    this->buttonBounceVelocity += 0x4800;
}

void UITAZoneModule::State_HasBeenSelected()
{
    UITAZoneModule::State_Selected();

    this->currentlySelected = true;

    if (this->timer >= 30) {
        this->timer = 0;
        this->actionCB.Run(this);
        this->state.Set(&UITAZoneModule::State_Selected);
    }
    else {
        this->timer++;
    }

    this->nameVisible = !((this->timer >> 1) & 1);
}

#if RETRO_INCLUDE_EDITOR
void UITAZoneModule::EditorDraw()
{
    this->startPos = this->position;
    this->drawPos  = this->position;

    this->inkEffect = this->disabled ? INK_BLEND : INK_NONE;

    UITAZoneModule::Draw();
}

void UITAZoneModule::EditorLoad() 
{
    sVars->aniFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE);
}
#endif

void UITAZoneModule::Serialize()
{
    RSDK_EDITABLE_VAR(UITAZoneModule, VAR_BOOL, disabled);
    RSDK_EDITABLE_VAR(UITAZoneModule, VAR_UINT8, zoneID);
    RSDK_EDITABLE_VAR(UITAZoneModule, VAR_BOOL, reverseVelocity);
    RSDK_EDITABLE_VAR(UITAZoneModule, VAR_ENUM, descriptionListID);
    RSDK_EDITABLE_VAR(UITAZoneModule, VAR_ENUM, descriptionFrameID);
}


} // namespace GameLogic