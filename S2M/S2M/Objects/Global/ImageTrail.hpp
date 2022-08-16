#pragma once
#include "S2M.hpp"

#include "Player.hpp"

namespace GameLogic
{

// Constants
#define IMAGETRAIL_TRACK_COUNT (7)

struct ImageTrail : RSDK::GameObject::Entity {

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
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    Player *player;
    int32 playerClassID;
    RSDK::Vector2 currentPos;
    RSDK::Vector2 statePos[IMAGETRAIL_TRACK_COUNT];
    int32 currentRotation;
    int32 stateRotation[IMAGETRAIL_TRACK_COUNT];
    uint8 currentDirection;
    uint8 stateDirection[IMAGETRAIL_TRACK_COUNT];
    uint8 currentVisible;
    uint8 stateVisible[IMAGETRAIL_TRACK_COUNT];
    int32 currentScale;
    int32 stateScale[IMAGETRAIL_TRACK_COUNT];
    RSDK::Animator currentAnimator;
    RSDK::Animator stateAnimator[IMAGETRAIL_TRACK_COUNT];
    int32 baseAlpha;
    int32 fadeoutTimer;
    RSDK::Vector2 lastPos;

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

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ImageTrail);
};
} // namespace GameLogic