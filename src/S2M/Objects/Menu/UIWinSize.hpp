#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIWinSize : RSDK::GameObject::Entity {

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

    S2M_UI_ITEM_BASE(UIWinSize);
    RSDK::Vector2 size;
    int32 align;
    int32 arrowWidth;
    int32 selection;
    int32 prevSelection;
    int32 bgEdgeSize;
    int32 textBounceOffset;
    int32 buttonBounceOffset;
    int32 textBounceVelocity;
    int32 buttonBounceVelocity;
    bool32 textVisible;
    RSDK::String text;
    int32 touchID;
    int32 maxScale;
    RSDK::Animator unusedAnimator;
    RSDK::Animator textAnimator;
    RSDK::Animator arrowAnimatorL;
    RSDK::Animator arrowAnimatorR;

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

    void SetupText(UIWinSize *entityPtr);
    void ApplySettings();
    void ProcessButtonCB();
    bool32 ProcessTouchCB();
    void TouchedCB_Left();
    void TouchedCB_Right();
    static void SetChoiceActive(UIWinSize *entity);
    static void SetChoiceInactive(UIWinSize *entity);

    // ==============================
    // STATES
    // ==============================

    void State_HandleButtonLeave();
    void State_HandleButtonEnter();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIWinSize);
};
} // namespace GameLogic
#pragma once
