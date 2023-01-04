#pragma once
#include "S2M.hpp"
#include "UIButton.hpp"
#include "UIHeading.hpp"

namespace GameLogic
{

// Forward declarations
// these tell the compiler these classes exist but not what they are
// can only be used for ptrs, including these files creates a circular dependency so dont do that :3
struct UIButtonPrompt;
struct UICarousel;
struct UIShifter;

#define UICONTROL_BUTTON_COUNT (64)

#define UICONTROL_PROMPT_COUNT (4)

struct UIControl : RSDK::GameObject::Entity {

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
        bool32 isProcessingInput;
        bool32 inputLocked;
        bool32 lockInput;
        bool32 upPress[PLAYER_COUNT];
        bool32 downPress[PLAYER_COUNT];
        bool32 leftPress[PLAYER_COUNT];
        bool32 rightPress[PLAYER_COUNT];
        bool32 backPress[PLAYER_COUNT];
        bool32 confirmPress[PLAYER_COUNT];
        bool32 yPress[PLAYER_COUNT];
        bool32 xPress[PLAYER_COUNT];
        bool32 startPress[PLAYER_COUNT];
        bool32 anyUpPress;
        bool32 anyDownPress;
        bool32 anyLeftPress;
        bool32 anyRightPress;
        bool32 anyConfirmPress;
        bool32 anyBackPress;
        bool32 anyYPress;
        bool32 anyXPress;
        bool32 anyStartPress;
        bool32 forceBackPress;
        bool32 hasTouchInput;
        int32 timer;
        RSDK::SpriteAnimation aniFrames;
    };

    RSDK::StateMachine<UIControl> state;
    RSDK::Animator animator;
    int32 buttonID;
    RSDK::String tag;
    RSDK::String parentTag;
    bool32 activeOnLoad;
    bool32 noWidgets;
    bool32 resetSelection;
    uint8 buttonCount;
    uint8 promptCount;
    uint8 rowCount;
    uint8 columnCount;
    uint8 startingID;
    RSDK::Vector2 size;
    RSDK::Vector2 cameraOffset;
    RSDK::Vector2 scrollSpeed;
    bool32 noClamp;
    bool32 noWrap;
    RSDK::Vector2 startPos;
    RSDK::Vector2 targetPos;
    bool32 childHasFocus;
    bool32 dialogHasFocus;
    bool32 popoverHasFocus;
    bool32 hasStoredButton;
    bool32 selectionDisabled;
    bool32 menuWasSetup;
    int32 backoutTimer;
    int32 storedButtonID;
    int32 lastButtonID;
    UIHeading *heading;
    UIShifter *shifter;
    UICarousel *carousel;
    UIButtonPrompt *prompts[UICONTROL_PROMPT_COUNT];
    UIButton *buttons[UICONTROL_BUTTON_COUNT];
    RSDK::Action<bool32> backPressCB;
    RSDK::Action<void> processButtonInputCB;
    RSDK::Action<void> menuSetupCB;
    RSDK::Action<void> menuUpdateCB;
    RSDK::Action<void> yPressCB;
    RSDK::Action<void> xPressCB;

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

    static int32 GetButtonID(UIControl *control, UIButton *entity);
    static void MenuChangeButtonInit(UIControl *control);

    static void SetActiveMenuButtonPrompts(UIControl *entity);
    static void SetActiveMenu(UIControl *entity);
    static void SetMenuLostFocus(UIControl *entity);
    void SetInactiveMenu(UIControl *entity);
    void SetupButtons();

    static UIControl *GetUIControl();
    static bool32 isMoving(UIControl *entity);
    static void MatchMenuTag(const char *text);
    static void HandleMenuChange(RSDK::String *newMenuTag);
    static void HandleMenuLoseFocus(UIControl *parent);
    static void ReturnToParentMenu();

    static void ClearInputs(uint8 buttonID);
    static void SetTargetPos(UIControl *entity, int32 x, int32 y);
    void HandlePosition();
    void ProcessInputs();
    void ProcessButtonInput();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIControl);
};
} // namespace GameLogic