// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ContinueSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "ContinueSetup.hpp"
#include "ContinuePlayer.hpp"
#include "Global/SaveGame.hpp"
#include "Global/Music.hpp"
#include "Helpers/FXFade.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ContinueSetup);

void ContinueSetup::Update()
{
    this->state.Run(this);

    this->angle = (this->angle - 2) & 0x3FF;
}

void ContinueSetup::LateUpdate() {}

void ContinueSetup::StaticUpdate() {}

void ContinueSetup::Draw()
{
    RSDKTable->Prepare3DScene(sVars->sceneIndex);

    RSDKTable->MatrixTranslateXYZ(&this->matTranslate, 0, -0xF0000, 0x500000, true);
    RSDKTable->MatrixRotateX(&this->matRotateX, this->rotationX);
    RSDKTable->MatrixRotateZ(&this->matRotateY, this->angle);

    // Number 1 (tens)
    RSDKTable->MatrixTranslateXYZ(&this->matTemp, -0x120000, 0, 0, true);
    RSDKTable->MatrixMultiply(&this->matFinal, &this->matRotateY, &this->matRotateX);
    RSDKTable->MatrixMultiply(&this->matFinal, &this->matTemp, &this->matFinal);
    RSDKTable->MatrixMultiply(&this->matFinal, &this->matFinal, &this->matTranslate);
    RSDKTable->AddModelTo3DScene(sVars->countIndex[this->countTimer / 10 % 10], sVars->sceneIndex, Graphics::Scene3D::SolidColor_Shaded_Blended_Screen,
                           &this->matFinal, &this->matFinal, this->numberColor);

    // Number 2 (single digits)
    RSDKTable->MatrixTranslateXYZ(&this->matTemp, 0x120000, 0, 0, true);
    RSDKTable->MatrixMultiply(&this->matFinal, &this->matRotateY, &this->matRotateX);
    RSDKTable->MatrixMultiply(&this->matFinal, &this->matTemp, &this->matFinal);
    RSDKTable->MatrixMultiply(&this->matFinal, &this->matFinal, &this->matTranslate);
    RSDKTable->AddModelTo3DScene(sVars->countIndex[this->countTimer % 10], sVars->sceneIndex, Graphics::Scene3D::SolidColor_Shaded_Blended_Screen,
                           &this->matFinal, &this->matFinal, this->numberColor);

    RSDKTable->Draw3DScene(sVars->sceneIndex);

    Vector2 drawPos;
    drawPos.y = 0x600000;
    drawPos.x = ((screenInfo->center.x + 4) << 16) - (globals->continues << 19);
    if (this->showContinues && globals->continues > 0) {
        sVars->animator.DrawSprite(&drawPos, true);
    }

    drawPos.x += 0x140000;
    for (int32 i = 0; i < globals->continues; ++i) {
        sVars->animator.DrawSprite(&drawPos, true);
        drawPos.x += 0x140000;
    }
}

void ContinueSetup::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_NORMAL;
        this->visible       = true;
        this->drawGroup     = 1;
        this->rotationX     = 240;
        this->angle         = 256;
        this->countTimer    = 10;
        this->numberColor   = 0xFF00FF;
        this->showContinues = true;
        this->state.Set(&ContinueSetup::State_FadeIn);
        this->updateRange.x = 0x4000000;

        this->updateRange.y = 0x4000000;
        switch (GET_CHARACTER_ID(1)) {
            default:
            case ID_SONIC: sVars->animator.SetAnimation(ContinuePlayer::sVars->aniFrames, ContinuePlayer::CONTPLR_ANI_ICON, true, 0); break;
            case ID_TAILS: sVars->animator.SetAnimation(ContinuePlayer::sVars->aniFrames, ContinuePlayer::CONTPLR_ANI_ICON, true, 1); break;
            case ID_KNUCKLES: sVars->animator.SetAnimation(ContinuePlayer ::sVars->aniFrames, ContinuePlayer::CONTPLR_ANI_ICON, true, 2); break;
        }
        RSDKTable->SetActivePalette(0, 0, screenInfo->size.y);
    }
}

void ContinueSetup::StageLoad()
{
    const char *paths[10] = {
        "Continue/Count0.bin", "Continue/Count1.bin", "Continue/Count2.bin", "Continue/Count3.bin", "Continue/Count4.bin",
        "Continue/Count5.bin", "Continue/Count6.bin", "Continue/Count7.bin", "Continue/Count8.bin", "Continue/Count9.bin",
    };

    for (int32 i = 0; i < 10; ++i) sVars->countIndex[i] = RSDKTable->LoadMesh(paths[i], SCOPE_STAGE);

    sVars->sceneIndex = RSDKTable->Create3DScene("View:Continue", 4096, SCOPE_STAGE);

    RSDKTable->SetDiffuseColor(sVars->sceneIndex, 0xA0, 0xA0, 0xA0);
    RSDKTable->SetDiffuseIntensity(sVars->sceneIndex, 8, 8, 8);
    RSDKTable->SetSpecularIntensity(sVars->sceneIndex, 15, 15, 15);

    sVars->sfxAccept.Get("Global/MenuAccept.wav");
}

void ContinueSetup::State_FadeIn()
{
    if (++this->timer >= 8 && !RSDKTable->GetEntityCount(FXFade::sVars->classID, true)) {
        this->timer = 0;
        this->state.Set(&ContinueSetup::State_HandleCountdown);
    }
}

void ContinueSetup::State_HandleCountdown()
{
    if (++this->secondTimer == 60) {
        this->secondTimer = 0;

        if (this->countTimer > 0) {
            this->countTimer--;

            if (this->alpha < 0xFF)
                this->alpha += 0x18;

            this->numberColor = paletteBank[2].GetEntry(this->alpha);
        }
    }

    if (controllerInfo->keyA.press || controllerInfo->keyStart.press || touchInfo->count) {
        for (auto player : GameObject::GetEntities<ContinuePlayer>(FOR_ACTIVE_ENTITIES))
        {
            if (!player->isPlayer2)
                player->animator.SetAnimation(ContinuePlayer::sVars->aniFrames, player->animator.animationID + 1, true, 0);

            player->state.Set(&ContinuePlayer::State_Idle);
        }

        this->state.Set(&ContinueSetup::State_ContinueGame);
        sVars->sfxAccept.Play(false, 255);
    }

    if (!this->countTimer && ++this->timer == 60) {
        this->timer = 0;
        this->state.Set(&ContinueSetup::State_ReturnToMenu);

        FXFade *fade       = GameObject::Create<FXFade>(nullptr, this->position.x, this->position.y);
        fade->active       = ACTIVE_ALWAYS;
        fade->state.Set(&FXFade::State_FadeOut);
        fade->speedIn      = 12;
        fade->wait         = 240;
    }
}

void ContinueSetup::State_ContinueGame()
{
    if (++this->timer == 90) {
        Music::FadeOut(0.0125);
    }

    if (this->timer == 180) {
        FXFade *fade       = GameObject::Create<FXFade>(nullptr, this->position.x, this->position.y);
        fade->active = ACTIVE_ALWAYS;
        fade->state.Set(&FXFade::State_FadeOut);
        fade->speedIn      = 12;
        fade->wait         = 240;
    }

    if (this->timer == 260) {
        Stage::SetScene("Mania Mode", "");

        SaveGame::GetSaveRAM()->continues = globals->continues;
        sceneInfo->listPos               = SaveGame::GetSaveRAM()->storedStageID;
        Stage::LoadScene();
    }

    if (this->timer < 58) {
        this->showContinues = ((this->timer >> 1) & 1);
    }

    if (this->timer == 60) {
        this->showContinues = true;

        if (globals->continues > 0)
            globals->continues--;
    }
}

void ContinueSetup::State_ReturnToMenu()
{
    if (++this->timer == 80) {
        Stage::SetScene("Presentation", "Menu");
        Stage::LoadScene();
    }
}

#if RETRO_INCLUDE_EDITOR
void ContinueSetup::EditorDraw() {}

void ContinueSetup::EditorLoad() {}
#endif

void ContinueSetup::Serialize() {}

} // namespace GameLogic