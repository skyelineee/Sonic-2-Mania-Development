#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct TimeAttackData : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum CharacterIDs {
        CHAR_SONIC_AND_TAILS,
        CHAR_SONIC,
        CHAR_TAILS,
        CHAR_KNUX,
    };

    enum ActIDs {
        ACT_1,
        ACT_2,
        ACT_3,
        ACT_NONE,
    };

    // ==============================
    // STRUCTS
    // ==============================

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
    uint16 records[3][12][2][3];

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
    static void TrackActClear(RSDK::StatInfo *stat, uint8 zone, uint8 act, uint8 charID, int32 time, int32 rings, int32 score);
    static void TrackTAClear(RSDK::StatInfo *stat, uint8 zone, uint8 actID, uint8 charID, int32 time);
    static void TrackEnemyDefeat(RSDK::StatInfo *stat, uint8 zoneID, uint8 actID, uint8 charID, int32 x, int32 y);

    static TimeAttackData *GetTimeAttackRAM();
    static void Clear();
    static int32 GetZoneListPos(int32 zoneID, int32 act, int32 characterID);
    static uint32 GetPackedTime(int32 minutes, int32 seconds, int32 milliseconds);
    static void GetUnpackedTime(int32 time, int32 *minutes, int32 *seconds, int32 *milliseconds);
    static uint16 *GetRecordedTime(uint8 zoneID, uint8 act, uint8 characterID, uint8 rank);

    static void CreateDB();
    static uint16 LoadDB(void (*callback)(bool32 success));
    static void SaveDB(void (*callback)(bool32 success));
    static void LoadDBCallback(int32 status);
    static void SaveDBCallback(int32 status);
    static void MigrateLegacySaves();
    static int32 AddDBRow(uint8 zoneID, uint8 act, uint8 characterID, int32 score);
    static int32 AddRecord(uint8 zoneID, uint8 act, uint8 characterID, int32 score, void (*callback)(bool32 success));
    static int32 GetScore(uint8 zoneID, uint8 act, uint8 characterID, int32 rank);
    static int32 GetReplayID(uint8 zoneID, uint8 act, uint8 characterID, int32 rank);
    static void ConfigureTableView(uint8 zoneID, uint8 act, uint8 characterID);

    static void Leaderboard_GetRank(bool32 success, int32 rank);
    static void AddLeaderboardEntry(uint8 zoneID, uint8 act, uint8 characterID, int32 score);
    static RSDK::LeaderboardID *GetLeaderboardInfo(uint8 zoneID, uint8 act, uint8 characterID);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TimeAttackData);
};
} // namespace GameLogic