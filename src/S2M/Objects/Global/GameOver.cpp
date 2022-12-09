// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: GameOver Object
// Object Author: Ducky + AChickMcNuggie
// ---------------------------------------------------------------------

#include "GameOver.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "HUD.hpp"
#include "Music.hpp"
#include "StarPost.hpp"
#include "SaveGame.hpp"
#include "Helpers/FXFade.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(GameOver);

void GameOver::Update() { this->state.Run(this); }
void GameOver::LateUpdate() {}
void GameOver::StaticUpdate() {}
void GameOver::Draw()
{
    Vector2 drawPos;
    Player *player = GameObject::Get<Player>(sceneInfo->currentScreenID + Player::sVars->playerCount);

    if (globals->gameMode != MODE_COMPETITION || sceneInfo->currentScreenID == this->playerID) {
        // Game/Time Over 
        drawPos.x = this->gameOverPos.x;
        drawPos.y = this->gameOverPos.y - 120;
        this->animator.DrawSprite(&drawPos, true);
    }
}

void GameOver::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active  = ACTIVE_ALWAYS;
        this->visible = true;
        this->drawGroup = Zone::sVars->hudDrawGroup;
        this->gameOverPos.x = TO_FIXED(212);
        this->gameOverPos.y = 120;
        if (data)
            this->animator.SetAnimation(sVars->aniFrames, 7, true, 0);
        else
            this->animator.SetAnimation(sVars->aniFrames, 6, true, 0);

        this->state.Set(&GameOver::State_MoveIn);

        for (auto hud : GameObject::GetEntities<HUD>(FOR_ALL_ENTITIES))
        {
            hud->state.Set(&HUD::State_MoveOut);
            hud->active = ACTIVE_ALWAYS;
        }
    }
}

void GameOver::StageLoad()
{
    switch GET_CHARACTER_ID(1) {
        default: break;
        case ID_SONIC: sVars->aniFrames.Load("Global/HUDSonic.bin", SCOPE_STAGE); break;
        case ID_TAILS: sVars->aniFrames.Load("Global/HUDTails.bin", SCOPE_STAGE); break;
        case ID_KNUCKLES: sVars->aniFrames.Load("Global/HUDKnux.bin", SCOPE_STAGE); break;
    }

    sVars->activeScreens = 0;
}

void GameOver::State_MoveIn()
{
    SET_CURRENT_STATE();

    if (this->gameOverPos.y < TO_FIXED(120)) {
        this->gameOverPos.y += TO_FIXED(3);
    }

    if (this->timer == 0) {
        if (globals->gameMode != MODE_COMPETITION) {
            Music::ClearMusicStack();
            Music::PlayOnFade(Music::TRACK_GAMEOVER, 0.025f);
        }
        else {
            // removed
        }
    }

    if (++this->timer == 120) {
        this->timer = 0;
        if (globals->gameMode == MODE_COMPETITION || Zone::sVars->gotTimeOver)
            this->state.Set(&GameOver::State_WaitComp);
        else
            this->state.Set(&GameOver::State_Wait);
    }
}

void GameOver::State_WaitComp() { SET_CURRENT_STATE(); }

void GameOver::State_Wait()
{
    SET_CURRENT_STATE();

    ++this->timer;

    int32 id = globals->gameMode == MODE_COMPETITION ? (this->playerID + 1) : Input::INPUT_NONE;

    if (controllerInfo[id].keyA.press || controllerInfo[id].keyB.press || controllerInfo[id].keyC.press || controllerInfo[id].keyX.press
        || controllerInfo[id].keyStart.press)
        this->timer = 600;

    if (this->timer == 700) {
        Music::ClearMusicStack();
        Music::FadeOut(0.05f);
        Zone::StartFadeOut(10, 0x000000);

        for (auto gameOver : GameObject::GetEntities<GameOver>(FOR_ALL_ENTITIES)) {
            gameOver->timer = 0;
            gameOver->state.Set(&GameOver::State_MoveOut);
        }
    }
}

void GameOver::State_MoveOut()
{
    SET_CURRENT_STATE();

    if (this->timer < 120) {
        ++this->timer;
    }

    if (this->timer == 90) {
        this->timer = 0;

        if (globals->gameMode == MODE_COMPETITION) {
            // TODO: ... maybe
        }
        else if (this->animator.animationID != 6) {
            StarPost::sVars->storedMilliseconds = 0;
            StarPost::sVars->storedSeconds      = 0;
            StarPost::sVars->storedMinutes      = 0;
        }
        else {
            SaveGame *saveRAM = SaveGame::GetSaveRAM();
            if (globals->gameMode >= MODE_TIMEATTACK) {
                Stage::SetScene("Presentation & Menus", "Menu");
            }
            else if (globals->continues > 0) {
                saveRAM->storedStageID = sceneInfo->listPos;
                saveRAM->lives         = 3;
                saveRAM->score         = 0;
                saveRAM->score1UP      = 0;

                Stage::SetScene("Presentation & Menus", "Continue");
            }
            else {
                saveRAM->lives    = 3;
                saveRAM->score    = 0;
                saveRAM->score1UP = 0;

                SaveGame::SaveFile(nullptr);
                Stage::SetScene("Presentation & Menus", "Menu");
            }
        }

        Stage::LoadScene();
    }
}

#if RETRO_INCLUDE_EDITOR
void GameOver::EditorDraw() { this->animator.DrawSprite(&this->position, false); }

void GameOver::EditorLoad() {}
#endif

#if RETRO_REV0U
void GameOver::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(GameOver); }
#endif

void GameOver::Serialize() {}

} // namespace GameLogic
