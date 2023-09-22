#pragma once
#include "S2M.hpp"
#include "Helpers/Soundboard.hpp"

namespace GameLogic
{

struct MetalArm : RSDK::GameObject::Entity {

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
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<MetalArm> state;
    int32 startAngleA;
    int32 startAngleB;
    int32 endAngleA;
    int32 endAngleB;
    int32 durationA;
    int32 durationB;
    int32 holdDuration;
    RSDK::Vector2 armAngle;
    uint8 stoodPlayers;
    RSDK::Vector2 startPos;
    int32 moveTimer;
    int32 holdTimer;
    RSDK::Vector2 armPosition;
    RSDK::Vector2 moveOffset;
    RSDK::Hitbox hitbox;
    RSDK::Animator baseAnimator;
    RSDK::Animator armAAnimator;
    RSDK::Animator armBAnimator;
    RSDK::Animator platformAnimator;

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

    RSDK::Vector2 GetArmPosition();
    void CheckPlayerCollisions();
    void State_Idle();
    void State_MoveToHold();
    void State_Holding();
    void State_MoveToStart();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(MetalArm);
};
} // namespace GameLogic
