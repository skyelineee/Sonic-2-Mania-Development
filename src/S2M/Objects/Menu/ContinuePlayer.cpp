// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ContinuePlayer Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "ContinuePlayer.hpp"
#include "Global/Player.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ContinuePlayer);

void ContinuePlayer::Update()
{
    this->animator.Process();

    this->state.Run(this);
}

void ContinuePlayer::LateUpdate() {}

void ContinuePlayer::StaticUpdate() {}

void ContinuePlayer::Draw() { this->animator.DrawSprite(nullptr, false); }

void ContinuePlayer::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->visible   = true;
        this->drawGroup = 1;
        this->active    = ACTIVE_NORMAL;

        if (!this->isPlayer2) {
            switch (GET_CHARACTER_ID(1)) {
                default:
                case ID_SONIC:
                    this->animator.SetAnimation(sVars->aniFrames, CONTPLR_ANI_IDLE_SONIC, true, 0);

                    if (globals->playerID & ID_TAILS_ASSIST)
                        this->position.x -= 0x100000;
                    break;

                case ID_TAILS:
                    this->animator.SetAnimation(sVars->aniFrames, CONTPLR_ANI_IDLE_TAILS, true, 0);
                    this->position.y += 0x40000;
                    break;

                case ID_KNUCKLES: this->animator.SetAnimation(sVars->aniFrames, CONTPLR_ANI_IDLE_KNUX, true, 0); break;
            }

            this->timer     = 60;
            this->aniFrames = sVars->playerAniFrames;
        }
        else {
            this->animator.SetAnimation(sVars->aniFrames, CONTPLR_ANI_IDLE_AI, true, 0);

            if (!(globals->playerID & ID_TAILS_ASSIST))
                this->active = ACTIVE_NEVER;

            this->timer     = 76;
            this->aniFrames = sVars->tailAniFrames;
        }
    }
}

void ContinuePlayer::StageLoad(void)
{
    sVars->aniFrames.Load("Players/Continue.bin", SCOPE_STAGE);

    if (globals->playerID == ID_NONE)
        globals->playerID = ID_DEFAULT_PLAYER;

    switch (GET_CHARACTER_ID(1)) {
        default:
        case ID_SONIC: sVars->playerAniFrames.Load("Players/Sonic.bin", SCOPE_STAGE); break;
        case ID_TAILS: sVars->playerAniFrames.Load("Players/Tails.bin", SCOPE_STAGE); break;
        case ID_KNUCKLES: sVars->playerAniFrames.Load("Players/Knux.bin", SCOPE_STAGE); break;
    }

    sVars->tailAniFrames.Load("Players/Tails.bin", SCOPE_STAGE);

    sVars->sfxRoll.Get("Global/Roll.wav");
    sVars->sfxCharge.Get("Global/Charge.wav");
    sVars->sfxRelease.Get("Global/Release.wav");
}

void ContinuePlayer::HandleDashAnim()
{
    if (this->groundVel >= 0x40000) {
        if (this->groundVel >= 0x60000) {
            if (this->groundVel >= 0xA0000) {
                this->animator.SetAnimation(this->aniFrames, Player::ANI_DASH, false, 0);
                this->animator.speed = 256;
            }
            else {
                this->animator.SetAnimation(this->aniFrames, Player::ANI_RUN, false, 1);

                this->animator.speed = (this->groundVel >> 12) + 96;
                if (this->animator.speed > 0x100)
                    this->animator.speed = 0x100;
            }
        }
        else {
            this->animator.SetAnimation(this->aniFrames, Player::ANI_JOG, false, 0);
            int32 vel            = (this->groundVel >> 16);
            this->animator.speed = 4 * ((vel << 1) + vel) + 64;
        }
    }
    else {
        this->animator.SetAnimation(this->aniFrames, Player::ANI_WALK, false, 0);
        this->animator.speed = (this->groundVel >> 12) + 48;
    }
}

void ContinuePlayer::State_Idle()
{
    if (this->timer <= 0) {
        ContinuePlayer::HandleDashAnim();
        this->state.Set(&ContinuePlayer::State_ChargeDash);

        sVars->sfxRoll.Play(false, 255);
    }
    else {
        this->timer--;
    }
}

void ContinuePlayer::State_ChargeDash()
{
    if (this->groundVel >= 0xC0000) {
        sVars->sfxRelease.Play(false, 255);

        this->state.Set(&ContinuePlayer::State_DashRelease);
    }
    else {
        this->groundVel += 0x4000;
    }

    ContinuePlayer::HandleDashAnim();
}

void ContinuePlayer::State_DashRelease()
{
    this->position.x += this->groundVel;
}

#if RETRO_INCLUDE_EDITOR
void ContinuePlayer::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, this->isPlayer2 ? CONTPLR_ANI_IDLE_SONIC : CONTPLR_ANI_IDLE_AI, true, 0);

    ContinuePlayer::Draw();
}

void ContinuePlayer::EditorLoad(void) { sVars->aniFrames.Load("Players/Continue.bin", SCOPE_STAGE); }
#endif

void ContinuePlayer::Serialize(void) { RSDK_EDITABLE_VAR(ContinuePlayer, VAR_BOOL, isPlayer2); }

} // namespace GameLogic