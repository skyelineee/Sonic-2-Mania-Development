#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SeeSaw : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum SeeSawTilts {
        SEESAW_TILT_L,
        SEESAW_TILT_M,
        SEESAW_TILT_R,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 tiltHeightTable[48];
        RSDK::Vector2 orbTargetPos;
        int32 launchVelocity;
        RSDK::Hitbox hitboxPlank;
        RSDK::Hitbox hitboxOrbinaut;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxSpring;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<SeeSaw> state;
    uint8 side;
    RSDK::Vector2 orbPos;
    int32 orbTimer; // unused... in mania, it's used in S1 & S2, which this object is based on
    int32 tiltTimerL;
    int32 tiltTimerM;
    int32 tiltTimerR;
    int32 tilt;
    int32 prevTilt;
    int32 targetTilt;
    int32 launchVelocity;
    int32 orbSide;
    bool32 stood;
    RSDK::Animator pivotAnimator;
    RSDK::Animator plankAnimator;
    RSDK::Animator orbAnimator;

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

    void DebugSpawn();
    void DebugDraw();
    void SetupHitbox(int32 playerX, int32 tilt);
    void State_OrbIdle();
    void State_None();
    void State_NoOrb();
    void State_OrbLaunched();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SeeSaw);
};
} // namespace GameLogic
