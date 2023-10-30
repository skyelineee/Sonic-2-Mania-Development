#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct GymBar : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum GymBarTypes { GYMBAR_HORIZONTAL, GYMBAR_VERTICAL };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxBumper;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 type;
    int32 size;
    bool32 noSwing;
    uint8 playerTimers[PLAYER_COUNT];
    int32 maxX;
    int32 minX;
    RSDK::Animator animator;
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

    void DebugSpawn();
    void DebugDraw();

    void HandlePlayerInteractions();

    void HandleSwingJump();
    void PlayerState_SwingV();
    void PlayerState_Hang();
    void PlayerState_SwingH();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(GymBar);
};
} // namespace GameLogic