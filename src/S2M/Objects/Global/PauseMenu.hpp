#pragma once
#include "S2M.hpp"
#include "Menu/UIControl.hpp"

namespace GameLogic
{

#define PAUSEMENU_BUTTON_COUNT (3)

struct PauseMenu : RSDK::GameObject::Entity {

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
        RSDK::SoundFX sfxBleep;
        RSDK::SoundFX sfxAccept;
        bool32 disableEvents;
        bool32 controllerDisconnect;
        bool32 forcedDisconnect;
        bool32 signOutDetected;
        bool32 activeChannels[0x10];
        uint16 tintLookupTable[0x10000];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<PauseMenu>state;
    RSDK::StateMachine<PauseMenu> stateDraw;
    int32 timer;
    int32 pauseTimer;
    int32 triangleSpeed;
    int32 centerSpeed;
    int32 tintAlpha;
    UIControl *manager;
    uint8 triggerPlayer;
    bool32 disableRestart;
    int32 buttonCount;
    uint8 buttonIDs[PAUSEMENU_BUTTON_COUNT];
    RSDK::Action<void> buttonActions[PAUSEMENU_BUTTON_COUNT];
    UIButton *buttonPtrs[PAUSEMENU_BUTTON_COUNT];
    int32 paused;
    int32 fadeTimer;
    RSDK::Action<bool32> disconnectCheck;
    int32 forcePaused;
    RSDK::Vector2 triangleLeftPos;
    RSDK::Vector2 triangleRightPos;
    RSDK::Vector2 bgCirclePos;
    RSDK::Vector2 pauseTextPos;
    RSDK::Vector2 emeraldsPos;
    RSDK::Vector2 buttonPos;
    RSDK::Action<void> fadeoutCB;
    RSDK::Animator bgCircleAnimator;
    RSDK::Animator pauseTextAnimator;
    RSDK::Animator emeraldsAnimator;
    RSDK::Animator trianglesLeftAnimator;
    RSDK::Animator trianglesRightAnimator;


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

    void SetupMenu();
    static void SetupTintTable();

    void AddButton(uint8 id, RSDK::Action<void> action);
    static void ClearButtons(PauseMenu *entity);
    void HandleButtonPositions();

    static void PauseSound();
    static void ResumeSound();
    static void StopSound();

    void FocusCamera();
    static void UpdateCameras();

    static void CheckAndReassignControllers();
    bool32 IsDisconnected();

    static uint8 GetPlayerCount();

    // Callbacks
    static void ResumeButtonCB();
    static void RestartButtonCB();
    static void ExitButtonCB();

    static void RestartDialog_YesCB();
    static void ExitDialog_YesCB();
    static void RestartFadeCB();
    void ExitFadeCB();

    void ActionCB_Button();

    // States
    void State_SetupButtons();

    void State_StartPause();

    void State_Paused();
    void State_ForcedPause();

    void State_Resume();

    static void State_SetupTitleFade();
    void State_FadeToTitle();
    void State_HandleFadeout();

    // Draw States
    void DrawStartPause();
    void DrawPauseMenu();
    void DrawEndPause();
    void Draw_RegularPause();

    void Draw_ForcePause();


    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(PauseMenu);
};
} // namespace GameLogic