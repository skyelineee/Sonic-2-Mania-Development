#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Aquis : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxRange;
        RSDK::Hitbox hitboxProjectile;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxShot;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Aquis> state;
    int32 timer;
    uint8 playerInRange;
    int32 remainingTurns;
    RSDK::Vector2 startPos;
    uint8 startDir;
    RSDK::Animator mainAnimator;
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

    void CheckPlayerCollisions();
    void CheckOffScreen();

    void State_Init();
    void State_Idle();
    void State_Moving();
    void State_Shoot();
    void State_Turning();
    void State_Flee();

    void State_Shot();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Aquis);
};
} // namespace GameLogic
