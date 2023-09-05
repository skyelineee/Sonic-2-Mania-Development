// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ActClear Object
// Object Author: Ducky + AChickMcNuggie
// ---------------------------------------------------------------------

#include "ActClear.hpp"
#include "Zone.hpp"
#include "SaveGame.hpp"
#include "DebugMode.hpp"
#include "Helpers/GameProgress.hpp"
#include "Helpers/MenuParam.hpp"
#include "Helpers/TimeAttackData.hpp"
#include "Menu/UIDialog.hpp"
#include "Menu/UIWidgets.hpp"
#include "Menu/UILoadingIcon.hpp"
#include "ReplayRecorder.hpp"
#include "HUD.hpp"
#include "Music.hpp"
#include "Animals.hpp"
#include "StarPost.hpp"
#include "Announcer.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ActClear);

void ActClear::Update() 
{ 
    this->state.Run(this); 
    this->trianglesLeftAnimator.Process();
    this->trianglesRightAnimator.Process();
    this->checkerboardBGAnimator.Process();
}

void ActClear::LateUpdate() {}
void ActClear::StaticUpdate()
{
    if (sVars->victoryTimer)
        sVars->victoryTimer--;
}
void ActClear::Draw()
{
    Vector2 drawPos;
    int32 milliseconds = 0;
    int32 seconds      = 0;
    int32 minutes      = 0;
    int32 offset       = 0;

    // this is where all of the sprites will be drawn based on their individual positions for every frame (their individual positions being altered by
    // sliding in or away)

    // checkboard doesnt move at all, so doesnt need its own position variable
    this->inkEffect = INK_ALPHA; // set to alpha for the checkerboard only
    drawPos.x       = 0;
    drawPos.y       = 0;
    this->checkerboardBGAnimator.DrawSprite(&drawPos, true);

    this->inkEffect = INK_NONE; // set back to none afterwards
    // left triangles position
    drawPos.x = this->triangleLeftPos.x;
    drawPos.y = this->triangleLeftPos.y;
    this->trianglesLeftAnimator.DrawSprite(&drawPos, true);

    // right triangles position
    drawPos.x = this->triangleRightPos.x;
    drawPos.y = this->triangleRightPos.y;
    this->trianglesRightAnimator.DrawSprite(&drawPos, true);

    // player name position
    drawPos.x                       = this->playerNamePos.x;
    drawPos.y                       = this->playerNamePos.y;
    this->scoreCardAnimator.frameID = 15;
    this->scoreCardAnimator.DrawSprite(&drawPos, true);
    this->playerNameAnimator.DrawSprite(&drawPos, true);

    // got position
    drawPos.x                        = this->gotPos.x;
    drawPos.y                        = this->gotPos.y;
    if (GET_CHARACTER_ID(1) == ID_KNUCKLES) {
        this->gotThroughAnimator.frameID = 2;
    }
    else {
        this->gotThroughAnimator.frameID = 0;
    }
    this->gotThroughAnimator.DrawSprite(&drawPos, true);

    // through position
    drawPos.x                        = this->throughPos.x;
    drawPos.y                        = this->throughPos.y;
    this->gotThroughAnimator.frameID = 1;
    this->gotThroughAnimator.DrawSprite(&drawPos, true);

    // act position
    drawPos.x                       = this->actPos.x;
    drawPos.y                       = this->actPos.y;
    this->scoreCardAnimator.frameID = 16;
    this->scoreCardAnimator.DrawSprite(&drawPos, true);
    this->actNumAnimator.DrawSprite(&drawPos, true);

    if (globals->gameMode != MODE_TIMEATTACK) {
        // time bonus position
        drawPos.x                         = this->timeBonusPos.x;
        drawPos.y                         = this->timeBonusPos.y;
        this->hudElementsAnimator.frameID = 7;
        this->hudElementsAnimator.DrawSprite(&drawPos, true);
        drawPos.x                       = timeNumPos.x;
        drawPos.y                       = timeNumPos.y;
        this->scoreCardAnimator.frameID = 14;
        this->scoreCardAnimator.DrawSprite(&drawPos, true);
        DrawNumbers(&drawPos, this->timeBonus, 0); // draw time bonus

        // ring bonus position
        drawPos.x                         = this->ringBonusPos.x;
        drawPos.y                         = this->ringBonusPos.y;
        this->hudElementsAnimator.frameID = 8;
        this->hudElementsAnimator.DrawSprite(&drawPos, true);
        drawPos.x                       = ringNumPos.x;
        drawPos.y                       = ringNumPos.y;
        this->scoreCardAnimator.frameID = 14;
        this->scoreCardAnimator.DrawSprite(&drawPos, true);
        DrawNumbers(&drawPos, this->ringBonus, 0); // draw ring bonus

        // cool bonus position
        drawPos.x                         = this->coolBonusPos.x;
        drawPos.y                         = this->coolBonusPos.y;
        this->hudElementsAnimator.frameID = 9;
        this->hudElementsAnimator.DrawSprite(&drawPos, true);
        drawPos.x                       = coolNumPos.x;
        drawPos.y                       = coolNumPos.y;
        this->scoreCardAnimator.frameID = 14;
        this->scoreCardAnimator.DrawSprite(&drawPos, true);
        DrawNumbers(&drawPos, this->coolBonus, 0); // draw cool bonus

        // total score position
        drawPos.x                         = this->totalScorePos.x;
        drawPos.y                         = this->totalScorePos.y;
        this->hudElementsAnimator.frameID = 10;
        this->hudElementsAnimator.DrawSprite(&drawPos, true);
        drawPos.x                       = totalNumPos.x;
        drawPos.y                       = totalNumPos.y;
        this->scoreCardAnimator.frameID = 14;
        this->scoreCardAnimator.DrawSprite(&drawPos, true);
        DrawNumbers(&drawPos, this->totalScore, 0); // draw total score
    }
    else {
        // time position
        drawPos.x                         = this->timeBonusPos.x;
        drawPos.y                         = this->timeBonusPos.y + TO_FIXED(16);
        this->hudElementsAnimator.frameID = 11;
        this->hudElementsAnimator.DrawSprite(&drawPos, true);
        drawPos.x                       = timeNumPos.x;
        drawPos.y                       = timeNumPos.y + TO_FIXED(16);
        this->scoreCardAnimator.frameID = 17;
        this->scoreCardAnimator.DrawSprite(&drawPos, true);
        drawPos.x -= TO_FIXED(96);
        drawPos.y -= TO_FIXED(14);
        DrawTime(&drawPos, sceneInfo->minutes, sceneInfo->seconds, sceneInfo->milliseconds); // draw time

        // best time position
        drawPos.x                         = this->ringBonusPos.x;
        drawPos.y                         = this->ringBonusPos.y + TO_FIXED(16);
        this->hudElementsAnimator.frameID = 12;
        this->hudElementsAnimator.DrawSprite(&drawPos, true);
        drawPos.x                       = ringNumPos.x;
        drawPos.y                       = ringNumPos.y + TO_FIXED(16);
        this->scoreCardAnimator.frameID = 17;
        this->scoreCardAnimator.DrawSprite(&drawPos, true);
        // draw best time
        TimeAttackData::GetUnpackedTime(this->time, &minutes, &seconds, &milliseconds);
        drawPos.x -= TO_FIXED(96);
        drawPos.y -= TO_FIXED(14);
        if (!this->isNewRecord || (this->isNewRecord && (Zone::sVars->timer & 8)))
            DrawTime(&drawPos, minutes, seconds, milliseconds);

        // rank position
        drawPos.x                         = this->coolBonusPos.x;
        drawPos.y                         = this->coolBonusPos.y + TO_FIXED(16);
        this->hudElementsAnimator.frameID = 13;
        this->hudElementsAnimator.DrawSprite(&drawPos, true);
        drawPos.x                       = coolNumPos.x;
        drawPos.y                       = coolNumPos.y + TO_FIXED(16);
        this->scoreCardAnimator.frameID = 17;
        this->scoreCardAnimator.DrawSprite(&drawPos, true);
        // draw rank
        if (!TimeAttackData::sVars->personalRank) {
            this->numbersAnimator.frameID = 16;
            this->numbersAnimator.DrawSprite(&drawPos, true);
        }
        else if (!this->achievedRank || (this->achievedRank && (Zone::sVars->timer & 8))) {
            DrawNumbers(&drawPos, TimeAttackData::sVars->personalRank, 0);
        }
    }
}

void ActClear::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        sVars->actClearActive = true;

        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawGroup = Zone::sVars->hudDrawGroup;
        // ink effect is set to none on creation, no transparency
        this->inkEffect = INK_NONE;
        // sets the alpha to 0 on creation for the checkerboard, shouldnt affect anything else as their ink effect is none
        this->alpha     = 0; // should increase or decrease for the sliding in and away
        // this is where the results start sliding in
        this->state.Set(&ActClear::State_EnterResults);
        this->stageFinishTimer = 0;
        this->newRecordTimer   = 0;
        this->slidingInTimer   = 0;
        this->slidingOutTimer = 0;
        // initial speeds, will slowly decrease or increase depending on if its sliding in or away
        // these are all set to the same amount, but they start decreasing at different times so they need to be different unfortunately
        this->topTextSpeed = TO_FIXED(25);
        this->timeBonusSpeed = TO_FIXED(25);
        this->ringBonusSpeed = TO_FIXED(25);
        this->coolBonusSpeed = TO_FIXED(25);
        this->totalScoreSpeed = TO_FIXED(25);
        Music::ClearMusicStack();
        Music::PlayTrack(Music::TRACK_ACTCLEAR);

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
                        if (!(globals->medalMods & MEDAL_NOTIMEOVER))
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
            this->time         = TimeAttackData::GetScore(param->zoneID, param->actID, param->characterID, 1);
            this->achievedRank = false;
            this->isNewRecord  = false;
        }

        // base positions before sliding in
        // each position is usually moved away by like 300 pixels so they all move at the same pace
        this->showCoolBonus   = true;
        triangleLeftPos.x     = -TO_FIXED(300);
        triangleLeftPos.y     = TO_FIXED(0);
        triangleRightPos.x    = TO_FIXED(678);
        triangleRightPos.y    = TO_FIXED(0);
        this->playerNamePos.x = -TO_FIXED(168);
        this->playerNamePos.y = TO_FIXED(54);
        this->gotPos.x        = -TO_FIXED(80);
        this->gotPos.y        = TO_FIXED(54);
        this->throughPos.x    = TO_FIXED(480);
        this->throughPos.y    = TO_FIXED(82);
        this->actPos.x        = TO_FIXED(602);
        this->actPos.y        = TO_FIXED(82);
        this->timeBonusPos.x  = -TO_FIXED(182);
        this->timeBonusPos.y  = TO_FIXED(122);
        this->ringBonusPos.x  = -TO_FIXED(182);
        this->ringBonusPos.y  = TO_FIXED(138);
        this->coolBonusPos.x  = -TO_FIXED(182);
        this->coolBonusPos.y  = TO_FIXED(154);
        this->totalScorePos.x = -TO_FIXED(158);
        this->totalScorePos.y = TO_FIXED(194);
        this->timeNumPos.x   = TO_FIXED(600);
        this->timeNumPos.y   = TO_FIXED(136);
        this->ringNumPos.x   = TO_FIXED(600);
        this->ringNumPos.y   = TO_FIXED(152);
        this->coolNumPos.x   = TO_FIXED(600);
        this->coolNumPos.y   = TO_FIXED(168);
        this->totalNumPos.x  = TO_FIXED(570);
        this->totalNumPos.y  = TO_FIXED(208);

        // final resting positions lol 
        /*
        triangleLeftPos.x     = TO_FIXED(0);
        triangleLeftPos.y     = TO_FIXED(0);
        triangleRightPos.x    = TO_FIXED(378);
        triangleRightPos.y    = TO_FIXED(0);
        this->playerNamePos.x = TO_FIXED(132);
        this->playerNamePos.y = TO_FIXED(54);
        this->gotPos.x        = TO_FIXED(220);
        this->gotPos.y        = TO_FIXED(54);
        this->throughPos.x    = TO_FIXED(180);
        this->throughPos.y    = TO_FIXED(82);
        this->actPos.x        = TO_FIXED(302);
        this->actPos.y        = TO_FIXED(82);
        this->timeBonusPos.x  = TO_FIXED(118);
        this->timeBonusPos.y  = TO_FIXED(122);
        this->ringBonusPos.x  = TO_FIXED(118);
        this->ringBonusPos.y  = TO_FIXED(138);
        this->coolBonusPos.x  = TO_FIXED(118);
        this->coolBonusPos.y  = TO_FIXED(154);
        this->totalScorePos.x = TO_FIXED(142);
        this->totalScorePos.y = TO_FIXED(194);
        this->scoreNumPos.x   = TO_FIXED(300);
        this->scoreNumPos.y   = TO_FIXED(136);
        */

        //setting animators
        this->hudElementsAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->numbersAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->timeElementsAnimator.SetAnimation(sVars->aniFrames, 0, true, 5);
        this->gotThroughAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
        this->actNumAnimator.SetAnimation(sVars->aniFrames, 5, true, Zone::sVars->actID);
        this->scoreCardAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->trianglesLeftAnimator.SetAnimation(sVars->aniFrames, 12, true, 0);
        this->trianglesRightAnimator.SetAnimation(sVars->aniFrames, 13, true, 0);
        this->checkerboardBGAnimator.SetAnimation(sVars->aniFrames, 14, true, 0);

        switch (GET_CHARACTER_ID(1)) {
            default:
            case ID_SONIC: this->playerNameAnimator.SetAnimation(sVars->aniFrames, 3, true, 0); break;

            case ID_TAILS:
                this->playerNameAnimator.SetAnimation(sVars->aniFrames, 3, true, 1);
                if (globals->secrets & SECRET_REGIONSWAP)
                    this->playerNameAnimator.frameID = this->playerNameAnimator.frameCount - 1;
                break;

            case ID_KNUCKLES: this->playerNameAnimator.SetAnimation(sVars->aniFrames, 3, true, 2); break;
        }
    }
}

void ActClear::StageLoad()
{
    switch GET_CHARACTER_ID(1) {
            default: break;
            case ID_SONIC: sVars->aniFrames.Load("Global/HUDSonic.bin", SCOPE_STAGE); break;
            case ID_TAILS: sVars->aniFrames.Load("Global/HUDTails.bin", SCOPE_STAGE); break;
            case ID_KNUCKLES: sVars->aniFrames.Load("Global/HUDKnux.bin", SCOPE_STAGE); break;
        }

    sVars->sfxScoreAdd.Get("Global/ScoreAdd.wav");
    sVars->sfxScoreTotal.Get("Global/ScoreTotal.wav");
    sVars->sfxEvent.Get("Special/Event.wav");

    sVars->actClearActive = false;
    sVars->forceNoSave    = false;
    sVars->disableJingle  = false;
    sVars->victoryTimer   = 0;
}

void ActClear::State_EnterResults()
{
    SET_CURRENT_STATE();

    // im aware its probably really ugly to be using multiple speed values, but im not sure how else to get them to all start moving at different times but at the same pace so
    // if i used only 1 speed value but had them all still start moving at different times, the speed would be too low to really do anything on the later ones as a result of it subtracting every frame

    // gets the hud object and sets its state to moving out (dont need it visible here lmao)
    for (auto hud : GameObject::GetEntities<HUD>(FOR_ALL_ENTITIES)) {
        hud->state.Set(&HUD::State_MoveOut);
    }

    // increases alpha for the checkerboard
    if (this->alpha < 96) {
        this->alpha += 8;
    }

    // moves the ai assist player on screen so it doesnt get stuck
    if (!this->timer && Zone::sVars->shouldRecoverPlayers)
        SetupForceOnScreenP2();

    // these pull in right at the start of the act clear so it doesnt need a timer to start
    // decreases the top text speed by 1 every frame if its above 0
    if (this->topTextSpeed > 0) {
        this->topTextSpeed -= TO_FIXED(1);
    }
    // makes sure top text speed never gets below 0 and starts moving in the wrong direction
    if (this->topTextSpeed < 0) {
        this->topTextSpeed = 0;
    }

    
    // triangle movement
    if (triangleLeftPos.x < TO_FIXED(0)) {
        triangleLeftPos.x += this->topTextSpeed; // these come in at the same time as the top text so fortunately can use the same speed value
    }
    if (triangleRightPos.x > TO_FIXED(378)) {
        triangleRightPos.x -= this->topTextSpeed;
    }

    // top text movement
    if (this->playerNamePos.x < TO_FIXED(132)) {
        this->playerNamePos.x += this->topTextSpeed;
    }
    if (this->gotPos.x < TO_FIXED(220)) {
        this->gotPos.x += this->topTextSpeed;
    }
    if (this->throughPos.x > TO_FIXED(180)) {
        this->throughPos.x -= this->topTextSpeed;
    }
    if (this->actPos.x > TO_FIXED(302)) {
        this->actPos.x -= this->topTextSpeed;
    }

    // bonus text movement
    if (++this->slidingInTimer >= 48) {
        // time bonus speed
        if (this->timeBonusSpeed > 0) {
            this->timeBonusSpeed -= TO_FIXED(1);
        }
        if (this->timeBonusSpeed < 0) {
            this->timeBonusSpeed = 0;
        }
        // time bonus movement
        if (this->timeBonusPos.x < TO_FIXED(118) || this->timeNumPos.x > TO_FIXED(300)) {
            this->timeBonusPos.x += this->timeBonusSpeed;
            this->timeNumPos.x -= this->timeBonusSpeed;
        }
        if (this->slidingInTimer >= 64) {
            // ring bonus speed
            if (this->ringBonusSpeed > 0) {
                this->ringBonusSpeed -= TO_FIXED(1);
            }
            if (this->ringBonusSpeed < 0) {
                this->ringBonusSpeed = 0;
            }
            // ring bonus movement
            if (this->ringBonusPos.x < TO_FIXED(118) || this->ringNumPos.x > TO_FIXED(300)) {
                this->ringBonusPos.x += this->ringBonusSpeed;
                this->ringNumPos.x -= this->ringBonusSpeed;
            }
            if (this->slidingInTimer >= 80) {
                // cool bonus speed
                if (this->coolBonusSpeed > 0) {
                    this->coolBonusSpeed -= TO_FIXED(1);
                }
                if (this->coolBonusSpeed < 0) {
                    this->coolBonusSpeed = 0;
                }
                // cool bonus movement
                if (this->coolBonusPos.x < TO_FIXED(118) || this->coolNumPos.x > TO_FIXED(300)) {
                    this->coolBonusPos.x += this->coolBonusSpeed;
                    this->coolNumPos.x -= this->coolBonusSpeed;
                }
            }
        }
    }

    if (this->slidingInTimer >= 96) {
        // total score speed
        if (this->totalScoreSpeed > 0) {
            this->totalScoreSpeed -= TO_FIXED(1);
        }
        if (this->totalScoreSpeed < 0) {
            this->totalScoreSpeed = 0;
        }
        // total score movement
        if (this->totalScorePos.x < TO_FIXED(142) || this->totalNumPos.x > TO_FIXED(270)) {
            this->totalScorePos.x += this->totalScoreSpeed;
            this->totalNumPos.x -= this->totalScoreSpeed;
        }
        else {
            if (this->slidingInTimer >= 128) {
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
            globals->enableIntro = true;
            Player::SaveValues();
            SaveGame::ClearRestartData();
            StarPost::ResetStarPosts();
            if (Zone::sVars->actID >= 0)
                SaveGame::ClearCollectedSpecialRings();
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

        if (sVars->isSavingGame)
            UILoadingIcon::StartWait();

        this->state.SetAndRun(&ActClear::State_WaitForSave, this);
    }
}

void ActClear::State_ShowResultsTA()
{
    SET_CURRENT_STATE();

    if (this->newRecordTimer > 0) {
        if (TimeAttackData::sVars->personalRank > 0 && !ReplayRecorder::sVars->hasSetupGhostView) {
            if (this->newRecordTimer == 120) {
                if (TimeAttackData::sVars->personalRank == 1)
                    this->isNewRecord = true;

                this->achievedRank = true;
                sVars->sfxEvent.Play();
            }

            if (this->newRecordTimer == 30) {
                if (TimeAttackData::sVars->personalRank == 1)
                    Announcer::sVars->sfxNewRecordTop.Play();
                else if (TimeAttackData::sVars->personalRank <= 3)
                   Announcer::sVars->sfxNewRecordMid.Play();
            }
        }
        --this->newRecordTimer;
    }

    if (!sVars->isSavingGame && !sVars->disableResultsInput) {
        if (controllerInfo->keyY.press) {
            if (!sVars->hasSavedReplay) {
                if (HUD::sVars->replaySaveEnabled) {
                    if (!UIDialog::sVars->activeDialog) {
                        sVars->saveReplay_CB.Run(this);
                        sVars->hasSavedReplay      = true;
                        sVars->disableResultsInput = true;
                        return;
                    }
                }
            }
        }

        if (controllerInfo->keyStart.press) {
            UIWidgets::sVars->sfxAccept.Play();

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

    // this does not work bc apparently state_moveout actually destroys the object lmao
    // im not sure how to get it to move back in after a moveout, creating a new one doesnt work as it gets set in its default position i assume so
    for (auto hud : GameObject::GetEntities<HUD>(FOR_ALL_ENTITIES)) {
        HUD::MoveIn(hud);
    }

    // decreases alpha for the checkerboard
    if (this->alpha <= 96) {
        this->alpha -= 8;
    }

    if (this->topTextSpeed >= 0) {
        this->topTextSpeed += TO_FIXED(1);
    }
    if (this->topTextSpeed < 0) {
        this->topTextSpeed = 0;
    }

    // triangle movement
    triangleLeftPos.x -= this->topTextSpeed;
    triangleRightPos.x += this->topTextSpeed;

    // top text movement
    this->playerNamePos.x -= this->topTextSpeed;
    this->gotPos.x -= this->topTextSpeed;
    this->throughPos.x += this->topTextSpeed;
    this->actPos.x += this->topTextSpeed;

    // bonus text movement
    if (++this->slidingOutTimer >= 8) {
        // time bonus speed
        if (this->timeBonusSpeed >= 0) {
            this->timeBonusSpeed += TO_FIXED(1);
        }
        if (this->timeBonusSpeed < 0) {
            this->timeBonusSpeed = 0;
        }
        // time bonus movement
        this->timeBonusPos.x -= this->timeBonusSpeed;
        this->timeNumPos.x += this->timeBonusSpeed;
        if (this->slidingOutTimer >= 16) {
            // ring bonus speed
            if (this->ringBonusSpeed >= 0) {
                this->ringBonusSpeed += TO_FIXED(1);
            }
            if (this->ringBonusSpeed < 0) {
                this->ringBonusSpeed = 0;
            }
            // ring bonus movement
            this->ringBonusPos.x -= this->ringBonusSpeed;
            this->ringNumPos.x += this->ringBonusSpeed;
            if (this->slidingOutTimer >= 24) {
                // cool bonus speed
                if (this->coolBonusSpeed >= 0) {
                    this->coolBonusSpeed += TO_FIXED(1);
                }
                if (this->coolBonusSpeed < 0) {
                    this->coolBonusSpeed = 0;
                }
                // cool bonus movement
                this->coolBonusPos.x -= this->coolBonusSpeed;
                this->coolNumPos.x += this->coolBonusSpeed;
            }
        }
    }

    if (this->slidingOutTimer >= 32) {
        // total score speed
        if (this->totalScoreSpeed >= 0) {
            this->totalScoreSpeed += TO_FIXED(1);
        }
        if (this->totalScoreSpeed < 0) {
            this->totalScoreSpeed = 0;
        }
        // total score movement
        this->totalScorePos.x -= this->totalScoreSpeed;
        this->totalNumPos.x += this->totalScoreSpeed;

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

                drawPos->x -= TO_FIXED(8);
                digit *= 10;
            }
        }
    }
    else {
        while (digitCount--) {
            this->numbersAnimator.frameID = 16;
            this->numbersAnimator.DrawSprite(drawPos, true);

            drawPos->x -= TO_FIXED(8);
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
