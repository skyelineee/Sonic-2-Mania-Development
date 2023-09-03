// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SeeSaw Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SeeSaw.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SeeSaw);

void SeeSaw::Update()
{
    if (this->tiltTimerL || this->tiltTimerR) {
        if (this->tiltTimerL == this->tiltTimerR) {
            this->targetTilt = SEESAW_TILT_M;
        }
        else {
            if (this->tiltTimerL > this->tiltTimerR)
                this->targetTilt = SEESAW_TILT_L;
            else
                this->targetTilt = SEESAW_TILT_R;
        }
    }
    else if (this->tiltTimerM != (this->tiltTimerR || this->tiltTimerL)) {
        this->targetTilt = SEESAW_TILT_M;
    }

    if (this->tilt != this->targetTilt) {
        this->tilt = this->tilt <= this->targetTilt ? this->tilt + 1 : this->tilt - 1;

        if (this->state.Matches(&SeeSaw::State_None)) {
            switch (this->targetTilt) {
                case SEESAW_TILT_L:
                    if (this->launchVelocity < 0xA0000) {
                        this->velocity.x = -0xCC00;
                        this->velocity.y = -0xAF000;
                    }
                    else {
                        this->velocity.x = -0xA000;
                        this->velocity.y = -0xE0000;
                    }
                    break;

                case SEESAW_TILT_M:
                    this->velocity.x = -0x11400;
                    this->velocity.y = -0x81800;
                    break;

                case SEESAW_TILT_R:
                    if (this->launchVelocity < 0x9C000) {
                        this->velocity.x = -0xF400;
                        this->velocity.y = -0x96000;
                    }
                    else {
                        this->velocity.x = -0x8000;
                        this->velocity.y = -0xA2000;
                    }
                    break;
            }

            if (this->orbPos.x < this->position.x)
                this->velocity.x = -this->velocity.x;

            this->state.Set(&SeeSaw::State_OrbLaunched);
            this->active = ACTIVE_NORMAL;
        }
    }

    if (this->orbPos.x < this->position.x) {
        switch (this->tilt) {
            case SEESAW_TILT_L:
                sVars->orbTargetPos.x = -0x280000;
                sVars->orbTargetPos.y = -0x20000;
                break;

            case SEESAW_TILT_M:
                sVars->orbTargetPos.x = -0x280000;
                sVars->orbTargetPos.y = -0x160000;
                break;

            case SEESAW_TILT_R:
                sVars->orbTargetPos.x = -0x280000;
                sVars->orbTargetPos.y = -0x290000;
                break;
        }
    }
    else {
        switch (this->tilt) {
            case SEESAW_TILT_L:
                sVars->orbTargetPos.x = 0x280000;
                sVars->orbTargetPos.y = -0x290000;
                break;

            case SEESAW_TILT_M:
                sVars->orbTargetPos.x = 0x280000;
                sVars->orbTargetPos.y = -0x160000;
                break;

            case SEESAW_TILT_R:
                sVars->orbTargetPos.x = 0x280000;
                sVars->orbTargetPos.y = -0x20000;
                break;
        }
    }

    this->state.Run(this);

    this->orbAnimator.Process();

    this->tiltTimerL = 0;
    this->tiltTimerM = 0;
    this->tiltTimerR = 0;
    this->stood      = false;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        SeeSaw::SetupHitbox(player->position.x, this->prevTilt);

        if (sVars->hitboxPlank.right) {
            if (player->velocity.y > this->launchVelocity)
                sVars->launchVelocity = player->velocity.y + 0x7000;
            else
                sVars->launchVelocity = this->launchVelocity;

            if (player->CheckCollisionPlatform(this, &sVars->hitboxPlank)) {
                this->stood = true;

                if (this->tilt != this->prevTilt) {
                    int32 top = sVars->hitboxPlank.top;
                    SeeSaw::SetupHitbox(player->position.x, this->tilt);
                    player->position.y += (sVars->hitboxPlank.top - top) << 16;
                }

                player->position.y += 0x20000;

                if (this->orbSide) {
                    sVars->launchVelocity = this->velocity.y;
                    if (abs(player->position.x - this->position.x) >= 0x80000) {
                        if (player->position.x >= this->position.x ? this->orbSide == 2 : this->orbSide == 1)
                            sVars->launchVelocity = 0;
                    }

                    if (sVars->launchVelocity) {
                        player->state.Set(&Player::State_Air);
                        player->onGround = false;

                        if (this->state.Matches(&SeeSaw::State_NoOrb) || this->orbTimer)
                            player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP,true, 0);
                        else
                            player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);

                        sVars->sfxSpring.Play(false, 255);
                        player->velocity.y = -sVars->launchVelocity;
                    }
                }
                else {
                    if (this->state.Matches(&SeeSaw::State_OrbIdle))
                        this->state.Set(&SeeSaw::State_None);
                    else if (this->state.Matches(&SeeSaw::State_OrbLaunched))
                        sVars->launchVelocity = this->launchVelocity;

                    if (abs(player->position.x - this->position.x) >= 0x80000) {
                        if (player->position.x >= this->position.x) {
                            ++this->tiltTimerR;
                            if (this->targetTilt != SEESAW_TILT_R)
                                this->launchVelocity = sVars->launchVelocity;
                        }
                        else {
                            ++this->tiltTimerL;
                            if (this->targetTilt != SEESAW_TILT_L)
                                this->launchVelocity = sVars->launchVelocity;
                        }
                    }
                    else {
                        ++this->tiltTimerM;
                    }
                }
            }
        }
    }

    this->prevTilt = this->tilt;
    this->orbSide  = 0;

    switch (this->prevTilt) {
        case SEESAW_TILT_L: this->rotation = 480; break;
        case SEESAW_TILT_M: this->rotation = 0; break;
        case SEESAW_TILT_R: this->rotation = 32; break;
    }

    if (!this->state.Matches(&SeeSaw::State_NoOrb)) {
        int32 storeX   = this->position.x;
        int32 storeY   = this->position.y;
        this->position = this->orbPos;

        for (auto playerPtr : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
        {
            if (playerPtr->CheckCollisionTouch(this, &sVars->hitboxOrbinaut)) {
                playerPtr->Hurt(this);
            }
        }

        this->position.x = storeX;
        this->position.y = storeY;
    }
}

void SeeSaw::LateUpdate() {}

void SeeSaw::StaticUpdate() {}

void SeeSaw::Draw()
{
    this->orbAnimator.DrawSprite(&this->orbPos, false);
    this->plankAnimator.DrawSprite(nullptr, false);
    this->pivotAnimator.DrawSprite(nullptr, false);
}

void SeeSaw::Create(void *data)
{
    this->drawFX = FX_FLIP;
    if (!sceneInfo->inEditor) {
        this->pivotAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->plankAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->orbAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);

        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->drawFX        = FX_ROTATE | FX_FLIP;
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[1];

        if (this->side == FLIP_X) {
            this->targetTilt = SEESAW_TILT_R;
            this->orbPos.x   = this->position.x;
        }

        this->state.Set(&SeeSaw::State_OrbIdle);
    }
}

void SeeSaw::StageLoad()
{
    sVars->aniFrames.Load("HTZ/SeeSaw.bin", SCOPE_STAGE);

    sVars->hitboxOrbinaut.left   = -8;
    sVars->hitboxOrbinaut.top    = -8;
    sVars->hitboxOrbinaut.right  = 8;
    sVars->hitboxOrbinaut.bottom = 8;

    sVars->sfxSpring.Get("Global/Spring.wav");

    DebugMode::AddObject(sVars->classID, &SeeSaw::DebugSpawn, &SeeSaw::DebugDraw);
}

void SeeSaw::DebugSpawn() { GameObject::Create<SeeSaw>(nullptr, this->position.x, this->position.y); }

void SeeSaw::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 1, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void SeeSaw::SetupHitbox(int32 playerX, int32 tilt)
{
    if (tilt == SEESAW_TILT_M) {
        sVars->hitboxPlank.top   = -8;
        sVars->hitboxPlank.right = 48;
    }
    else {
        int32 distance = CLAMP((((playerX - this->position.x) >> 16) + 48) >> 1, 0, 47);

        if (tilt != SEESAW_TILT_L)
            sVars->hitboxPlank.top = sVars->tiltHeightTable[distance] + 12;
        else
            sVars->hitboxPlank.top = sVars->tiltHeightTable[47 - distance] + 12;
        sVars->hitboxPlank.right = 40;
    }

    sVars->hitboxPlank.bottom = sVars->hitboxPlank.top + 24;
    sVars->hitboxPlank.left   = -sVars->hitboxPlank.right;
}

void SeeSaw::State_OrbIdle()
{
    this->orbPos.x       = this->position.x + sVars->orbTargetPos.x;
    this->orbPos.y       = this->position.y + sVars->orbTargetPos.y;
    this->launchVelocity = 0;
}

void SeeSaw::State_None()
{
    // do absolutely nothing. not a thing.
}

void SeeSaw::State_NoOrb()
{
    // gone :(
    // dont be sad :]
}

void SeeSaw::State_OrbLaunched()
{
    this->orbPos.x += this->velocity.x;
    this->orbPos.y += this->velocity.y;
    this->velocity.y += 0x3800;

    if (this->velocity.y > 0) {
        sVars->orbTargetPos.y += this->position.y;

        if (this->orbPos.y >= sVars->orbTargetPos.y) {
            this->orbPos.y = sVars->orbTargetPos.y;
            this->state.Set(&SeeSaw::State_OrbIdle);

            if (this->orbTimer && this->velocity.x)
                this->orbTimer = 0;

            if (this->orbPos.x >= this->position.x) {
                this->targetTilt = SEESAW_TILT_R;
                this->orbSide    = 2;
            }
            else {
                this->targetTilt = SEESAW_TILT_L;
                this->orbSide    = 1;
            }

            this->active = ACTIVE_BOUNDS;
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void SeeSaw::EditorDraw()
{
    this->orbPos.x = this->position.x;
    this->orbPos.y = this->position.y;

    this->pivotAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->plankAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    this->orbAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);

    SeeSaw::Draw();
}

void SeeSaw::EditorLoad()
{
    sVars->aniFrames.Load("HTZ/SeeSaw.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, side);
    RSDK_ENUM_VAR("Left", FLIP_NONE);
    RSDK_ENUM_VAR("Right", FLIP_X);
}
#endif

#if RETRO_REV0U
void SeeSaw::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(SeeSaw);

    int32 tiltHeightTable[] = { -36, -36, -38, -40, -42, -44, -42, -40, -38, -36, -35, -34, -33, -32, -31, -30, -29, -28, -27, -26, -25, -24, -23, -22,
                                -21, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,  -2,  -2,  -2,  -2 };
    memcpy(sVars->tiltHeightTable, tiltHeightTable, sizeof(tiltHeightTable));
}
#endif

void SeeSaw::Serialize() { RSDK_EDITABLE_VAR(SeeSaw, VAR_UINT8, side); }

} // namespace GameLogic