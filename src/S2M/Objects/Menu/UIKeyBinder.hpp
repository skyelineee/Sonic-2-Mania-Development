#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIKeyBinder : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum UIKeyBinderKeyIDs {
        UIKEYBINDER_UP,
        UIKEYBINDER_DOWN,
        UIKEYBINDER_LEFT,
        UIKEYBINDER_RIGHT,
        UIKEYBINDER_A,
        UIKEYBINDER_B,
        UIKEYBINDER_X,
        UIKEYBINDER_Y,
        UIKEYBINDER_START,
    };

    enum UIKeyBinderKeyFrameIDs {
        UIKEYBINDER_FRAME_UP    = 7,
        UIKEYBINDER_FRAME_DOWN  = 8,
        UIKEYBINDER_FRAME_LEFT  = 9,
        UIKEYBINDER_FRAME_RIGHT = 10,
        UIKEYBINDER_FRAME_A     = 13,
        UIKEYBINDER_FRAME_B     = 1,
        UIKEYBINDER_FRAME_X     = 3,
        UIKEYBINDER_FRAME_Y     = 11,
        UIKEYBINDER_FRAME_START = 12,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxFail;
        UIKeyBinder *activeBinder;
        int32 activeInputID;
        int32 activeButtonID;
        bool32 isSelected;
    };

    S2M_UI_ITEM_BASE(UIKeyBinder);
    RSDK::StateMachine<UIKeyBinder> state;
    uint8 type;
    uint8 inputID;
    int32 listID;
    int32 frameID;
    RSDK::Vector2 size;
    int32 bgEdgeSize;
    int32 textBounceOffset;
    int32 buttonBounceOffset;
    int32 textBounceVelocity;
    int32 buttonBounceVelocity;
    int32 unused1; // set to 512, never used
    bool32 textVisible;
    int32 unused2;
    RSDK::Animator labelAnimator;
    RSDK::Animator keyAnimator;
    RSDK::SpriteAnimation textFrames;
    int32 lasyKeyMap;

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

    static int32 GetButtonListID();
    static int32 GetMappings(int32 input, int32 button);
    static void SetMappings(int32 input, int32 button, int32 keyMap);
    static int32 GetKeyNameFrameID(int32 id);
    void DrawSprites();

    static void ActionCB();
    bool32 CheckButtonEnterCB();
    bool32 CheckSelectedCB();
    void ButtonEnterCB();
    void ButtonLeaveCB();
    void SelectedCB();

    static void MoveKeyToActionCB_No();
    static void MoveKeyToActionCB_Yes();

    // ==============================
    // STATES
    // ==============================

    void State_HandleButtonLeave();
    void State_HandleButtonEnter();
    void State_Selected();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIKeyBinder);
};
} // namespace GameLogic
