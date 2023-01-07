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
    int32 listID;
    int32 frameID;
    int32 align;
    int32 choiceCount;
    uint8 choiceDir;
    bool32 invisible;
    bool32 assignsP1;
    bool32 freeBindP2;
    bool32 transition;
    bool32 stopMusic;
    bool32 isDisabled;
    int32 bgEdgeSize;
    int32 textBounceOffset;
    int32 buttonBounceOffset;
    int32 textBounceVelocity;
    int32 buttonBounceVelocity;
    bool32 textVisible;
    bool32 clearParentState;
    RSDK::Vector2 firstChoicePos;
    int32 selection;
    RSDK::Action<void> choiceChangeCB;
    RSDK::Animator animator;
    RSDK::SpriteAnimation textFrames;
    int32 startListID;
    int32 startFrameID;

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
