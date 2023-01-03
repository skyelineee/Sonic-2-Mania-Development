#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"

namespace GameLogic
{

struct UIShifter : RSDK::GameObject::Entity {

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
        RSDK::Animator animator;
        RSDK::SpriteAnimation aniFrames;
    };

    RSDK::Vector2 shift;
    RSDK::Vector2 lastButtonShift;
    RSDK::Vector2 shiftOffset;
    int32 unused;
    RSDK::Vector2 startPos;
    UIControl *parent;

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

    void HandleShift();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIShifter);
};
} // namespace GameLogic
