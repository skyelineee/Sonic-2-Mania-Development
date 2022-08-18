#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SuperSparkle : RSDK::GameObject::Entity {

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
    RSDK::StateMachine<SuperSparkle> state;
    RSDK::Animator animator;
    int32 parentSlot;
    int32 timer;
    bool32 canSpawnSparkle;

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

    void State_SuperSparkle();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SuperSparkle);
};
} // namespace GameLogic