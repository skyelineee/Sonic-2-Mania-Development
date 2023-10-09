#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SeltzerWater : RSDK::GameObject::Entity {

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

    RSDK::StateMachine<SeltzerWater> state;
    int32 offsetAngle;
    int32 gravityStrength;
    int32 oscillateRadius;
    uint16 nodeSlot;
    RSDK::Animator animator;

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

    void State_Sprayed();
    void State_Falling();
    void State_Splash();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SeltzerWater);
};
} // namespace GameLogic
