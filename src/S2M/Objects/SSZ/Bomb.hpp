#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Bomb : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxHurt;
        RSDK::Hitbox hitboxRange;
        RSDK::Hitbox hitboxShrapnel;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxExplosion;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Bomb> state;
    int32 planeFilter;
    RSDK::Vector2 startPos;
    uint8 startDir;
    int32 timer;
    int32 fuseOffset;
    RSDK::Animator mainAnimator;
    RSDK::Animator fuseAnimator;

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

    void CheckOffScreen();
    void CheckPlayerCollisions();

    void State_Init();
    void State_Walk();
    void State_Idle();
    void State_Explode();
    void State_Shrapnel();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Bomb);
};
} // namespace GameLogic