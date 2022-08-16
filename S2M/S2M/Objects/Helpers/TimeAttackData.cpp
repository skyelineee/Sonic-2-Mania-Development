// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TimeAttackData Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "TimeAttackData.hpp"
#include "LogHelpers.hpp"

#include "Global/SaveGame.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_STATIC_VARS(TimeAttackData);

TimeAttackData *TimeAttackData::GetTimeAttackRAM()
{
    if (globals->saveLoaded != STATUS_OK)
        return nullptr;

    return (TimeAttackData *)&globals->saveRAM[0x800];
}

void TimeAttackData::GetUnpackedTime(uint32 time, int32 *minutes, int32 *seconds, int32 *milliseconds)
{
    if (minutes)
        *minutes = time / 6000;

    if (seconds)
        *seconds = time % 6000 / 100;

    if (milliseconds)
        *milliseconds = time % 100;
}

uint32 TimeAttackData::GetPackedTime(int32 minutes, int32 seconds, int32 milliseconds) { return 6000 * minutes + 100 * seconds + milliseconds; }

int32 TimeAttackData::GetScore(uint8 zoneID, uint8 act, uint8 characterID, bool32 encore, int32 rank) { return 0; }

int32 TimeAttackData::SetScore(uint8 zoneID, uint8 act, uint8 characterID, int32 time)
{
    TimeAttackData *recordsRAM = GetTimeAttackRAM();

    int32 rank = 0;
    for (int32 r = 0; r < 3; ++r) {
        if (time < recordsRAM->records[zoneID][act][0].time || !recordsRAM->records[zoneID][act][0].characterID) {
            rank = 1 + r;
            break;
        }
    }

    if (rank) {
        recordsRAM->records[zoneID][act][rank - 1].characterID = characterID;
        recordsRAM->records[zoneID][act][rank - 1].time        = time;
        SaveGame::SaveFile(nullptr);
    }

    return rank;
}

#if RETRO_REV0U
void TimeAttackData::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(TimeAttackData); }
#endif

} // namespace GameLogic