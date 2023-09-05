#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Redz : RSDK::GameObject::Entity {

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
        RSDK::Hitbox attackbox;
        RSDK::Hitbox hitboxFlame;
        RSDK::Hitbox hitboxRange;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxFlame;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Redz> state;
    RSDK::Vector2 startPos;
    uint8 startDir;
    uint16 timer;
    uint16 attackDelay;
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

    void State_Init();
    void State_Walk();
    void State_Turn();
    void State_PrepareAttack();
    void State_Attack();
    void Flame_Setup();
    void Flame_State();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Redz);
};
} // namespace GameLogic
