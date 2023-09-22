#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct FlasherMKII : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum FlasherMKIIOrientations {
        FLASHERMKII_ORIENTATION_UP,
        FLASHERMKII_ORIENTATION_DOWN,
        FLASHERMKII_ORIENTATION_RIGHT,
        FLASHERMKII_ORIENTATION_LEFT,
    };


    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxFlop;
        RSDK::SoundFX sfxZap;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<FlasherMKII> state;
    uint8 orientation;
    int32 type;
    int32 timer;
    RSDK::Vector2 startPos;
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

    void DebugSpawn();
    void DebugDraw();

    void CheckOffScreen();

    void HandlePlayerCollisions();
    void HandleHarmPlayerCollisions();

    void State_Idle();
    void State_Moving();
    void State_WeakFlash();
    void State_StrongFlash();
    void State_FinishedFlashing();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(FlasherMKII);
};
} // namespace GameLogic
