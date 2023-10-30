#pragma once
#include "S2M.hpp"

namespace GameLogic
{

#define UNCURLPLANT_NODE_COUNT (8)

struct UncurlPlant : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxNode;
        int32 startingNodeAngles[8];
        int32 targetNodeAnglesReleased[8];
        int32 targetNodeAnglesStood[8];
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<UncurlPlant> state; // unused
    int32 unused;
    RSDK::Vector2 drawPositions[UNCURLPLANT_NODE_COUNT];
    RSDK::Vector2 nodePositions[UNCURLPLANT_NODE_COUNT];
    int32 nodeAngles[UNCURLPLANT_NODE_COUNT];
    int32 targetNodeAngles[UNCURLPLANT_NODE_COUNT];
    int32 stood;
    int32 uncurlPercent;
    int32 stoodNodeID;
    int32 uncurlMode;
    RSDK::Animator nodeAnimator;
    RSDK::Animator decorAnimators[UNCURLPLANT_NODE_COUNT];

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

    void CalculateDrawPositions();
    void CalculatePositions();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UncurlPlant);
};
} // namespace GameLogic
