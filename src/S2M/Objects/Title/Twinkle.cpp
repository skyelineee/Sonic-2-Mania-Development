// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Twinkle Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Twinkle.hpp"
#include "Sega.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Twinkle);

void Twinkle::Update() { this->animator.Process(); }
void Twinkle::LateUpdate() {}
void Twinkle::StaticUpdate() {}
void Twinkle::Draw() 
{
    Vector2 drawPos;
    if (this->type == TwinkleStar) {
        this->animator.DrawSprite(nullptr, false); // twinkle star just draws at object position
    }
    else {
        drawPos = this->starPos; // shooting star moves across the screen based on starPos
        this->animator.DrawSprite(&drawPos, true);
        this->starPos.x -= TO_FIXED(4);
        this->starPos.y += TO_FIXED(2);
    }
}

void Twinkle::Create(void *data) 
{
    // this is a really simple object, originally planned for the Twinkles to like jump out and slowly descend from the presents text but i have no idea how to do that 
    // so im settling for something way simpler
    // this object should be created when i tell it to in sega.cpp, making the Twinkles twinkle around the presents text then get destroyed when its done
    // tho making this its own object also has the benefit of also using it on the title screen when it needs its twinkles
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_ALWAYS;
        this->visible   = true;
        if (data) { // this happens if the object is created with "INT_TO_VOID(true)" set as its data, if not (nullptr) it does the else instead, in this case data is for the shooting star
            this->drawGroup = 1;
            this->starPos.x = TO_FIXED(350);
            this->starPos.y = TO_FIXED(0);
            this->animator.SetAnimation(sVars->aniFrames, 8, false, 0);
        }
        else { // normal twinkling star
            this->drawGroup = 4;
            sVars->sfxTwinkle.Play(false, 255); // making this a part of the twinkle object itself is just more convenient lol
            if (Stage::CheckSceneFolder("Logos")) { // different bins are used for whether its title or logos, so if check here to change the animation used
                this->animator.SetAnimation(sVars->aniFrames, 4, false, 0);
            }
            else {
                this->animator.SetAnimation(sVars->aniFrames, 7, false, 0);
            }
        }
    }
}

void Twinkle::StageLoad() 
{
    if (Stage::CheckSceneFolder("Logos")) {
        sVars->aniFrames.Load("Logos/Logos.bin", SCOPE_STAGE);
    }
    else {
        sVars->aniFrames.Load("Title/Logo.bin", SCOPE_STAGE);
    }
    sVars->sfxTwinkle.Get("Stage/Twinkle.wav");
}

#if RETRO_INCLUDE_EDITOR
void Twinkle::EditorDraw() {}

void Twinkle::EditorLoad() {}
#endif

void Twinkle::Serialize() {}

} // namespace GameLogic