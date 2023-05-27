// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIButton Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIButton.hpp"
#include "UIWidgets.hpp"
#include "UIControl.hpp"
#include "UIChoice.hpp"
#include "UIResPicker.hpp"
#include "UIWinSize.hpp"
#include "UITransition.hpp"
#include "Global/Music.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIButton);

void UIButton::Update()
{
    this->buttonAnimator.SetAnimation(&UIWidgets::sVars->buttonFrames, this->buttonListID, true, this->buttonFrameID);
    this->nameAnimator.SetAnimation(&UIWidgets::sVars->nameFrames, this->nameListID, true, this->nameFrameID);
    this->descriptionAnimator.SetAnimation(&UIWidgets::sVars->descFrames, this->descriptionListID, true, this->descriptionFrameID);
    this->touchPosSizeS.x   = this->size.x;
    this->touchPosOffsetS.x = 0;
    this->touchPosOffsetS.y = 0;
    this->touchPosSizeS.x += 3 * this->size.y;
    this->touchPosSizeS.y = this->size.y + 0x60000;

    if (!this->nameFrames.Matches(UIWidgets::sVars->nameFrames) || this->startNameListID != this->nameListID || this->startNameFrameID != this->nameFrameID
        || this->isDisabled != this->disabled) {
        if (this->disabled)
            this->nameAnimator.SetAnimation(&UIWidgets::sVars->nameFrames, 7, true, 0);
        else
            this->nameAnimator.SetAnimation(&UIWidgets::sVars->nameFrames, this->nameListID, true, this->nameFrameID);

        this->nameFrames   = UIWidgets::sVars->nameFrames;
        this->startNameListID  = this->nameListID;
        this->startNameFrameID = this->nameFrameID;
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

void UIButton::Draw()
{
    Vector2 drawPos;

    if (this->nameVisible) {
        drawPos.x = this->buttonBounceOffset + this->position.x;
        drawPos.y = this->position.y;

        switch (this->align) {
            case ALIGN_LEFT: drawPos.x += -0x60000 - (this->size.x >> 1); break;

            case ALIGN_CENTER: break;

            case ALIGN_RIGHT:
                drawPos.x -= 0x60000;
                drawPos.x += this->size.x >> 1;
                break;
        }

        if (this->disabled && this->align == ALIGN_LEFT)
            drawPos.x += 0x150000;

        this->buttonAnimator.DrawSprite(&drawPos, false);
        this->nameAnimator.DrawSprite(&drawPos, false);
    }

    this->descOffset.y = Math::Sin256(this->descAngle) << 10;
    this->descAngle    = (this->descAngle + 1) & 0xFF;

    Vector2 descDrawPos;

    descDrawPos = this->position;
    descDrawPos.y += descOffset.y;

    this->descriptionAnimator.DrawSprite(&descDrawPos, false);
}

void UIButton::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->drawGroup     = 3;
        this->visible       = !this->invisible;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x400000;
        this->bgEdgeSize    = this->size.y >> 16;
        this->size.y        = abs(this->size.y);

        this->processButtonCB.Set(&UIButton::ProcessButtonCB);
        this->touchCB.Set(&UIButton::ProcessTouchCB_Single);
        this->selectedCB.Set(&UIButton::SelectedCB);
        this->failCB.Set(&UIButton::FailCB);
        this->buttonEnterCB.Set(&UIButton::ButtonEnterCB);
        this->buttonLeaveCB.Set(&UIButton::ButtonLeaveCB);
        this->checkButtonEnterCB.Set(&UIButton::CheckButtonEnterCB);
        this->checkSelectedCB.Set(&UIButton::CheckSelectedCB);

        this->nameVisible = true;
        this->nameFrames   = UIWidgets::sVars->nameFrames;
        this->buttonFrames = UIWidgets::sVars->buttonFrames;
        this->startNameListID  = this->nameListID;
        this->startNameFrameID = this->nameFrameID;
        this->startButtonListID = this->buttonListID;
        this->startButtonFrameID = this->buttonFrameID;
        this->startDescriptionListID = this->descriptionListID;
        this->startDescriptionFrameID = this->descriptionFrameID;

        int32 slot = this->Slot() - this->choiceCount;
        for (int32 i = 0; i < this->choiceCount; ++i) {
            UIButton *item = GameObject::Get<UIButton>(slot + i);

            if ((UIChoice::sVars && item->classID == UIChoice::sVars->classID)
                || (UIResPicker::sVars && item->classID == UIResPicker::sVars->classID) || (UIWinSize::sVars && item->classID == UIWinSize::sVars->classID)) {
                item->parent = (Entity *)this;
            }

            if (i) {
                item->position.x = this->firstChoicePos.x;
                item->position.y = this->firstChoicePos.y;
                item->active     = ACTIVE_NEVER;
            }
            else {
                this->firstChoicePos.x = item->position.x;
                this->firstChoicePos.y = item->position.y;
            }
        }

        this->startPos = this->position;
    }
}

void UIButton::StageLoad() {}

void UIButton::ManageChoices(UIButton *button)
{
    for (int32 i = 0; i < button->choiceCount; ++i) {
        UIButton *choice = GameObject::Get<UIButton>(i % button->choiceCount - button->choiceCount + button->Slot());
        if (button->choiceCount > 0
            && (choice->classID == UIChoice::sVars->classID || choice->classID == UIResPicker::sVars->classID
                || choice->classID == UIWinSize::sVars->classID)) {

            choice->visible = i == button->selection;
            choice->active  = i == button->selection ? ACTIVE_NORMAL : ACTIVE_NEVER;
        }
    }
}

UIButton *UIButton::GetChoicePtr(UIButton *button, int32 selection)
{
    if (button->choiceCount <= 0)
        return nullptr;

    UIButton *choice = GameObject::Get<UIButton>(button->Slot() - button->choiceCount + (selection % button->choiceCount));
    if (choice->classID == UIChoice::sVars->classID || choice->classID == UIResPicker::sVars->classID
        || choice->classID == UIWinSize::sVars->classID) {
        return choice;
    }

    return nullptr;
}

void UIButton::SetChoiceSelectionWithCB(UIButton *button, int32 selection)
{
    if (button->choiceCount) {

        UIButton *curChoice = UIButton::GetChoicePtr(button, button->selection);
        if (curChoice) {
            if (curChoice->classID == UIChoice::sVars->classID) {
                UIChoice *choice     = (UIChoice *)curChoice;
                choice->buttonBounceOffset = 0;
                choice->isSelected         = false;
                choice->state.Set(&UIChoice::State_HandleButtonLeave);
            }
            else if (curChoice->classID == UIResPicker::sVars->classID) {
                UIResPicker::SetChoiceInactive((UIResPicker *)curChoice);
            }
            else if (curChoice->classID == UIWinSize::sVars->classID) {
                UIWinSize::SetChoiceInactive((UIWinSize *)curChoice);
            }

            curChoice->active = ACTIVE_NEVER;
        }

        button->selection = selection;

        UIChoice *newChoice = (UIChoice *)UIButton::GetChoicePtr(button, selection);
        if (newChoice) {
            if (button->state.Matches(&UIButton::State_HandleButtonEnter) || button->state.Matches(&UIButton::State_Selected)) {
                UIChoice::SetChoiceActive(newChoice);
            }
            else {
                newChoice->active  = ACTIVE_BOUNDS;
                newChoice->visible = true;
                UIChoice::SetChoiceInactive(newChoice);
            }
        }

        if (!button->choiceChangeCB.Matches(nullptr)) {
            Entity *entStore  = (Entity *)sceneInfo->entity;
            sceneInfo->entity = (Entity *)button;
#if RETRO_USE_MOD_LOADER
            button->choiceChangeCB.Run(button);
#else
            button->choiceChangeCB();
#endif
            sceneInfo->entity = entStore;
        }
    }
}

void UIButton::SetChoiceSelection(UIButton *button, int32 selection)
{
    if (button->choiceCount) {

        UIButton *choicePtr = UIButton::GetChoicePtr(button, button->selection);
        if (choicePtr) {
            if (choicePtr->classID == UIChoice::sVars->classID) {
                UIChoice *choice     = (UIChoice *)choicePtr;
                choice->buttonBounceOffset = 0;
                choice->isSelected         = false;
                choice->state.Set(&UIChoice::State_HandleButtonLeave);
            }
            else if (choicePtr->classID == UIResPicker::sVars->classID) {
                UIResPicker::SetChoiceInactive((UIResPicker *)choicePtr);
            }
            else if (choicePtr->classID == UIWinSize::sVars->classID) {
                UIWinSize::SetChoiceInactive((UIWinSize *)choicePtr);
            }

            choicePtr->active = ACTIVE_NEVER;
        }

        button->selection         = selection;
        UIButton *newChoice = UIButton::GetChoicePtr(button, selection);
        newChoice->active         = ACTIVE_NORMAL;
    }
}

Action<void> *UIButton::GetActionCB()
{
    UIButton *choice = UIButton::GetChoicePtr(this, this->selection);
    if (!choice)
        return& this->actionCB;

    if (!UIChoice::sVars || this->choiceCount <= 0 || choice->actionCB.Matches(nullptr))
        return& this->actionCB;

    return& choice->actionCB;
}

void UIButton::FailCB()
{
    UIWidgets::sVars->sfxFail.Play(false, 255);
}

void UIButton::ProcessButtonCB_Scroll()
{
    UIControl *control = (UIControl *)this->parent;

    UIControl::SetTargetPos(control, this->position.x, this->position.y);

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

        if (UIControl::sVars->anyLeftPress) {
            --colID;
            changedSelection = true;
        }

        if (UIControl::sVars->anyRightPress) {
            ++colID;
            changedSelection = true;
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
                if (!this->isSelected) {
                    if (control->buttonID == UIControl::GetButtonID(control, this) && control->state.Matches(&UIControl::ProcessInputs)
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

bool32 UIButton::ProcessTouchCB_Multi()
{
    UIControl *control = (UIControl *)this->parent;

    bool32 touched       = false;
    int32 lastTouchID    = -1;
    uint32 lastTouchDist = 0xFFFFFFFF;

    for (int32 i = 0; i < this->touchPosCount; ++i) {
        Vector2 touchPosSize   = this->touchPosSizeM[i];
        Vector2 touchPosOffset = this->touchPosOffsetM[i];

        if (touchInfo->count) {
            int32 screenX = screenInfo->position.x << 16;
            int32 screenY = screenInfo->position.y << 16;

            for (int32 t = 0; t < touchInfo->count; ++t) {
                int32 x  = abs(touchPosOffset.x + this->position.x - (screenX - (int32)((touchInfo->x[t] * screenInfo->size.x) * -65536.0f)));
                int32 y  = abs(touchPosOffset.y + this->position.y - (screenY - (int32)((touchInfo->y[t] * screenInfo->size.y) * -65536.0f)));
                int32 x1 = touchPosSize.x >> 1;
                int32 y1 = touchPosSize.y >> 1;

                if (x < x1 && y < y1) {
                    touched = true;

                    if ((uint32)((touchPosSize.x >> 16) * (touchPosSize.y >> 16)) < lastTouchDist) {
                        lastTouchDist = (touchPosSize.x >> 16) * (touchPosSize.y >> 16);
                        lastTouchID   = i;
                    }
                }
            }
        }
        else {
            if (this->touchPressed && this->touchPosID == i && !this->disabled) {
                if (!UIControl::isMoving(control)) {
                    this->touchPosCallbacks[i].Run(this);
                }
            }
        }
    }

    this->touchPosID   = lastTouchID;
    this->touchPressed = touched;

    return this->touchPressed;
}

bool32 UIButton::ProcessTouchCB_Single()
{
    UIControl *control = (UIControl *)this->parent;

    bool32 touched = false;
    if (this->classID != UIButton::sVars->classID || !this->invisible) {
        if (touchInfo->count) {
            int32 screenX = (screenInfo->position.x << 16);
            int32 screenY = (screenInfo->position.y << 16);
            int32 sizeX   = this->touchPosSizeS.x >> 1;
            int32 sizeY   = this->touchPosSizeS.y >> 1;

            for (int32 i = 0; i < touchInfo->count; ++i) {
                int32 x = (int32)(screenX - ((touchInfo->x[i] * screenInfo->size.x) * -65536.0f));
                int32 y = (int32)(screenY - ((touchInfo->y[i] * screenInfo->size.y) * -65536.0f));

                int32 touchX = abs(this->touchPosOffsetS.x + this->position.x - x);
                int32 touchY = abs(this->touchPosOffsetS.y + this->position.y - y);
                if (touchX < sizeX && touchY < sizeY)
                    touched = true;
            }
        }
        else {
            if (this->touchPressed && !UIControl::isMoving(control)) {
                if (this->disabled) {
                    this->failCB.Run(this);
                }
                else {
                    this->isSelected       = false;
                    actionCB.Set(nullptr);

                    if (this->classID == UIButton::sVars->classID)
                        actionCB.Copy(UIButton::GetActionCB());
                    else
                        actionCB = this->actionCB;

                    if (!actionCB.Matches(nullptr)) {
                        this->selectedCB.Run(this);
                    }
                }
            }
        }
    }

    if (!touched) {
        if (!this->touchPressed && !this->checkButtonEnterCB.Matches(nullptr)) {
            for (int32 i = 0; i < control->buttonCount; ++i) {
                if (this == control->buttons[i] && control->buttonID != i) {
                    this->isSelected = false;
                    this->buttonLeaveCB.Run(this);

                    break;
                }
            }
        }
    }

    bool32 childTouchFlag = false;
    this->touchPressed    = touched;
    if (this->classID == UIButton::sVars->classID && this->choiceCount > 0) {
        UIButton *entPtr = UIButton::GetChoicePtr(this, this->selection);
        if (entPtr) {
            Entity *entStore  = (Entity *)sceneInfo->entity;
            sceneInfo->entity = (Entity *)entPtr;
            if (!entPtr->touchCB.Matches(nullptr))
                childTouchFlag = !entPtr->touchCB.Matches(nullptr);
            sceneInfo->entity = entStore;
        }
    }

    return this->touchPressed || childTouchFlag;
}

void UIButton::ProcessButtonCB()
{
    UIControl *control = (UIControl *)this->parent;
    UIButton *choice   = UIButton::GetChoicePtr(this, this->selection);

    int32 columnID = 0, rowID = 0;
    if (control->rowCount && control->columnCount)
        rowID = control->buttonID / control->columnCount;

    if (control->columnCount)
        columnID = control->buttonID % control->columnCount;

    bool32 movedV = false;
    if (control->rowCount > 1) {
        if (UIControl::sVars->anyUpPress) {
            movedV = true;
            --rowID;
        }

        if (UIControl::sVars->anyDownPress) {
            movedV = true;
            rowID++;
        }
    }

    int32 selection = this->selection;
    bool32 movedH   = 0;

    if (choice && this->choiceCount == 1 && !choice->processButtonCB.Matches(nullptr) && !this->choiceDir && !this->disabled) {
        Entity *entStore  = (Entity *)sceneInfo->entity;
        sceneInfo->entity = (Entity *)choice;
#if RETRO_USE_MOD_LOADER
        choice->processButtonCB.Run(choice);
#else
        choice->processButtonCB();
#endif
        sceneInfo->entity = entStore;
    }
    else {
        if (UIControl::sVars->anyLeftPress) {
            if (this->choiceCount <= 0 || this->choiceDir || this->disabled) {
                if (control->columnCount > 1) {
                    movedV = true;
                    columnID--;
                }

                movedH = false;
            }
            else {
                if (--selection < 0) {
                    while (selection < 0) selection += this->choiceCount;
                }
                UIButton *choice = UIButton::GetChoicePtr(this, selection);

                while ((choice && choice->disabled) && selection != this->selection) {
                    if (--selection < 0) {
                        while (selection < 0) selection += this->choiceCount;
                    }

                    choice = UIButton::GetChoicePtr(this, selection);
                }

                movedH = true;
            }
        }

        if (UIControl::sVars->anyRightPress) {
            if (this->choiceCount <= 0 || this->choiceDir || this->disabled) {
                if (control->columnCount > 1) {
                    ++columnID;
                    movedV = true;
                }

                movedH = false;
            }
            else {
                selection              = (selection + 1) % this->choiceCount;
                UIButton *choice = UIButton::GetChoicePtr(this, selection);

                while ((choice && choice->disabled) && selection != this->selection) {
                    selection = (selection + 1) % this->choiceCount;

                    choice = UIButton::GetChoicePtr(this, selection);
                }

                movedH = true;
            }
        }
    }

    if (movedH) {
        if (selection < 0)
            selection += this->choiceCount;

        if (selection >= this->choiceCount)
            selection -= this->choiceCount;

        if (selection != this->selection) {
            UIButton::SetChoiceSelectionWithCB(this, selection);
            UIWidgets::sVars->sfxBleep.Play(false, 255);
        }
    }

    if (movedV) {
        if (control->noWrap) {
            int32 count = control->rowCount;
            if (rowID < control->rowCount)
                count = rowID;

            if (rowID >= control->rowCount)
                rowID = control->rowCount;
            else if (count < 0)
                rowID = 0;

            count = control->columnCount;
            if (columnID < control->columnCount)
                count = columnID;

            if (columnID >= control->columnCount)
                columnID = control->columnCount;
            else if (count < 0)
                columnID = 0;
        }
        else {
            if (rowID < 0)
                rowID += control->rowCount;

            if (rowID >= control->rowCount)
                rowID -= control->rowCount;

            if (columnID < 0)
                columnID += control->columnCount;

            if (columnID >= control->columnCount)
                columnID -= control->columnCount;
        }

        int32 id = columnID + control->columnCount * rowID;

        if (id >= control->buttonCount - 1)
            id = control->buttonCount - 1;

        if (control->buttonID != id && this != control->buttons[id]) {
            control->buttonID = id;
            UIButton::ButtonLeaveCB();
            UIWidgets::sVars->sfxBleep.Play(false, 255);
        }
    }
    else {
        actionCB.Copy(UIButton::GetActionCB());

        if (UIControl::sVars->anyConfirmPress && (!UIChoice::sVars || !actionCB.Matches(nullptr))) {
            if (this->disabled || (this->choiceCount > 0 && choice->disabled))
                UIWidgets::sVars->sfxFail.Play(false, 255);
            else
                UIButton::SelectedCB();
        }
        else {
            if (!this->state.Matches(&UIButton::State_HandleButtonEnter) && !this->state.Matches(&UIButton::State_Selected)) {
                if (control->buttonID == columnID + rowID * control->columnCount && control->state.Matches(&UIControl::ProcessInputs))
                    UIButton::ButtonEnterCB();
            }
        }
    }
}

bool32 UIButton::CheckButtonEnterCB()
{
    return this->state.Matches(&UIButton::State_HandleButtonEnter);
}

bool32 UIButton::CheckSelectedCB()
{
    return this->state.Matches(&UIButton::State_Selected);
}

void UIButton::ButtonEnterCB()
{
    if (!this->state.Matches(&UIButton::State_HandleButtonEnter)) {
        this->buttonBounceOffset   = 0;
        this->buttonBounceVelocity = -0x25000;
        this->state.Set(&UIButton::State_HandleButtonEnter);

        if (UIChoice::sVars) {
            UIButton *choicePtr = UIButton::GetChoicePtr(this, this->selection);

            if (choicePtr) {
                if (choicePtr->classID == UIChoice::sVars->classID) {
                    UIChoice::SetChoiceActive((UIChoice *)choicePtr);
                }

                if (choicePtr->classID == UIResPicker::sVars->classID) {
                    UIResPicker::SetChoiceActive((UIResPicker *)choicePtr);
                }

                if (choicePtr->classID == UIWinSize::sVars->classID) {
                    UIWinSize::SetChoiceActive((UIWinSize *)choicePtr);
                }
            }
        }
    }
}

void UIButton::ButtonLeaveCB()
{
    this->state.Set(&UIButton::State_HandleButtonLeave);

    if (UIChoice::sVars) {
        UIButton *widget = UIButton::GetChoicePtr(this, this->selection);
        if (widget) {
            if (widget->classID == UIChoice::sVars->classID) {
                ((UIChoice *)widget)->buttonBounceOffset = 0;
                widget->isSelected                             = false;
                widget->state.Set(&UIChoice::State_HandleButtonLeave);
            }
            else if (widget->classID == UIResPicker::sVars->classID) {
                UIResPicker::SetChoiceInactive((UIResPicker *)widget);
            }
            else if (widget->classID == UIWinSize::sVars->classID) {
                UIWinSize::SetChoiceInactive((UIWinSize *)widget);
            }
        }
    }
}

void UIButton::SelectedCB()
{
    UIControl *parent = (UIControl *)this->parent;
    UIButton *choice  = UIButton::GetChoicePtr(this, this->selection);

    if (this->clearParentState || (choice && choice->classID == UIChoice::sVars->classID && choice->buttonBounceVelocity))
        parent->state.Set(nullptr);

    if (this->assignsP1) {
        int32 id = Input::GetFilteredInputDeviceID(false, false, 0);
        Input::ResetInputSlotAssignments();
        Input::AssignInputSlotToDevice(Input::CONT_P1, id);
    }

    if (this->freeBindP2)
        Input::AssignInputSlotToDevice(Input::CONT_P2, Input::INPUT_AUTOASSIGN);

    parent->backoutTimer = 30;

    if (this->transition) {
        actionCB.Copy(&this->actionCB);
        if (UIChoice::sVars && this->choiceCount > 0 && choice)
            actionCB = choice->actionCB;

        UITransition::StartTransition(this->actionCB, 14);
    }

    if (this->stopMusic)
        Music::Stop();

    this->timer = 0;
    this->state.Set(&UIButton::State_Selected);
    UIWidgets::sVars->sfxAccept.Play(false, 255);
}

void UIButton::State_HandleButtonLeave()
{
    this->nameVisible = true;
    if (this->buttonBounceOffset) {
        int32 offset = -(this->buttonBounceOffset / abs(this->buttonBounceOffset));
        this->buttonBounceOffset += offset << 16;

        if (offset < 0 && this->buttonBounceOffset < 0)
            this->buttonBounceOffset = 0;
        else if (offset > 0 && this->buttonBounceOffset > 0)
            this->buttonBounceOffset = 0;
    }
}

void UIButton::State_HandleButtonEnter()
{
    this->buttonBounceVelocity += 0x4800;
    this->buttonBounceOffset += this->buttonBounceVelocity;

    if (this->buttonBounceOffset >= -0x20000 && this->buttonBounceVelocity > 0) {
        this->buttonBounceOffset   = 0;
        this->buttonBounceVelocity = 0;
    }
}

void UIButton::State_Selected()
{ 
    UIButton::State_HandleButtonEnter();

    if (++this->timer == 30) {
        this->timer = 0;

        if (!this->transition) {
            actionCB.Copy(UIButton::GetActionCB());
            actionCB.Run(&this->actionCB);
        }

        this->state.Set(&UIButton::State_HandleButtonEnter);
    }

    this->nameVisible = !((this->timer >> 1) & 1);
}

#if RETRO_INCLUDE_EDITOR
void UIButton::EditorDraw()
{
    int32 sizeY = this->size.y;

    if (this->disabled)
        this->nameAnimator.SetAnimation(&UIWidgets::sVars->nameFrames, 7, true, 0);
    else
        this->nameAnimator.SetAnimation(&UIWidgets::sVars->nameFrames, this->nameListID, true, this->nameFrameID);

    this->nameFrames   = UIWidgets::sVars->nameFrames;
    this->startNameListID  = this->nameListID;
    this->startNameFrameID = this->nameFrameID;

    this->buttonFrames       = UIWidgets::sVars->buttonFrames;
    this->startButtonListID  = this->buttonListID;
    this->startButtonFrameID = this->buttonFrameID;

    this->buttonAnimator.SetAnimation(&UIWidgets::sVars->buttonFrames, this->buttonListID, true, this->buttonFrameID);
    this->descriptionAnimator.SetAnimation(&UIWidgets::sVars->descFrames, this->descriptionListID, true, this->descriptionFrameID);

    this->drawGroup     = 3;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x400000;
    this->bgEdgeSize    = this->size.y >> 16;
    this->size.y        = abs(this->size.y);
    this->nameVisible   = true;

    this->inkEffect = this->invisible ? INK_BLEND : INK_NONE;
    UIButton::Draw();

    this->size.y = sizeY;

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        int32 slot = this->Slot() - this->choiceCount;
        for (int32 i = 0; i < this->choiceCount; ++i) {
            UIButton *item = GameObject::Get<UIButton>(slot + i);

            if (item)
                DrawHelpers::DrawArrow(this->position.x, this->position.y, item->position.x, item->position.y, i ? 0xFFFF00 : 0xE0E0E0, INK_NONE,
                                      0xFF);
        }

        RSDK_DRAWING_OVERLAY(false);
    }
}

void UIButton::EditorLoad()
{
    UIWidgets::sVars->buttonFrames.Load("UI/UIButtons.bin", SCOPE_STAGE);
    UIWidgets::sVars->nameFrames.Load("UI/UIButtons.bin", SCOPE_STAGE);
    UIWidgets::sVars->descFrames.Load("UI/DescriptionText.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, align);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Center");
    RSDK_ENUM_VAR("Right");
}
#endif

void UIButton::Serialize()
{
    RSDK_EDITABLE_VAR(UIButton, VAR_BOOL, disabled);
    RSDK_EDITABLE_VAR(UIButton, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(UIButton, VAR_ENUM, nameListID);
    RSDK_EDITABLE_VAR(UIButton, VAR_ENUM, nameFrameID);
    RSDK_EDITABLE_VAR(UIButton, VAR_ENUM, buttonListID);
    RSDK_EDITABLE_VAR(UIButton, VAR_ENUM, buttonFrameID);
    RSDK_EDITABLE_VAR(UIButton, VAR_ENUM, descriptionListID);
    RSDK_EDITABLE_VAR(UIButton, VAR_ENUM, descriptionFrameID);
    RSDK_EDITABLE_VAR(UIButton, VAR_ENUM, align);
    RSDK_EDITABLE_VAR(UIButton, VAR_ENUM, choiceCount);
    RSDK_EDITABLE_VAR(UIButton, VAR_UINT8, choiceDir);
    RSDK_EDITABLE_VAR(UIButton, VAR_BOOL, invisible);
    RSDK_EDITABLE_VAR(UIButton, VAR_BOOL, assignsP1);
    RSDK_EDITABLE_VAR(UIButton, VAR_BOOL, freeBindP2);
    RSDK_EDITABLE_VAR(UIButton, VAR_BOOL, transition);
    RSDK_EDITABLE_VAR(UIButton, VAR_BOOL, stopMusic);
}

} // namespace GameLogic