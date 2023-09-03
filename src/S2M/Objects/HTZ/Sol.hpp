#pragma once
#include "S2M.hpp"

namespace GameLogic
{

#define SOL_FLAMEORB_COUNT (4)

struct Sol : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxOrb;
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Sol> state;
    RSDK::Vector2 startPos;
    uint8 startDir;
    uint8 activeOrbs;
    RSDK::Vector2 positions[SOL_FLAMEORB_COUNT];
    int32 oscillateAngle;
    bool32 isFlameFX;
    bool32 fireOrbs;
    RSDK::Animator mainAnimator;
    RSDK::Animator ballAnimator;

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
    void HandlePlayerHurt();
    void HandleRotation();
    void CheckOffScreen();

    void State_Init();
    void State_Moving();
    void State_ShootingOrbs();
    void State_NoOrbs();

    void State_SmallFireball();
    void State_ActiveFireball();

    void State_FlameDissipate();
    void State_FireballOilFlame();
    void State_OilFlame();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Sol);
};
} // namespace GameLogic
