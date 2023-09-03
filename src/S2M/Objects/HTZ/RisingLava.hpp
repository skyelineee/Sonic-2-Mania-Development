#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct RisingLava : RSDK::GameObject::Entity {

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
        RSDK::Animator animator;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxRumble;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<RisingLava> state;
    RSDK::Vector2 size;
    RSDK::Vector2 offset;
    int32 limit;
    RSDK::Hitbox hitbox;

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

    void State_CheckRiseStart();
    void State_RiseShake();
    void State_StoppedRising();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(RisingLava);
};
} // namespace GameLogic