// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ThanksSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "ThanksSetup.hpp"
#include "UIPicture.hpp"
#include "Global/Music.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ThanksSetup);

void ThanksSetup::Update()
{
    this->state.Run(this);

    screenInfo->position.x = 0x100 - screenInfo->center.x;
}

void ThanksSetup::LateUpdate() {}

void ThanksSetup::StaticUpdate() {}

void ThanksSetup::Draw() { this->stateDraw.Run(this); }

void ThanksSetup::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_ALWAYS;
        this->visible   = true;
        this->drawGroup = 12;
        this->drawFX    = FX_FLIP;
        this->state.Set(&ThanksSetup::State_ThanksForPlaying);
        this->stateDraw.Set(&ThanksSetup::Draw_Fade);

        this->timer   = 0x300;
    }
}

void ThanksSetup::StageLoad()
{
    GameObject::Reset(0, sVars->classID, 0);
}

void ThanksSetup::State_ThanksForPlaying()
{
    if (this->timer <= 0) {
        this->timer = 0;
        this->stateDraw.Set(nullptr);
    }
    else {
        this->timer -= 16;
    }

    bool32 confirmPress = API::GetConfirmButtonFlip() ? controllerInfo->keyB.press : controllerInfo->keyA.press;
    if (confirmPress || controllerInfo->keyStart.press) {
        this->state.Set(&ThanksSetup::State_FadeOut);
        this->stateDraw.Set(&ThanksSetup::Draw_Fade);
    }
}

void ThanksSetup::State_FadeOut()
{
    if (this->timer >= 1024) {
        if (Stage::CheckSceneFolder("Thanks")) {
            Stage::SetScene("Presentation", "Menu");
            Stage::LoadScene();
        }
        else if (Stage::CheckSceneFolder("Credits")) {
            Stage::SetScene("Presentation", "Signatures");
            Stage::LoadScene();
        }
    }
    else {
        Music::FadeOut(0.5);
        this->timer += 16;
    }
}

void ThanksSetup::Draw_Fade()
{
    Graphics::FillScreen(0x000000, this->timer, this->timer - 128, this->timer - 256);
}

#if RETRO_INCLUDE_EDITOR
void ThanksSetup::EditorDraw() {}

void ThanksSetup::EditorLoad() {}
#endif

void ThanksSetup::Serialize() {}

} // namespace GameLogic