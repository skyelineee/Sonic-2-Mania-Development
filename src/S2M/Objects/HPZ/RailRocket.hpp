#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct RailRocket : RSDK::GameObject::Entity {

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
        RSDK::Hitbox idleHandleHitbox;
        RSDK::Hitbox activeHandleHitbox;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxLaunch;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 timer;
    RSDK::Vector2 startPos;
    RSDK::Vector2 handlePos;
    int32 handleAngle;
    RSDK::Vector2 chainPos;
    int32 grabDelay;
    int32 targetPosY;
    bool32 hasPlayer;
    int32 rocketDir;
    int32 handleAngleVel;
    int32 animationTimer;
    int32 frame;
    uint8 distance;
    RSDK::StateMachine<RailRocket> state;
    RSDK::Animator animator;
    RSDK::Animator flameAnimator;

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

    void State_AwaitPlayer();
    void State_MoveToTarget();
    void State_ReachedTarget();
    void State_ReturnToStart();
    void State_Finished();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(RailRocket);
};
} // namespace GameLogic
