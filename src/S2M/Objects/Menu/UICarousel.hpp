#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"

namespace GameLogic
{

struct UICarousel : RSDK::GameObject::Entity {

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
        RSDK::SpriteAnimation aniFrames;  // both of these aren't used, and are thus prolly editor-only
        RSDK::Animator animator; // both of these aren't used, and are thus prolly editor-only
    };

    RSDK::Vector2 shift;
    RSDK::Vector2 clipSize;
    RSDK::Vector2 clipOffset;
    int32 unused1;
    int32 unused2;
    int32 unused3;
    Entity *unused4;
    int32 unused5;
    int32 scrollOffset;
    int32 virtualIndex;
    int32 unused6;
    RSDK::Vector2 startPos;
    int32 buttonID;
    int32 minOffset;
    int32 maxOffset;
    RSDK::Vector2 offset;
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

    void HandleScrolling();
    void HandleButtonPositions();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UICarousel);
};
} // namespace GameLogic
