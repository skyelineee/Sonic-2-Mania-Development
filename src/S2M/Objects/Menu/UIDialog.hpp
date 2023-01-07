#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"
#include "UIButton.hpp"

#define UIDIALOG_OPTION_COUNT (3)

namespace GameLogic
{

struct UIDialog : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum DialogOptions {
        DIALOG_YES,
        DIALOG_NO,
        DIALOG_OK,
        DIALOG_CANCEL,
        DIALOG_CONTINUE,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        UIDialog *activeDialog;
        UIControl *controlStore;
        RSDK::StateMachine<UIDialog> controlStateStore;
    };

    RSDK::StateMachine<UIDialog> state;
    int32 timer;
    int32 closeDelay;
    RSDK::String textInfo;
    int32 buttonCount;
    RSDK::Vector2 bgRectSize;
    RSDK::Vector2 dialogPos;
    UIControl *parent;
    Entity *entityPtr;
    uint8 buttonFrames[UIDIALOG_OPTION_COUNT];
    RSDK::StateMachine<UIDialog> callbacks[UIDIALOG_OPTION_COUNT];
    bool32 closeOnSelect[UIDIALOG_OPTION_COUNT];
    UIButton *buttons[UIDIALOG_OPTION_COUNT];
    RSDK::StateMachine<UIDialog> closeCB;
    bool32 playEventSfx;
    bool32 useAltColor;
    int32 lineLength[3];
    int32 lineCount;
    RSDK::Animator animator;

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
    static void StageLoad();
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    static UIDialog *CreateActiveDialog(RSDK::String *msg);
    static void SetupText(UIDialog *dialog, RSDK::String *text);

    template <typename T> static inline void AddButton(uint8 frame, UIDialog *dialog, void (T::*callback)(), bool32 closeOnSelect)
    {
        int32 id = dialog->buttonCount;

        if (dialog->buttonCount < UIDIALOG_OPTION_COUNT) {
            dialog->buttonFrames[dialog->buttonCount] = frame;
            dialog->callbacks[dialog->buttonCount].Set(&callback);
            dialog->closeOnSelect[dialog->buttonCount] = closeOnSelect;

            GameObject::Reset(SLOT_DIALOG_BUTTONS + dialog->buttonCount, UIButton::sVars->classID, nullptr);

            UIButton *button   = GameObject::Get<UIButton>(SLOT_DIALOG_BUTTONS + dialog->buttonCount);
            button->position.x = (screenInfo->position.x + screenInfo->center.x) << 16;
            button->position.y = (screenInfo->position.y + screenInfo->center.y) << 16;
            button->animator.SetAnimation(&UIWidgets::sVars->textFrames, 9, true, frame);
            button->textFrames = UIWidgets::sVars->textFrames;

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

    static void Setup(UIDialog *dialog);

    template <typename T> static inline void CloseOnSel_HandleSelection(UIDialog *dialog, void (T::*callback)())
    {
        if (dialog && !dialog->state.Matches(&UIDialog::State_Close)) {
            dialog->parent->selectionDisabled = true;
            dialog->timer                     = 0;
            dialog->state.Set(&UIDialog::State_Close);
            dialog->closeCB.Set(&callback);
        }
    }

    void DrawBGShapes();
    void HandleButtonPositions();
    void Close();
    static bool32 HandleAutoClose();
    static void ButtonActionCB();

    // ==============================
    // STATES
    // ==============================

    void State_Appear();
    void State_Idle();
    void State_Close();

    // ==============================
    // HELPERS
    // ==============================

    template <typename T> static inline UIDialog *CreateDialogOk(RSDK::String *text, void (T::*callback)(), bool32 closeOnSelect)
    {
        UIDialog *dialog = UIDialog::CreateActiveDialog(text);

        if (dialog) {
            UIDialog::AddButton(DIALOG_OK, dialog, callback, closeOnSelect);
            UIDialog::Setup(dialog);
        }

        return dialog;
    }

    template <typename T> static inline UIDialog *CreateDialogYesNo(RSDK::String *text, void (T::*callbackYes)(), void (T::*callbackNo)(), bool32 closeOnSelect_Yes, bool32 closeOnSelect_No)
    {
        UIDialog *dialog = UIDialog::CreateActiveDialog(text);

        if (dialog) {
            UIDialog::AddButton(DIALOG_NO, dialog, callbackNo, closeOnSelect_No);
            UIDialog::AddButton(DIALOG_YES, dialog, callbackYes, closeOnSelect_Yes);
            UIDialog::Setup(dialog);
        }

        return dialog;
    }

    template <typename T> static inline UIDialog *CreateDialogOkCancel(RSDK::String *text, void (T::*callbackOk)(), void (T::*callbackCancel)(), bool32 closeOnSelect_Ok, bool32 closeOnSelect_Cancel)
    {
        UIDialog *dialog = UIDialog::CreateActiveDialog(text);

        if (dialog) {
            UIDialog::AddButton(DIALOG_OK, dialog, callbackOk, closeOnSelect_Ok);
            UIDialog::AddButton(DIALOG_CANCEL, dialog, callbackCancel, closeOnSelect_Cancel);
            UIDialog::Setup(dialog);
        }

        return dialog;
    }

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIDialog);
};
} // namespace GameLogic