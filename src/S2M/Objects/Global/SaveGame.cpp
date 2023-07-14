// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SaveGame Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "SaveGame.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "StarPost.hpp"
#include "ItemBox.hpp"
#include "ActClear.hpp"
#include "Menu/UISaveSlot.hpp"
#include "Helpers/GameProgress.hpp"
#include "Helpers/TimeAttackData.hpp"

#include "Helpers/LogHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_STATIC_VARS(SaveGame);

void SaveGame::StageLoad() { SaveGame::LoadSaveData(); }

void SaveGame::LoadSaveData()
{
    int32 slot = globals->saveSlotID;

    sVars->saveRAM = GetSaveDataPtr(slot);

    SaveRAM *saveRAM = sVars->saveRAM;
    if (!saveRAM->lives)
        saveRAM->lives = 3;

    while (saveRAM->score1UP <= saveRAM->score) saveRAM->score1UP += 50000;

    if (Player::sVars) {
        Player::sVars->savedLives    = saveRAM->lives;
        Player::sVars->savedScore    = saveRAM->score;
        Player::sVars->savedScore1UP = saveRAM->score1UP;
        globals->continues           = saveRAM->continues;
        globals->stock               = saveRAM->stock;
        globals->characterFlags      = saveRAM->characterFlags;
    }

    if (Zone::sVars && sceneInfo->activeCategory < 2) {
        Zone::sVars->prevListPos = Zone::sVars->listPos;
        Zone::sVars->listPos     = sceneInfo->listPos;
    }

    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

    if (globals->recallEntities) {
        if (sceneInfo->activeCategory < 3) {
            RecallCollectedEntities();
        }
    }
    else if (!Zone::sVars || Zone::sVars->listPos != Zone::sVars->prevListPos) {
        if (StarPost::sVars) {
            for (int32 p = 0; p < PLAYER_COUNT; ++p) {
                StarPost::sVars->playerPositions[p].x = 0;
                StarPost::sVars->playerPositions[p].y = 0;
                StarPost::sVars->playerDirections[p]  = FLIP_NONE;
                StarPost::sVars->postIDs[p]           = 0;
            }

            StarPost::sVars->storedMilliseconds      = 0;
            StarPost::sVars->storedSeconds = 0;
            StarPost::sVars->storedMinutes = 0;
        }

        globals->tempMilliseconds = 0;
        globals->tempSeconds      = 0;
        globals->tempMinutes      = 0;
    }
}

void SaveGame::RecallCollectedEntities()
{
    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

    for (int32 p = 0; p < PLAYER_COUNT; ++p) {
        StarPost::sVars->playerPositions[p].x = globals->restartPos[p].x;
        StarPost::sVars->playerPositions[p].y = globals->restartPos[p].y;
        StarPost::sVars->playerDirections[p]  = globals->restartDir[p];
        StarPost::sVars->postIDs[p]           = globals->restartSlot[p];
    }

    StarPost::sVars->storedMilliseconds = globals->restartMilliseconds;
    StarPost::sVars->storedSeconds      = globals->restartSeconds;
    StarPost::sVars->storedMinutes      = globals->restartMinutes;

    sceneInfo->milliseconds = globals->tempMilliseconds;
    sceneInfo->seconds      = globals->tempSeconds;
    sceneInfo->minutes      = globals->tempMinutes;

    Player::sVars->savedScore    = globals->restartScore;
    Player::sVars->rings         = globals->restartRings;
    Player::sVars->ringExtraLife = globals->restart1UP;
    Player::sVars->powerups      = globals->restartPowerups;
    globals->restartRings        = 0;
    globals->restart1UP          = 100;
    globals->restartPowerups     = 0;

    LogHelpers::Print("RecallCollectedEntities");

    int32 *atlEntityData = (int32 *)globals->atlEntityData;
    for (int32 e = RESERVE_ENTITY_COUNT; e < RESERVE_ENTITY_COUNT + SCENEENTITY_COUNT; ++e) {
        switch (atlEntityData[(0x200 * 1) + e]) {
            default:
            case SaveGame::RecallNormal: break;

            case SaveGame::RecallDisabled: {
                Entity *entity  = GameObject::Get<Entity>(e);
                entity->classID = TYPE_NONE;
                entity->active  = ACTIVE_DISABLED;
                break;
            }

            case SaveGame::RecallBrokenItemBox: {
                ItemBox *itemBox = GameObject::Get<ItemBox>(e);
                itemBox->boxAnimator.SetAnimation(ItemBox::sVars->aniFrames, 1, true, 0);
                itemBox->overlayAnimator.SetAnimation(nullptr, 0, true, 0);
                itemBox->debrisAnimator.SetAnimation(nullptr, 0, true, 0);
                itemBox->contentsAnimator.SetAnimation(nullptr, 0, true, 0);
                itemBox->state.Set(&ItemBox::State_Done);
                break;
            }
        }
    }

    globals->recallEntities      = false;
    globals->restartMilliseconds = 0;
    globals->restartSeconds      = 0;
    globals->restartMinutes      = 0;
    memset(globals->atlEntityData, 0, SCENEENTITY_COUNT * sizeof(int32));
}

void SaveGame::LoadFileCB(int32 status)
{
    bool32 success = false;
    if (status == STATUS_OK || status == STATUS_NOTFOUND) {
        success             = true;
        globals->saveLoaded = STATUS_OK;
    }
    else {
        success             = false;
        globals->saveLoaded = STATUS_ERROR;
    }

    if (sVars->loadCallback) {
        void *store = sceneInfo->entity;
        if (sVars->loadEntityPtr)
            sceneInfo->entity = sVars->loadEntityPtr;

        sVars->loadCallback(success);
        sceneInfo->entity = store;

        sVars->loadCallback  = nullptr;
        sVars->loadEntityPtr = nullptr;
    }
}

void SaveGame::SaveFileCB(int32 status)
{
    if (sVars->saveCallback) {
        void *store = sceneInfo->entity;
        if (sVars->saveEntityPtr)
            sceneInfo->entity = sVars->saveEntityPtr;

        sVars->saveCallback(status == STATUS_OK);

        sceneInfo->entity = store;

        sVars->saveCallback  = nullptr;
        sVars->saveEntityPtr = nullptr;
    }
}

SaveGame::SaveRAM *SaveGame::GetSaveDataPtr(uint8 saveSlot)
{
    if (saveSlot == NO_SAVE_SLOT)
        return (SaveRAM *)globals->noSaveSlot;

    int32 slot = saveSlot + 8;
    if (saveSlot >= 0)
        slot = saveSlot;

    int32 slotPos = saveSlot * 0x100;

    return (SaveRAM *)&globals->saveRAM[0x000 + slotPos]; // 0x000 -> 0x800 => 8 save slots
}

bool32 SaveGame::CheckDisableRestart()
{
    return false;
}

SaveGame::SaveRAM *SaveGame::GetSaveRAM() { return sVars->saveRAM; }

void SaveGame::LoadFile(void (*callback)(bool32 success))
{
    if (!sVars->saveRAM || globals->saveLoaded == STATUS_CONTINUE) {
        if (callback)
            callback(false);
        return;
    }

    if (globals->saveLoaded == STATUS_OK) {
        if (callback)
            callback(false);
        return;
    }

    globals->saveLoaded  = STATUS_CONTINUE;
    sVars->loadEntityPtr = sceneInfo->entity;
    sVars->loadCallback  = callback;
    API::Storage::LoadUserFile("SaveData.bin", globals->saveRAM, sizeof(globals->saveRAM), SaveGame::LoadFileCB);
}

void SaveGame::SaveFile(void (*callback)(bool32 success))
{
    if (!sVars->saveRAM || globals->saveLoaded != STATUS_OK) {
        if (callback)
            callback(false);
    }
    else {
        sVars->saveEntityPtr = sceneInfo->entity;
        sVars->saveCallback  = callback;
        API::Storage::SaveUserFile("SaveData.bin", globals->saveRAM, sizeof(globals->saveRAM), SaveGame::SaveFileCB, false);
    }
}

void SaveGame::SaveLoadedCB(bool32 success)
{
    LogHelpers::Print("SaveLoadedCB(%d)", success);

    if (success) {
        for (auto entity : GameObject::GetEntities<UISaveSlot>(FOR_ALL_ENTITIES))
        {
            if (!entity->type) {
                Entity *store = (Entity *)sceneInfo->entity;

                entity->LoadSaveInfo();
                entity->HandleSaveIcons();

                sceneInfo->entity = store;
            }
        }

        GameProgress::DumpProgress();
    }

    if ((globals->taTableID == -1 || globals->taTableLoaded != STATUS_OK) && globals->taTableLoaded != STATUS_CONTINUE)
        TimeAttackData::LoadDB(nullptr);
}

void SaveGame::SaveProgress()
{
    SaveRAM *saveRAM = GetSaveRAM();

    saveRAM->lives          = Player::sVars->savedLives;
    saveRAM->score          = Player::sVars->savedScore;
    saveRAM->score1UP       = Player::sVars->savedScore1UP;
    saveRAM->continues      = globals->continues;
    saveRAM->characterFlags = globals->characterFlags;
    saveRAM->stock          = globals->stock;
    saveRAM->playerID       = globals->playerID;

    if (!ActClear::sVars || ActClear::sVars->displayedActID <= 0) {
        if (globals->saveSlotID != NO_SAVE_SLOT) {
            if (Zone::CurrentStageSaveable()) {
                if (saveRAM->zoneID < Zone::CurrentID() + 1)
                    saveRAM->zoneID = Zone::CurrentID() + 1;

                // if (saveRAM->zoneID >= ZONE_ERZ) {
                //     saveRAM->saveState = SAVEGAME_COMPLETE;
                //     saveRAM->zoneID    = ZONE_ERZ;
                // }
            }
        }
    }

}

void SaveGame::ClearNoSave()
{
    memset(globals->noSaveSlot, 0, sizeof(globals->noSaveSlot));
    globals->continues = 0;
}

void SaveGame::ClearSaveSlot(uint8 slotID, void (*callback)(bool32 success))
{
    SaveRAM *saveSlot = GetSaveDataPtr(slotID);
    memset(saveSlot, 0, 0x400);

    if (sVars->saveRAM && globals->saveLoaded == STATUS_OK) {
        sVars->saveEntityPtr = sceneInfo->entity;
        sVars->saveCallback  = callback;
        API::Storage::SaveUserFile("SaveData.bin", &globals->saveRAM, sizeof(globals->saveRAM), SaveGame::SaveFileCB, false);
    }
    else if (callback) {
        callback(false);
    }
}

void SaveGame::ClearRestartData()
{
    globals->recallEntities      = 0;
    globals->restartMilliseconds = 0;
    globals->restartSeconds      = 0;
    globals->restartMinutes      = 0;
    memset(globals->atlEntityData, 0, (RESERVE_ENTITY_COUNT + SCENEENTITY_COUNT) * sizeof(int32));
}

void SaveGame::StoreStageState() {

    SaveRAM *saveRAM        = GetSaveRAM();
    globals->recallEntities = true;

    for (int32 p = 0; p < PLAYER_COUNT; ++p) {
        globals->restartPos[p].x = StarPost::sVars->playerPositions[p].x;
        globals->restartPos[p].y = StarPost::sVars->playerPositions[p].y;
        globals->restartDir[p]   = StarPost::sVars->playerDirections[p];
        globals->restartSlot[p]  = StarPost::sVars->postIDs[p];
    }

    Player *player1              = GameObject::Get<Player>(SLOT_PLAYER1);
    globals->restartMilliseconds = StarPost::sVars->storedMilliseconds;
    globals->restartSeconds      = StarPost::sVars->storedSeconds;
    globals->restartMinutes      = StarPost::sVars->storedMinutes;
    globals->tempMinutes         = sceneInfo->milliseconds;
    globals->tempSeconds         = sceneInfo->seconds;
    globals->tempMinutes         = sceneInfo->minutes;

    saveRAM->lives           = player1->lives;
    globals->restartLives[0] = player1->lives;
    saveRAM->continues       = globals->continues;
    saveRAM->playerID        = globals->playerID;
    saveRAM->characterFlags  = globals->characterFlags;
    saveRAM->stock           = globals->stock;
    saveRAM->score           = player1->score;
    globals->restartScore    = player1->score;
    saveRAM->score1UP        = player1->score1UP;
    globals->restartScore1UP = player1->score1UP;
    globals->restartRings    = player1->rings;
    globals->restart1UP      = player1->ringExtraLife;
    globals->restartPowerups = player1->shield | (player1->hyperRing << 6);

    int32 *atlEntityData = (int32 *)globals->atlEntityData;
    for (int32 i = RESERVE_ENTITY_COUNT; i < RESERVE_ENTITY_COUNT + SCENEENTITY_COUNT; ++i) {
        ItemBox *itemBox = GameObject::Get<ItemBox>(i);

        atlEntityData[(0x200 * 1) + i] = SaveGame::RecallNormal;
        if (!itemBox->classID && itemBox->active == ACTIVE_DISABLED)
            atlEntityData[(0x200 * 1) + i] = SaveGame::RecallDisabled;
        else if (itemBox->classID == ItemBox::sVars->classID && itemBox->state.Matches(&ItemBox::State_Done))
            atlEntityData[(0x200 * 1) + i] = SaveGame::RecallBrokenItemBox;
    }
}

void SaveGame::SavePlayerState()
{
    SaveRAM *saveRAM = GetSaveRAM();
    Player *player1   = GameObject::Get<Player>(SLOT_PLAYER1);

    saveRAM->lives           = player1->lives;
    globals->restartLives[0] = player1->lives;
    saveRAM->continues       = globals->continues;
    saveRAM->playerID        = globals->playerID;
    saveRAM->characterFlags  = globals->characterFlags;
    saveRAM->stock           = globals->stock;
    saveRAM->score           = player1->score;
    globals->restartScore    = player1->score;
    saveRAM->score1UP        = player1->score1UP;
    globals->restartScore1UP = player1->score1UP;
    globals->restartRings    = player1->rings;
    globals->restart1UP      = player1->ringExtraLife;
    globals->restartPowerups = player1->shield | (player1->hyperRing << 6);
}

void SaveGame::LoadPlayerState()
{
    sceneInfo->milliseconds = globals->restartMilliseconds;
    sceneInfo->seconds      = globals->restartSeconds;
    sceneInfo->minutes      = globals->restartMinutes;

    Player::sVars->rings         = globals->restartRings;
    Player::sVars->ringExtraLife = globals->restart1UP;
    Player::sVars->powerups      = globals->restartPowerups;

    globals->restartRings    = 0;
    globals->restart1UP      = 100;
    globals->restartPowerups = 0;
}
void SaveGame::ResetPlayerState()
{
    globals->restartMilliseconds = 0;
    globals->restartSeconds      = 0;
    globals->restartMinutes      = 0;
    globals->restartRings        = 0;
    globals->restart1UP          = 0;
    globals->restartPowerups     = 0;

    if (Player::sVars) {
        Player::sVars->rings         = globals->restartRings;
        Player::sVars->ringExtraLife = globals->restart1UP;
        Player::sVars->powerups      = globals->restartPowerups;
    }
}

void SaveGame::LoadGameState()
{
    SaveRAM *saveRAM = GetSaveRAM();
}

void SaveGame::SaveGameState()
{
    SaveRAM *saveRAM = GetSaveRAM();

    if (saveRAM->saveState != 2) {
        Player *player = GameObject::Get<Player>(SLOT_PLAYER1);

        saveRAM->zoneID    = Zone::CurrentID();
        saveRAM->score     = player->score;
        saveRAM->lives     = player->lives;
        saveRAM->continues = globals->continues;

        SaveFile(nullptr);
    }
}

bool32 SaveGame::AllChaosEmeralds() // added this for the special ring to work properly lol, this is only for chaos emeralds not super but shouldnt matter really
{
    SaveRAM *saveRAM = SaveGame::GetSaveRAM();
    return saveRAM->collectedEmeralds == 0b01111111;
}

bool32 SaveGame::GetEmeralds(EmeraldCheckTypes type)
{
    if (type > SaveGame::EmeraldAny2)
        return false;

    SaveRAM *saveRAM = GetSaveRAM();

    int32 totalCount = 0;
    switch (type) {
        case SaveGame::EmeraldNone:
        case SaveGame::EmeraldChaosOnly:
        case SaveGame::EmeraldSuperOnly:
        case SaveGame::EmeraldUnused:
        case SaveGame::EmeraldUnused2:
        case SaveGame::EmeraldBoth:
        case SaveGame::EmeraldSuper:
        case SaveGame::EmeraldAny: totalCount = 7; break;

        case SaveGame::EmeraldChaosOnly2:
        case SaveGame::EmeraldSuperOnly2:
        case SaveGame::EmeraldBoth2:
        case SaveGame::EmeraldSuper2:
        case SaveGame::EmeraldAny2: totalCount = 8; break;
    }

    uint32 emeralds = saveRAM->collectedEmeralds;
    switch (type) {
        case SaveGame::EmeraldNone: return emeralds == 0;

        case SaveGame::EmeraldChaosOnly:
        case SaveGame::EmeraldChaosOnly2: {
            int32 emeraldCount = 0;
            for (int32 e = 0; e < totalCount; ++e) {
                uint32 emerald = emeralds >> e;
                // check if we have the chaos AND NOT the super emerald
                if ((emerald & 1) && !(emerald & 0x100))
                    emeraldCount++;
            }

            return emeraldCount == totalCount;
        }

        case SaveGame::EmeraldSuperOnly:
        case SaveGame::EmeraldSuperOnly2: {
            int32 emeraldCount = 0;
            for (int32 e = 0; e < totalCount; ++e) {
                uint32 emerald = emeralds >> e;
                // check if we DONT'T have the chaos AND DO have the super emerald
                if (!(emerald & 1) && (emerald & 0x100))
                    emeraldCount++;
            }
            return emeraldCount == totalCount;
        }

        case SaveGame::EmeraldBoth:
        case SaveGame::EmeraldBoth2: {
            int32 emeraldCount = 0;
            for (int32 e = 0; e < totalCount; ++e) {
                uint32 emerald = emeralds >> e;

                // check if we have the chaos AND super emerald
                if ((emerald & 1) && (emerald & 0x100))
                    emeraldCount++;
            }
            return emeraldCount == totalCount;
        }

        case SaveGame::EmeraldSuper:
        case SaveGame::EmeraldSuper2: {
            int32 emeraldCount = 0;
            for (int32 e = 0; e < totalCount; ++e) {
                uint32 emerald = emeralds >> e;

                // check if we have the super emerald (doesn't care about chaos emerald)
                if ((emerald & 0x100))
                    emeraldCount++;
            }

            return emeraldCount == totalCount;
        }

        case SaveGame::EmeraldAny:
        case SaveGame::EmeraldAny2: {
            int32 emeraldCount = 0;
            for (int32 e = 0; e < totalCount; ++e) {
                // check if we have the chaos OR super emerald
                if (((emeralds >> e) & 1) || ((emeralds >> e) & 0x100))
                    ++emeraldCount;
            }
            return emeraldCount == totalCount;
        }

        default: break;
    }

    return false;
}

void SaveGame::SetEmeralds(EmeraldCheckTypes type)
{
    if (type > SaveGame::EmeraldAny2)
        return;

    SaveRAM *saveRAM = GetSaveRAM();

    int32 totalCount = 0;
    switch (type) {
        case SaveGame::EmeraldNone:
        case SaveGame::EmeraldChaosOnly:
        case SaveGame::EmeraldSuperOnly:
        case SaveGame::EmeraldUnused:
        case SaveGame::EmeraldUnused2:
        case SaveGame::EmeraldBoth:
        case SaveGame::EmeraldSuper:
        case SaveGame::EmeraldAny: totalCount = 7; break;

        case SaveGame::EmeraldChaosOnly2:
        case SaveGame::EmeraldSuperOnly2:
        case SaveGame::EmeraldBoth2:
        case SaveGame::EmeraldSuper2:
        case SaveGame::EmeraldAny2: totalCount = 8; break;
    }

    switch (type) {
        case SaveGame::EmeraldNone: saveRAM->collectedEmeralds = 0; break;

        case SaveGame::EmeraldChaosOnly:
        case SaveGame::EmeraldChaosOnly2: {
            uint32 emeralds = saveRAM->collectedEmeralds;

            // give all chaos emeralds, remove all super emeralds
            for (int32 e = 0; e < totalCount; ++e) {
                emeralds |= (1 << e) & ~(0x100 << e);
            }

            saveRAM->collectedEmeralds = emeralds;
            break;
        }

        case SaveGame::EmeraldSuperOnly:
        case SaveGame::EmeraldSuperOnly2:
        case SaveGame::EmeraldUnused:
        case SaveGame::EmeraldUnused2: {
            uint32 emeralds = saveRAM->collectedEmeralds;

            // give all super emeralds, remove all chaos emeralds
            for (int32 e = 0; e < totalCount; ++e) {
                emeralds |= (0x100 << e) & ~(1 << e);
            }

            saveRAM->collectedEmeralds = emeralds;
            break;
        }

        case SaveGame::EmeraldBoth:
        case SaveGame::EmeraldBoth2:
        case SaveGame::EmeraldSuper:
        case SaveGame::EmeraldSuper2:
        case SaveGame::EmeraldAny:
        case SaveGame::EmeraldAny2: {
            uint32 emeralds = saveRAM->collectedEmeralds;

            // give all chaos emeralds, give all super emeralds
            for (int32 e = 0; e < totalCount; ++e) {
                emeralds |= (1 << e) | (0x100 << e);
            }

            saveRAM->collectedEmeralds = emeralds;
            break;
        }

        default: break;
    }
}

bool32 SaveGame::GetEmerald(uint8 emerald) { return (sVars->saveRAM->collectedEmeralds >> emerald) & 1; }

void SaveGame::SetEmerald(uint8 emeraldID) { sVars->saveRAM->collectedEmeralds |= 1 << emeraldID; }

void SaveGame::ClearCollectedSpecialRings() { sVars->saveRAM->collectedSpecialRings = 0; }
bool32 SaveGame::GetCollectedSpecialRing(uint8 id) { return sVars->saveRAM->collectedSpecialRings & (1 << (16 * Zone::sVars->actID - 1 + id)); }
void SaveGame::SetCollectedSpecialRing(uint8 id) { sVars->saveRAM->collectedSpecialRings |= 1 << (16 * Zone::sVars->actID - 1 + id); }

#if RETRO_REV0U
void SaveGame::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(SaveGame); }
#endif

} // namespace GameLogic
