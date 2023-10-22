#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SpeedBooster : RSDK::GameObject::Entity {

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
        RSDK::StateMachine<SpeedBooster> defaultState;
        RSDK::Hitbox hitbox;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxSpeedBooster;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<SpeedBooster> state;
    uint8 speed;
    int32 playerPos[PLAYER_COUNT];
    RSDK::Vector2 drawPos;
    uint8 timer;
    uint8 playerTimer[PLAYER_COUNT];
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

    void State_SpeedBooster();
    void HandleInteractions();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SpeedBooster);
};
} // namespace GameLogic