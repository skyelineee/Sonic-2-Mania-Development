#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"
#include "UIButton.hpp"

namespace GameLogic
{

#define UIPOPOVER_OPTION_COUNT (4)

struct UIPopover : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum PopoverOptions {
        POPOVER_WATCH,
        POPOVER_CHALLENGE,
        POPOVER_DELETE,
        POPOVER_WATCH_PIVOTED,
        POPOVER_CHALLENGE_PIVOTED,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        UIPopover *activePopover;
        UIControl *storedControl;
        RSDK::StateMachine<UIControl> storedControlState;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<UIPopover> state;
    int32 timer;
    int32 closeDelay;
    RSDK::String unusedTextInfo;
    int32 buttonCount;
    RSDK::Vector2 drawPos; // unused, it's here cuz its based off UIDialog
    UIControl *parent;
    Entity *storedEntity;
    uint8 frameIDs[UIPOPOVER_OPTION_COUNT];
    RSDK::Action<void> callbacks[UIPOPOVER_OPTION_COUNT];
    bool32 closeOnSelect[UIPOPOVER_OPTION_COUNT];
    UIButton *buttons[UIPOPOVER_OPTION_COUNT];
    RSDK::Action<void> closeCB;
    bool32 playEventSfx; // unused, so just a guess based on what this is in UIDialog, though theres about a 50/50 chance its right to some extent
    bool32 useAltColor;  // unused, so just a guess based on what this is in UIDialog, though theres about a 50/50 chance its right to some extent
    RSDK::Animator bgAnimator;
    int32 popoverBGFrame;

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

    static UIPopover *CreatePopover();
    static void AddButton(UIPopover *popover, uint8 frameID, void (*callback)(), bool32 closeOnSelect);
    static void Setup(UIPopover *popover, int32 x, int32 y);
    void DrawSprites();
    void SetupButtonPositions();
    void Close();
    static bool32 BackPressCB();
    static void ButtonActionCB();

    void State_Appear();
    void State_Idle();
    void State_Close();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIPopover);
};
} // namespace GameLogic