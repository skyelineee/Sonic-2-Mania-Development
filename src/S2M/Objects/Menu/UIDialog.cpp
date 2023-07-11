// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIDialog Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIDialog.hpp"
#include "UIWidgets.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Helpers/MathHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIDialog);

void UIDialog::Update()
{
    this->state.Run(this);

    UIDialog::HandleButtonPositions();

    if (this->closeDelay > 0) {
        if (!--this->closeDelay)
            UIDialog::HandleAutoClose();
    }
}

void UIDialog::LateUpdate() {}

void UIDialog::StaticUpdate() {}

void UIDialog::Draw()
{
    UIDialog::DrawBGShapes();

    Vector2 drawPos;
    switch (this->lineCount) {
        case 0:
        case 1:
            drawPos.x = (this->position.x + 0x100000) + 0x100000;
            drawPos.y = this->position.y - 0x200000;
            break;

        case 2:
            drawPos.x = (this->position.x + 0x100000) + 0x180000;
            drawPos.y = this->position.y - 0x280000;
            break;

        case 3:
            drawPos.x = (this->position.x + 0x100000) + 0x200000;
            drawPos.y = this->position.y - 0x300000;
            break;

        default: break;
    }

    drawPos.x += this->dialogPos.x;
    drawPos.y += this->dialogPos.y;

    int32 w     = 0;
    int32 count = this->lineCount + 1;
    if (count > 0) {
        for (int32 i = 0; i < count; ++i) {
            int32 start = !i ? 0 : this->lineLength[i - 1] + 1;
            int32 len   = i >= this->lineCount ? this->textInfo.length : this->lineLength[i];

            int32 width = this->textInfo.GetWidth(UIWidgets::sVars->fontFrames, 0, start, len, 0);
            if (width > w)
                w = width;
        }
    }

    for (int32 i = 0; i < count; ++i) {
        int32 offset = -0x8000 * w;

        int32 start = !i ? 0 : this->lineLength[i - 1] + 1;
        int32 len   = i >= this->lineCount ? this->textInfo.length : this->lineLength[i];

        this->textInfo.GetWidth(UIWidgets::sVars->fontFrames, 0, start, len, 0);

        drawPos.x += offset;
        this->animator.DrawString(&drawPos, &this->textInfo, start, len, 0, 0, nullptr, false);

        drawPos.y += 0x120000;
        drawPos.x = drawPos.x - offset - 0x120000;
    }
}

void UIDialog::Create(void *data)
{
    this->active    = ACTIVE_ALWAYS;
    this->drawGroup = 15;
    this->visible   = true;

    if (data)
        UIDialog::SetupText(this, (String *)data);

    this->buttonCount = 0;
}

void UIDialog::StageLoad()
{
    UIDialog::sVars->controlStore = nullptr;
    UIDialog::sVars->activeDialog = nullptr;
}

UIDialog *UIDialog::CreateActiveDialog(String *msg)
{
    if (UIDialog::sVars->activeDialog) {
        LogHelpers::Print("EXCEPTION: Called CreateDialog when an activeDialog already existed.");
    }
    else {
        int32 id = GameObject::Get<UIDialog>(SLOT_DIALOG)->classID;

        if (id) {
            LogHelpers::Print("Can't create UIDialog (%d), entity already exists in slot (class ID: %d)", UIDialog::sVars->classID, id);
        }
        else {
            GameObject::Reset(SLOT_DIALOG, UIDialog::sVars->classID, msg);

            UIDialog *dialog              = GameObject::Get<UIDialog>(SLOT_DIALOG);
            dialog->position.x            = (screenInfo->position.x + screenInfo->center.x) << 16;
            dialog->position.y            = (screenInfo->position.y + screenInfo->center.y) << 16;
            UIDialog::sVars->activeDialog = dialog;

            return dialog;
        }
    }

    return nullptr;
}

void UIDialog::SetupText(UIDialog *dialog, String *text)
{
    if (text) {
        dialog->lineCount = 0;
        int32 charPos     = 0;

        for (int32 i = 0; i < text->length; ++i) {
            if (text->chars[charPos] == '\n' && dialog->lineCount < 3) {
                dialog->lineLength[dialog->lineCount] = charPos;
                ++dialog->lineCount;
            }

            ++charPos;
        }

        String::Copy(&dialog->textInfo, text);
        dialog->animator.SetAnimation(UIWidgets::sVars->fontFrames, 0, true, 0);
        dialog->textInfo.SetSpriteString(UIWidgets::sVars->fontFrames, 0);
    }
}

void UIDialog::AddButton(uint8 frame, UIDialog *dialog, RSDK::Action<void> callback, bool32 closeOnSelect)
{
    int32 id = dialog->buttonCount;

    if (dialog->buttonCount < UIDIALOG_OPTION_COUNT) {
        dialog->buttonFrames[dialog->buttonCount] = frame;
        dialog->callbacks[dialog->buttonCount].Copy(&callback);
        dialog->closeOnSelect[dialog->buttonCount] = closeOnSelect;

        GameObject::Reset(SLOT_DIALOG_BUTTONS + dialog->buttonCount, UIButton::sVars->classID, nullptr);

        UIButton *button   = GameObject::Get<UIButton>(SLOT_DIALOG_BUTTONS + dialog->buttonCount);
        button->position.x = (screenInfo->position.x + screenInfo->center.x) << 16;
        button->position.y = (screenInfo->position.y + screenInfo->center.y) << 16;
        button->visibleArrow = true;
        if (Stage::CheckSceneFolder("Menu")) { // if its the menu, load these, load others if not
            button->buttonListID  = 16;
            button->buttonFrameID = 1;
            button->nameListID  = 25;
            button->nameFrameID = frame;
            button->descriptionListID  = 5;
            button->descriptionFrameID = 0;
        }
        else {
            button->buttonListID       = 7;
            button->buttonFrameID      = 0;
            button->nameListID         = 9;
            button->nameFrameID        = frame;
            button->descriptionListID  = 7;
            button->descriptionFrameID = 0;
        }

        if (frame == DIALOG_CONTINUE)
            button->size.x = 0x640000;
        else
            button->size.x = 0x320000;
        button->size.y = 0x180000;
        button->actionCB.Set(&UIDialog::ButtonActionCB);
        button->bgEdgeSize                     = 24;
        button->align                          = UIButton::ALIGN_CENTER;
        button->active                         = ACTIVE_ALWAYS;
        button->drawGroup                      = dialog->drawGroup;
        dialog->buttons[dialog->buttonCount++] = button;

        UIControl *parent = dialog->parent;
        if (parent) {
            button->parent      = (Entity *)parent;
            parent->buttons[id] = button;
            parent->buttonCount = dialog->buttonCount;
        }
    }
}

void UIDialog::Setup(UIDialog *dialog)
{
    if (dialog) {
        bool32 tookFocus = false;

        Vector2 size;
        size.x = screenInfo->size.x << 16;
        size.y = screenInfo->size.y << 16;


        for (auto control : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES)) {
            if (control->active == ACTIVE_ALWAYS) {
                tookFocus                     = true;
                control->dialogHasFocus       = true;
                UIDialog::sVars->controlStore = control;
                UIDialog::sVars->controlStateStore.Copy(&control->state);
                break;
            }
        }

        GameObject::Reset(SLOT_DIALOG_UICONTROL, UIControl::sVars->classID, &size);

        UIControl *curControl    = GameObject::Get<UIControl>(SLOT_DIALOG_UICONTROL);
        curControl->menuWasSetup = true;
        curControl->position.x   = (screenInfo->position.x + screenInfo->center.x) << 16;
        curControl->position.y   = (screenInfo->position.y + screenInfo->center.y) << 16;
        curControl->rowCount     = 1;
        curControl->columnCount  = dialog->buttonCount;
        curControl->buttonID     = 0;
        curControl->backPressCB.Set(&UIDialog::HandleAutoClose);
        curControl->selectionDisabled = true;

        dialog->parent = curControl;
        if (!tookFocus) {
            UIDialog::sVars->controlStore = nullptr;
            UIDialog::sVars->controlStateStore.Set(nullptr);
        }

        int32 i = 0;
        for (; i < UIDIALOG_OPTION_COUNT; ++i) {
            if (!dialog->buttons[i])
                break;

            dialog->buttons[i]->parent = (Entity *)curControl;
            curControl->buttons[i]     = dialog->buttons[i];
        }

        curControl->buttonCount = i;
        dialog->timer        = 0;
        dialog->state.Set(&UIDialog::State_Appear);
    }
}

void UIDialog::CloseOnSel_HandleSelection(UIDialog *dialog, RSDK::Action<void> callback)
{
    if (dialog && !dialog->state.Matches(&UIDialog::State_Close)) {
        dialog->parent->selectionDisabled = true;
        dialog->timer                     = 0;
        dialog->state.Set(&UIDialog::State_Close);
        dialog->closeCB.Copy(&callback);
    }
}

void UIDialog::DrawBGShapes()
{
    Graphics::DrawRect(((screenInfo->position.x + screenInfo->center.x) << 16) - (this->bgRectSize.x >> 1),
                       ((screenInfo->position.y + screenInfo->center.y) << 16) - (this->bgRectSize.y >> 1), this->bgRectSize.x, this->bgRectSize.y,
                       this->useAltColor ? 0x282028 : 0x000000, 0xFF, INK_NONE, false);

    UIWidgets::DrawParallelogram(this->dialogPos.x + ((screenInfo->position.x + screenInfo->center.x) << 16),
                                 this->dialogPos.y + ((screenInfo->position.y + screenInfo->center.y) << 16), 0xC8, 0x8F, 0x8F, 0x30, 0xA0, 0xF0);
}

void UIDialog::HandleButtonPositions()
{
    int32 offsets[] = { 0, 0, 0x80, 0x70 };

    int32 offset = offsets[this->buttonCount] << 16;
    int32 x      = this->position.x - 0x240000 + this->dialogPos.x - ((offset * MAX(this->buttonCount - 1, 0)) >> 1);
    int32 y      = this->position.y + 0x2C0000 + this->dialogPos.y;

    for (int32 i = 0; i < UIDIALOG_OPTION_COUNT; ++i) {
        if (!this->buttons[i])
            break;

        UIButton *button   = this->buttons[i];
        button->startPos.x = x;
        button->startPos.y = y;
        button->position.x = x;
        button->position.y = y;

        x += offset;
    }
}

void UIDialog::Close()
{
    UIControl *control = this->parent;

    if (control) {
        control->SetInactiveMenu(control);
        control->Destroy();
    }

    for (int32 i = 0; i < UIDIALOG_OPTION_COUNT; ++i) {
        if (this->buttons[i])
            this->buttons[i]->Destroy();
    }

    UIControl *storedControl = UIDialog::sVars->controlStore;
    if (storedControl) {
        UIControl::SetMenuLostFocus(UIDialog::sVars->controlStore);
        storedControl->state.Copy((StateMachine<UIControl> *)&sVars->controlStateStore);
        storedControl->dialogHasFocus = false;
    }

    sVars->controlStore = nullptr;
    sVars->controlStateStore.Set(nullptr);
    sVars->activeDialog = nullptr;

    this->closeCB.Run(this);
    this->Destroy();
}

bool32 UIDialog::HandleAutoClose()
{
    UIDialog *entity   = UIDialog::sVars->activeDialog;
    UIControl *control = entity->parent;

    for (int32 i = 0; i < control->buttonCount; ++i) {
        int32 frame = entity->buttonFrames[i];
        if (frame == DIALOG_NO || frame == DIALOG_CANCEL) {
            if (entity->closeOnSelect[i]) {
                UIDialog::CloseOnSel_HandleSelection(entity, entity->callbacks[i]);
            }
            else {
                entity->callbacks[i].Run(entity);
            }

            return true;
        }
    }

    return false;
}

void UIDialog::ButtonActionCB()
{
    UIDialog *entity = UIDialog::sVars->activeDialog;

    if (entity->parent) {
        int32 id = entity->parent->buttonID;

        if (id >= 0 && id < entity->parent->buttonCount) {
            if (entity->closeOnSelect[id]) {
                UIDialog::CloseOnSel_HandleSelection(entity, entity->callbacks[id]);
            }
            else {
                entity->callbacks[id].Run(entity);
            }
        }
    }
}

void UIDialog::State_Appear()
{
    Vector2 pos;
    pos.x = 0;
    pos.y = 0;

    if (this->timer == 1) {
        UIWidgets::sVars->sfxWoosh.Play(false, 255);
        UIControl::HandleMenuLoseFocus(this->parent);
    }

    if (this->timer >= 8) {
        if (this->timer >= 16) {
            if (this->timer >= 26) {
                this->parent->selectionDisabled = false;
                this->timer                     = 0;
                this->state.Set(&UIDialog::State_Idle);
            }
            else {
                this->bgRectSize.x = screenInfo->size.x << 16;
                this->bgRectSize.y = 0x900000;
                MathHelpers::Lerp2Sin1024(&pos, MAX(((this->timer - 16) << 8) / 10, 0), -0x400000 - (screenInfo->size.x << 16), 0, 0, 0);
                this->dialogPos = pos;

                if (this->timer - 16 == 1 && this->playEventSfx)
                    UIWidgets::sVars->sfxEvent.Play(false, 255);

                ++this->timer;
            }
        }
        else {
            this->dialogPos.x = -0x400000 - (screenInfo->size.x << 16);
            this->dialogPos.y = 0;
            MathHelpers::Lerp(&pos, MAX(((this->timer - 8) << 8) / 8, 0), screenInfo->size.x << 16, 0x10000, screenInfo->size.x << 16, 0x900000);
            this->bgRectSize = pos;

            this->timer++;
        }
    }
    else {
        this->dialogPos.x = -0x400000 - (screenInfo->size.x << 16);
        this->dialogPos.y = 0;
        MathHelpers::Lerp2Sin1024(&pos, MAX((this->timer << 8) / 8, 0), 0, 0x10000, screenInfo->size.x << 16, 0x10000);
        this->bgRectSize = pos;

        ++this->timer;
    }
}

void UIDialog::State_Idle()
{
    this->bgRectSize.x = screenInfo->size.x << 16;
    this->bgRectSize.y = 0x900000;
    this->dialogPos.x  = 0;
    this->dialogPos.y  = 0;
}

void UIDialog::State_Close()
{
    if (this->timer >= 8) {
        if (this->timer >= 16) {
            UIDialog::Close();
        }
        else {
            this->dialogPos.x = (screenInfo->size.x + 64) << 16;
            this->dialogPos.y = 0;
            MathHelpers::Lerp2Sin1024(&this->bgRectSize, MAX(((this->timer - 8) << 8) / 8, 0), screenInfo->size.x << 16, 0x900000,
                                      screenInfo->size.x << 16, 0);

            ++this->timer;
        }
    }
    else {
        this->bgRectSize.x = screenInfo->size.x << 16;
        this->bgRectSize.y = 0x900000;
        MathHelpers::LerpSin1024(&this->dialogPos, MAX((this->timer << 8) / 8, 0), 0, 0, (screenInfo->size.x + 64) << 16, 0);

        ++this->timer;
    }
}

// ==============================
// HELPERS
// ==============================

UIDialog *UIDialog::CreateDialogOk(RSDK::String *text, RSDK::Action<void> callback, bool32 closeOnSelect)
{
    UIDialog *dialog = UIDialog::CreateActiveDialog(text);

    if (dialog) {
        UIDialog::AddButton(DIALOG_OK, dialog, callback, closeOnSelect);
        UIDialog::Setup(dialog);
    }

    return dialog;
}

UIDialog *UIDialog::CreateDialogYesNo(RSDK::String *text, RSDK::Action<void> callbackYes, RSDK::Action<void> callbackNo, bool32 closeOnSelect_Yes,
                                      bool32 closeOnSelect_No)
{
    UIDialog *dialog = UIDialog::CreateActiveDialog(text);

    if (dialog) {
        UIDialog::AddButton(DIALOG_NO, dialog, callbackNo, closeOnSelect_No);
        UIDialog::AddButton(DIALOG_YES, dialog, callbackYes, closeOnSelect_Yes);
        UIDialog::Setup(dialog);
    }

    return dialog;
}

UIDialog *UIDialog::CreateDialogOkCancel(RSDK::String *text, RSDK::Action<void> callbackOk, RSDK::Action<void> callbackCancel,
                                         bool32 closeOnSelect_Ok, bool32 closeOnSelect_Cancel)
{
    UIDialog *dialog = UIDialog::CreateActiveDialog(text);

    if (dialog) {
        UIDialog::AddButton(DIALOG_OK, dialog, callbackOk, closeOnSelect_Ok);
        UIDialog::AddButton(DIALOG_CANCEL, dialog, callbackCancel, closeOnSelect_Cancel);
        UIDialog::Setup(dialog);
    }

    return dialog;
}

#if RETRO_INCLUDE_EDITOR
void UIDialog::EditorDraw() {}

void UIDialog::EditorLoad() {}
#endif

void UIDialog::Serialize() {}
} // namespace GameLogic