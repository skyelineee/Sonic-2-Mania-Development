// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: PauseMenu Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "PauseMenu.hpp"
#include "Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(PauseMenu);

void PauseMenu::Update()
{
    if (sVars->disablePause)
        return;

    if (this->isPaused) {
        if (controllerInfo->keyStart.press || unknownInfo->pausePress) {
            if (sceneInfo->state == ENGINESTATE_FROZEN) {
                Stage::SetEngineState(ENGINESTATE_REGULAR);
                ResumeSound();
                this->Destroy();
            }
        }

        if (sceneInfo->debugMode) {
            if (controllerInfo->keyA.press) {
                globals->recallEntities = false;
                globals->restartFlags   = 0;

                Stage::SetScene("Presentation & Menus", "Level Select");
                Zone::StartFadeOut(10, 0x000000);
            }

            if (controllerInfo->keyB.down || controllerInfo->keyC.press) {
                this->isPaused = false;
            }
        }
    }
    else {
        this->isPaused = true;
        Stage::SetEngineState(ENGINESTATE_FROZEN);
        PauseSound();
    }
}
void PauseMenu::LateUpdate() {}
void PauseMenu::StaticUpdate() {}
void PauseMenu::Draw() {}

void PauseMenu::Create(void *data)
{
    this->active        = ACTIVE_ALWAYS;
    this->updateRange.x = 0x800000;
    this->updateRange.x = 0x800000;
    this->visible       = true;
    this->drawGroup     = 1;

    if (!sceneInfo->inEditor)
        this->state.Set(&PauseMenu::State_Init);
}

void PauseMenu::StageLoad() {}

void PauseMenu::State_Init()
{
    SET_CURRENT_STATE();

    this->state.Set(&PauseMenu::State_Idle);
}

void PauseMenu::State_Idle() { SET_CURRENT_STATE(); }

void PauseMenu::PauseSound()
{
    for (int32 i = 0; i < CHANNEL_COUNT; ++i) {
        if (channels[i].IsActive()) {
            channels[i].Pause();
            sVars->activeChannels[i] = true;
        }
    }
}

void PauseMenu::ResumeSound()
{
    for (int32 i = 0; i < CHANNEL_COUNT; ++i) {
        if (sVars->activeChannels[i]) {
            channels[i].Resume();
            sVars->activeChannels[i] = false;
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void PauseMenu::EditorDraw() {}

void PauseMenu::EditorLoad() {}
#endif

#if RETRO_REV0U
void PauseMenu::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(PauseMenu);

    sVars->sfxBleep.Init();
    sVars->sfxAccept.Init();
}
#endif

void PauseMenu::Serialize() {}

} // namespace GameLogic