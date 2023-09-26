#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct ArrowLauncher : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    
    enum ArrowLauncherAnims {
        Idle,
        Detected,
        Launch,
        Open,
        Shut,
        Arrow,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX firedSFX;
        RSDK::SoundFX swishSFX;
        RSDK::Hitbox idleHitbox;
        RSDK::Hitbox arrowHitbox;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 timer;
    int32 drawPosX;
    bool32 isArrow;
    RSDK::Animator animator;
    RSDK::Animator arrowAnimator;
    RSDK::StateMachine<ArrowLauncher> state;

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

    // STATES
    void State_AwaitPlayer();
    void State_Activate();
    void State_FireArrow();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ArrowLauncher);
};
} // namespace GameLogic
