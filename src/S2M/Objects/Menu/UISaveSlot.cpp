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

    if (this->isSelected) {
        this->buttonBounceVelocity += 0x4800;
        this->buttonBounceOffset += this->buttonBounceVelocity;

        if (this->buttonBounceOffset >= -0x20000 && this->buttonBounceVelocity > 0) {
            this->buttonBounceOffset   = 0;
            this->buttonBounceVelocity = 0;
        }
    }
    else if (!this->state.Matches(&UISaveSlot::State_Selected)) {
        if (this->buttonBounceOffset) {
            int32 offset = -(this->buttonBounceOffset / abs(this->buttonBounceOffset));
            this->buttonBounceOffset += offset << 16;

            if (offset < 0 && this->buttonBounceOffset < 0)
                this->buttonBounceOffset = 0;
            else if (offset > 0 && this->buttonBounceOffset > 0)
                this->buttonBounceOffset = 0;
        }
    }

    if (this->currentlySelected && (control->buttons[control->lastButtonID] != (UIButton *)this || control->buttonID == -1)) {
        this->currentlySelected = false;
        this->state.Set(&UISaveSlot::State_NotSelected);
        this->stateInput.Set(nullptr);
    }
    if (!control->state.Matches(&UIControl::ProcessInputs) || control->active != ACTIVE_ALWAYS) {
        if (this->currentlySelected) {
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

    if (this->isSelected) {
        drawPos.x = TO_FIXED(25);
        drawPos.y = TO_FIXED(148);
        UISaveSlot::DrawPlayerInfo(drawPos.x, drawPos.y);
        for (int32 e = 0; e < 7; ++e) {
            if (GET_BIT(this->saveEmeralds, e)) {
                this->emeraldsAnimator.frameID = e;
                drawPos.x                      = TO_FIXED(25);
                drawPos.y                      = TO_FIXED(178);
                this->emeraldsAnimator.DrawSprite(&drawPos, true);
            }
            else if (this->saveEmeralds == 0) {
                this->emeraldsAnimator.frameID = 7;
                drawPos.x                      = TO_FIXED(25);
                drawPos.y                      = TO_FIXED(178);
                this->emeraldsAnimator.DrawSprite(&drawPos, true);
            }
        }
    }

    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    this->slotNumAnimator.frameID = this->saveSlotPlacement;

    if (this->type == UISAVESLOT_REGULAR) {
        if (this->isNewSave) {
            DrawFileIcons(drawPos.x, drawPos.y);
        }
        else {
            if (this->type == UISAVESLOT_NOSAVE) {
                if (!this->saveContinues) {
                }
            }
            else {
                DrawFileIcons(drawPos.x, drawPos.y);
            }
        }

        if (this->isSelected) {
            this->saveStatusAnimator.frameID = 0;
        }
        else {
            this->saveStatusAnimator.frameID = 1;
        }                             
        
        SaveGame::SaveRAM *saveRAM = (SaveGame::SaveRAM *)SaveGame::GetSaveDataPtr(this->slotID);

        int32 saveState = saveRAM->saveState;

        if (saveState == SaveGame::SaveCompleted)
            if (this->isSelected) {
                this->saveStatusAnimator.frameID = 5;
            }
            else {
                this->saveStatusAnimator.frameID = 4;
            }
        
        if (!this->isSelected && this->isNewSave)
            this->slotNumAnimator.SetAnimation(sVars->aniFrames, 13, true, this->slotNumAnimator.frameID);

        drawPos.x = this->buttonBounceOffset + this->position.x;
        drawPos.y = this->position.y;

        this->slotNumAnimator.DrawSprite(&drawPos, false);

        if (this->isNewSave || this->saveZoneID == 0xFF) {
            drawPos.x = this->buttonBounceOffset + this->position.x;
            drawPos.y = this->position.y;
            this->saveStatusAnimator.DrawSprite(&drawPos, false);
        }
        else if ((this->isSelected || this->state.Matches(&UISaveSlot::State_Selected) && this->saveZoneID <= Zone::ZoneCountSaveFile)) {
            // this->zoneIconAnimator.DrawSprite(&drawPos, false);
            drawPos.x = this->buttonBounceOffset + this->position.x + TO_FIXED(20);
            drawPos.y = this->position.y + TO_FIXED(15);
            this->zoneNameAnimator.DrawSprite(&drawPos, false);
        }
        else {
            this->drawFX = FX_NONE;
            drawPos.x    = this->buttonBounceOffset + this->position.x + TO_FIXED(50);
            drawPos.y    = this->position.y + TO_FIXED(8);
            this->zoneNameAnimator.DrawSprite(&drawPos, false);
        }
    }
    else {
        DrawFileIcons(this->position.x, this->position.y);
        this->saveStatusAnimator.frameID = 3;

        drawPos.x = this->buttonBounceOffset + this->position.x;
        if (!this->isSelected) {
            this->saveStatusAnimator.frameID = 2;
            this->saveStatusAnimator.DrawSprite(&drawPos, false);
        }
        else {
            drawPos.y = this->position.y + TO_FIXED(8);
            this->saveStatusAnimator.DrawSprite(&drawPos, false);
        }
        drawPos.y = this->position.y;
        if (!this->isSelected)
            this->slotNumAnimator.SetAnimation(sVars->aniFrames, 13, true, this->slotNumAnimator.frameID);
        
        this->slotNumAnimator.DrawSprite(&drawPos, false);
    }

    if (this->isSelected || this->type == UISAVESLOT_REGULAR) {
        if (this->isSelected && !(this->zoneIconSprX & 8)) {
            drawPos.x = this->buttonBounceOffset + this->position.x;
            drawPos.y = this->position.y;
            if (this->type == UISAVESLOT_NOSAVE) {
                drawPos.x += TO_FIXED(65);
                drawPos.y += TO_FIXED(30);
                UIWidgets::DrawLeftRightArrows(drawPos.x, drawPos.y, TO_FIXED(120));
            }
            else if (this->isNewSave) {
                drawPos.x += TO_FIXED(75);
                drawPos.y += TO_FIXED(30);
                UIWidgets::DrawLeftRightArrows(drawPos.x, drawPos.y, TO_FIXED(140));
            }
            else if (this->listID == 1) {
                drawPos.x += TO_FIXED(85);
                drawPos.y += TO_FIXED(30);
                UIWidgets::DrawLeftRightArrows(drawPos.x, drawPos.y, TO_FIXED(160));
            }
        }
    }

    Graphics::DrawCircle(screenInfo->center.x, screenInfo->center.y, this->fxRadius, 0x000000, this->alpha, INK_ALPHA, true);
}

void UISaveSlot::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->visible         = true;
        this->drawGroup       = 6;
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
        this->startPos = this->position;
    }
}

void UISaveSlot::StageLoad() { sVars->aniFrames.Load("UI/SaveSelectNEW.bin", SCOPE_STAGE); }

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

void UISaveSlot::DrawFileIcons(int32 drawX, int32 drawY)
{
    Vector2 drawPos;

    this->fileAnimator.frameID = 0;

    int32 frames[]                = { 0, 1, 2, 3, 4 };

    if (this->isNewSave || this->type == UISAVESLOT_NOSAVE) {
        this->fileAnimator.frameID = 4;
        if (this->isSelected)
            this->fileAnimator.frameID = frames[this->frameID];
    }
    else {
        this->fileAnimator.frameID = frames[this->frameID];
    }

    drawPos.x = this->buttonBounceOffset + drawX;
    drawPos.y = drawY;
    this->fileAnimator.DrawSprite(&drawPos, false);
}

void UISaveSlot::DrawPlayerInfo(int32 drawX, int32 drawY)
{
    this->livesAnimator.SetAnimation(sVars->aniFrames, 19, true, 0);
    this->continuesAnimator.SetAnimation(sVars->aniFrames, 20, true, 0);

    int32 playerID = 0;

    int32 frames[] = { 0, 1, 2, 3, 4 };
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
        positions[0].x = drawX + 0x320000;
    }
    else {
        positions[0].x = drawX - TO_FIXED(2);
        positions[1].x = drawX + 0x650000;
        count          = 2;
    }

    for (int32 i = 0; i < count; ++i) {
        Vector2 drawPos;

        drawPos.x = positions[i].x;
        drawPos.y = positions[i].y;
        if (true) {
            drawPos.x -= 0xC0000;
            animators[i]->DrawSprite(&drawPos, true);

            if (this->saveLives >= 10) {
                drawPos.x += 0x300000;
                this->numbersAnimator.frameID = retries[i] / 10;
                this->numbersAnimator.DrawSprite(&drawPos, true);

                drawPos.x += TO_FIXED(15);
                this->numbersAnimator.frameID = retries[i] % 10;
                this->numbersAnimator.DrawSprite(&drawPos, true);
            }
            else {
                drawPos.x += 0x300000;
                if (this->type == UISAVESLOT_NOSAVE) {
                    this->numbersAnimator.frameID = 3;
                }
                else {
                    this->numbersAnimator.frameID = retries[i] % 10;
                }
                this->numbersAnimator.DrawSprite(&drawPos, true);
            }

            this->uiAnimator.frameID = 0;
            drawPos.x                = positions[i].x + TO_FIXED(19);
            this->uiAnimator.DrawSprite(&drawPos, true);
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
    this->uiAnimator.SetAnimation(sVars->aniFrames, 17, true, 0);
    this->fileAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    this->livesAnimator.SetAnimation(sVars->aniFrames, 19, true, 0);
    this->saveStatusAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    this->emeraldsAnimator.SetAnimation(sVars->aniFrames, 18, true, 0);

    if (this->type == UISAVESLOT_NOSAVE)
        this->zoneNameAnimator.SetAnimation(sVars->aniFrames, 17, true, 1);
    else
        this->zoneNameAnimator.SetAnimation(sVars->aniFrames, 6, true, 0);

    this->numbersAnimator.SetAnimation(sVars->aniFrames, 21, true, 0);
    this->slotNumAnimator.SetAnimation(sVars->aniFrames, 13, true, 0);
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
            this->frameID       = 0;
            this->saveZoneID   = Zone::OWZ;
            this->saveEmeralds = 0;
            this->saveLives    = 3;
            this->saveContinues = 0;
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

    UIControl::SetTargetPos(control, this->position.x, this->position.y + TO_FIXED(48));

    if (control->position.x == control->targetPos.x) {
        if (control->rowCount > 1) {
            if (UIControl::sVars->anyDownPress) {
                if (control->buttonID > 0) {
                    control->buttonID--;
                    UIWidgets::sVars->sfxBleep.Play(false, 255);
                    UISaveSlot::HandleSaveIconChange();
                    return;
                }
            }
            else if (UIControl::sVars->anyUpPress) {
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
    this->buttonBounceOffset   = 0;
    this->buttonBounceVelocity = -0x25000;

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
    this->buttonBounceOffset   = 0;
    this->buttonBounceVelocity = -0x25000;

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
        this->currentlySelected = true;

        UISaveSlot::HandleSaveIcons();

        this->buttonBounceOffset   = 0;
        this->buttonBounceVelocity = -0x25000;
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

void UISaveSlot::State_NotSelected() {}

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
        if (UIControl::sVars->anyRightPress)
            UISaveSlot::NextCharacter();
        else if (UIControl::sVars->anyLeftPress)
            UISaveSlot::PrevCharacter();
    }
}

void UISaveSlot::State_CompletedSave()
{
    UIControl *control = (UIControl *)this->parent;

    if (control->active == ACTIVE_ALWAYS) {
        if (UIControl::sVars->anyRightPress)
            UISaveSlot::NextZone();
        else if (UIControl::sVars->anyLeftPress)
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
    sVars->aniFrames.Load("UI/SaveSelectNEW.bin", SCOPE_STAGE);

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
    RSDK_EDITABLE_VAR(UISaveSlot, VAR_ENUM, saveSlotPlacement);
}
} // namespace GameLogic