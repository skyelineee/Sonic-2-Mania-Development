#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Pinata : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum PinataPriorities { PINATA_PRIO_HIGH, PINATA_PRIO_LOW };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxPinata;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxPinata;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Pinata> state;
    bool32 respawn;
    int32 priority;
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

    void DebugDraw();
    void DebugSpawn();

    void State_CheckPlayerCollisions();
    void State_Destroyed();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Pinata);
};
} // namespace GameLogic