#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct RotatingStair : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum RotatingStairModes {
        ROTATINGSTAIR_LEFT,
        ROTATINGSTAIR_DOWN,
        ROTATINGSTAIR_RIGHT,
        ROTATINGSTAIR_UP,
        ROTATINGSTAIR_LEFT_INTERVAL,
        ROTATINGSTAIR_DOWN_INTERVAL,
        ROTATINGSTAIR_RIGHT_INTERVAL,
        ROTATINGSTAIR_UP_INTERVAL,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        uint8 unused; // its set to 0, but its not actually used
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<RotatingStair> state;
    RSDK::StateMachine<RotatingStair> stateCollide;
    int32 mode;
    RSDK::Vector2 amplitude;
    int32 speed;
    bool32 hasTension;
    int8 frameID;
    uint8 collision;
    RSDK::Vector2 tileOrigin;
    RSDK::Vector2 centerPos;
    RSDK::Vector2 drawPos;
    RSDK::Vector2 collisionOffset;
    int32 stood;
    int32 timer;
    int32 stoodAngle;
    uint8 stoodPlayers;
    uint8 pushPlayersL;
    uint8 pushPlayersR;
    RSDK::Hitbox hitbox;
    RSDK::Animator animator;
    int32 childCount;
    uint16 oscOff;
    int32 duration;
    int32 interval;

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

    void State_Move();
    void State_Move_Intervals();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(RotatingStair);
};
} // namespace GameLogic