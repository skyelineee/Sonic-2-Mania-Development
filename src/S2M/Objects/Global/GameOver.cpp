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
#include "Menu/UILoadingIcon.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(GameOver);

void GameOver::Update() 
{ 
    this->state.Run(this); 
    this->trianglesLeftAnimator.Process();
    this->trianglesRightAnimator.Process();
    this->checkerboardBGAnimator.Process();
}
void GameOver::LateUpdate() {}
void GameOver::StaticUpdate() {}
void GameOver::Draw()
{
    Vector2 drawPos;
    Player *player = GameObject::Get<Player>(sceneInfo->currentScreenID + Player::sVars->playerCount);

    // bg checkerboard, doesnt move at all so doesnt need its own position variable
    this->inkEffect = INK_ALPHA;
    drawPos.x = 0;
    drawPos.y = 0;
    this->checkerboardBGAnimator.DrawSprite(&drawPos, true);

    // left triangles
    this->inkEffect = INK_NONE;
    drawPos.x = this->triangleLeftPos.x;
    drawPos.y = this->triangleLeftPos.y;
    this->trianglesLeftAnimator.DrawSprite(&drawPos, true);

    // right triangles
    drawPos.x = this->triangleRightPos.x;
    drawPos.y = this->triangleRightPos.y;
    this->trianglesRightAnimator.DrawSprite(&drawPos, true);

    if (sceneInfo->currentScreenID == this->playerID) {
        // Game/Time Over 
        drawPos.x = this->gameOverPos.x;
        drawPos.y = this->gameOverPos.y;
        this->animator.DrawSprite(&drawPos, true);
    }
}

void GameOver::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active  = ACTIVE_ALWAYS;
        this->visible = true;
        this->drawGroup = Zone::sVars->hudDrawGroup;
        // ink effect is set to none on creation, no transparency
        this->inkEffect = INK_NONE;
        this->alpha     = 0; // should increase for the sliding in
        this->triangleSpeed = TO_FIXED(10);
        this->centerSpeed = TO_FIXED(4);

        // grabs current screen
        ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

        // base positions before sliding in
        this->gameOverPos.x = TO_FIXED(screen->center.x);
        this->gameOverPos.y = -TO_FIXED(20);
        triangleLeftPos.x   = -TO_FIXED(46);
        triangleLeftPos.y   = TO_FIXED(0);
        triangleRightPos.x  = TO_FIXED(424);
        triangleRightPos.y  = TO_FIXED(0);
        // time overs are currently bugged
        // how it works is the player is killed when the time reaches 10 minutes, and it then brings up the time over
        // however the stage fading out from the players death will still happen, cutting off the time over
        // normal game overs work perfectly fine tho
        if (data)
            this->animator.SetAnimation(sVars->aniFrames, 7, true, 0);
        else
            this->animator.SetAnimation(sVars->aniFrames, 6, true, 0);

        this->trianglesLeftAnimator.SetAnimation(sVars->aniFrames, 12, true, 0);
        this->trianglesRightAnimator.SetAnimation(sVars->aniFrames, 13, true, 0);
        this->checkerboardBGAnimator.SetAnimation(sVars->aniFrames, 14, true, 0);

        this->state.Set(&GameOver::State_MoveIn);

        for (auto hud : GameObject::GetEntities<HUD>(FOR_ALL_ENTITIES))
        {
            HUD::MoveOut(hud);
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

void GameOver::SaveGameCallback(bool32 success)
{
    UILoadingIcon::FinishWait();
    Stage::LoadScene();
}

void GameOver::State_MoveIn()
{
    SET_CURRENT_STATE();

    // grabs current screen
    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    // decreases the speed if its above 0
    if (this->triangleSpeed > 0) {
        this->triangleSpeed -= TO_FIXED(1);
    }
    // makes sure it never gets below 0
    if (this->triangleSpeed < 0) {
        this->triangleSpeed = 0;
    }

    // fades in alpha
    if (this->alpha < 96) {
        this->alpha += 8;
    }

    // text
    if (this->gameOverPos.y < TO_FIXED(screen->center.y)) { // if the gameover pos y position is less than the center of the screen (above it) then add to it by whatever the center speed is
        this->gameOverPos.y += this->centerSpeed;
        if (this->gameOverPos.y > TO_FIXED(screen->center.y) - TO_FIXED(16)) { // if the gameover pos is greater than (below) 16 pixels above the screen center, then decrease the speed every frame
            // decreases the speed if its above 0
            if (this->centerSpeed > 0) {
                this->centerSpeed -= TO_FIXED(1);
            }
            // makes sure it never gets below 0
            if (this->centerSpeed < 0) {
                this->centerSpeed = 0;
            }
        }
    }

    // left triangles
    if (this->triangleLeftPos.x < TO_FIXED(0)) {
        this->triangleLeftPos.x += this->triangleSpeed;
    }

    // right triangles
    if (this->triangleRightPos.x > TO_FIXED(378)) {
        this->triangleRightPos.x -= this->triangleSpeed;
    }

    if (this->timer == 0) {
        Music::ClearMusicStack();
        Music::PlayOnFade(Music::TRACK_GAMEOVER, 0.025f);
    }

    if (++this->timer == 120) {
        this->timer = 0;
        this->state.Set(&GameOver::State_Wait);
    }
}

void GameOver::State_Wait()
{
    SET_CURRENT_STATE();

    ++this->timer;

    int32 id = 4 ? (this->playerID + 1) : Input::INPUT_NONE;

    if (controllerInfo[id].keyA.press || controllerInfo[id].keyB.press || controllerInfo[id].keyC.press || controllerInfo[id].keyX.press
        || controllerInfo[id].keyStart.press)
        this->timer = 700;

    if (this->timer == 700) {
        Music::ClearMusicStack();
        Music::FadeOut(0.05f);
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

    if (this->timer == 80) {
        Zone::StartFadeOut(10, 0x000000); // this also loads the scene, which lead to a bug where it was loaded twice in quick succession, just gotta make sure they both get called on the same frame
    }

    if (this->timer == 90) {
        this->timer = 0;

        if (this->animator.animationID != 6) {
            StarPost::sVars->storedMilliseconds = 0;
            StarPost::sVars->storedSeconds      = 0;
            StarPost::sVars->storedMinutes      = 0;
        }
        else {
            SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();
            if (globals->gameMode >= MODE_TIMEATTACK) {
                Stage::SetScene("Presentation", "Menu");
                Stage::LoadScene();
            }
            else if (globals->continues > 0) {
                saveRAM->storedStageID = sceneInfo->listPos;
                saveRAM->lives         = 3;
                saveRAM->score         = 0;
                saveRAM->score1UP      = 0;

                Stage::SetScene("Presentation", "Continue");
                Stage::LoadScene();
            }
            else {
                saveRAM->lives    = 3;
                saveRAM->score    = 0;
                saveRAM->score1UP = 0;

                UILoadingIcon::StartWait();
                SaveGame::SaveFile(&GameOver::SaveGameCallback);
                Stage::SetScene("Presentation", "Menu");
            }
        }
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
