// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIPicture Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "UIPicture.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIPicture);

void UIPicture::Update()
{
    this->animator.Process(); 

    this->rotation = (this->rotation + this->rotSpeed) & 0x1FF;
}
void UIPicture::LateUpdate() {}
void UIPicture::StaticUpdate() {}
void UIPicture::Draw()
{
    if (this->zonePalette)
        paletteBank[0].Copy((zonePalette >> 3) + 1, 32 * zonePalette, 224, 32);

    this->animator.DrawSprite(nullptr, false);
}

void UIPicture::Create(void *data)
{
    this->animator.SetAnimation(sVars->aniFrames, this->listID, true, this->frameID);

    if (!sceneInfo->inEditor) {
        if (Stage::CheckSceneFolder("Menu") || Stage::CheckSceneFolder("LSelect") || Stage::CheckSceneFolder("LSelect2")
            || Stage::CheckSceneFolder("LSelectEx") || Stage::CheckSceneFolder("Thanks") || Stage::CheckSceneFolder("DAGarden")|| Stage::CheckSceneFolder("Continue")) {
            this->active    = ACTIVE_NORMAL;
            this->visible   = true;
            this->drawGroup = 2;
            if (this->rotSpeed) {
                this->drawFX   = FX_ROTATE | FX_FLIP;
                this->rotation = this->startRot;
            }
        }
        else {
            if (Stage::CheckSceneFolder("Logos") || Stage::CheckSceneFolder("Summary"))
                this->active = ACTIVE_NORMAL;
            this->visible   = true;
            this->drawGroup = 2;
        }
    }
}

void UIPicture::StageLoad()
{
    if (Stage::CheckSceneFolder("Menu"))
        sVars->aniFrames.Load("UI/UIPicture.bin", SCOPE_STAGE);
    else if (Stage::CheckSceneFolder("Logos"))
        sVars->aniFrames.Load("Logos/Logos.bin", SCOPE_STAGE);
    else if (Stage::CheckSceneFolder("Thanks") || Stage::CheckSceneFolder("Continue"))
        sVars->aniFrames.Load("LSelect/Icons.bin", SCOPE_STAGE);
    else if (Stage::CheckSceneFolder("DAGarden"))
        sVars->aniFrames.Load("Cabaret/Characters.bin", SCOPE_STAGE);
}

#if RETRO_INCLUDE_EDITOR
void UIPicture::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, listID, true, frameID);
    this->animator.DrawSprite(nullptr, false);
}

void UIPicture::EditorLoad()
{
    if (Stage::CheckSceneFolder("Menu"))
        sVars->aniFrames.Load("UI/UIPicture.bin", SCOPE_STAGE);
    else if (Stage::CheckSceneFolder("Logos"))
        sVars->aniFrames.Load("Logos/Logos.bin", SCOPE_STAGE);
    else if (Stage::CheckSceneFolder("Thanks"))
        sVars->aniFrames.Load("LSelect/Icons.bin", SCOPE_STAGE);
}
#endif

void UIPicture::Serialize()
{
    RSDK_EDITABLE_VAR(UIPicture, VAR_ENUM, listID);
    RSDK_EDITABLE_VAR(UIPicture, VAR_ENUM, frameID);
    RSDK_EDITABLE_VAR(UIPicture, VAR_ENUM, tag);
    RSDK_EDITABLE_VAR(UIPicture, VAR_BOOL, zonePalette);
    RSDK_EDITABLE_VAR(UIPicture, VAR_UINT8, zoneID);
    RSDK_EDITABLE_VAR(UIPicture, VAR_INT32, rotSpeed);
    RSDK_EDITABLE_VAR(UIPicture, VAR_INT32, startRot);
}
} // namespace GameLogic