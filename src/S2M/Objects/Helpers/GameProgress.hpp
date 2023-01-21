#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct GameProgress : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum GameProgressUnlockIDs {
        UnlockCount
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
    bool32 medals[32];
    bool32 zoneCleared[13];
    bool32 emeraldObtained[7];
    bool32 allEmeraldsObtained;
    bool32 unreadNotifs[9];
    bool32 specialCleared[7];
    bool32 disableSaving;
    uint8 unlockedEndingID;
    uint8 goldMedalCount;
    uint8 silverMedalCount;

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