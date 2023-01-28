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

    if (!this->timer) {
        this->state.Run(this);
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
            Graphics::DrawRect(this->dioramaPos.x, this->dioramaPos.y, this->dioramaSize.x, this->dioramaSize.y, this->maskColor, 255, INK_MASKED,
                               false);
        }

        this->inkEffect = INK_MASKED;
        this->stateDraw.Run(this);
        this->inkEffect = INK_NONE;
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
    int32 ids[] = { 0x00, 0x0C, 0x0C, 0x01, 0x03, 0x0F, 0x0D, 0x0E };

    this->lastDioramaID = dioramaID;

    uint8 bankID = 1 + (ids[this->dioramaID] >> 3);
    paletteBank[0].Copy(bankID, 32 * ids[this->dioramaID], 224, 32);

    this->needsSetup = true;

    int32 size = sizeof(int32) + sizeof(Vector2) + sizeof(Animator);
    memset(this->values, 0, size * 16);

    switch (dioramaID) {
        case UIDIORAMA_ADVENTURE:
            this->stateDraw.Set(&UIDiorama::Draw_Adventure);
            this->state.Set(&UIDiorama::State_Adventure);
            break;

        case UIDIORAMA_TIMEATTACK:
            this->stateDraw.Set(&UIDiorama::Draw_TimeAttack);
            this->state.Set(&UIDiorama::State_TimeAttack);
            break;

        case UIDIORAMA_OPTIONS:
            this->stateDraw.Set(&UIDiorama::Draw_Options);
            this->state.Set(&UIDiorama::State_Options);
            break;

        case UIDIORAMA_EXTRAS:
            this->stateDraw.Set(&UIDiorama::Draw_Extras);
            this->state.Set(&UIDiorama::State_Extras);
            break;

        case UIDIORAMA_EXIT:
            this->stateDraw.Set(&UIDiorama::Draw_Exit);
            this->state.Set(&UIDiorama::State_Exit);
            break;

        default: break;
    }
}

void UIDiorama::State_Adventure()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Adventure *info = (UIDiorama_StateInfo_Adventure *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        this->needsSetup = false;
    }
}

void UIDiorama::State_TimeAttack()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_TimeAttack *info = (UIDiorama_StateInfo_TimeAttack *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        this->needsSetup = false;
    }
}

void UIDiorama::State_Options()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Options *info = (UIDiorama_StateInfo_Options *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        this->needsSetup = false;
    }
}

void UIDiorama::State_Extras()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Extras *info = (UIDiorama_StateInfo_Extras *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        this->needsSetup          = false;
    }
}

void UIDiorama::State_Exit()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Exit *info = (UIDiorama_StateInfo_Exit *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        this->needsSetup               = false;
    }
}

void UIDiorama::Draw_Adventure()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Adventure *info = (UIDiorama_StateInfo_Adventure *)this->values;

    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y;

    if (sceneInfo->currentDrawGroup == this->drawGroup) {
        this->inkEffect = INK_NONE;
        info->ehzAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        info->ehzAnimator.DrawSprite(&drawPos, true);
    }
}

void UIDiorama::Draw_TimeAttack()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_TimeAttack *info = (UIDiorama_StateInfo_TimeAttack *)this->values;

    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y;

    if (sceneInfo->currentDrawGroup == this->drawGroup) {
        this->inkEffect = INK_NONE;
        info->tempAnimator.SetAnimation(sVars->aniFrames, 0, true, 1);
        info->tempAnimator.DrawSprite(&drawPos, true);
    }
}

void UIDiorama::Draw_Options()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Options *info = (UIDiorama_StateInfo_Options *)this->values;

    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y;

    if (sceneInfo->currentDrawGroup == this->drawGroup) {
        this->inkEffect = INK_NONE;
        info->tempAnimator1.SetAnimation(sVars->aniFrames, 0, true, 2);
        info->tempAnimator1.DrawSprite(&drawPos, true);
    }
}

void UIDiorama::Draw_Extras()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Extras *info = (UIDiorama_StateInfo_Extras *)this->values;

    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y;

    if (sceneInfo->currentDrawGroup == this->drawGroup) {
        this->inkEffect = INK_NONE;
        info->tempAnimator2.SetAnimation(sVars->aniFrames, 0, true, 3);
        info->tempAnimator2.DrawSprite(&drawPos, true);
    }
}

void UIDiorama::Draw_Exit()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Exit *info = (UIDiorama_StateInfo_Exit *)this->values;

    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y;

    if (sceneInfo->currentDrawGroup == this->drawGroup) {
        this->inkEffect = INK_NONE;
        info->tempAnimator3.SetAnimation(sVars->aniFrames, 0, true, 4);
        info->tempAnimator3.DrawSprite(&drawPos, true);
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
