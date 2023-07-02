#pragma once
#include "S2M.hpp"
#include "Player.hpp"

namespace GameLogic
{

struct TimeAttackGate : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxGate;
        RSDK::SpriteAnimation aniFrames;
        TimeAttackGate *restartManager;
        Player *triggerPlayer;
        bool32 started;
        int32 isFinished;
        int32 unused1;
        bool32 suppressedTitlecard;
        bool32 disableRecords;
        RSDK::SoundFX sfxSignpost;
        RSDK::SoundFX sfxTeleport;
        int32 teleportChannel;
        RSDK::StateMachine<TimeAttackGate> startCB;
        RSDK::StateMachine<TimeAttackGate> endCB;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<TimeAttackGate> state;
    RSDK::StateMachine<TimeAttackGate> stateDraw;
    bool32 finishLine;
    int32 extendTop;
    int32 extendBottom;
    RSDK::Vector2 boundsSize;
    RSDK::Vector2 boundsOffset;
    int32 topBoundary;
    int32 timer;
    bool32 hasFinished;
    int32 baseRotation;
    int32 spinSpeed;
    int32 spinTimer;
    int32 radius;
    int32 restartTimer;
    int32 fadeTimer;
    RSDK::Vector2 playerPos;
    RSDK::Animator baseAnimator;
    RSDK::Animator topAnimator;
    RSDK::Animator finAnimator;

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

    void HandleSpin();
    void HandleStart();
    static void AddRecord();
    static void WaitSave_Leaderboards(bool32 success);
    void CheckTouch();

    // States
    void State_Gate();
    void State_Restarter();
    void State_Fadeout();

    // Draw States
    void Draw_Gate();
    void Draw_Restarter();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TimeAttackGate);
};
} // namespace GameLogic