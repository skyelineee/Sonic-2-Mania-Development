#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Octus : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxProjectile;
        RSDK::Hitbox hitboxRange;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxShot;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Octus> state;
    int32 timer;
    uint8 unused1;
    RSDK::Vector2 startPos;
    uint8 startDir;
    int32 unused2;
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
    void CheckOffScreen();

    void State_Init();
    void State_CheckPlayerInRange();
    void State_JumpDelay();
    void State_Jump();
    void State_Shoot();
    void State_Fall();
    void State_Shot();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Octus);
};
} // namespace GameLogic
