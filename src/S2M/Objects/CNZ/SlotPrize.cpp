// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SlotPrize Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SlotPrize.hpp"
#include "Global/Player.hpp"
#include "Global/Ring.hpp"
#include "Global/Music.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SlotPrize);

void SlotPrize::Update()
{
    this->animator.Process();

    int32 prizePos = this->position.x;
    prizePos -= this->originPos.x;
    prizePos >>= 4;
    this->position.x -= prizePos;

    prizePos = this->position.y;
    prizePos -= this->originPos.y;
    prizePos >>= 4;
    this->position.y -= prizePos;

    if (this->alpha < 0x100) {
        this->alpha += 0x20;
    }

    this->timer--;
    if (this->timer == 0) {
        for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (currentPlayer->CheckCollisionTouch(this, &sVars->hitbox)) {
                if (this->winner) {
                    currentPlayer->rings++;
                    if (currentPlayer->rings > 999) {
                        currentPlayer->rings = 999;
                    }

                    if (currentPlayer->rings
                        >= currentPlayer->ringExtraLife) { // checks if the ring amount the player has is enough to get a new life
                        currentPlayer->GiveLife();

                        currentPlayer->ringExtraLife += 100; // increases ring extra life amount for the next 1up
                        if (currentPlayer->ringExtraLife > 300) {
                            currentPlayer->ringExtraLife = 1000; // never reaches 1000, gets set to a number the player can never reach
                        }
                    }

                    if (Ring::sVars->pan == 0) {
                        int32 channel = Ring::sVars->sfxRing.Play();
                        channels[channel].SetAttributes(1.0, 1.0, 1.0);
                        Ring::sVars->pan = 1;
                    }
                    else {
                        int32 channel = Ring::sVars->sfxRing.Play();
                        channels[channel].SetAttributes(1.0, -1.0, 1.0);
                        Ring::sVars->pan = 0;
                    }

                    int32 max = 0x80000;
                    int32 min = -max;
                    for (int32 i = 0; i < 4; ++i) {
                        int32 x       = this->position.x + Math::Rand(min, max);
                        int32 y       = this->position.y + Math::Rand(min, max);
                        Ring *sparkle = GameObject::Create<Ring>(nullptr, x, y);

                        sparkle->state.Set(&Ring::State_Sparkle);
                        sparkle->stateDraw.Set(&Ring::Draw_Sparkle);
                        sparkle->active  = ACTIVE_NORMAL;
                        sparkle->visible = false;
                        if (this->drawGroup == 1)
                            sparkle->drawGroup = 1;
                        else
                            sparkle->drawGroup = Zone::sVars->objectDrawGroup[1];

                        sparkle->animator.SetAnimation(Ring::sVars->aniFrames, Ring::Sparkle1 + (i % 3), true, 0);
                        int32 frameCount = sparkle->animator.frameCount;
                        if (sparkle->animator.animationID == 2) {
                            sparkle->alpha = 0xE0;
                            frameCount >>= 1;
                        }
                        sparkle->maxFrameCount  = frameCount - 1;
                        sparkle->animator.speed = Math::Rand(6, 8);
                        sparkle->timer          = 2 * i++;
                    }
                }
                else {
                    if (currentPlayer->rings > 0) {
                        currentPlayer->rings--;
                    }

                    if (sVars->spikeSFXTimer == 0) {
                        sVars->sfxSpike.Play(false, 255);
                    }

                    sVars->spikeSFXTimer++;
                    sVars->spikeSFXTimer %= 3;
                }
                this->Destroy();
            }
        }
    }
}

void SlotPrize::LateUpdate() {}
void SlotPrize::StaticUpdate() {}
void SlotPrize::Draw()
{
    this->animator.SetAnimation(sVars->aniFrames, this->listID, false, 0);
    this->animator.DrawSprite(nullptr, false);
}

void SlotPrize::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        if (data) {
            this->visible   = true;
            this->active    = ACTIVE_NORMAL;
            this->drawGroup = 6;
        }
    }
}

void SlotPrize::StageLoad()
{
    sVars->aniFrames.Load("CNZ/SlotPrize.bin", SCOPE_STAGE);
    sVars->sfxSpike.Get("Global/Spike.wav");

    sVars->hitbox.left   = -16;
    sVars->hitbox.top    = -16;
    sVars->hitbox.right  = 16;
    sVars->hitbox.bottom = 16;
}

#if RETRO_INCLUDE_EDITOR
void SlotPrize::EditorDraw() {}

void SlotPrize::EditorLoad() {}
#endif

void SlotPrize::Serialize() {}

} // namespace GameLogic