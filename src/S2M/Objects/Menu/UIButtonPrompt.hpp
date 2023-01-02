#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIButtonPrompt : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum UIButtonPromptTypes {
        UIBUTTONPROMPT_NONE,
        UIBUTTONPROMPT_KEYBOARD,
        UIBUTTONPROMPT_XBOX,
        UIBUTTONPROMPT_PS4,
        UIBUTTONPROMPT_SWITCH,
        UIBUTTONPROMPT_SATURN_BLACK,
        UIBUTTONPROMPT_SATURN_WHITE,
        UIBUTTONPROMPT_JOYCON_L,
        UIBUTTONPROMPT_JOYCON_R,
        UIBUTTONPROMPT_KEYBOARD_FR,
        UIBUTTONPROMPT_KEYBOARD_IT,
        UIBUTTONPROMPT_KEYBOARD_GE,
        UIBUTTONPROMPT_KEYBOARD_SP,
    };

    enum UIButtonPromptAnchors {
        UIBUTTONPROMPT_ANCHOR_NONE,
        UIBUTTONPROMPT_ANCHOR_TOPLEFT,
        UIBUTTONPROMPT_ANCHOR_TOPRIGHT,
        UIBUTTONPROMPT_ANCHOR_BOTTOMRIGHT,
        UIBUTTONPROMPT_ANCHOR_BOTTOMLEFT,
    };

    enum UIButtonPromptButtons {
        UIBUTTONPROMPT_BUTTON_A,
        UIBUTTONPROMPT_BUTTON_B,
        UIBUTTONPROMPT_BUTTON_X,
        UIBUTTONPROMPT_BUTTON_Y,
        UIBUTTONPROMPT_BUTTON_START,
        UIBUTTONPROMPT_BUTTON_SELECT,
    };

    enum UIButtonPromptTypes {

    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 type;
        int32 inputSlot;
        RSDK::SpriteAnimation aniFrames;
    };

    Entity *parent;
    RSDK::StateMachine<UIButtonPrompt> state;
    int32 timer;
    RSDK::Vector2 startPos;
    int32 promptID;
    int32 buttonID;
    uint8 headingAnchor;
    int32 unused;
    int32 prevPrompt;
    int32 prevButton;
    int32 mappings;
    bool32 textVisible;
    int32 scaleMax;
    int32 scaleSpeed;
    bool32 disableScale;
    RSDK::Vector2 touchSize;
    RSDK::Vector2 touchPos;
    bool32 touched;
    RSDK::Animator decorAnimator;
    RSDK::Animator buttonAnimator;
    RSDK::Animator promptAnimator;
    RSDK::SpriteAnimation textSprite;

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

    static int32 GetButtonMappings(int32 input, int32 button);
    static int32 GetGamepadType();
    static uint8 MappingsToFrame(int32 mappings);
    bool32 CheckTouch();
    void SetButtonSprites();

    // ==============================
    // STATES
    // ==============================

    void State_CheckIfSelected();
    void State_Selected();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIButtonPrompt);
};
} // namespace GameLogic