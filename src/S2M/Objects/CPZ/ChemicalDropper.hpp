#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct ChemicalDropper : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum DropperTypes {
        Main,
        Debris,
        Drop,
        Splash,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::Hitbox dropHitbox;
        RSDK::SoundFX splashSFX;
        int32 tubeFrames[18];
        int32 dropletFrames[19];
        int32 dropperFrames[3];
        int32 hatchFrames[6];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::Vector2 originPos;
    RSDK::Vector2 debrisPos;
    int32 timer;
    int32 tubeAnimTimer;
    int32 dropletAnimTimer;
    int32 boundsL;
    int32 boundsR;
    int32 dropperLength;
    int32 tubeFrame;
    int32 dropperFrame;
    int32 chemicalFrame;
    int32 dropletFrame;
    int32 hatchFrame;
    int32 debrisFrame;
    int32 dropletNodeID;
    int32 dropperOffset;
    int32 nextDir;
    int32 type;
    RSDK::StateMachine<ChemicalDropper> state;
    RSDK::StateMachine<ChemicalDropper> dropperAnim; // these would not be needed normally (as anims are handled in the .bin) but in this case since the animations are kinda tedious and wierdly done like states in the og, old v4 method babyyy
    RSDK::StateMachine<ChemicalDropper> machineAnim;
    RSDK::Animator machineAnimator;
    RSDK::Animator tubeAnimator;
    RSDK::Animator dropperAnimator;
    RSDK::Animator chemicalAnimator;
    RSDK::Animator dropletAnimator;
    RSDK::Animator hatchAnimator;
    RSDK::Animator debrisAnimator;
    RSDK::Animator dropAnimator;
    ChemicalDropper *main;
    ChemicalDropper *drop;

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
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif
#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================
    void SpawnDebris();
    void Oscillate();

    // STATES
    void State_MoveRight();
    void State_GetChemicals();
    void State_DropChemicals();
    void State_MoveLeft();
    void State_Destroyed();
    void State_DebrisExplode();
    void State_DebrisDelay();
    void State_Debris();

    // MANI STATES
    void Mani_Idle();
    void Mani_Extend();
    void Mani_Filling();
    void Mani_Retract();

    // DANI STATES
    void Dani_Closed();
    void Dani_Filling();
    void Dani_Wait();
    void Dani_Extend();
    void Dani_OpenHatch();
    void Dani_Retract();

    // DROP STATES
    void Drop_BubbleHit();
    void Drop_CreateSplash();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ChemicalDropper);
};
} // namespace GameLogic