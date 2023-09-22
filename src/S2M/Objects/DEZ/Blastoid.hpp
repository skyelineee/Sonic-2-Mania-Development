#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Blastoid : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxBody;
        RSDK::Hitbox hitboxProjectile;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxShot;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<Blastoid> state;
    uint8 timer;
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
    void State_Body();
    void State_Projectile();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Blastoid);
};
} // namespace GameLogic
