#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Player;

struct Grabber : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxBadnik;
        RSDK::Hitbox hitboxGrab;
        RSDK::Hitbox hitboxRange;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SpriteAnimation nullFrames;
        RSDK::SoundFX sfxGrab;
        RSDK::SoundFX sfxDestroy;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Grabber> state;
    RSDK::Animator bodyAnimator;
    RSDK::Animator clampAnimator;
    RSDK::Animator wheelAnimator;
    Player *grabbedPlayer;
    uint16 struggleDelay;
    uint16 struggleTimer;
    uint16 struggleFlags;
    uint16 grabDelay;
    RSDK::Vector2 startPos;
    uint8 startDir;
    int32 turnTimer;
    int32 timer;

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

    void DebugDraw();
    void DebugSpawn();

    void CheckPlayerCollisions();
    void CheckOffScreen();
    void HandleExplode();

    void State_Init();
    void State_CheckForGrab();
    void State_GrabDelay();
    void State_HandleTurn();
    void State_TryToGrab();
    void State_RiseUp();
    void State_GrabbedPlayer();
    void State_Struggle();
    void State_PlayerEscaped();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Grabber);
};
} // namespace GameLogic