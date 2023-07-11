// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Sega Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "Sega.hpp"
#include "Twinkle.hpp"
#include "Menu/UIPicture.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Sega);

void Sega::Update()
{
    this->state.Run(this);
    this->sonicAnimator.Process();
}
void Sega::LateUpdate() {}
void Sega::StaticUpdate() {}
void Sega::Draw()
{
    Vector2 drawPos;
    drawPos = this->position;

    if (this->timer < 300) { // this is so the alpha can reset back to 0 for the "original game by" text fading in after the sega
        this->inkEffect = INK_ALPHA; // this is for the sega fading in every time sonic runs past
    }
    this->drawFX = FX_NONE; // dont want sega or text to flip, only the trail and sonic himself, so drawfx is set to none here for the sega drawing
    this->segaAnimator.DrawSprite(&drawPos, false);

    this->inkEffect = INK_ALPHA;
    drawPos.y -= TO_FIXED(40);
    if (this->timer >= 320) {
        this->textAnimator.DrawSprite(&drawPos, false);
    }

    drawPos = this->sonicPos;
    this->inkEffect = INK_NONE;
    this->drawFX = FX_FLIP; // learned sprite flipping :D
    this->trailAnimator.DrawSprite(&drawPos, false);
    this->sonicAnimator.DrawSprite(&drawPos, false);
}

void Sega::Create(void *data) 
{
    if (!sceneInfo->inEditor) {
        this->active = ACTIVE_ALWAYS;
        this->drawFX    = FX_NONE;
        this->drawGroup = 2;
        this->timer     = 0;
        this->visible   = true; // forgot this initially lmao, the object wasnt drawing and i was confused, but it was just this
        this->inkEffect = INK_NONE; // none on create
        this->alpha     = 0; // this is for the "sega" sprite, fades in as sonic runs through
        this->state.Set(&Sega::State_SonicRunIn);

        // setting animators
        this->segaAnimator.SetAnimation(sVars->aniFrames, 0, true, 1);
        this->textAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->sonicAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
        this->trailAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);

        // base positions before moving
        this->sonicPos.x = this->position.x - TO_FIXED(300);
        this->sonicPos.y = this->position.y;
    }
}

void Sega::StageLoad()
{
    sVars->aniFrames.Load("Logos/Logos.bin", SCOPE_STAGE); 
    sVars->sfxSega.Get("Stage/Sega.wav");
    sVars->sfxRing.Get("Global/Ring.wav");
    sVars->sfxRelease.Get("Global/Release.wav");
}

void Sega::State_SonicRunIn()
{
    this->timer++;
    if (this->timer >= 70) { // wait a little over 1 second
        if (this->sonicPos.x < TO_FIXED(750)) {
            this->sonicPos.x += TO_FIXED(40);
        }
        if (this->timer == 72) {
            sVars->sfxRelease.Play(false, 255); // need this called on only 1 frame
        }
        if (this->timer >= 78) {
            if (this->alpha < 128) {
                this->alpha += 8;
            }
        }
        if (this->timer == 120) {
            this->state.Set(&Sega::State_SonicRunOut); // needed this to be a different state because having it all in the same state lead to conflicts with sonics position lmao
        }
    }
}

void Sega::State_SonicRunOut()
{
    this->timer++;
    if (this->timer >= 120) {
        this->direction = FLIP_X; // learned sprite flipping :D
        if (this->sonicPos.x > -TO_FIXED(300)) {
            this->sonicPos.x -= TO_FIXED(40);
        }
        if (this->timer == 122) {
            sVars->sfxRelease.Play(false, 255); // need this called on only 1 frame
        }
        if (this->timer >= 132) {
            if (this->timer == 134)
                this->segaAnimator.frameID = 2;

            if (this->alpha < 255) {
                this->alpha += 8;
            }
        }
        if (this->timer == 130) {
            sVars->sfxSega.Play(false, 255);
        }
        if (this->timer == 300) {
            this->alpha = 0; // this sets it back to 0 on one frame, for the "original game by" text, and so the sega sprite is unaffected by it
            this->state.Set(&Sega::State_OriginalGameBy);
        }
    }
}

void Sega::State_OriginalGameBy()
{
    this->timer++;
    if (this->timer >= 300) {
        if (this->position.y <= TO_FIXED(128)) { // sega position going down so the "original game by" text can fade in
            this->position.y += TO_FIXED(1);
        }
        if (this->timer == 320) {
            sVars->sfxRing.Play(false, 255); // need this called on only 1 frame so need to make it a different if from the one afterwards
        }
        if (this->timer >= 320) {
            if (this->alpha < 255) {
                this->alpha += 8;
            }
            else if (this->alpha >= 255) { // state should only be set to presents text when the alpha is fully opaque
                this->state.Set(&Sega::State_PresentsText);
            }
        }
    }
}

void Sega::State_PresentsText()
{
     // by this point the sega itself should not be doing anything, so past this point is just getting the uipicture and playing the twinkles on it, since uipicture is just a basic general object for drawing sprites
    this->timer++;
    if (this->timer >= 600) {
        for (auto picture : GameObject::GetEntities<UIPicture>(FOR_ALL_ENTITIES)) { // gets ui pictures (there should be only one) so its position can be used
            if (this->timer == 600) { // creates stars
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, picture->position.x - TO_FIXED(64), picture->position.y + TO_FIXED(8));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 620) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, picture->position.x + TO_FIXED(64), picture->position.y - TO_FIXED(8));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 640) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, picture->position.x + TO_FIXED(16), picture->position.y + TO_FIXED(16));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer >= 700) {
                for (auto twinkle : GameObject::GetEntities<Twinkle>(FOR_ALL_ENTITIES)) { // gets all of the twinkles currently in the stage to destroy them all
                    twinkle->Destroy();
                }
            }
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void Sega::EditorDraw() {}

void Sega::EditorLoad() { sVars->aniFrames.Load("Logos/Logos.bin", SCOPE_STAGE); }
#endif

void Sega::Serialize() {}
} // namespace GameLogic