#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Player;

struct Wisp : RSDK::GameObject::Entity {

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
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Wisp> state;
    int32 timer;
    int32 buzzCount;
    Player *target;
    RSDK::Vector2 startPos;
    RSDK::Animator bodyAnimator;
    RSDK::Animator wingAnimator;

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

    void HandlePlayerInteractions();
    void CheckOffScreen();

    void State_Init();
    void State_Idle();
    void State_FlyTowardTarget();
    void State_FlyAway();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Wisp);
};
} // namespace GameLogic
