#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIButton : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum UIButtonAlignments {
        ALIGN_LEFT,
        ALIGN_CENTER,
        ALIGN_RIGHT,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
    };

    S2M_UI_ITEM_BASE(UIButton);
    RSDK::Vector2 size;
    int32 nameListID;
    int32 nameFrameID;
    int32 buttonListID;
    int32 buttonFrameID;
    int32 descriptionListID;
    int32 descriptionFrameID;
    int32 align;
    int32 choiceCount;
    int32 descAngle;
    uint8 choiceDir;
    bool32 invisible;
    bool32 assignsP1;
    bool32 freeBindP2;
    bool32 transition;
    bool32 stopMusic;
    bool32 isDisabled;
    int32 bgEdgeSize;
    int32 buttonBounceOffset;
    int32 buttonBounceVelocity;
    bool32 reverseVelocity;
    bool32 nameVisible;
    bool32 clearParentState;
    bool32 visibleArrow;
    bool32 visibleArrowHorizontal;
    RSDK::Vector2 firstChoicePos;
    int32 selection;
    RSDK::Action<void> choiceChangeCB;
    RSDK::Animator nameAnimator;
    RSDK::Animator buttonAnimator;
    RSDK::Animator descriptionAnimator;
    RSDK::SpriteAnimation buttonFrames;
    RSDK::Vector2 descOffset;
    int32 startNameListID;
    int32 startNameFrameID;
    int32 startButtonListID;
    int32 startButtonFrameID;
    int32 startDescriptionListID;
    int32 startDescriptionFrameID;

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

    static void ManageChoices(UIButton *button);
    static UIButton *GetChoicePtr(UIButton *button, int32 selection);
    static void SetChoiceSelectionWithCB(UIButton *button, int32 selection);
    static void SetChoiceSelection(UIButton *button, int32 selection);
    RSDK::Action<void> *GetActionCB();
    static void FailCB();
    void ProcessButtonCB_Scroll();
    bool32 ProcessTouchCB_Multi();
    bool32 ProcessTouchCB_Single();
    void ProcessButtonCB();
    bool32 CheckButtonEnterCB();
    bool32 CheckSelectedCB();
    void ButtonEnterCB();
    void ButtonLeaveCB();
    void SelectedCB();

    // ==============================
    // STATES
    // ==============================

    void State_HandleButtonLeave();
    void State_HandleButtonEnter();
    void State_Selected();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIButton);
};
} // namespace GameLogic
