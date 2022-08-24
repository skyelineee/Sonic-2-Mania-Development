#pragma once
#include "Game.hpp"

namespace GameLogic
{

struct GenericTrigger : RSDK::GameObject::Entity {

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
        RSDK::StateMachine<GenericTrigger> callbacks[16];
        int32 playerID;
        RSDK::SpriteAnimation aniFrames;
        GenericTrigger *trigger;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::Vector2 size;
    uint8 triggerID;
    bool32 allPlayers;
    int32 activationLimit;
    RSDK::Hitbox hitbox;
    RSDK::Animator animator;
    bool32 triggered;
    int32 parameter;

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

    RSDK_DECLARE(GenericTrigger);
};
} // namespace GameLogic