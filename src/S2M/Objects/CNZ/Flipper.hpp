#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Flipper : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitbox;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxFlipper;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Flipper> state;
    uint8 activePlayers;
    RSDK::Hitbox hitbox;
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

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Flipper);
};
} // namespace GameLogic
