#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UISubHeading : RSDK::GameObject::Entity {

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

    RSDK::StateMachine<UISubHeading> state;
    int32 unused1;
    int32 unused2;
    int32 unused3;
    int32 unused4;
    RSDK::Vector2 size;
    int32 listID;
    int32 frameID;
    int32 align;
    int32 offset;
    int32 bgEdgeSize;
    int32 storedListID;
    int32 storedFrameID;
    RSDK::Animator animator;
    RSDK::SpriteAnimation textFrames;

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

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UISubHeading);
};
} // namespace GameLogic