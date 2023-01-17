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
        RSDK::StateMachine<UIControl> controlStateStore;
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
    RSDK::Action<void> callbacks[UIDIALOG_OPTION_COUNT];
    bool32 closeOnSelect[UIDIALOG_OPTION_COUNT];
    UIButton *buttons[UIDIALOG_OPTION_COUNT];
    RSDK::Action<void> closeCB;
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

    static void AddButton(uint8 frame, UIDialog *dialog, RSDK::Action<void> callback, bool32 closeOnSelect = true);

    static void Setup(UIDialog *dialog);

    static void CloseOnSel_HandleSelection(UIDialog *dialog, RSDK::Action<void> callback);

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

    static UIDialog *CreateDialogOk(RSDK::String *text, RSDK::Action<void> callbackCallback, bool32 closeOnSelect = true);

    static UIDialog *CreateDialogYesNo(RSDK::String *text, RSDK::Action<void> callbackYes, RSDK::Action<void> callbackNo,
                                       bool32 closeOnSelect_Yes = true, bool32 closeOnSelect_No = true);

    static UIDialog *CreateDialogOkCancel(RSDK::String *text, RSDK::Action<void> callbackOk, RSDK::Action<void> callbackCancel,
                                          bool32 closeOnSelect_Ok = true, bool32 closeOnSelect_Cancel = true);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIDialog);
};
} // namespace GameLogic