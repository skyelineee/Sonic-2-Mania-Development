#pragma once
#include "S2M.hpp"

#include "Player.hpp"

namespace GameLogic
{

struct BoundsMarker : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum Types {
        AnyY,
        AboveY,
        BelowY,
        BelowYAny,
    };
    enum AlignTypes {
        AlignPlayer,
        AlignCameraL,
        AlignCameraR,
    };
    enum LockTypes {
        LockNone,
        LockAtBounds,
        LockWithCam,
    };
    enum ExitTypes {
        ExitRestoreBounds,
        ExitDeactivateObj,
        ExitDestroyObj,
        ExitKeepBounds,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<BoundsMarker> state;
    RSDK::Animator animator;
    uint8 type;
    uint8 alignFrom;
    int32 width;
    int32 offset;
    RSDK::Vector2 restoreSpd;
    uint32 lockLeft;
    uint32 lockRight;
    bool32 vsDisable;
    bool32 deactivated;
    int32 boundsT[PLAYER_COUNT];
    int32 boundsB[PLAYER_COUNT];
    int32 boundsL[PLAYER_COUNT];
    int32 boundsR[PLAYER_COUNT];
    int32 deathBounds[PLAYER_COUNT];
    bool32 playerBoundActiveT[PLAYER_COUNT];
    bool32 playerBoundActiveB[PLAYER_COUNT];
    bool32 playerBoundActiveL[PLAYER_COUNT];
    bool32 playerBoundActiveR[PLAYER_COUNT];
    int32 field_14C[PLAYER_COUNT];
    bool32 lockedL[PLAYER_COUNT];
    bool32 lockedR[PLAYER_COUNT];
    bool32 storedBounds;
    int32 field_180[PLAYER_COUNT];
    bool32 destroyOnDeactivate;
    uint8 onExit;
    RSDK::Vector2 restoreAccel;
    int32 field_1A0;
    int32 initialWidth;
    int32 parameter;

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

    void State_Init();
    void State_Apply();
    void State_Restore();

    void StoreBounds();
    void RestoreBounds(bool32 useState);
    void Unknown3(uint8 side);
    void Unknown4(uint8 side);
    void ApplyBounds(Player *player, bool32 setPos);

    static void ApplyAllBounds(Player *player, bool32 setPos);
    static bool32 CheckOnScreen(RSDK::Vector2 *position, RSDK::Vector2 *range);
    static BoundsMarker *GetNearestX();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(BoundsMarker);
};
} // namespace GameLogic