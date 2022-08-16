#pragma once
#include "S2M.hpp"

namespace GameLogic
{

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
        bool32 disablePause;
        uint8 pauseDisabled;
        uint8 activeChannels[CHANNEL_COUNT];
        uint16 tintLookupTable[0x10000];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<PauseMenu> state;
    RSDK::StateMachine<PauseMenu> stateDraw;
    bool32 isPaused;
    uint8 triggerPlayer;
    bool32 disableRestart;

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
    static void StageLoad();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    void State_Init();
    void State_Idle();

    static void PauseSound();
    static void ResumeSound();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(PauseMenu);
};
} // namespace GameLogic