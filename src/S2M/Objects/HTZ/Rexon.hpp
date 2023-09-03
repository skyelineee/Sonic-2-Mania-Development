#pragma once
#include "S2M.hpp"

namespace GameLogic
{

#define REXON_SEGMENT_COUNT (5)

struct Rexon : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum RexonTypes {
        REXON_MAIN,
        REXON_DEBRIS,
        REXON_SHOT,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxBadnik;
        RSDK::Hitbox hitboxShell;
        RSDK::Hitbox hitboxRange;
        RSDK::Hitbox hitboxProjectile;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxShot;
        RSDK::SoundFX sfxExplosion;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Rexon> state;
    bool32 noMove;
    int32 type;
    RSDK::Vector2 startPos;
    uint8 startDir;
    RSDK::Vector2 positions[REXON_SEGMENT_COUNT + 1]; // neck + head
    int32 segmentMagnitude[REXON_SEGMENT_COUNT];
    int32 segmentAmplitude[REXON_SEGMENT_COUNT];
    uint8 segmentDirections[REXON_SEGMENT_COUNT];
    uint8 segmentID;
    int32 timer;
    int32 destroyDelay;
    RSDK::Animator bodyAnimator;
    RSDK::Animator neckAnimator;
    RSDK::Animator headAnimator;

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
    void DestroyRexon(Rexon *rexon, bool32 crushed);

    void State_Init();
    void State_Hidden();
    void State_Rising();
    void State_Shooting();
    void State_Destroyed();
    void State_Explode();

    void State_Debris();
    void State_Projectile();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Rexon);
};
} // namespace GameLogic
