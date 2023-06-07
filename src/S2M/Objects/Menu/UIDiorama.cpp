// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIDiorama Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIDiorama.hpp"
#include "UIWidgets.hpp"
#include "Global/Player.hpp"
#include "Global/Localization.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIDiorama);

void UIDiorama::Update()
{
    bool32 changeDiorama = this->lastDioramaID != this->dioramaID;

    UIControl *parent = this->parent;
    if (parent) {
        if (parent->active != ACTIVE_ALWAYS)
            this->timer = 2;

        if (this->parentActivity != parent->active && parent->active == ACTIVE_ALWAYS)
            changeDiorama = true;

        this->parentActivity = parent->active;
    }

    if (changeDiorama) {
        this->timer = 12;
        UIDiorama::ChangeDiorama(this->dioramaID);
    }

    if (this->timer > 0) {
        this->timer--;
        this->staticAnimator.Process();
    }
}

void UIDiorama::LateUpdate() {}

void UIDiorama::StaticUpdate()
{
    for (auto diorama : GameObject::GetEntities<UIDiorama>(FOR_ALL_ENTITIES)) {
        Graphics::AddDrawListRef(diorama->drawGroup, diorama->Slot());
    }
}

void UIDiorama::Draw()
{
    if (this->timer <= 0) {
        if (sceneInfo->currentDrawGroup == this->drawGroup) {
            this->dioramaPos.x  = this->position.x;
            this->dioramaPos.y  = this->position.y - 0x510000;
            this->dioramaSize.x = 0x1260000;
            this->dioramaSize.y = 0xA20000;
            // Draw Lime BG Rect to clear the screen, ONLY draw over the already existing lime pixels
            // Extra Note: as far as I can tell this doesn't *actually* do anything, the sprite already has a lime area setup
            Graphics::DrawRect(this->dioramaPos.x, this->dioramaPos.y, this->dioramaSize.x, this->dioramaSize.y, 0x00FF00, 255, INK_MASKED,
                               false);
        }

        this->inkEffect = INK_MASKED;
        this->inkEffect = INK_NONE;

        if (sceneInfo->currentDrawGroup == this->drawGroup) {
            this->inkEffect = INK_NONE;
            this->dioramaAnimator.DrawSprite(nullptr, false);
        }
    }
    else {
        this->staticAnimator.DrawSprite(nullptr, false);
    }
}

void UIDiorama::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->lastDioramaID = -1;

        this->staticAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 1, true, 0);

        this->active    = ACTIVE_BOUNDS;
        this->visible   = true;
        this->drawGroup = 1;
    }
}

void UIDiorama::StageLoad()
{
    UIDiorama::sVars->active = ACTIVE_ALWAYS;

    Palette::SetPaletteMask(0x00FF00);

    UIDiorama::sVars->aniFrames.Load("UI/BGIcons.bin", SCOPE_STAGE);

}

void UIDiorama::ChangeDiorama(uint8 dioramaID)
{
    this->lastDioramaID = dioramaID;

    this->needsSetup = true;

    if (this->needsSetup) {
        this->needsSetup = false;
    }

    switch (dioramaID) {
        case UIDIORAMA_ADVENTURE:
            this->dioramaAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
            break;

        case UIDIORAMA_TIMEATTACK:
            this->dioramaAnimator.SetAnimation(sVars->aniFrames, 0, true, 1);
            break;

        case UIDIORAMA_EXTRAS:
            this->dioramaAnimator.SetAnimation(sVars->aniFrames, 0, true, 2);
            break;

        case UIDIORAMA_OPTIONS:
            this->dioramaAnimator.SetAnimation(sVars->aniFrames, 0, true, 3);
            break;

        case UIDIORAMA_EXIT:
            this->dioramaAnimator.SetAnimation(sVars->aniFrames, 0, true, 4);
            break;

        case UIDIORAMA_BOSSRUSH:
            this->dioramaAnimator.SetAnimation(sVars->aniFrames, 0, true, 5);
            break;

        case UIDIORAMA_MUSICPLAYER:
            this->dioramaAnimator.SetAnimation(sVars->aniFrames, 0, true, 6);
            break;

        case UIDIORAMA_LEVELSELECT:
            this->dioramaAnimator.SetAnimation(sVars->aniFrames, 0, true, 7);
            break;

        case UIDIORAMA_EXTRALEVELS:
            this->dioramaAnimator.SetAnimation(sVars->aniFrames, 0, true, 8);
            break;

        case UIDIORAMA_CREDITS:
            this->dioramaAnimator.SetAnimation(sVars->aniFrames, 0, true, 9);
            break;

        default: break;
    }
}

#if RETRO_INCLUDE_EDITOR
void UIDiorama::EditorDraw()
{
    this->staticAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 1, true, 0);

    this->staticAnimator.DrawSprite(nullptr, false);
}

void UIDiorama::EditorLoad() { UIDiorama::sVars->aniFrames.Load("UI/Diorama.bin", SCOPE_STAGE); }
#endif

void UIDiorama::Serialize() { RSDK_EDITABLE_VAR(UIDiorama, VAR_ENUM, dioramaID); }
} // namespace GameLogic
