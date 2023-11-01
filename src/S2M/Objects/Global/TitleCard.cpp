// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TitleCard Object
// Object Author: Ducky + AChickMcNuggie
// ---------------------------------------------------------------------

#include "TitleCard.hpp"
#include "Player.hpp"
#include "Camera.hpp"
#include "Zone.hpp"
#include "ActClear.hpp"
#include "ImageTrail.hpp"
#include "InvincibleStars.hpp"
#include "Announcer.hpp"
#include "HUD.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TitleCard);

void TitleCard::Update()
{

    this->state.Run(this);

    this->yellowPieceAnimator.Process();
    this->redPieceAnimator.Process();
    this->bluePieceAnimator.Process();
}
void TitleCard::LateUpdate() {}
void TitleCard::StaticUpdate() {}
void TitleCard::Draw() { this->stateDraw.Run(this); }

void TitleCard::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active      = ACTIVE_ALWAYS;
        this->visible     = true;
        this->drawGroup   = Zone::sVars->hudDrawGroup + 1;
        this->enableIntro = globals->enableIntro;
        this->timer       = 0;
    }

    if (!globals->suppressTitlecard || globals->enableIntro || globals->gameMode == MODE_TIMEATTACK)
        sceneInfo->timeEnabled = false;

    if (globals->suppressTitlecard) {
        sVars->suppressCB.Run(this);
        sVars->suppressCB.Set(nullptr);
        this->state.Set(&TitleCard::State_Supressed);
    }
    else {
        if (!globals->atlEnabled)
            globals->persistentTimer = 0;

        this->state.Set(&TitleCard::State_SetupBGElements);
        this->stateDraw.Set(&TitleCard::Draw_SlideIn);
    }

    SetupTitleWords();
    ChangeTitleColors();

    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    this->bluePiecePos.x   = TO_FIXED(40);
    this->bluePiecePos.y   = TO_FIXED(-20);
    this->decorationPos.x  = TO_FIXED(185);
    this->decorationPos.y  = TO_FIXED(120);
    this->yellowPiecePos.x = screen->size.x;
    this->yellowPiecePos.y = TO_FIXED(178);
    this->redPiecePos.x    = TO_FIXED(0);
    this->redPiecePos.y    = TO_FIXED(0);
    this->zoneNamePos.x    = TO_FIXED(400);
    this->zoneNamePos.y    = TO_FIXED(75);
    this->zonePos.x        = TO_FIXED(310);
    this->zonePos.y        = TO_FIXED(100);
    this->actNumPos.x      = TO_FIXED(380);
    this->actNumPos.y      = TO_FIXED(101);

    this->bluePieceAnimator.SetAnimation(&sVars->aniFrames, 5, false, 0);
    this->decorationAnimator.SetAnimation(&sVars->aniFrames, 0, false, 0);
    this->yellowPieceAnimator.SetAnimation(&sVars->aniFrames, 6, false, 0);
    if (!globals->atlEnabled && !globals->suppressTitlecard) {
        this->redPieceAnimator.SetAnimation(&sVars->aniFrames, 4, false, 0);
    }
    else {
        this->redPieceAnimator.SetAnimation(&sVars->aniFrames, 7, false, 0);
    }
    this->zoneAnimator.SetAnimation(&sVars->aniFrames, 2, false, 0);
    this->actNumbersAnimator.SetAnimation(&sVars->aniFrames, 3, false, 0);
    this->zoneNameAnimator.SetAnimation(&sVars->aniFrames, 1, false, 0);

    if (this->actID > Zone::ActNone)
        this->actID = Zone::ActNone;

    this->actNumbersAnimator.frameID = this->actID;
    this->decorationAnimator.frameID = this->decorationFrame;
}

void TitleCard::StageLoad()
{
    sVars->aniFrames.Load("Global/TitleCard.bin", SCOPE_STAGE);

    for (auto titleCard : GameObject::GetEntities<TitleCard>(FOR_ALL_ENTITIES)) {
        Zone::sVars->actID = titleCard->actID;
    }
}

void TitleCard::HandleCamera()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->camera)
            player->camera->offset.y = 0;
    }
}

void TitleCard::ChangeTitleColors()
{
    if (!globals->suppressTitlecard) {
        switch (GET_CHARACTER_ID(1)) {
            default: break;
            case ID_SONIC:
                paletteBank[0].SetEntry(3, 0x0F16AD);
                paletteBank[0].SetEntry(4, 0x1D2EE2);
                break;

            case ID_TAILS:
                paletteBank[0].SetEntry(3, 0xE24F05);
                paletteBank[0].SetEntry(4, 0xFD7300);
                break;

            case ID_KNUCKLES:
                paletteBank[0].SetEntry(3, 0x057F19);
                paletteBank[0].SetEntry(4, 0x4CB00A);
                break;
        }
    }
}

void TitleCard::SetupTitleWords()
{
    if (!this->zoneName.chars)
        this->zoneName = "UNTITLED";

    this->zoneName.SetSpriteString(sVars->aniFrames, 1);
}

void TitleCard::PiecePositions()
{
    Vector2 piecePos;

    piecePos.x = this->bluePiecePos.x;
    piecePos.y = this->bluePiecePos.y - TO_FIXED(260);
    this->bluePieceAnimator.DrawSprite(&piecePos, true);

    piecePos.x = this->decorationPos.x + TO_FIXED(400);
    piecePos.y = this->decorationPos.y;
    this->decorationAnimator.DrawSprite(&piecePos, true);

    piecePos.x = this->yellowPiecePos.x + TO_FIXED(400);
    piecePos.y = this->yellowPiecePos.y;
    this->yellowPieceAnimator.DrawSprite(&piecePos, true);

    piecePos.x = this->redPiecePos.x - TO_FIXED(300);
    piecePos.y = this->redPiecePos.y;
    this->redPieceAnimator.DrawSprite(&piecePos, true);

    piecePos.x = this->zonePos.x - TO_FIXED(400);
    piecePos.y = this->zonePos.y;
    this->zoneAnimator.DrawSprite(&piecePos, true);

    piecePos.x = this->actNumPos.x - TO_FIXED(400);
    piecePos.y = this->actNumPos.y;
    this->actNumbersAnimator.DrawSprite(&piecePos, true);

    piecePos.x = this->zoneNamePos.x + TO_FIXED(300);
    piecePos.y = this->zoneNamePos.y;
    this->zoneNameAnimator.DrawString(&piecePos, &this->zoneName, 0, 0, 2, -1, nullptr, true);
}

// States
void TitleCard::State_SetupBGElements()
{
    SET_CURRENT_STATE();

    MovePositions_SlideIn();

    if (ActClear::sVars && ActClear::sVars->actClearActive)
        ActClear::sVars->actClearActive = false;

    Zone::ApplyWorldBounds();

    if (!globals->atlEnabled && !globals->suppressTitlecard)
        Stage::SetEngineState(ENGINESTATE_PAUSED);

    this->timer += 24;
    if (this->timer >= 512) {

        this->state.Set(&TitleCard::State_OpeningBG);
    }
}
void TitleCard::State_OpeningBG()
{
    SET_CURRENT_STATE();

    MovePositions_SlideIn();

    Zone::ApplyWorldBounds();

    if (this->timer >= 1024) {
        this->state.Set(&TitleCard::State_EnterTitle);
        this->stateDraw.Set(&TitleCard::Draw_ShowTitleCard);
    }
    else {
        this->timer += 32;
    }
}

void TitleCard::State_EnterTitle()
{
    SET_CURRENT_STATE();

    Zone::ApplyWorldBounds();

    MovePositions_ShowTitle();

    this->state.Set(&TitleCard::State_ShowingTitle);
}
void TitleCard::State_ShowingTitle()
{
    SET_CURRENT_STATE();

    MovePositions_ShowTitle();

    if (this->actionTimer >= 60) {
        this->actionTimer = 0;
        this->state.Set(&TitleCard::State_SlideAway);
        this->stateDraw.Set(&TitleCard::Draw_SlideAway);
        Stage::SetEngineState(ENGINESTATE_REGULAR);
        sVars->finishedCB.Run(this);
    }
    else {
        this->actionTimer++;
        if (this->actionTimer == 16) {
            if (Zone::sVars->setATLBounds) {
                Camera *camera = GameObject::Get<Camera>(SLOT_CAMERA1);
                Player *player = GameObject::Get<Player>(SLOT_PLAYER1);
                player->camera = camera;
                camera->target = (Entity *)player;
                camera->state.Set(&Camera::State_FollowXY);
                Camera::sVars->centerBounds.x = TO_FIXED(2);
                Camera::sVars->centerBounds.y = TO_FIXED(2);
            }
            Zone::sVars->setATLBounds = false;
        }
    }

    Zone::ApplyWorldBounds();
    HandleCamera();
}
void TitleCard::State_SlideAway()
{
    SET_CURRENT_STATE();

    MovePositions_SlideAway();

    Zone::ApplyWorldBounds();

    if (this->actionTimer == 6 && globals->gameMode < MODE_TIMEATTACK) {
        sceneInfo->timeEnabled = true;
    }

    if (actionTimer == 10) {
        for (auto hud : GameObject::GetEntities<HUD>(FOR_ACTIVE_ENTITIES)) {
            HUD::MoveIn(hud);
        }
    }

    if (this->actionTimer > 30) {
        globals->atlEnabled  = false;
        globals->enableIntro = false;
        if (globals->gameMode == MODE_TIMEATTACK) {
            Announcer::StartCountdown();
        }
        else {
            globals->suppressTitlecard = false;
            globals->suppressAutoMusic = false;
        }
        switch (GET_CHARACTER_ID(1)) {
            default: break;
            case ID_TAILS:
                paletteBank[0].SetEntry(3, 0x0F16AD);
                paletteBank[0].SetEntry(4, 0x1D2EE2);
                break;
            case ID_KNUCKLES:
                paletteBank[0].SetEntry(3, 0x0F16AD);
                paletteBank[0].SetEntry(4, 0x1D2EE2);
                break;
        }

        this->Destroy();
    }
    else {
        this->actionTimer++;
    }
}

void TitleCard::State_Supressed()
{
    SET_CURRENT_STATE();

    HandleCamera();
    Stage::SetEngineState(ENGINESTATE_REGULAR);

    globals->atlEnabled = false;
    if (globals->gameMode == MODE_TIMEATTACK || globals->enableIntro)
        sceneInfo->timeEnabled = false;

    this->active = ACTIVE_NEVER;
    if ((globals->suppressTitlecard && !this->enableIntro) || globals->gameMode == MODE_TIMEATTACK) {
        this->Destroy();
        globals->suppressTitlecard = false;
    }

    sVars->finishedCB.Run(this);
}

void TitleCard::MovePositions_SlideIn()
{
    if (!globals->atlEnabled && !globals->suppressTitlecard) {

        if (this->bluePiecePos.y < TO_FIXED(240))
            this->bluePiecePos.y += TO_FIXED(16);

        if (this->decorationPos.x > TO_FIXED(-195))
            this->decorationPos.x -= TO_FIXED(18);
    }

    if (this->yellowPiecePos.x > TO_FIXED(-424))
        this->yellowPiecePos.x -= TO_FIXED(16);

    if (this->redPiecePos.x < TO_FIXED(300))
        this->redPiecePos.x += TO_FIXED(10);

    if (this->zonePos.x < TO_FIXED(680))
        this->zonePos.x += TO_FIXED(16);

    if (this->actNumPos.x < TO_FIXED(750))
        this->actNumPos.x += TO_FIXED(16);

    if (this->zoneNamePos.x > TO_FIXED(100))
        this->zoneNamePos.x -= TO_FIXED(16);

    if (this->actNumPos.x < TO_FIXED(870)) {
        if (this->moveTimer >= 4) {
            this->zonePos.x += TO_FIXED(1);
            this->actNumPos.x += TO_FIXED(1);
            this->zoneNamePos.x -= TO_FIXED(1);
            this->moveTimer = 0;
        }
        else {
            this->moveTimer += 2;
        }
    }
}

// Draw States
void TitleCard::Draw_SlideIn()
{
    SET_CURRENT_STATE();

    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    if (!globals->atlEnabled && !globals->suppressTitlecard) {
        Graphics::DrawRect(0, 0, screen->size.x, screen->size.y, 0x000000, 0xFF, INK_NONE, true);
    }

    PiecePositions();
}

void TitleCard::MovePositions_ShowTitle()
{

    if (this->actNumPos.x < TO_FIXED(870)) {
        if (this->moveTimer >= 4) {
            this->zonePos.x += TO_FIXED(1);
            this->actNumPos.x += TO_FIXED(1);
            this->zoneNamePos.x -= TO_FIXED(1);
            this->moveTimer = 0;
        }
        else {
            this->moveTimer += 2;
        }
    }
}

void TitleCard::Draw_ShowTitleCard()
{
    SET_CURRENT_STATE();

    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    Graphics::SetClipBounds(sceneInfo->currentScreenID, 0, 0, screen->size.x, screen->size.y);

    PiecePositions();
    MovePositions_ShowTitle();
}

void TitleCard::MovePositions_SlideAway()
{
    if (!globals->atlEnabled && !globals->suppressTitlecard) {
        if (this->bluePiecePos.y <= TO_FIXED(260))
            this->bluePiecePos.y -= TO_FIXED(16);

        if (this->decorationPos.x < TO_FIXED(585))
            this->decorationPos.x -= TO_FIXED(18);
    }

    if (this->yellowPiecePos.x <= TO_FIXED(100))
        this->yellowPiecePos.x += TO_FIXED(16);

    if (this->redPiecePos.x >= TO_FIXED(0))
        this->redPiecePos.x -= TO_FIXED(10);

    if (this->zonePos.x <= TO_FIXED(900))
        this->zonePos.x += TO_FIXED(16);

    if (this->actNumPos.x <= TO_FIXED(970))
        this->actNumPos.x += TO_FIXED(16);

    if (this->zoneNamePos.x <= TO_FIXED(100))
        this->zoneNamePos.x -= TO_FIXED(16);
}

void TitleCard::Draw_SlideAway()
{
    SET_CURRENT_STATE();

    PiecePositions();

    MovePositions_SlideAway();
}

#if RETRO_INCLUDE_EDITOR
void TitleCard::EditorDraw()
{
    this->decorationAnimator.SetAnimation(sVars->aniFrames, 0, true, 3);
    this->decorationAnimator.DrawSprite(NULL, false);
}

void TitleCard::EditorLoad()
{
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, actID);
    RSDK_ENUM_VAR("Act 1");
    RSDK_ENUM_VAR("Act 2");
    RSDK_ENUM_VAR("No Act");
}
#endif

#if RETRO_REV0U
void TitleCard::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(TitleCard);

    sVars->aniFrames.Init();
}
#endif

void TitleCard::Serialize()
{
    RSDK_EDITABLE_VAR(TitleCard, VAR_STRING, zoneName);
    RSDK_EDITABLE_VAR(TitleCard, VAR_UINT8, actID);
    RSDK_EDITABLE_VAR(TitleCard, VAR_UINT8, decorationFrame);
}

} // namespace GameLogic