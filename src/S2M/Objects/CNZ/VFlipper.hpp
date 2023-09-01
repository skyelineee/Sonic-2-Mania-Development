#pragma once
#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct VFlipper : RSDK::GameObject::Entity {

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

    int32 timer;
    RSDK::Animator animator;
    RSDK::StateMachine<VFlipper> state;

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

    void DebugDraw();
    void DebugSpawn();

    // STATES
    void State_Idle();
    void State_Activated_L();
    void State_Activated_R();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(VFlipper);
};
} // namespace GameLogic