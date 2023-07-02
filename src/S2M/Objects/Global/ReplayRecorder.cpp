// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ReplayRecorder Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "ReplayRecorder.hpp"
#include "TitleCard.hpp"
#include "ActClear.hpp"
#include "Zone.hpp"
#include "PauseMenu.hpp"
#include "TimeAttackGate.hpp"
#include "HUD.hpp"
#include "Localization.hpp"
#include "Helpers/DialogRunner.hpp"
#include "Helpers/MenuParam.hpp"
#include "Helpers/TimeAttackData.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Helpers/MathHelpers.hpp"
#include "Helpers/DrawHelpers.hpp"
#include "Helpers/ReplayDB.hpp"
#include "Menu/UILoadingIcon.hpp"
#include "Menu/UIDialog.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ReplayRecorder);

void ReplayRecorder::Update()
{
    if (!this->paused) {
        this->state.Run(this);
    }
}

void ReplayRecorder::LateUpdate()
{
    if (!this->paused) {
        this->stateLate.Run(this);

        Replay *replayPtr = nullptr;
        if (RSDKTable->GetEntitySlot(this) == SLOT_REPLAYRECORDER_RECORD)
            replayPtr = sVars->recordBuffer;
        else
            replayPtr = sVars->playbackBuffer;

        if (this->replayFrame > replayPtr->header.frameCount && !this->state.Matches(nullptr))
            ReplayRecorder::Stop(this);

        if (sVars->reachedGoal && !this->player->playerID && this->state.Matches(&ReplayRecorder::State_Record)) {
            if (this->replayStopDelay <= 0) {
                if (!this->replayStopDelay) {
                    ReplayRecorder::Stop(this);
                    ReplayRecorder::Rewind(this);
                    this->replayStopDelay = -1;
                }
            }
            else {
                this->replayStopDelay--;
            }
        }
    }
}

void ReplayRecorder::StaticUpdate()
{
    if (globals->gameMode == MODE_TIMEATTACK) {
        if (sceneInfo->state & ENGINESTATE_REGULAR)
            ++sVars->frameCounter;

        if (Zone::sVars) {
            if (!sVars->initialized) {
                TitleCard::sVars->finishedCB.Set(&ReplayRecorder::TitleCardCB);
                TimeAttackGate::sVars->startCB.Set(&ReplayRecorder::StartCB);
                TimeAttackGate::sVars->endCB.Set(&ReplayRecorder::FinishCB);
                ActClear::sVars->bufferMove_CB.Set(&ReplayRecorder::Buffer_Move);
                ActClear::sVars->saveReplay_CB.Set(&ReplayRecorder::SaveReplay);
                sVars->initialized = true;
            }

            if (sVars->startRecording) {
                Player *player = GameObject::Get<Player>(SLOT_PLAYER1);
                ReplayRecorder::StartRecording(player);
                sVars->startRecording   = false;
                sVars->startedRecording = true;
            }

            if (sVars->startPlayback) {
                Player *player = nullptr;
                if (Player::sVars->playerCount <= 1) {
                    MenuParam *param = MenuParam::GetMenuParam();

                    player = GameObject::Get<Player>(SLOT_PLAYER1);
                    API::Achievements::SetEnabled(false);
                    API::Stats::SetEnabled(false);
                    TimeAttackGate::sVars->disableRecords = true;
                    TimeAttackData::sVars->personalRank   = param->replayRankID;
                }                
                else {
                    player = GameObject::Get<Player>(SLOT_PLAYER2);
                }

                ReplayRecorder *playbackManager = sVars->playbackManager;
                ReplayRecorder::Rewind(playbackManager);
                ReplayRecorder::Play(player);
                sVars->startPlayback = false;
                sVars->isReplaying   = true;
            }

            ReplayRecorder *recordingManager = sVars->recordingManager;
            Player *recordingPlayer          = recordingManager->player;
            if (recordingPlayer) {
                recordingManager->prevPlayerState = recordingPlayer->state;
                recordingManager->storedAnim      = recordingPlayer->animator.animationID;
                recordingManager->storedFrame     = recordingPlayer->animator.frameID;
            }

            ReplayRecorder *playbackManager = sVars->playbackManager;
            Player *playbackPlayer          = playbackManager->player;
            if (playbackManager->isGhostPlayback && playbackPlayer)
                playbackPlayer->state.Set(&ReplayRecorder::PlayerState_PlaybackReplay);

            if (!playbackManager->isGhostPlayback) {
                if ((controllerInfo->keyStart.press || Unknown_pausePress) && sceneInfo->state == ENGINESTATE_REGULAR) {
                    PauseMenu *pauseMenu = GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);

                    bool32 allowPause = true;
                    if (ActClear::sVars && ActClear::sVars->actClearActive)
                        allowPause = false;

                    if (!RSDKTable->GetEntityCount(TitleCard::sVars->classID, false) && !pauseMenu->classID && allowPause) {
                        GameObject::Reset(SLOT_PAUSEMENU, PauseMenu::sVars->classID, nullptr);
                        pauseMenu->triggerPlayer = RSDKTable->GetEntitySlot(sceneInfo->entity);
                    }
                }
            }
        }
    }
}

void ReplayRecorder::Draw()
{
    if (this->isGhostPlayback)
        ReplayRecorder::DrawGhostDisplay();
}

void ReplayRecorder::Create(void *data)
{
    this->active          = ACTIVE_NEVER;
    this->drawGroup       = 13;
    this->inkEffect       = INK_NONE;
    this->replayStopDelay = -1;
    this->drawFX          = FX_FLIP;
    this->ghostAlpha      = 0x100;
    this->visible         = globals->gameMode == MODE_TIMEATTACK;
}

void ReplayRecorder::StageLoad()
{
    sVars->replayID    = 0;
    sVars->replayRowID = -1;

    if (globals->gameMode == MODE_TIMEATTACK) {
        sVars->active = ACTIVE_ALWAYS;

        sVars->frameCounter      = -1;
        sVars->savedReplay       = 0;
        sVars->initialized       = false;
        sVars->startedRecording  = false;
        sVars->isReplaying       = false;
        sVars->hasSetupGhostVS   = false;
        sVars->hasSetupGhostView = false;
        sVars->passedStartLine   = false;
        sVars->reachedGoal       = false;
        sVars->packedStartFrame  = false;
        sVars->loadCallback      = nullptr;
        sVars->fileBuffer        = nullptr;

        memset(sVars->filename, 0, sizeof(sVars->filename));
        sVars->recordBuffer    = (Replay *)globals->replayWriteBuffer;
        sVars->recordingFrames = sVars->recordBuffer->frames;
        sVars->playbackBuffer  = (Replay *)globals->replayReadBuffer;
        sVars->playbackFrames  = sVars->playbackBuffer->frames;

        if (Zone::sVars) {
            if (!sceneInfo->inEditor)
                ReplayRecorder::SetupActions();

            Math::SetRandSeed(1624633040);

            GameObject::Reset(SLOT_REPLAYRECORDER_RECORD, sVars->classID, nullptr);
            ReplayRecorder *recordingManager = GameObject::Get<ReplayRecorder>(SLOT_REPLAYRECORDER_RECORD);
            recordingManager->maxFrameCount        = REPLAY_MAX_FRAMECOUNT;
            sVars->recordingManager       = recordingManager;

            GameObject::Reset(SLOT_REPLAYRECORDER_PLAYBACK, sVars->classID, nullptr);
            ReplayRecorder *playbackManager = GameObject::Get<ReplayRecorder>(SLOT_REPLAYRECORDER_PLAYBACK);
            playbackManager->maxFrameCount        = REPLAY_MAX_FRAMECOUNT;
            sVars->playbackManager       = playbackManager;

            sVars->startRecording = false;
            sVars->startPlayback  = false;

            Replay *replayPtr = nullptr;
            if (RSDKTable->GetEntitySlot(sVars->playbackManager) == SLOT_REPLAYRECORDER_RECORD)
                replayPtr = sVars->recordBuffer;
            else
                replayPtr = sVars->playbackBuffer;

            MenuParam *param = MenuParam::GetMenuParam();
            if (param->viewReplay && replayPtr->header.isNotEmpty) {
                if (param->showGhost) {
                    globals->playerID        = GET_CHARACTER_ID(1) | (GET_CHARACTER_ID(1) << 8);
                    Player::sVars->configureGhostCB.Set(&ReplayRecorder::ConfigureGhost_CB);
                }
                else {
                    globals->playerID &= 0xFF;
                }
            }

            LogHelpers::Print("CharID: %08x", globals->playerID);
        }
    }
}

void ReplayRecorder::TitleCardCB(void)
{
    Replay *buffer = nullptr;
    if (RSDKTable->GetEntitySlot(sVars->playbackManager) == SLOT_REPLAYRECORDER_RECORD)
        buffer = sVars->recordBuffer;
    else
        buffer = sVars->playbackBuffer;

    if (Player::sVars->playerCount != 1 || !buffer->header.isNotEmpty)
        sVars->startRecording = true;
    if (buffer->header.isNotEmpty)
        sVars->startPlayback = true;
}

void ReplayRecorder::Resume(ReplayRecorder *recorder)
{
    LogHelpers::Print("ReplayRecorder::Resume()");
    recorder->paused          = false;
    recorder->player->visible = true;
}

void ReplayRecorder::StartCB()
{
    if (sVars->startedRecording)
        sVars->recordingManager->changeFlags = 2; // Passed Gate

    ReplayRecorder *recorder = sVars->playbackManager;
    if (sVars->isReplaying && recorder->isGhostPlayback) {
        if (sVars->packedStartFrame)
            ReplayRecorder::Resume(recorder);
        else
            ReplayRecorder::SeekFunc(recorder);
    }

    sVars->passedStartLine = true;
}

void ReplayRecorder::FinishCB()
{
    ReplayRecorder *recorder = sVars->recordingManager;
    if (sVars->startedRecording)
        recorder->replayStopDelay = 120;

    sVars->reachedGoal = true;
}

void ReplayRecorder::Buffer_Move()
{
    Replay *replayPtr = nullptr;
    if (RSDKTable->GetEntitySlot(sVars->recordingManager) == SLOT_REPLAYRECORDER_RECORD)
        replayPtr = sVars->recordBuffer;
    else
        replayPtr = sVars->playbackBuffer;

    if (replayPtr->header.isNotEmpty) {
        if (replayPtr->header.frameCount < sVars->recordingManager->maxFrameCount - 1) {
            memset(globals->replayTempWBuffer, 0, sizeof(globals->replayTempWBuffer));
            LogHelpers::Print("Buffer_Move(0x%08x, 0x%08x)", globals->replayTempWBuffer, replayPtr);
            memcpy(globals->replayTempWBuffer, replayPtr, sizeof(globals->replayTempWBuffer));
            memset(replayPtr, 0, sizeof(globals->replayWriteBuffer));
            ReplayRecorder::Buffer_PackInPlace(globals->replayTempWBuffer);
            HUD::sVars->replaySaveEnabled = true;
        }
    }
}

void ReplayRecorder::SaveReplayDLG_NoCB()
{
    HUD::sVars->replaySaveEnabled = true;

    ActClear::sVars->hasSavedReplay      = false;
    ActClear::sVars->disableResultsInput = false;
}

void ReplayRecorder::SaveReplayDLG_YesCB()
{
    sVars->replayID    = 0;
    sVars->replayRowID = -1;

    int32 mins      = sceneInfo->minutes;
    int32 secs      = sceneInfo->seconds;
    int32 millisecs = sceneInfo->milliseconds;
    LogHelpers::Print("Bout to create ReplayDB entry...");

    MenuParam *param = MenuParam::GetMenuParam();
    int32 rowID            = ReplayDB::AddReplay(param->zoneID, param->actID, param->characterID, millisecs + 100 * (secs + 60 * mins));
    if (rowID == -1) {
        LogHelpers::Print("Table row ID invalid! %d", -1);
        ReplayRecorder::SaveFile_Replay(false);
    }
    else {
        sVars->replayID    = APITable->GetUserDBRowUUID(globals->replayTableID, rowID);
        sVars->replayRowID = rowID;

        char fileName[0x20];
        sprintf_s(fileName, (int32)sizeof(fileName), "Replay_%08X.bin", sVars->replayID);
        LogHelpers::Print("Replay Filename: %s", fileName);
        UILoadingIcon::StartWait();

        sVars->savedReplay = true;
        ReplayRecorder::Buffer_SaveFile(fileName, globals->replayTempWBuffer, ReplayRecorder::SaveFile_Replay);
        HUD::sVars->replaySaveEnabled = false;
    }
}

void ReplayRecorder::SaveReplayDLG_CB()
{
    String message = {};

    Localization::GetString(&message, Localization::SaveReplay);

    Action<void> callbackYes;
    callbackYes.Set(&ReplayRecorder::SaveReplayDLG_YesCB);

    Action<void> callbackNo;
    callbackNo.Set(&ReplayRecorder::SaveReplayDLG_NoCB);

    UIDialog::CreateDialogYesNo(&message, callbackYes, callbackNo, true, true);
}

void ReplayRecorder::SaveReplay()
{
    Replay *replayPtr = (Replay *)globals->replayTempWBuffer;

    if (replayPtr->header.isNotEmpty) {
        LogHelpers::Print("Saving replay...");

        for (int32 i = 0; i < SCENEENTITY_COUNT; ++i) {
            HUD *hud = GameObject::Get<HUD>(i);
            hud->replayClapAnimator.SetAnimation(HUD::sVars->aniFrames, 11, true, 0);
            HUD::sVars->sfxClick.Play(false, 0xFF);
        }

        Action<void> handleCallback;
        handleCallback.Set(&DialogRunner::HandleCallback);

        DialogRunner *callback = GameObject::Create<DialogRunner>(&handleCallback, 0, 0);
        callback->state.Set(&ReplayRecorder::SaveReplayDLG_CB);
        callback->timer              = 45;
        callback->isPermanent        = true;
    }
    else {
        LogHelpers::Print("Can't save replay! No data available");
        ActClear::sVars->disableResultsInput = false;
    }
}

void ReplayRecorder::SaveFile_Replay(bool32 success)
{
    if (success) {
        LogHelpers::Print("Replay save successful!");
        ReplayDB::SaveDB(&ReplayRecorder::SaveCallback_ReplayDB);
    }
    else {
        if (sVars->replayRowID != -1)
            APITable->RemoveDBRow(globals->replayTableID, sVars->replayRowID);

        String message = {};
        Localization::GetString(&message, Localization::NoReplaySpace);

        Action<void> callback;
        callback.Set(nullptr);

        UIDialog::CreateDialogOk(&message, callback, true);

        UILoadingIcon::FinishWait();

        ActClear::sVars->disableResultsInput = false;
        ActClear::sVars->hasSavedReplay      = false;
        HUD::sVars->replaySaveEnabled        = true;
    }
}

void ReplayRecorder::SaveCallback_ReplayDB(bool32 success)
{
    if (success) {
        if (TimeAttackData::sVars->rowID == -1) {
            UILoadingIcon::FinishWait();
            ActClear::sVars->disableResultsInput = false;
            for (auto hud : GameObject::GetEntities<HUD>(FOR_ALL_ENTITIES))
            {
                hud->replayClapAnimator.SetAnimation(HUD::sVars->aniFrames, 10, true, 0);
                break;
            }
            HUD::sVars->sfxStarpost.Play(false, 255);
            HUD::sVars->replaySaveEnabled = false;
        }
        else {
            APITable->SetUserDBValue(globals->taTableID, TimeAttackData::sVars->rowID, API::Storage::UserDB::UInt32, "replayID", &sVars->replayID);
            TimeAttackData::SaveDB(&ReplayRecorder::SaveCallback_TimeAttackDB);
        }
    }
    else {
        String message = {};

        char fileName[0x20];
        sprintf_s(fileName, (int32)sizeof(fileName), "Replay_%08X.bin", sVars->replayID);

        if (sVars->replayRowID != -1)
            APITable->RemoveDBRow(globals->replayTableID, sVars->replayRowID);
        APITable->DeleteUserFile(fileName, nullptr);

        Localization::GetString(&message, Localization::NoReplaySpace);

        Action<void> callback;
        callback.Set(nullptr);

        UIDialog::CreateDialogOk(&message, callback, true);

        UILoadingIcon::FinishWait();

        ActClear::sVars->disableResultsInput = false;
        ActClear::sVars->hasSavedReplay      = false;
        HUD::sVars->replaySaveEnabled        = true;
    }
}

void ReplayRecorder::SaveCallback_TimeAttackDB(bool32 success)
{
    UILoadingIcon::FinishWait();

    ActClear::sVars->disableResultsInput = false;
    if (!success) {
        ActClear::sVars->hasSavedReplay = false;
        HUD::sVars->replaySaveEnabled   = true;
    }
    else {
        for (auto hud : GameObject::GetEntities<HUD>(FOR_ALL_ENTITIES))
        {
            hud->replayClapAnimator.SetAnimation(HUD::sVars->aniFrames, 10, true, 0);
            break;
        }
        HUD::sVars->sfxStarpost.Play(false, 255);

        HUD::sVars->replaySaveEnabled = false;
    }
}

void ReplayRecorder::Buffer_PackInPlace(int32 *tempWriteBuffer)
{
    Replay *replayPtr = (Replay *)tempWriteBuffer;

    LogHelpers::Print("Buffer_PackInPlace(%08x)", tempWriteBuffer);

    if (replayPtr->header.signature == REPLAY_SIGNATURE) {
        if (replayPtr->header.isPacked) {
            LogHelpers::Print("Buffer_Ppack ERROR: Buffer is already packed");
        }
        else {
            int32 compressedSize   = sizeof(ReplayHeader);
            int32 uncompressedSize = sizeof(ReplayFrame) * (replayPtr->header.frameCount + 2);

            ReplayFrame *framePtr   = replayPtr->frames;
            uint8 *compressedFrames = (uint8 *)replayPtr->frames;
            for (int32 f = 0; f < replayPtr->header.frameCount; ++f) {
                ReplayFrame uncompressedFrame;
                memcpy(&uncompressedFrame, framePtr, sizeof(ReplayFrame));

                memset(framePtr, 0, sizeof(ReplayFrame));

                int32 size = ReplayDB::Buffer_PackEntry(compressedFrames, &uncompressedFrame);
                compressedFrames += size;
                compressedSize += size;
                framePtr++;
            }
            LogHelpers::Print("Packed %d frames: %luB -> %luB", replayPtr->header.frameCount, uncompressedSize, compressedSize);

            replayPtr->header.bufferSize = compressedSize;
            replayPtr->header.isPacked   = true;
        }
    }
    else {
        LogHelpers::Print("Buffer_Pack ERROR: Signature does not match");
    }
}

void ReplayRecorder::Buffer_Unpack(int32 *readBuffer, int32 *tempReadBuffer)
{
    LogHelpers::Print("Buffer_Unpack(0x%08x, 0x%08x)", readBuffer, tempReadBuffer);
    Replay *replayPtr     = (Replay *)readBuffer;
    Replay *tempReplayPtr = (Replay *)tempReadBuffer;

    uint8 *compressedFrames = (uint8 *)tempReplayPtr->frames;
    if ((uint32)*tempReadBuffer == REPLAY_SIGNATURE) {
        if (tempReplayPtr->header.isPacked) {
            int32 compressedSize            = tempReplayPtr->header.bufferSize;
            replayPtr->header.signature     = tempReplayPtr->header.signature;
            replayPtr->header.version       = tempReplayPtr->header.version;
            replayPtr->header.isPacked      = tempReplayPtr->header.isPacked;
            replayPtr->header.isNotEmpty    = tempReplayPtr->header.isNotEmpty;
            replayPtr->header.frameCount    = tempReplayPtr->header.frameCount;
            replayPtr->header.startingFrame = tempReplayPtr->header.startingFrame;
            int32 uncompressedSize          = sizeof(ReplayFrame) * (tempReplayPtr->header.frameCount + 2);
            ReplayFrame *uncompressedBuffer = replayPtr->frames;

            for (int32 i = 0; i < tempReplayPtr->header.frameCount; ++i) {
                int32 size = ReplayDB::Buffer_UnpackEntry(uncompressedBuffer, compressedFrames);
                compressedFrames += size;
                uncompressedBuffer++;
            }
            LogHelpers::Print("Unpacked %d frames: %luB -> %luB", tempReplayPtr->header.frameCount, compressedSize, uncompressedSize);

            replayPtr->header.isPacked   = false;
            replayPtr->header.bufferSize = uncompressedSize;
            memset(tempReadBuffer, 0, sizeof(globals->replayTempRBuffer));
        }
        else {
            LogHelpers::Print("Buffer_Unpack ERROR: Buffer is not packed");
        }
    }
    else {
        LogHelpers::Print("Buffer_Unpack ERROR: Signature does not match");
    }
}

void ReplayRecorder::Buffer_SaveFile(const char *fileName, int32 *buffer, void (*callback)(bool32 success))
{
    LogHelpers::Print("Buffer_SaveFile(%s, %08x)", fileName, buffer);

    Replay *replayPtr = (Replay *)buffer;
    if (replayPtr->header.isNotEmpty) {
        sVars->saveCallback = callback;
        APITable->SaveUserFile(fileName, buffer, replayPtr->header.bufferSize, ReplayRecorder::SaveReplayCallback, true);
    }
    else {
        LogHelpers::Print("Attempted to save an empty replay buffer");
        if (callback)
            callback(false);
    }
}

void ReplayRecorder::SaveReplayCallback(int32 status)
{
    if (sVars->saveCallback)
        sVars->saveCallback(status == STATUS_OK);

    sVars->saveCallback = nullptr;
}

void ReplayRecorder::Buffer_LoadFile(const char *fileName, void *buffer, void (*callback)(bool32 success))
{
    LogHelpers::Print("Buffer_LoadFile(%s, %08x)", fileName, buffer);

    memset(buffer, 0, sizeof(globals->replayReadBuffer));
    sVars->fileBuffer   = buffer;
    sVars->loadCallback = callback;
    strcpy(sVars->filename, fileName);

    API::Storage::LoadUserFile(fileName, buffer, sizeof(globals->replayReadBuffer), ReplayRecorder::LoadReplayCallback);
}

void ReplayRecorder::LoadReplayCallback(int32 status)
{
    if (sVars->loadCallback)
        sVars->loadCallback(status == STATUS_OK);

    sVars->loadCallback = nullptr;
    sVars->fileBuffer   = nullptr;
    memset(sVars->filename, 0, sizeof(sVars->filename));
}

void ReplayRecorder::ConfigureGhost_CB()
{
    Player *player = (Player *)this;

    LogHelpers::Print("ConfigureGhost_CB()");
    LogHelpers::Print("Ghost Slot %d", player->playerID);

    LogHelpers::PrintVector2("Ghost pos ", player->position);
    player->isGhost        = true;
    player->stateInput.Set(nullptr);
    player->state.Set(&ReplayRecorder::PlayerState_PlaybackReplay);
    player->sidekick       = false;
    player->interaction    = false;
    player->tileCollisions = TILECOLLISION_NONE;
    player->visible        = true;
    player->alpha          = 0xFF;
}

void ReplayRecorder::SetupActions()
{
    for (int32 i = 0; i < 64; ++i) sVars->actions[i].Set(nullptr);

    sVars->actions[3].Set(nullptr); //Current_PlayerState_Down;
    sVars->actions[4].Set(nullptr); //Current_PlayerState_Left;
    sVars->actions[5].Set(nullptr); //Current_PlayerState_Right;
    sVars->actions[6].Set(nullptr); //Current_PlayerState_Up;

    sVars->actions[7].Set(nullptr); //Cylinder_PlayerState_InkRoller_Stand;
    sVars->actions[8].Set(nullptr); //Cylinder_PlayerState_InkRoller_Roll;
    sVars->actions[9].Set(nullptr); //Cylinder_PlayerState_Pillar;
    sVars->actions[10].Set(nullptr); //Cylinder_PlayerState_Spiral;

    sVars->actions[13].Set(nullptr); //GymBar_PlayerState_Hang;
    sVars->actions[14].Set(nullptr); //GymBar_PlayerState_SwingV;
    sVars->actions[15].Set(nullptr); //GymBar_PlayerState_SwingH;

    sVars->actions[16].Set(nullptr); //Ice_PlayerState_Frozen

    sVars->actions[17].Set(nullptr); //OOZSetup_PlayerState_OilFall;
    sVars->actions[18].Set(nullptr); //OOZSetup_PlayerState_OilPool;
    sVars->actions[19].Set(nullptr); //OOZSetup_PlayerState_OilSlide;
    sVars->actions[20].Set(nullptr); //OOZSetup_PlayerState_OilStrip;

    sVars->actions[21].Set(nullptr);
    sVars->actions[22].Set(nullptr);

    sVars->actions[23].Set(&Player::State_Air);
    sVars->actions[24].Set(&Player::State_BubbleBounce);
    sVars->actions[25].Set(&Player::State_FlyCarried);
    sVars->actions[26].Set(&Player::State_KnuxWallClimb);
    sVars->actions[27].Set(&Player::State_Crouch);
    sVars->actions[28].Set(&Player::State_Death);
    sVars->actions[29].Set(nullptr);
    sVars->actions[30].Set(&Player::State_DropDash);
    sVars->actions[31].Set(&Player::State_Drown);
    sVars->actions[32].Set(&Player::State_TailsFlight);
    sVars->actions[33].Set(&Player::State_FlyToPlayer);
    sVars->actions[34].Set(&Player::State_KnuxGlideDrop);
    sVars->actions[35].Set(&Player::State_KnuxGlideLeft);
    sVars->actions[36].Set(&Player::State_KnuxGlideRight);
    sVars->actions[37].Set(&Player::State_KnuxGlideSlide);
    sVars->actions[38].Set(&Player::State_Ground);
    sVars->actions[39].Set(&Player::State_HoldRespawn);
    sVars->actions[40].Set(&Player::State_Hurt);
    sVars->actions[41].Set(&Player::State_KnuxLedgePullUp);
    sVars->actions[42].Set(&Player::State_LookUp);
    sVars->actions[43].Set(nullptr);
    sVars->actions[44].Set(&Player::State_Peelout);
    sVars->actions[45].Set(&Player::State_Roll);
    sVars->actions[46].Set(&Player::State_Spindash);
    sVars->actions[47].Set(&Player::State_StartSuper);
    sVars->actions[48].Set(&Player::State_Static);
    sVars->actions[49].Set(&Player::State_Transform);
    sVars->actions[50].Set(&Player::State_TransportTube);
    sVars->actions[51].Set(&Player::State_TubeAirRoll);
    sVars->actions[52].Set(&Player::State_TubeRoll);
    sVars->actions[53].Set(&Player::State_Victory);

    sVars->actions[54].Set(nullptr);
    sVars->actions[55].Set(nullptr);
    sVars->actions[56].Set(nullptr);
}

void ReplayRecorder::SetupWriteBuffer()
{
    MenuParam *param = MenuParam::GetMenuParam();
    Replay *replayPtr      = sVars->recordBuffer;

    replayPtr->header.signature     = REPLAY_SIGNATURE;
    replayPtr->header.version       = GAME_VERSION;
    replayPtr->header.isPacked      = false;
    replayPtr->header.isNotEmpty    = true;
    replayPtr->header.startingFrame = sVars->frameCounter;
    replayPtr->header.zoneID        = param->zoneID;
    replayPtr->header.act           = param->actID;
    replayPtr->header.characterID   = param->characterID;
    replayPtr->header.oscillation   = Zone::sVars->timer;
    replayPtr->header.bufferSize    = sizeof(ReplayHeader);

    LogHelpers::Print("characterID = %d", replayPtr->header.characterID);
    LogHelpers::Print("zoneID = %d", replayPtr->header.zoneID);
    LogHelpers::Print("act = %d", replayPtr->header.act);
    LogHelpers::Print("oscillation = %d", replayPtr->header.oscillation);
}

void ReplayRecorder::DrawGhostDisplay()
{
    Player *player = this->player;

    if (this->state.Matches(nullptr)) {
        if (this->ghostAlpha > 0)
            this->ghostAlpha -= 4;
    }

    if (!sceneInfo->currentScreenID && this->ghostAlpha) {
        this->inkEffect = INK_NONE;
        Vector2 screen;
        screen.x   = (screenInfo->position.x + screenInfo->center.x) << 16;
        screen.y   = (screenInfo->position.y + screenInfo->center.y) << 16;

        Hitbox hitbox;
        hitbox.left   = -(screenInfo->size.x >> 1);
        hitbox.top    = -(screenInfo->size.y >> 1);
        hitbox.right  = screenInfo->size.x >> 1;
        hitbox.bottom = screenInfo->size.y >> 1;

        if (!MathHelpers::PointInHitbox(screen.x, screen.y, player->position.x, player->position.y, FLIP_NONE, &hitbox)) {
            // Draw Player Preview (when ghost is off screen)
            Vector2 drawPos;
            drawPos.x = 0;
            drawPos.y = 0;

            int32 dist = CLAMP(MathHelpers::Distance(screen, player->position) >> 16, 100, 2000);

            int32 size    = 12 - 4 * (3 * dist - 300) / 2000;
            hitbox.right  = hitbox.right - 8 - (size + 24);
            hitbox.bottom = hitbox.bottom - 8 - (size + 24);
            hitbox.top += size + 24 + 8;
            hitbox.left += size + 24 + 8;

            Vector2 screenPos;
            screenPos.x = screen.x;
            screenPos.y = screen.y;
            if (MathHelpers::ConstrainToBox(&drawPos, player->position.x, player->position.y, screenPos, hitbox)) {
                int32 angle = Math::ATan2(player->position.x - drawPos.x, player->position.y - drawPos.y);
                int32 x     = ((size + 18) * Math::Cos256(angle) << 8) + drawPos.x;
                int32 y     = ((size + 18) * Math::Sin256(angle) << 8) + drawPos.y;
                // Draw Direction Arrow
                DrawHelpers::DrawIsocelesTriangle(x, y, (Math::Cos256(angle) << 10) + x, (Math::Sin256(angle) << 10) + y, 4, 0xC0E0E0, INK_ALPHA,
                                                 this->ghostAlpha >> 1);

                this->alpha     = this->ghostAlpha;
                this->inkEffect = INK_ALPHA;
                this->drawFX    = FX_FLIP | FX_ROTATE | FX_SCALE;
                this->scale.x   = 0x100;
                this->scale.y   = 0x100;
                if (!player->tailFrames.Matches(nullptr)) {
                    // Draw Tail Preview
                    this->rotation   = player->tailRotation;
                    this->direction  = player->tailDirection;
                    this->velocity.x = player->velocity.x;
                    this->velocity.y = player->velocity.y;
                    player->tailAnimator.DrawSprite(&drawPos, false);
                }

                // Draw Player Preview
                this->rotation  = player->rotation;
                this->direction = player->direction;
                player->animator.DrawSprite(&drawPos, false);

                this->drawFX     = FX_NONE;
                this->alpha      = 0xFF;
                this->velocity.x = 0;
                this->velocity.y = 0;
            }
        }
    }
}

void ReplayRecorder::Record(ReplayRecorder *recorder, Player *player)
{
    LogHelpers::Print("ReplayRecorder::Record()");

    if (player)
        recorder->player = player;

    recorder->prevPlayerState = player->state;
    recorder->state.Set(&ReplayRecorder::State_Record);
    recorder->stateLate.Set(&ReplayRecorder::Late_RecordFrames);
    recorder->storedAnim      = player->animator.animationID;
    recorder->storedFrame     = player->animator.frameID;
}

void ReplayRecorder::StartRecording(Player *player)
{
    ReplayRecorder *recorder = sVars->recordingManager;
    LogHelpers::Print("ReplayRecorder::StartRecording()");

    recorder->active = ACTIVE_NORMAL;
    memset(globals->replayTempWBuffer, 0, sizeof(globals->replayTempWBuffer));
    memset(globals->replayWriteBuffer, 0, sizeof(globals->replayWriteBuffer));

    ReplayRecorder::Rewind(recorder);
    ReplayRecorder::SetupWriteBuffer();
    ReplayRecorder::Record(recorder, player);
}

void ReplayRecorder::Play(Player *player)
{
    LogHelpers::Print("ReplayRecorder::Play()");
    ReplayRecorder *recorder = sVars->playbackManager;

    Replay *replayPtr = nullptr;
    if (RSDKTable->GetEntitySlot(recorder) == SLOT_REPLAYRECORDER_RECORD)
        replayPtr = sVars->recordBuffer;
    else
        replayPtr = sVars->playbackBuffer;

    if (replayPtr->header.isNotEmpty) {
        recorder->active = ACTIVE_NORMAL;
        if (player) {
            recorder->player           = player;
            recorder->ghostPlayerState = player->state;
        }

        recorder->isGhostPlayback = false;
        if (player->playerID) {
            recorder->isGhostPlayback       = true;
            sVars->hasSetupGhostVS = true;
        }
        else {
            Zone::sVars->timer              = replayPtr->header.oscillation;
            player->stateInputReplay.Set(&ReplayRecorder::PlayBackInput);
            player->controllerID     = Input::CONT_P2;
            Input::AssignInputSlotToDevice(Input::CONT_P2, Input::INPUT_UNASSIGNED);
            sVars->hasSetupGhostView = true;
        }

        recorder->state.Set(&ReplayRecorder::State_SetupPlayback);
        recorder->stateLate.Set(nullptr);
        // Set the dim timer to dim after 15 mins instead of the usual 5
        Graphics::SetVideoSetting(VIDEOSETTING_DIMTIMER, 15 * 60 * 60);
    }
    else {
        LogHelpers::Print("No replay to play");
    }
}

void ReplayRecorder::Rewind(ReplayRecorder *recorder)
{
    LogHelpers::Print("ReplayRecorder::Rewind()");

    recorder->replayFrame = 0;
}

void ReplayRecorder::Seek(ReplayRecorder *recorder, uint32 frame)
{
    LogHelpers::Print("ReplayRecorder::Seek(%u)", frame);

    recorder->replayFrame = frame;

    ReplayFrame *frameBuffer = nullptr;
    if (RSDKTable->GetEntitySlot(recorder) == SLOT_REPLAYRECORDER_RECORD)
        frameBuffer = sVars->recordingFrames;
    else
        frameBuffer = sVars->playbackFrames;

    int32 newFrame = frame;

    ReplayFrame *framePtr = &frameBuffer[frame];
    while (framePtr->info != REPLAY_INFO_STATECHANGE) {
        if (framePtr->info == REPLAY_INFO_PASSEDGATE)
            break;
        if (newFrame > (int32)frame)
            break;
        framePtr--;
        --newFrame;
    }

    if (newFrame <= (int32)frame) {
        ReplayRecorder::ForceApplyFramePtr(recorder, framePtr);
        if (newFrame < (int32)frame) {
            int32 count      = frame - newFrame;
            ReplayFrame *ptr = &frameBuffer[frame];
            for (int32 i = 0; i < count; ++i) {
                ptr++;
                ReplayRecorder::ApplyFramePtr(recorder, ptr);
            }
        }
    }
}

void ReplayRecorder::SeekFunc(ReplayRecorder *recorder)
{
    ReplayFrame *framePtr = nullptr;
    if (RSDKTable->GetEntitySlot(recorder) == SLOT_REPLAYRECORDER_RECORD)
        framePtr = sVars->recordingFrames;
    else
        framePtr = sVars->playbackFrames;

    for (int32 f = 0; f < recorder->maxFrameCount; ++f) {
        if (framePtr[f].info == REPLAY_INFO_PASSEDGATE) {
            ReplayRecorder::Seek(recorder, f);
            break;
        }
    }
}

void ReplayRecorder::Stop(ReplayRecorder *recorder)
{
    LogHelpers::Print("ReplayRecorder::Stop()");

    recorder->state.Set(nullptr);
    recorder->stateLate.Set(nullptr);

    Player *player = recorder->player;
    if (player) {
        if (player->stateInputReplay.Matches(&ReplayRecorder::PlayBackInput))
            player->stateInputReplay.Set(nullptr);
    }
}

void ReplayRecorder::SetGimmickState(ReplayRecorder *recorder, bool32 allowSpriteChanges)
{
    Player *player = recorder->player;
    if (player) {
        player->tailFrames.Init();

        if (Stage::CheckSceneFolder("MMZ") || Stage::CheckSceneFolder("PSZ2")) {
            if (allowSpriteChanges) {
                if (Stage::CheckSceneFolder("MMZ")) {
                    /*switch (player->characterID) {
                        default:
                        case ID_SONIC: player->aniFrames = SizeLaser->sonicFrames; break;

                        case ID_TAILS:
                            player->aniFrames  = SizeLaser->tailsFrames;
                            player->tailFrames = SizeLaser->tailFrames;
                            break;

                        case ID_KNUCKLES: player->aniFrames = SizeLaser->knuxFrames; break;
                    }*/
                }
                else if (Stage::CheckSceneFolder("PSZ2")) {
                    //player->aniFrames = Ice->aniFrames;
                }
            }
            else {
                switch (player->characterID) {
                    default:
                    case ID_SONIC: player->aniFrames = Player::sVars->sonicFrames; break;

                    case ID_TAILS:
                        player->aniFrames  = Player::sVars->tailsFrames;
                        player->tailFrames = Player::sVars->tailsTailsFrames;
                        break;

                    case ID_KNUCKLES: player->aniFrames = Player ::sVars->knuxFrames; break;
                }
            }
        }
    }
}

void ReplayRecorder::ForceApplyFramePtr(ReplayRecorder *recorder, ReplayFrame *framePtr)
{
    Player *player = recorder->player;
    if (player) {
        player->position.x = framePtr->position.x & 0xFFFF0000;
        player->position.y = framePtr->position.y & 0xFFFF0000;
        player->velocity.x = framePtr->velocity.x & 0xFFFF0000;
        player->velocity.y = framePtr->velocity.y & 0xFFFF0000;
        player->direction  = framePtr->direction;
        player->rotation   = framePtr->rotation;

        ReplayRecorder::SetGimmickState(recorder, (framePtr->changedValues & REPLAY_CHANGED_GIMMICK) != 0);
        player->animator.SetAnimation(player->aniFrames, framePtr->anim, true, framePtr->frame);
        player->animator.speed = 0;
    }
}

void ReplayRecorder::ApplyFramePtr(ReplayRecorder *recorder, ReplayFrame *framePtr)
{
    Player *player = recorder->player;
    if (player) {
        if (framePtr->changedValues & REPLAY_CHANGED_POS) {
            player->position.x = framePtr->position.x & 0xFFFF0000;
            player->position.y = framePtr->position.y & 0xFFFF0000;
        }

        if (framePtr->changedValues & REPLAY_CHANGED_VEL) {
            player->velocity.x = framePtr->velocity.y & 0xFFFF0000;
            player->velocity.y = framePtr->velocity.y & 0xFFFF0000;
        }

        if (framePtr->changedValues & REPLAY_CHANGED_ROT)
            player->rotation = framePtr->rotation;

        if (framePtr->changedValues & REPLAY_CHANGED_DIR)
            player->direction = framePtr->direction;

        ReplayRecorder::SetGimmickState(recorder, (framePtr->changedValues & REPLAY_CHANGED_GIMMICK) != 0);

        if (framePtr->changedValues & REPLAY_CHANGED_ANIM)
            player->animator.SetAnimation(player->aniFrames, framePtr->anim, true, framePtr->frame);
        else if (framePtr->changedValues & REPLAY_CHANGED_FRAME)
            player->animator.SetAnimation(player->aniFrames, player->animator.animationID, true, framePtr->frame);

        player->animator.speed = 0;
    }
}

bool32 ReplayRecorder::CheckPlayerGimmickState(ReplayRecorder *recorder)
{
    Player *player = recorder->player;

    if (!player || (!Stage::CheckSceneFolder("MMZ") && !Stage::CheckSceneFolder("PSZ2")))
        return false;

    if (Stage::CheckSceneFolder("MMZ"))
        return player->isChibi;

    if (Stage::CheckSceneFolder("PSZ2"))
        return player->state.Set(nullptr);
        //return player->state == Ice_PlayerState_Frozen;

    return false;
}

void ReplayRecorder::PackFrame(ReplayFrame *recording)
{
    uint8 buffer[sizeof(ReplayFrame)];
    memset(&buffer, 0, sizeof(ReplayFrame));

    Replay *replayPtr        = sVars->recordBuffer;
    ReplayFrame *frameBuffer = sVars->recordingFrames;

    int32 size = ReplayDB::Buffer_PackEntry(buffer, recording);
    memcpy(&frameBuffer[this->replayFrame], recording, sizeof(ReplayFrame));

    if (replayPtr->header.frameCount) {
        uint32 frameCount                  = replayPtr->header.frameCount;
        float avg                          = replayPtr->header.averageFrameSize;
        float sizef                        = size;
        replayPtr->header.averageFrameSize = ((avg * frameCount) + sizef) / (frameCount + 1);
    }
    else {
        replayPtr->header.averageFrameSize = size;
    }

    replayPtr->header.bufferSize += size;

    ++this->replayFrame;
    ++replayPtr->header.frameCount;
}

void ReplayRecorder::PlayBackInput()
{
    Player *player           = (Player *)this;
    ReplayRecorder *recorder = sVars->playbackManager;

    Replay *replayPtr = nullptr;
    if (RSDKTable->GetEntitySlot(recorder) == SLOT_REPLAYRECORDER_RECORD)
        replayPtr = sVars->recordBuffer;
    else
        replayPtr = sVars->playbackBuffer;

    if (sVars->frameCounter >= replayPtr->header.startingFrame && player == recorder->player) {
        ReplayFrame *frameBuffer = nullptr;
        if (RSDKTable->GetEntitySlot(recorder) == SLOT_REPLAYRECORDER_RECORD)
            frameBuffer = sVars->recordingFrames;
        else
            frameBuffer = sVars->playbackFrames;
        ReplayFrame *framePtr = &frameBuffer[recorder->replayFrame];

        bool32 setPos = false;
        if (framePtr->info) {
            bool32 forceChange = framePtr->info == REPLAY_INFO_STATECHANGE || framePtr->info == REPLAY_INFO_PASSEDGATE;

            setPos          = forceChange || (framePtr->info == REPLAY_INFO_USEFLAGS && (framePtr->changedValues & REPLAY_CHANGED_POS));
            bool32 setInput = forceChange || (framePtr->info == REPLAY_INFO_USEFLAGS && (framePtr->changedValues & REPLAY_CHANGED_INPUT));
            if (setInput) {
                int32 inputs                                     = framePtr->inputs;
                controllerInfo[player->controllerID].keyUp.down    = (inputs & 0x01) != 0;
                controllerInfo[player->controllerID].keyDown.down  = (inputs & 0x02) != 0;
                controllerInfo[player->controllerID].keyLeft.down  = (inputs & 0x04) != 0;
                controllerInfo[player->controllerID].keyRight.down = (inputs & 0x08) != 0;
                controllerInfo[player->controllerID].keyA.press    = (inputs & 0x10) != 0;
                controllerInfo[player->controllerID].keyA.down     = (inputs & 0x20) != 0;
            }
        }

        ControllerState *controller = &controllerInfo[player->controllerID];
        if (controller->keyUp.down)
            controller->keyUp.press = !controller->keyUp.press;
        else
            controller->keyUp.press = false;

        if (controller->keyDown.down)
            controller->keyDown.press = !controller->keyDown.press;
        else
            controller->keyDown.press = false;

        if (controller->keyLeft.down)
            controller->keyLeft.press = !controller->keyLeft.press;
        else
            controller->keyLeft.press = false;

        if (controller->keyRight.down)
            controller->keyRight.press = !controller->keyRight.press;
        else
            controller->keyRight.press = false;

        if (setPos) {
            recorder->storedPos.x = framePtr->position.x;
            recorder->storedPos.y = framePtr->position.y;
        }
    }
}

void ReplayRecorder::Pause(ReplayRecorder *recorder)
{
    LogHelpers::Print("ReplayRecorder::Pause()");

    recorder->paused = true;
}

void ReplayRecorder::PlayerState_PlaybackReplay()
{
    Player *player = (Player *)this;

    ReplayRecorder *recorder = sVars->playbackManager;
    if (recorder->isGhostPlayback) {
        player->animator.speed = 0;

        ReplayFrame *frameBuffer = nullptr;
        if (RSDKTable->GetEntitySlot(recorder) == SLOT_REPLAYRECORDER_RECORD)
            frameBuffer = sVars->recordingFrames;
        else
            frameBuffer = sVars->playbackFrames;

        ReplayFrame *framePtr = &frameBuffer[recorder->replayFrame];
        if (!recorder->state.Matches(nullptr)) {
            if (framePtr->info == REPLAY_INFO_USEFLAGS) {
                ReplayRecorder::ApplyFramePtr(recorder, framePtr);
            }
            else if (framePtr->info == REPLAY_INFO_STATECHANGE || framePtr->info == REPLAY_INFO_PASSEDGATE) {
                ReplayRecorder::ForceApplyFramePtr(recorder, framePtr);
                if (framePtr->info == REPLAY_INFO_PASSEDGATE && !sVars->packedStartFrame) {
                    sVars->packedStartFrame = true;
                    if (!sVars->passedStartLine)
                        ReplayRecorder::Pause(recorder);
                }
            }
        }
        else {
            player->velocity.x = 0;
            player->velocity.y = 0;
        }

        if (sVars->packedStartFrame && !sVars->passedStartLine) {
            player->visible = ~(player->timer >> 2) & 1;
            player->timer++;
        }
    }
}

void ReplayRecorder::State_SetupPlayback()
{
    Replay *replayPtr = nullptr;
    if (RSDKTable->GetEntitySlot(this) == SLOT_REPLAYRECORDER_RECORD)
        replayPtr = sVars->recordBuffer;
    else
        replayPtr = sVars->playbackBuffer;

    ReplayFrame *frameBuffer = nullptr;
    if (RSDKTable->GetEntitySlot(this) == SLOT_REPLAYRECORDER_RECORD)
        frameBuffer = sVars->recordingFrames;
    else
        frameBuffer = sVars->playbackFrames;

    if (sVars->frameCounter >= replayPtr->header.startingFrame) {
        if (sVars->frameCounter != replayPtr->header.startingFrame) {
            if (!this->isGhostPlayback)
                ReplayRecorder::Seek(this, sVars->frameCounter - replayPtr->header.startingFrame);
            this->replayFrame = sVars->frameCounter - replayPtr->header.startingFrame;
        }

        this->state.Set(&ReplayRecorder::State_Playback);
        this->stateLate.Set(&ReplayRecorder::Late_Playback);
    }
    else if (this->isGhostPlayback) {
        ReplayRecorder::ForceApplyFramePtr(this, frameBuffer);
    }
}

void ReplayRecorder::State_Playback() {}

void ReplayRecorder::State_Record() {}

void ReplayRecorder::Late_Playback()
{
    Player *player = this->player;
    if (!this->replayFrame) {
        Camera *camera = player->camera;
        if (camera) {
            camera->position.x                      = player->position.x;
            camera->position.y                      = player->position.y;
            screenInfo[camera->screenID].position.x = (camera->position.x >> 16);
            screenInfo[camera->screenID].position.y = (camera->position.y >> 16);

            camera->state.Set(&Camera::State_FollowXY);
            player->scrollDelay = 0;
        }
    }

    if (this->isGhostPlayback) {
        if (!player->state.Matches(&ReplayRecorder::PlayerState_PlaybackReplay))
            this->ghostPlayerState = player->state;
    }

    ReplayFrame *frameBuffer = nullptr;
    if (RSDKTable->GetEntitySlot(this) == SLOT_REPLAYRECORDER_RECORD)
        frameBuffer = sVars->recordingFrames;
    else
        frameBuffer = sVars->playbackFrames;

    ReplayFrame *framePtr = &frameBuffer[this->replayFrame];

    if (!this->isGhostPlayback && framePtr->info) {
        if ((framePtr->info == REPLAY_INFO_STATECHANGE || framePtr->info == REPLAY_INFO_PASSEDGATE)
            || (framePtr->info == REPLAY_INFO_USEFLAGS && (framePtr->changedValues & REPLAY_CHANGED_POS))) {
            if (MathHelpers::Distance(framePtr->position, player->position) >= 0x20000) {
                player->position.x += (framePtr->position.x - player->position.x) >> 1;
                player->position.y += (framePtr->position.y - player->position.y) >> 1;
            }
        }
    }

    ++this->replayFrame;
}

void ReplayRecorder::Late_RecordFrames()
{
    ReplayFrame frame;

    if (this->replayFrame < this->maxFrameCount - 1) {
        Player *player = this->player;
        memset(&frame, 0, sizeof(frame));

        ControllerState *controller = &controllerInfo[player->controllerID];
        int32 inputState                = 0;
        if (controller->keyUp.down || analogStickInfoL[player->controllerID].keyUp.down)
            inputState |= 0x01;
        if (controller->keyDown.down || analogStickInfoL[player->controllerID].keyDown.down)
            inputState |= 0x02;
        if (controller->keyLeft.down || analogStickInfoL[player->controllerID].keyLeft.down)
            inputState |= 0x04;
        if (controller->keyRight.down || analogStickInfoL[player->controllerID].keyRight.down)
            inputState |= 0x08;
        if (controller->keyA.press || controller->keyB.press || controller->keyC.press || controller->keyX.press)
            inputState |= 0x10;
        if (controller->keyA.down || controller->keyB.down || controller->keyC.down || controller->keyX.down)
            inputState |= 0x20;

        bool32 isGimmickState = ReplayRecorder::CheckPlayerGimmickState(this);
        this->curPlayerState  = player->state;

        Animator *animator = &player->animator;
        //if (isGimmickState && RSDK.CheckSceneFolder("PSZ2") && player->state == Ice_PlayerState_Frozen && ice->classID == Ice->classID)
        //    animator = &ice->contentsAnimator;

        this->animID  = animator->animationID;
        this->frameID = animator->frameID;

        // Not sure what this is for, since it doesn't do anything
        for (int32 i = 0; i < 64; ++i) {
            if (this->prevPlayerState.Matches(sVars->actions[i].state))
                break;
        }

        for (int32 i = 0; i < 64; ++i) {
            if (this->curPlayerState.Matches(sVars->actions[i].state))
                break;
        }

        // byte 0 info:
        // 0 = none
        // 1 = state change (first frame is always a state change)
        // 2 = some values changed (see byte 1 for info)
        // 3 = passed TA Gate

        // byte 1 info:
        // its flags for what values changed

        if (this->replayFrame == 0 || this->changeFlags > 0 || !this->prevPlayerState.Matches(this->curPlayerState.state)) {
            frame.info          = this->changeFlags == 2 ? REPLAY_INFO_PASSEDGATE : REPLAY_INFO_STATECHANGE;
            frame.changedValues = REPLAY_CHANGED_NONE;
            frame.inputs        = inputState;
            frame.direction     = player->direction;
            frame.rotation      = player->rotation;
            frame.position.x    = player->position.x;
            frame.position.y    = player->position.y;
            frame.velocity.x    = player->velocity.x;
            frame.velocity.y    = player->velocity.y;
            frame.anim          = this->animID;
            frame.frame         = this->frameID;
            if (isGimmickState)
                frame.changedValues = REPLAY_CHANGED_GIMMICK;

            this->changeFlags = 0;
        }
        else {
            int32 changedVals = 0;
            if (inputState != this->storedInputs) {
                frame.inputs = inputState;
                changedVals  = REPLAY_CHANGED_INPUT;
            }

            if (player->direction != this->storedDirection) {
                frame.direction = player->direction;
                changedVals |= REPLAY_CHANGED_DIR;
            }

            if (player->position.x != this->storedPos.x || player->position.y != this->storedPos.y) {
                changedVals |= REPLAY_CHANGED_POS;
                frame.position.x = player->position.x;
                frame.position.y = player->position.y;
            }

            if (player->velocity.x != this->storedVel.x || player->velocity.y != this->storedVel.y) {
                changedVals |= REPLAY_CHANGED_VEL;
                frame.velocity.x = player->velocity.x;
                frame.velocity.y = player->velocity.y;
            }

            // what ??? ??? ???
            // this is code from mania
            if (player->groundVel != this->storedRotation) {
                changedVals |= REPLAY_CHANGED_ROT;
                frame.rotation = player->rotation;
            }

            if (animator->animationID != this->storedAnim) {
                changedVals |= (REPLAY_CHANGED_ANIM | REPLAY_CHANGED_FRAME);
                frame.anim  = this->animID;
                frame.frame = this->frameID;
            }

            if (animator->frameID != this->storedFrame) {
                changedVals |= REPLAY_CHANGED_FRAME;
                frame.frame = this->frameID;
            }

            if (isGimmickState)
                changedVals |= REPLAY_CHANGED_GIMMICK;

            if (changedVals) {
                frame.info          = REPLAY_INFO_USEFLAGS;
                frame.changedValues = changedVals;
            }
            else {
                frame.info = REPLAY_INFO_NONE;
            }
        }

        ReplayRecorder::PackFrame(&frame);
        this->storedPos.x     = player->position.x;
        this->storedPos.y     = player->position.y;
        this->storedVel.x     = player->velocity.x;
        this->storedVel.y     = player->velocity.y;
        this->storedDirection = player->direction;
        this->storedSpeed     = player->groundVel;
        this->storedRotation  = player->rotation;
        this->storedInputs    = inputState;
        this->storedAnim      = animator->animationID;
        this->storedFrame     = animator->frameID;
    }
    else {
        this->state.Set(nullptr);
        this->stateLate.Set(nullptr);
    }
}

#if RETRO_INCLUDE_EDITOR
void ReplayRecorder::EditorDraw() {}

void ReplayRecorder::EditorLoad() {}
#endif

void ReplayRecorder::Serialize() {}
} // namespace GameLogic