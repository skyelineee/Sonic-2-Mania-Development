// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: GameProgress Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "GameProgress.hpp"
#include "LogHelpers.hpp"
#include "Global/Localization.hpp"
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

int32 GameProgress::GetNotifStringID(int32 type)
{
    /*switch (type) {
        //case GAMEPROGRESS_UNLOCK_TIMEATTACK: return STR_TAUNLOCKED;

        //case GAMEPROGRESS_UNLOCK_COMPETITION: return STR_COMPUNLOCKED;

        //case GAMEPROGRESS_UNLOCK_PEELOUT: return STR_PEELOUTUNLOCKED;

        //case GAMEPROGRESS_UNLOCK_INSTASHIELD: return STR_INSTASHIELDUNLOCKED;

        //case GAMEPROGRESS_UNLOCK_ANDKNUX: return STR_ANDKNUXUNLOCKED;

        //case GAMEPROGRESS_UNLOCK_DEBUGMODE: return STR_DEBUGMODEUNLOCKED;

        //case GAMEPROGRESS_UNLOCK_MEANBEAN: return STR_MBMUNLOCKED;

        //case GAMEPROGRESS_UNLOCK_DAGARDEN: return STR_DAGARDENUNLOCKED;

        //case GAMEPROGRESS_UNLOCK_BLUESPHERES: return STR_BLUESPHERESUNLOCKED;

        //default: return Localization::FeatureUnimplemented;
    }*/

    // Remove this when the above is uncommented
    return Localization::FeatureUnimplemented;
}

int32 GameProgress::CountUnreadNotifs()
{
    if (sceneInfo->inEditor || API::Storage::GetNoSave() || globals->saveLoaded != STATUS_OK) {
        LogHelpers::Print("WARNING GameProgress Attempted to count unread notifs before loading SaveGame file");
        return 0;
    }
    else {
        int32 unreadCount     = 0;
        GameProgress *progressRAM = GetProgressRAM();
        for (int32 i = 0; i < UnlockCount; ++i) {
            bool32 unlocked = progressRAM->unreadNotifs[i];
            bool32 notif    = GameProgress::CheckUnlock(i);

            if (!unlocked && notif)
                unreadCount++;
        }

        return unreadCount;
    }
}
int32 GameProgress::GetNextNotif()
{
    if (sceneInfo->inEditor || API::Storage::GetNoSave() || globals->saveLoaded != STATUS_OK) {
        LogHelpers::Print("WARNING GameProgress Attempted to get next unread notif before loading SaveGame file");
        return -1;
    }
    else {
        GameProgress *progressRAM = GetProgressRAM();
        for (int32 i = 0; i < UnlockCount; ++i) {
            bool32 unlocked = progressRAM->unreadNotifs[i];
            bool32 notif    = GameProgress::CheckUnlock(i);

            if (!unlocked && notif)
                return i;
        }
    }

    return -1;
}

bool32 GameProgress::CheckUnlock(uint8 id)
{
    if (sceneInfo->inEditor || API::Storage::GetNoSave() || globals->saveLoaded != STATUS_OK) {
        LogHelpers::Print("WARNING GameProgress Attempted to check unlock before loading SaveGame file");
        return false;
    }
    else {
        GameProgress *progressRAM = GetProgressRAM();
        /*switch (id) {
            case GAMEPROGRESS_UNLOCK_TIMEATTACK:
            case GAMEPROGRESS_UNLOCK_COMPETITION: return progress->zoneCleared[0];

            case GAMEPROGRESS_UNLOCK_PEELOUT: return progress->silverMedalCount >= 1;

            case GAMEPROGRESS_UNLOCK_INSTASHIELD: return progress->silverMedalCount >= 6;

            case GAMEPROGRESS_UNLOCK_ANDKNUX: return progress->silverMedalCount >= 11;

            case GAMEPROGRESS_UNLOCK_DEBUGMODE: return progress->silverMedalCount >= 16;

            case GAMEPROGRESS_UNLOCK_MEANBEAN: return progress->silverMedalCount >= 21;

            case GAMEPROGRESS_UNLOCK_DAGARDEN: return progress->silverMedalCount >= 26;

            case GAMEPROGRESS_UNLOCK_BLUESPHERES: return progress->silverMedalCount >= GAMEPROGRESS_MEDAL_COUNT;

            default: return false;
        }*/

        return false;
    }
}

#if RETRO_REV0U
void GameProgress::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(GameProgress); }
#endif

} // namespace GameLogic