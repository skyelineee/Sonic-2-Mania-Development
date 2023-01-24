// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HUD Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HUD.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "SaveGame.hpp"
#include "DebugMode.hpp"
#include "ActClear.hpp"
#include "GameOver.hpp"

// #include "Menu/UIButtonPrompt.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HUD);

void HUD::Update()
{
    this->enableTimeFlash = false;
    this->enableRingFlash = false;
    
    if (sceneInfo->minutes == 9)
        this->enableTimeFlash = true;

    if (this->replayClapAnimator.animationID == 11)
        this->replayClapAnimator.Process();
}
void HUD::LateUpdate()
{
    if (globals->gameMode == MODE_COMPETITION) {
        for (this->screenID = 0; this->screenID < Graphics::GetVideoSetting(VIDEOSETTING_SCREENCOUNT); ++this->screenID) {
            this->vsStates[this->screenID].Run(this);
        }
    }
    else {
        this->state.Run(this);
    }

    if (globals->gameMode < MODE_TIMEATTACK) {
        Player *player = GameObject::Get<Player>(SLOT_PLAYER1);

        if (sceneInfo->timeEnabled && player->rings >= 50 && player->superState < Player::SuperStateSuper
            && SaveGame::GetSaveRAM()->collectedEmeralds >= 0b01111111) {
            if (SKU->platform == PLATFORM_PC || SKU->platform == PLATFORM_SWITCH || SKU->platform == PLATFORM_DEV)
                HUD::GetActionButtonFrames();

            if (this->superButtonPos < TO_FIXED(24))
                this->superButtonPos += TO_FIXED(8);
        }
        else {
            if (this->superButtonPos > -TO_FIXED(32))
                this->superButtonPos -= TO_FIXED(8);
        }
    }
    else if (globals->gameMode == MODE_TIMEATTACK) {
        if (sVars->showTAPrompt) {
            if (SKU->platform == PLATFORM_PC || SKU->platform == PLATFORM_SWITCH || SKU->platform == PLATFORM_DEV) {
                HUD::GetButtonFrame(&this->superButtonAnimator, KeyY);
                HUD::GetButtonFrame(&this->saveReplayButtonAnimator, KeyY);
                HUD::GetButtonFrame(&this->thumbsUpButtonAnimator, KeyStart);
            }

            if (this->superButtonPos < TO_FIXED(24))
                this->superButtonPos += TO_FIXED(8);
        }
        else {
            if (this->superButtonPos > -TO_FIXED(64))
                this->superButtonPos -= TO_FIXED(8);
        }
    }
}
void HUD::StaticUpdate() {}
void HUD::Draw()
{
    Player *player = GameObject::Get<Player>(sceneInfo->currentScreenID);

    Vector2 lifePos;
    Vector2 scoreOffset, timeOffset, ringsOffset, lifeOffset;

    scoreOffset.x = this->scoreOffset.x;
    scoreOffset.y = this->scoreOffset.y;
    timeOffset.x  = this->timeOffset.x;
    timeOffset.y  = this->timeOffset.y;
    ringsOffset.x = this->ringsOffset.x;
    ringsOffset.y = this->ringsOffset.y;
    lifeOffset.x  = this->lifeOffset.x;
    lifeOffset.y  = this->lifeOffset.y;

    {
        if (globals->gameMode == MODE_COMPETITION) {
            scoreOffset.x = this->vsScoreOffsets[sceneInfo->currentScreenID].x;
            scoreOffset.y = this->vsScoreOffsets[sceneInfo->currentScreenID].y;
            timeOffset.x  = this->vsTimeOffsets[sceneInfo->currentScreenID].x;
            timeOffset.y  = this->vsTimeOffsets[sceneInfo->currentScreenID].y;
            ringsOffset.x = this->vsRingsOffsets[sceneInfo->currentScreenID].x;
            ringsOffset.y = this->vsRingsOffsets[sceneInfo->currentScreenID].y;
            lifeOffset.x  = this->vsLifeOffsets[sceneInfo->currentScreenID].x;
            lifeOffset.y  = this->vsLifeOffsets[sceneInfo->currentScreenID].y;

            if (globals->useManiaBehavior) {
                for (auto playerPtr : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                    if (playerPtr != player) {
                        this->playerIDAnimator.frameID = playerPtr->playerID;
                        this->playerIDAnimator.DrawSprite(&playerPtr->position, false);
                    }
                }
            }
        }
        else {
            // encore leftover
            if (sVars->swapCooldown > 0) {
                this->playerIDAnimator.Process();
                this->playerIDAnimator.DrawSprite(&player->position, false);
                --sVars->swapCooldown;
            }
        }

        this->ringFlashFrame = player->rings ? 0 : ((globals->persistentTimer >> 3) & 1);

        this->timeFlashFrame = 0;
        if (sceneInfo->minutes == 9)
            this->timeFlashFrame = (globals->persistentTimer >> 3) & 1;

        // Draw "Score"
        this->hudElementsAnimator.frameID = 0;
        this->hudElementsAnimator.DrawSprite(&scoreOffset, true);

        // Draw Score
        lifePos.x = scoreOffset.x + TO_FIXED(97);
        lifePos.y = scoreOffset.y + TO_FIXED(14);
        DrawNumbersBase10(&lifePos, player->score, 0);

        // Draw "Time"
        this->hudElementsAnimator.frameID = this->timeFlashFrame + 1;
        this->hudElementsAnimator.DrawSprite(&timeOffset, true);

        if (!this->enableTimeFlash || globals->persistentTimer & 8) {
            // Draw ":"
            lifePos.x                         = timeOffset.x + TO_FIXED(52);
            lifePos.y                         = timeOffset.y - TO_FIXED(2);
            this->hudElementsAnimator.frameID = this->showMilliseconds ? 12 : 20;
            this->hudElementsAnimator.DrawSprite(&lifePos, true);

            this->numbersAnimator.frameID = 0;
            lifePos.y                     = timeOffset.y + (this->numbersAnimator.GetFrame(sVars->aniFrames)->height << 16);
            if (this->showMilliseconds) {
                // Draw Milliseconds
                lifePos.x = timeOffset.x + TO_FIXED(97);
                DrawNumbersBase10(&lifePos, sceneInfo->milliseconds, 2);

                lifePos.x -= TO_FIXED(8);
            }
            else {
                lifePos.x = timeOffset.x + 0x4B0000;
            }

            if (sceneInfo->minutes > 9) {
                // Draw Seconds
                DrawNumbersBase10(&lifePos, 59, 2);
                lifePos.x -= TO_FIXED(9);

                // Draw Minutes
                DrawNumbersBase10(&lifePos, 9, 1);
            }
            else {
                // Draw Seconds
                DrawNumbersBase10(&lifePos, sceneInfo->seconds, 2);
                lifePos.x -= TO_FIXED(9);

                // Draw Minutes
                if (globals->medalMods & MEDAL_NOTIMEOVER)
                    DrawNumbersBase10(&lifePos, sceneInfo->minutes, 2);
                else
                    DrawNumbersBase10(&lifePos, sceneInfo->minutes, 1);
            }
        }

        // Draw "Rings"
        this->hudElementsAnimator.frameID = this->ringFlashFrame + 3;
        this->hudElementsAnimator.DrawSprite(&ringsOffset, true);

        if (!this->enableRingFlash || globals->persistentTimer & 8) {
            // Draw Rings
            this->numbersAnimator.frameID = 0;
            lifePos.x                     = ringsOffset.x + (this->showMilliseconds ? TO_FIXED(97) : TO_FIXED(14));
            lifePos.y                     = ringsOffset.y + (this->numbersAnimator.GetFrame(sVars->aniFrames)->height << 16);
            if (player->hyperRing)
                DrawNumbersHyperRing(&lifePos, player->rings);
            else
                DrawNumbersBase10(&lifePos, player->rings, 0);
        }

        if (sceneInfo->debugMode) {
            if (player->camera) {
                // Draw Camera YPos
                lifePos.x = (screenInfo[player->camera->screenID].size.x - 16) << 16;
                lifePos.y = TO_FIXED(24);
                DrawNumbersBase16(&lifePos, screenInfo[player->camera->screenID].position.y);

                // Draw Camera XPos
                lifePos.x -= TO_FIXED(9);
                DrawNumbersBase16(&lifePos, screenInfo[player->camera->screenID].position.x);

                // Draw Player YPos
                lifePos.x = (screenInfo[player->camera->screenID].size.x - 16) << 16;
                lifePos.y += TO_FIXED(16);
                DrawNumbersBase16(&lifePos, player->position.y >> 0x10);

                // Draw Player XPos
                lifePos.x -= TO_FIXED(9);
                DrawNumbersBase16(&lifePos, player->position.x >> 0x10);
            }
        }
        else if (this->superButtonPos > -TO_FIXED(64) && globals->gameMode == MODE_TIMEATTACK) {
            // RIP replay buttons
            lifePos.x = (screenInfo[sceneInfo->currentScreenID].size.x << 16) - this->superButtonPos;
            lifePos.y = TO_FIXED(20);
        }
        else if (this->superButtonPos > -TO_FIXED(32)) {
            // Draw Super Icon
            lifePos.x = (screenInfo[sceneInfo->currentScreenID].size.x << 16) - this->superButtonPos;
            lifePos.y = TO_FIXED(20);
            this->superIconAnimator.DrawSprite(&lifePos, true);

            lifePos.x -= TO_FIXED(20);
            bool32 canSuper = true;

            if (Player::sVars->canSuperCB)
                canSuper = Player::sVars->canSuperCB(true);

            // Draw Super Button
            if (player->state.Matches(&Player::State_Air) && player->jumpAbilityState == 1 && canSuper) {
                this->superButtonAnimator.DrawSprite(&lifePos, true);
            }
            else {
                this->inkEffect = INK_BLEND;
                this->superButtonAnimator.DrawSprite(&lifePos, true);
                this->inkEffect = INK_NONE;
            }
        }
    }

    {
        // Draw Life Icon (aka the Leader Icon if in encore mode)
        int32 lifeIconFrame = -1;
        lifePos.x           = lifeOffset.x;
        lifePos.y           = lifeOffset.y;
        int32 lifeCount     = 0;

        lifeCount = this->lives[player->playerID];

        int32 charID = player->characterID;
        for (; charID > 0; ++lifeIconFrame) charID >>= 1;
        this->lifeIconAnimator.frameID  = lifeIconFrame;
        this->lifeNamesAnimator.frameID = lifeIconFrame;

        this->lifeFrameIDs[player->playerID] = lifeIconFrame;
        this->lives[player->playerID]        = player->lives;

        if (GET_CHARACTER_ID(1) == ID_SONIC && player->superState == Player::SuperStateSuper) {
            lifeIconAnimator.frameID += 3;
        }
        
        if (GET_CHARACTER_ID(1) == ID_SONIC && player->superState == Player::SuperStateDone) {
            lifeIconAnimator.frameID = 0;
        }

        if ((globals->playerID & 0xFF) == ID_TAILS && (globals->secrets & SECRET_REGIONSWAP))
            this->lifeNamesAnimator.frameID = this->lifeNamesAnimator.frameCount - 1; // miles

        if (this->lifeIconAnimator.frameID < 0) {
            this->lifeIconAnimator.frameID = this->lifeFrameIDs[player->playerID];
            lifeCount--;
        }

        this->lifeIconAnimator.DrawSprite(&lifePos, true);

        if (globals->gameMode == MODE_ENCORE) {
            for (int32 p = 0; p < PLAYER_COUNT; ++p) {
                if (sVars->stockFlashTimers[p] > 0)
                    sVars->stockFlashTimers[p]--;
            }

            lifePos.x += TO_FIXED(20);
            Player *sidekick = GameObject::Get<Player>(SLOT_PLAYER2);
            if (sidekick->classID) {
                // Draw Buddy Icon
                int32 charID       = sidekick->characterID;
                int32 stockFrameID = -1;
                for (stockFrameID = -1; charID > 0; ++stockFrameID) charID >>= 1;
                this->lifeIconAnimator.frameID = stockFrameID;
                if (stockFrameID >= 0 && !(sVars->stockFlashTimers[0] & 4)) {
                    if ((!sidekick->state.Matches(&Player::State_Death) && !sidekick->state.Matches(&Player::State_Drown)
                         && !sidekick->state.Matches(&Player::State_DeathHold))
                        || !sidekick->abilityValues[0]) {
                        this->lifeIconAnimator.DrawSprite(&lifePos, true);
                    }
                }

                // Draw Stock Icons
                lifePos.x += TO_FIXED(20);
                this->lifeIconAnimator.SetAnimation(sVars->aniFrames, 12, true, 0);
                for (int32 i = 0; i < 3; ++i) {
                    stockFrameID = -1;
                    int32 stock  = (globals->stock >> (i * 8)) & 0xFF;
                    while (stock > 0) {
                        stock >>= 1;
                        ++stockFrameID;
                    }

                    this->lifeIconAnimator.frameID = stockFrameID;
                    if (stockFrameID >= 0 && !(sVars->stockFlashTimers[i + 1] & 4))
                        this->lifeIconAnimator.DrawSprite(&lifePos, true);

                    lifePos.x += TO_FIXED(16);
                }

                this->lifeIconAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
            }
        }
        else {
            if (globals->useManiaBehavior) {
                // Draw Life Icon "X"
                this->hudElementsAnimator.frameID = 14;
                this->hudElementsAnimator.DrawSprite(&lifePos, true);

                // Draw Lives
                lifePos.x += TO_FIXED(44);
                if (player->lives < 10)
                    lifePos.x -= TO_FIXED(0);

                DrawLifeNumbers(&lifePos, lifeCount, 0);
            }
            else { // Draw Life Name
                this->lifeNamesAnimator.DrawSprite(&lifePos, true);

                this->hudElementsAnimator.frameID = 14; // 'x'
                // Draw "x"
                this->hudElementsAnimator.DrawSprite(&lifePos, true);

                // Draw Life Count
                lifePos.x += 0x290000;
                DrawLifeNumbers(&lifePos, lifeCount, 1);
            }
        }
    }
}

void HUD::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        ActClear::sVars->disableTimeBonus = false;

        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawGroup = Zone::sVars->hudDrawGroup;

        this->scoreOffset.x  = TO_FIXED(16);
        this->scoreOffset.y  = TO_FIXED(12);
        this->timeOffset.x   = TO_FIXED(16);
        this->timeOffset.y   = TO_FIXED(28);
        this->ringsOffset.x  = TO_FIXED(16);
        this->ringsOffset.y  = TO_FIXED(44);
        this->lifeOffset.x   = TO_FIXED(16);
        this->lifeOffset.y   = (screenInfo->size.y - 12) << 16;
        this->superButtonPos = -TO_FIXED(32);

        for (int32 p = 0; p < PLAYER_COUNT; ++p) {
            this->vsScoreOffsets[p].x = this->scoreOffset.x;
            this->vsScoreOffsets[p].y = this->scoreOffset.y;
            this->vsTimeOffsets[p].x  = this->timeOffset.x;
            this->vsTimeOffsets[p].y  = this->timeOffset.y;
            this->vsRingsOffsets[p].x = this->ringsOffset.x;
            this->vsRingsOffsets[p].y = this->ringsOffset.y;
            this->vsLifeOffsets[p].x  = this->lifeOffset.x;
            this->vsLifeOffsets[p].y  = this->lifeOffset.y;
        }

        this->hudElementsAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->numbersAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->hyperNumbersAnimator.SetAnimation(sVars->aniFrames, 9, true, 0);
        this->lifeIconAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
        this->lifeNamesAnimator.SetAnimation(sVars->aniFrames, 15, true, 0);
        if (globals->useManiaBehavior)
            this->lifeNumbersAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        else
            this->lifeNumbersAnimator.SetAnimation(sVars->aniFrames, 14, true, 0);

        this->playerIDAnimator.SetAnimation(sVars->aniFrames, globals->gameMode == MODE_ENCORE ? 13 : 8, true, 0);
        this->thumbsUpIconAnimator.SetAnimation(sVars->aniFrames, 10, true, 2);
        this->replayClapAnimator.SetAnimation(sVars->aniFrames, 10, true, 1);
        this->superIconAnimator.SetAnimation(sVars->superButtonFrames, 0, true, 0);
        this->showMilliseconds = true;

        HUD::GetActionButtonFrames();

        Dev::AddViewableVariable("Show HUD", &this->visible, Dev::VIEWVAR_UINT8, false, true);
    }
}

void HUD::StageLoad()
{
    switch GET_CHARACTER_ID(1) {
        default: break;
        case ID_SONIC: sVars->aniFrames.Load("Global/HUDSonic.bin", SCOPE_STAGE); break;
        case ID_TAILS: sVars->aniFrames.Load("Global/HUDTails.bin", SCOPE_STAGE); break;
        case ID_KNUCKLES: sVars->aniFrames.Load("Global/HUDKnux.bin", SCOPE_STAGE); break;
    }
    sVars->superButtonFrames.Load("Global/SuperButtons.bin", SCOPE_STAGE);

    sVars->sfxClick.Get("Stage/Click.wav");
    sVars->sfxStarpost.Get("Global/StarPost.wav");

    sVars->showTAPrompt = false;
}

void HUD::DrawLifeNumbers(RSDK::Vector2 *drawPos, int32 value, int32 digitCount)
{
    if (!digitCount && value > 0) {
        int32 v = value;
        while (v > 0) {
            ++digitCount;
            v /= 10;
        }
    }
    else {
        if (!digitCount && value <= 0)
            digitCount = 1;
    }

    int32 digit = 1;
    while (digitCount--) {
        this->lifeNumbersAnimator.frameID = value / digit % 10;
        this->lifeNumbersAnimator.DrawSprite(drawPos, true);
        digit *= 10;
        drawPos->x -= TO_FIXED(6);
    }
}
void HUD::DrawNumbersBase10(RSDK::Vector2 *drawPos, int32 value, int32 digitCount)
{
    if (!digitCount && value > 0) {
        int32 v = value;
        while (v > 0) {
            ++digitCount;
            v /= 10;
        }
    }
    else {
        if (!digitCount && value <= 0)
            digitCount = 1;
    }

    int32 digit = 1;
    while (digitCount--) {
        this->numbersAnimator.frameID = value / digit % 10;
        this->numbersAnimator.DrawSprite(drawPos, true);
        digit *= 10;
        drawPos->x -= TO_FIXED(8);
    }
}
void HUD::DrawNumbersBase16(RSDK::Vector2 *drawPos, int32 value)
{
    int32 mult = 1;
    for (int32 i = 4; i; --i) {
        this->numbersAnimator.frameID = value / mult & 0xF;
        this->numbersAnimator.DrawSprite(drawPos, true);
        drawPos->x -= TO_FIXED(8);
        mult *= 16;
    }
}
void HUD::DrawNumbersHyperRing(RSDK::Vector2 *drawPos, int32 value)
{

    int32 cnt   = 0;
    int32 mult  = 1;
    int32 mult2 = 1;
    if (value <= 0) {
        cnt = 1;
    }
    else {
        int32 v = value;
        while (v) {
            ++cnt;
            v /= 10;
        }
    }

    if (cnt > 0) {
        while (cnt--) {
            this->hyperNumbersAnimator.frameID = value / mult % 10;
            this->hyperNumbersAnimator.DrawSprite(drawPos, true);
            drawPos->x -= TO_FIXED(8);
            mult = 10 * mult2;
            mult2 *= 10;
        }
    }

    this->hyperNumbersAnimator.frameID = 10;
    drawPos->x -= TO_FIXED(4);
    hyperNumbersAnimator.DrawSprite(drawPos, true);
}

void HUD::GetButtonFrame(RSDK::Animator *animator, int32 buttonID)
{
    // int32 gamepadType = UIButtonPrompt::GetGamepadType();
    //
    // // Gamepad
    // if (gamepadType != UIButtonPrompt::Keyboard && (gamepadType < UIButtonPrompt::Keyboard_FR || gamepadType > UIButtonPrompt::Keyboard_SP)) {
    //     animator->SetAnimation(sVars->superButtonFrames, gamepadType, true, buttonID);
    // }
    // else {
    //     // Keyboard
    //     Player *player = GameObject::Get<Player>(SLOT_PLAYER1);
    //
    //     int32 id     = Input::GetInputDeviceID(player->controllerID);
    //     int32 contID = id == Input::INPUT_UNASSIGNED ? Input::CONT_P1 : player->controllerID;
    //
    //     int32 map = 0;
    //     switch (buttonID) {
    //         default: break;
    //         case 0: map = controllerInfo[contID].keyA.keyMap; break;
    //         case 1: map = controllerInfo[contID].keyB.keyMap; break;
    //         case 2: map = controllerInfo[contID].keyX.keyMap; break;
    //         case 3: map = controllerInfo[contID].keyY.keyMap; break;
    //         case 4: map = controllerInfo[contID].keyStart.keyMap; break;
    //     }
    //
    //     int32 frame = UIButtonPrompt::MappingsToFrame(map);
    //     unused(frame);
    //
    //     animator->SetAnimation(sVars->superButtonFrames, 1, true, buttonID);
    // }
}
void HUD::GetActionButtonFrames()
{
    GetButtonFrame(&this->superButtonAnimator, KeyY);
    GetButtonFrame(&this->saveReplayButtonAnimator, KeyY);
    GetButtonFrame(&this->thumbsUpButtonAnimator, KeyStart);
}

void HUD::State_MoveIn()
{
    SET_CURRENT_STATE();

    StateMachine<HUD> *state = nullptr;
    Vector2 *scoreOffset = nullptr, *timeOffset = nullptr, *ringsOffset = nullptr, *lifeOffset = nullptr;
    int32 *max = nullptr;

    if (globals->gameMode == MODE_COMPETITION) {
        state       = &this->vsStates[sceneInfo->currentScreenID];
        scoreOffset = &this->vsScoreOffsets[sceneInfo->currentScreenID];
        timeOffset  = &this->vsTimeOffsets[sceneInfo->currentScreenID];
        ringsOffset = &this->vsRingsOffsets[sceneInfo->currentScreenID];
        lifeOffset  = &this->vsLifeOffsets[sceneInfo->currentScreenID];
        max         = &this->vsMaxOffsets[sceneInfo->currentScreenID];
    }
    else {
        state       = &this->state;
        scoreOffset = &this->scoreOffset;
        timeOffset  = &this->timeOffset;
        ringsOffset = &this->ringsOffset;
        lifeOffset  = &this->lifeOffset;
        max         = &this->maxOffset;
    }

    if (scoreOffset->x < *max)
        scoreOffset->x += TO_FIXED(8);

    if (timeOffset->x < *max)
        timeOffset->x += TO_FIXED(8);

    if (ringsOffset->x < *max)
        ringsOffset->x += TO_FIXED(8);

    if (lifeOffset->x < *max)
        lifeOffset->x += TO_FIXED(8);
    else
        state->Set(nullptr);
}
void HUD::State_MoveOut()
{
    SET_CURRENT_STATE();

    Vector2 *scoreOffset = nullptr, *timeOffset = nullptr, *ringsOffset = nullptr, *lifeOffset = nullptr;
    StateMachine<HUD> *state = nullptr;

    if (globals->gameMode == MODE_COMPETITION) {
        state       = &this->vsStates[this->screenID];
        scoreOffset = &this->vsScoreOffsets[this->screenID];
        timeOffset  = &this->vsTimeOffsets[this->screenID];
        ringsOffset = &this->vsRingsOffsets[this->screenID];
        lifeOffset  = &this->vsLifeOffsets[this->screenID];
    }
    else {
        state       = &this->state;
        scoreOffset = &this->scoreOffset;
        timeOffset  = &this->timeOffset;
        ringsOffset = &this->ringsOffset;
        lifeOffset  = &this->lifeOffset;
    }

    scoreOffset->x -= TO_FIXED(8);
    if (timeOffset->x - scoreOffset->x > TO_FIXED(16))
        timeOffset->x -= TO_FIXED(8);

    if (ringsOffset->x - timeOffset->x > TO_FIXED(16))
        ringsOffset->x -= TO_FIXED(8);

    if (lifeOffset->x - ringsOffset->x > TO_FIXED(16))
        lifeOffset->x -= TO_FIXED(8);

    if (lifeOffset->x < -TO_FIXED(80)) {
        if (globals->gameMode == MODE_COMPETITION) {
            state->Set(nullptr);
        }
        else {
            this->Destroy();
        }
    }
}

void HUD::EnableRingFlash()
{
    for (auto hud : GameObject::GetEntities<HUD>(FOR_ALL_ENTITIES)) {
        if (hud)
            hud->enableRingFlash = true;
        break;
    }
}

int32 HUD::CharacterIndexFromID(int32 characterID)
{
    int32 id = -1;
    for (int32 i = characterID; i > 0; ++id, i >>= 1)
        ;
    return id;
}

void HUD::MoveIn()
{
    this->maxOffset = this->scoreOffset.x;
    this->scoreOffset.x -= TO_FIXED(0x100);
    this->timeOffset.x -= TO_FIXED(0x110);
    this->ringsOffset.x -= TO_FIXED(0x120);
    this->lifeOffset.x -= TO_FIXED(0x130);

    this->state.Set(&HUD::State_MoveIn);
}

#if RETRO_INCLUDE_EDITOR
void HUD::EditorDraw()
{
    this->hudElementsAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->hudElementsAnimator.DrawSprite(&this->position, false);
}

void HUD::EditorLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void HUD::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(HUD);

    sVars->aniFrames.Init();
    sVars->superButtonFrames.Init();

    sVars->sfxClick.Init();
    sVars->sfxStarpost.Init();
}
#endif

void HUD::Serialize() {}

} // namespace GameLogic
