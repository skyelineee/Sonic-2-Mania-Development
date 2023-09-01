#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct CircleBumper : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum CircleBumperTypes {
        CIRCLEBUMPER_FIXED,
        CIRCLEBUMPER_LINEAR,
        CIRCLEBUMPER_CIRCLE,
        CIRCLEBUMPER_TRACK,
        CIRCLEBUMPER_PATH,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxBumper;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxBumper;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<CircleBumper> stateCollide;
    RSDK::StateMachine<CircleBumper> stateMove;
    int32 hitCount;
    int32 type;
    RSDK::Vector2 amplitude;
    int32 speed;
    RSDK::Vector2 drawPos;
    RSDK::Animator animator;
    int32 unused;

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

    void CheckPlayerCollisions();

    void Collide_Normal();
    void Collide_Bumped();

    void Move_Fixed();
    void Move_Linear();
    void Move_Circular();
    void Move_Path();
    void Move_Track();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(CircleBumper);
};
} // namespace GameLogic
