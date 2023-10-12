#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Camera;

struct TornadoPath : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum TornadoPathTypes {
        TORNADOPATH_START,
        TORNADOPATH_DUD,
        TORNADOPATH_SETSPEED,
        TORNADOPATH_SETCAMERA,
        TORNADOPATH_DISABLEINTERACTIONS,
        TORNADOPATH_EXITTORNADO_STOPAUTOSCROLL,
        TORNADOPATH_EXITTORNADO_TRAINSEQ,
        TORNADOPATH_ENTERTORNADO,
        TORNADOPATH_ENTERTORNADO_FLYTOBOSS,
        TORNADOPATH_HANDLEBOSS_MSZ1ST,
        TORNADOPATH_SETSPEED_HIGH,
        TORNADOPATH_GOTOSTOP,
        TORNADOPATH_SETUPMSZ1CUTSCENEST,
        TORNADOPATH_CRASHFINISH,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        Camera *camera;
        RSDK::Vector2 moveVel;
        int32 hitboxID;
        RSDK::SpriteAnimation aniFrames;
        RSDK::Animator animator;
        bool32 crashFinished;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<TornadoPath> state;
    int32 type;
    int32 targetSpeed;
    int32 timer;
    bool32 easeToSpeed;
    bool32 fastMode;
    RSDK::Vector2 size;
    int32 forceSpeed;
    RSDK::Hitbox hitbox;
    int32 currentSpeed;
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

    void SetupHitbox();
    void HandleMoveSpeed();
    void State_SetTornadoSpeed();
    void State_ReturnCamera();
    void State_DisablePlayerInteractions();
    void State_ExitTornadoSequence();
    void State_PrepareCatchPlayer();
    void State_CatchPlayer();
    void State_UberCaterkillerBossNode();
    void State_HandleUberCaterkillerBoss();
    void State_GoToStopNode();
    void State_SetupMSZ1CutsceneST();
    void State_FinishCrash();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TornadoPath);
};
} // namespace GameLogic
