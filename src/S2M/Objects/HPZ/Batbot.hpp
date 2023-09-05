#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Player; 

struct Batbot : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxSpinCheck;
        RSDK::Hitbox hitboxAttack;
        RSDK::Hitbox hitboxPlayer;
        RSDK::SpriteAnimation aniFrames;
        uint8 directionTable[5];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Batbot> state;
    RSDK::Vector2 startPos;
    uint8 startDir;
    uint16 timer;
    int32 arcAngle;
    int32 originY;
    RSDK::Vector2 landPos;
    int32 unused1;
    int32 unused2;
    int32 swoopAngle;
    Player *playerPtr;
    uint8 swoopDir;
    bool32 showJet;
    uint16 unused3; // set to 0, not used, value isn't in S2
    RSDK::Animator bodyAnimator;
    RSDK::Animator jetAnimator;

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

    void State_Init();
    void State_Idle();
    void State_Attack();
    void State_SwoopLeft();
    void State_SwoopRight();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Batbot);
};
} // namespace GameLogic
