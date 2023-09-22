#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Press : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitbox;
        RSDK::SoundFX sfxImpact;
        RSDK::SoundFX sfxPress;
        bool32 canSuper;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Press> state;
    uint16 size;
    uint32 speed;
    int32 offTop;
    int32 offBottom;
    int32 topOffset;
    uint32 threads;
    int32 height;
    int32 timerStart;
    int32 timer;
    RSDK::Vector2 drawPos;
    uint16 threadSprY;
    uint8 stoodPlayersRoof;
    uint8 stoodPlayersFloor;
    RSDK::Animator crusherAnimator;
    RSDK::Animator threadAnimator;
    RSDK::Animator bumperAnimator;

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

    static bool32 CheckCanSuper(bool32 isHUD);
    void Move();
    void State_Crush();
    void State_FinalCrush();
    void HandleMovement();
    void State_HandleCrates();
    void DrawHandle();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Press);
};
} // namespace GameLogic
