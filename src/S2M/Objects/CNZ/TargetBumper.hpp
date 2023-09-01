#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct TargetBumper : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum TargetBumperTypes { TARGETBUMP_HORIZONTAL, TARGETBUMP_VERTICAL, TARGETBUMP_DIAGONAL };

    enum TargetBumperHitCount { TARGETBUMP_THREE_HIT, TARGETBUMP_TWO_HIT, TARGETBUMP_ONE_HIT };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxTargetBumper;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<TargetBumper> state;
    int32 type;
    int32 hitCount;
    RSDK::Vector2 startPos;
    RSDK::Vector2 curPos;
    int32 hitTimer;
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

    void CheckPlayerCollisions();

    void State_Idle();
    void State_Hit();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TargetBumper);
};
} // namespace GameLogic
