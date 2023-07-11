// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_SpecialClear Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_SpecialClear.hpp"
#include "HP_Setup.hpp"
#include "HP_Player.hpp"
#include "Global/Zone.hpp"
#include "Global/Music.hpp"
#include "Global/SaveGame.hpp"
#include "Global/HUD.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_SpecialClear);

void HP_SpecialClear::Update() { this->state.Run(this); }
void HP_SpecialClear::LateUpdate() {}
void HP_SpecialClear::StaticUpdate() {}
void HP_SpecialClear::Draw() { this->stateDraw.Run(this); }

void HP_SpecialClear::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawGroup = Zone::sVars->hudDrawGroup;

        this->state.Set(&HP_SpecialClear::State_FadeIn);
        this->stateDraw.Set(&HP_SpecialClear::Draw_FadeIn);
        this->fillColor = 0xE0E0E0;

        SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();
        this->score       = saveRAM->score;
        this->score1UP    = saveRAM->score1UP;
        this->lives       = saveRAM->lives;

        this->uiElementsAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->numbersAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->emeraldsAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    }
}

void HP_SpecialClear::StageLoad()
{
    sVars->aniFrames.Load("Special/Results.bin", SCOPE_STAGE);

    sVars->sfxScoreAdd.Get("Global/ScoreAdd.wav");
    sVars->sfxScoreTotal.Get("Global/ScoreTotal.wav");
    sVars->sfxWarp.Get("Global/SpecialWarp.wav");
    sVars->sfxEvent.Get("Special/Event.wav");

    sVars->hiddenPalaceEnabled = true;
}

void HP_SpecialClear::GiveScoreBonus(int32 score)
{
    this->score += score;
    if (this->score > 9999999)
        this->score = 9999999;

    if (this->score >= this->score1UP) {
        if (this->lives < 99)
            this->lives++;

        Music::PlayJingle(Music::TRACK_1UP);

        Music::sVars->nextTrack = Music::TRACK_NONE;
        while (this->score1UP <= this->score) this->score1UP += 50000;
    }
}

void HP_SpecialClear::DrawNumbers(RSDK::Vector2 *drawPos, int32 value, int32 digitCount)
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
        drawPos->x -= this->numbersAnimator.GetFrame(sVars->aniFrames)->width << 16;
    }
}

// States
void HP_SpecialClear::State_FadeIn()
{
    SET_CURRENT_STATE();

    if (this->timer >= 256) {
        SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();

        if (HP_Setup::sVars->gotEmerald) {
            if (Zone::sVars->actID != HP_Setup::ActNone)
                SaveGame::SetEmerald(Zone::sVars->actID);

            if (SaveGame::GetEmeralds(SaveGame::EmeraldAny)) {
                this->resultsTextTop = HP_SpecialClear::MessageAllEmeralds;
                this->resultsTextMid = HP_SpecialClear::MessageAllEmeralds;
                this->messageAnimator.SetAnimation(sVars->aniFrames, 4 + (5 * HUD::CharacterIndexFromID(GET_CHARACTER_ID(1))), true, 0);
            }
            else {
                this->resultsTextTop = HP_SpecialClear::MessageEmeraldGet;
                this->resultsTextMid = HP_SpecialClear::MessageEmeraldGet;
                this->messageAnimator.SetAnimation(sVars->aniFrames, 3 + (5 * HUD::CharacterIndexFromID(GET_CHARACTER_ID(1))), true, 0);
            }
            this->gemBonus = 10000;

            saveRAM->nextSpecialStage = (saveRAM->nextSpecialStage + 1) % 7;
        }
        else {
            this->resultsTextTop = HP_SpecialClear::MessageNone;
            this->resultsTextMid = HP_SpecialClear::MessageFailed;
            this->messageAnimator.SetAnimation(sVars->aniFrames, 3 + (5 * HUD::CharacterIndexFromID(GET_CHARACTER_ID(1))), true, 0);
            this->gemBonus       = 0;
        }
        this->resultsTextBottom = 0;

        HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
        HP_Player *player2 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER2);

        this->ringBonus[0] = player1->rings;
        this->ringBonus[1] = 0;
        if (GET_CHARACTER_ID(2))
            this->ringBonus[1] = player2->rings;

        this->timer = 256;

        for (int32 e = 0; e < RESERVE_ENTITY_COUNT + SCENEENTITY_COUNT; ++e) {
            if (e != this->Slot())
                GameObject::Get(e)->Destroy();
        }

        for (int32 l = 0; l < LAYER_COUNT; ++l) {
            TileLayer *layer = SceneLayer::GetTileLayer(l);
            if (layer)
                layer->drawGroup[0] = DRAWGROUP_COUNT;
        }

        // this->controlMode = 0;
        // keyDown[0].left   = false;
        // keyDown[0].right  = false;
        this->direction  = FLIP_NONE;
        this->textTopPos = -TO_FIXED(384);
        this->textMidPos = TO_FIXED(384);
        this->scorePos   = TO_FIXED(640 + 128);
        this->ringPos[0] = TO_FIXED(656 + 128);
        this->ringPos[1] = TO_FIXED(672 + 128);
        this->gemPos     = TO_FIXED(688 + 128);
        this->inkEffect  = INK_ALPHA;
        Music::PlayTrack(Music::TRACK_ACTCLEAR);

        this->state.Set(&HP_SpecialClear::State_FadeIdle);
    }
    else {
        this->timer += 8;
    }
}
void HP_SpecialClear::State_FadeIdle()
{
    SET_CURRENT_STATE();

    if (++this->timer >= 256 + 15) {
        this->timer = 0;
        this->state.Set(&HP_SpecialClear::State_EnterText);
        this->stateDraw.Set(&HP_SpecialClear::Draw_Results);

        this->alphaTimer = (this->alphaTimer + 16) & 0x1FF;
    }
}
void HP_SpecialClear::State_EnterText()
{
    SET_CURRENT_STATE();

    this->alphaTimer = (this->alphaTimer + 16) & 0x1FF;

    if (this->textTopPos < TO_FIXED(0))
        this->textTopPos += TO_FIXED(16);

    if (this->textMidPos > TO_FIXED(0))
        this->textMidPos -= TO_FIXED(16);

    if (this->scorePos > TO_FIXED(0))
        this->scorePos -= TO_FIXED(16);

    if (this->ringPos[0] > TO_FIXED(0))
        this->ringPos[0] -= TO_FIXED(16);

    if (this->ringPos[1] > TO_FIXED(0))
        this->ringPos[1] -= TO_FIXED(16);

    if (this->gemPos > TO_FIXED(0))
        this->gemPos -= TO_FIXED(16);
    else
        this->state.Set(&HP_SpecialClear::State_EnterEmeralds);
}
void HP_SpecialClear::State_EnterEmeralds()
{
    SET_CURRENT_STATE();

    this->alphaTimer = (this->alphaTimer + 16) & 0x1FF;

    if (this->emeraldAlpha < 256)
        this->emeraldAlpha += 8;

    if (++this->timer >= 300) {
        this->timer = 0;
        this->state.Set(&HP_SpecialClear::State_TallyScore);
    }
}
void HP_SpecialClear::State_TallyScore()
{
    SET_CURRENT_STATE();

    this->alphaTimer = (this->alphaTimer + 16) & 0x1FF;

    if (this->ringBonus[0] > 0) {
        this->ringBonus[0]--;
        this->GiveScoreBonus(100);
    }

    if (this->ringBonus[1] > 0) {
        this->ringBonus[1]--;
        this->GiveScoreBonus(100);
    }

    if (this->gemBonus > 0) {
        this->gemBonus -= 100;
        this->GiveScoreBonus(100);
    }

    if (controllerInfo->keyA.press || controllerInfo->keyStart.press) {
        this->GiveScoreBonus(this->gemBonus + (this->ringBonus[0] * 100) + (this->ringBonus[1] * 100));
        this->ringBonus[0] = 0;
        this->ringBonus[1] = 0;
        this->gemBonus     = 0;
    }

    if (this->gemBonus + this->ringBonus[0] + this->ringBonus[1] <= 0) {
        this->timer = 0;

        SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();
        this->state.Set(&HP_SpecialClear::State_Failed);
        if (HP_Setup::sVars->gotEmerald) {
            if (SaveGame::GetEmeralds(SaveGame::EmeraldBoth)) { // hyper !!!
                this->state.Set(&HP_SpecialClear::State_WinShowReward);
            }
            else if (SaveGame::GetEmeralds(SaveGame::EmeraldAny)) {
                switch (GET_CHARACTER_ID(1)) {
                    default:
                    case ID_SONIC:
                    case ID_KNUCKLES: this->state.Set(&HP_SpecialClear::State_WinShowReward); break;

                    case ID_TAILS:
                        // if (globals->superTails)
                        this->state.Set(&HP_SpecialClear::State_WinShowReward);
                        break;
                }
            }
        }

        sVars->sfxScoreTotal.Play();
    }
    else if (++this->timer == 2) {
        this->timer = 0;
        sVars->sfxScoreAdd.Play();
    }
}
void HP_SpecialClear::State_Failed()
{
    SET_CURRENT_STATE();

    this->alphaTimer = (this->alphaTimer + 16) & 0x1FF;

    if (++this->timer >= 160) {
        this->timer     = 0;
        this->fillColor = 0xF8F8F8;
        sVars->sfxWarp.Play();
        this->state.Set(&HP_SpecialClear::State_FadeOut);
        this->stateDraw.Set(&HP_SpecialClear::Draw_FadeOut);
    }
}
void HP_SpecialClear::State_FadeOut()
{
    SET_CURRENT_STATE();

    this->alphaTimer = (this->alphaTimer + 16) & 0x1FF;

    if (this->timer >= 400) {
        this->timer = 248;
        this->state.Set(&HP_SpecialClear::State_ExitResults);
        this->stateDraw.Set(&HP_SpecialClear::Draw_Exit);
    }
    else {
        this->timer += 8;
    }
}
void HP_SpecialClear::State_ExitResults()
{
    SET_CURRENT_STATE();

    this->alphaTimer = (this->alphaTimer + 16) & 0x1FF;

    if (this->timer < 0) {
        SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();

        Stage::SetScene("Adventure Mode", "");
        sceneInfo->listPos = saveRAM->storedStageID;

        this->state.Set(&HP_SpecialClear::State_Failed);
        if (HP_Setup::sVars->gotEmerald) {
            if (SaveGame::GetEmeralds(SaveGame::EmeraldBoth)) { // hyper !!!
                // cool ok
            }
            else if (SaveGame::GetEmeralds(SaveGame::EmeraldAny) && sVars->hiddenPalaceEnabled) {
                Stage::SetScene("Extras", "Proto Palace Zone");
            }
        }

        if (!Stage::CheckValidScene())
            Stage::SetScene("Presentation", "Title Screen");

        Stage::LoadScene();
    }
    else {
        this->timer -= 8;
    }
}
void HP_SpecialClear::State_WinShowReward()
{
    SET_CURRENT_STATE();

    this->alphaTimer = (this->alphaTimer + 16) & 0x1FF;

    if (++this->timer >= 90) {
        this->timer         = 0;
        this->textBottomPos = TO_FIXED(0);
        this->state.Set(&HP_SpecialClear::State_WinEnterText);
    }
}
void HP_SpecialClear::State_WinEnterText()
{
    SET_CURRENT_STATE();

    this->alphaTimer = (this->alphaTimer + 16) & 0x1FF;

    SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();

    if (this->textTopPos > -TO_FIXED(384)) {
        this->textTopPos -= TO_FIXED(32);
        this->textBottomPos -= TO_FIXED(32);
    }

    if (this->textMidPos < TO_FIXED(384)) {
        this->textMidPos += TO_FIXED(32);
    }
    else {
        if (saveRAM->collectedEmeralds == 0x3FFF) { // hyper !!!
            this->resultsTextTop    = HP_SpecialClear::MessageCanChangeForm;
            this->resultsTextMid    = HP_SpecialClear::MessageCanChangeForm;
            this->resultsTextBottom = HP_SpecialClear::MessageCanChangeForm;
            this->messageAnimator.SetAnimation(sVars->aniFrames, 7 + (5 * HUD::CharacterIndexFromID(GET_CHARACTER_ID(1))), true, 0);
        }
        else if (sVars->hiddenPalaceEnabled) {
            this->resultsTextTop    = HP_SpecialClear::MessageToHiddenPalace;
            this->resultsTextMid    = HP_SpecialClear::MessageToHiddenPalace;
            this->resultsTextBottom = HP_SpecialClear::MessageToHiddenPalace;
            this->messageAnimator.SetAnimation(sVars->aniFrames, 6 + (5 * HUD::CharacterIndexFromID(GET_CHARACTER_ID(1))), true, 0);
        }
        else {
            this->resultsTextTop    = HP_SpecialClear::MessageCanChangeForm;
            this->resultsTextMid    = HP_SpecialClear::MessageCanChangeForm;
            this->resultsTextBottom = HP_SpecialClear::MessageCanChangeForm;
            this->messageAnimator.SetAnimation(sVars->aniFrames, 5 + (5 * HUD::CharacterIndexFromID(GET_CHARACTER_ID(1))), true, 0);
        }

        this->state.Set(&HP_SpecialClear::State_WinEvent);
    }
}
void HP_SpecialClear::State_WinEvent()
{
    SET_CURRENT_STATE();

    this->alphaTimer = (this->alphaTimer + 16) & 0x1FF;

    if (this->textTopPos < TO_FIXED(0)) {
        this->textTopPos += TO_FIXED(16);
        this->textBottomPos += TO_FIXED(16);
    }

    if (this->textMidPos > TO_FIXED(0)) {
        this->textMidPos -= TO_FIXED(16);
    }
    else {
        sVars->sfxEvent.Play();
        this->state.Set(&HP_SpecialClear::State_WinIdle);
    }
}
void HP_SpecialClear::State_WinIdle()
{
    SET_CURRENT_STATE();

    this->alphaTimer = (this->alphaTimer + 16) & 0x1FF;

    if (++this->timer >= 200) {
        this->timer     = 0;
        this->fillColor = 0xF8F8F8;
        sVars->sfxWarp.Play();
        this->state.Set(&HP_SpecialClear::State_FadeOut);
        this->stateDraw.Set(&HP_SpecialClear::Draw_FadeOut);
    }
}

// Draw States
void HP_SpecialClear::Draw_FadeIn()
{
    SET_CURRENT_STATE();

    Graphics::FillScreen(this->fillColor, this->timer, this->timer, this->timer);
}
void HP_SpecialClear::Draw_Results()
{
    SET_CURRENT_STATE();

    SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();

    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    color bgColor = paletteBank[0].GetEntry(152);
    Graphics::FillScreen(bgColor, 0xFF, 0xFF, 0xFF);

    this->alpha = ((190 + (Math::Sin512(this->alphaTimer) >> 3)) * this->emeraldAlpha) >> 8;

    int32 emeraldPosTableX[] = { TO_FIXED(0), TO_FIXED(24), TO_FIXED(24), TO_FIXED(0), -TO_FIXED(24), -TO_FIXED(24), TO_FIXED(0) };
    int32 emeraldPosTableY[] = { TO_FIXED(84), TO_FIXED(96), TO_FIXED(120), TO_FIXED(132), TO_FIXED(120), TO_FIXED(96), TO_FIXED(108) };

    this->inkEffect = INK_ALPHA;
    for (int32 e = 0; e < 7; ++e) {
        if (GET_BIT(saveRAM->collectedEmeralds, e)) {
            Vector2 drawPos;
            drawPos.x = TO_FIXED(screen->center.x) + emeraldPosTableX[e];
            drawPos.y = emeraldPosTableY[e];
            this->emeraldsAnimator.frameID = e;
            this->emeraldsAnimator.DrawSprite(&drawPos, false);
        }
    }
    this->inkEffect = INK_NONE;

    Vector2 drawPos;
    drawPos.x = TO_FIXED(screen->center.x) + this->textTopPos;
    drawPos.y = TO_FIXED(0);

    switch (this->resultsTextTop) {
        default: break;

        case HP_SpecialClear::MessageEmeraldGet:
            this->messageAnimator.frameID = 1; // player
            if (GET_CHARACTER_ID(1) == ID_TAILS && (globals->secrets & SECRET_REGIONSWAP))
                this->uiElementsAnimator.frameID = this->uiElementsAnimator.frameCount - 1; // miles
            this->messageAnimator.DrawSprite(&drawPos, true);

            this->messageAnimator.frameID = 2; // got a
            this->messageAnimator.DrawSprite(&drawPos, true);
            break;

        case HP_SpecialClear::MessageAllEmeralds:
            this->messageAnimator.frameID = 1; // player
            if (GET_CHARACTER_ID(1) == ID_TAILS && (globals->secrets & SECRET_REGIONSWAP))
                this->uiElementsAnimator.frameID = this->uiElementsAnimator.frameCount - 1; // miles
            this->messageAnimator.DrawSprite(&drawPos, true);

            this->messageAnimator.frameID = 2; // has all the
            this->messageAnimator.DrawSprite(&drawPos, true);
            break;

        case HP_SpecialClear::MessageCanChangeForm:
            this->messageAnimator.frameID = 1; // now
            this->messageAnimator.DrawSprite(&drawPos, true);

            this->messageAnimator.frameID = 2; // player
            if (GET_CHARACTER_ID(1) == ID_TAILS && (globals->secrets & SECRET_REGIONSWAP))
                this->uiElementsAnimator.frameID = this->uiElementsAnimator.frameCount - 2; // miles
            this->messageAnimator.DrawSprite(&drawPos, true);

            this->messageAnimator.frameID = 3; // can
            this->messageAnimator.DrawSprite(&drawPos, true);
            break;

        case HP_SpecialClear::MessageToHiddenPalace:
            this->messageAnimator.frameID = 1; // now
            this->messageAnimator.DrawSprite(&drawPos, true);

            this->messageAnimator.frameID = 2; // player
            if (GET_CHARACTER_ID(1) == ID_TAILS && (globals->secrets & SECRET_REGIONSWAP))
                this->uiElementsAnimator.frameID = this->uiElementsAnimator.frameCount - 1; // miles
            this->messageAnimator.DrawSprite(&drawPos, true);
            break;
    }

    drawPos.x = TO_FIXED(screen->center.x) + this->textMidPos;
    switch (this->resultsTextMid) {
        default: break;

        case HP_SpecialClear::MessageFailed:
            this->messageAnimator.frameID = 0; // special stage
            this->messageAnimator.DrawSprite(&drawPos, true);
            break;

        case HP_SpecialClear::MessageEmeraldGet:
            this->messageAnimator.frameID = 3; // chaos emerald
            this->messageAnimator.DrawSprite(&drawPos, true);
            break;

        case HP_SpecialClear::MessageAllEmeralds:
            this->messageAnimator.frameID = 3; // chaos emeralds
            this->messageAnimator.DrawSprite(&drawPos, true);
            break;

        case HP_SpecialClear::MessageCanChangeForm:
            this->messageAnimator.frameID = 4; // change into
            this->messageAnimator.DrawSprite(&drawPos, true);
            break;

        case HP_SpecialClear::MessageToHiddenPalace:
            this->messageAnimator.frameID = 3; // can
            this->messageAnimator.DrawSprite(&drawPos, true);

            this->messageAnimator.frameID = 4; // go
            this->messageAnimator.DrawSprite(&drawPos, true);

            this->messageAnimator.frameID = 5; // to
            this->messageAnimator.DrawSprite(&drawPos, true);
            break;
    }

    drawPos.x = TO_FIXED(screen->center.x) + this->textBottomPos;
    switch (this->resultsTextBottom) {
        default: break;

        case HP_SpecialClear::MessageCanChangeForm:
            this->messageAnimator.frameID = 5; // super
            this->messageAnimator.DrawSprite(&drawPos, true);

            this->messageAnimator.frameID = 6; // player
            if (GET_CHARACTER_ID(1) == ID_TAILS && (globals->secrets & SECRET_REGIONSWAP))
                this->uiElementsAnimator.frameID = this->uiElementsAnimator.frameCount - 1; // miles

            this->messageAnimator.DrawSprite(&drawPos, true);
            break;

        case HP_SpecialClear::MessageToHiddenPalace:
            this->messageAnimator.frameID = 6; // hidden palace
            this->messageAnimator.DrawSprite(&drawPos, true);
            break;
    }

    drawPos.x                        = TO_FIXED(screen->center.x) + this->scorePos;
    drawPos.y                        = TO_FIXED(145);
    this->uiElementsAnimator.frameID = 0; // score
    this->uiElementsAnimator.DrawSprite(&drawPos, true);

    drawPos.x += TO_FIXED(88);
    this->DrawNumbers(&drawPos, this->score, 0);

    if (GET_CHARACTER_ID(2)) {
        drawPos.x                        = TO_FIXED(screen->center.x) + this->ringPos[0];
        drawPos.y                        = TO_FIXED(161);
        this->uiElementsAnimator.frameID = 2; // sonic rings
        this->uiElementsAnimator.DrawSprite(&drawPos, true);

        drawPos.x += TO_FIXED(88);
        this->DrawNumbers(&drawPos, this->ringBonus[0], 0);

        drawPos.x                        = TO_FIXED(screen->center.x) + this->ringPos[0];
        drawPos.y                        = TO_FIXED(177);
        this->uiElementsAnimator.frameID = 3; // tails rings
        if (GET_CHARACTER_ID(2) == ID_TAILS && (globals->secrets & SECRET_REGIONSWAP))
            this->uiElementsAnimator.frameID = this->uiElementsAnimator.frameCount - 1;  // miles rings
        this->uiElementsAnimator.DrawSprite(&drawPos, true);

        drawPos.x += TO_FIXED(88);
        this->DrawNumbers(&drawPos, this->ringBonus[1], 0);

        if (HP_Setup::sVars->gotEmerald) {
            drawPos.x                        = TO_FIXED(screen->center.x) + this->gemPos;
            drawPos.y                        = TO_FIXED(193);
            this->uiElementsAnimator.frameID = 5; // gems bonus
            this->uiElementsAnimator.DrawSprite(&drawPos, true);

            drawPos.x += TO_FIXED(88);
            this->DrawNumbers(&drawPos, this->gemBonus, 0);
        }
    }
    else {
        drawPos.x                        = TO_FIXED(screen->center.x) + this->ringPos[0];
        drawPos.y                        = TO_FIXED(169);
        this->uiElementsAnimator.frameID = 1; // rings
        this->uiElementsAnimator.DrawSprite(&drawPos, true);

        drawPos.x += TO_FIXED(88);
        this->DrawNumbers(&drawPos, this->ringBonus[0], 0);

        if (HP_Setup::sVars->gotEmerald) {
            drawPos.x                        = TO_FIXED(screen->center.x) + this->ringPos[1];
            drawPos.y                        = TO_FIXED(193);
            this->uiElementsAnimator.frameID = 4; // gems bonus
            this->uiElementsAnimator.DrawSprite(&drawPos, true);

            drawPos.x += TO_FIXED(88);
            this->DrawNumbers(&drawPos, this->gemBonus, 0);
        }
    }
}
void HP_SpecialClear::Draw_FadeOut()
{
    SET_CURRENT_STATE();

    Draw_Results();

    Graphics::FillScreen(this->fillColor, this->timer, this->timer, this->timer);
}
void HP_SpecialClear::Draw_Exit()
{
    SET_CURRENT_STATE();

    Draw_Results();

    uint8 c         = CLAMP(this->timer, 0, 255);
    Graphics::FillScreen((c << 16) | (c << 8) | (c << 0), 0xFF, 0xFF, 0xFF);
}

#if RETRO_INCLUDE_EDITOR
void HP_SpecialClear::EditorDraw() {}

void HP_SpecialClear::EditorLoad() { sVars->aniFrames.Load("Special/Results.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void HP_SpecialClear::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(HP_SpecialClear);
    sVars->aniFrames.Init();
}
#endif

void HP_SpecialClear::Serialize() {}

} // namespace GameLogic