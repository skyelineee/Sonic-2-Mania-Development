// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TitleCard Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "TitleCard.hpp"
#include "Player.hpp"
#include "Camera.hpp"
#include "Zone.hpp"
#include "ActClear.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TitleCard);

void TitleCard::Update() { this->state.Run(this); }
void TitleCard::LateUpdate() {}
void TitleCard::StaticUpdate() {}
void TitleCard::Draw() { this->stateDraw.Run(this); }

void TitleCard::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active      = ACTIVE_ALWAYS;
        this->visible     = true;
        this->drawGroup   = Zone::sVars->hudDrawGroup;
        this->enableIntro = globals->enableIntro;

        SetupColors();

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

        this->stripPos[0] = (screenInfo->center.x - 152) << 16;
        this->stripPos[1] = (screenInfo->center.x - 152) << 16;
        this->stripPos[2] = (screenInfo->center.x - 160) << 16;
        this->stripPos[3] = (screenInfo->center.x + 20) << 16;
        SetupTitleWords();
        SetupVertices();

        this->decorationPos.y = -(52 << 16);
        this->decorationPos.x = (screenInfo->size.x - 160) << 16;

        this->decorationAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->nameLetterAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->zoneLetterAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
        this->actNumbersAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);

        if (this->actID > Zone::ActNone)
            this->actID = Zone::ActNone;

        this->actNumbersAnimator.frameID = this->actID;
        this->actNumPos.y                = (168) << 16;
        this->actNumPos.x                = (screenInfo->center.x + 106) << 16;
        this->actNumScale                = -0x400;
        if (this->word2XPos - this->word2Width < (16 << 16)) {
            int32 dist = (this->word2XPos - this->word2Width) - (16 << 16);
            this->word1XPos -= dist;
            this->zoneXPos -= dist;
            this->actNumPos.x -= dist;
            this->word2XPos = this->word2XPos - dist;
        }
    }
}

void TitleCard::StageLoad()
{
    sVars->aniFrames.Load("Global/TitleCard.bin", SCOPE_STAGE);

    for (auto titleCard : GameObject::GetEntities<TitleCard>(FOR_ALL_ENTITIES)) {
        Zone::sVars->actID = titleCard->actID;
    }
}

void TitleCard::SetupColors()
{
    this->colors[0] = 0xF08C18; // orange
    this->colors[1] = 0x60C0A0; // green
    this->colors[2] = 0xF05030; // red
    this->colors[3] = 0x4060B0; // blue
    this->colors[4] = 0xF0C800; // yellow
}
void TitleCard::SetupVertices()
{
    this->vertMovePos[0].x = TO_FIXED(240);
    this->vertMovePos[0].y = TO_FIXED(496);
    this->vertMovePos[1].x = TO_FIXED(752);
    this->vertMovePos[1].y = TO_FIXED(1008);

    this->vertTargetPos[0].x = TO_FIXED(0);
    this->vertTargetPos[0].y = TO_FIXED(138);
    this->vertTargetPos[1].x = TO_FIXED(74);
    this->vertTargetPos[1].y = TO_FIXED(112);

    if (this->titleCardWord2 > 0) {
        this->word2DecorVerts[0].x = -this->word1Width;
        this->word2DecorVerts[0].y = TO_FIXED(82);
        this->word2DecorVerts[1].x = TO_FIXED(0);
        this->word2DecorVerts[1].y = TO_FIXED(82);
        this->word2DecorVerts[2].x = TO_FIXED(0);
        this->word2DecorVerts[2].y = TO_FIXED(98);
        this->word2DecorVerts[3].x = -this->word1Width;
        this->word2DecorVerts[3].y = TO_FIXED(98);
    }

    this->word1DecorVerts[0].x = -this->word2Width;
    this->word1DecorVerts[0].y = TO_FIXED(186);
    this->word1DecorVerts[1].x = TO_FIXED(0);
    this->word1DecorVerts[1].y = TO_FIXED(186);
    this->word1DecorVerts[2].x = TO_FIXED(0);
    this->word1DecorVerts[2].y = TO_FIXED(202);
    this->word1DecorVerts[3].x = -this->word2Width;
    this->word1DecorVerts[3].y = TO_FIXED(202);

    this->zoneDecorVerts[0].x = TO_FIXED(screenInfo->size.x);
    this->zoneDecorVerts[0].y = TO_FIXED(154);
    this->zoneDecorVerts[1].x = TO_FIXED(120) + this->zoneDecorVerts[0].x;
    this->zoneDecorVerts[1].y = TO_FIXED(154);
    this->zoneDecorVerts[2].x = TO_FIXED(120) + this->zoneDecorVerts[0].x;
    this->zoneDecorVerts[2].y = TO_FIXED(162);
    this->zoneDecorVerts[3].x = this->zoneDecorVerts[0].x;
    this->zoneDecorVerts[3].y = TO_FIXED(162);

    this->stripVertsBlue[0].x = this->stripPos[0];
    this->stripVertsBlue[0].y = TO_FIXED(240);
    this->stripVertsBlue[1].x = TO_FIXED(64) + this->stripVertsBlue[0].x;
    this->stripVertsBlue[1].y = TO_FIXED(240);
    this->stripVertsBlue[2].x = TO_FIXED(304) + this->stripVertsBlue[0].x;
    this->stripVertsBlue[2].y = TO_FIXED(240);
    this->stripVertsBlue[3].x = TO_FIXED(240) + this->stripVertsBlue[0].x;
    this->stripVertsBlue[3].y = TO_FIXED(240);

    this->stripVertsRed[0].x = this->stripPos[1];
    this->stripVertsRed[0].y = TO_FIXED(240);
    this->stripVertsRed[1].x = TO_FIXED(128) + this->stripVertsRed[0].x;
    this->stripVertsRed[1].y = TO_FIXED(240);
    this->stripVertsRed[2].x = TO_FIXED(230) + this->stripVertsRed[0].x;
    this->stripVertsRed[2].y = TO_FIXED(240);
    this->stripVertsRed[3].x = TO_FIXED(102) + this->stripVertsRed[0].x;
    this->stripVertsRed[3].y = TO_FIXED(240);

    this->stripVertsOrange[0].x = this->stripPos[2];
    this->stripVertsOrange[0].y = TO_FIXED(240);
    this->stripVertsOrange[1].x = TO_FIXED(240) + this->stripVertsOrange[0].x;
    this->stripVertsOrange[1].y = TO_FIXED(240);
    this->stripVertsOrange[2].x = TO_FIXED(262) + this->stripVertsOrange[0].x;
    this->stripVertsOrange[2].y = TO_FIXED(240);
    this->stripVertsOrange[3].x = TO_FIXED(166) + this->stripVertsOrange[0].x;
    this->stripVertsOrange[3].y = TO_FIXED(240);

    this->stripVertsGreen[0].x = this->stripPos[3];
    this->stripVertsGreen[0].y = TO_FIXED(240);
    this->stripVertsGreen[1].x = TO_FIXED(32) + this->stripVertsGreen[0].x;
    this->stripVertsGreen[1].y = TO_FIXED(240);
    this->stripVertsGreen[2].x = TO_FIXED(160) + this->stripVertsGreen[0].x;
    this->stripVertsGreen[2].y = TO_FIXED(240);
    this->stripVertsGreen[3].x = TO_FIXED(128) + this->stripVertsGreen[0].x;
    this->stripVertsGreen[3].y = TO_FIXED(240);

    this->bgLCurtainVerts[0].x = TO_FIXED(0);
    this->bgLCurtainVerts[0].y = TO_FIXED(0);
    this->bgLCurtainVerts[1].x = (this->stripVertsBlue[1].x + this->stripVertsBlue[0].x) >> 1;
    this->bgLCurtainVerts[1].y = TO_FIXED(0);
    this->bgLCurtainVerts[2].x = (this->stripVertsBlue[3].x + this->stripVertsBlue[2].x) >> 1;
    this->bgLCurtainVerts[2].y = TO_FIXED(240);
    this->bgLCurtainVerts[3].x = TO_FIXED(0);
    this->bgLCurtainVerts[3].y = TO_FIXED(240);

    this->bgRCurtainVerts[0].x = (this->stripVertsBlue[1].x + this->stripVertsBlue[0].x) >> 1;
    this->bgRCurtainVerts[0].y = TO_FIXED(0);
    this->bgRCurtainVerts[1].x = TO_FIXED(screenInfo->size.x);
    this->bgRCurtainVerts[1].y = TO_FIXED(0);
    this->bgRCurtainVerts[2].x = TO_FIXED(screenInfo->size.x);
    this->bgRCurtainVerts[2].y = TO_FIXED(240);
    this->bgRCurtainVerts[3].x = (this->stripVertsBlue[3].x + this->stripVertsBlue[2].x) >> 1;
    this->bgRCurtainVerts[3].y = TO_FIXED(240);
}
void TitleCard::SetupTitleWords()
{
    if (!this->zoneName.chars)
        this->zoneName = "UNTITLED";

    this->zoneName.SetSpriteString(sVars->aniFrames, 1);

    int32 offset = TO_FIXED(40);
    for (int32 c = 0; c < this->zoneName.length; ++c) {
        this->charPos[c].y = offset;
        this->charVel[c]   = -TO_FIXED(8);
        offset += TO_FIXED(16);
    }

    for (int32 i = 0; i < 4; ++i) {
        this->zoneCharPos[i] = ((2 - this->zoneName.length) << 19) - ((i * 2) << 19);
        this->zoneCharVel[i] = TO_FIXED(4);
    }

    for (int32 c = 0; c < this->zoneName.length; ++c) {
        if (this->zoneName.chars[c] == (uint16)-1)
            this->titleCardWord2 = c;
    }

    if (this->titleCardWord2) {
        this->word1Width = TO_FIXED(this->zoneName.GetWidth(sVars->aniFrames, 1, 0, this->titleCardWord2 - 1, 1) + 24);
        this->word2Width = TO_FIXED(this->zoneName.GetWidth(sVars->aniFrames, 1, this->titleCardWord2, 0, 1) + 24);
    }
    else {
        this->word2Width = TO_FIXED(this->zoneName.GetWidth(sVars->aniFrames, 1, 0, 0, 1) + 24);
    }

    this->zoneXPos  = TO_FIXED(screenInfo->center.x - ((screenInfo->center.x - 160) >> 3) + 72);
    this->word2XPos = TO_FIXED(screenInfo->center.x - ((screenInfo->center.x - 160) >> 3) + 72);

    if (this->word2Width < TO_FIXED(128))
        this->word2XPos -= TO_FIXED(40);

    this->word1XPos = this->word1Width - this->word2Width + this->word2XPos - TO_FIXED(32);
}
void TitleCard::HandleWordMovement()
{
    if (this->titleCardWord2 > 0) {
        this->word2DecorVerts[1].x -= TO_FIXED(32);
        if (this->word2DecorVerts[1].x < this->word1XPos - TO_FIXED(16))
            this->word2DecorVerts[1].x = this->word1XPos - TO_FIXED(16);

        this->word2DecorVerts[2].x -= TO_FIXED(32);
        this->word2DecorVerts[0].x = this->word2DecorVerts[1].x - this->word1Width;
        if (this->word2DecorVerts[2].x < this->word1XPos)
            this->word2DecorVerts[2].x = this->word1XPos;

        this->word2DecorVerts[3].x = this->word2DecorVerts[2].x - this->word1Width;
    }

    this->word1DecorVerts[1].x -= TO_FIXED(32);
    if (this->word1DecorVerts[1].x < this->word2XPos - TO_FIXED(16))
        this->word1DecorVerts[1].x = this->word2XPos - TO_FIXED(16);

    this->word1DecorVerts[2].x -= TO_FIXED(32);
    this->word1DecorVerts[0].x = this->word1DecorVerts[1].x - this->word2Width;
    if (this->word1DecorVerts[2].x < this->word2XPos)
        this->word1DecorVerts[2].x = this->word2XPos;

    this->zoneDecorVerts[1].x += TO_FIXED(32);
    this->word1DecorVerts[3].x = this->word1DecorVerts[2].x - this->word2Width;
    if (this->zoneDecorVerts[1].x > this->zoneXPos - TO_FIXED(8))
        this->zoneDecorVerts[1].x = this->zoneXPos - TO_FIXED(8);

    this->zoneDecorVerts[2].x += TO_FIXED(32);
    this->zoneDecorVerts[0].x = this->zoneDecorVerts[1].x - TO_FIXED(120);
    if (this->zoneDecorVerts[2].x > this->zoneXPos)
        this->zoneDecorVerts[2].x = this->zoneXPos;

    this->zoneDecorVerts[3].x = this->zoneDecorVerts[2].x - TO_FIXED(120);

    if (this->decorationPos.y < TO_FIXED(12)) {
        this->decorationPos.x += TO_FIXED(2);
        this->decorationPos.y += TO_FIXED(2);
    }
}
void TitleCard::HandleZoneCharMovement()
{
    for (int32 c = 0; c < this->zoneName.length; ++c) {
        if (this->charPos[c].y < 0)
            this->charVel[c] += 0x28000;

        this->charPos[c].y += this->charVel[c];
        if (this->charPos[c].y > 0 && this->charVel[c] > 0)
            this->charPos[c].y = 0;
    }

    for (int32 i = 0; i < 4; ++i) {
        if (this->zoneCharPos[i] > 0)
            this->zoneCharVel[i] -= 0x14000;

        this->zoneCharPos[i] += this->zoneCharVel[i];
        if (this->zoneCharPos[i] < 0 && this->zoneCharVel[i] < 0)
            this->zoneCharPos[i] = 0;
    }
}
void TitleCard::HandleCamera()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->camera)
            player->camera->offset.y = 0;
    }
}

// States
void TitleCard::State_SetupBGElements()
{
    SET_CURRENT_STATE();

    if (ActClear::sVars && ActClear::sVars->actClearActive)
        ActClear::sVars->actClearActive = false;

    Zone::ApplyWorldBounds();

    if (!globals->atlEnabled && !globals->suppressTitlecard)
        Stage::SetEngineState(ENGINESTATE_PAUSED);

    this->timer += 24;
    if (this->timer >= 512) {
        this->word2DecorVerts[0].y += TO_FIXED(32);
        this->word2DecorVerts[1].y += TO_FIXED(32);
        this->word2DecorVerts[2].y += TO_FIXED(32);
        this->word2DecorVerts[3].y += TO_FIXED(32);

        this->word1DecorVerts[0].y -= TO_FIXED(32);
        this->word1DecorVerts[1].y -= TO_FIXED(32);
        this->word1DecorVerts[2].y -= TO_FIXED(32);
        this->word1DecorVerts[3].y -= TO_FIXED(32);

        this->zoneDecorVerts[0].y += TO_FIXED(32);
        this->zoneDecorVerts[1].y += TO_FIXED(32);
        this->zoneDecorVerts[2].y += TO_FIXED(32);
        this->zoneDecorVerts[3].y += TO_FIXED(32);

        this->state.Set(&TitleCard::State_OpeningBG);
    }

    this->word2DecorVerts[0].x += TO_FIXED(40);
    this->word2DecorVerts[1].x += TO_FIXED(40);
    this->word2DecorVerts[2].x += TO_FIXED(40);
    this->word2DecorVerts[3].x += TO_FIXED(40);

    this->word1DecorVerts[0].x += TO_FIXED(40);
    this->word1DecorVerts[1].x += TO_FIXED(40);
    this->word1DecorVerts[2].x += TO_FIXED(40);
    this->word1DecorVerts[3].x += TO_FIXED(40);

    this->zoneDecorVerts[0].x -= TO_FIXED(40);
    this->zoneDecorVerts[1].x -= TO_FIXED(40);
    this->zoneDecorVerts[2].x -= TO_FIXED(40);
    this->zoneDecorVerts[3].x -= TO_FIXED(40);
}
void TitleCard::State_OpeningBG()
{
    SET_CURRENT_STATE();

    Zone::ApplyWorldBounds();

    if (this->timer >= 1024) {
        this->state.Set(&TitleCard::State_EnterTitle);
        this->stateDraw.Set(&TitleCard::Draw_ShowTitleCard);
    }
    else {
        this->timer += 32;
    }

    HandleWordMovement();
}
void TitleCard::State_EnterTitle()
{
    SET_CURRENT_STATE();

    Zone::ApplyWorldBounds();

    this->vertMovePos[0].x += (this->vertTargetPos[0].x - this->vertMovePos[0].x - TO_FIXED(16)) / 6;
    if (this->vertMovePos[0].x < this->vertTargetPos[0].x)
        this->vertMovePos[0].x = this->vertTargetPos[0].x;

    this->vertMovePos[0].y += (this->vertTargetPos[0].y - this->vertMovePos[0].y - TO_FIXED(16)) / 6;
    if (this->vertMovePos[0].y < this->vertTargetPos[0].y)
        this->vertMovePos[0].y = this->vertTargetPos[0].y;

    this->vertMovePos[1].x += (this->vertTargetPos[1].x - this->vertMovePos[1].x - TO_FIXED(16)) / 6;
    if (this->vertMovePos[1].x < this->vertTargetPos[1].x)
        this->vertMovePos[1].x = this->vertTargetPos[1].x;

    this->vertMovePos[1].y += (this->vertTargetPos[1].y - this->vertMovePos[1].y - TO_FIXED(16)) / 6;
    if (this->vertMovePos[1].y < this->vertTargetPos[1].y)
        this->vertMovePos[1].y = this->vertTargetPos[1].y;

    this->stripVertsBlue[0].x = (this->vertMovePos[0].x - TO_FIXED(240)) + this->stripVertsBlue[3].x;
    this->stripVertsBlue[0].y = this->vertMovePos[0].x;
    this->stripVertsBlue[1].x = (this->vertMovePos[0].x - TO_FIXED(240)) + this->stripVertsBlue[2].x;
    this->stripVertsBlue[1].y = this->vertMovePos[0].x;

    this->stripVertsRed[0].x = (this->vertMovePos[0].y - TO_FIXED(240)) + this->stripVertsRed[3].x;
    this->stripVertsRed[0].y = this->vertMovePos[0].y;
    this->stripVertsRed[1].x = (this->vertMovePos[0].y - TO_FIXED(240)) + this->stripVertsRed[2].x;
    this->stripVertsRed[1].y = this->vertMovePos[0].y;

    this->stripVertsOrange[0].x = (this->vertMovePos[1].x - TO_FIXED(240)) + this->stripVertsOrange[3].x;
    this->stripVertsOrange[0].y = this->vertMovePos[1].x;
    this->stripVertsOrange[1].x = (this->vertMovePos[1].x - TO_FIXED(240)) + this->stripVertsOrange[2].x;
    this->stripVertsOrange[1].y = this->vertMovePos[1].x;

    this->stripVertsGreen[0].x = (this->vertMovePos[1].y - TO_FIXED(240)) + this->stripVertsGreen[3].x;
    this->stripVertsGreen[0].y = this->vertMovePos[1].y;
    this->stripVertsGreen[1].x = (this->vertMovePos[1].y - TO_FIXED(240)) + this->stripVertsGreen[2].x;
    this->stripVertsGreen[1].y = this->vertMovePos[1].y;

    HandleWordMovement();
    HandleZoneCharMovement();

    if (this->actNumScale < 0x300)
        this->actNumScale += 0x40;

    if (!this->zoneCharPos[3] && this->zoneCharVel[3] < 0)
        this->state.Set(&TitleCard::State_ShowingTitle);
}
void TitleCard::State_ShowingTitle()
{
    SET_CURRENT_STATE();

    Zone::ApplyWorldBounds();
    HandleCamera();

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
}
void TitleCard::State_SlideAway()
{
    SET_CURRENT_STATE();

    Zone::ApplyWorldBounds();

    int32 speed = ++this->actionTimer << 18;
    this->stripVertsGreen[0].x -= speed;
    this->stripVertsGreen[0].y -= speed;
    this->stripVertsGreen[1].x -= speed;
    this->stripVertsGreen[1].y -= speed;
    this->stripVertsGreen[2].x -= speed;
    this->stripVertsGreen[2].y -= speed;
    this->stripVertsGreen[3].x -= speed;
    this->stripVertsGreen[3].y -= speed;

    if (this->actionTimer > 6) {
        speed = (this->actionTimer - 6) << 18;
        this->stripVertsOrange[0].x -= speed;
        this->stripVertsOrange[0].y -= speed;
        this->stripVertsOrange[1].x -= speed;
        this->stripVertsOrange[1].y -= speed;
        this->stripVertsOrange[2].x -= speed;
        this->stripVertsOrange[2].y -= speed;
        this->stripVertsOrange[3].x -= speed;
        this->stripVertsOrange[3].y -= speed;
        this->decorationPos.x += speed;
        this->decorationPos.y += speed;
    }

    if (this->actionTimer > 12) {
        speed = (this->actionTimer - 12) << 18;
        this->stripVertsRed[0].x -= speed;
        this->stripVertsRed[0].y -= speed;
        this->stripVertsRed[1].x -= speed;
        this->stripVertsRed[1].y -= speed;
        this->stripVertsRed[2].x -= speed;
        this->stripVertsRed[2].y -= speed;
        this->stripVertsRed[3].x -= speed;
        this->stripVertsRed[3].y -= speed;
    }

    if (this->actionTimer > 18) {
        speed = (this->actionTimer - 12) << 18;
        this->stripVertsBlue[0].x -= speed;
        this->stripVertsBlue[0].y -= speed;
        this->stripVertsBlue[1].x -= speed;
        this->stripVertsBlue[1].y -= speed;
        this->stripVertsBlue[2].x -= speed;
        this->stripVertsBlue[2].y -= speed;
        this->stripVertsBlue[3].x -= speed;
        this->stripVertsBlue[3].y -= speed;
    }

    if (this->actionTimer > 4) {
        speed = (this->actionTimer - 4) << 17;

        this->bgLCurtainVerts[0].x -= speed;
        this->bgLCurtainVerts[1].x -= speed;
        this->bgLCurtainVerts[2].x -= speed;
        this->bgLCurtainVerts[3].x -= speed;

        this->bgRCurtainVerts[0].x += speed;
        this->bgRCurtainVerts[1].x += speed;
        this->bgRCurtainVerts[2].x += speed;
        this->bgRCurtainVerts[3].x += speed;
    }

    if (this->actionTimer > 60) {
        speed = TO_FIXED(32);
        this->zoneXPos -= speed;
        this->word1XPos -= speed;
        this->word2XPos += speed;
        this->actNumPos.x += speed;
        this->actNumPos.y += speed;
        this->word2DecorVerts[0].x -= speed;
        this->word2DecorVerts[1].x -= speed;
        this->word2DecorVerts[2].x -= speed;
        this->word2DecorVerts[3].x -= speed;
        this->word1DecorVerts[0].x += speed;
        this->word1DecorVerts[1].x += speed;
        this->word1DecorVerts[2].x += speed;
        this->word1DecorVerts[3].x += speed;
        this->zoneDecorVerts[0].x -= speed;
        this->zoneDecorVerts[1].x -= speed;
        this->zoneDecorVerts[2].x -= speed;
        this->zoneDecorVerts[3].x -= speed;
    }

    if (this->actionTimer == 6 && globals->gameMode < MODE_TIMEATTACK) {
        sceneInfo->timeEnabled = true;
    }

    if (this->actionTimer > 80) {
        globals->atlEnabled  = false;
        globals->enableIntro = false;
        if (globals->gameMode >= MODE_TIMEATTACK) {
            if (globals->gameMode == MODE_COMPETITION) {
                // Competition::ClearMatchData();
                // Announcer::StartCountdown();
            }
        }
        else {
            globals->suppressTitlecard = false;
            globals->suppressAutoMusic = false;
        }
        this->Destroy();
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

// Draw States
void TitleCard::Draw_SlideIn()
{
    SET_CURRENT_STATE();

    color colors[5];
    colors[0] = this->colors[0];
    colors[1] = this->colors[1];
    colors[2] = this->colors[2];
    colors[3] = this->colors[3];
    colors[4] = this->colors[4];

    // The big ol' BG
    if (!globals->atlEnabled && !globals->suppressTitlecard) {
        if (this->timer < 256)
            Graphics::DrawRect(0, 0, screenInfo->size.x, screenInfo->size.y, 0, 0xFF, INK_NONE, true);

        // Blue
        int32 height = this->timer;
        if (this->timer < 512)
            Graphics::DrawRect(0, screenInfo->center.y - (height >> 1), screenInfo->size.x, height, colors[3], 0xFF, INK_NONE, true);

        // Red
        height = this->timer - 128;
        if (this->timer > 128 && this->timer < 640)
            Graphics::DrawRect(0, screenInfo->center.y - (height >> 1), screenInfo->size.x, height, colors[2], 0xFF, INK_NONE, true);

        // Orange
        height = this->timer - 256;
        if (this->timer > 256 && this->timer < 768)
            Graphics::DrawRect(0, screenInfo->center.y - (height >> 1), screenInfo->size.x, height, colors[0], 0xFF, INK_NONE, true);

        // Green
        height = this->timer - 384;
        if (this->timer > 384 && this->timer < 896)
            Graphics::DrawRect(0, screenInfo->center.y - (height >> 1), screenInfo->size.x, height, colors[1], 0xFF, INK_NONE, true);

        // Yellow
        height = this->timer - 512;
        if (this->timer > 512)
            Graphics::DrawRect(0, screenInfo->center.y - (height >> 1), screenInfo->size.x, height, colors[4], 0xFF, INK_NONE, true);
    }

    // Draw the BG thingos
    if (this->titleCardWord2 > 0)
        Graphics::DrawFace(this->word2DecorVerts, 4, 0x00, 0x00, 0x00, 0xFF, INK_NONE);

    Graphics::DrawFace(this->word1DecorVerts, 4, 0x00, 0x00, 0x00, 0xFF, INK_NONE);
    Graphics::DrawFace(this->zoneDecorVerts, 4, 0xF0, 0xF0, 0xF0, 0xFF, INK_NONE);

    // Draw Act Number
    this->decorationAnimator.frameID = 2 * 0 + 1;
    this->decorationAnimator.DrawSprite(&this->decorationPos, true);
}
void TitleCard::Draw_ShowTitleCard()
{
    SET_CURRENT_STATE();

    color colors[5];
    colors[0] = this->colors[0];
    colors[1] = this->colors[1];
    colors[2] = this->colors[2];
    colors[3] = this->colors[3];
    colors[4] = this->colors[4];

    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    // Draw Yellow BG
    if (!globals->atlEnabled && !globals->suppressTitlecard)
        Graphics::DrawRect(0, 0, screenInfo->size.x, screenInfo->size.y, colors[4], 0xFF, INK_NONE, true);

    // Draw Orange Strip
    if (this->vertMovePos[1].x < TO_FIXED(240))
        Graphics::DrawFace(this->stripVertsOrange, 4, (colors[0] >> 16) & 0xFF, (colors[0] >> 8) & 0xFF, (colors[0] >> 0) & 0xFF, 0xFF, INK_NONE);

    // Draw Green Strip
    if (this->vertMovePos[1].y < TO_FIXED(240))
        Graphics::DrawFace(this->stripVertsGreen, 4, (colors[1] >> 16) & 0xFF, (colors[1] >> 8) & 0xFF, (colors[1] >> 0) & 0xFF, 0xFF, INK_NONE);

    // Draw Red Strip
    if (this->vertMovePos[0].y < TO_FIXED(240))
        Graphics::DrawFace(this->stripVertsRed, 4, (colors[2] >> 16) & 0xFF, (colors[2] >> 8) & 0xFF, (colors[2] >> 0) & 0xFF, 0xFF, INK_NONE);

    // Draw Blue Strip
    if (this->vertMovePos[0].x < TO_FIXED(240))
        Graphics::DrawFace(this->stripVertsBlue, 4, (colors[3] >> 16) & 0xFF, (colors[3] >> 8) & 0xFF, (colors[3] >> 0) & 0xFF, 0xFF, INK_NONE);

    // Draw "Sonic Mania"
    if (!globals->atlEnabled && !globals->suppressTitlecard) {
        this->decorationAnimator.frameID = 2 * 0 + 1;
        this->decorationAnimator.DrawSprite(&this->decorationPos, true);
    }

    // Draw the BG thingos
    if (this->titleCardWord2 > 0)
        Graphics::DrawFace(this->word2DecorVerts, 4, 0x00, 0x00, 0x00, 0xFF, INK_NONE);

    Graphics::DrawFace(this->word1DecorVerts, 4, 0x00, 0x00, 0x00, 0xFF, INK_NONE);
    Graphics::DrawFace(this->zoneDecorVerts, 4, 0xF0, 0xF0, 0xF0, 0xFF, INK_NONE);

    // Draw "ZONE"
    Graphics::SetClipBounds(sceneInfo->currentScreenID, 0, 170, screen->size.x, SCREEN_YSIZE);

    Vector2 drawPos;
    drawPos.x = this->zoneXPos;
    for (int32 i = 0; i < 4; ++i) {
        this->zoneLetterAnimator.frameID = i;
        drawPos.y                        = TO_FIXED(186) + this->zoneCharPos[i];
        this->zoneLetterAnimator.DrawSprite(&drawPos, true);
    }

    // Draw TitleCard Word 1 (if there are 2 words)
    if (this->titleCardWord2 > 0) {
        Graphics::SetClipBounds(sceneInfo->currentScreenID, 0, 0, screen->size.x, 130);
        drawPos.x = this->word1XPos - TO_FIXED(20);
        drawPos.y = TO_FIXED(114);
        this->nameLetterAnimator.DrawString(&drawPos, &this->zoneName, 0, this->titleCardWord2, 2, 1, this->charPos, true);
    }

    // Draw TitleCard Word 2 (if there are 2 words, otherwise draw the entire zoneName)
    Graphics::SetClipBounds(sceneInfo->currentScreenID, 0, 0, screen->size.x, 170);
    drawPos.y = TO_FIXED(154);
    drawPos.x = this->word2XPos - TO_FIXED(20);
    this->nameLetterAnimator.DrawString(&drawPos, &this->zoneName, this->titleCardWord2, 0, 2, 1, this->charPos, true);

    Graphics::SetClipBounds(sceneInfo->currentScreenID, 0, 0, screen->size.x, screen->size.y);

    // Draw Act Number
    if (this->actID != 3) {
        if (this->actNumScale > 0) {
            this->drawFX  = FX_SCALE;
            this->scale.x = MIN(this->actNumScale, 0x200);
            this->scale.y = 0x200;

            this->decorationAnimator.frameID = 0;
            this->decorationAnimator.DrawSprite(&this->actNumPos, true);

            this->scale.x = CLAMP(this->actNumScale - 0x100, 0, 0x200);
            this->actNumbersAnimator.DrawSprite(&this->actNumPos, true);
            this->drawFX = FX_NONE;
        }
    }
}
void TitleCard::Draw_SlideAway()
{
    SET_CURRENT_STATE();

    color colors[5];
    colors[0] = this->colors[0];
    colors[1] = this->colors[1];
    colors[2] = this->colors[2];
    colors[3] = this->colors[3];
    colors[4] = this->colors[4];

    if (!globals->atlEnabled && !globals->suppressTitlecard) {
        // Draw Yellow BG curtain "opening"
        Graphics::DrawFace(this->bgLCurtainVerts, 4, (colors[4] >> 16) & 0xFF, (colors[4] >> 8) & 0xFF, (colors[4] >> 0) & 0xFF, 0xFF, INK_NONE);
        Graphics::DrawFace(this->bgRCurtainVerts, 4, (colors[4] >> 16) & 0xFF, (colors[4] >> 8) & 0xFF, (colors[4] >> 0) & 0xFF, 0xFF, INK_NONE);
    }

    // Orange Strip
    if (this->vertMovePos[1].x < TO_FIXED(240))
        Graphics::DrawFace(this->stripVertsOrange, 4, (colors[0] >> 16) & 0xFF, (colors[0] >> 8) & 0xFF, (colors[0] >> 0) & 0xFF, 0xFF, INK_NONE);

    // Green Strip
    if (this->vertMovePos[1].y < TO_FIXED(240))
        Graphics::DrawFace(this->stripVertsGreen, 4, (colors[1] >> 16) & 0xFF, (colors[1] >> 8) & 0xFF, (colors[1] >> 0) & 0xFF, 0xFF, INK_NONE);

    // Red Strip
    if (this->vertMovePos[0].y < TO_FIXED(240))
        Graphics::DrawFace(this->stripVertsRed, 4, (colors[2] >> 16) & 0xFF, (colors[2] >> 8) & 0xFF, (colors[2] >> 0) & 0xFF, 0xFF, INK_NONE);

    // Blue Strip
    if (this->vertMovePos[0].x < TO_FIXED(240))
        Graphics::DrawFace(this->stripVertsBlue, 4, (colors[3] >> 16) & 0xFF, (colors[3] >> 8) & 0xFF, (colors[3] >> 0) & 0xFF, 0xFF, INK_NONE);

    // Draw "Sonic Mania"
    if (!globals->atlEnabled && !globals->suppressTitlecard) {
        this->decorationAnimator.frameID = 2 * 0 + 1;
        this->decorationAnimator.DrawSprite(&this->decorationPos, true);
    }

    // Draw Act Number
    if (this->actID != 3 && this->actNumScale > 0) {
        this->decorationAnimator.frameID = 0;
        this->decorationAnimator.DrawSprite(&this->actNumPos, true);
        this->actNumbersAnimator.DrawSprite(&this->actNumPos, true);
    }

    // Draw the BG thingos
    if (this->titleCardWord2 > 0)
        Graphics::DrawFace(this->word2DecorVerts, 4, 0x00, 0x00, 0x00, 0xFF, INK_NONE);

    Graphics::DrawFace(this->word1DecorVerts, 4, 0x00, 0x00, 0x00, 0xFF, INK_NONE);
    Graphics::DrawFace(this->zoneDecorVerts, 4, 0xF0, 0xF0, 0xF0, 0xFF, INK_NONE);

    // Draw "ZONE"
    Vector2 drawPos;
    drawPos.x = this->zoneXPos;
    drawPos.y = TO_FIXED(186);
    for (int32 i = 0; i < 4; ++i) {
        this->zoneLetterAnimator.frameID = i;
        this->zoneLetterAnimator.DrawSprite(&drawPos, true);
    }

    // Draw TitleCard Word 1 (if there are 2 words)
    if (this->titleCardWord2 > 0) {
        drawPos.y = TO_FIXED(114);
        drawPos.x = this->word1XPos - TO_FIXED(20);
        this->nameLetterAnimator.DrawString(&drawPos, &this->zoneName, 0, this->titleCardWord2, 2, 1, nullptr, true);
    }

    // Draw TitleCard Word 2 (if there are 2 words, otherwise draw the entire zoneName)
    drawPos.y = TO_FIXED(154);
    drawPos.x = this->word2XPos - TO_FIXED(20);
    this->nameLetterAnimator.DrawString(&drawPos, &this->zoneName, this->titleCardWord2, 0, 2, 1, nullptr, true);
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
}

} // namespace GameLogic