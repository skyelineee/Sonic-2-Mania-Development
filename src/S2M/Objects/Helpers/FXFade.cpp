// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: FXFade Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "FXFade.hpp"

#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(FXFade);

void FXFade::Update() { this->state.Run(this); }
void FXFade::LateUpdate() {}
void FXFade::StaticUpdate() {}
void FXFade::Draw() { Graphics::FillScreen(this->color, this->timer - this->delayR, this->timer - this->delayG, this->timer - this->delayB); }

void FXFade::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->visible = true;
        this->active  = ACTIVE_NORMAL;

        if (Stage::CheckSceneFolder("Menu"))
            this->drawGroup = 14;
        else if (Zone::sVars)
            this->drawGroup = this->overHUD ? Zone::sVars->hudDrawGroup : Zone::sVars->hudDrawGroup - 1;
        else
            this->drawGroup = 15;

        if (!this->speedIn)
            this->speedIn = 32;

        if (!this->speedOut)
            this->speedOut = 32;

        if (!this->color)
            this->color = VOID_TO_INT(data);

        if (!this->eventOnly) {
            this->transitionScene = false;
            this->delayR          = 0x000;
            this->delayG          = 0x080;
            this->delayB          = 0x100;
            this->state.Set(this->timer <= 0 ? &FXFade::State_FadeIn : &FXFade::State_FadeOut);
        }
    }
}

void FXFade::StageLoad() {}

void FXFade::State_FadeOut()
{
    SET_CURRENT_STATE();

    if (this->timer >= 512) {
        if (this->oneWay) {
            this->state.Set(nullptr);

            if (this->transitionScene)
                Stage::LoadScene();
        }
        else if (this->wait <= 0) {
            this->state.Set(this->fadeOutBlack ? &FXFade::State_FadeInBlack : &FXFade::State_FadeIn);
        }
        else {
            this->state.Set(&FXFade::State_Wait);
        }
    }
    else {
        this->timer += this->speedIn;
    }
}
void FXFade::State_Wait()
{
    SET_CURRENT_STATE();

    if (--this->wait <= 0) {
        this->state.Set(this->fadeOutBlack ? &FXFade::State_FadeInBlack : &FXFade::State_FadeIn);
    }
}
void FXFade::State_FadeIn()
{
    SET_CURRENT_STATE();

    if (this->timer <= 0) {
        if (this->oneWay)
            this->state.Set(nullptr);
        else
            this->Destroy();
    }
    else {
        this->timer -= this->speedOut;
    }
}
void FXFade::State_FadeInBlack()
{
    SET_CURRENT_STATE();

    int32 r = (this->color >> 16) & 0xFF;
    int32 g = (this->color >> 8) & 0xFF;
    int32 b = (this->color >> 0) & 0xFF;

    if (r)
        r -= this->speedOut;
    else if (g)
        g -= this->speedOut;
    else if (b)
        b -= this->speedOut;
    else
        this->state.Set(nullptr);

    this->color = (MAX(r, 0) << 16) | (MAX(g, 0) << 8) | (MAX(b, 0) << 0);
}

#if RETRO_INCLUDE_EDITOR
void FXFade::EditorDraw()
{
    Animator animator;
    animator.SetAnimation(sVars->aniFrames, 0, true, 11);
    animator.DrawSprite(&this->position, false);
}

void FXFade::EditorLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void FXFade::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(FXFade);

    sVars->aniFrames.Init();
}
#endif

void FXFade::Serialize() {}

} // namespace GameLogic