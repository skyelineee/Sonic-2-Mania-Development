#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIChoice : RSDK::GameObject::Entity {

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

    S2M_UI_ITEM_BASE(UIChoice)
    RSDK::Vector2 size;
    int32 listID;
    int32 frameID;
    bool32 noText;
    bool32 auxIcon;
    int32 auxListID;
    int32 auxFrameID;
    int32 align;
    int32 arrowWidth;
    bool32 isDisabled;
    int32 bgEdgeSize;
    int32 buttonBounceOffset;
    int32 buttonBounceVelocity;
    bool32 textVisible;
    bool32 clearParentState;
    int32 touchID;
    RSDK::Animator labelAnimator;
    RSDK::Animator iconAnimator;
    RSDK::Animator leftArrowAnimator;
    RSDK::Animator rightArrowAnimator;
    RSDK::SpriteAnimation aniFrames;

    // ==============================
    // INSTANCE VARS
    // ==============================

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

    static void SetChoiceActive(UIChoice *choice);
    static void SetChoiceInactive(UIChoice *choice);
    void TouchedCB_Left();
    void TouchedCB_Right();
    bool32 CheckTouch();

    // ==============================
    // STATES
    // ==============================

    void State_HandleButtonLeave();
    void State_HandleButtonEnter();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIChoice);
};
} // namespace GameLogic
