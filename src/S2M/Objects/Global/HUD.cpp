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
#include "Menu/UIButtonPrompt.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HUD);

void HUD::Update()
{
    this->enableTimeFlash = false;
    this->enableRingFlash = false;
    
    if (sceneInfo->minutes == 9 && !(globals->medalMods & MEDAL_NOTIMEOVER))
        this->enableTimeFlash = true;

    if (this->replayClapAnimator.animationID == 11)
        this->replayClapAnimator.Process();
}
void HUD::LateUpdate()
{
    this->state.Run(this);

    if (globals->gameMode < MODE_TIMEATTACK) {
        Player *player = GameObject::Get<Player>(SLOT_PLAYER1);

        if (sceneInfo->timeEnabled && player->rings >= 50 && player->superState < Player::SuperStateSuper
            && SaveGame::GetSaveRAM()->collectedEmeralds >= 0b01111111) {
            if (SKU->platform == PLATFORM_PC || SKU->platform == PLATFORM_SWITCH || SKU->platform == PLATFORM_DEV)
                HUD::GetActionButtonFrames();

            if (this->actionPromptPos < TO_FIXED(24))
                this->actionPromptPos += TO_FIXED(8);
        }
        else {
            if (this->actionPromptPos > -TO_FIXED(32))
                this->actionPromptPos -= TO_FIXED(8);
        }
    }
    else if (globals->gameMode == MODE_TIMEATTACK) {
        if (sVars->showTAPrompt) {
            if (SKU->platform == PLATFORM_PC || SKU->platform == PLATFORM_SWITCH || SKU->platform == PLATFORM_DEV) {
                HUD::GetButtonFrame(&this->superButtonAnimator, KeyY);
                HUD::GetButtonFrame(&this->saveReplayButtonAnimator, KeyY);
                HUD::GetButtonFrame(&this->thumbsUpButtonAnimator, KeyStart);
            }

            if (this->actionPromptPos < TO_FIXED(24))
                this->actionPromptPos += TO_FIXED(8);
        }
        else {
            if (this->actionPromptPos > -TO_FIXED(64))
                this->actionPromptPos -= TO_FIXED(8);
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
        // encore leftover
        if (sVars->swapCooldown > 0) {
            this->playerIDAnimator.Process();
            this->playerIDAnimator.DrawSprite(&player->position, false);
            --sVars->swapCooldown;
        }

        this->ringFlashFrame = player->rings ? 0 : ((globals->persistentTimer >> 3) & 1);

        this->timeFlashFrame = 0;
        if ((sceneInfo->minutes == 9 && !(globals->medalMods & MEDAL_NOTIMEOVER)))
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
            this->hudElementsAnimator.frameID = this->showMilliseconds ? 5 : 20;
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
                if (sceneInfo->minutes > 9 && globals->medalMods & MEDAL_NOTIMEOVER)
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
        else if (this->actionPromptPos > -TO_FIXED(64) && globals->gameMode == MODE_TIMEATTACK) {
            lifePos.x = TO_FIXED(screenInfo[sceneInfo->currentScreenID].size.x) - this->actionPromptPos;
            lifePos.y = TO_FIXED(20);

            // Draw Replay Save Icon
            this->replayClapAnimator.DrawSprite(&lifePos, true);

            // Draw Replay Save Button
            lifePos.x -= TO_FIXED(28);
            if (sVars->replaySaveEnabled) {
                this->saveReplayButtonAnimator.DrawSprite(&lifePos, true);
            }
            else {
                this->inkEffect = INK_BLEND;
                this->saveReplayButtonAnimator.DrawSprite(&lifePos, true);

                this->inkEffect = INK_NONE;
            }

            lifePos.x = TO_FIXED(screenInfo[sceneInfo->currentScreenID].size.x) - this->actionPromptPos;
            lifePos.y += TO_FIXED(28);

            // Draw Thumbs Up Icon
            this->thumbsUpIconAnimator.DrawSprite(&lifePos, true);

            // Draw Thumbs Up Button
            lifePos.x -= TO_FIXED(28);
            this->thumbsUpButtonAnimator.DrawSprite(&lifePos, true);
        }
        else if (this->actionPromptPos > -TO_FIXED(32)) {
            // Draw Super Icon
            lifePos.x = TO_FIXED(screenInfo[sceneInfo->currentScreenID].size.x) - this->actionPromptPos;
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

        if (globals->useManiaBehavior) {
            // Draw Life Icon "X"
            this->hudElementsAnimator.frameID = 6;
            this->hudElementsAnimator.DrawSprite(&lifePos, true);

            // Draw Lives
            lifePos.x += TO_FIXED(45);
            if (player->lives < 10)
                lifePos.x -= TO_FIXED(1);

            DrawLifeNumbers(&lifePos, lifeCount, 0);
        }
        else { // Draw Life Name
            this->lifeNamesAnimator.DrawSprite(&lifePos, true);

            this->hudElementsAnimator.frameID = 6; // 'x'
            // Draw "x"
            this->hudElementsAnimator.DrawSprite(&lifePos, true);

            // Draw Life Count
            lifePos.x += 0x290000;
            DrawLifeNumbers(&lifePos, lifeCount, 1);
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
        this->actionPromptPos = -TO_FIXED(32);

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

        this->playerIDAnimator.SetAnimation(sVars->aniFrames, 2 ? 13 : 8, true, 0);
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
        drawPos->x -= TO_FIXED(7);
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
    int32 gamepadType = UIButtonPrompt::GetGamepadType();
    if (API::GetConfirmButtonFlip() && buttonID <= 1)
        buttonID ^= 1;
    
    // Gamepad
    if (gamepadType != UIButtonPrompt::UIBUTTONPROMPT_KEYBOARD && (gamepadType < UIButtonPrompt::UIBUTTONPROMPT_KEYBOARD_FR || gamepadType > UIButtonPrompt::UIBUTTONPROMPT_KEYBOARD_SP)) {
        animator->SetAnimation(sVars->superButtonFrames, gamepadType, true, buttonID);
    }
    else {
        // Keyboard
        Player *player = GameObject::Get<Player>(SLOT_PLAYER1);
    
        int32 id     = Input::GetInputDeviceID(player->controllerID);
        int32 contID = id == Input::INPUT_UNASSIGNED ? Input::CONT_P1 : player->controllerID;
    
        int32 map = 0;
        switch (buttonID) {
            default: break;
            case 0: map = controllerInfo[contID].keyA.keyMap; break;
            case 1: map = controllerInfo[contID].keyB.keyMap; break;
            case 2: map = controllerInfo[contID].keyX.keyMap; break;
            case 3: map = controllerInfo[contID].keyY.keyMap; break;
            case 4: map = controllerInfo[contID].keyStart.keyMap; break;
        }
    
        int32 frame = UIButtonPrompt::MappingsToFrame(map);
        animator->SetAnimation(sVars->superButtonFrames, 1, true, frame);
    }
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

    state       = &this->state;
    scoreOffset = &this->scoreOffset;
    timeOffset  = &this->timeOffset;
    ringsOffset = &this->ringsOffset;
    lifeOffset  = &this->lifeOffset;
    max         = &this->maxOffset;

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

    state       = &this->state;
    scoreOffset = &this->scoreOffset;
    timeOffset  = &this->timeOffset;
    ringsOffset = &this->ringsOffset;
    lifeOffset  = &this->lifeOffset;

    if (scoreOffset->x > -TO_FIXED(112))
        scoreOffset->x -= TO_FIXED(8);

    if (timeOffset->x > -TO_FIXED(168))
        timeOffset->x -= TO_FIXED(12);

    if (ringsOffset->x > -TO_FIXED(224))
        ringsOffset->x -= TO_FIXED(16);

    if (lifeOffset->x > -TO_FIXED(280))
        lifeOffset->x -= TO_FIXED(20);

    if (globals->gameMode != MODE_TIMEATTACK) { // this is done because the replay prompts in time attack are done with hud, but if its destroyed then they will not appear, this is just a simple way of getting around it
        if (lifeOffset->x <= -TO_FIXED(280)) {
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

void HUD::MoveIn(HUD *hud)
{
    hud->maxOffset = hud->scoreOffset.x;
    hud->scoreOffset.x -= TO_FIXED(0x100);
    hud->timeOffset.x -= TO_FIXED(0x110);
    hud->ringsOffset.x -= TO_FIXED(0x120);
    hud->lifeOffset.x -= TO_FIXED(0x130);

    hud->state.Set(&HUD::State_MoveIn);
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
