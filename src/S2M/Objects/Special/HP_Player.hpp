#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct HP_Player : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum PlayerAnis {
        // ANI_IDLE,
        ANI_RUN,
        ANI_JUMP,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        uint16 upState;
        uint16 downState;
        uint16 leftState;
        uint16 rightState;
        uint16 jumpPressState;
        uint16 jumpHoldState;
        int32 aiInputSwapTimer;
        bool32 disableP2KeyCheck;
        RSDK::SpriteAnimation sonicFrames;
        RSDK::SpriteAnimation tailsFrames;
        RSDK::SpriteAnimation tailSpriteFrames;
        RSDK::SpriteAnimation knuxFrames;
        RSDK::SoundFX sfxJump;
        RSDK::SoundFX sfxLoseRings;
        uint8 leadingPlayer;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    int32 zpos;
    RSDK::StateMachine<HP_Player> state;
    RSDK::StateMachine<HP_Player> stateInput;
    RSDK::SpriteAnimation aniFrames;
    RSDK::SpriteAnimation tailFrames;
    RSDK::Animator animator;
    RSDK::Animator tailAnimator;
    RSDK::Animator shadowAnimator;
    int32 characterID;
    int32 rings;
    int32 invincibilityTimer;
    int32 spinTimer;
    int32 timer;
    bool32 isVisible;
    Vector3 localPos;
    Vector3 localShadowPos;
    int32 groundPos;
    bool32 sidekick;
    uint8 playerID;
    uint8 controllerID;
    bool32 up;
    bool32 down;
    bool32 left;
    bool32 right;
    bool32 jumpPress;
    bool32 jumpHold;
    int32 xBoundsL;
    int32 xBoundsR;
    int32 vsCheckpointID;
    int32 vsCheckpointRings[3];
    int32 currentLeader;
    int32 changedLeader;
    int32 automoveActive;

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
    static void StageLoad();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    static void LoadSprites();
    static void LoadSpritesVS();
    void LoadPlayerSprites();
    bool32 CheckP2KeyPress();
    void GetDelayedInput();
    void Input_Gamepad();
    void Input_GamepadAssist();
    void Input_AI_Follow();
    void State_Ground();
    void State_Air();
    void State_Hurt();
    void Draw_Normal();
    bool32 Hurt();
    void Hit();

    void GiveRings(int32 amount, bool32 playSfx);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(HP_Player);
};
} // namespace GameLogic