#pragma once
#include "S2M.hpp"

namespace GameLogic
{

#define TUBINAUT_ORB_COUNT (3)

struct Player;

struct Tubinaut : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxFace;
        RSDK::Hitbox hitboxOrb;
        RSDK::Hitbox hitboxSpin;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxPowerdown;
        RSDK::SoundFX sfxRepel;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Tubinaut> state;
    RSDK::StateMachine<Tubinaut> orbState;
    RSDK::Animator bodyAnimator;
    RSDK::Animator fieldAnimator;
    RSDK::Vector2 startPos;
    RSDK::Vector2 orbPositions[TUBINAUT_ORB_COUNT];
    int32 orbAngles[TUBINAUT_ORB_COUNT];
    int16 reAdjustTimers[TUBINAUT_ORB_COUNT];
    bool32 ballsVisible[TUBINAUT_ORB_COUNT]; // hehe
    RSDK::Animator ballAnimators[TUBINAUT_ORB_COUNT];
    uint16 playerTimers[PLAYER_COUNT];
    int32 attackTimer;
    uint16 activeOrbs;
    uint8 orbCount;
    int32 timer;
    uint16 distanceRemain;
    uint16 distance;

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

    void CheckPlayerCollisions();
    bool32 CheckAttacking(Player *player);
    void OrbHit(Player *player, int32 orbID);

    void HandleRepel(Player *player, int32 playerID);
    void HandleOrbs();
    void CheckOffScreen();

    void State_Init();
    void State_Move();

    void Orb_Relax();
    void Orb_PrepareAttack();
    void Orb_Attack();
    void Orb_Cooldown();

    void Orb_BodyDeath();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Tubinaut);
};
} // namespace GameLogic
