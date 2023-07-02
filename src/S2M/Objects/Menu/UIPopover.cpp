// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIPopover Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "UIPopover.hpp"
#include "UIWidgets.hpp"
#include "Helpers/LogHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIPopover);

void UIPopover::Update()
{
    this->state.Run(this);

    UIPopover::SetupButtonPositions();

    if (this->closeDelay > 0) {
        this->closeDelay--;

        if (!this->closeDelay)
            UIPopover::BackPressCB();
    }
}

void UIPopover::LateUpdate() {}

void UIPopover::StaticUpdate() {}

void UIPopover::Draw() { UIPopover::DrawSprites(); }

void UIPopover::Create(void *data)
{
    this->active      = ACTIVE_ALWAYS;
    this->drawGroup   = 14;
    this->visible     = true;
    this->buttonCount = 0;
}

void UIPopover::StageLoad()
{
    sVars->storedControl = nullptr;
    sVars->activePopover = nullptr;
}

UIPopover *UIPopover::CreatePopover()
{
    if (sVars->activePopover) {
        LogHelpers::Print("EXCEPTION: Called CreatePopover when an activePopover already existed.");
        return nullptr;
    }
    else {
        if (GameObject::Get<UIPopover>(SLOT_POPOVER)->classID) {
            LogHelpers::Print("Can't create UIPopover (%d), entity already exists in slot (class ID: %d)", sVars->classID,
                             GameObject::Get<UIPopover>(SLOT_POPOVER)->classID);
            return nullptr;
        }
        else {
            GameObject::Reset(SLOT_POPOVER, sVars->classID, nullptr);

            UIPopover *popover       = GameObject::Get<UIPopover>(SLOT_POPOVER);
            popover->position.x      = (screenInfo->position.x + screenInfo->center.x) << 16;
            popover->position.y      = (screenInfo->position.y + screenInfo->center.y) << 16;
            sVars->activePopover     = popover;

            return popover;
        }
    }
}

void UIPopover::AddButton(UIPopover *popover, uint8 frameID, void (*callback)(), bool32 closeOnSelect)
{
    int32 id = popover->buttonCount;

    if (id < UIPOPOVER_OPTION_COUNT) {
        popover->frameIDs[id]      = frameID;
        popover->callbacks[id].Set(callback);
        popover->closeOnSelect[id] = closeOnSelect;

        int32 slot = popover->buttonCount + SLOT_POPOVER_BUTTONS;
        GameObject::Reset(slot, UIButton::sVars->classID, 0);
        UIButton *button = GameObject::Get<UIButton>(slot);

        button->position.x = (screenInfo->position.x + screenInfo->center.x) << 16;
        button->position.y = (screenInfo->position.y + screenInfo->center.y) << 16;
        button->buttonListID = 18;
        button->buttonFrameID = 0;
        button->nameListID = 19;
        button->nameFrameID = frameID;
        button->descriptionListID = 5;
        button->descriptionFrameID = 0;

        button->actionCB.Set(&UIPopover::ButtonActionCB);
        button->size.x     = 0;
        button->size.y     = 0;
        button->bgEdgeSize = 16;
        button->align      = UIButton::ALIGN_CENTER;
        button->active     = ACTIVE_ALWAYS;
        button->drawGroup  = popover->drawGroup;

        popover->buttons[id] = button;
        ++popover->buttonCount;

        UIControl *parent = popover->parent;
        if (parent) {
            button->parent      = (Entity *)parent;
            parent->buttons[id] = button;
            parent->buttonCount = popover->buttonCount;
        }
    }
}

void UIPopover::Setup(UIPopover *popover, int32 x, int32 y)
{
    if (popover) {
        Vector2 size;
        size.x = screenInfo->size.x << 16;
        size.y = screenInfo->size.y << 16;

        bool32 tookFocus = false;
        for (auto controlPtr : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES))
        {
            if (controlPtr->active == ACTIVE_ALWAYS) {
                tookFocus                     = true;
                controlPtr->popoverHasFocus   = true;
                sVars->storedControl      = controlPtr;
                sVars->storedControlState = controlPtr->state;
                break;
            }
        }

        GameObject::Reset(SLOT_POPOVER_UICONTROL, UIControl::sVars->classID, &size);

        UIControl *control         = GameObject::Get<UIControl>(SLOT_POPOVER_UICONTROL);
        control->menuWasSetup      = true;
        control->position.x        = (screenInfo->position.x + screenInfo->center.x) << 16;
        control->position.y        = (screenInfo->position.y + screenInfo->center.y) << 16;
        control->rowCount          = popover->buttonCount;
        control->columnCount       = 1;
        control->buttonID          = 0;
        control->backPressCB.Set(&UIPopover::BackPressCB);
        control->selectionDisabled = true;

        popover->parent = control;
        if (!tookFocus) {
            sVars->storedControl      = nullptr;
            sVars->storedControlState.Set(nullptr);
        }

        int32 b = 0;
        for (; b < UIPOPOVER_OPTION_COUNT; ++b) {
            if (!popover->buttons[b])
                break;

            UIButton *button = popover->buttons[b];
            control->buttons[b]    = button;
            button->parent         = (Entity *)control;
            if (popover->buttonCount == 3) {
                control->buttons[0]->buttonFrameID = 0;
                control->buttons[1]->buttonFrameID = 1;
                control->buttons[2]->buttonFrameID = 2;
            }
            if (popover->buttonCount == 2) {
                control->buttons[0]->buttonFrameID = 3;
                control->buttons[1]->buttonFrameID = 4;
            }
        }

        control->buttonCount = b;
        int32 sizeY          = (0x180000 * b) + 0x200000;
        popover->position.x  = x;
        popover->position.y  = y;
        popover->position.y += -0x80000 - (sizeY >> 1);
        popover->timer        = 0;
        popover->state.Set(&UIPopover::State_Appear);
    }
}

void UIPopover::DrawSprites()
{
    this->bgAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, 11, true, this->popoverBGFrame);
    this->bgAnimator.DrawSprite(nullptr, false);

    for (int32 b = 0; b < UIPOPOVER_OPTION_COUNT; ++b) {
        if (!this->buttons[b])
            break;

        UIButton *button = this->buttons[b];
        if (button->isSelected) {
            Vector2 drawPos;
            if (this->buttonCount == 3) {
                drawPos.x = button->position.x + TO_FIXED(118);
                drawPos.y = button->position.y + TO_FIXED(17);
                UIWidgets::sVars->arrowLeftAnimator.DrawSprite(&drawPos, false);
            }
            if (this->buttonCount == 2) {
                drawPos.x = button->position.x + TO_FIXED(118);
                drawPos.y = button->position.y + TO_FIXED(26);
                UIWidgets::sVars->arrowLeftAnimator.DrawSprite(&drawPos, false);
            }
        }
    }
}

void UIPopover::SetupButtonPositions()
{
    int32 posX = this->position.x + TO_FIXED(31);
    int32 posY = this->position.y + TO_FIXED(1);

    for (int32 b = 0; b < UIPOPOVER_OPTION_COUNT; ++b) {
        if (!this->buttons[b])
            break;

        UIButton *button = this->buttons[b];
        button->position.x     = posX;
        button->startPos.x     = posX;

        if (this->buttonCount == 3) {
            this->popoverBGFrame         = 0;
            this->buttons[0]->position.y = posY;
            this->buttons[0]->startPos.y = posY;
            this->buttons[1]->position.y = posY + TO_FIXED(34);
            this->buttons[1]->startPos.y = posY + TO_FIXED(34);
            this->buttons[2]->position.y = posY + TO_FIXED(68);
            this->buttons[2]->startPos.y = posY + TO_FIXED(68);
        }
        else if (this->buttonCount == 2) {
            this->popoverBGFrame         = 1;
            this->buttons[0]->position.y = posY;
            this->buttons[0]->startPos.y = posY;
            this->buttons[1]->position.y = posY + TO_FIXED(51);
            this->buttons[1]->startPos.y = posY + TO_FIXED(51);
        }
    }
}

void UIPopover::Close()
{
    UIControl *parent = (UIControl *)this->parent;

    if (parent) {
        parent->SetInactiveMenu(parent);
        parent->Destroy();
    }

    for (int32 i = UIPOPOVER_OPTION_COUNT - 1; i > 0; --i) {
        if (this->buttons[i]) {
            this->buttons[i]->Destroy();
            this->buttons[0]->Destroy();
        }
    }

    UIControl *control = sVars->storedControl;
    if (control) {
        UIControl::SetMenuLostFocus(sVars->storedControl);
        control->state           = sVars->storedControlState;
        control->popoverHasFocus = false;
    }

    sVars->storedControl      = nullptr;
    sVars->storedControlState.Set(nullptr);

    this->closeCB.Run(this);

    sVars->activePopover = nullptr;
    this->Destroy();
}

bool32 UIPopover::BackPressCB()
{
    UIPopover *popover = sVars->activePopover;

    if (popover && !popover->state.Matches(&UIPopover::State_Close)) {
        popover->parent->selectionDisabled = true;
        popover->timer                     = 0;
        popover->state.Set(&UIPopover::State_Close);
        popover->closeCB.Set(nullptr);
    }

    return true;
}

void UIPopover::ButtonActionCB()
{
    UIPopover *popover = sVars->activePopover;
    UIControl *control = (UIControl *)popover->parent;

    if (control && control->buttonID >= 0 && control->buttonID < control->buttonCount) {
        if (popover->closeOnSelect[control->buttonID]) {
            if (popover && !popover->state.Matches(&UIPopover::State_Close)) {
                control                    = popover->parent;
                control->selectionDisabled = true;
                popover->timer             = 0;
                popover->state.Set(&UIPopover::State_Close);
                popover->closeCB           = popover->callbacks[control->buttonID];
            }
        }
        else {
            popover->callbacks[control->buttonID].Run(popover);
        }
    }
}

void UIPopover::State_Appear()
{
    if (this->timer == 1) {
        UIWidgets::sVars->sfxWoosh.Play(false, 255);

        UIControl *control = (UIControl *)this->parent;
        UIControl::HandleMenuLoseFocus(control);
        control->selectionDisabled = false;

        this->timer = 0;
        this->state.Set(&UIPopover::State_Idle);
    }
    else {
        this->timer++;
    }
}

void UIPopover::State_Idle() {}

void UIPopover::State_Close() { UIPopover::Close(); }

#if RETRO_INCLUDE_EDITOR
void UIPopover::EditorDraw() { UIPopover::DrawSprites(); }

void UIPopover::EditorLoad() {}
#endif

void UIPopover::Serialize() {}

} // namespace GameLogic