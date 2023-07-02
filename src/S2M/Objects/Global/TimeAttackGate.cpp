// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TimeAttackGate Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "TimeAttackGate.hpp"
#include "PauseMenu.hpp"
#include "DebugMode.hpp"
#include "Zone.hpp"
#include "Music.hpp"
#include "ActClear.hpp"
#include "HUD.hpp"
#include "Announcer.hpp"
#include "ReplayRecorder.hpp"
#include "Helpers/MathHelpers.hpp"
#include "Helpers/DrawHelpers.hpp"
#include "Helpers/MenuParam.hpp"
#include "Helpers/TimeAttackData.hpp"
#include "Menu/UILoadingIcon.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TimeAttackGate);

void TimeAttackGate::Update()
{
    if (GameObject::Get<PauseMenu>(SLOT_PAUSEMENU)->classID != PauseMenu::sVars->classID) {
        this->state.Run(this);
    }
}

void TimeAttackGate::LateUpdate() {}

void TimeAttackGate::StaticUpdate()
{
    if (globals->gameMode == MODE_TIMEATTACK) {
        Player *player = GameObject::Get<Player>(SLOT_PLAYER1);

        if (sVars->isFinished || !sVars->started)
            player->drownTimer = 0;

        if (sVars->suppressedTitlecard) {
            TimeAttackGate *entity              = GameObject::Create<TimeAttackGate>(INT_TO_VOID(true), 0, 0);
            entity->state.Set(&TimeAttackGate::State_Fadeout);
            entity->fadeTimer                   = 532;
            sVars->suppressedTitlecard          = false;
        }

        if (DebugMode::sVars->debugActive)
            sVars->disableRecords = true;
    }
}

void TimeAttackGate::Draw() { this->stateDraw.Run(this); }

void TimeAttackGate::Create(void *data)
{
    if (sceneInfo->inEditor || globals->gameMode == MODE_TIMEATTACK) {
        this->baseAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->topAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

        this->visible = true;
        if (this->finishLine)
            this->finAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
        else
            this->finAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);

        if (!sceneInfo->inEditor) {
            if (data == INT_TO_VOID(true)) {
                this->drawGroup = 14;
                this->active    = ACTIVE_ALWAYS;
                this->state.Set(&TimeAttackGate::State_Restarter);
                this->stateDraw.Set(&TimeAttackGate::Draw_Restarter);
            }
            else {
                this->active        = ACTIVE_NORMAL;
                this->updateRange.x = TO_FIXED(64);
                this->updateRange.y = TO_FIXED(64);
                this->drawGroup     = Zone::sVars->playerDrawGroup[0] + 1;

                int32 left   = this->boundsOffset.x - (this->boundsSize.x >> 1);
                int32 top    = this->boundsOffset.y - (this->boundsSize.y >> 1);
                int32 right  = this->boundsOffset.x + (this->boundsSize.x >> 1);
                int32 bottom = this->boundsOffset.y + (this->boundsSize.y >> 1);

                int32 extendTop    = -(this->extendTop << 16);
                int32 extendBottom = (this->extendBottom << 16);

                if (extendTop < top)
                    top = extendTop;

                if (extendBottom < bottom)
                    bottom = extendBottom;

                if (abs(left) > right)
                    this->updateRange.x = abs(left);
                else
                    this->updateRange.x = right;

                if (abs(top) > bottom)
                    this->updateRange.y = abs(top);
                else
                    this->updateRange.y = bottom;

                this->updateRange.x += TO_FIXED(64);
                this->updateRange.y += TO_FIXED(64);

                this->scale.y   = 0x200;
                this->state.Set(&TimeAttackGate::State_Gate);
                this->stateDraw.Set(&TimeAttackGate::Draw_Gate);
            }
        }
    }
    else {
        this->Destroy();
    }
}

void TimeAttackGate::StageLoad()
{
    sVars->teleportChannel = -1;

    sVars->aniFrames.Load("Global/SpeedGate.bin", SCOPE_STAGE);

    sVars->hitboxGate.left   = -8;
    sVars->hitboxGate.top    = -44;
    sVars->hitboxGate.right  = 8;
    sVars->hitboxGate.bottom = 20;

    sVars->sfxSignpost.Get("Global/SignPost.wav");
    sVars->sfxTeleport.Get("Global/Teleport.wav");

    sVars->started        = false;
    sVars->disableRecords = false;
    sceneInfo->timeEnabled         = false;

    if (globals->suppressTitlecard)
        sVars->suppressedTitlecard = true;
}

void TimeAttackGate::HandleSpin()
{
    int32 top    = this->position.y + ((sVars->hitboxGate.top - this->extendTop) << 16);
    int32 bottom = this->position.y + ((sVars->hitboxGate.bottom + this->extendBottom) << 16);

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (!player->sidekick && !player->isGhost) {
            if (!this->playerPos.x || !this->playerPos.y) {
                break;
            }

            int32 xVel = player->onGround ? player->groundVel : player->velocity.x;
            if (xVel >> 15
                && MathHelpers::CheckPositionOverlap(player->position.x, player->position.y, this->playerPos.x, this->playerPos.y, this->position.x,
                                                    bottom, this->position.x, top)) {
                int32 spinSpeed = (xVel >> 15) - 2;
                if ((xVel >> 15) >= 0)
                    spinSpeed = (xVel >> 15) + 2;

                if (abs(spinSpeed) > abs(this->spinSpeed))
                    this->spinSpeed = spinSpeed;

                this->spinTimer = 0;
            }
        }
    }

    bool32 finishedSpinning = false;

    this->angle += this->spinSpeed;
    if (this->spinSpeed <= 0) {
        if (this->angle <= -0x200) {
            ++this->spinTimer;
            this->angle += 0x200;

            this->spinSpeed += 4;
            if (this->spinSpeed > -2)
                this->spinSpeed = -2;

            finishedSpinning = this->spinSpeed == -2;
        }
    }
    else {
        if (this->angle >= 0x200) {
            ++this->spinTimer;
            this->angle -= 0x200;

            this->spinSpeed -= 4;
            if (this->spinSpeed < 2)
                this->spinSpeed = 2;

            finishedSpinning = this->spinSpeed == 2;
        }
    }

    if (finishedSpinning) {
        this->spinSpeed = 0;
        this->angle     = 0;
    }
}

void TimeAttackGate::HandleStart()
{
    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);
    int32 top             = ((sVars->hitboxGate.top - this->extendTop) << 16) + this->position.y;
    int32 bottom          = ((sVars->hitboxGate.bottom + this->extendBottom) << 16) + this->position.y;

    if (MathHelpers::CheckPositionOverlap(player1->position.x, player1->position.y, this->playerPos.x, this->playerPos.y, this->position.x, bottom,
                                         this->position.x, top)) {
        this->hasFinished = true;
        if (!this->finishLine) {
            if (!sVars->started) {
                sVars->sfxSignpost.Play(false, 255);

                sVars->triggerPlayer          = player1;
                sVars->started                = true;
                sceneInfo->timeEnabled        = true;

                TimeAttackGate *restarter       = GameObject::Create<TimeAttackGate>(INT_TO_VOID(true), this->position.x, this->position.y);
                sVars->restartManager           = restarter;
                restarter->isPermanent          = true;

                sVars->startCB.Run(this);
            }
        }
        else if (sVars->started) {
            Music::FadeOut(0.025);
            Announcer::AnnounceGoal(0);

            sVars->sfxSignpost.Play(false, 255);
            sVars->triggerPlayer          = nullptr;
            sVars->started                = false;
            sceneInfo->timeEnabled        = false;

            sVars->endCB.Run(this);

            if (!sVars->disableRecords)
                ActClear::sVars->bufferMoveEnabled = true;

            TimeAttackGate::AddRecord();
        }
    }

    this->playerPos.x = player1->position.x;
    this->playerPos.y = player1->position.y;
}

void TimeAttackGate::AddRecord()
{
    if (!sVars->disableRecords) {
        if (ActClear::sVars)
            ActClear::sVars->isSavingGame = true;

        if (UILoadingIcon::sVars)
            UILoadingIcon::StartWait();

        MenuParam *param = MenuParam::GetMenuParam();
        int32 characterID      = param->characterID;
        int32 zoneID           = param->zoneID;
        int32 score            = sceneInfo->milliseconds + 100 * (sceneInfo->seconds + 60 * sceneInfo->minutes);
        int32 act              = param->actID;

        param->timeAttackRank = TimeAttackData::AddRecord(zoneID, act, characterID, score, TimeAttackGate::WaitSave_Leaderboards);
        TimeAttackData::AddLeaderboardEntry(zoneID, act, characterID, score);
    }
}

void TimeAttackGate::WaitSave_Leaderboards(bool32 success)
{
    if (ActClear::sVars)
        ActClear::sVars->isSavingGame = false;

    if (UILoadingIcon::sVars)
        UILoadingIcon::FinishWait();
}

void TimeAttackGate::CheckTouch()
{
    int32 x = this->position.x + this->boundsOffset.x;
    int32 y = this->position.y + this->boundsOffset.y;

    Hitbox hitbox;
    hitbox.left   = -(this->boundsSize.x >> 17);
    hitbox.right  = this->boundsSize.x >> 17;
    hitbox.top    = -(this->boundsSize.y >> 17);
    hitbox.bottom = this->boundsSize.y >> 17;

    for (int32 p = 0; p < Player::sVars->playerCount; ++p) {
        Player *player = GameObject::Get<Player>(p);

        bool32 passedGate = false;
        if (this->boundsSize.x <= 0 || this->boundsSize.y <= 0) {
            passedGate = this->position.x - player->position.x < TO_FIXED(256);
        }
        else {
            int32 storeX     = this->position.x;
            int32 storeY     = this->position.y;
            this->position.x = x;
            this->position.y = y;
            passedGate       = player->CheckCollisionTouch(this, &hitbox);
            this->position.x = storeX;
            this->position.y = storeY;
        }

        if (passedGate) {
            Zone::sVars->cameraBoundsL[p] = (this->position.x >> 16) - screenInfo[p].center.x;
            Zone::sVars->cameraBoundsR[p] = (this->position.x >> 16) + screenInfo[p].center.x;

            if (this->topBoundary)
                Zone::sVars->cameraBoundsT[p] = (this->position.y) - screenInfo[p].center.y;
        }
    }
}

void TimeAttackGate::State_Gate()
{
    TimeAttackGate::HandleSpin();
    TimeAttackGate::HandleStart();

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        player->lives = 1;

        if (!player->sidekick) {
            if (!this->finishLine) {
                if (!sVars->started) {
                    for (auto hud : GameObject::GetEntities<HUD>(FOR_ACTIVE_ENTITIES))
                    {
                        if (hud)
                            hud->enableTimeFlash = true;
                        break;
                    }
                }
            }
            else {
                if (this->hasFinished && !player->stateInput.Matches(nullptr)) {
                    player->stateInput.Set(nullptr);
                    player->up         = false;
                    player->left       = false;
                    player->right      = false;
                    player->down       = false;
                    player->jumpPress  = false;
                    player->jumpHold   = false;
                }

                if (!this->finishLine && !sVars->started) {
                    for (auto hud : GameObject::GetEntities<HUD>(FOR_ACTIVE_ENTITIES))
                    {
                        if (hud)
                            hud->enableTimeFlash = true;
                        break;
                    }
                }
            }

            this->playerPos.x = player->position.x;
            this->playerPos.y = player->position.y;
        }
    }

    if (this->finishLine) {
        TimeAttackGate::CheckTouch();

        if (this->hasFinished) {
            sVars->isFinished = true;

            if (this->timer >= 180) {
                if (this->timer == 180) {
                    Music::PlayTrack(Music::TRACK_ACTCLEAR);
                    GameObject::Reset(SLOT_ACTCLEAR, ActClear::sVars->classID, nullptr);
                    ++this->timer;
                }
            }
            else {
                this->timer++;
            }
        }
    }
}

void TimeAttackGate::State_Restarter()
{
    if (sVars->isFinished) {
        this->Destroy();
    }
    else {
        Player *player = sVars->triggerPlayer;

        if (player) {
            if (player->state.Matches(&Player::State_Death) || player->state.Matches(&Player::State_Drown)) {
                this->restartTimer = 0;
            }
            else {
                if (controllerInfo[player->controllerID].keyY.press)
                    this->restartTimer = 0;

                if (!controllerInfo[player->controllerID].keyY.down) {
                    if (this->restartTimer > 0) {
                        sVars->teleportChannel = -1;
                        sVars->sfxTeleport.Stop();
                        this->restartTimer -= 4;
                    }

                    if (this->restartTimer < 0)
                        this->restartTimer = 0;
                }
                else {
                    if (!this->restartTimer)
                        sVars->teleportChannel = sVars->sfxTeleport.Play(false, 255);

                    if (this->restartTimer == 35) {
                        this->state.Set(nullptr);
                        globals->suppressTitlecard = true;

                        for (int32 c = 0; c < CHANNEL_COUNT; ++c) {
                            if (c != Music::sVars->channelID && c != sVars->teleportChannel)
                                RSDKTable->StopChannel(c);
                        }

                        if (!Music::sVars->activeTrack)
                            globals->suppressAutoMusic = true;

                        Stage::LoadScene();
                    }
                    else if (this->restartTimer < 35)
                        this->restartTimer++;
                }
            }

            this->radius = (720 * this->restartTimer / 35);
        }
    }
}

void TimeAttackGate::State_Fadeout()
{
    if (this->fadeTimer <= 0) {
        globals->suppressTitlecard = false;
        this->Destroy();
    }
    else {
        this->fadeTimer -= 32;
    }
}

void TimeAttackGate::Draw_Gate()
{
    Vector2 drawPos;

    this->rotation = this->baseRotation;
    this->drawFX   = FX_ROTATE;
    this->topAnimator.DrawSprite(nullptr, false);
    this->baseAnimator.DrawSprite(nullptr, false);

    this->drawFX = FX_SCALE;
    drawPos.y    = this->position.y;

    this->scale.x             = abs(Math::Sin512(this->angle & 0x7F));
    drawPos.x                 = this->position.x + TO_FIXED(3);
    this->finAnimator.frameID = 1;
    this->finAnimator.DrawSprite(&drawPos, false);

    this->scale.x             = abs(Math::Cos512(this->angle & 0x7F));
    drawPos.x                 = this->position.x - TO_FIXED(3);
    this->finAnimator.frameID = 0;
    this->finAnimator.DrawSprite(&drawPos, false);

    drawPos.x                 = 0x180 * Math::Cos512(2 * (this->angle & 0x7F)) + this->position.x;
    this->finAnimator.frameID = 1;
    this->finAnimator.DrawSprite(&drawPos, false);

    this->scale.x             = abs(Math::Sin512(this->angle & 0x7F));
    drawPos.x                 = 0xB40 * Math::Cos512(this->angle & 0x7F) + this->position.x;
    this->finAnimator.frameID = 2;
    this->finAnimator.DrawSprite(&drawPos, false);

    this->scale.x             = abs(Math::Sin512(this->angle & 0x7F));
    drawPos.x                 = 0x180 * Math::Cos512(2 * (this->angle & 0x7F)) + this->position.x;
    this->finAnimator.frameID = 0;
    this->finAnimator.DrawSprite(&drawPos, false);

    this->scale.x             = abs(Math::Cos512(this->angle & 0x7F));
    drawPos.x                 = this->position.x - 0xB40 * Math::Sin512(this->angle & 0x7F);
    this->finAnimator.frameID = 2;
    this->finAnimator.DrawSprite(&drawPos, false);
}

void TimeAttackGate::Draw_Restarter()
{
    Player *player = sVars->triggerPlayer;
    if (player) {
        if (this->radius > 0)
            Graphics::DrawCircle(player->position.x, player->position.y, this->radius, 0xF0F0F0, (this->restartTimer << 8) / 35, INK_ADD, false);

        if (this->fadeTimer > 0)
            Graphics::FillScreen(0xFFFFFF, this->fadeTimer, this->fadeTimer - 128, this->fadeTimer - 256);
    }
}

#if RETRO_INCLUDE_EDITOR
void TimeAttackGate::EditorDraw()
{
    Vector2 drawPos;

    if (this->finishLine)
        this->finAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
    else
        this->finAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);

    this->rotation = this->baseRotation;
    this->drawFX   = FX_ROTATE;
    this->topAnimator.DrawSprite(nullptr, false);
    this->baseAnimator.DrawSprite(nullptr, false);

    this->drawFX  = FX_SCALE;
    drawPos.y     = this->position.y;
    this->scale.y = 0x200;

    this->scale.x             = abs(Math::Sin512(this->angle & 0x7F));
    drawPos.x                 = this->position.x + TO_FIXED(3);
    this->finAnimator.frameID = 1;
    this->finAnimator.DrawSprite(&drawPos, false);

    this->scale.x             = abs(Math::Cos512(this->angle & 0x7F));
    drawPos.x                 = this->position.x - TO_FIXED(3);
    this->finAnimator.frameID = 0;
    this->finAnimator.DrawSprite(&drawPos, false);

    drawPos.x                 = 0x180 * Math::Cos512(2 * (this->angle & 0x7F)) + this->position.x;
    this->finAnimator.frameID = 1;
    this->finAnimator.DrawSprite(&drawPos, false);

    this->scale.x             = abs(Math::Sin512(this->angle & 0x7F));
    drawPos.x                 = 0xB40 * Math::Cos512(this->angle & 0x7F) + this->position.x;
    this->finAnimator.frameID = 2;
    this->finAnimator.DrawSprite(&drawPos, false);

    this->scale.x             = abs(Math::Sin512(this->angle & 0x7F));
    drawPos.x                 = 0x180 * Math::Cos512(2 * (this->angle & 0x7F)) + this->position.x;
    this->finAnimator.frameID = 0;
    this->finAnimator.DrawSprite(&drawPos, false);

    this->scale.x             = abs(Math::Cos512(this->angle & 0x7F));
    drawPos.x                 = this->position.x - 0xB40 * Math::Sin512(this->angle & 0x7F);
    this->finAnimator.frameID = 2;
    this->finAnimator.DrawSprite(&drawPos, false);

    this->updateRange.x = TO_FIXED(64);
    this->updateRange.y = TO_FIXED(64);
    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        int32 left   = this->boundsOffset.x - (this->boundsSize.x >> 1);
        int32 top    = this->boundsOffset.y - (this->boundsSize.y >> 1);
        int32 right  = this->boundsOffset.x + (this->boundsSize.x >> 1);
        int32 bottom = this->boundsOffset.y + (this->boundsSize.y >> 1);

        int32 extendTop    = -(this->extendTop << 16);
        int32 extendBottom = (this->extendBottom << 16);

        if (extendTop < top)
            top = extendTop;

        if (extendBottom < bottom)
            bottom = extendBottom;

        if (abs(left) > right)
            this->updateRange.x = abs(left);
        else
            this->updateRange.x = right;

        if (abs(top) > bottom)
            this->updateRange.y = abs(top);
        else
            this->updateRange.y = bottom;

        this->updateRange.x += TO_FIXED(64);
        this->updateRange.y += TO_FIXED(64);

        DrawHelpers::DrawArenaBounds(this, FROM_FIXED(left), FROM_FIXED(top), FROM_FIXED(right + 64), FROM_FIXED(bottom + 64), 1 | 2 | 4 | 8, 0xFFFF00);

        Hitbox hitbox = sVars->hitboxGate;
        hitbox.top    = sVars->hitboxGate.top - this->extendTop;
        hitbox.bottom = sVars->hitboxGate.bottom + this->extendBottom;
        DrawHelpers::DrawHitboxOutline(this->position.x, this->position.y, &hitbox, FLIP_NONE, 0xFF0000);

        RSDK_DRAWING_OVERLAY(false);
    }
}

void TimeAttackGate::EditorLoad(void)
{
    sVars->aniFrames.Load("Global/SpeedGate.bin", SCOPE_STAGE);

    sVars->hitboxGate.left   = -8;
    sVars->hitboxGate.top    = -44;
    sVars->hitboxGate.right  = 8;
    sVars->hitboxGate.bottom = 20;
}
#endif

void TimeAttackGate::Serialize()
{
    RSDK_EDITABLE_VAR(TimeAttackGate, VAR_BOOL, finishLine);
    RSDK_EDITABLE_VAR(TimeAttackGate, VAR_ENUM, extendTop);
    RSDK_EDITABLE_VAR(TimeAttackGate, VAR_ENUM, extendBottom);
    RSDK_EDITABLE_VAR(TimeAttackGate, VAR_VECTOR2, boundsSize);
    RSDK_EDITABLE_VAR(TimeAttackGate, VAR_VECTOR2, boundsOffset);
    RSDK_EDITABLE_VAR(TimeAttackGate, VAR_BOOL, topBoundary);
}


} // namespace GameLogic