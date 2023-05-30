#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIOptionPanel : RSDK::GameObject::Entity {

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

    int32 panelSize;
    int32 topListID;
    int32 topFrameID;
    int32 botListID;
    int32 botFrameID;
    bool32 botAlignRight;
    bool32 botHidden;
    RSDK::Vector2 startPos;
    RSDK::Vector2 size;
    RSDK::Vector2 drawPosTop;
    RSDK::Vector2 drawPosBottom;
    RSDK::Hitbox hitbox;
    RSDK::Animator topAnimator;
    RSDK::Animator botAnimator;
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

    void DrawBG();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIOptionPanel);
};
} // namespace GameLogic