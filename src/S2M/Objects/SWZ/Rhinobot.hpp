#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Rhinobot : RSDK::GameObject::Entity {

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
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxHuff;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Rhinobot> state;
    RSDK::StateMachine<Rhinobot> stateDelay;
    int32 unused1;
    int32 timer;
    int32 skidDir;
    int32 moveDir;
    bool32 drawDust;
    int32 acceleration;
    int32 topSpeed;
    RSDK::Vector2 startPos;
    uint8 startDir;
    RSDK::Animator bodyAnimator;
    RSDK::Animator dustAnimator;

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

    bool32 CheckTileCollisions();
    void CheckPlayerCollisions();
    void CheckOffScreen();

    void Delay_Charge();
    void Delay_Skidding();
    void Delay_SkidFinish();

    void State_Init();
    void State_Moving();
    void State_Skidding();
    void State_Idle();
    void State_Fall();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Rhinobot);
};
} // namespace GameLogic
