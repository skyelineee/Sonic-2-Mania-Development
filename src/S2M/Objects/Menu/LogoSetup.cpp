#include "Game.hpp"
#include "S2M.hpp"
#include "LogoSetup.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(LogoSetup);

void LogoSetup::Update() { state.Run(this); }
void LogoSetup::LateUpdate() {}
void LogoSetup::StaticUpdate() {}
void LogoSetup::Draw() { stateDraw.Run(this); }

void LogoSetup::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_ALWAYS;
        this->visible   = true;
        this->drawGroup = 12;
        this->state.Set(SKU->region == REGION_JP ? &LogoSetup::State_CESAScreen : &LogoSetup::State_ShowLogos);
        this->stateDraw.Set(&LogoSetup::Draw_Fade);
        this->timer = 1024;
    }
}

void LogoSetup::StageLoad()
{
    sVars->sfxSega.Get("Stage/Sega.wav");
    GameObject::Reset(0, sVars->classID, NULL);
    sVars->aniFrames.Load("Logos/Logos.bin", SCOPE_STAGE);
}

bool32 LogoSetup::ImageCallback()
{
    if (sVars->timer > 120 && (controllerInfo->keyA.press || controllerInfo->keyB.press || controllerInfo->keyStart.press || sVars->timer >= 300)) {
        return true;
    }

    sVars->timer++;
    return false;
}

void LogoSetup::State_CESAScreen()
{
    sVars->timer = 0;
    Video::LoadImage("CESA.png", 60.0, 2.0, ImageCallback);

    this->timer = 1024;
    this->state.Set(&LogoSetup::State_ShowLogos);
}

void LogoSetup::State_ShowLogos()
{
    if (this->timer <= 0) {
        if (!screenInfo->position.y)
            sVars->sfxSega.Play();

        this->timer = 0;
        this->state.Set(&LogoSetup::State_FadeToNextLogos);
        this->stateDraw.Set(nullptr);
    }
    else {
        this->timer -= 16;
    }
}

void LogoSetup::State_FadeToNextLogos()
{
    if (++this->timer > 120 || (this->timer > 30 && controllerInfo->keyStart.press)) {
        this->timer     = 0;
        this->state.Set(&LogoSetup::State_NextLogos);
        this->stateDraw.Set(&LogoSetup::Draw_Fade);
    }
}

void LogoSetup::State_NextLogos()
{
    if (this->timer >= 1024) {
        if (screenInfo->position.y >= SCREEN_YSIZE) {
            ++sceneInfo->listPos;
            Stage::SetScene("Presentation", "Logos");
        }
        else {
            screenInfo->position.y += SCREEN_YSIZE;
            this->state.Set(&LogoSetup::State_ShowLogos);
            this->stateDraw.Set(&LogoSetup::Draw_Fade);
            this->timer = 0x400;
        }
    }
    else {
        this->timer += 16;
    }
}

void LogoSetup::Draw_Fade() { Graphics::FillScreen(0, this->timer, this->timer - 0x80, this->timer - 0x100); }

#if RETRO_INCLUDE_EDITOR
void LogoSetup::EditorDraw() {}

void LogoSetup::EditorLoad() {}
#endif

void LogoSetup::Serialize() {}
} // namespace GameLogic