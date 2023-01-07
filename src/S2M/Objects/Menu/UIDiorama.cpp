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
    if (!(UIWidgets::sVars->timer & 3))
        RSDKTable->RotatePalette(0, 60, 63, true);

    for (auto diorama : GameObject::GetEntities<UIDiorama>(FOR_ALL_ENTITIES)) {
        Graphics::AddDrawListRef(diorama->drawGroup + 1, diorama->Slot());
    }
}

void UIDiorama::Draw()
{
    if (this->timer <= 0) {
        if (sceneInfo->currentDrawGroup == this->drawGroup) {
            this->maskAnimator.DrawSprite(nullptr, false);

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

        this->maskAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 0, true, 0);
        this->staticAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 1, true, 0);

        this->active    = ACTIVE_BOUNDS;
        this->visible   = true;
        this->drawGroup = 2;
    }
}

void UIDiorama::StageLoad()
{
    UIDiorama::sVars->active = ACTIVE_ALWAYS;

    Palette::SetPaletteMask(0x00FF00);

    UIDiorama::sVars->aniFrames.Load("UI/Diorama.bin", SCOPE_STAGE);
    UIDiorama::sVars->capsuleFrames.Load("AIZ/SchrodingersCapsule.bin", SCOPE_STAGE);
    UIDiorama::sVars->sonicFrames.Load("Players/Sonic.bin", SCOPE_STAGE);
    UIDiorama::sVars->tailsFrames.Load("Players/Tails.bin", SCOPE_STAGE);
    UIDiorama::sVars->knuxFramesAIZ.Load("Players/KnuxCutsceneAIZ.bin", SCOPE_STAGE);
    UIDiorama::sVars->knuxFramesHCZ.Load("Players/KnuxCutsceneHPZ.bin", SCOPE_STAGE);
    UIDiorama::sVars->mightyFrames.Load("Players/Mighty.bin", SCOPE_STAGE);
    UIDiorama::sVars->rayFrames.Load("Players/Ray.bin", SCOPE_STAGE);
    UIDiorama::sVars->ringFrames.Load("Global/Ring.bin", SCOPE_STAGE);
    UIDiorama::sVars->speedGateFrames.Load("Global/SpeedGate.bin", SCOPE_STAGE);
    UIDiorama::sVars->bssSonicFrames.Load("SpecialBS/Sonic.bin", SCOPE_STAGE);
    UIDiorama::sVars->bssFrames.Load("SpecialBS/StageObjects.bin", SCOPE_STAGE);

    UIDiorama::sVars->dioramaAlt = UIDIORAMA_ALT_JOG;
    Dev::AddViewableVariable("Diorama Alt", &UIDiorama::sVars->dioramaAlt, Dev::VIEWVAR_UINT8, UIDIORAMA_ALT_RUN, UIDIORAMA_ALT_LOOP);
}

void UIDiorama::ChangeDiorama(uint8 dioramaID)
{
    int32 ids[] = { 0x00, 0x0C, 0x0C, 0x01, 0x03, 0x0F, 0x0D, 0x0E };

    this->lastDioramaID = dioramaID;
    RSDKTable->CopyPalette(((ids[this->dioramaID] >> 3) + 1), (32 * ids[this->dioramaID]), 0, 224, 32);
    this->needsSetup = true;

    int32 size = sizeof(int32) + sizeof(Vector2) + sizeof(Animator);
    memset(this->values, 0, size * 16);

    switch (dioramaID) {
        case UIDIORAMA_MANIAMODE:
            this->stateDraw.Set(&UIDiorama::Draw_ManiaMode);

            switch (UIDiorama::sVars->dioramaAlt) {
                default:
                case UIDIORAMA_ALT_RUN: this->state.Set(&UIDiorama::State_ManiaMode_Alt_Run); break;
                case UIDIORAMA_ALT_JOG: this->state.Set(&UIDiorama::State_ManiaMode_Alt_Jog); break;
                case UIDIORAMA_ALT_LOOP: this->state.Set(&UIDiorama::State_ManiaMode_Alt_Loop); break;
            }
            break;

        case UIDIORAMA_PLUSUPSELL:
            this->stateDraw.Set(&UIDiorama::Draw_PlusUpsell);
            this->state.Set(&UIDiorama::State_PlusUpsell);
            break;

        case UIDIORAMA_ENCOREMODE:
            this->stateDraw.Set(&UIDiorama::Draw_EncoreMode);
            this->state.Set(&UIDiorama::State_EncoreMode);
            break;

        case UIDIORAMA_TIMEATTACK:
            this->stateDraw.Set(&UIDiorama::Draw_TimeAttack);
            this->state.Set(&UIDiorama::State_TimeAttack);
            break;

        case UIDIORAMA_COMPETITION:
            this->stateDraw.Set(&UIDiorama::Draw_Competition);
            this->state.Set(&UIDiorama::State_Competition);
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

void UIDiorama::SetText(String *text)
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_PlusUpsell *info = (UIDiorama_StateInfo_PlusUpsell *)this->values;

    if (text) {
        int32 lineCount = 0;
        int32 linePos   = 0;

        int32 *linePosPtr = info->linePos;
        for (int32 i = 0; i < text->length; ++i) {
            if (text->chars[linePos] == '\n' && lineCount < 3) {
                linePosPtr[lineCount] = linePos;
                ++lineCount;
            }
            ++linePos;
        }

        info->lineCount = lineCount;
        String::Copy(&this->texts[0], text);
        info->textAnimator.SetAnimation(&UIWidgets::sVars->fontFrames, 0, true, 0);
        this->texts->SetSpriteString(UIWidgets::sVars->fontFrames, 0);
    }
}

void UIDiorama::State_ManiaMode_Alt_Run()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_ManiaMode *info = (UIDiorama_StateInfo_ManiaMode *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        info->terrainAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 0);
        info->clouds1Animator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 1);
        info->clouds2Animator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 2);
        info->clouds3Animator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 3);
        info->mountainsAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 4);
        info->backgroundAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 5);
        info->sonicAnimator.SetAnimation(&UIDiorama::sVars->sonicFrames, Player::ANI_RUN, true, 1);
        info->tailsAnimator.SetAnimation(&UIDiorama::sVars->tailsFrames, Player::ANI_RUN, true, 1);
        this->needsSetup = false;
    }
    else {
        info->clouds1ScrollPos += 0x100;
        info->clouds2ScrollPos += 0x80;
        info->clouds3ScrollPos += 0x40;
        info->scrollPos = (info->scrollPos + 0x40000) & 0x3FFFFFFF;
        info->sonicAnimator.Process();
        info->tailsAnimator.Process();
    }
}

void UIDiorama::State_ManiaMode_Alt_Jog()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_ManiaMode *info = (UIDiorama_StateInfo_ManiaMode *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        info->terrainAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 0);
        info->clouds1Animator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 1);
        info->clouds2Animator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 2);
        info->clouds3Animator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 3);
        info->mountainsAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 4);
        info->backgroundAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 5);
        info->sonicAnimator.SetAnimation(&UIDiorama::sVars->sonicFrames, Player::ANI_WALK, true, 1);
        info->tailsAnimator.SetAnimation(&UIDiorama::sVars->tailsFrames, Player::ANI_WALK, true, 1);
        this->needsSetup = false;
    }
    else {
        info->clouds1ScrollPos += 0x100;
        info->clouds2ScrollPos += 0x80;
        info->clouds3ScrollPos += 0x40;
        info->scrollPos = (info->scrollPos + 0x20000) & 0x3FFFFFFF;
        info->sonicAnimator.Process();
        info->tailsAnimator.Process();
    }
}

void UIDiorama::State_ManiaMode_Alt_Loop()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_ManiaMode *info = (UIDiorama_StateInfo_ManiaMode *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        info->terrainAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 0);
        info->clouds1Animator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 1);
        info->clouds2Animator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 2);
        info->clouds3Animator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 3);
        info->mountainsAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 4);
        info->backgroundAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 2, true, 5);
        info->sonicAnimator.SetAnimation(&UIDiorama::sVars->sonicFrames, Player::ANI_WALK, true, 1);
        info->tailsAnimator.SetAnimation(&UIDiorama::sVars->tailsFrames, Player::ANI_WALK, true, 1);
        this->needsSetup  = false;
        info->scrollPos   = 0x1780000;
        info->playerPos.x = -0xC00000;
    }
    else {
        info->playerPos.x += 0x40000;
        if (info->playerPos.x >= 0xC00000)
            info->playerPos.x -= 0x1800000;

        info->scrollPos &= 0x3FFFFFFF;
        info->clouds1ScrollPos += 0x100;
        info->clouds2ScrollPos += 0x80;
        info->clouds3ScrollPos += 0x40;

        info->sonicAnimator.Process();
        info->tailsAnimator.Process();
    }
}

void UIDiorama::State_PlusUpsell()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_PlusUpsell *info = (UIDiorama_StateInfo_PlusUpsell *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        info->dioramaAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 3, true, 0);
        info->flashAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 3, true, 1);
        info->logoAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 3, true, 2);
        info->plusAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 3, true, 3);
        info->arrowAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 3, true, 4);
        info->lightningAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 11, true, 0);

        String text;
        text.Init(nullptr);
        text.Init("", 0);
        this->texts->Init("", 0);
        Localization::GetString(&text, Localization::Default);
        UIDiorama::SetText(&text);
        info->showFlash  = false;
        info->plusPos.y  = 0x520000;
        info->delay      = 30;
        this->needsSetup = false;
    }

    if (info->delay) {
        info->delay--;
    }
    else if (info->showFlash) {
        info->lightningAnimator.Process();

        if (info->flashAlpha) {
            info->flashAlpha -= 8;
        }
    }
    else {
        info->plusVelocity.y -= 0x7000;
        info->plusPos.y += info->plusVelocity.y;
        if (info->plusPos.y < 0) {
            info->plusPos.y             = 0;
            info->showFlash             = true;
            info->flashAlpha            = 256;
            info->arrowAnimator.frameID = 5;
        }
    }
}

void UIDiorama::State_EncoreMode()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_EncoreMode *info = (UIDiorama_StateInfo_EncoreMode *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        info->dioramaAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 4, true, 0);
        info->capsuleAnimator.SetAnimation(&UIDiorama::sVars->capsuleFrames, 0, true, 0);
        info->buttonAnimator.SetAnimation(&UIDiorama::sVars->capsuleFrames, 1, true, 0);
        info->glassAnimator.SetAnimation(&UIDiorama::sVars->capsuleFrames, 2, true, 0);
        info->mightyAnimator.SetAnimation(&UIDiorama::sVars->capsuleFrames, 3, true, 0);
        info->rayAnimator.SetAnimation(&UIDiorama::sVars->capsuleFrames, 4, true, 0);
        this->needsSetup = false;
    }
    else {
        info->glassAnimator.Process();
        info->mightyAnimator.Process();
        info->rayAnimator.Process();
    }
}

void UIDiorama::State_TimeAttack()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_TimeAttack *info = (UIDiorama_StateInfo_TimeAttack *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        info->dioramaAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 5, true, 0);
        info->sonicAnimator.SetAnimation(&UIDiorama::sVars->sonicFrames, Player::ANI_BORED_2, true, 3);
        info->sonicAnimator.loopIndex  = 3;
        info->sonicAnimator.frameCount = 65;

        info->ringAnimator.SetAnimation(&UIDiorama::sVars->ringFrames, 0, true, 0);
        info->ringAnimator.speed = 128;

        info->gateBaseAnimator.SetAnimation(&UIDiorama::sVars->speedGateFrames, 0, true, 0);
        info->gateTopAnimator.SetAnimation(&UIDiorama::sVars->speedGateFrames, 1, true, 0);
        info->gateFinsAnimator.SetAnimation(&UIDiorama::sVars->speedGateFrames, 3, true, 0);
        this->needsSetup = false;
    }
    else {
        info->sonicAnimator.Process();
        info->ringAnimator.Process();
    }
}

void UIDiorama::State_Competition()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Competition *info = (UIDiorama_StateInfo_Competition *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        info->dioramaAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 6, true, 0);
        info->platformAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 10, true, 0);
        info->ringAnimator.SetAnimation(&UIDiorama::sVars->ringFrames, 0, true, 0);
        info->ringAnimator.speed = 128;

        info->tailsAnimator.SetAnimation(&UIDiorama::sVars->tailsFrames, Player::ANI_FLY, true, 0);
        info->tailsAngle = 85;

        info->knuxAnimator.SetAnimation(&UIDiorama::sVars->knuxFramesHCZ, 4, true, 0);
        info->rayAnimator.SetAnimation(&UIDiorama::sVars->rayFrames, Player::ANI_FLY, true, 5);
        info->rayAngle = 0;

        info->mightyAnimator.SetAnimation(&UIDiorama::sVars->mightyFrames, Player::ANI_LOOK_UP, true, 5);
        this->needsSetup = false;
    }
    else {
        info->scrollPos[0] += 0x40;
        info->scrollPos[1] += 0x18;
        info->scrollPos[2] += 0x20;
        info->scrollPos[3] += 0x60;
        info->scrollPos[4] += 0x30;
        info->scrollPos[5] += 0x18;
        info->scrollPos[6] += 0x20;
        info->scrollPos[7] += 0x30;
        info->scrollPos[8] += 0x40;

        info->terrainPos.y = (Math::Sin512(UIWidgets::sVars->timer) + 512) << 10;

        info->platformPos.x = this->position.x + 0x2B0000;
        info->platformPos.y = this->position.y + 0x320000;
        info->platformPos.y += Math::Sin1024(5 * (UIWidgets::sVars->timer + 128)) << 11;
        info->platformPos.y &= 0xFFFF0000;

        info->tailsPos.x = this->position.x + 0x8B0000;
        info->tailsPos.y = this->position.y - 0xE0000;
        info->tailsPos.y += Math::Sin256(info->tailsAngle) << 10;
        info->tailsAngle = (info->tailsAngle + 1) & 0xFF;

        info->knuxPos.x = info->platformPos.x;
        info->knuxPos.y = info->platformPos.y - 0x180000;

        info->rayPos.x = this->position.x + 0x360000;
        info->rayPos.y = this->position.y - 0x350000;
        info->rayPos.y += Math::Sin256(info->rayAngle) << 10;
        info->rayAngle = (info->rayAngle + 1) & 0xFF;

        info->mightyPos.x = info->platformPos.x + 0x100000;
        info->mightyPos.y = info->platformPos.y - 0x180000;

        info->platformAnimator.Process();
        info->ringAnimator.Process();
        info->tailsAnimator.Process();
        info->knuxAnimator.Process();
        info->rayAnimator.Process();
    }
}

void UIDiorama::State_Options()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Options *info = (UIDiorama_StateInfo_Options *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        info->dioramaAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 7, true, 0);
        info->sonicAnimator.SetAnimation(&UIDiorama::sVars->sonicFrames, Player::ANI_IDLE, true, 17);
        info->tailsAnimator.SetAnimation(&UIDiorama::sVars->tailsFrames, Player::ANI_IDLE, true, 19);
        info->knuxAnimator.SetAnimation(&UIDiorama::sVars->knuxFramesAIZ, 1, true, 0);
        info->itemConstellationAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 7, true, 1);
        info->contConstellationAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 7, true, 2);
        info->audioConstellationAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 7, true, 3);
        info->itemPos.x = 0x560000;
        info->itemPos.y = -0x240000;
        info->itemAlpha = 0x80;
        info->itemAngle = 0;

        info->contPos.x = 0x940000;
        info->contPos.y = 0;
        info->contAlpha = 0x100;
        info->contAngle = 85;

        info->audioPos.x = 0xCA0000;
        info->audioPos.y = -0x240000;
        info->audioAlpha = 0xD4;
        info->audioAngle = 170;

        this->needsSetup = false;
    }
    else {
        info->itemAlpha    = CLAMP(info->itemAlpha + Math::Rand(0, 20) - 10, 0xC8, 0xFF);
        info->itemOffset.y = Math::Sin256(info->itemAngle) << 10;
        info->itemAngle    = (info->itemAngle + 1) & 0xFF;

        info->contAlpha    = CLAMP(info->contAlpha + Math::Rand(0, 20) - 10, 0xC8, 0xFF);
        info->contOffset.y = Math::Sin256(info->contAngle) << 10;
        info->contAngle    = (info->contAngle + 1) & 0xFF;

        info->audioAlpha    = CLAMP(info->audioAlpha + Math::Rand(0, 20) - 10, 0xC8, 0xFF);
        info->audioOffset.y = Math::Sin256(info->audioAngle) << 10;
        info->audioAngle    = (info->audioAngle + 1) & 0xFF;

        info->knuxAnimator.Process();
    }
}

void UIDiorama::State_Extras()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Extras *info = (UIDiorama_StateInfo_Extras *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        info->dioramaAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 8, true, 0);
        info->medalAnimator.SetAnimation(&UIDiorama::sVars->bssFrames, 8, true, 0);
        info->sonicAnimator.SetAnimation(&UIDiorama::sVars->bssSonicFrames, 1, true, 0);
        info->sonicAnimator.speed = 12;
        this->needsSetup          = false;
    }
    else {
        info->medalAnimator.Process();
        info->sonicAnimator.Process();
    }
}

void UIDiorama::State_Exit()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Exit *info = (UIDiorama_StateInfo_Exit *)this->values;

    if (this->needsSetup) {
        this->maskColor = 0x00FF00;
        info->dioramaAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 9, true, 0);
        info->sonicAnimator.SetAnimation(&UIDiorama::sVars->sonicFrames, Player::ANI_IDLE, true, 0);
        info->sonicAnimator.loopIndex  = 5;
        info->sonicAnimator.frameCount = 11;
        this->needsSetup               = false;
    }
    else {
        if (info->sonicAnimator.frameCount == 11 && info->sonicAnimator.frameID == 10
            && info->sonicAnimator.timer == info->sonicAnimator.frameDuration - 1 && ++info->animDelay == 1) {
            info->sonicAnimator.loopIndex  = 14;
            info->sonicAnimator.frameCount = 15;
        }

        if (info->sonicAnimator.frameID == 14 && !info->processVelocity && !info->isOffScreen) {
            info->processVelocity = true;
            info->sonicVelocity.y = -0x40000;
            info->sonicVelocity.x = 0x10000;
        }

        if (info->processVelocity) {
            info->sonicPos.x += info->sonicVelocity.x;
            info->sonicPos.y += info->sonicVelocity.y;
            info->sonicVelocity.y += 0x3800;

            if (info->sonicPos.y > 0x800000) {
                info->processVelocity = false;
                info->isOffScreen     = true;
            }
        }

        info->sonicAnimator.Process();
    }
}

void UIDiorama::Draw_ManiaMode()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_ManiaMode *info = (UIDiorama_StateInfo_ManiaMode *)this->values;

    Vector2 drawPos;
    int32 frameWidths[] = { 0x400, 0x200, 0x200, 0x200, 0x200, 0x129 };
    int32 frameSpeeds[] = { 0x100, 0x30, 0x30, 0x30, 0x60, 0x80 };

    if (sceneInfo->currentDrawGroup == this->drawGroup) {
        drawPos.x = this->position.x + 0x380000;
        drawPos.y = this->position.y + 0x1D0000;
        drawPos.x = info->playerPos.x + this->position.x + 0x380000;
        drawPos.y = info->playerPos.y + this->position.y + 0x1D0000;
        info->tailsAnimator.DrawSprite(&drawPos, false);

        drawPos.y -= 0x40000;
        drawPos.x += 0x280000;
        info->sonicAnimator.DrawSprite(&drawPos, false);

        // Draw Stage "Parallax"
        int32 offsets[]            = { 0, info->clouds1ScrollPos, info->clouds2ScrollPos, info->clouds3ScrollPos, 0, 0 };
        Animator *levelAnimators[] = { &info->terrainAnimator, &info->clouds1Animator,   &info->clouds2Animator,
                                       &info->clouds3Animator, &info->mountainsAnimator, &info->backgroundAnimator };

        for (int32 i = 0; i < 6; ++i) {
            drawPos     = this->position;
            int32 width = frameWidths[i] << 16;

            int32 offset2 = -(int32)((offsets[i] << 8) + ((frameSpeeds[i] * (uint32)(info->scrollPos >> 4)) >> 4));
            while (offset2 < -0x10000 * frameWidths[i]) {
                offset2 += width;
            }

            drawPos.x += offset2;
            levelAnimators[i]->DrawSprite(&drawPos, false);

            drawPos.x += width;
            levelAnimators[i]->DrawSprite(&drawPos, false);
        }

        // Draw Sky blue rect to fill in the sky BG for any pixels that aren't covered up
        Graphics::DrawRect(this->dioramaPos.x, this->dioramaPos.y, this->dioramaSize.x, this->dioramaSize.y, 0x2001A0, 255, INK_MASKED, false);
    }
}

void UIDiorama::Draw_PlusUpsell()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_PlusUpsell *info = (UIDiorama_StateInfo_PlusUpsell *)this->values;

    Vector2 drawPos;

    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    if (sceneInfo->currentDrawGroup == this->drawGroup) {
        info->dioramaAnimator.DrawSprite(&drawPos, false);

        drawPos.x = this->position.x + 0x500000;
        drawPos.y = this->position.y + 0x2E0000;
        info->logoAnimator.DrawSprite(&drawPos, false);
        info->arrowAnimator.DrawSprite(&drawPos, false);

        drawPos.y += info->plusPos.y;
        info->plusAnimator.DrawSprite(&drawPos, false);

        color rectColor = info->showFlash ? 0x01D870 : 0xF0C801;
        Graphics::DrawRect(this->dioramaPos.x, this->dioramaPos.y, this->dioramaSize.x, this->dioramaSize.y, rectColor, 0xFF, INK_MASKED, false);
    }
    else {
        this->inkEffect = INK_ADD;
        this->alpha     = 0xFF;
        if (info->showFlash) {
            drawPos.x = this->position.x + 0x500000;
            drawPos.y = this->position.y + 0x2E0000;
            info->lightningAnimator.DrawSprite(&drawPos, false);
        }

        this->inkEffect = INK_NONE;
        drawPos.x       = this->position.x + 0x840000;
        drawPos.y       = this->position.y - 0x480000;

        int32 length[5];
        length[0] = info->lineCount;
        length[1] = info->linePos[0];
        length[2] = info->linePos[1];
        length[3] = info->linePos[2];
        length[4] = info->linePos[3];

        int32 lineCount = length[0];
        for (int32 i = 0; i < lineCount + 1; ++i) {
            int32 start = 0;
            int32 end   = 0;
            if (i > 0)
                start = length[i] + 1;

            if (i >= lineCount)
                end = this->texts[0].length;
            else
                end = length[i + 1];

            int32 width = -0x8000 * this->texts->GetWidth(UIWidgets::sVars->fontFrames, 0, start, end, 0);
            drawPos.x += width;
            RSDKTable->DrawText(&info->textAnimator, &drawPos, &this->texts[0], start, end, UIButton::ALIGN_LEFT, 0, nullptr, nullptr, false);

            drawPos.x -= width;
            drawPos.y += 0x120000;
        }

        drawPos         = this->position;
        this->inkEffect = INK_ALPHA;
        this->alpha     = info->flashAlpha;
        info->flashAnimator.DrawSprite(&drawPos, false);

        this->inkEffect = INK_NONE;
    }
}

void UIDiorama::Draw_EncoreMode()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_EncoreMode *info = (UIDiorama_StateInfo_EncoreMode *)this->values;

    Vector2 drawPos;

    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    if (sceneInfo->currentDrawGroup == this->drawGroup) {
        info->dioramaAnimator.frameID = 0;
        info->dioramaAnimator.DrawSprite(&drawPos, false);
    }
    else {
        this->inkEffect = INK_NONE;
        drawPos.y += 0x200000;
        drawPos.x += 0x500000;
        int32 x = drawPos.x;
        info->buttonAnimator.DrawSprite(&drawPos, false);

        drawPos.x += 0xE0000;
        info->mightyAnimator.DrawSprite(&drawPos, false);

        drawPos.x -= 0x1C0000;
        info->rayAnimator.DrawSprite(&drawPos, false);

        drawPos.x = x;
        info->capsuleAnimator.DrawSprite(&drawPos, false);

        this->inkEffect = INK_ADD;
        this->alpha     = 0x80;
        info->glassAnimator.DrawSprite(&drawPos, false);

        this->inkEffect               = INK_NONE;
        info->dioramaAnimator.frameID = 1;
        info->dioramaAnimator.DrawSprite(&this->position, false);
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
        info->dioramaAnimator.frameID = 0;
        info->dioramaAnimator.DrawSprite(&drawPos, false);
    }
    else {
        this->inkEffect = INK_NONE;
        drawPos.x += 0x340000;
        drawPos.y -= 0x30000;
        info->sonicAnimator.DrawSprite(&drawPos, false);

        drawPos.x = this->position.x + 0x9B0000;
        drawPos.y = this->position.y - 0x400000;
        for (int32 i = 0; i < 3; ++i) {
            info->ringAnimator.DrawSprite(&drawPos, false);
            drawPos.x += 0x1C0000;
        }

        Vector2 gatePos;
        gatePos.x = this->position.x + 0x6C0000;
        gatePos.y = this->position.y - 0x130000;
        info->gateTopAnimator.DrawSprite(&gatePos, false);
        info->gateBaseAnimator.DrawSprite(&gatePos, false);

        this->drawFX                   = FX_SCALE;
        this->scale.y                  = 0x200;
        drawPos.x                      = gatePos.x;
        drawPos.y                      = gatePos.y;
        this->scale.x                  = abs(Math::Sin512(0));
        drawPos.x                      = gatePos.x + 0x30000;
        info->gateFinsAnimator.frameID = 1;
        info->gateFinsAnimator.DrawSprite(&drawPos, false);

        this->scale.x                  = abs(Math::Cos512(0));
        drawPos.x                      = gatePos.x - 0x30000;
        info->gateFinsAnimator.frameID = 0;
        info->gateFinsAnimator.DrawSprite(&drawPos, false);

        drawPos.x                      = gatePos.x + 0x180 * Math::Cos512(0);
        info->gateFinsAnimator.frameID = 1;
        info->gateFinsAnimator.DrawSprite(&drawPos, false);

        this->scale.x                  = abs(Math::Sin512(0));
        drawPos.x                      = gatePos.x + 0xB40 * Math::Cos512(0);
        info->gateFinsAnimator.frameID = 2;
        info->gateFinsAnimator.DrawSprite(&drawPos, false);

        this->scale.x                  = abs(Math::Sin512(0));
        drawPos.x                      = gatePos.x + 0x180 * Math::Cos512(0);
        info->gateFinsAnimator.frameID = 0;
        info->gateFinsAnimator.DrawSprite(&drawPos, false);

        this->scale.x                  = abs(Math::Cos512(0));
        drawPos.x                      = gatePos.x - 0xB40 * Math::Sin512(0);
        info->gateFinsAnimator.frameID = 2;
        info->gateFinsAnimator.DrawSprite(&drawPos, false);

        this->drawFX = FX_NONE;
    }
}

void UIDiorama::Draw_Competition()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Competition *info = (UIDiorama_StateInfo_Competition *)this->values;

    Vector2 drawPos;

    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    if (sceneInfo->currentDrawGroup == this->drawGroup) {
        drawPos.x = this->position.x + 0xAB0000;
        drawPos.y = this->position.y - 0x190000;
        for (int32 i = 0; i < 3; ++i) {
            drawPos.x += 0x200000;
            info->ringAnimator.DrawSprite(&drawPos, false);
        }

        drawPos = info->platformPos;
        info->platformAnimator.DrawSprite(&drawPos, false);

        drawPos                       = this->position;
        info->dioramaAnimator.frameID = 0;
        info->dioramaAnimator.DrawSprite(&drawPos, false);

        drawPos.y += info->terrainPos.y;
        for (int32 i = 1; i <= 11; ++i) {
            drawPos.x                     = this->position.x;
            info->dioramaAnimator.frameID = i;

            int32 scrollPos = -(info->scrollPos[i - 1] << 8);
            if (scrollPos < -0x2000000)
                scrollPos += ((-0x2000001 - scrollPos) & 0xFE000000) + 0x2000000;
            drawPos.x += scrollPos;
            info->dioramaAnimator.DrawSprite(&drawPos, false);

            drawPos.x += 0x2000000;
            info->dioramaAnimator.DrawSprite(&drawPos, false);
        }

        Graphics::DrawRect(this->dioramaPos.x, this->dioramaPos.y, this->dioramaSize.x, this->dioramaSize.y, 0x860F0, 255, INK_MASKED, false);
    }
    else {
        this->inkEffect   = INK_NONE;
        int32 playerCount = 2;

        Vector2 *playerPos[]        = { &info->tailsPos, &info->knuxPos, &info->rayPos, &info->mightyPos };
        Animator *playerAnimators[] = { &info->tailsAnimator, &info->knuxAnimator, &info->rayAnimator, &info->mightyAnimator };

        for (int32 i = 0; i < playerCount; ++i) {
            playerAnimators[i]->DrawSprite(playerPos[i], false);
        }
    }
}

void UIDiorama::Draw_Options()
{
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Options *info = (UIDiorama_StateInfo_Options *)this->values;

    Vector2 drawPos;

    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    if (sceneInfo->currentDrawGroup != this->drawGroup) {
        Vector2 *positions[]  = { &info->itemPos, &info->contPos, &info->audioPos };
        Vector2 *offsets[]    = { &info->itemOffset, &info->contOffset, &info->audioOffset };
        int32 alpha[]         = { info->itemAlpha, info->contAlpha, info->audioAlpha };
        Animator *animators[] = { &info->itemConstellationAnimator, &info->contConstellationAnimator, &info->audioConstellationAnimator };

        // Draw Constellations
        for (int32 i = 0; i < 3; ++i) {
            drawPos = this->position;
            drawPos.x += positions[i]->x;
            drawPos.y += positions[i]->y;
            drawPos.x += offsets[i]->x;
            drawPos.y += offsets[i]->y;

            this->alpha     = alpha[i];
            this->inkEffect = INK_ALPHA;
            animators[i]->DrawSprite(&drawPos, false);

            this->inkEffect = INK_ADD;
            animators[i]->DrawSprite(&drawPos, false);
        }

        this->inkEffect = INK_NONE;
        drawPos.x       = this->position.x + 0x380000;
        drawPos.y       = this->position.y + 0x1E0000;
        info->knuxAnimator.DrawSprite(&drawPos, false);

        drawPos.x = this->position.x + 0x6C0000;
        drawPos.y = this->position.y + 0x210000;
        info->tailsAnimator.DrawSprite(&drawPos, false);

        drawPos.y -= 0x30000;
        drawPos.x += 0x180000;
        info->sonicAnimator.DrawSprite(&drawPos, false);
    }
    else {
        info->dioramaAnimator.DrawSprite(&drawPos, false);
    }
}

void UIDiorama::Draw_Extras()
{
    Vector2 drawPos;
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Extras *info = (UIDiorama_StateInfo_Extras *)this->values;

    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    if (sceneInfo->currentDrawGroup != this->drawGroup) {
        this->inkEffect = INK_NONE;
        drawPos.x       = this->position.x + 0x520000;
        drawPos.y       = this->position.y - 0x150000;
        info->medalAnimator.DrawSprite(&drawPos, false);

        drawPos.x = this->position.x + 0x520000;
        drawPos.y = this->position.y + 0x390000;
        info->sonicAnimator.DrawSprite(&drawPos, false);
    }
    else {
        info->dioramaAnimator.DrawSprite(&drawPos, false);
    }
}

void UIDiorama::Draw_Exit()
{
    Vector2 drawPos;
    // Using this makes these states FAR more readable
    UIDiorama_StateInfo_Exit *info = (UIDiorama_StateInfo_Exit *)this->values;

    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    if (sceneInfo->currentDrawGroup != this->drawGroup) {
        this->inkEffect = INK_NONE;
        drawPos.x += 0x480000;
        drawPos.y += 0xD0000;
        drawPos.x += info->sonicPos.x;
        drawPos.y += info->sonicPos.y;
        info->sonicAnimator.DrawSprite(&drawPos, false);
    }
    else {
        info->dioramaAnimator.DrawSprite(&drawPos, false);
    }
}

#if RETRO_INCLUDE_EDITOR
void UIDiorama::EditorDraw()
{
    this->maskAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 0, true, 0);
    this->staticAnimator.SetAnimation(&UIDiorama::sVars->aniFrames, 1, true, 0);

    this->maskAnimator.DrawSprite(nullptr, false);
    this->staticAnimator.DrawSprite(nullptr, false);
}

void UIDiorama::EditorLoad() { UIDiorama::sVars->aniFrames.Load("UI/Diorama.bin", SCOPE_STAGE); }
#endif

void UIDiorama::Serialize() { RSDK_EDITABLE_VAR(UIDiorama, VAR_ENUM, dioramaID); }
} // namespace GameLogic
