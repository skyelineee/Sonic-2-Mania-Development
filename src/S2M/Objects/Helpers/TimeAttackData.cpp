// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TimeAttackData Object
// Object Author: Ducky & AChickMcNuggie
// ---------------------------------------------------------------------

#include "TimeAttackData.hpp"
#include "LogHelpers.hpp"

#include "Global/SaveGame.hpp"
#include "Global/Zone.hpp"
#include "Helpers/MenuParam.hpp"

static const char *playerNames[] = { "Invalid", "Sonic", "Tails", "Knuckles" };
static const char *actNames[]    = { "Act 1", "Act 2", "Act 3", "" };
static const char *zoneNames[]   = { "OWZ", "EHZ", "CPZ", "ARZ", "SWZ", "CNZ", "HTZ", "MCZ", "SSZ", "OOZ", "MTZ", "CCZ", "SFZ", "DEZ" };
static const char *modeNames[]   = { "Adventure" };

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_STATIC_VARS(TimeAttackData);

void TimeAttackData::Update() {}

void TimeAttackData::LateUpdate() {}

void TimeAttackData::StaticUpdate() {}

void TimeAttackData::Draw() {}

void TimeAttackData::Create(void *data) {}

void TimeAttackData::StageLoad() {}

void TimeAttackData::TrackActClear(StatInfo *stat, uint8 zone, uint8 act, uint8 characterID, int32 time, int32 rings, int32 score)
{
    stat->statID = 0;
    stat->name   = "ACT_CLEAR";
    memset(stat->data, 0, sizeof(stat->data));

    stat->data[0] = (void *)zoneNames[zone];
    stat->data[1] = (void *)actNames[act];
    stat->data[2] = (void *)playerNames[characterID];
    stat->data[3] = 0;
    stat->data[4] = INT_TO_VOID(time);
    stat->data[5] = INT_TO_VOID(rings);
    stat->data[6] = INT_TO_VOID(score);
}

void TimeAttackData::TrackTAClear(StatInfo *stat, uint8 zone, uint8 actID, uint8 characterID, int32 time)
{
    stat->statID = 1;
    stat->name   = "TA_CLEAR";
    memset(stat->data, 0, sizeof(stat->data));

    stat->data[0] = (void *)zoneNames[zone];
    stat->data[1] = (void *)actNames[actID];
    stat->data[2] = (void *)playerNames[characterID];
    stat->data[3] = 0;
    stat->data[4] = INT_TO_VOID(time);
}

void TimeAttackData::TrackEnemyDefeat(StatInfo *stat, uint8 zoneID, uint8 actID, uint8 characterID, int32 x, int32 y)
{
    stat->statID = 2;
    stat->name   = "ENEMY_DEFEAT";
    memset(stat->data, 0, sizeof(stat->data));

    stat->data[0] = (void *)zoneNames[zoneID];
    stat->data[1] = (void *)actNames[actID];
    stat->data[2] = (void *)playerNames[characterID];
    stat->data[3] = 0;
    stat->data[4] = INT_TO_VOID(x);
    stat->data[5] = INT_TO_VOID(y);
}

TimeAttackData *TimeAttackData::GetTimeAttackRAM() { return (TimeAttackData *)&globals->saveRAM[0x800]; }

void TimeAttackData::Clear()
{
    MenuParam *param = MenuParam::GetMenuParam();

    memset(param->menuTag, 0, sizeof(param->menuTag));
    param->menuSelection    = 0;
    param->startedTAAttempt = false;
    param->inTimeAttack     = false;
    param->zoneID           = 0;
    param->actID            = 0;

    param->timeAttackRank = 0;
    globals->gameMode     = MODE_MANIA;

    globals->suppressAutoMusic = false;
    globals->suppressTitlecard = false;
}

int32 TimeAttackData::GetZoneListPos(int32 zoneID, int32 act, int32 characterID)
{
    int32 listPos = 0;
    switch (zoneID) {
        case Zone::OWZ: listPos = act * zoneID; break;
        case Zone::EHZ: listPos = act + 1 * zoneID; break;
        case Zone::CPZ: listPos = act + 1 * zoneID + 1; break;
        case Zone::ARZ: listPos = act + 1 * zoneID + 2; break;
        case Zone::SWZ: listPos = act + 1 * zoneID + 3; break;
        case Zone::CNZ: listPos = act + 1 * zoneID + 4; break;
        case Zone::HTZ: listPos = act + 1 * zoneID + 5; break;
        case Zone::MCZ: listPos = act + 1 * zoneID + 6; break;
        case Zone::SSZ: listPos = act + 1 * zoneID + 7; break;
        case Zone::OOZ: listPos = act + 1 * zoneID + 8; break;
        case Zone::MTZ: listPos = act + 1 * zoneID + 9; break;
        case Zone::CCZ: listPos = act + 1 * zoneID + 10; break;
        case Zone::SFZ: listPos = act + 1 * zoneID + 11; break;
        case Zone::DEZ: listPos = act + 1 * zoneID + 12; break;
        default: break;
    }

    LogHelpers::Print("playerID = %d, zoneID = %d, act = %d", characterID, zoneID, act);
    LogHelpers::Print("listPos = %d", listPos);

    return listPos;
}

uint32 TimeAttackData::GetPackedTime(int32 minutes, int32 seconds, int32 milliseconds) { return 6000 * minutes + 100 * seconds + milliseconds; }

void TimeAttackData::GetUnpackedTime(int32 time, int32 *minutes, int32 *seconds, int32 *milliseconds)
{
    if (minutes)
        *minutes = time / 6000;

    if (seconds)
        *seconds = time % 6000 / 100;

    if (milliseconds)
        *milliseconds = time % 100;
}

uint16 *TimeAttackData::GetRecordedTime(uint8 zoneID, uint8 act, uint8 characterID, uint8 rank)
{
    rank--;
    if (rank >= 3)
        return nullptr;

    TimeAttackData *recordsRAM = nullptr;
    if (globals->saveLoaded == STATUS_OK)
        recordsRAM = TimeAttackData::GetTimeAttackRAM();
    else
        return nullptr;

    return &recordsRAM->records[characterID - 1][zoneID][act][rank];
}

void TimeAttackData::CreateDB()
{
    uint16 id = APITable->InitUserDB("TimeAttackDB.bin", API::Storage::UserDB::UInt8, "zoneID", API::Storage::UserDB::UInt8, "act", API::Storage::UserDB::UInt8, "characterID",
                               API::Storage::UserDB::UInt32, "score", API::Storage::UserDB::UInt32, "replayID", nullptr);
    globals->taTableID = id;
    if (id == (uint16)-1) {
        globals->taTableLoaded = STATUS_ERROR;
    }
    else {
        globals->taTableLoaded = STATUS_OK;
        if (!API::Storage::GetNoSave() && globals->saveLoaded == STATUS_OK) {
            TimeAttackData::MigrateLegacySaves();
        }
    }
}

uint16 TimeAttackData::LoadDB(void (*callback)(bool32 success))
{
    LogHelpers::Print("Loading Time Attack DB");
    globals->taTableLoaded = STATUS_CONTINUE;

    sVars->loadEntityPtr          = (Entity *)sceneInfo->entity;
    sVars->loadCallback           = callback;
    globals->taTableID            = APITable->LoadUserDB("TimeAttackDB.bin", TimeAttackData::LoadDBCallback);

    if (globals->taTableID == -1) {
        LogHelpers::Print("Couldn't claim a slot for loading %s", "TimeAttackDB.bin");
        globals->taTableLoaded = STATUS_ERROR;
    }

    return globals->taTableID;
}

void TimeAttackData::SaveDB(void (*callback)(bool32 success))
{
    if (API::Storage::GetNoSave() || globals->taTableID == (uint16)-1 || globals->taTableLoaded != STATUS_OK) {
        if (callback)
            callback(false);
    }
    else {
        LogHelpers::Print("Saving Time Attack DB");

        sVars->saveEntityPtr = (Entity *)sceneInfo->entity;
        sVars->saveCallback  = callback;

        APITable->SaveUserDB(globals->taTableID, TimeAttackData::SaveDBCallback);
    }
}

void TimeAttackData::LoadDBCallback(int32 status)
{
    if (status == STATUS_OK) {
        globals->taTableLoaded = STATUS_OK;
        APITable->SetupUserDBRowSorting(globals->taTableID);
        LogHelpers::Print("Load Succeeded! Replay count: %d", APITable->GetSortedUserDBRowCount(globals->taTableID));
    }
    else {
        LogHelpers::Print("Load Failed! Creating new Time Attack DB");
        TimeAttackData::CreateDB();
    }

    // Bug Details:
    // Due to how options work, this is called after the db is loaded, but before the result is assigned to globals->taTableID
    // meaning that globals->taTableID will be 0xFFFF initially, even if the tabel id was loaded and returned successfully
    LogHelpers::Print("Replay DB Slot => %d, Load Status => %d", globals->taTableID, globals->taTableLoaded);

    if (sVars->loadCallback) {
        Entity *entStore = (Entity *)sceneInfo->entity;
        if (sVars->loadEntityPtr)
            sceneInfo->entity = sVars->loadEntityPtr;
        sVars->loadCallback(status == STATUS_OK);
        sceneInfo->entity = entStore;

        sVars->loadCallback  = nullptr;
        sVars->loadEntityPtr = nullptr;
    }
}

void TimeAttackData::SaveDBCallback(int32 status)
{
    if (sVars->saveCallback) {
        Entity *entStore = (Entity *)sceneInfo->entity;
        if (sVars->saveEntityPtr)
            sceneInfo->entity = sVars->saveEntityPtr;
        sVars->saveCallback(status == STATUS_OK);
        sceneInfo->entity = entStore;

        sVars->saveCallback  = nullptr;
        sVars->saveEntityPtr = nullptr;
    }
}

void TimeAttackData::MigrateLegacySaves()
{
    if (globals->saveLoaded == STATUS_OK) {
        sVars->isMigratingData = true;

        LogHelpers::Print("===========================");
        LogHelpers::Print("Migrating Legacy TA Data...");
        LogHelpers::Print("===========================");

        for (int32 zone = Zone::OWZ; zone <= Zone::DEZ; ++zone) {
            for (int32 act = ACT_1; act <= ACT_2; ++act) {
                for (int32 charID = CHAR_SONIC; charID <= CHAR_KNUX; ++charID) {
                    for (int32 rank = 0; rank < 3; ++rank) {
                        uint16 *records = TimeAttackData::GetRecordedTime(zone, act, charID, rank + 1);
                        if (records && *records) {
                            int32 score = *records;
                            LogHelpers::Print("Import: zone=%d act=%d charID=%d rank=%d -> %d", zone, act, charID, rank, score);
                            TimeAttackData::AddRecord(zone, act, charID, score, nullptr);
                        }
                    }
                }
            }
        }

        sVars->isMigratingData = false;
    }
}

int32 TimeAttackData::AddDBRow(uint8 zoneID, uint8 act, uint8 characterID, int32 score)
{
    if (globals->taTableLoaded != STATUS_OK)
        return -1;

    uint16 rowID    = APITable->AddUserDBRow(globals->taTableID);
    uint32 replayID = 0;

    APITable->SetUserDBValue(globals->taTableID, rowID, API::Storage::UserDB::UInt8, "zoneID", &zoneID);
    APITable->SetUserDBValue(globals->taTableID, rowID, API::Storage::UserDB::UInt8, "act", &act);
    APITable->SetUserDBValue(globals->taTableID, rowID, API::Storage::UserDB::UInt8, "characterID", &characterID);
    APITable->SetUserDBValue(globals->taTableID, rowID, API::Storage::UserDB::UInt32, "score", &score);
    APITable->SetUserDBValue(globals->taTableID, rowID, API::Storage::UserDB::UInt32, "replayID", &replayID);

    uint32 uuid = APITable->GetUserDBRowUUID(globals->taTableID, rowID);
    char buf[0x20];
    memset(buf, 0, sizeof(buf));
    APITable->GetUserDBRowCreationTime(globals->taTableID, rowID, buf, sizeof(buf) - 1, "%Y/%m/%d %H:%M:%S");

    LogHelpers::Print("Time Attack DB Added Entry");
    LogHelpers::Print("Created at %s", buf);
    LogHelpers::Print("Row ID: %d", rowID);
    LogHelpers::Print("UUID: %08X", uuid);

    return rowID;
}

int32 TimeAttackData::AddRecord(uint8 zoneID, uint8 act, uint8 characterID, int32 score, void (*callback)(bool32 success))
{
    uint16 row  = TimeAttackData::AddDBRow(zoneID, act, characterID, score);
    uint32 uuid = APITable->GetUserDBRowUUID(globals->taTableID, row);

    TimeAttackData::ConfigureTableView(zoneID, act, characterID);

    if (APITable->GetSortedUserDBRowCount(globals->taTableID) > 3) {
        APITable->RemoveDBRow(globals->taTableID, APITable->GetSortedUserDBRowID(globals->taTableID, 3));

        TimeAttackData::ConfigureTableView(zoneID, act, characterID);
    }

    int32 rank  = 0;
    int32 rowID = 0;
    for (rank = 0; rank < 3; ++rank) {
        rowID = APITable->GetSortedUserDBRowID(globals->taTableID, rank);
        if (APITable->GetUserDBRowUUID(globals->taTableID, rowID) == uuid)
            break;
    }

    if (rank == 3) {
        if (callback)
            callback(false);

        return 0;
    }

    sVars->uuid         = uuid;
    sVars->rowID        = rowID;
    sVars->personalRank = rank + 1;

    if (sVars->isMigratingData) {
        if (callback)
            callback(true);
    }
    else {
        TimeAttackData::SaveDB(callback);
    }

    return rank + 1;
}

int32 TimeAttackData::GetScore(uint8 zoneID, uint8 act, uint8 characterID, int32 rank)
{
    if (rank > 3 && rank)
        return 0;

    uint8 rankID = rank - 1;
    if (!sVars->loaded || characterID != sVars->characterID || zoneID != sVars->zoneID || act != sVars->act) {
        TimeAttackData::ConfigureTableView(zoneID, act, characterID);
    }

    int32 rowID = APITable->GetSortedUserDBRowID(globals->taTableID, rankID);
    if (rowID == -1)
        return 0;

    int32 score = 0;
    APITable->GetUserDBValue(globals->taTableID, rowID, API::Storage::UserDB::UInt32, "score", &score);

    return score;
}

int32 TimeAttackData::GetReplayID(uint8 zoneID, uint8 act, uint8 characterID, int32 rank)
{
    if (rank > 3 && rank)
        return 0;

    uint8 rankID = rank - 1;
    if (!sVars->loaded || characterID != sVars->characterID || zoneID != sVars->zoneID || act != sVars->act) {
        TimeAttackData::ConfigureTableView(zoneID, act, characterID);
    }

    int32 row = APITable->GetSortedUserDBRowID(globals->taTableID, rankID);
    if (row != -1) {
        int32 replayID = 0;
        APITable->GetUserDBValue(globals->taTableID, row, API::Storage::UserDB::UInt32, "replayID", &replayID);

        return replayID;
    }

    return 0;
}

void TimeAttackData::ConfigureTableView(uint8 zoneID, uint8 act, uint8 characterID)
{
    LogHelpers::Print("ConfigureTableView(%d, %d, %d, %d)", characterID, zoneID, act);

    // setup every sort row ID for every entry
    APITable->SetupUserDBRowSorting(globals->taTableID);

    // remove any sort row IDs that dont match the following values
    APITable->AddRowSortFilter(globals->taTableID, API::Storage::UserDB::UInt8, "zoneID", &zoneID);
    APITable->AddRowSortFilter(globals->taTableID, API::Storage::UserDB::UInt8, "act", &act);
    APITable->AddRowSortFilter(globals->taTableID, API::Storage::UserDB::UInt8, "characterID", &characterID);

    // sort the remaining rows
    APITable->SortDBRows(globals->taTableID, API::Storage::UserDB::UInt32, "score", false);

    sVars->loaded      = true;
    sVars->zoneID      = zoneID;
    sVars->act         = act;
    sVars->characterID = characterID;
}

void TimeAttackData::Leaderboard_GetRank(bool32 success, int32 rank)
{
    if (success) {
        LogHelpers::Print("Got back leaderboard rank: %d. Not bad!", rank);
        sVars->leaderboardRank = rank;
    }
}

void TimeAttackData::AddLeaderboardEntry(uint8 zoneID, uint8 act, uint8 characterID, int32 score)
{
    StatInfo stat;
    TimeAttackData::TrackTAClear(&stat, zoneID, act, characterID, score);
    API::Stats::TryTrackStat(&stat);

    LeaderboardID *curLeaderboard = TimeAttackData::GetLeaderboardInfo(zoneID, act, characterID);

    API::Leaderboards::TrackScore(curLeaderboard, score, TimeAttackData::Leaderboard_GetRank);
}

LeaderboardID *TimeAttackData::GetLeaderboardInfo(uint8 zoneID, uint8 act, uint8 characterID)
{
    LeaderboardID *info = nullptr;
    if (zoneID < 14 && act < 2 && characterID <= 3) {
        int32 id = 10 * zoneID + 5 * act + (characterID - 1);
    }

    return info;
}

#if RETRO_INCLUDE_EDITOR
void TimeAttackData::EditorDraw() {}

void TimeAttackData::EditorLoad() {}
#endif

void TimeAttackData::Serialize() {}


} // namespace GameLogic