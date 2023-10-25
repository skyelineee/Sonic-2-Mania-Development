#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIControl;
struct UIInfoLabel;
struct Music;

struct DASetup : RSDK::GameObject::Entity {

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
        bool32 initialized;
        int32 returnToMenu;
        int32 trackCount;
        int32 trackID;
        int32 activeTrack;
        UIControl *control;
        UIInfoLabel *trackSelLabel;
        UIInfoLabel *trackTitleLabel;
        Music *trackList[64];
        RSDK::SoundFX sfxEmerald;
        RSDK::SoundFX sfxMedal;
        RSDK::SoundFX sfxSSExit;
        RSDK::SoundFX sfxScoreTotal;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<DASetup> state;
    RSDK::StateMachine<DASetup> stateDraw;
    int32 timer;

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

    void State_Idle();
    void Draw_Fade();

    static void DisplayTrack(int32 trackID);
    static void SetupUI();
    static void State_ManageControl();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(DASetup);
};
} // namespace GameLogic
