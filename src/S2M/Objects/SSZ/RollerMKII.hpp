#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Player;

struct Platform;

struct RollerMKII : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxObject;
        RSDK::Hitbox hitboxInner_Rolling;
        RSDK::Hitbox hitboxOuter_Rolling;
        RSDK::Hitbox hitboxInner_Idle;
        RSDK::Hitbox hitboxOuter_Idle;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxBumper;
        RSDK::SoundFX sfxJump;
        RSDK::SoundFX sfxDropDash;
        RSDK::SoundFX sfxRelease;
        RSDK::SoundFX sfxSkidding;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<RollerMKII> state;
    RSDK::Vector2 startPos;
    uint8 startDir;
    uint8 timer;
    bool32 touchedGround;
    Player *playerPtr;
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

    void DebugSpawn();
    void DebugDraw();

    void CheckOffScreen();

    void CheckPlayerCollisions();
    void CheckPlayerCollisions_Rolling();
    int32 HandleObjectCollisions(Entity *otherEntity, RSDK::Hitbox *hitbox);
    bool32 HandlePlatformCollisions(Platform *platform);
    void HandleCollisions();

    void State_Init();
    void State_Idle();
    void State_SpinUp();
    void State_RollDelay();
    void State_Rolling_Air();
    void State_Rolling_Ground();
    void State_Bumped();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(RollerMKII);
};
} // namespace GameLogic