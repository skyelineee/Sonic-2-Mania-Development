#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct BreakoffPillar : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitbox;
        RSDK::Hitbox fallenHitbox;
        RSDK::Hitbox idleHitbox;
        RSDK::SpriteAnimation aniFrames;
        int32 shakePosTable[8];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 offsetY;
    int32 shakePosX;
    int32 timer;
    int32 bottomFrame;
    RSDK::Animator animator;
    RSDK::StateMachine<BreakoffPillar> state;

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

    void DebugSpawn();
    void DebugDraw();

    // STATES
    void State_AwaitPlayer();
    void State_Shaking();
    void State_Falling();
    void State_Fallen();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(BreakoffPillar);
};
} // namespace GameLogic
