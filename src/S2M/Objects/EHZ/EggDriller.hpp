#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct EHZEggman;
    
enum EHZBossTypes {
    None,
    Car,
    Drill,
    BackWheel,
    FrontWheel,
};

struct EggDriller : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitbox;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<EggDriller> state;
    int32 type;
    int32 timer;
    int32 drillTimer;
    int32 wheelTimer;
    int32 xOffset;
    RSDK::Vector2 drawPos;
    RSDK::Vector2 carPos;
    int32 boundsL;
    int32 boundsR;
    bool32 isDrillOOB;
    RSDK::Animator backWheelAnimator;
    RSDK::Animator carAnimator;
    RSDK::Animator drillAnimator;
    RSDK::Animator frontWheelAnimator;
    EHZEggman *eggman;
    EggDriller *wheel[3]; // object[-1]
    EggDriller *car;   // object[+1]
    EggDriller *drill; // object[+2]

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

    // CAR STATES
    void Car_SelfDriving();
    void Car_EggmanEnter();
    void Car_Driving();
    void Car_Explode();
    void Car_Destroyed();

    // DRILL STATES
    void Drill_Attached();
    void Drill_Idle();
    void Drill_Fired();

    // WHEEL STATES
    void Wheel_Attached();
    void Wheel_Idle();
    void Wheel_Bounce();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(EggDriller);
};
} // namespace GameLogic
