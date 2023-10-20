#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Button;

struct Fan : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum FanTypes {
        FAN_V,
        FAN_H,
    };

    enum FanActivationTypes {
        FAN_ACTIVATE_NONE,
        FAN_ACTIVATE_INTERVAL,
        FAN_ACTIVATE_PLATFORM,
        FAN_ACTIVATE_BUTTON,
    };

    enum FanDeactivationTypes {
        FAN_DEACTIVATE_NONE,
        FAN_DEACTIVATE_BUTTON,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 activePlayers;
        uint8 unused;
        int32 minVelocity;
        RSDK::Hitbox hitboxTop;
        RSDK::Hitbox hitboxBottom;
        RSDK::Hitbox hitboxSides;
        RSDK::Hitbox playerHitbox;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxFan;
        bool32 playingFanSfx;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Fan> state;
    RSDK::StateMachine<Fan> stateActivate;
    RSDK::StateMachine<Fan> stateDeactivate;
    uint8 type;
    uint8 activation;
    uint8 deactivation;
    int32 size;
    uint16 interval;
    uint16 intervalOffset;
    uint16 duration;
    uint16 delay;
    int32 buttonTag;
    Button *taggedButton;
    RSDK::Animator animator;

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

    void SetupTagLink();

    void HandlePlayerInteractions_Top();
    void HandlePlayerInteractions_Bottom();
    void HandlePlayerInteractions_Left();
    void HandlePlayerInteractions_Right();

    void State_Started();
    void State_Stopped();

    void Activate_Interval();
    void Activate_Button();
    void Deactivate_Button();
    void Activate_Platform();
    void Activate();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Fan);
};
} // namespace GameLogic
