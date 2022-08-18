#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct TimeAttackData : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    // ==============================
    // STRUCTS
    // ==============================
    struct TARecord {
        int32 time;
        int32 characterID;
    };

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        uint8 loaded;
        uint8 zoneID;
        uint8 act;
        uint8 characterID;
        int32 uuid;
        int32 rowID;
        int32 personalRank;
        int32 leaderboardRank;
        bool32 isMigratingData;
        Entity *loadEntityPtr;
        void(*loadCallback)(bool32 success);
        Entity *saveEntityPtr;
        void(*saveCallback)(bool32 success);
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    uint8 padding[0x58];
    TARecord records[18][2][3];

    // ==============================
    // EVENTS
    // ==============================

#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif

    // ==============================
    // FUNCTIONS
    // ==============================
    static TimeAttackData *GetTimeAttackRAM();

    static void GetUnpackedTime(uint32 time, int32 *minutes, int32 *seconds, int32 *milliseconds);
    static uint32 GetPackedTime(int32 minutes, int32 seconds, int32 milliseconds);

    static int32 GetScore(uint8 zoneID, uint8 act, uint8 characterID, bool32 encore, int32 rank);
    static int32 SetScore(uint8 zoneID, uint8 act, uint8 characterID, int32 time);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TimeAttackData);
};
} // namespace GameLogic