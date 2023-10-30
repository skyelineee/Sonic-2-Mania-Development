#pragma once
#include "S2M.hpp"

namespace GameLogic
{

#define DRAGONFLY_SPINE_COUNT (6)

struct Dragonfly : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxSpine;
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Dragonfly> state;
    uint8 dir;
    uint8 dist;
    uint8 speed;
    RSDK::Vector2 positions[DRAGONFLY_SPINE_COUNT];
    uint8 directions[DRAGONFLY_SPINE_COUNT];
    RSDK::Vector2 startPos;
    RSDK::Animator animator;
    RSDK::Animator wingAnimator;
    RSDK::Animator bodyAnimator;

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

    void DebugDraw();
    void DebugSpawn();

    void CheckOffScreen();
    void CheckPlayerCollisions();

    void State_Init();
    void State_Move();
    void State_Debris();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Dragonfly);
};
} // namespace GameLogic
