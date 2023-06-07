#pragma once
#include "S2M.hpp"

namespace GameLogic
{

#define UISLIDER_MIN       (0)
#define UISLIDER_MAX       (0x400)
#define UISLIDER_INCREMENT (UISLIDER_MAX / 0x10)

struct UISlider : RSDK::GameObject::Entity {

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
        Entity *activeEntity;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    S2M_UI_ITEM_BASE(UISlider);
    int32 listID;
    int32 frameID;
    RSDK::Vector2 size;
    int32 bgEdgeSize;
    int32 buttonBounceOffset;
    int32 buttonBounceVelocity;
    int32 sliderPos;
    bool32 textVisible;
    bool32 isTouchSelected;
    int32 sliderPosTouch;
    RSDK::StateMachine<UISlider> sliderChangedCB;
    RSDK::Animator textAnimator;

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

    void DrawBGShapes();
    void DrawSlider();

    void ButtonPressCB();
    bool32 TouchCB();
    void ButtonEnterCB();
    void ButtonLeaveCB();
    bool32 CheckButtonEnterCB();
    static bool32 CheckSelectedCB();

    void State_HandleButtonLeave();
    void State_HandleButtonEnter();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UISlider);
};
} // namespace GameLogic