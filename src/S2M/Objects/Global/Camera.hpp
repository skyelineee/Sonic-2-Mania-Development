#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Camera : RSDK::GameObject::Entity {

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
        RSDK::Vector2 centerBounds;
        RSDK::Vector2 lastCenterBounds;
        int32 field_14;
        int32 field_18;
        int32 field_1C;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<Camera> state;
    RSDK::GameObject::Entity *target;
    int32 screenID;
    RSDK::Vector2 center;
    RSDK::Vector2 targetMoveVel;
    RSDK::Vector2 lastPos;
    RSDK::Vector2 shakePos;
    RSDK::Vector2 shakeDuration;
    RSDK::Vector2 shakeTimer;
    RSDK::Vector2 lookPos;
    RSDK::Vector2 offset;
    bool32 disableYOffset;
    int32 centerY;
    int32 adjustY;
    int32 lerpPercent;
    int32 lerpSpeed;
    int32 lerpType;
    RSDK::Vector2 endLerpPos;
    RSDK::Vector2 startLerpPos;
    RSDK::Vector2 unknown3;
    RSDK::Vector2 boundsOffset;
    int32 boundsL;
    int32 boundsR;
    int32 boundsT;
    int32 boundsB;
    int32 field_10C;

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
    void SetCameraBoundsXY();
    void SetCameraBoundsX();

    static Camera *SetTargetEntity(int32 screen, RSDK::GameObject::Entity *target);
    static void ShakeScreen(int32 screen, int32 strengthX, int32 strengthY, int32 durationX, int32 durationY);

    void HandleHBounds();
    void HandleVBounds();

    void State_Init();
    void State_MapView();
    void State_Center();
    void State_FollowXY();
    void State_FollowX();
    void State_FollowY();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Camera);
};
} // namespace GameLogic