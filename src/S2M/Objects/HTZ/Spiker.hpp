#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Spiker : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxBadnik;
        RSDK::Hitbox activateHitbox;
        RSDK::Hitbox coneHitbox;
        RSDK::SpriteAnimation aniFrames;

    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Spiker> state;
    RSDK::StateMachine<Spiker> coneState;
    RSDK::StateMachine<Spiker> backupState;
    RSDK::Animator animator;
    RSDK::Animator coneAnimator;
    int32 timer;
    int32 launchTimer;
    int32 startPosX;

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
    void Setup();

    void State_Pause();
    void State_Wandering();
    void State_Launching();
    void State_Cone();

    void Cone_None();
    void Cone_Attached();
    void Cone_Launching();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Spiker);
};
} // namespace GameLogic
