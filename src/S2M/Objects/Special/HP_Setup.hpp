#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct HP_Setup : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum ActIDs {
        Act1,
        Act2,
        Act3,
        Act4,
        Act5,
        Act6,
        Act7,
        ActNone,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        color stageColor1;
        color stageColor2;
        color arrowColor;
        color railEdgeColor;
        RSDK::SceneLayer controlLayer;
        uint8 checkpointID;
        uint8 paletteTimer;
        uint8 paletteIndex1;
        uint8 paletteIndex2;
        bool32 gotEmerald;
        int32 *ringCounts;
        int32 ringCountSonic[3];
        int32 ringCountKnux[3];
        int32 ringCount2P[3];
        color starPalCycle[16];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    uint8 stageID;
    uint8 paletteID;

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

    RSDK_DECLARE(HP_Setup);
};
} // namespace GameLogic