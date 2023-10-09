#pragma once
#include "S2M.hpp"

namespace GameLogic
{

#define CATERKILLER_BODY_COUNT (3)

struct Caterkiller : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitbox;
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Caterkiller> state;
    RSDK::StateMachine<Caterkiller> stateDraw;
    int32 planeFilter;
    RSDK::Vector2 bodyPosition[CATERKILLER_BODY_COUNT];
    uint8 bodyDirection[CATERKILLER_BODY_COUNT];
    uint8 bodyOffset[CATERKILLER_BODY_COUNT];
    uint8 headOffset;
    uint8 timer;
    RSDK::Vector2 startPos;
    uint8 startDir;
    RSDK::Animator headAnimator;
    RSDK::Animator bodyAnimator;

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

    void CheckOffScreen();
    void CheckTileCollisions();

    void Draw_Body();
    void Draw_Segment();

    void HandlePlayerInteractions();
    bool32 CheckTileAngle(int32 x, int32 y, int32 dir);

    // Body States
    void State_Init();
    void State_Contract();
    void State_LiftHead();
    void State_Uncontract();
    void State_LowerHead();

    // Split States
    void StateSplit_Head();
    void StateSplit_Body();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Caterkiller);
};
} // namespace GameLogic