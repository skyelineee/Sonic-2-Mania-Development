#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct ContinueSetup : RSDK::GameObject::Entity {

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
        RSDK::Animator animator;
        RSDK::SoundFX sfxAccept;
        uint16 countIndex[10];
        uint16 sceneIndex;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<ContinueSetup> stateDraw;
    RSDK::StateMachine<ContinueSetup> state;
    int32 fadeTimer;
    int32 timer;
    int32 secondTimer;
    int32 countTimer;
    int32 rotationX;
    int32 unused1;
    int32 numberColor;
    bool32 showContinues;
    RSDK::Matrix matTemp;
    RSDK::Matrix matTranslate;
    RSDK::Matrix matRotateX;
    RSDK::Matrix matRotateY;
    RSDK::Matrix matFinal;

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

    void Draw_Fade();

    void State_FadeIn();
    void State_HandleCountdown();
    void State_ContinueGame();
    void State_ReturnToMenu();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ContinueSetup);
};
} // namespace GameLogic