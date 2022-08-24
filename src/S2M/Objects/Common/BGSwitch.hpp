#pragma once
#include "Game.hpp"

namespace GameLogic
{

struct BGSwitch : RSDK::GameObject::Entity {

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
        int32 screenID;
        RSDK::SceneLayer layerIDs[8];
        RSDK::StateMachine<BGSwitch> switchCallback[8];
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::Vector2 size;
    uint8 bgID;
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

    RSDK_DECLARE(BGSwitch);
};
} // namespace GameLogic