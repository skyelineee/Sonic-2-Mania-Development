#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct OneWayDoor : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxTop;
        RSDK::Hitbox hitboxBottom;
        RSDK::Hitbox hitboxRange;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<OneWayDoor> state;
    RSDK::StateMachine<OneWayDoor> stateDraw;
    int32 offsetY;
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

    void HandlePlayerInteractions();

    void State_MoveDown();
    void State_MoveUp();

    void Draw_CPZ();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(OneWayDoor);
};
} // namespace GameLogic
