#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Player;

struct TransportTube : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum TransportTubeTypes {
        TRANSPORTTUBE_CHANGEDIR,
        TRANSPORTTUBE_ENTRY,
        TRANSPORTTUBE_TOTARGET_NEXT,
        TRANSPORTTUBE_TOTARGET_PREV,
        TRANSPORTTUBE_TOTARGET_NODE,
        TRANSPORTTUBE_JUNCTION,
        TRANSPORTTUBE_EXIT,
    };

    enum TransportTubeDirMasks {
        TRANSPORTTUBE_DIR_NONE = 0,
        TRANSPORTTUBE_DIR_N    = 1 << 0,
        TRANSPORTTUBE_DIR_S    = 1 << 1,
        TRANSPORTTUBE_DIR_W    = 1 << 2,
        TRANSPORTTUBE_DIR_E    = 1 << 3,
        TRANSPORTTUBE_DIR_NE   = 1 << 4,
        TRANSPORTTUBE_DIR_NW   = 1 << 5,
        TRANSPORTTUBE_DIR_SE   = 1 << 6,
        TRANSPORTTUBE_DIR_SW   = 1 << 7,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 nextSlot[4];
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxTravel;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<TransportTube> state;
    uint8 type;
    uint8 dirMask;
    int32 directionCount;
    RSDK::Vector2 dirVelocity[8];
    int32 directionIDs[8];
    int32 playerTimers[PLAYER_COUNT];
    Player *players[PLAYER_COUNT];
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

    void SetupDirections(TransportTube *entity);
    void HandleVelocityChange(int32 velX, int32 velY);

    void State_ChangeDir();
    void State_Entry();
    void State_ToTargetEntity();
    void State_TargetSeqNode();
    void State_ChooseDir();
    void State_Exit();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TransportTube);
};
} // namespace GameLogic