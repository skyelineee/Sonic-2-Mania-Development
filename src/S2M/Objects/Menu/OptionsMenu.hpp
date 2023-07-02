#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"
#include "UIButtonPrompt.hpp"
#include "UIDiorama.hpp"

namespace GameLogic
{

struct OptionsMenu : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        UIControl *optionsControl;
        UIControl *videoControl_Windows;
        UIControl *soundControl;
        UIControl *dataOptionsControl;
        UIControl *controlsControl_Windows;
        UIControl *controlsControl_KB;
        UIControl *controlsControl_PS4;
        UIControl *controlsControl_XB1;
        UIControl *controlsControl_NX;
        UIControl *controlsControl_NXGrip;
        UIControl *controlsControl_NXJoycon;
        UIControl *controlsControl_NXPro;
        UIDiorama *diorama;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

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

    static void Initialize();
    static void HandleUnlocks();
    static void SetupActions();
    static void HandleMenuReturn();
    static void InitVideoOptionsMenu();
    static void VideoControl_Win_MenuUpdateCB();
    static void VideoControl_Win_YPressCB();
    static void DlgRunnerCB_RevertVideoChanges();
    static bool32 VideoControl_Win_BackPressCB();
    static void ApplyChangesDlg_BackPress_YesCB();
    static void ApplyChangesDlg_Win_NoCB();
    static void ApplyChangesDlg_NoCB();
    static void ApplyChangesDlg_Win_YesCB();
    static void ApplyChangesDlg_Win_BackPress_YesCB();
    static void ApplyChangesDlg_BackPress_NoCB();
    static void VideoMenuButton_ActionCB();
    static void SoundMenuButton_ActionCB();
    static void DataOptionsMenuButton_ActionCB();
    static void ControlsMenuButton_DefaultKB_ActionCB();
    static void ControlsMenuButton_Default_ActionCB();
    static void ControlsMenuButton_ActionCB();
    static void SetDefaultMappings();
    static void SetupKBControlsMenu(int32 playerID);
    void KeyboardIDButton_Win_ActionCB();
    static void MenuSetupCB();
    static void SaveOptionsCB_Load(bool32 success);
    static void TransitionCB_ReloadScene();
    static void SaveOptionsCB_Action(bool32 success);
    void ShaderButton_ActionCB();
    void WindowScaleButton_ActionCB();
    void BorderlessButton_ActionCB();
    void FullScreenButton_ActionCB();
    void VSyncButton_ActionCB();
    void TripleBufferButton_ActionCB();
    void UISlider_ChangedCB();
    static void EraseSaveDataCB(bool32 success);
    static void AreYouSureDlg_YesCB_EraseSaveGame();
    static void AreYouSureDlg_YesCB_EraseAllData();
    static void EraseSaveGameButton_ActionCB();
    static void AreYouSureDlg_YesCB_EraseTimeAttack();
    static void EraseTimeAttackButton_ActionCB();
    static void AreYouSureDlg_YesCB_EraseReplays();
    static void EraseReplaysCB(bool32 success);
    static void EraseReplaysButton_ActionCB();
    static void EraseAllButton_ActionCB();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(OptionsMenu);
};
} // namespace GameLogic