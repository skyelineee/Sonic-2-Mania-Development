// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SaveMenu Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SaveMenu.hpp"
#include "ManiaModeMenu.hpp"
#include "UIControl.hpp"
#include "UILoadingIcon.hpp"
#include "UIWidgets.hpp"
#include "UISaveSlot.hpp"
#include "UITransition.hpp"
#include "Helpers/MenuParam.hpp"
#include "Helpers/GameProgress.hpp"
#include "Helpers/MathHelpers.hpp"
#include "Helpers/TimeAttackData.hpp"
#include "Global/SaveGame.hpp"
#include "Global/Zone.hpp"
#include "Common/BGSwitch.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SaveMenu);

void SaveMenu::Update() {}
void SaveMenu::LateUpdate() {}
void SaveMenu::StaticUpdate()
{
    UIControl *control = ManiaModeMenu::sVars->saveSelectMenu;

    if (control && control->active) {
        RSDKTable->GetTileLayer(1)->drawGroup[BGSwitch::sVars->screenID] = 0;
        RSDKTable->GetTileLayer(2)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
        RSDKTable->GetTileLayer(3)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
        RSDKTable->GetTileLayer(4)->drawGroup[BGSwitch::sVars->screenID] = 1;
        RSDKTable->GetTileLayer(5)->drawGroup[BGSwitch::sVars->screenID] = 1;
        RSDKTable->GetTileLayer(6)->drawGroup[BGSwitch::sVars->screenID] = 2;
        RSDKTable->GetTileLayer(7)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;

        UISaveSlot *saveSlot = (UISaveSlot *)control->buttons[control->lastButtonID];

        if (saveSlot) {
            int32 selectedID = saveSlot->saveSlotPlacement;

            for (int i = 0; i < control->buttonCount; ++i) {
                if (control->buttons[i]) {
                    UISaveSlot *saveSlot       = (UISaveSlot *)control->buttons[i];
                    UIControl *control         = (UIControl *)saveSlot->parent;
                    SaveGame::SaveRAM *saveRAM = (SaveGame::SaveRAM *)SaveGame::GetSaveDataPtr(saveSlot->slotID);
                    int32 saveState            = saveRAM->saveState;
                    if (saveSlot->saveSlotPlacement > selectedID) {
                        saveSlot->position.y = saveSlot->startPos.y + TO_FIXED(48);
                        saveSlot->fileAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 1, true, saveSlot->frameID);
                        saveSlot->saveStatusAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 2, true, saveSlot->saveStatusAnimator.frameID);
                        saveSlot->isSelected = false;
                        if (!saveSlot->isSelected)
                            if (saveState == SaveGame::SaveCompleted)
                                switch (saveSlot->fileAnimator.frameID) {
                                    case 0:
                                    case 1:
                                        saveSlot->saveStatusAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 3, true,
                                                                                  saveSlot->saveStatusAnimator.frameID);
                                        break;
                                    case 2:
                                        saveSlot->saveStatusAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 4, true,
                                                                                  saveSlot->saveStatusAnimator.frameID);
                                        break;
                                    case 3:
                                        saveSlot->saveStatusAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 5, true,
                                                                                  saveSlot->saveStatusAnimator.frameID);
                                        break;
                                }

                        switch (saveSlot->fileAnimator.frameID) {
                            case 0:
                            case 1:
                                saveSlot->zoneNameAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 6, true, saveSlot->zoneNameAnimator.frameID);
                                saveSlot->slotNumAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 14, true, saveSlot->slotNumAnimator.frameID);
                                break;
                            case 2:
                                saveSlot->zoneNameAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 7, true, saveSlot->zoneNameAnimator.frameID);
                                saveSlot->slotNumAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 15, true, saveSlot->slotNumAnimator.frameID);
                                break;
                            case 3:
                                saveSlot->zoneNameAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 8, true, saveSlot->zoneNameAnimator.frameID);
                                saveSlot->slotNumAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 16, true, saveSlot->slotNumAnimator.frameID);
                                break;
                        }
                    }
                    else if (saveSlot->saveSlotPlacement == selectedID) {
                        saveSlot->position.y = saveSlot->startPos.y;
                        saveSlot->fileAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 0, true, saveSlot->frameID);
                        saveSlot->isSelected = true;
                        switch (saveSlot->fileAnimator.frameID) {
                            case 0:
                            case 1:
                                saveSlot->saveStatusAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 3, true,
                                                                          saveSlot->saveStatusAnimator.frameID);
                                saveSlot->zoneNameAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 6, true, saveSlot->zoneNameAnimator.frameID);
                                saveSlot->slotNumAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 10, true, saveSlot->slotNumAnimator.frameID);
                                break;
                            case 2:
                                saveSlot->saveStatusAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 4, true,
                                                                          saveSlot->saveStatusAnimator.frameID);
                                saveSlot->zoneNameAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 7, true, saveSlot->zoneNameAnimator.frameID);
                                saveSlot->slotNumAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 11, true, saveSlot->slotNumAnimator.frameID);
                                break;
                            case 3:
                                saveSlot->saveStatusAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 5, true,
                                                                          saveSlot->saveStatusAnimator.frameID);
                                saveSlot->zoneNameAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 8, true, saveSlot->zoneNameAnimator.frameID);
                                saveSlot->slotNumAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 12, true, saveSlot->slotNumAnimator.frameID);
                                break;
                        }
                        // big boy
                    }
                    else {
                        saveSlot->position.y = saveSlot->startPos.y;
                        saveSlot->fileAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 1, true, saveSlot->frameID);
                        saveSlot->saveStatusAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 2, true, saveSlot->saveStatusAnimator.frameID);
                        saveSlot->isSelected = false;
                        if (!saveSlot->isSelected)
                            if (saveState == SaveGame::SaveCompleted)
                                switch (saveSlot->fileAnimator.frameID) {
                                    case 0:
                                    case 1:
                                        saveSlot->saveStatusAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 3, true,
                                                                                  saveSlot->saveStatusAnimator.frameID);
                                        break;
                                    case 2:
                                        saveSlot->saveStatusAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 4, true,
                                                                                  saveSlot->saveStatusAnimator.frameID);
                                        break;
                                    case 3:
                                        saveSlot->saveStatusAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 5, true,
                                                                                  saveSlot->saveStatusAnimator.frameID);
                                        break;
                                }

                        switch (saveSlot->fileAnimator.frameID) {
                            case 0:
                            case 1:
                                saveSlot->zoneNameAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 6, true, saveSlot->zoneNameAnimator.frameID);
                                saveSlot->slotNumAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 14, true, saveSlot->slotNumAnimator.frameID);
                                break;
                            case 2:
                                saveSlot->zoneNameAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 7, true, saveSlot->zoneNameAnimator.frameID);
                                saveSlot->slotNumAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 15, true, saveSlot->slotNumAnimator.frameID);
                                break;
                            case 3:
                                saveSlot->zoneNameAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 8, true, saveSlot->zoneNameAnimator.frameID);
                                saveSlot->slotNumAnimator.SetAnimation(UISaveSlot::sVars->aniFrames, 16, true, saveSlot->slotNumAnimator.frameID);
                                break;
                        }
                    }
                    switch (selectedID) {
                        case 0:
                        case 1: UIControl::SetTargetPos(control, saveSlot->position.x, saveSlot->position.y + TO_FIXED(84)); break;
                    }
                }
            }
        }
    }
    UIControl *controlSecrets = ManiaModeMenu::sVars->secretsMenu;

    if (controlSecrets && controlSecrets->active) {
        UIButton *button = controlSecrets->buttons[controlSecrets->lastButtonID];

        if (button) {
            int32 selectedID = button->buttonFrameID;

            for (int i = 0; i < controlSecrets->buttonCount; ++i) {
                if (controlSecrets->buttons[i]) {
                    UIButton *button = controlSecrets->buttons[i];
                    if (button->buttonFrameID > selectedID) {
                        button->position.y   = button->startPos.y + TO_FIXED(48);
                        button->buttonListID = 6;
                    }
                    else if (button->buttonFrameID == selectedID) {
                        button->position.y   = button->startPos.y;
                        button->buttonListID = 5;
                        // big boy
                    }
                    else {
                        button->position.y   = button->startPos.y;
                        button->buttonListID = 6;
                    }
                }
            }
        }
    }
}

void SaveMenu::Draw() {}

void SaveMenu::Create(void *data) {}

void SaveMenu::StageLoad() {}

void SaveMenu::Initialize()
{
    String tag = {};

    for (auto control : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES)) {
        tag.Set("Save Select");
        if (tag.Compare(&tag, &control->tag, false))
            ManiaModeMenu::sVars->saveSelectMenu = control;

        tag.Set("No Save Mode");
        if (tag.Compare(&tag, &control->tag, false))
            ManiaModeMenu::sVars->noSaveMenu = control;

        tag.Set("Secrets");
        if (tag.Compare(&tag, &control->tag, false))
            ManiaModeMenu::sVars->secretsMenu = control;
    }
}

void SaveMenu::HandleUnlocks()
{
    UIControl *control  = ManiaModeMenu::sVars->secretsMenu;
    UIButton *debugMode = control->buttons[1];

    UIButton *sonicAbility      = control->buttons[2];
    UIButton *maxControlChoice  = UIButton::GetChoicePtr(sonicAbility, 0);
    UIButton *peeloutChoice     = UIButton::GetChoicePtr(sonicAbility, 2);
    UIButton *instaShieldChoice = UIButton::GetChoicePtr(sonicAbility, 3);

    UIButton *andKnux = control->buttons[3];
}

void SaveMenu::SetupActions()
{
    for (auto slot : GameObject::GetEntities<UISaveSlot>(FOR_ALL_ENTITIES)) {
        slot->actionCB.Set(&SaveMenu::SaveButton_ActionCB);
    }

    for (auto prompt : GameObject::GetEntities<UIButtonPrompt>(FOR_ALL_ENTITIES)) {
        Hitbox hitbox;
        UIControl *saveSel = ManiaModeMenu::sVars->saveSelectMenu;
        int32 x            = saveSel->startPos.x - saveSel->cameraOffset.x;
        int32 y            = saveSel->startPos.y - saveSel->cameraOffset.y;
        hitbox.right       = saveSel->size.x >> 17;
        hitbox.left        = -(saveSel->size.x >> 17);
        hitbox.bottom      = saveSel->size.y >> 17;
        hitbox.top         = -(saveSel->size.y >> 17);

        if (MathHelpers::PointInHitbox(x, y, prompt->position.x, prompt->position.y, FLIP_NONE, &hitbox) && prompt->buttonID == 2) {
            ManiaModeMenu::sVars->delSavePrompt = prompt;
        }
    }

    UIControl *saveSel = ManiaModeMenu::sVars->saveSelectMenu;
    saveSel->menuUpdateCB.Set(&SaveMenu::SaveSel_MenuUpdateCB);
    saveSel->yPressCB.Set(&SaveMenu::SaveSel_YPressCB);

    saveSel->processButtonInputCB.Set(&ManiaModeMenu::MovePromptCB);
    saveSel->promptOffset = 12;
}

void SaveMenu::HandleMenuReturn(int32 slot)
{
    UIControl *control          = ManiaModeMenu::sVars->secretsMenu;
    SaveGame::SaveRAM *saveGame = (SaveGame::SaveRAM *)SaveGame::GetSaveDataPtr(slot);

    UIButton::SetChoiceSelection(control->buttons[0], (saveGame->medalMods & MEDAL_NOTIMEOVER) != 0);
    UIButton::SetChoiceSelection(control->buttons[1], (saveGame->medalMods & MEDAL_ANDKNUCKLES) != 0);

    if (saveGame->medalMods & MEDAL_NODROPDASH) {
        if (saveGame->medalMods & MEDAL_PEELOUT)
            UIButton::SetChoiceSelection(control->buttons[2], 1);
        else if (saveGame->medalMods & MEDAL_INSTASHIELD)
            UIButton::SetChoiceSelection(control->buttons[2], 2);
    }
    else {
        UIButton::SetChoiceSelection(control->buttons[2], 0);
    }

    if (saveGame->medalMods & MEDAL_ANDKNUCKLES)
        UIButton::SetChoiceSelection(control->buttons[3], 1);
    else
        UIButton::SetChoiceSelection(control->buttons[3], 0);
}

int32 SaveMenu::GetMedalMods()
{
    UIControl *control = ManiaModeMenu::sVars->secretsMenu;

    int32 mods = 0;
    if (control->buttons[0]->selection == 1)
        mods |= MEDAL_NOTIMEOVER;

    if (control->buttons[1]->selection == 1)
        mods |= MEDAL_DEBUGMODE;

    if (control->buttons[2]->selection == 0) {
        mods |= MEDAL_PEELOUT;
        mods |= MEDAL_INSTASHIELD;
    }
    else if (control->buttons[2]->selection == 2) {
        mods |= MEDAL_NODROPDASH;
        mods |= MEDAL_PEELOUT;
    }
    else if (control->buttons[2]->selection == 3) {
        mods |= MEDAL_NODROPDASH;
        mods |= MEDAL_INSTASHIELD;
    }

    if (control->buttons[3]->selection == 1)
        mods |= MEDAL_ANDKNUCKLES;

    return mods;
}

void SaveMenu::SaveFileCB(bool32 success)
{
    UILoadingIcon::FinishWait();

    Stage::LoadScene();
}

void SaveMenu::SecretsTransitionCB()
{
    UIControl *control     = ManiaModeMenu::sVars->saveSelectMenu;
    control->childHasFocus = true;

    UIControl::MatchMenuTag("Secrets");
}

void SaveMenu::LeaveSecretsMenu()
{
    UIControl *control = ManiaModeMenu::sVars->saveSelectMenu;
    if (ManiaModeMenu::sVars->inSecretsMenu) {
        UISaveSlot *slot = (UISaveSlot *)control->buttons[control->lastButtonID];

        SaveMenu::HandleMenuReturn(slot->slotID);
        ManiaModeMenu::sVars->inSecretsMenu = false;
    }
}

void SaveMenu::SaveSel_MenuUpdateCB()
{
    UIControl *control = (UIControl *)this;

    if (this->active == ACTIVE_ALWAYS) {
        UIButtonPrompt *prompt = ManiaModeMenu::sVars->delSavePrompt;

        if (control->lastButtonID != ManiaModeMenu::sVars->saveSelLastButtonID) {
            SaveMenu::LeaveSecretsMenu();
            ManiaModeMenu::sVars->saveSelLastButtonID = control->lastButtonID;
        }

        bool32 canDeleteSave = false;
        bool32 showPrompt    = false;
        for (int32 i = 0; i < control->buttonCount; ++i) {
            showPrompt |= control->buttons[i]->state.Matches(&UISaveSlot::State_Selected);

            if (control->lastButtonID >= 0) {
                if (control->buttons[i] == control->buttons[control->lastButtonID]) {
                    UISaveSlot *slot = (UISaveSlot *)control->buttons[control->lastButtonID];
                    if (!slot->isNewSave)
                        canDeleteSave = true;
                }
            }
        }

        if (!showPrompt) {
            if ((control == ManiaModeMenu::sVars->saveSelectMenu && control->lastButtonID == 8)) {
                prompt->visible = false;
            }
            else {
                prompt->visible = canDeleteSave;
            }
        }
    }
}

void SaveMenu::SaveSel_YPressCB()
{
    UIControl *control = ManiaModeMenu::sVars->saveSelectMenu;

    if (control->active == ACTIVE_ALWAYS) {
        if (!ManiaModeMenu::sVars->inSecretsMenu) {
            SaveMenu::HandleMenuReturn(((UISaveSlot *)control->buttons[control->buttonID])->slotID);
            ManiaModeMenu::sVars->inSecretsMenu = true;
        }

        UIWidgets::sVars->sfxAccept.Play(false, 255);
        UIControl::sVars->inputLocked = true;

        UITransition::StartTransition(SaveMenu::SecretsTransitionCB, 0);
    }
}

void SaveMenu::SaveButton_ActionCB()
{
    UISaveSlot *saveSlot = (UISaveSlot *)this;

    MenuParam *param   = (MenuParam *)globals->menuParam;
    UIControl *control = (UIControl *)saveSlot->parent;

    SaveGame::SaveRAM *saveGame = (SaveGame::SaveRAM *)SaveGame::GetSaveDataPtr(saveSlot->slotID);
    TimeAttackData::Clear();

    control->tag.CStr(param->menuTag);
    param->menuSelection = control->lastButtonID;
    param->replayID      = 0;

    bool32 loadingSave = false;
    if (saveSlot->type) {
        // Bug Details(?):
        // sizeof(globals->noSaveSlot) and sizeof(saveData) is 4096 (sizeof(int32) * 0x400)
        // but the memset size is only 1024 (sizeof(uint8) * 0x400)
        // so only about 1/4th of the save slot is cleared, though nothin uses the extra space so it's not a big deal

        memset(globals->noSaveSlot, 0, 0x400);
        globals->continues  = 0;
        globals->saveSlotID = NO_SAVE_SLOT;
    }
    else {
        globals->saveSlotID = saveSlot->slotID;
        globals->medalMods  = 0;

        if (saveSlot->isNewSave) {
            SaveGame::SaveRAM *saveData = SaveGame::GetSaveDataPtr(saveSlot->slotID % 8);

            // Bug Details(?):
            // see above

            memset(saveData, 0, 0x400);
            saveGame->saveState = 1;

            saveGame->characterID       = saveSlot->frameID;
            saveGame->zoneID            = 0;
            saveGame->lives             = 3;
            saveGame->collectedEmeralds = saveSlot->saveEmeralds;
            saveGame->continues         = 0;
            UILoadingIcon::StartWait();
            loadingSave = true;
            SaveGame::SaveFile(SaveMenu::SaveFileCB);
        }
        else {
            if (saveGame->saveState == 2) {
                saveGame->collectedSpecialRings = 0;
                saveGame->score                 = 0;
                saveGame->score1UP              = 500000;
            }

            loadingSave = true;
            SaveGame::SaveFile(SaveMenu::SaveFileCB);
        }
    }

    globals->medalMods  = SaveMenu::GetMedalMods();
    saveGame->medalMods = globals->medalMods;

    switch (saveSlot->frameID) {
        case 0: // Sonic & Tails
        case 1: globals->playerID = ID_SONIC; break;
        case 2: globals->playerID = ID_TAILS; break;
        case 3: globals->playerID = ID_KNUCKLES; break;
        default: break;
    }

    if ((globals->medalMods & MEDAL_ANDKNUCKLES))
        globals->playerID |= ID_KNUCKLES_ASSIST;
    else if (!saveSlot->frameID)
        globals->playerID |= ID_TAILS_ASSIST;

    if (saveSlot->type == UISaveSlot::UISAVESLOT_NOSAVE || saveSlot->isNewSave) {
        if (((globals->medalMods & MEDAL_DEBUGMODE) && (controllerInfo->keyC.down || controllerInfo->keyX.down))
            && saveSlot->type == UISaveSlot::UISAVESLOT_NOSAVE) {
            Stage::SetScene("Presentation", "Level Select");
        }
        else {
            Stage::SetScene("Adventure Mode", "Ocean Wind Zone");
        }
    }
    else {
        Stage::SetScene("Adventure Mode", "");
        sceneInfo->listPos += Zone::GetZoneListPos(saveSlot->saveZoneID, Zone::Act1, saveSlot->frameID);
    }

    if (!loadingSave) {
        globals->initCoolBonus = false;
        Stage::LoadScene();
    }
}

#if RETRO_INCLUDE_EDITOR
void SaveMenu::EditorDraw() {}

void SaveMenu::EditorLoad() {}
#endif

void SaveMenu::Serialize() {}
} // namespace GameLogic