// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ReplayDB Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "ReplayDB.hpp"
#include "LogHelpers.hpp"
#include "TimeAttackData.hpp"
#include "Global/ReplayRecorder.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ReplayDB);

void ReplayDB::Update() {}

void ReplayDB::LateUpdate() {}

void ReplayDB::StaticUpdate() {}

void ReplayDB::Draw() {}

void ReplayDB::Create(void *data) {}

void ReplayDB::StageLoad() {}

void ReplayDB::CreateDB()
{
    globals->replayTableID = APITable->InitUserDB("ReplayDB.bin", API::Storage::UserDB::UInt32, "score", API::Storage::UserDB::UInt8, "zoneID", API::Storage::UserDB::UInt8, "act", API::Storage::UserDB::UInt8,
                                            "characterID", API::Storage::UserDB::UInt32, "zoneSortVal", nullptr);

    if (globals->replayTableID == -1)
        globals->replayTableLoaded = STATUS_ERROR;
    else
        globals->replayTableLoaded = STATUS_OK;
}

void ReplayDB::LoadDB(void (*callback)(bool32 success))
{
    if ((globals->replayTableID != -1 && globals->replayTableLoaded == STATUS_OK) || globals->replayTableLoaded == STATUS_CONTINUE) {
        if (callback)
            callback(false);
    }
    else {
        LogHelpers::Print("Loading Replay DB");
        globals->replayTableLoaded = STATUS_CONTINUE;

        sVars->loadEntity      = (Entity *)sceneInfo->entity;
        sVars->loadCallback = callback;
        globals->replayTableID = APITable->LoadUserDB("ReplayDB.bin", ReplayDB::LoadDBCallback);

        if (globals->replayTableID == -1) {
            LogHelpers::Print("Couldn't claim a slot for loading %s", "ReplayDB.bin");
            globals->replayTableLoaded = STATUS_ERROR;
        }
    }
}

void ReplayDB::SaveDB(void (*callback)(bool32 success))
{
    if (API::Storage::GetNoSave() || globals->replayTableID == (uint16)-1 || globals->replayTableLoaded != STATUS_OK) {
        if (callback)
            callback(false);
    }
    else {
        LogHelpers::Print("Saving Replay DB");
        sVars->saveEntity   = (Entity *)sceneInfo->entity;
        sVars->saveCallback = callback;
        APITable->SaveUserDB(globals->replayTableID, ReplayDB::SaveDBCallback);
    }
}

uint32 ReplayDB::AddReplay(uint8 zoneID, uint8 act, uint8 characterID, int32 score)
{
    if (globals->replayTableLoaded == STATUS_OK) {
        uint32 rowID       = APITable->AddUserDBRow(globals->replayTableID);
        int32 zoneStortVal = (score & 0x3FFFFFF) | (((zoneID << 2) | (act & 1)));

        APITable->SetUserDBValue(globals->replayTableID, rowID, API::Storage::UserDB::UInt32, "score", &score);
        APITable->SetUserDBValue(globals->replayTableID, rowID, API::Storage::UserDB::UInt8, "zoneID", &zoneID);
        APITable->SetUserDBValue(globals->replayTableID, rowID, API::Storage::UserDB::UInt8, "act", &act);
        APITable->SetUserDBValue(globals->replayTableID, rowID, API::Storage::UserDB::UInt8, "characterID", &characterID);
        APITable->SetUserDBValue(globals->replayTableID, rowID, API::Storage::UserDB::UInt32, "zoneSortVal", &zoneStortVal);

        uint32 UUID = APITable->GetUserDBRowUUID(globals->replayTableID, rowID);
        char createTime[24];
        sprintf_s(createTime, (int32)sizeof(createTime), "");
        APITable->GetUserDBRowCreationTime(globals->replayTableID, rowID, createTime, sizeof(createTime) - 1, "%Y/%m/%d %H:%M:%S");

        LogHelpers::Print("Replay DB Added Entry");
        LogHelpers::Print("Created at %s", createTime);
        LogHelpers::Print("Row ID: %d", rowID);
        LogHelpers::Print("UUID: %08X", UUID);

        return rowID;
    }

    return -1;
}

void ReplayDB::DeleteReplay(int32 row, void (*callback)(bool32 success), bool32 useAltCB)
{
    int32 id       = APITable->GetUserDBRowUUID(globals->replayTableID, row);
    int32 replayID = 0;

    sVars->deleteEntity   = (Entity *)sceneInfo->entity;
    sVars->deleteCallback = callback;
    APITable->RemoveDBRow(globals->replayTableID, row);
    TimeAttackData::sVars->loaded = false;

    APITable->SetupUserDBRowSorting(globals->taTableID);
    APITable->AddRowSortFilter(globals->taTableID, API::Storage::UserDB::UInt32, "replayID", &id);

    int32 count = APITable->GetSortedUserDBRowCount(globals->taTableID);
    for (int32 i = 0; i < count; ++i) {
        uint32 uuid = APITable->GetSortedUserDBRowID(globals->taTableID, i);
        LogHelpers::Print("Deleting Time Attack replay from row #%d", uuid);
        APITable->SetUserDBValue(globals->taTableID, uuid, API::Storage::UserDB::UInt32, "replayID", &replayID);
    }

    char filename[0x20];
    sprintf_s(filename, (int32)sizeof(filename), "Replay_%08X.bin", id);
    if (!useAltCB)
        API::Storage::DeleteUserFile(filename, ReplayDB::DeleteReplay_CB);
    else
        API::Storage::DeleteUserFile(filename, ReplayDB::DeleteReplaySave2_CB);
}

void ReplayDB::DeleteReplay_CB(int32 status)
{
    LogHelpers::Print("DeleteReplay_CB(%d)", status);

    APITable->SaveUserDB(globals->replayTableID, ReplayDB::DeleteReplaySave_CB);
}

void ReplayDB::DeleteReplaySave_CB(int32 status)
{
    LogHelpers::Print("DeleteReplaySave_CB(%d)", status);

    APITable->SaveUserDB(globals->taTableID, ReplayDB::DeleteReplaySave2_CB);
}

void ReplayDB::DeleteReplaySave2_CB(int32 status)
{
    LogHelpers::Print("DeleteReplaySave2_CB(%d)", status);

    if (sVars->deleteCallback) {
        Entity *store = (Entity *)sceneInfo->entity;
        if (sVars->deleteEntity)
            sceneInfo->entity = sVars->deleteEntity;
        sVars->deleteCallback(status == STATUS_OK);
        sceneInfo->entity = store;

        sVars->deleteCallback = nullptr;
        sVars->deleteEntity   = nullptr;
    }
}

void ReplayDB::LoadDBCallback(int32 status)
{
    if (status == STATUS_OK) {
        globals->replayTableLoaded = STATUS_OK;
        APITable->SetupUserDBRowSorting(globals->replayTableID);
        LogHelpers::Print("Load Succeeded! Replay count: %d", APITable->GetSortedUserDBRowCount(globals->replayTableID));
    }
    else {
        LogHelpers::Print("Load Failed! Creating new Replay DB");
        ReplayDB::CreateDB();
    }

    LogHelpers::Print("Replay DB Slot => %d, Load Status => %d", globals->replayTableID, globals->replayTableLoaded);

    if (sVars->loadCallback) {
        Entity *store = (Entity *)sceneInfo->entity;
        if (sVars->loadEntity)
            sceneInfo->entity = sVars->loadEntity;
        sVars->loadCallback(status == STATUS_OK);
        sceneInfo->entity = store;

        sVars->loadCallback = nullptr;
        sVars->loadEntity   = nullptr;
    }
}

void ReplayDB::SaveDBCallback(int32 status)
{
    if (sVars->saveCallback) {
        Entity *store = (Entity *)sceneInfo->entity;
        if (sVars->saveEntity)
            sceneInfo->entity = sVars->saveEntity;
        sVars->saveCallback(status == STATUS_OK);
        sceneInfo->entity = store;

        sVars->saveCallback = nullptr;
        sVars->saveEntity   = nullptr;
    }
}

void ReplayDB::LoadCallback(bool32 success) {}

int32 ReplayDB::Buffer_PackEntry(uint8 *compressed, void *uncompressed)
{
    ReplayRecorder::ReplayFrame *framePtr = (ReplayRecorder::ReplayFrame *)uncompressed;

    compressed[0]    = framePtr->info;
    compressed[1]    = framePtr->changedValues;
    bool32 forcePack = framePtr->info == ReplayRecorder::REPLAY_INFO_STATECHANGE || framePtr->info == ReplayRecorder::REPLAY_INFO_PASSEDGATE;
    uint8 changes    = framePtr->changedValues;

    uint8 *compressedBuffer = &compressed[2];

    // input
    if (forcePack || (changes & ReplayRecorder::REPLAY_CHANGED_INPUT)) {
        *compressedBuffer = framePtr->inputs;
        ++compressedBuffer;
    }

    // position
    if (forcePack || (changes & ReplayRecorder::REPLAY_CHANGED_POS)) {
        *((int32 *)compressedBuffer) = framePtr->position.x;
        compressedBuffer += sizeof(int32);

        *((int32 *)compressedBuffer) = framePtr->position.y;
        compressedBuffer += sizeof(int32);
    }

    // velocity
    if (forcePack || (changes & ReplayRecorder::REPLAY_CHANGED_VEL)) {
        *((int32 *)compressedBuffer) = framePtr->velocity.x;
        compressedBuffer += sizeof(int32);

        *((int32 *)compressedBuffer) = framePtr->velocity.y;
        compressedBuffer += sizeof(int32);
    }

    // rotation
    if (forcePack || (changes & ReplayRecorder::REPLAY_CHANGED_ROT)) {
        *compressedBuffer = framePtr->rotation >> 1;
        compressedBuffer += sizeof(uint8);
    }

    // direction
    if (forcePack || (changes & ReplayRecorder::REPLAY_CHANGED_DIR)) {
        *compressedBuffer = framePtr->direction;
        ++compressedBuffer;
    }

    // anim
    if (forcePack || (changes & ReplayRecorder::REPLAY_CHANGED_ANIM)) {
        *compressedBuffer = framePtr->anim;
        ++compressedBuffer;
    }

    // frame
    if (forcePack || (changes & ReplayRecorder::REPLAY_CHANGED_FRAME)) {
        *compressedBuffer = framePtr->frame;
        ++compressedBuffer;
    }

    return (int32)(compressedBuffer - compressed);
}

int32 ReplayDB::Buffer_UnpackEntry(void *uncompressed, uint8 *compressed)
{
    ReplayRecorder::ReplayFrame *framePtr = (ReplayRecorder::ReplayFrame *)uncompressed;

    // compress state
    framePtr->info = compressed[0];

    bool32 forceUnpack      = *compressed == ReplayRecorder::REPLAY_INFO_STATECHANGE || *compressed == ReplayRecorder::REPLAY_INFO_PASSEDGATE;
    uint8 changes           = compressed[1];
    framePtr->changedValues = changes;

    uint8 *compressedBuffer = &compressed[2];

    // input
    if (forceUnpack || (changes & ReplayRecorder::REPLAY_CHANGED_INPUT)) {
        framePtr->inputs = *compressedBuffer++;
    }

    // position
    if (forceUnpack || (changes & ReplayRecorder::REPLAY_CHANGED_POS)) {
        int32 x = *(int32 *)compressedBuffer;
        compressedBuffer += sizeof(int32);

        int32 y = *(int32 *)compressedBuffer;
        compressedBuffer += sizeof(int32);

        framePtr->position.x = x;
        framePtr->position.y = y;
    }

    // velocity
    if (forceUnpack || (changes & ReplayRecorder::REPLAY_CHANGED_VEL)) {
        int32 x = *(int32 *)compressedBuffer;
        compressedBuffer += sizeof(int32);

        int32 y = *(int32 *)compressedBuffer;
        compressedBuffer += sizeof(int32);

        framePtr->velocity.x = x;
        framePtr->velocity.y = y;
    }

    // rotation
    if (forceUnpack || (changes & ReplayRecorder::REPLAY_CHANGED_ROT)) {
        int32 rotation     = *compressedBuffer++;
        framePtr->rotation = rotation << 1;
    }

    // direction
    if (forceUnpack || (changes & ReplayRecorder::REPLAY_CHANGED_DIR)) {
        framePtr->direction = *compressedBuffer++;
    }

    // anim
    if (forceUnpack || (changes & ReplayRecorder::REPLAY_CHANGED_ANIM)) {
        framePtr->anim = *compressedBuffer++;
    }

    // frame
    if (forceUnpack || (changes & ReplayRecorder::REPLAY_CHANGED_FRAME)) {
        framePtr->frame = *compressedBuffer++;
    }

    return (int32)(compressedBuffer - compressed);
}

#if RETRO_INCLUDE_EDITOR
void ReplayDB::EditorDraw() {}

void ReplayDB::EditorLoad() {}
#endif

void ReplayDB::Serialize() {}

} // namespace GameLogic