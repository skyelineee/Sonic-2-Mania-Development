#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Tornado : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum TornadoCollideModes {
        TORNADO_MODE_IDLE,
        TORNADO_MODE_JUMP_RECOIL,
        TORNADO_MODE_JUMP_REBOUND,
        TORNADO_MODE_LAND,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 flameOffsets[5];
        RSDK::SpriteAnimation aniFrames;
        RSDK::SpriteAnimation nullFrames;
        RSDK::SoundFX sfxExplosion;
        RSDK::SoundFX sfxImpact;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Tornado> state;
    int32 collideTimer;
    int32 timer;
    int32 turnAngle;
    int32 mode;
    bool32 isStood;
    bool32 showFlame;
    int32 storeY;
    int32 gravityForce;
    int32 prevPosY;
    int32 moveVelocityY;
    RSDK::Vector2 movePos;
    RSDK::Vector2 knuxPos;
    RSDK::Vector2 knuxVel;
    int32 offsetX;
    RSDK::Animator animatorTornado;
    RSDK::Animator animatorPropeller;
    RSDK::Animator animatorPilot;
    RSDK::Animator animatorFlame;

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
    static void StageLoad();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    void State_Init();
    void State_SetupControl();
    void HandlePlayerCollisions();
    void State_PlayerControlled();
    void State_Mayday();
    void State_FlyAway_Right();
    void State_FlyAway_Left();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Tornado);
};
} // namespace GameLogic
