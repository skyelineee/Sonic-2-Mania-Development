// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: LogoSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "LogoSetup.hpp"
#include "Menu/UIPicture.hpp"
#include "Helpers/RPCHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(LogoSetup);

void LogoSetup::Update()
{
    this->state.Run(this);

    screenInfo->position.x = 0x100 - screenInfo->center.x;
}

void LogoSetup::LateUpdate() {}

void LogoSetup::StaticUpdate() {}

void LogoSetup::Draw()
{
    this->stateDraw.Run(this);
}

void LogoSetup::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_ALWAYS;
        this->visible   = true;
        this->drawGroup = 12;
        this->state.Set(&LogoSetup::State_ShowLogos);
        this->stateDraw.Set(&LogoSetup::Draw_Fade);
        this->timer     = 1024;
    }
}

void LogoSetup::StageLoad()
{
    RSDKTable->ResetEntitySlot(0, sVars->classID, nullptr);
    UIPicture::sVars->aniFrames.Load("Logos/Logos.bin", SCOPE_STAGE);

    // What... is this for...?
    // LogoSetup is never in any other stages besides category 0, scene 0 (Logos) so this should never be used...
    // However it was even updated for plus which means it has to have been some sort of dev/debug feature

    if (sceneInfo->listPos >= 3) {
        switch (sceneInfo->listPos) {
            case 3: globals->playerID = ID_SONIC; break;
            case 4: globals->playerID = ID_TAILS; break;
            case 5: globals->playerID = ID_KNUCKLES; break;
            default: break;
        }
    }

    SetPresence("", "Watching Logos", "doggy", "doggy", "", "");
}

bool32 LogoSetup::ImageCallback()
{
    if (sVars->timer > 120
        && (controllerInfo->keyA.press || controllerInfo->keyB.press || controllerInfo->keyStart.press || sVars->timer >= 300)) {
        return true;
    }

    sVars->timer++;
    return false;
}

void LogoSetup::State_ShowLogos()
{
    if (this->timer <= -4000) {
        this->timer     = 0;
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
            Stage::LoadScene();
        }
        else {
            screenInfo->position.y += SCREEN_YSIZE;
            this->state.Set(&LogoSetup::State_ShowNextLogos); // needed a new state for the different timers
            this->stateDraw.Set(&LogoSetup::Draw_Fade);
            this->timer     = 0x400;
        }
    }
    else {
        this->timer += 16;
    }
}

void LogoSetup::State_ShowNextLogos()
{
    if (this->timer <= -1000) {
        this->timer = 0;
        this->state.Set(&LogoSetup::State_FadeToNextLogos);
        this->stateDraw.Set(nullptr);
    }
    else {
        this->timer -= 16;
    }
}

void LogoSetup::Draw_Fade()
{
    Graphics::FillScreen(0x000000, this->timer, this->timer - 128, this->timer - 256);
}

#if RETRO_INCLUDE_EDITOR
void LogoSetup::EditorDraw() {}

void LogoSetup::EditorLoad() {}
#endif

void LogoSetup::Serialize() {}
} // namespace GameLogic