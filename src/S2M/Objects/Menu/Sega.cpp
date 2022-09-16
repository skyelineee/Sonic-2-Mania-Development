// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Sega Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "Sega.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Sega);

void Sega::Update() { this->animator.Process(); }
void Sega::LateUpdate() {}
void Sega::StaticUpdate() {}
void Sega::Draw() { 
    this->animator.DrawSprite(NULL, false);
    Graphics::DrawRect(0, 0, screenInfo->size.x, screenInfo->size.y, 0xFF, 0xFF, INK_NONE, true);

    switch (this->activeCategory) {
        default: break;
        case 0:
            variable1 = this->trailPos.x;
            variable1 += screenInfo->center.x;
            //i genuinely have no idea rn how to deal with the drawspritescreen stuff so this is prob all wrong
            this->animator.SetAnimation(sVars->aniFrames, 2, false, 0);
            this->animator.DrawSprite(&this->segaPosition, true);
            this->direction &= FLIP_NONE;
            break;
    }
    

}

void Sega::Create(void *data) 
{
    this->animator.SetAnimation(sVars->aniFrames, listID, true, frameID); 

        if (Stage::CheckSceneFolder("Logos")) {
        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawGroup = 2;
        this->drawFX |= FX_FLIP;
    }
}

void Sega::StageLoad()
{
    sVars->aniFrames.Load("Logos/Logos.bin", SCOPE_STAGE); 
    sVars->sfxSega.Get("Stage/Sega.wav");
}

void Sega::State_Setup()
{
    this->alpha     = 0;
    this->inkEffect = INK_ALPHA;
    this->timer     = 320;
    this->state.Set(&Sega::State_Setup);

}

void Sega::State_FadeIn()
{
    if (this->timer > 0) {
        this->timer -= 8;
    }
    else {
        this->direction = 1;
        this->alpha = 0x100;
        this->trailPos.x = 256;
        this->state.Set(&Sega::State_FadeIn);
    }
}

void Sega::State_SonicRun_Left()
{
    this->listID  = 2;

    if (this->trailPos.x > -320) {
        this->trailPos.x -= 32;
    }
    else {
        this->direction = FLIP_NONE;
        this->state.Set(&Sega::State_SonicRun_Left);
    }
}

void Sega::State_SonicRun_Right()
{
    this->listID = 2;

    if (this->trailPos.x < 320) {
        this->trailPos.x += 32;
    }
    else {
        this->alpha = 0x100;
        this->state.Set(&Sega::State_SonicRun_Right);
    }
}

void Sega::State_FadeOutTrail_Left()
{ 
    if (this->alpha > -0x40) {
        this->alpha -= 0x10;
    }
    else {
        this->state.Set(&Sega::State_FadeOutTrail_Left);
    }
}

void Sega::State_FadeOutTrail_Right()
{ 
    if (this->alpha > -0x40) {
        this->alpha -= 0x10;
    }
    else {
        this->state.Set(&Sega::State_FadeOutTrail_Right);
    }
}

void Sega::State_ShowSegaLogo()
{
    if (this->timer == 160) {
        this->timer = 0;
        this->state.Set(&Sega::State_ShowSegaLogo);
    }

    if (this->timer == 2) {
        sVars->sfxSega.Play();
    }

}

#if RETRO_INCLUDE_EDITOR
void Sega::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, listID, true, frameID);
    this->animator.DrawSprite(NULL, false);
}

void Sega::EditorLoad() { sVars->aniFrames.Load("Logos/Logos.bin", SCOPE_STAGE); }
#endif

void Sega::Serialize()
{
    RSDK_EDITABLE_VAR(Sega, VAR_ENUM, listID);
    RSDK_EDITABLE_VAR(Sega, VAR_ENUM, frameID);
}
} // namespace GameLogic