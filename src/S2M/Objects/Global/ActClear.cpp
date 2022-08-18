// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ActClear Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "ActClear.hpp"
#include "Zone.hpp"
#include "SaveGame.hpp"
#include "DebugMode.hpp"
#include "Helpers/GameProgress.hpp"
#include "Helpers/MenuParam.hpp"
#include "Helpers/TimeAttackData.hpp"
#include "HUD.hpp"
#include "Music.hpp"
#include "Animals.hpp"
#include "StarPost.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ActClear);

void ActClear::Update() { this->state.Run(this); }
void ActClear::LateUpdate() {}
void ActClear::StaticUpdate()
{
    if (sVars->victoryTimer)
        sVars->victoryTimer--;
}
void ActClear::Draw()
{
    Vector2 drawPos;
    Vector2 verts[4];
    int32 milliseconds = 0;
    int32 seconds      = 0;
    int32 minutes      = 0;
    int32 offset       = 0;

    drawPos.x  = this->gotThroughPos.x;
    drawPos.y  = this->gotThroughPos.y;
    verts[0].y = drawPos.y - TO_FIXED(20);
    verts[1].y = drawPos.y - TO_FIXED(20);
    verts[2].y = drawPos.y - TO_FIXED(4);
    verts[3].y = drawPos.y - TO_FIXED(4);
    if ((GET_CHARACTER_ID(1)) == ID_KNUCKLES) {
        int32 center = TO_FIXED(screenInfo->center.x + 16);

        drawPos.x  = 2 * this->gotThroughPos.x + center;
        verts[0].x = drawPos.x - TO_FIXED(145);
        verts[1].x = drawPos.x + TO_FIXED(52);
        verts[2].x = drawPos.x + TO_FIXED(68);
        verts[3].x = drawPos.x - TO_FIXED(129);
        Graphics::DrawFace(verts, 4, 0x00, 0x00, 0x00, 0xFF, INK_NONE);

        drawPos.x  = 2 * this->playerNamePos.x + center;
        drawPos.y  = this->playerNamePos.y;
        verts[0].x = drawPos.x - TO_FIXED(124);
        verts[0].y = drawPos.y + TO_FIXED(28);
        verts[1].x = drawPos.x + TO_FIXED(80);
        verts[2].x = drawPos.x + TO_FIXED(96);
        verts[3].x = drawPos.x - TO_FIXED(108);
        verts[1].y = drawPos.y + TO_FIXED(28);
        verts[2].y = drawPos.y + TO_FIXED(44);
        verts[3].y = drawPos.y + TO_FIXED(44);
        Graphics::DrawFace(verts, 4, 0x00, 0x00, 0x00, 0xFF, INK_NONE);

        drawPos.x = this->playerNamePos.x + center;
        drawPos.y = this->playerNamePos.y;
        this->playerNameAnimator.DrawSprite(&drawPos, true);

        this->gotThroughAnimator.frameID = 2;
        this->gotThroughAnimator.DrawSprite(&drawPos, true);

        drawPos.x                        = this->gotThroughPos.x + center;
        drawPos.y                        = this->gotThroughPos.y;
        this->gotThroughAnimator.frameID = 3;
        this->gotThroughAnimator.DrawSprite(&drawPos, true);
        this->actNumAnimator.DrawSprite(&drawPos, true);

        offset = center - TO_FIXED(10);
    }
    else {
        int32 center = TO_FIXED(screenInfo->center.x + 16);

        drawPos.x  = 2 * this->gotThroughPos.x + center;
        verts[0].x = drawPos.x - TO_FIXED(109);
        verts[1].x = drawPos.x + TO_FIXED(52);
        verts[2].x = drawPos.x + TO_FIXED(68);
        verts[3].x = drawPos.x - TO_FIXED(93);
        Graphics::DrawFace(verts, 4, 0x00, 0x00, 0x00, 0xFF, INK_NONE);

        drawPos.x  = 2 * this->playerNamePos.x + center;
        drawPos.y  = this->playerNamePos.y;
        verts[0].x = drawPos.x - TO_FIXED(88);
        verts[0].y = drawPos.y + TO_FIXED(28);
        verts[1].x = drawPos.x + TO_FIXED(80);
        verts[2].x = drawPos.x + TO_FIXED(96);
        verts[3].x = drawPos.x - TO_FIXED(72);
        verts[1].y = drawPos.y + TO_FIXED(28);
        verts[2].y = drawPos.y + TO_FIXED(44);
        verts[3].y = drawPos.y + TO_FIXED(44);
        Graphics::DrawFace(verts, 4, 0x00, 0x00, 0x00, 0xFF, INK_NONE);

        drawPos.x = this->playerNamePos.x + center;
        drawPos.y = this->playerNamePos.y;
        this->playerNameAnimator.DrawSprite(&drawPos, true);

        if ((GET_CHARACTER_ID(1)) == ID_MIGHTY)
            this->gotThroughAnimator.frameID = 2;
        else
            this->gotThroughAnimator.frameID = 0;
        this->gotThroughAnimator.DrawSprite(&drawPos, true);

        drawPos.x                        = this->gotThroughPos.x + center;
        drawPos.y                        = this->gotThroughPos.y;
        this->gotThroughAnimator.frameID = 1;
        this->gotThroughAnimator.DrawSprite(&drawPos, true);
        this->actNumAnimator.DrawSprite(&drawPos, true);

        offset = center + TO_FIXED(6);
    }

    // Draw "Time" Bonus Sprite
    drawPos.x                         = offset + this->timeBonusPos.x - TO_FIXED(92);
    drawPos.y                         = this->timeBonusPos.y;
    this->hudElementsAnimator.frameID = 1;
    this->hudElementsAnimator.DrawSprite(&drawPos, true);

    // Draw "Bonus" Sprite
    drawPos.x += TO_FIXED(50);
    if (globals->gameMode != MODE_TIMEATTACK) {
        this->hudElementsAnimator.frameID = 8; // "Bonus"
        this->hudElementsAnimator.DrawSprite(&drawPos, true);
    }

    // Draw Time Bonus BG thingy
    this->hudElementsAnimator.frameID = 10;
    drawPos.x += TO_FIXED(52);
    this->hudElementsAnimator.DrawSprite(&drawPos, true);

    drawPos.x += TO_FIXED(67);
    drawPos.y += TO_FIXED(14);

    if (globals->gameMode == MODE_TIMEATTACK) {
        drawPos.x -= TO_FIXED(98);
        drawPos.y -= TO_FIXED(14);
        // Draw Current Time
        DrawTime(&drawPos, sceneInfo->minutes, sceneInfo->seconds, sceneInfo->milliseconds);
    }
    else {
        DrawNumbers(&drawPos, this->timeBonus, 0); // Draw Time Bonus
    }

    // Draw Ring Bonus
    drawPos.x = offset + this->ringBonusPos.x - TO_FIXED(92);
    drawPos.y = this->ringBonusPos.y;
    if (globals->gameMode == MODE_TIMEATTACK)
        this->hudElementsAnimator.frameID = 17; // "Best"
    else
        this->hudElementsAnimator.frameID = 5; // "Ring"
    this->hudElementsAnimator.DrawSprite(&drawPos, true);

    drawPos.x += TO_FIXED(50);
    if (globals->gameMode == MODE_TIMEATTACK) {
        drawPos.x -= TO_FIXED(10);
        this->hudElementsAnimator.frameID = 1; // "Time"
        this->hudElementsAnimator.DrawSprite(&drawPos, true);
        drawPos.x += TO_FIXED(10);
    }
    else {
        this->hudElementsAnimator.frameID = 8; // "Bonus"
        this->hudElementsAnimator.DrawSprite(&drawPos, true);
    }

    // Draw Ring Bonus BG thingy
    this->hudElementsAnimator.frameID = 10;
    drawPos.x += TO_FIXED(52);
    this->hudElementsAnimator.DrawSprite(&drawPos, true);

    drawPos.x += TO_FIXED(67);
    drawPos.y += TO_FIXED(14);
    if (globals->gameMode == MODE_TIMEATTACK) { // Draw Best Time
        TimeAttackData::GetUnpackedTime(this->time, &minutes, &seconds, &milliseconds);
        drawPos.x -= TO_FIXED(98);
        drawPos.y -= TO_FIXED(14);
        if (!this->isNewRecord || (this->isNewRecord && (Zone::sVars->timer & 8)))
            DrawTime(&drawPos, minutes, seconds, milliseconds);
    }
    else {
        DrawNumbers(&drawPos, this->ringBonus, 0);
    }

    if (this->showCoolBonus) { // Draw Cool Bonus
        drawPos.x = offset + this->coolBonusPos.x - TO_FIXED(92);
        drawPos.y = this->coolBonusPos.y;
        if (globals->gameMode == MODE_TIMEATTACK)
            this->hudElementsAnimator.frameID = 18; // "Rank"
        else
            this->hudElementsAnimator.frameID = 15; // "Cool"
        this->hudElementsAnimator.DrawSprite(&drawPos, true);

        drawPos.x += TO_FIXED(50);
        if (globals->gameMode != MODE_TIMEATTACK) {
            this->hudElementsAnimator.frameID = 8; // "Bonus"
            this->hudElementsAnimator.DrawSprite(&drawPos, true);
        }

        // Draw Cool Bonus BG thingy
        this->hudElementsAnimator.frameID = 10;
        drawPos.x += TO_FIXED(52);
        this->hudElementsAnimator.DrawSprite(&drawPos, true);

        drawPos.x += TO_FIXED(67);
        drawPos.y += TO_FIXED(14);

        if (globals->gameMode != MODE_TIMEATTACK) {
            DrawNumbers(&drawPos, this->coolBonus, 0); // Draw Cool bonus
        }
        else {
            // Draw Rank
            if (!TimeAttackData::sVars->personalRank) {
                this->numbersAnimator.frameID = 16;
                this->numbersAnimator.DrawSprite(&drawPos, true);

                drawPos.x -= TO_FIXED(9);
            }
            else if (!this->achievedRank || (this->achievedRank && (Zone::sVars->timer & 8)))
                DrawNumbers(&drawPos, TimeAttackData::sVars->personalRank, 0);
        }
    }

    drawPos.x = this->totalScorePos.x;
    drawPos.y = this->totalScorePos.y;

    if (globals->gameMode == MODE_TIMEATTACK) {
        // Draw World Rank
        drawPos.x                         = offset + this->totalScorePos.x - TO_FIXED(92);
        this->hudElementsAnimator.frameID = 19; // "World"
        this->hudElementsAnimator.DrawSprite(&drawPos, true);

        drawPos.x += TO_FIXED(50);
        this->hudElementsAnimator.frameID = 18; // "Rank"
        this->hudElementsAnimator.DrawSprite(&drawPos, true);

        // Draw World Rank BG thingy
        this->hudElementsAnimator.frameID = 10;
        drawPos.x += TO_FIXED(52);
        this->hudElementsAnimator.DrawSprite(&drawPos, true);

        drawPos.x += TO_FIXED(67);
        drawPos.y += TO_FIXED(14);
        if (!TimeAttackData::sVars->leaderboardRank) {
            this->numbersAnimator.frameID = 16; // "-" (no rank)
            this->numbersAnimator.DrawSprite(&drawPos, true);
        }
        else {
            // Draw Rank
            DrawNumbers(&drawPos, TimeAttackData::sVars->leaderboardRank, 0);
        }
    }
    else {
        // Draw Total Score
        drawPos.x                         = offset + this->totalScorePos.x - TO_FIXED(68);
        this->hudElementsAnimator.frameID = 9; // "Total"
        this->hudElementsAnimator.DrawSprite(&drawPos, true);

        // Draw Total Score BG thingy
        this->hudElementsAnimator.frameID = 10;
        drawPos.x += TO_FIXED(52);
        this->hudElementsAnimator.DrawSprite(&drawPos, true);

        // Draw Total Score
        drawPos.x += TO_FIXED(67);
        drawPos.y += TO_FIXED(14);
        DrawNumbers(&drawPos, this->totalScore, 0);
    }
}

void ActClear::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        sVars->actClearActive = true;

        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawGroup = Zone::sVars->hudDrawGroup;
        this->state.Set(&ActClear::State_EnterText);
        this->stageFinishTimer = 0;
        this->newRecordTimer   = 0;

        Player *player1    = GameObject::Get<Player>(SLOT_PLAYER1);
        this->targetPlayer = player1;

        if (Zone::CurrentID() > -1) {
            uint16 time = TimeAttackData::GetPackedTime(sceneInfo->minutes, sceneInfo->seconds, sceneInfo->milliseconds);
            UNUSED(time);
        }

        if (!sVars->disableTimeBonus) {
            switch (sceneInfo->minutes) {
                case 0:
                    if (sceneInfo->seconds >= 30)
                        this->timeBonus = sceneInfo->seconds < 45 ? 10000 : 5000;
                    else
                        this->timeBonus = 50000;
                    break;

                case 1: this->timeBonus = sceneInfo->seconds < 30 ? 4000 : 3000; break;
                case 2: this->timeBonus = 2000; break;
                case 3: this->timeBonus = 1000; break;
                case 4: this->timeBonus = 500; break;
                case 5: this->timeBonus = 100; break;

                case 9:
                    if (!sceneInfo->debugMode && globals->gameMode < MODE_TIMEATTACK && sceneInfo->seconds == 59) {
                        if (globals->gameMode != MODE_ENCORE && !(globals->medalMods & MEDAL_NOTIMEOVER))
                            this->timeBonus = 100000;
                    }
                    break;

                default: break;
            }
        }

        this->ringBonus        = 100 * player1->rings;
        this->coolBonus        = globals->coolBonus[0];
        globals->initCoolBonus = false;

        if (globals->gameMode == MODE_TIMEATTACK) {
            MenuParam *param   = MenuParam::GetMenuParam();
            this->time         = TimeAttackData::GetScore(param->zoneID, param->actID, param->characterID, sceneInfo->filter == 5, 1);
            this->achievedRank = false;
            this->isNewRecord  = false;
        }

        this->showCoolBonus   = true;
        this->playerNamePos.x = TO_FIXED(224);
        this->playerNamePos.y = TO_FIXED(88);
        this->gotThroughPos.x = -TO_FIXED(224);
        this->gotThroughPos.y = TO_FIXED(112);
        this->timeBonusPos.x  = TO_FIXED(488);
        this->timeBonusPos.y  = TO_FIXED(120);
        this->ringBonusPos.x  = TO_FIXED(776);
        this->ringBonusPos.y  = TO_FIXED(136);
        this->coolBonusPos.x  = TO_FIXED(1064);
        this->coolBonusPos.y  = TO_FIXED(152);
        this->totalScorePos.x = -TO_FIXED(1352);
        this->totalScorePos.y = TO_FIXED(192);

        this->hudElementsAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->numbersAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->timeElementsAnimator.SetAnimation(sVars->aniFrames, 0, true, 12);

        switch (GET_CHARACTER_ID(1)) {
            default:
            case ID_SONIC: this->playerNameAnimator.SetAnimation(sVars->aniFrames, 3, true, 0); break;

            case ID_TAILS:
                this->playerNameAnimator.SetAnimation(sVars->aniFrames, 3, true, 1);
                if (globals->secrets & SECRET_REGIONSWAP)
                    this->playerNameAnimator.frameID = this->playerNameAnimator.frameCount - 1;
                break;

            case ID_KNUCKLES: this->playerNameAnimator.SetAnimation(sVars->aniFrames, 3, true, 2); break;
            case ID_MIGHTY: this->playerNameAnimator.SetAnimation(sVars->aniFrames, 3, true, 3); break;
            case ID_RAY: this->playerNameAnimator.SetAnimation(sVars->aniFrames, 3, true, 4); break;
        }

        this->gotThroughAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);

        // Used in cases like OOZ1 outro where the act clear actually happens in AIZ2
        if (sVars->displayedActID <= 0)
            this->actNumAnimator.SetAnimation(sVars->aniFrames, 5, true, Zone::sVars->actID > 0);
        else
            this->actNumAnimator.SetAnimation(sVars->aniFrames, 5, true, sVars->displayedActID - 1);

        if (GET_CHARACTER_ID(1) == ID_KNUCKLES)
            this->actNumAnimator.frameID += 2;
    }
}

void ActClear::StageLoad()
{
    sVars->aniFrames.Load("Global/HUD.bin", SCOPE_STAGE);

    sVars->sfxScoreAdd.Get("Global/ScoreAdd.wav");
    sVars->sfxScoreTotal.Get("Global/ScoreTotal.wav");
    sVars->sfxEvent.Get("Special/Event.wav");

    sVars->actClearActive = false;
    sVars->forceNoSave    = false;
    sVars->disableJingle  = false;
    sVars->victoryTimer   = 0;
}

void ActClear::State_EnterText()
{
    SET_CURRENT_STATE();

    if (this->playerNamePos.x > 0)
        this->playerNamePos.x -= TO_FIXED(16);

    if (this->gotThroughPos.x < 0)
        this->gotThroughPos.x += TO_FIXED(16);

    if (!this->timer && Zone::sVars->shouldRecoverPlayers)
        SetupForceOnScreenP2();

    if (++this->timer == 48) {
        this->timer = 0;
        this->state.Set(&ActClear::State_AdjustText);
    }

    CheckPlayerVictory();
}

void ActClear::State_AdjustText()
{
    SET_CURRENT_STATE();

    this->playerNamePos.y -= 0x8000;
    this->gotThroughPos.y -= 0x8000;

    if (++this->timer == 48) {
        this->timer = 0;
        this->state.Set(&ActClear::State_EnterResults);
    }

    CheckPlayerVictory();
}
void ActClear::State_EnterResults()
{
    SET_CURRENT_STATE();

    if (this->timeBonusPos.x > 0)
        this->timeBonusPos.x -= TO_FIXED(16);

    if (this->ringBonusPos.x > 0)
        this->ringBonusPos.x -= TO_FIXED(16);

    if (this->coolBonusPos.x > 0)
        this->coolBonusPos.x -= TO_FIXED(16);

    if (this->totalScorePos.x < -TO_FIXED(8)) {
        this->totalScorePos.x += TO_FIXED(16);
    }
    else {
        if (globals->gameMode == MODE_TIMEATTACK) {
            sVars->bufferMove_CB.Run(this);

            HUD::sVars->showTAPrompt = true;
            sVars->hasSavedReplay    = false;
            this->newRecordTimer     = 240;
            this->state.Set(&ActClear::State_ShowResultsTA);
            Stage::SetScene("Presentation", "Menu");
        }
        else {
            this->state.Set(&ActClear::State_ScoreShownDelay);
        }
    }

    CheckPlayerVictory();
}

void ActClear::State_ScoreShownDelay()
{
    SET_CURRENT_STATE();

    if (++this->timer == 120) {
        this->timer = 0;
        this->state.Set(&ActClear::State_TallyScore);
    }

    CheckPlayerVictory();
}

void ActClear::State_TallyScore()
{
    SET_CURRENT_STATE();

    Player *player = this->targetPlayer;

    if (this->timeBonus > 0) {
        this->totalScore += 100;
        this->timeBonus -= 100;
        player->GiveScore(100);
    }

    if (this->ringBonus > 0) {
        this->totalScore += 100;
        this->ringBonus -= 100;
        player->GiveScore(100);
    }

    if (this->coolBonus > 0) {
        this->totalScore += 100;
        this->coolBonus -= 100;
        player->GiveScore(100);
    }

    if (controllerInfo[player->controllerID].keyA.press || controllerInfo[player->controllerID].keyStart.press) {
        player->GiveScore(this->timeBonus + this->ringBonus + this->coolBonus);
        this->totalScore += this->timeBonus + this->ringBonus + this->coolBonus;
        this->timeBonus = 0;
        this->ringBonus = 0;
        this->coolBonus = 0;
    }

    if (this->timeBonus + this->ringBonus + this->coolBonus <= 0) {
        this->timer = 0;
        this->state.Set(&ActClear::State_SaveGameProgress);
        sVars->sfxScoreTotal.Play();
    }
    else if (++this->timer == 2) {
        this->timer = 0;
        sVars->sfxScoreAdd.Play();
    }

    Music::sVars->nextTrack = Music::TRACK_NONE;

    ActClear::CheckPlayerVictory();
}

void ActClear::State_SaveGameProgress()
{
    SET_CURRENT_STATE();

    if (++this->timer == 120) {
        this->timer            = 0;
        globals->specialRingID = 0;
        if (sVars->displayedActID <= 0) {
            if (globals->gameMode == MODE_COMPETITION) {
                Stage::SetScene("Presentation", "Menu");
            }
            else {
                globals->enableIntro = true;
                Player::SaveValues();
                SaveGame::ClearRestartData();
                StarPost::ResetStarPosts();
                if (Zone::sVars->actID > 0)
                    SaveGame::GetSaveRAM()->collectedSpecialRings = 0;
                SaveGame::SaveProgress();

                if (globals->saveSlotID != NO_SAVE_SLOT && !sVars->forceNoSave) {
                    if (Zone::CurrentStageSaveable())
                        GameProgress::MarkZoneCompleted(Zone::CurrentID());

                    sVars->isSavingGame = true;
                    SaveGame::SaveFile(ActClear::SaveGameCallback);
                }

                ++sceneInfo->listPos;
                if (!Stage::CheckValidScene())
                    Stage::SetScene("Presentation", "Title Screen");
            }
        }
        else {
            Player::SaveValues();
            SaveGame::ClearRestartData();
            StarPost::ResetStarPosts();
            SaveGame::SaveProgress();

            if (globals->saveSlotID != NO_SAVE_SLOT && !sVars->forceNoSave) {
                sVars->isSavingGame = true;
                SaveGame::SaveFile(ActClear::SaveGameCallback);
            }
        }

        // if (sVars->isSavingGame)
        //     UIWaitSpinner::StartWait();

        this->state.SetAndRun(&ActClear::State_WaitForSave, this);
    }
}

void ActClear::State_ShowResultsTA()
{
    SET_CURRENT_STATE();

    if (this->newRecordTimer > 0) {
        if (TimeAttackData::sVars->personalRank > 0 /*&& !ReplayRecorder::sVars->hasSetupGhostView*/) {
            if (this->newRecordTimer == 120) {
                if (TimeAttackData::sVars->personalRank == 1)
                    this->isNewRecord = true;

                this->achievedRank = true;
                sVars->sfxEvent.Play();
            }

            if (this->newRecordTimer == 30) {
                // if (TimeAttackData::sVars->personalRank == 1)
                //     Announcer::sVars->sfxNewRecordTop.Play();
                // else if (TimeAttackData::sVars->personalRank <= 3)
                //     Announcer::sVars->sfxNewRecordMid.Play();
            }
        }
        --this->newRecordTimer;
    }

    if (!sVars->isSavingGame && !sVars->disableResultsInput) {
        if (controllerInfo->keyY.press) {
            if (!sVars->hasSavedReplay) {
                if (HUD::sVars->replaySaveEnabled) {
                    // if (!UIDialog->activeDialog) {
                    sVars->saveReplay_CB.Run(this);
                    sVars->hasSavedReplay      = true;
                    sVars->disableResultsInput = true;
                    return;
                    // }
                }
            }
        }

        if (controllerInfo->keyStart.press) {
            // UIWidgets::sVars->sfxAccept.Play();

            State_WaitForSave();
        }
    }
}

void ActClear::State_WaitForSave()
{
    SET_CURRENT_STATE();

    if (!sVars->isSavingGame) {
        if (sVars->displayedActID > 0 || !Zone::sVars->stageFinishCallback.Matches(nullptr)) {
            this->state.Set(&ActClear::State_EndEvent);
        }
        else {
            this->state.Set(nullptr);
            Zone::StartFadeOut(10, 0x000000);
        }
    }
}

void ActClear::State_EndEvent()
{
    SET_CURRENT_STATE();

    this->playerNamePos.x += TO_FIXED(32);
    this->gotThroughPos.x -= TO_FIXED(32);

    if (this->playerNamePos.x > TO_FIXED(64))
        this->timeBonusPos.x += TO_FIXED(32);

    if (this->timeBonusPos.x > TO_FIXED(64))
        this->ringBonusPos.x += TO_FIXED(32);

    if (this->ringBonusPos.x > TO_FIXED(64))
        this->coolBonusPos.x += TO_FIXED(32);

    if (this->coolBonusPos.x > TO_FIXED(64))
        this->totalScorePos.x -= TO_FIXED(32);

    if (this->totalScorePos.x < -TO_FIXED(512)) {
        if (sVars->displayedActID <= 0) {
            if (!Zone::sVars->stageFinishCallback.Matches(nullptr)) {
                if (Zone::sVars->shouldRecoverPlayers) {
                    this->timer = 0;
                    this->state.Set(&ActClear::State_RecoverPlayers);
                }
                else {
                    for (auto animal : GameObject::GetEntities<Animals>(FOR_ACTIVE_ENTITIES)) {
                        if (animal->behaviour == Animals::BehaveFollow)
                            animal->behaviour = Animals::BehaveFree;
                    }
                    Zone::sVars->stageFinishCallback.Run(this);
                    Zone::sVars->stageFinishCallback.Set(nullptr);
                }
            }
        }
        else {
            sVars->finished         = true;
            sVars->displayedActID   = 0;
            sceneInfo->milliseconds = 0;
            sceneInfo->seconds      = 0;
            sceneInfo->minutes      = 0;
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                player->ringExtraLife = 100;
                player->rings         = 0;
            }
        }

        if (!this->state.Matches(&ActClear::State_RecoverPlayers))
            this->Destroy();
    }
}

void ActClear::State_RecoverPlayers()
{
    SET_CURRENT_STATE();

    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);
    Player *player2 = GameObject::Get<Player>(SLOT_PLAYER2);

    bool32 finishedP2  = false;
    bool32 finishedP1  = false;
    int32 screenOffX   = (screenInfo->position.x + screenInfo->size.x - 16) << 16;
    player1->up        = false;
    player1->down      = false;
    player1->jumpPress = false;
    player1->jumpHold  = false;

    if (player1->position.x >= screenOffX) {
        player1->DisableInputs(true);
        player1->state.Set(&Player::State_Ground);
        player1->groundVel = -0x40000;
        player1->left      = true;
        player1->right     = false;
    }
    else {
        if ((!player1->onGround || player1->groundVel) && player1->position.x < screenOffX - (screenInfo->center.x << 15)) {
            player1->right = true;
            if (!player1->skidding) {
                if (!player1->left)
                    player1->right = false;
                else
                    player1->left = false;
            }
        }

        if (player1->onGround && !player1->groundVel)
            finishedP1 = true;
    }

    if (player2->classID != Player::sVars->classID) {
        finishedP2 = true;
    }
    else {
        player2->up        = false;
        player2->down      = false;
        player2->jumpPress = false;
        player2->jumpHold  = false;

        if (player2->state.Matches(&Player::State_FlyToPlayer) || player2->state.Matches(&Player::State_ReturnToPlayer)) {
            if (player2->position.x < screenOffX) {
                if (player2->onGround && !player2->groundVel) {
                    player2->animator.SetAnimation(player2->aniFrames, Player::ANI_IDLE, false, 0);
                    player2->direction = FLIP_NONE;
                    finishedP2         = true;
                }
            }
        }
        else if (player2->position.x >= screenOffX) {
            player2->stateInput.Set(&Player::Input_AI_Follow);
            player2->state.Set(&Player::State_Ground);
            player2->animator.SetAnimation(player2->aniFrames, Player::ANI_RUN, false, 0);
            player2->groundVel = -0x40000;
            player2->left      = true;
            player2->right     = false;

            if (player2->position.x < screenOffX) {
                if (player2->onGround && !player2->groundVel) {
                    player2->animator.SetAnimation(player2->aniFrames, Player::ANI_IDLE, false, 0);
                    player2->direction = FLIP_NONE;
                    finishedP2         = true;
                }
            }
        }
        else {
            if (player2->onGround && !player2->groundVel) {
                player2->animator.SetAnimation(player2->aniFrames, Player::ANI_IDLE, false, 0);
                player2->direction = FLIP_NONE;
                finishedP2         = true;
            }
        }
    }

    if (finishedP1) {
        player1->animator.SetAnimation(player1->aniFrames, Player::ANI_IDLE, false, 0);
        player1->direction = FLIP_NONE;
    }

    ++this->stageFinishTimer;
    if ((finishedP1 && finishedP2) || this->stageFinishTimer >= 900) {
        if (this->timer >= 10) {
            Player::sVars->respawnTimer = 0;

            Zone::sVars->stageFinishCallback.Run((Zone *)this);
            Zone::sVars->stageFinishCallback.Set(nullptr);

            this->Destroy();
        }
        else {
            this->timer++;
        }
    }
}

void ActClear::DrawNumbers(RSDK::Vector2 *drawPos, int32 value, int32 digitCount)
{
    if (value >= 0) {
        if (!digitCount && value > 0) {
            int32 v = value;
            while (v > 0) {
                ++digitCount;
                v /= 10;
            }
        }
        else {
            if (!digitCount && !value)
                digitCount = 1;
        }

        if (digitCount > 0) {
            int32 digit = 1;
            while (digitCount--) {
                this->numbersAnimator.frameID = value / digit % 10;
                this->numbersAnimator.DrawSprite(drawPos, true);

                drawPos->x -= TO_FIXED(9);
                digit *= 10;
            }
        }
    }
    else {
        while (digitCount--) {
            this->numbersAnimator.frameID = 16;
            this->numbersAnimator.DrawSprite(drawPos, true);

            drawPos->x -= TO_FIXED(9);
        }
    }
}

void ActClear::DrawTime(RSDK::Vector2 *drawPosPtr, int32 mins, int32 secs, int32 millisecs)
{
    Vector2 drawPos;
    if (!mins && !secs && !millisecs) {
        millisecs = -1;
        secs      = -1;
        mins      = -1;
    }

    // The ":" thing
    drawPos.x = drawPosPtr->x + TO_FIXED(50);
    drawPos.y = drawPosPtr->y - TO_FIXED(2);
    this->timeElementsAnimator.DrawSprite(&drawPos, true);

    // Miliseconds
    drawPos.x = drawPosPtr->x + TO_FIXED(97);
    drawPos.y = drawPosPtr->y + TO_FIXED(14);
    DrawNumbers(&drawPos, millisecs, 2);

    // Seconds
    drawPos.x -= TO_FIXED(9);
    if (!mins && !secs && !millisecs)
        secs = -1;
    DrawNumbers(&drawPos, secs, 2);

    // Minutes
    drawPos.x -= TO_FIXED(9);
    if (!mins && !secs && !millisecs)
        mins = -1;
    DrawNumbers(&drawPos, mins, 1);
}

void ActClear::CheckPlayerVictory()
{
    if (!sVars->disableVictory) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (!player->state.Matches(&Player::State_FlyToPlayer) && !player->state.Matches(&Player::State_ReturnToPlayer)
                && !player->state.Matches(&Player::State_Victory)) {
                player->state.Set(&Player::State_Victory);
                player->nextAirState.Set(nullptr);
                player->nextGroundState.Set(nullptr);
                if (Zone::sVars->shouldRecoverPlayers)
                    player->DisableInputs(true);

                player->animator.SetAnimation(player->aniFrames, Player::ANI_VICTORY, true, 0);
            }
        }
    }
}

void ActClear::SetupForceOnScreenP2()
{
    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);
    Player *player2 = GameObject::Get<Player>(SLOT_PLAYER2);

    if (player2 && player2->sidekick) {
        if (!player2->state.Matches(&Player::State_FlyToPlayer) && !player2->state.Matches(&Player::State_ReturnToPlayer)) {
            if (player2->position.x <= (screenInfo->size.x + screenInfo->position.x) << 16
                || abs(player2->position.y - player1->position.y) > 0x100000) {
                Player::sVars->respawnTimer = 240;

                player2->HandleSidekickRespawn();

                if (player2->state.Matches(&Player::State_FlyToPlayer) || player2->state.Matches(&Player::State_ReturnToPlayer)
                    || player2->state.Matches(&Player::State_HoldRespawn)) {
                    player2->active     = ACTIVE_NORMAL;
                    player2->position.y = ((screenInfo->position.y - 16) << 16);
                }
            }
            else {
                Player::sVars->respawnTimer = -3600;
            }
        }
    }
}

void ActClear::SaveGameCallback(bool32 success) { sVars->isSavingGame = false; }

#if RETRO_INCLUDE_EDITOR
void ActClear::EditorDraw()
{
    Animator animator;

    animator.SetAnimation(sVars->aniFrames, 0, true, 18);
    animator.DrawSprite(&this->position, false);
}

void ActClear::EditorLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void ActClear::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(ActClear);

    sVars->aniFrames.Init();
}
#endif

void ActClear::Serialize() {}

} // namespace GameLogic
