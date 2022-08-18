// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: GameOver Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "GameOver.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "HUD.hpp"
#include "Music.hpp"
#include "StarPost.hpp"
#include "SaveGame.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(GameOver);

void GameOver::Update() { this->state.Run(this); }
void GameOver::LateUpdate() {}
void GameOver::StaticUpdate() {}
void GameOver::Draw()
{
    Player *player = GameObject::Get<Player>(sceneInfo->currentScreenID + Player::sVars->playerCount);

    if (globals->gameMode != MODE_COMPETITION || sceneInfo->currentScreenID == this->playerID) {
        if (globals->useManiaBehavior) {
            if (sceneInfo->currentScreenID == this->playerID || player->classID != sVars->classID)
                Graphics::DrawFace(this->verts, 4, 0x00, 0x00, 0x00, 0xFF, INK_NONE);

            for (this->animator.frameID = 0; this->animator.frameID < 8; ++this->animator.frameID) {
                this->rotation = this->letterRotations[this->animator.frameID];
                this->animator.DrawSprite(&this->letterPositions[this->animator.frameID], true);
            }
        }
        else {
            // Game/Time
            this->animator.frameID = 0;
            this->animator.DrawSprite(&this->letterPositions[0], true);

            // Over
            this->animator.frameID = 1;
            this->animator.DrawSprite(&this->letterPositions[1], true);
        }
    }
}

void GameOver::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active  = ACTIVE_ALWAYS;
        this->visible = true;
        if (data)
            this->animator.SetAnimation(sVars->aniFrames, 7, true, 1);
        else
            this->animator.SetAnimation(sVars->aniFrames, 6, true, 0);

        this->finalOffsets[0].x = -0x480000;
        this->finalOffsets[1].x = -0x370000;
        this->finalOffsets[2].x = -0x260000;
        this->finalOffsets[3].x = -0x150000;
        this->finalOffsets[4].x = 0x0C0000;
        this->finalOffsets[5].x = 0x1D0000;
        this->finalOffsets[6].x = 0x2E0000;
        this->finalOffsets[7].x = 0x3F0000;

        if (globals->useManiaBehavior) {
            this->drawFX = FX_ROTATE | FX_SCALE;

            int32 posY = -0x200000;
            for (int32 i = 0; i < 8; ++i) {
                this->letterPosMove[i].x = -(this->finalOffsets[i].x >> 4);
                this->letterPosMove[i].y = 0x2000;

                this->finalOffsets[i].y = (screenInfo->center.y - 4) << 16;

                this->letterPositions[i].x = 8 * ((screenInfo->center.x << 13) + this->finalOffsets[i].x);
                this->letterPositions[i].y = posY;

                posY -= 0x100000;
            }

            this->barPos.x  = 0x1000000;
            this->barPos.y  = screenInfo->center.y << 16;
            this->scale.x   = 0x800;
            this->state.Set(&GameOver::State_MoveIn_Mania);
            this->drawGroup = Zone::sVars->hudDrawGroup + 1;
        }
        else {
            this->letterPositions[0].x = 0;
            this->letterPositions[0].y = screenInfo->center.y << 16;
            this->letterPositions[1].x = screenInfo->size.x << 16;
            this->letterPositions[1].y = screenInfo->center.y << 16;
            // this->state.Set(&GameOver::State_MoveIn);
            this->drawGroup = Zone::sVars->hudDrawGroup;
        }

        for (auto hud : GameObject::GetEntities<HUD>(FOR_ALL_ENTITIES))
        {
            hud->state.Set(&HUD::State_MoveOut);
            hud->active = ACTIVE_ALWAYS;
        }
    }
}

void GameOver::StageLoad()
{
    sVars->aniFrames.Load("Global/HUD.bin", SCOPE_STAGE);

    sVars->activeScreens = 0;
}

void GameOver::State_MoveIn_Mania()
{
    SET_CURRENT_STATE();

    if (this->barPos.x > 0)
        this->barPos.x -= 0x40000;

    this->verts[0].x = this->barPos.x + ((screenInfo->center.x - 104) << 16);
    this->verts[1].x = this->barPos.x + ((screenInfo->center.x + 88) << 16);
    this->verts[2].x = this->barPos.x + ((screenInfo->center.x + 104) << 16);
    this->verts[3].x = this->barPos.x + ((screenInfo->center.x - 88) << 16);
    this->verts[0].y = this->barPos.y - 0x80000;
    this->verts[1].y = this->barPos.y - 0x80000;
    this->verts[2].y = this->barPos.y + 0x80000;
    this->verts[3].y = this->barPos.y + 0x80000;

    for (int32 i = 0; i < 8; ++i) {
        this->letterPositions[i].x = (screenInfo->center.x << 16) + this->scale.x * (this->finalOffsets[i].x >> 9);
        if (this->letterBounceCount[i] < 3) {
            this->letterPosMove[i].y += 0x4000;
            this->letterPositions[i].y += this->letterPosMove[i].y;

            if (this->letterPosMove[i].y > 0 && this->letterPositions[i].y > this->finalOffsets[i].y) {
                this->letterPositions[i].y = this->finalOffsets[i].y;
                this->letterPosMove[i].y   = -(this->letterPosMove[i].y / 3);

                ++this->letterBounceCount[i];
            }
        }
    }

    if (this->scale.x <= 0x200)
        this->scale.x = 0x200;
    else
        this->scale.x = this->scale.x - this->scale.x / 40;
    this->scale.y = this->scale.x;

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
            this->state.Set(&GameOver::State_WaitComp_Mania);
        else
            this->state.Set(&GameOver::State_Wait_Mania);
    }
}

void GameOver::State_WaitComp_Mania() { SET_CURRENT_STATE(); }

void GameOver::State_Wait_Mania()
{
    SET_CURRENT_STATE();

    ++this->timer;

    int32 id = globals->gameMode == MODE_COMPETITION ? (this->playerID + 1) : Input::INPUT_NONE;

    if (controllerInfo[id].keyA.press || controllerInfo[id].keyB.press || controllerInfo[id].keyC.press || controllerInfo[id].keyX.press
        || controllerInfo[id].keyStart.press)
        this->timer = 420;

    if (this->timer == 420) {
        Music::ClearMusicStack();
        Music::FadeOut(0.05f);

        for (auto gameOver : GameObject::GetEntities<GameOver>(FOR_ALL_ENTITIES)) {
            int32 angle = 0x88;
            for (int32 i = 0; i < 8; ++i) {
                gameOver->letterPosMove[i].x   = Math::Cos256(angle) << 11;
                gameOver->letterPosMove[i].y   = Math::Sin256(angle) << 11;
                gameOver->letterRotateSpeed[i] = Math::Rand(-8, 8);
                angle += 0x10;
            }
            gameOver->timer = 0;
            gameOver->state.Set(&GameOver::State_MoveOut_Mania);
        }
    }
}

void GameOver::State_MoveOut_Mania()
{
    SET_CURRENT_STATE();

    if (this->timer < 120) {
        for (int32 i = 0; i < 8; ++i) {
            this->letterPositions[i].x += this->letterPosMove[i].x;
            this->letterPositions[i].y += this->letterPosMove[i].y;
            this->letterRotations[i] += this->letterRotateSpeed[i];
        }
        this->verts[0].x -= 0x100000;
        this->verts[0].y -= 0x80000;
        this->verts[1].x += 0x100000;
        this->verts[1].y -= 0x80000;
        this->verts[2].x += 0x100000;
        this->verts[2].y += 0x80000;
        this->verts[3].x -= 0x100000;
        this->verts[3].y += 0x80000;
        this->scale.x += 0x20;
        this->scale.y += 0x20;
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

                if (globals->gameMode == MODE_ENCORE) {
                    globals->playerID &= 0xFF;
                    globals->characterFlags = 1 << HUD::CharacterIndexFromID(GET_CHARACTER_ID(1));
                    saveRAM->characterFlags = globals->characterFlags;
                    saveRAM->stock          = globals->stock;
                }

                Stage::SetScene("Presentation & Menus", "Continue");
            }
            else {
                saveRAM->lives    = 3;
                saveRAM->score    = 0;
                saveRAM->score1UP = 0;

                if (globals->gameMode == MODE_ENCORE) {
                    globals->playerID &= 0xFF;
                    int32 id                = -1;
                    saveRAM->characterFlags = -1;
                    for (int32 i = GET_CHARACTER_ID(1); i > 0; ++id, i >>= 1)
                        ;
                    globals->characterFlags = 1 << id;
                    saveRAM->characterFlags = globals->characterFlags;
                    saveRAM->stock          = globals->stock;
                }

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
