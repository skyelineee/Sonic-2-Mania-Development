#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct GameProgress : RSDK::GameObject::Entity {

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

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(GameProgress);
};
} // namespace GameLogic