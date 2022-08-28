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
void Sega::Draw() { this->animator.DrawSprite(NULL, false); }

void Sega::Create(void *data) 
{
    this->animator.SetAnimation(sVars->aniFrames, listID, true, frameID); 

        if (Stage::CheckSceneFolder("Logos")) {
        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawGroup = 2;
    }
}

void Sega::StageLoad()
{
    sVars->aniFrames.Load("Logos/Logos.bin", SCOPE_STAGE); 
}

void Sega::State_Setup()
{
    this->alpha     = 0;
    this->inkEffect = INK_ALPHA;
    this->timer     = 320;
    Graphics::FillScreen(0, 0, 0, 0);
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
    RSDK_EDITABLE_VAR(Sega, VAR_ENUM, listID);
}
} // namespace GameLogic