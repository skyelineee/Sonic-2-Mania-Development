// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: GameProgress Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "GameProgress.hpp"
#include "LogHelpers.hpp"

#include "Global/SaveGame.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_STATIC_VARS(GameProgress);

GameProgress *GameProgress::GetProgressRAM() { return (GameProgress *)&globals->saveRAM[0x900]; }
void GameProgress::Save(void (*callback)(bool32 success))
{
    GameProgress *progressRAM = GetProgressRAM();

    if (sceneInfo->inEditor || globals->saveLoaded != STATUS_OK) {
        LogHelpers::Print("WARNING GameProgress Attempted to save before loading SaveGame file");
        if (callback)
            callback(false);

        return;
    }
    else if (progressRAM->disableSaving) {
        if (callback)
            callback(false);

        return;
    }

    SaveGame::SaveFile(callback);
}
void GameProgress::DumpProgress()
{
    if (sceneInfo->inEditor  || globals->saveLoaded != STATUS_OK) {
        LogHelpers::Print("WARNING GameProgress Attempted to dump before loading SaveGame file");
        return;
    }

    GameProgress *progress = GetProgressRAM();

    LogHelpers::Print("=========================");
    LogHelpers::Print("Game Progress:\n");

    for (int32 e = 0; e < 7; ++e) {
        if (progress->emeraldObtained[e])
            LogHelpers::Print("Emerald %d => TRUE", e);
        else
            LogHelpers::Print("Emerald %d => FALSE", e);
    }

    if (progress->allEmeraldsObtained)
        LogHelpers::Print("ALL EMERALDS!\n");
    else
        LogHelpers::Print("YOU'VE NOT ENOUGH EMERALDS!\n");

    for (int32 z = 0; z < 13; ++z) {
        if (progress->zoneCleared[z])
            LogHelpers::Print("Zone %d clear => TRUE", z);
        else
            LogHelpers::Print("Zone %d clear => FALSE", z);
    }

    if (progress->unlockedEndingID < 3) {
        const char *endings[] = { "NO ENDING!", "BAD ENDING!", "GOOD ENDING!" };

        LogHelpers::Print(endings[progress->unlockedEndingID]);
    }

    LogHelpers::Print("\n=========================");
}
void GameProgress::MarkZoneCompleted(int32 zoneID)
{
    if (sceneInfo->inEditor || globals->saveLoaded != STATUS_OK) {
        LogHelpers::Print("WARNING GameProgress Attempted to mark completed zone before loading SaveGame file");
        return;
    }

    if (zoneID > -1) {
        GameProgress *progress = GetProgressRAM();
        for (int32 z = 0; z <= zoneID; ++z) {
            if (!progress->zoneCleared[z]) {
                LogHelpers::Print("PROGRESS Cleared zone %d", z);
                progress->zoneCleared[z] = true;
            }
        }
    }
}
void GameProgress::CollectEmerald(int32 emeraldID)
{
    if (sceneInfo->inEditor || globals->saveLoaded != STATUS_OK) {
        LogHelpers::Print("WARNING GameProgress Attempted to get emerald before loading SaveGame file");
        return;
    }

    GameProgress *progress = GetProgressRAM();

    progress->emeraldObtained[emeraldID] = true;
    bool32 allEmeralds                   = true;
    for (int32 i = 0; i < 7; ++i) {
        allEmeralds = allEmeralds && progress->emeraldObtained[i];
    }

    if (allEmeralds)
        progress->allEmeraldsObtained = true;
}

#if RETRO_REV0U
void GameProgress::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(GameProgress); }
#endif

} // namespace GameLogic