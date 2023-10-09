#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Vultron : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum VultronTypes {
        VULTRON_DIVE,
        VULTRON_TARGET,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxRange;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SpriteAnimation nullFrames;
        RSDK::SoundFX sfxVultron;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Vultron> state;
    uint8 type;
    RSDK::Vector2 startPos;
    uint8 startDir;
    uint16 dist;
    uint16 distRemain;
    int32 storeY;
    RSDK::Hitbox hitboxBadnik;
    RSDK::Animator bodyAnimator;
    RSDK::Animator flameAnimator;

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
    void State_Hop();
    void State_Dive();
    void State_Flying();
    void State_Rise();
    void State_PrepareDive();

    void State_Targeting();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Vultron);
};
} // namespace GameLogic