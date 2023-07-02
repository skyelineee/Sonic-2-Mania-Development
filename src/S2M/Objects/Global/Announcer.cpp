// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Announcer Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Announcer.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Helpers/MathHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Announcer);

void Announcer::Update()
{
    this->state.Run(this);
}

void Announcer::LateUpdate() {}

void Announcer::StaticUpdate() {}

void Announcer::Draw()
{
    this->stateDraw.Run(this);
}

void Announcer::Create(void *data)
{
    this->active        = ACTIVE_NORMAL;
    this->drawGroup     = 13;
    this->visible       = true;
    this->drawFX        = FX_FLIP | FX_SCALE;
    this->isPermanent   = true;
    this->updateRange.x = TO_FIXED(128);
    this->updateRange.y = TO_FIXED(128);
}

void Announcer::StageLoad()
{
    if (globals->suppressTitlecard) {
        sVars->finishedCountdown = true;
    }
    else {
        sVars->finishedCountdown = false;
    }

    if (Stage::CheckSceneFolder("Menu")) {
        sVars->sfxSonic.Get("VO/Sonic.wav");
        sVars->sfxTails.Get("VO/Tails.wav");
        sVars->sfxKnuckles.Get("VO/Knuckles.wav");
    }
    else if (globals->gameMode == MODE_TIMEATTACK) {
        sVars->aniFrames.Load("Global/Announcer.bin", SCOPE_STAGE);

        sVars->sfxNewRecordTop.Get("VO/NewRecordTop.wav");
        sVars->sfxNewRecordMid.Get("VO/NewRecordMid.wav");
        sVars->sfxThree.Get("VO/Three.wav");
        sVars->sfxTwo.Get("VO/Two.wav");
        sVars->sfxOne.Get("VO/One.wav");
        sVars->sfxGo.Get("VO/Go.wav");
        sVars->sfxGoal.Get("VO/Goal.wav");
    }
}

void Announcer::StartCountdown()
{
    sVars->finishedCountdown = false;
    Announcer *announcer         = GameObject::Create<Announcer>(nullptr, 0, 0);
    announcer->state.Set(&Announcer::State_Countdown);
    announcer->stateDraw.Set(&Announcer::Draw_Countdown);
    announcer->playerID          = 3;
    announcer->scale.x           = 0x200;
    announcer->scale.y           = 0x200;
}

void Announcer::AnnounceGoal(int32 screen)
{
    Announcer *announcer       = GameObject::Create<Announcer>(nullptr, 0, 0);
    announcer->state.Set(&Announcer::State_Finished);
    announcer->stateDraw.Set(&Announcer::Draw_Finished);
    announcer->screen = screen;
    announcer->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    sVars->sfxGoal.Play(false, 255);
}

void Announcer::Draw_Countdown()
{
    Vector2 drawPos;
    drawPos.y = (screenInfo->center.y - 32) << 16;
    drawPos.x = screenInfo->center.x << 16;
    drawPos.x += this->drawOffset.x;
    drawPos.y += this->drawOffset.y;
    this->animator.DrawSprite(&drawPos, true);
}

void Announcer::Draw_Finished()
{
    Vector2 drawPos;
    if (sceneInfo->currentScreenID == this->screen) {
        drawPos.x = screenInfo->center.x << 16;
        drawPos.y = (screenInfo->center.y - 32) << 16;
        drawPos.x += this->drawOffset.x;
        drawPos.y += this->drawOffset.y;
        this->animator.DrawSprite(&drawPos, true);
    }
}

void Announcer::State_Countdown()
{
    this->inkEffect = INK_ALPHA;
    if (this->playerID <= 0) {
        if (this->timer >= 60) {
            this->Destroy();
        }
        else {
            if (!this->timer) {
                sVars->sfxGo.Play(false, 255);
                this->animator.SetAnimation(sVars->aniFrames, 1, true, 3);
            }

            int32 timer = 0;
            if (this->timer - 15 > 0)
                timer = (this->timer - 15) << 9;
            this->alpha = 0x200 - timer / 45;

            this->timer++;
        }
    }
    else {
        if (this->timer >= 45) {
            this->timer = 0;

            this->playerID--;
            if (!this->playerID) {
                sVars->finishedCountdown     = true;
                sceneInfo->timeEnabled       = true;
            }
        }
        else {
            if (!this->timer) {
                switch (this->playerID) {
                    default: break;

                    case 1:
                        sVars->sfxOne.Play(false, 255);
                        this->animator.SetAnimation(sVars->aniFrames, 1, true, 2);
                        break;

                    case 2:
                        sVars->sfxTwo.Play(false, 255);
                        this->animator.SetAnimation(sVars->aniFrames, 1, true, 1);
                        break;

                    case 3:
                        sVars->sfxThree.Play(false, 255);
                        this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);
                        break;
                }
            }
            this->drawOffset.x = 0;
            this->drawOffset.y = 0;
            this->alpha        = 0x200 - (this->timer << 9) / 45;
            this->timer++;
        }
    }
}
void Announcer::State_Finished()
{
    this->scale.x = 0x200;
    this->scale.y = 0x200;
    if (this->timer >= 16) {
        if (this->timer >= 76) {
            if (this->timer >= 92) {
                this->Destroy();
            }
            else {
                this->visible = true;
                MathHelpers::Lerp(&this->drawOffset, ((this->timer - 76) << 8) / 16, 0, 0, screenInfo->size.x << 16, 0);
                ++this->timer;
            }
        }
        else {
            ++this->timer;
            this->drawOffset.x = 0;
            this->drawOffset.y = 0;
        }
    }
    else {
        this->visible = true;
        int32 t       = 16 * this->timer;
        int32 xOffset = -TO_FIXED(1) * screenInfo->size.x;
        if (t > 0) {
            if (t < 256)
                this->drawOffset.x = xOffset + t * (-xOffset >> 8);
            else
                this->drawOffset.x = 0;
            this->drawOffset.y = 0;
        }
        else {
            this->drawOffset.x = xOffset;
            this->drawOffset.y = 0;
        }
        ++this->timer;
    }
}

#if RETRO_INCLUDE_EDITOR
void Announcer::EditorDraw() {}

void Announcer::EditorLoad() {}
#endif

void Announcer::Serialize() {}

} // namespace GameLogic