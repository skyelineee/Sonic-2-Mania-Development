#pragma once
#include "S2M.hpp"

#define GAMEPROGRESS_MEDAL_COUNT   (32)
#define GAMEPROGRESS_EMERALD_COUNT (7)

namespace GameLogic
{

struct GameProgress : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum GameProgressUnlockIDs {
        UnlockCount
    };

    enum GameProgressEndingIDs {
        EndingNone,
        EndingBad,
        EndingGood,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    bool32 zoneCleared[13];
    bool32 emeraldObtained[7];
    bool32 allEmeraldsObtained;
    bool32 unreadNotifs[9];
    bool32 specialCleared[7];
    bool32 disableSaving;
    uint8 unlockedEndingID;

    // ==============================
    // EVENTS
    // ==============================

#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif

    // ==============================
    // FUNCTIONS
    // ==============================
    static GameProgress *GetProgressRAM();
    static void Save(void (*callback)(bool32 success));
    static void DumpProgress();
    static void ClearProgress();
    static void MarkZoneCompleted(int32 zoneID);
    static void CollectEmerald(int32 emeraldID);
    static int32 GetNotifStringID(int32 type);
    static int32 CountUnreadNotifs();
    static int32 GetNextNotif();
    static bool32 CheckUnlock(uint8 id);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(GameProgress);
};
} // namespace GameLogic