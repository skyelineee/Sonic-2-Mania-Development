// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UISaveSlot Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "UISaveSlot.hpp"
#include "UIWidgets.hpp"
#include "UIControl.hpp"
#include "UIDialog.hpp"
#include "UIButtonPrompt.hpp"
#include "UILoadingIcon.hpp"
#include "Global/Zone.hpp"
#include "Global/SaveGame.hpp"
#include "Global/Localization.hpp"
#include "Global/Music.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UISaveSlot);

void UISaveSlot::Update()
{
    UISaveSlot::SetupButtonElements();

    if (!this->textFrames.Matches(&UIWidgets::sVars->textFrames)) {
        UISaveSlot::SetupAnimators();
        UISaveSlot::HandleSaveIcons();
        this->textFrames = UIWidgets::sVars->textFrames;
    }

    this->state.Run(this);

    if (++this->zoneIconSprX >= 192)
        this->zoneIconSprX -= 192;

    UIControl *control = (UIControl *)this->parent;

    // ???
    for (int32 i = 0; i < control->buttonCount; ++i) {
        if (this == (UISaveSlot *)control->buttons[i])
            break;
    }

    if (this->isSelected) {
        this->textBouncePos -= 0x600;
        this->textBounceOffset += this->textBouncePos;

        if (this->textBounceOffset >= 0x13600)
            this->textBounceOffset = 0x13600;

        if (this->textBounceOffset <= 0x10000 && this->textBouncePos < 0) {
            this->textBounceOffset = 0x10000;
            this->textBouncePos    = 0;
        }

        this->buttonBouncePos -= 0x1800;
        this->buttonBounceOffset += this->buttonBouncePos;

        if (this->buttonBounceOffset >= 0x11800)
            this->buttonBounceOffset = 0x11800;

        if (this->buttonBounceOffset <= 0x8000 && this->buttonBouncePos < 0) {
            this->buttonBounceOffset = 0x8000;
            this->buttonBouncePos    = 0;
        }
    }
    else if (!this->state.Matches(&UISaveSlot::State_Selected)) {
        if (this->textBounceOffset > 0) {
            this->textBounceOffset -= 0x2000;

            if (this->textBounceOffset < 0)
                this->textBounceOffset = 0;
        }

        if (this->buttonBounceOffset > 0) {
            this->buttonBounceOffset -= 0x2000;

            if (this->buttonBounceOffset < 0)
                this->buttonBounceOffset = 0;
        }
    }

    if (this->currentlySelected && (control->buttons[control->lastButtonID] != (UIButton *)this || control->buttonID == -1)) {
        this->isSelected        = false;
        this->currentlySelected = false;
        this->state.Set(&UISaveSlot::State_NotSelected);
        this->stateInput.Set(nullptr);
    }
    if (!control->state.Matches(&UIControl::ProcessInputs) || control->active != ACTIVE_ALWAYS) {
        if (this->currentlySelected) {
            this->isSelected        = false;
            this->currentlySelected = false;
            this->state.Set(&UISaveSlot::State_NotSelected);
            this->stateInput.Set(nullptr);
        }

        this->active = ACTIVE_BOUNDS;
    }
}

void UISaveSlot::LateUpdate() {}

void UISaveSlot::StaticUpdate() {}

void UISaveSlot::Draw()
{
    Vector2 drawPos;
    if (this->type == UISAVESLOT_REGULAR) {
        drawPos.y = this->position.y + 0x30000;
        drawPos.x = this->position.x + 0x30000;
        Graphics::DrawRect(this->position.x - 0x2D0000, this->position.y - 0x4F0000, 0x600000, 0xA40000, 0xFFFFFF, 127, INK_BLEND, false);

        drawPos.x = this->position.x - 0x2D0000;
        drawPos.y = this->position.y - 0x100000;
        UIWidgets::DrawRightTriangle(drawPos.x, drawPos.y, (this->textBounceOffset >> 11), 232, 40, 88);

        drawPos.x = this->position.x + 0x2D0000;
        drawPos.y = 0x4F0000 + this->position.y;
        UIWidgets::DrawRightTriangle(drawPos.x, drawPos.y, (-64 * this->textBounceOffset) >> 16, 0x60, 0xA0, 0xB0);
        UIWidgets::DrawRightTriangle(drawPos.x, drawPos.y, (-44 * this->textBounceOffset) >> 16, 0x58, 0x70, 0xE0);

        drawPos.x = this->position.x + 0x30000;
        drawPos.y = this->position.y + 0x30000;
        UIWidgets::DrawRectOutline_Blended(drawPos.x, drawPos.y, 96, 164);

        Graphics::DrawRect(drawPos.x - 0x2D0000, drawPos.y - 0x130000, 0x5A0000, 0x30000, 0x000000, 0xFF, INK_BLEND, false);

        if (this->isSelected)
            UIWidgets::DrawRectOutline_Flash(this->position.x, this->position.y, 96, 164);
        else
            UIWidgets::DrawRectOutline_Black(this->position.x, this->position.y, 96, 164);

        this->uiAnimator.frameID = 0;
        drawPos.x                = this->position.x;
        drawPos.y                = this->position.y - 0x300000;
        this->uiAnimator.DrawSprite(&drawPos, false);

        if (this->isNewSave || this->saveZoneID == 0xFF) {
            this->iconBGAnimator.DrawSprite(&drawPos, false);
            this->saveStatusAnimator.DrawSprite(&drawPos, false);
        }
        else if ((this->isSelected || this->state.Matches(&UISaveSlot::State_Selected) && this->saveZoneID <= Zone::ZoneCountSaveFile)) {
            SpriteFrame *frame = sVars->aniFrames.GetFrame(5, this->saveZoneID);
            frame->pivotX      = -43;
            frame->width       = 86;
            frame->sprX        = this->zoneIconSprX;

            if (frame->sprX > 106) {
                int32 width  = frame->sprX - 106;
                frame->width = 86 - width;
                this->zoneIconAnimator.DrawSprite(&drawPos, false);

                frame->pivotX += frame->width;
                frame->sprX  = 0;
                frame->width = width;
            }

            this->zoneIconAnimator.DrawSprite(&drawPos, false);
            this->zoneNameAnimator.DrawSprite(&drawPos, false);
        }
        else {
            this->drawFX = FX_FLIP;
            this->fuzzAnimator.DrawSprite(&drawPos, false);

            this->drawFX = FX_NONE;
            this->zoneNameAnimator.DrawSprite(&drawPos, false);
        }

        if (this->isNewSave) {
            drawPos.x = this->position.x;
            drawPos.y = this->position.y + 0x200000;
            UISaveSlot::DrawPlayerIcons(drawPos.x, drawPos.y);
        }
        else {
            Graphics::DrawRect(this->position.x - 0x2D0000, this->position.y + 0x3D0000, 0x5A0000, 0x100000, 0x000000, 0xFF, INK_NONE, false);

            drawPos.x = this->position.x - 0x240000;
            drawPos.y = 0x450000 + this->position.y;
            for (int32 i = 0; i < 7; ++i) {
                this->emeraldsAnimator.frameID = ((1 << i) & this->saveEmeralds) ? i : 7;
                this->emeraldsAnimator.DrawSprite(&drawPos, false);

                drawPos.x += 0xC0000;
            }

            drawPos.x = this->position.x;
            drawPos.y = this->position.y + 0x100000;
            if (this->type == UISAVESLOT_NOSAVE) {
                if (!this->saveContinues) {
                    drawPos.y += 0x20000;
                }
                drawPos.y += 0x20000;
            }
            else
                drawPos.y += 0x80000;
            UISaveSlot::DrawPlayerIcons(drawPos.x, drawPos.y);

            drawPos.x = this->position.x;
            drawPos.y = this->position.y + 0x330000;
            UISaveSlot::DrawPlayerInfo(drawPos.x, drawPos.y);
        }
    }
    else {
        drawPos.y = this->position.y + 0x30000;
        drawPos.x = this->position.x + 0x30000;
        Graphics::DrawRect(this->position.x - 0x2D0000, this->position.y - 0x1F0000, 0x600000, 0x440000, 0xFFFFFF, 0x7F, INK_BLEND, false);

        drawPos.x = this->position.x + 0x30000;
        drawPos.y = this->position.y + 0x30000;
        UIWidgets::DrawRectOutline_Blended(drawPos.x, drawPos.y, 96, 68);

        this->uiAnimator.frameID = 1;
        this->uiAnimator.DrawSprite(nullptr, false);

        drawPos = this->position;
        UISaveSlot::DrawPlayerIcons(this->position.x, this->position.y);
        this->zoneNameAnimator.DrawSprite(nullptr, false);
    }

    if (this->isSelected || this->type == UISAVESLOT_REGULAR) {
        if (this->type == UISAVESLOT_NOSAVE)
            UIWidgets::DrawRectOutline_Flash(this->position.x, this->position.y, 96, 68);

        if (this->isSelected && !(this->zoneIconSprX & 8)) {
            drawPos.x = this->position.x;
            drawPos.y = this->position.y;
            if (this->type == UISAVESLOT_NOSAVE) {
                drawPos.y -= 0x60000;
                UIWidgets::DrawUpDownArrows(drawPos.x, drawPos.y, 40);
            }
            else if (this->isNewSave) {
                drawPos.y += 0x200000;
                UIWidgets::DrawUpDownArrows(drawPos.x, drawPos.y, 64);
            }
            else if (this->listID == 1) {
                drawPos.y -= 0x300000;
                UIWidgets::DrawUpDownArrows(drawPos.x, drawPos.y, 40);
            }
        }
    }

    Graphics::DrawCircle(screenInfo->center.x, screenInfo->center.y, this->fxRadius, 0x000000, this->alpha, INK_ALPHA, true);
}

void UISaveSlot::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->visible         = true;
        this->drawGroup       = 2;
        this->active          = ACTIVE_BOUNDS;
        this->updateRange.x   = 0x800000;
        this->updateRange.y   = 0x300000;
        this->processButtonCB.Set(&UISaveSlot::ProcessButtonCB);
        this->state.Set(&UISaveSlot::State_NotSelected);

        if (this->type == UISAVESLOT_REGULAR) {
            UISaveSlot::LoadSaveInfo();
        }
        else {
            this->slotID = NO_SAVE_SLOT;
            UISaveSlot::SetupAnimators();
        }

        this->textFrames = UIWidgets::sVars->textFrames;
        UISaveSlot::HandleSaveIcons();
    }
}

void UISaveSlot::StageLoad() { sVars->aniFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE); }

uint8 UISaveSlot::GetPlayerIDFromID(uint8 id)
{
    switch (id) {
        case 1: return ID_SONIC;
        case 2: return ID_TAILS;
        case 3: return ID_KNUCKLES;
        default: break;
    }

    return ID_NONE;
}
uint8 UISaveSlot::GetIDFromPlayerID(uint8 playerID)
{
    switch (playerID) {
        case ID_SONIC: return 1;
        case ID_TAILS: return 2;
        case ID_KNUCKLES: return 3;
        default: break;
    }

    return ID_NONE;
}

void UISaveSlot::DrawPlayerIcons(int32 drawX, int32 drawY)
{
    Vector2 drawPos;

    this->playersAnimator.SetAnimation(sVars->aniFrames, 1, true, 3);
    this->shadowsAnimator.SetAnimation(sVars->aniFrames, 2, true, 3);

    int32 frames[]                = { 3, 0, 1, 2, 4, 5 };
    this->shadowsAnimator.frameID = frames[this->frameID];
    this->playersAnimator.frameID = frames[this->frameID];

    drawX += 4 * this->buttonBounceOffset;
    drawY += 4 * this->buttonBounceOffset;
    drawPos.x = drawX;
    drawPos.y = drawY;
    this->shadowsAnimator.DrawSprite(&drawPos, false);

    drawX -= 8 * this->buttonBounceOffset;
    drawY -= 8 * this->buttonBounceOffset;
    drawPos.x = drawX;
    drawPos.y = drawY;
    this->playersAnimator.DrawSprite(&drawPos, false);
}

void UISaveSlot::DrawPlayerInfo(int32 drawX, int32 drawY)
{
    this->livesAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
    this->continuesAnimator.SetAnimation(sVars->aniFrames, 21, true, 0);

    int32 playerID = 0;

    int32 frames[] = { 0, 0, 1, 2, 3, 4 };
    playerID       = frames[this->frameID];

    this->livesAnimator.frameID = playerID;
    this->continuesAnimator.frameID = playerID;

    Animator *animators[2];
    animators[0] = &this->livesAnimator;
    animators[1] = &this->continuesAnimator;

    int32 retries[2];
    retries[0] = MIN(this->saveLives, 99);
    retries[1] = MIN(this->saveContinues, 99);

    Vector2 positions[2];
    positions[0].x = drawX;
    positions[0].y = drawY;
    positions[1].y = drawY;
    positions[1].x = drawX;

    int32 count = 1;
    if (retries[1] <= 0) {
        positions[0].x = drawX - 0x20000;
    }
    else {
        positions[0].x = drawX - 0x170000;
        positions[1].x = drawX + 0x170000;
        count          = 2;
    }

    for (int32 i = 0; i < count; ++i) {
        Vector2 drawPos;

        drawPos.x = positions[i].x;
        drawPos.y = positions[i].y;
        if (true) {
            this->uiAnimator.frameID = 3;
            this->uiAnimator.DrawSprite(&drawPos, false);

            drawPos.x -= 0xC0000;
            animators[i]->DrawSprite(&drawPos, false);

            drawPos.x += 0x140000;
            this->numbersAnimator.frameID = retries[i] / 10;
            this->numbersAnimator.DrawSprite(&drawPos, false);

            drawPos.x += 0x80000;
            this->numbersAnimator.frameID = retries[i] % 10;
            this->numbersAnimator.DrawSprite(&drawPos, false);
        }
    }
}

void UISaveSlot::SetupButtonElements()
{
    this->touchCB.Set(&UIButton::ProcessTouchCB_Multi);
    this->selectedCB.Set(&UISaveSlot::SelectedCB);
    this->failCB.Set(nullptr);
    this->buttonEnterCB.Set(&UISaveSlot::ButtonEnterCB);
    this->buttonLeaveCB.Set(&UISaveSlot::ButtonLeaveCB);
    this->checkButtonEnterCB.Set(&UISaveSlot::CheckButtonEnterCB);
    this->checkSelectedCB.Set(&UISaveSlot::CheckSelectedCB);

    SaveGame::SaveRAM *saveRAM = (SaveGame::SaveRAM *)SaveGame::GetSaveDataPtr(this->slotID);

    if (this->type == UISAVESLOT_NOSAVE) {
        this->touchPosSizeM[0].x   = 0x600000;
        this->touchPosSizeM[0].y   = 0x440000;
        this->touchPosOffsetM[0].x = 0;
        this->touchPosOffsetM[0].y = 0;
        this->touchPosCallbacks[0].Set(&UISaveSlot::SelectedCB);

        this->touchPosSizeM[1].x   = 0x200000;
        this->touchPosSizeM[1].y   = 0x200000;
        this->touchPosOffsetM[1].x = 0;
        this->touchPosOffsetM[1].y = -0x1A0000;
        this->touchPosCallbacks[1].Set(&UISaveSlot::NextCharacter);

        this->touchPosSizeM[2].x   = 0x200000;
        this->touchPosSizeM[2].y   = 0x200000;
        this->touchPosOffsetM[2].x = 0;
        this->touchPosOffsetM[2].y = 0xE0000;
        this->touchPosCallbacks[2].Set(&UISaveSlot::PrevCharacter);

        this->touchPosCount = 3;
    }
    else {
        this->touchPosSizeM[0].x   = 0x600000;
        this->touchPosSizeM[0].y   = 0xA40000;
        this->touchPosOffsetM[0].x = 0;
        this->touchPosOffsetM[0].y = 0;
        this->touchPosCallbacks[0].Set(&UISaveSlot::SelectedCB);

        switch (saveRAM->saveState) {
            default:
            case SaveGame::SaveEmpty:
                this->touchPosSizeM[1].x   = 0x200000;
                this->touchPosSizeM[1].y   = 0x200000;
                this->touchPosOffsetM[1].x = 0;
                this->touchPosOffsetM[1].y = 0;
                this->touchPosCallbacks[1].Set(&UISaveSlot::NextCharacter);

                this->touchPosSizeM[2].x   = 0x200000;
                this->touchPosSizeM[2].y   = 0x200000;
                this->touchPosOffsetM[2].x = 0;
                this->touchPosOffsetM[2].y = 0x400000;
                this->touchPosCallbacks[2].Set(&UISaveSlot::PrevCharacter);

                this->touchPosCount = 3;
                break;

            case SaveGame::SaveActive: this->touchPosCount = 1; break;

            case SaveGame::SaveCompleted:
                this->touchPosSizeM[1].x   = 0x200000;
                this->touchPosSizeM[1].y   = 0x200000;
                this->touchPosOffsetM[1].y = -0x440000;
                this->touchPosOffsetM[1].x = 0;
                this->touchPosCallbacks[1].Set(&UISaveSlot::NextZone);

                this->touchPosSizeM[2].x   = 0x200000;
                this->touchPosSizeM[2].y   = 0x200000;
                this->touchPosOffsetM[2].x = 0;
                this->touchPosOffsetM[2].y = 0x280000 - 0x440000;
                this->touchPosCallbacks[2].Set(&UISaveSlot::PrevZone);

                this->touchPosCount = 3;
                break;
        }
    }
}

void UISaveSlot::SetupAnimators()
{
    this->uiAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->playersAnimator.SetAnimation(sVars->aniFrames, 1, true, 3);
    this->shadowsAnimator.SetAnimation(sVars->aniFrames, 2, true, 3);
    this->livesAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
    this->iconBGAnimator.SetAnimation(sVars->aniFrames, 0, true, 2);
    this->saveStatusAnimator.SetAnimation(UIWidgets::sVars->textFrames, 2, true, 0);
    this->fuzzAnimator.SetAnimation(sVars->aniFrames, 7, true, 0);
    this->emeraldsAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
    this->zoneIconAnimator.SetAnimation(sVars->aniFrames, 5, true, 0);

    if (this->type == UISAVESLOT_NOSAVE)
        this->zoneNameAnimator.SetAnimation(UIWidgets::sVars->textFrames, 2, true, 2);
    else
        this->zoneNameAnimator.SetAnimation(sVars->aniFrames, 6, true, 0);

    this->numbersAnimator.SetAnimation(sVars->aniFrames, 8, true, 0);
}

void UISaveSlot::LoadSaveInfo()
{
    SaveGame::SaveRAM *saveRAM = (SaveGame::SaveRAM *)SaveGame::GetSaveDataPtr(this->slotID);

    int32 saveState = saveRAM->saveState;
    if (saveState == SaveGame::SaveActive || saveState == SaveGame::SaveCompleted) {
       this->frameID = saveRAM->characterID;
    }

    switch (saveState) {
        default: break;

        case SaveGame::SaveEmpty:
            this->saveZoneID   = Zone::OWZ;
            this->saveEmeralds = 0;
            this->saveLives    = 3;
            this->saveContinues = 0;
            this->frameID       = 0;
            this->isNewSave = true;
            this->listID    = 0;
            break;

        case SaveGame::SaveActive:
            this->saveZoneID   = saveRAM->zoneID;
            this->saveEmeralds = saveRAM->collectedEmeralds;
            this->saveLives    = saveRAM->lives;
            this->saveContinues = saveRAM->continues;
            this->isNewSave = false;
            this->listID    = 0;
            break;

        case SaveGame::SaveCompleted:
            this->saveZoneID   = NO_SAVE_SLOT;
            this->saveEmeralds = saveRAM->collectedEmeralds;
            this->saveLives    = saveRAM->lives;
            this->saveContinues = saveRAM->continues;
            this->listID    = 1;
            this->isNewSave = false;
            break;
    }

    UISaveSlot::SetupAnimators();
    UISaveSlot::HandleSaveIcons();
}
void UISaveSlot::HandleSaveIcons()
{
    this->saveStatusAnimator.frameID = this->listID;
    this->zoneIconAnimator.frameID   = this->saveZoneID;

    if (this->type == UISAVESLOT_REGULAR) {
        this->zoneNameAnimator.frameID = this->saveZoneID;

        if (this->currentlySelected) {
            paletteBank[0].Copy(((this->saveZoneID >> 3) + 1), 32 * this->saveZoneID, 224, 32);
        }
    }
}

void UISaveSlot::DeleteDLG_CB()
{
    UIDialog *dialog     = (UIDialog *)UIDialog::sVars->activeDialog;
    UISaveSlot *saveSlot = (UISaveSlot *)dialog->entityPtr;

    dialog->parent->state.Set(nullptr);

    UILoadingIcon::StartWait();

    SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveDataPtr(saveSlot->slotID % 8);
    // Bug Details(?):
    // sizeof(globals->noSaveSlot) and sizeof(saveData) is 4096 (sizeof(int32) * 0x400)
    // but the memset size is only 1024 (sizeof(uint8) * 0x400)
    // so only about 1/4th of the save slot is cleared, though nothin uses the extra space so it's not a big deal
    memset(saveRAM, 0, 0x400);

    SaveGame::SaveFile(UISaveSlot::DeleteSaveCB);
}

void UISaveSlot::DeleteSaveCB(bool32 success)
{
    UIDialog *dialog     = (UIDialog *)UIDialog::sVars->activeDialog;
    UISaveSlot *saveSlot = (UISaveSlot *)dialog->entityPtr;
    UILoadingIcon::FinishWait();
    Action<void> callback;
    callback.Set(nullptr);
    UIDialog::CloseOnSel_HandleSelection(dialog, callback);

    Entity *storeEntity = (Entity *)sceneInfo->entity;
    sceneInfo->entity   = (Entity *)saveSlot;

    saveSlot->LoadSaveInfo();
    saveSlot->HandleSaveIcons();
    sceneInfo->entity = storeEntity;
}

void UISaveSlot::ProcessButtonCB()
{
    UIControl *control = (UIControl *)this->parent;
    SaveGame::SaveRAM *saveRAM = (SaveGame::SaveRAM *)SaveGame::GetSaveDataPtr(this->slotID);

    this->active = ACTIVE_NORMAL;

    UIControl::SetTargetPos(control, this->position.x, 0);

    if (control->position.x == control->targetPos.x) {
        if (control->columnCount > 1) {
            if (UIControl::sVars->anyLeftPress) {
                if (control->buttonID > 0) {
                    control->buttonID--;
                    UIWidgets::sVars->sfxBleep.Play(false, 255);
                    UISaveSlot::HandleSaveIconChange();
                    return;
                }
            }
            else if (UIControl::sVars->anyRightPress) {
                if (control->buttonID < control->buttonCount - 1) {
                    control->buttonID++;
                    UIWidgets::sVars->sfxBleep.Play(false, 255);
                    UISaveSlot::HandleSaveIconChange();
                    return;
                }
            }
        }

        this->stateInput.Run(this);

        String msg = {};

        if (UIControl::sVars->anyConfirmPress) {
            if (this->frameID < 4) {
                UISaveSlot::SelectedCB();
            }
            else {
                Localization::GetString(&msg, Localization::FeatureUnimplemented);
                Action<void> callback;
                callback.Set(nullptr);
                UIDialog::CreateDialogOk(&msg, callback, true);
            }
        }
        else if (UIControl::sVars->anyXPress && saveRAM->saveState != SaveGame::SaveEmpty && this->type == UISAVESLOT_REGULAR) {
            Localization::GetString(&msg, Localization::Delete);
            Action<void> callbackYes;
            callbackYes.Set(&UISaveSlot::DeleteDLG_CB);
            Action<void> callbackNo;
            callbackNo.Set(nullptr);
            UIDialog *dialog = UIDialog::CreateDialogYesNo(&msg, callbackYes, callbackNo, false, true);
            if (dialog) {
                dialog->entityPtr = (Entity *)this;
            }
        }
        else if (!this->currentlySelected && control->buttons[control->lastButtonID] == (UIButton *)this && control->state.Matches(&UIControl::ProcessInputs)) {
            UISaveSlot::ButtonEnterCB();
        }
    }
}

void UISaveSlot::SelectedCB()
{
    UIControl *control = (UIControl *)this->parent;

    if (control->position.x == control->targetPos.x) {
        control->state.Set(nullptr);
        this->state.Set(&UISaveSlot::State_Selected);
        this->isSelected        = false;
        this->currentlySelected = false;
        this->processButtonCB.Set(nullptr);

        for (auto saveSlot : GameObject::GetEntities<UISaveSlot>(FOR_ALL_ENTITIES))
        {
            if (saveSlot != this) {
                if (saveSlot->position.x >= this->position.x) {
                    if (saveSlot->position.x > this->position.x)
                        saveSlot->velocity.x = 0x200000;
                }
                else {
                    saveSlot->velocity.x = -0x200000;
                }
                saveSlot->state.Set(&UISaveSlot::State_OtherWasSelected);
            }
        }

        for (auto prompt : GameObject::GetEntities<UIButtonPrompt>(FOR_ALL_ENTITIES)) { prompt->visible = false; }

        int32 id = Input::GetFilteredInputDeviceID(true, false, 5);
        Input::ResetInputSlotAssignments();
        Input::AssignInputSlotToDevice(Input::CONT_P1, id);

        UIWidgets::sVars->sfxAccept.Play(false, 255);
        Music::Stop();
    }
}

void UISaveSlot::NextCharacter()
{
    ++this->frameID;
    int32 player = this->frameID;

    int32 max = 4;
    while (player >= max) player -= max;

    this->frameID            = player;
    this->buttonBouncePos    = 0x8000;
    this->buttonBounceOffset = 0;

    UIWidgets::sVars->sfxBleep.Play(false, 255);

    UISaveSlot::HandleSaveIcons();
}

void UISaveSlot::PrevCharacter()
{
    --this->frameID;
    int32 player = this->frameID;

    int32 max = 4;
    while (player < 0) player += max;

    this->frameID            = player;
    this->buttonBouncePos    = 0x8000;
    this->buttonBounceOffset = 0;

    UIWidgets::sVars->sfxBleep.Play(false, 255);

    UISaveSlot::HandleSaveIcons();
}

void UISaveSlot::NextZone()
{
    if (this->saveZoneID == NO_SAVE_SLOT) {
        this->saveZoneID = Zone::OWZ;
    }
    else {
        this->saveZoneID++;
        if (this->saveZoneID > Zone::DEZ)
            this->saveZoneID = Zone::OWZ;
    }

    UIWidgets::sVars->sfxBleep.Play(false, 255);

    UISaveSlot::HandleSaveIcons();
}

void UISaveSlot::PrevZone()
{
    if (this->saveZoneID == NO_SAVE_SLOT) {
        this->saveZoneID = Zone::DEZ;
    }
    else {
        this->saveZoneID--;
        if (this->saveZoneID < Zone::OWZ)
            this->saveZoneID = Zone::DEZ;
    }

    UIWidgets::sVars->sfxBleep.Play(false, 255);

    UISaveSlot::HandleSaveIcons();
}

bool32 UISaveSlot::CheckButtonEnterCB()
{
    return this->isSelected;
}
bool32 UISaveSlot::CheckSelectedCB()
{
    return this->state.Matches(&UISaveSlot::State_Selected);
}

void UISaveSlot::ButtonEnterCB()
{
    if (!this->currentlySelected) {
        this->isSelected        = true;
        this->currentlySelected = true;

        UISaveSlot::HandleSaveIcons();

        this->textBouncePos   = 0x4000;
        this->buttonBouncePos = 0x8000;
        this->stateInput.Set(nullptr);

        if (!this->isNewSave && this->type != UISAVESLOT_NOSAVE) {
            if (this->listID)
                this->state.Set(&UISaveSlot::State_CompletedSave);
            else
                this->state.Set(&UISaveSlot::State_ActiveSave);
        }
        else {
            this->state.Set(&UISaveSlot::State_NewSave);
            this->stateInput.Set(&UISaveSlot::StateInput_NewSave);
        }
    }
}

void UISaveSlot::ButtonLeaveCB()
{
    this->isSelected        = false;
    this->currentlySelected = false;
    this->state.Set(&UISaveSlot::State_NotSelected);
    this->stateInput.Set(nullptr);
}

void UISaveSlot::HandleSaveIconChange()
{
    SaveGame::SaveRAM *saveRAM = (SaveGame::SaveRAM *)SaveGame::GetSaveDataPtr(this->slotID);

    if (saveRAM->saveState == SaveGame::SaveEmpty) {
        int32 frame = 0;
        if (this->frameID != frame) {
            this->frameID = frame;
            UISaveSlot::HandleSaveIcons();
        }
    }

    if (this->type == UISAVESLOT_NOSAVE && saveRAM->saveState == SaveGame::SaveCompleted) {
        this->saveZoneID = NO_SAVE_SLOT;
        UISaveSlot::HandleSaveIcons();
    }
}

void UISaveSlot::State_NotSelected()
{
    this->fuzzAnimator.Process();

    this->direction = this->fuzzAnimator.frameID & 3;
}

void UISaveSlot::State_OtherWasSelected()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
}

void UISaveSlot::State_NewSave() {}

void UISaveSlot::State_ActiveSave()
{
    if (!this->currentlySelected)
        this->state.Set(&UISaveSlot::State_NotSelected);
}

void UISaveSlot::StateInput_NewSave()
{
    UIControl *control = (UIControl *)this->parent;

    if (control->active == ACTIVE_ALWAYS) {
        if (UIControl::sVars->anyUpPress)
            UISaveSlot::NextCharacter();
        else if (UIControl::sVars->anyDownPress)
            UISaveSlot::PrevCharacter();
    }
}

void UISaveSlot::State_CompletedSave()
{
    UIControl *control = (UIControl *)this->parent;

    if (control->active == ACTIVE_ALWAYS) {
        if (UIControl::sVars->anyUpPress)
            UISaveSlot::NextZone();
        else if (UIControl::sVars->anyDownPress)
            UISaveSlot::PrevZone();
    }
}

void UISaveSlot::State_Selected()
{
    ++this->timer;

    if (this->timer == 32)
        UIWidgets::sVars->sfxWarp.Play(false, 255);
    
    if (this->timer > 32) {
        this->alpha += 0x20;
        this->fxRadius += 12;
    }
    
    if (this->fxRadius > 0x200) {
        this->actionCB.Run(this);
        this->state.Set(nullptr);
    }
}

#if RETRO_INCLUDE_EDITOR
void UISaveSlot::EditorDraw()
{
    this->drawGroup     = 2;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x300000;

    if (this->type == UISAVESLOT_REGULAR) {
        UISaveSlot::LoadSaveInfo();
    }
    else {
        this->slotID = NO_SAVE_SLOT;
        UISaveSlot::SetupAnimators();
    }

    this->textFrames = UIWidgets::sVars->textFrames;
    UISaveSlot::HandleSaveIcons();

    UISaveSlot::Draw();
}

void UISaveSlot::EditorLoad()
{
    sVars->aniFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Regular Save Slot", UISAVESLOT_REGULAR);
    RSDK_ENUM_VAR("No Save Slot", UISAVESLOT_NOSAVE);
}
#endif

void UISaveSlot::Serialize()
{
    RSDK_EDITABLE_VAR(UISaveSlot, VAR_BOOL, disabled);
    RSDK_EDITABLE_VAR(UISaveSlot, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(UISaveSlot, VAR_ENUM, slotID);
}
} // namespace GameLogic