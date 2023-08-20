#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct ChopChop : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum ChopChopAnims {
        Swim,
        Attack,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxBadnik;
        RSDK::Hitbox hitboxRange;
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 timer;
    int32 bubbleTimer;
    RSDK::Vector2 startPos;
    RSDK::Animator animator;
    RSDK::StateMachine<ChopChop> state;

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

    // STATES
    void State_Swimming();
    void State_AttackDelay();
    void State_Attack();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ChopChop);
};
} // namespace GameLogic
