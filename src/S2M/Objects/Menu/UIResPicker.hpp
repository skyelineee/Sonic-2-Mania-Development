#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIResPicker : RSDK::GameObject::Entity {

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

    S2M_UI_ITEM_BASE(UIResPicker);
    RSDK::Vector2 size;
    int32 align;
    int32 arrowWidth;
    int32 selection;
    int32 prevSelection;
    int32 bgEdgeSize;
    int32 buttonBounceOffset;
    int32 buttonBounceVelocity;
    bool32 textVisible;
    RSDK::String text;
    int32 touchID;
    int32 displayWidth;
    int32 displayHeight;
    int32 displayRefreshRate;
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

    static void GetDisplayInfo(UIResPicker *entity);
    void ApplySettings();
    void ProcessButtonCB();
    bool32 ProcessTouchCB();
    void TouchedCB_Left();
    void TouchedCB_Right();
    static void SetChoiceActive(UIResPicker *entity);
    static void SetChoiceInactive(UIResPicker *entity);

    // ==============================
    // STATES
    // ==============================

    void State_HandleButtonLeave();
    void State_HandleButtonEnter();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIResPicker);
};
} // namespace GameLogic
