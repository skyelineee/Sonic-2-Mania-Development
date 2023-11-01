// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TimeAttackMenu Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "TimeAttackMenu.hpp"
#include "UITAZoneModule.hpp"
#include "UIButton.hpp"
#include "UIChoice.hpp"
#include "UICharButton.hpp"
#include "UIRankButton.hpp"
#include "UILoadingIcon.hpp"
#include "UIWidgets.hpp"
#include "UITransition.hpp"
#include "UIPopover.hpp"
#include "MenuSetup.hpp"
#include "Helpers/GameProgress.hpp"
#include "Helpers/MenuParam.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Helpers/TimeAttackData.hpp"
#include "Helpers/ReplayDB.hpp"
#include "Helpers/RPCHelpers.hpp"
#include "Global/Localization.hpp"
#include "Global/ReplayRecorder.hpp"
#include "Global/Music.hpp"
#include "Global/SaveGame.hpp"
#include "Common/BGSwitch.hpp"
#include "ManiaModeMenu.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TimeAttackMenu);

void TimeAttackMenu::Update() { this->state.Run(this); }

void TimeAttackMenu::LateUpdate() {}

void TimeAttackMenu::StaticUpdate()
{
    UIButtonPrompt *switchModePrompt = sVars->switchModePrompt;

    if (switchModePrompt) {
        switchModePrompt->visible = false;
    }

    UIControl *control = sVars->taZoneSelControl;

    if (control && control->active) {
        UITAZoneModule *module = (UITAZoneModule *)control->buttons[control->lastButtonID];

        if (module) {
            int32 selectedID = module->zoneID;

            for (int i = 0; i < control->buttonCount; ++i) {
                if (control->buttons[i]) {
                    UITAZoneModule *module = (UITAZoneModule *)control->buttons[i];
                    if (module->zoneID > selectedID) {
                        module->position.y = module->startPos.y + TO_FIXED(48);
                        module->buttonAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, 1, true, module->buttonAnimator.frameID);
                        module->descriptionListID = 5;
                        module->isSelected        = false;
                    }
                    else if (module->zoneID == selectedID) {
                        module->buttonAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, 0, true, module->buttonAnimator.frameID);
                        module->position.y        = module->startPos.y;
                        module->descriptionListID = 4;
                        module->isSelected        = true;
                        // big boy
                    }
                    else {
                        module->buttonAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, 1, true, module->buttonAnimator.frameID);
                        module->position.y        = module->startPos.y;
                        module->descriptionListID = 5;
                        module->isSelected        = false;
                    }
                }
            }
        }
    }

    UIControl *replayControl = sVars->replaysControl;
    if (replayControl && replayControl->active) {
        RSDKTable->GetTileLayer(1)->drawGroup[BGSwitch::sVars->screenID] = 0;
        RSDKTable->GetTileLayer(2)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
        RSDKTable->GetTileLayer(3)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
        RSDKTable->GetTileLayer(4)->drawGroup[BGSwitch::sVars->screenID] = 1;
        RSDKTable->GetTileLayer(5)->drawGroup[BGSwitch::sVars->screenID] = 1;
        RSDKTable->GetTileLayer(6)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
        RSDKTable->GetTileLayer(7)->drawGroup[BGSwitch::sVars->screenID] = 2;
    }

    UIControl *detailsControl = sVars->taDetailsControl;
    if (detailsControl && detailsControl->active) {
        RSDKTable->GetTileLayer(1)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
        RSDKTable->GetTileLayer(2)->drawGroup[BGSwitch::sVars->screenID] = 0;
        RSDKTable->GetTileLayer(3)->drawGroup[BGSwitch::sVars->screenID] = 1;
        RSDKTable->GetTileLayer(4)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
        RSDKTable->GetTileLayer(5)->drawGroup[BGSwitch::sVars->screenID] = 1;
        RSDKTable->GetTileLayer(6)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
        RSDKTable->GetTileLayer(7)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
    }
}

void TimeAttackMenu::Draw() {}

void TimeAttackMenu::Create(void *data) {}

void TimeAttackMenu::StageLoad()
{
    sVars->prevIsUser = false;
    sVars->isUser     = false;
}

void TimeAttackMenu::Initialize()
{
    String tag = {};

    for (auto control : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES)) {
        tag.Set("Time Attack");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->timeAttackControl = control;

        tag.Set("Time Attack Zones");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->taZoneSelControl = control;

        tag.Set("Time Attack Detail");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->taDetailsControl = control;

        tag.Set("Replays");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->replaysControl = control;
    }

    UIControl *zoneControl    = sVars->taZoneSelControl;
    UIControl *replayControl  = sVars->replaysControl;
    UIControl *detailsControl = sVars->taDetailsControl;

    for (auto prompt : GameObject::GetEntities<UIButtonPrompt>(FOR_ALL_ENTITIES)) {
        if (UIControl::ContainsPos(zoneControl, &prompt->position) && prompt->buttonID == 3)
            sVars->switchModePrompt = prompt;

        if (UIControl::ContainsPos(replayControl, &prompt->position) && prompt->buttonID == 2)
            sVars->replayPrompt = prompt;
    }

    for (auto carousel : GameObject::GetEntities<UIReplayCarousel>(FOR_ALL_ENTITIES)) {
        sVars->replayCarousel = carousel;
    }

    for (auto banner : GameObject::GetEntities<UITABanner>(FOR_ALL_ENTITIES)) {
        if (UIControl::ContainsPos(detailsControl, &banner->position)) {
            sVars->detailsBanner = banner;
            banner->parent       = sVars->taDetailsControl;
        }
    }
}

void TimeAttackMenu::HandleUnlocks() {}

void TimeAttackMenu::SetupActions()
{
    UIControl *control        = sVars->timeAttackControl;
    UIControl *replayControl  = sVars->replaysControl;
    UIControl *zoneSelControl = sVars->taZoneSelControl;
    UIControl *detailsControl = sVars->taDetailsControl;

    control->menuUpdateCB.Set(&TimeAttackMenu::MenuUpdateCB);

    UIButton *replaysButton = control->buttons[control->buttonCount - 1];
    replaysButton->actionCB.Set(&TimeAttackMenu::ReplayButton_ActionCB);

    int32 newCount = (control->buttonCount - 1) + control->columnCount;
    for (int32 i = control->buttonCount; i < newCount; ++i) {
        control->buttons[i] = control->buttons[i - 1];
    }
    control->buttonCount = newCount;

    for (auto charButton : GameObject::GetEntities<UICharButton>(FOR_ALL_ENTITIES)) {
        if (charButton->parent == (Entity *)sVars->timeAttackControl)
            charButton->actionCB.Set(&TimeAttackMenu::CharButton_ActionCB);
    }

    for (auto module : GameObject::GetEntities<UITAZoneModule>(FOR_ALL_ENTITIES)) {
        module->actionCB.Set(&TimeAttackMenu::TAModule_ActionCB);
    }

    replayControl->menuSetupCB.Set(&TimeAttackMenu::MenuSetupCB_Replay);
    replayControl->menuUpdateCB.Set(&TimeAttackMenu::MenuUpdateCB_Replay);
    replayControl->xPressCB.Set(&TimeAttackMenu::DeleteReplayActionCB);
    replayControl->yPressCB.Set(&TimeAttackMenu::YPressCB_Replay);
    replayControl->processButtonInputCB.Set(&ManiaModeMenu::MovePromptCB);
    replayControl->promptOffset = -10;

    zoneSelControl->yPressCB.Set(&TimeAttackMenu::YPressCB_ZoneSel);
    zoneSelControl->backPressCB.Set(&TimeAttackMenu::BackPressCB_ZoneSel);
    zoneSelControl->processButtonInputCB.Set(&ManiaModeMenu::MovePromptCB);
    replayControl->promptOffset = 8;

    detailsControl->yPressCB.Set(&TimeAttackMenu::YPressCB_Details);
    detailsControl->xPressCB.Set(&TimeAttackMenu::XPressCB_Details);
    detailsControl->menuSetupCB.Set(&TimeAttackMenu::MenuSetupCB_Details);

    UIButton *replayButton = detailsControl->buttons[0];
    // Actually a UIChoice not a TAZoneModule but it fits better with pre-plus this way
    replayButton->actionCB.Set(&TimeAttackMenu::TAZoneModule_ActionCB);
    replayButton->choiceChangeCB.Set(&TimeAttackMenu::TAZoneModule_ChoiceChangeCB);

    UIReplayCarousel *replayCarousel = sVars->replayCarousel;
    replayCarousel->actionCB.Set(&TimeAttackMenu::ReplayCarousel_ActionCB);
    replayControl->buttons[0]->choiceChangeCB.Set(&TimeAttackMenu::SortReplayChoiceCB);
}

void TimeAttackMenu::HandleMenuReturn()
{
    MenuParam *param = MenuParam::GetMenuParam();

    UIControl *control = sVars->timeAttackControl;
    if (param->inTimeAttack) {
        int32 charID                         = param->characterID - 1;
        control->buttonID                    = charID;
        control->buttons[charID]->isSelected = true;
    }

    UIControl *zoneControl = sVars->taZoneSelControl;
    if (param->inTimeAttack) {
        zoneControl->buttonID                           = param->zoneID;
        zoneControl->buttons[param->zoneID]->isSelected = true;
    }

    UIControl *detailsControl = sVars->taDetailsControl;
    if (param->inTimeAttack) {
        UITABanner::SetupDetails(sVars->detailsBanner, param->zoneID, param->actID, param->characterID);
        TimeAttackMenu::SetupDetailsView();

        UIButton::SetChoiceSelectionWithCB(detailsControl->buttons[0], param->actID);
        detailsControl->buttonID = param->menuSelection;

        TimeAttackMenu::TAZoneModule_ChoiceChangeCB();
    }

    UIControl *replayControl = sVars->replaysControl;
    if (replayControl->active == ACTIVE_ALWAYS) {
        replayControl->buttonID = 1; // Select Replay Carousel
        UIButton::SetChoiceSelectionWithCB(replayControl->buttons[0], param->selectedReplay & 0xFF);

        UIReplayCarousel *carousel = sVars->replayCarousel;
        int32 replayCount          = APITable->GetSortedUserDBRowCount(globals->replayTableID);
        int32 targetID             = APITable->GetUserDBRowByID(globals->replayTableID, param->replayUUID);

        int32 replayID = 0;
        for (; replayID < replayCount; ++replayID) {
            if (APITable->GetSortedUserDBRowID(globals->replayTableID, replayID) == targetID)
                break;
        }

        if (replayCount <= 0 || replayID >= replayCount)
            carousel->curReplayID = param->replayID;
        else
            carousel->curReplayID = replayID;
    }
}

void TimeAttackMenu::DeleteReplayActionCB()
{
    UIControl *replayControl   = sVars->replaysControl;
    UIReplayCarousel *carousel = sVars->replayCarousel;

    if (replayControl->buttonID == 1 && carousel->stateDraw.Matches(&UIReplayCarousel::Draw_Carousel)
        && APITable->GetSortedUserDBRowCount(globals->replayTableID)) {
        String string = {};

        Localization::GetString(&string, Localization::DeleteReplay);

        Action<void> callbackYes;
        callbackYes.Set(&TimeAttackMenu::ConfirmDeleteReplay_Yes_CB);

        Action<void> callbackNo;
        callbackNo.Set(nullptr);

        UIDialog::CreateDialogYesNo(&string, callbackYes, callbackNo, true, true);
    }
}

void TimeAttackMenu::ConfirmDeleteReplay_Yes_CB()
{
    UIReplayCarousel *carousel = sVars->replayCarousel;

    int32 row = APITable->GetSortedUserDBRowID(globals->replayTableID, carousel->curReplayID);
    ReplayDB::DeleteReplay(row, TimeAttackMenu::DeleteReplayCB, false);
}

void TimeAttackMenu::DeleteReplayCB(bool32 success)
{
    TimeAttackMenu::SortReplayChoiceCB();

    UIReplayCarousel *carousel = sVars->replayCarousel;

    int32 count = APITable->GetSortedUserDBRowCount(globals->replayTableID) - 1;
    if (carousel->curReplayID > count)
        carousel->curReplayID = count;
}

void TimeAttackMenu::ReplayCarousel_ActionCB()
{
    UIReplayCarousel *carousel = (UIReplayCarousel *)this;

    UIPopover *popover = UIPopover::CreatePopover();
    if (popover) {
        int32 y = carousel->position.y;
        if (!carousel->curReplayID)
            y += carousel->popoverPos;

        UIPopover::AddButton(popover, UIPopover::POPOVER_WATCH, TimeAttackMenu::WatchReplayActionCB_ReplaysMenu, false);
        UIPopover::AddButton(popover, UIPopover::POPOVER_CHALLENGE, TimeAttackMenu::ChallengeReplayActionCB_ReplaysMenu, false);
        UIPopover::AddButton(popover, UIPopover::POPOVER_DELETE, TimeAttackMenu::DeleteReplayActionCB, true);
        if (carousel->curReplayID == 0) {
            UIPopover::Setup(popover, carousel->position.x - TO_FIXED(7), y + TO_FIXED(40));
        }
        else {
            UIPopover::Setup(popover, carousel->position.x - TO_FIXED(7), y + TO_FIXED(28));
        }
    }
}

void TimeAttackMenu::WatchReplay(int32 row, bool32 showGhost)
{
    MenuParam *param = MenuParam::GetMenuParam();

    int32 id = Input::GetFilteredInputDeviceID(false, false, 0);
    Input::ResetInputSlotAssignments();
    Input::AssignInputSlotToDevice(Input::CONT_P1, id);

    uint32 uuid = APITable->GetUserDBRowUUID(globals->replayTableID, row);
    LogHelpers::Print("Go_Replay(%d, %d)", row, showGhost);
    LogHelpers::Print("uuid: %08X", uuid);

    int32 score       = 0;
    uint8 zoneID      = 0;
    uint8 act         = 0;
    uint8 characterID = 0;
    APITable->GetUserDBValue(globals->replayTableID, row, API::Storage::UserDB::UInt32, "score", &score);
    APITable->GetUserDBValue(globals->replayTableID, row, API::Storage::UserDB::UInt8, "zoneID", &zoneID);
    APITable->GetUserDBValue(globals->replayTableID, row, API::Storage::UserDB::UInt8, "act", &act);
    APITable->GetUserDBValue(globals->replayTableID, row, API::Storage::UserDB::UInt8, "characterID", &characterID);

    param->viewReplay  = true;
    param->showGhost   = showGhost;
    param->replayUUID  = uuid;
    param->zoneID      = zoneID;
    param->actID       = act;
    param->characterID = characterID;

    int32 replayID = 0;
    if (!showGhost) {
        if (!TimeAttackData::sVars->loaded || characterID != TimeAttackData::sVars->characterID || zoneID != TimeAttackData::sVars->zoneID
            || act != TimeAttackData::sVars->act) {
            TimeAttackData::ConfigureTableView(zoneID, act, characterID);
        }

        int32 entryCount = APITable->GetSortedUserDBRowCount(globals->taTableID);
        for (int32 rank = 1; rank < entryCount; ++rank) {
            if (uuid == (uint32)TimeAttackData::GetReplayID(zoneID, act, characterID, rank))
                break;

            ++replayID;
        }
    }
    param->selectedReplay = replayID;

    UIPopover *popover = UIPopover::sVars->activePopover;
    if (popover)
        popover->parent->selectionDisabled = true;

    UILoadingIcon::StartWait();

    char fileName[0x20];
    sprintf_s(fileName, (int32)sizeof(fileName), "Replay_%08X.bin", uuid);

    memset(globals->replayTempRBuffer, 0, sizeof(globals->replayTempRBuffer));
    memset(globals->replayReadBuffer, 0, sizeof(globals->replayReadBuffer));
    ReplayRecorder::Buffer_LoadFile(fileName, globals->replayTempRBuffer, TimeAttackMenu::ReplayLoad_CB);
}

void TimeAttackMenu::ReplayLoad_CB(bool32 success)
{
    UILoadingIcon::FinishWait();

    int32 strID = 0;
    if (success) {
        ReplayRecorder::Replay *replayPtr = (ReplayRecorder::Replay *)globals->replayTempRBuffer;

        if (replayPtr->header.version == GAME_VERSION) {
            LogHelpers::Print("WARNING: Replay Load OK");
            ReplayRecorder::Buffer_Unpack(globals->replayReadBuffer, globals->replayTempRBuffer);
            TimeAttackMenu::LoadScene_Fadeout();
            return;
        }

        strID = Localization::CannotLoadReplay;
    }
    else {
        strID = Localization::ErrorLoadingReplay;
    }

    String message = {};
    Localization::GetString(&message, strID);

    Action<void> callback;
    callback.Set(nullptr);

    UIDialog *dialog = UIDialog::CreateDialogOk(&message, callback, true);
    if (dialog) {
        UIPopover *popover = UIPopover::sVars->activePopover;
        if (popover)
            popover->parent->selectionDisabled = false;
    }
}

void TimeAttackMenu::WatchReplayActionCB_ReplaysMenu()
{
    MenuParam *param = MenuParam::GetMenuParam();

    UIControl *control         = sVars->replaysControl;
    UIReplayCarousel *carousel = sVars->replayCarousel;
    UIButton *button           = control->buttons[0];

    sprintf_s(param->menuTag, (int32)sizeof(param->menuTag), "Replays");
    param->replayRankID = button->selection;
    param->replayID     = carousel->curReplayID;

    int32 id = APITable->GetSortedUserDBRowID(globals->replayTableID, carousel->curReplayID);
    TimeAttackMenu::WatchReplay(id, false);
}

void TimeAttackMenu::ChallengeReplayActionCB_ReplaysMenu()
{
    MenuParam *param = MenuParam::GetMenuParam();

    UIControl *control         = sVars->replaysControl;
    UIReplayCarousel *carousel = sVars->replayCarousel;
    UIButton *button           = control->buttons[0];

    sprintf_s(param->menuTag, (int32)sizeof(param->menuTag), "Replays");
    param->replayRankID = button->selection;
    param->replayID     = carousel->curReplayID;

    int32 id = APITable->GetSortedUserDBRowID(globals->replayTableID, carousel->curReplayID);
    TimeAttackMenu::WatchReplay(id, true);
}

void TimeAttackMenu::WatchReplayCB_RanksMenu()
{
    MenuParam *param     = MenuParam::GetMenuParam();
    UIPopover *popover   = UIPopover::sVars->activePopover;
    UIRankButton *button = (UIRankButton *)popover->storedEntity;
    UIControl *parent    = (UIControl *)button->parent;

    int32 uuid = APITable->GetUserDBRowByID(globals->replayTableID, button->replayID);
    if (uuid != -1) {
        parent->tag.CStr(param->menuTag);
        param->menuSelection = parent->buttonID;
        param->inTimeAttack  = true;
        TimeAttackMenu::WatchReplay(uuid, false);
    }
}

void TimeAttackMenu::ChallengeReplayCB_RanksMenu()
{
    MenuParam *param     = MenuParam::GetMenuParam();
    UIPopover *popover   = UIPopover::sVars->activePopover;
    UIRankButton *button = (UIRankButton *)popover->storedEntity;
    UIControl *parent    = (UIControl *)button->parent;

    int32 uuid = APITable->GetUserDBRowByID(globals->replayTableID, button->replayID);
    if (uuid != -1) {
        parent->tag.CStr(param->menuTag);
        param->menuSelection = parent->buttonID;
        param->inTimeAttack  = true;
        TimeAttackMenu::WatchReplay(uuid, true);
    }
}

void TimeAttackMenu::LoadScene_Fadeout()
{
    Music::FadeOut(0.05);
    MenuSetup::StartTransition(TimeAttackMenu::LoadScene, 32);
}

void TimeAttackMenu::MenuSetupCB_Replay()
{
    if (APITable->GetUserDBRowsChanged(globals->replayTableID))
        TimeAttackMenu::SortReplayChoiceCB();
}

void TimeAttackMenu::MenuUpdateCB_Replay()
{
    UIControl *replayControl   = sVars->replaysControl;
    UIReplayCarousel *carousel = sVars->replayCarousel;

    if (replayControl->lastButtonID != 1)
        carousel->curReplayID = -1;

    UIButtonPrompt *prompt = sVars->replayPrompt;
    if (prompt)
        prompt->visible = APITable->GetSortedUserDBRowCount(globals->replayTableID) != 0;
}

void TimeAttackMenu::MenuUpdateCB()
{
    UIControl *control = sVars->timeAttackControl;

    // Keep "Replays" button centered
    control->buttons[control->buttonCount - 1]->position.x = control->position.x;
}

void TimeAttackMenu::ReplayButton_ActionCB()
{
    UIControl *control         = sVars->replaysControl;
    UIReplayCarousel *carousel = sVars->replayCarousel;

    control->buttonID     = 1;
    carousel->curReplayID = 0;
    UIControl::MatchMenuTag("Replays");

    UIButton::SetChoiceSelectionWithCB(control->buttons[0], 0);
}

void TimeAttackMenu::YPressCB_Replay()
{
    UIControl *control         = sVars->replaysControl;
    UIReplayCarousel *carousel = sVars->replayCarousel;

    if (carousel->stateDraw.Matches(&UIReplayCarousel::Draw_Carousel)) {
        control->buttonID = 0;

        carousel->curReplayID      = -1;
        carousel->curViewOffset    = 0;
        carousel->targetViewOffset = 0;

        UIButton::SetChoiceSelectionWithCB(control->buttons[0], control->buttons[0]->selection ^ 1);

        UIWidgets::sVars->sfxBleep.Play(false, 255);
    }
}

void TimeAttackMenu::SortReplayChoiceCB()
{
    UIControl *control         = sVars->replaysControl;
    UIReplayCarousel *carousel = sVars->replayCarousel;

    APITable->SetupUserDBRowSorting(globals->replayTableID);

    if (control->buttons[0]->selection == 1)
        APITable->SortDBRows(globals->replayTableID, API::Storage::UserDB::UInt32, "zoneSortVal", false);
    else
        APITable->SortDBRows(globals->replayTableID, 0, nullptr, true);

    carousel->stateDraw.Set(&UIReplayCarousel::Draw_Loading);
}

void TimeAttackMenu::SetupDetailsView()
{
    MenuParam *param   = MenuParam::GetMenuParam();
    UIControl *control = sVars->taDetailsControl;
    UITABanner *banner = sVars->detailsBanner;

    UIChoice *act1 = (UIChoice *)UIButton::GetChoicePtr(control->buttons[0], 0);
    UIChoice *act2 = (UIChoice *)UIButton::GetChoicePtr(control->buttons[0], 1);

    if (banner->zoneID == 0) {
        // Prevent us from selecting OWZ act 2
        act2->disabled = true;
        UIButton::SetChoiceSelectionWithCB(control->buttons[0], 0);
        act1->arrowWidth = 0;
    }
    else {
        act2->disabled = false;
        act2->labelAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 21, true, 3);
        UIButton::SetChoiceSelectionWithCB(control->buttons[0], 0);
        act1->arrowWidth = 48;
    }
}

void TimeAttackMenu::TAModule_ActionCB()
{
    UITAZoneModule *module = (UITAZoneModule *)this;

    MenuParam *param   = MenuParam::GetMenuParam();
    UITABanner *banner = sVars->detailsBanner;
    UIControl *control = sVars->taDetailsControl;

    param->zoneID     = module->zoneID;
    control->buttonID = 0;

    UITABanner::SetupDetails(banner, module->zoneID, 0, module->characterID);
    TimeAttackMenu::SetupDetailsView();

    UITransition::SetNewTag("Time Attack Detail");
}

void TimeAttackMenu::StartTAAttempt()
{
    MenuParam *param = MenuParam::GetMenuParam();

    sprintf_s(param->menuTag, (int32)sizeof(param->menuTag), "Time Attack Detail");
    param->menuSelection = 0;
    param->inTimeAttack  = true;

    ReplayRecorder::Replay *replayPtr = (ReplayRecorder::Replay *)globals->replayReadBuffer;
    if (replayPtr->header.isNotEmpty && replayPtr->header.signature == REPLAY_SIGNATURE)
        memset(globals->replayReadBuffer, 0, sizeof(globals->replayReadBuffer));

    TimeAttackMenu::LoadScene();
}

void TimeAttackMenu::LoadScene()
{
    MenuParam *param = MenuParam::GetMenuParam();

    SaveGame::ResetPlayerState();

    // Bug Details(?):
    // sizeof(globals->noSaveSlot) and sizeof(saveData) is 4096 (sizeof(int32) * 0x400)
    // but the memset size is only 1024 (sizeof(uint8) * 0x400)
    // so only about 1/4th of the save slot is cleared, though nothin uses the extra space so it's not a big deal
    memset(globals->noSaveSlot, 0, 0x400);

    globals->continues  = 0;
    globals->saveSlotID = NO_SAVE_SLOT;
    globals->gameMode   = MODE_TIMEATTACK;
    globals->medalMods  = 0;

    Stage::SetScene("Adventure Mode", "");
    sceneInfo->listPos += TimeAttackData::GetZoneListPos(param->zoneID, param->actID, param->characterID);

    const char *playingAsText  = "";
    const char *characterImage = "";
    const char *characterText  = "";
    switch (param->characterID) {
        case 1:
            globals->playerID = ID_SONIC;
            playingAsText     = "Playing as Sonic";
            characterImage    = "sonic";
            characterText     = "Sonic";
            break;
        case 2:
            globals->playerID = ID_TAILS;
            playingAsText     = "Playing as Tails";
            characterImage    = "tails";
            characterText     = "Tails";
            break;
        case 3:
            globals->playerID = ID_KNUCKLES;
            playingAsText     = "Playing as Knuckles";
            characterImage    = "knuckles";
            characterText     = "Knuckles";
            break;
        default: break;
    }

    SetPresence(playingAsText, "In Time Attack", "doggy", "doggy", characterImage, characterText);

    Stage::LoadScene();
}

void TimeAttackMenu::YPressCB_ZoneSel() {}

bool32 TimeAttackMenu::BackPressCB_ZoneSel()
{
    UITransition::SetNewTag("Time Attack");

    return true;
}

void TimeAttackMenu::YPressCB_Details()
{
    UITABanner *banner = sVars->detailsBanner;

    int32 rowCount = APITable->GetSortedUserDBRowCount(globals->taTableID);
}

void TimeAttackMenu::ResetTimes_YesCB()
{
    UITABanner *banner = sVars->detailsBanner;
    UIControl *control = sVars->taDetailsControl;

    int32 act = control->buttons[0]->selection;
    while (APITable->GetSortedUserDBRowCount(globals->taTableID) > 0) {
        int32 rowID = APITable->GetSortedUserDBRowID(globals->taTableID, 0);
        APITable->RemoveDBRow(globals->taTableID, rowID);

        TimeAttackData::ConfigureTableView(banner->zoneID, act, banner->characterID);
    }

    control->buttonID = 0;
    TimeAttackData::SaveDB(nullptr);

    TimeAttackMenu::TAZoneModule_ChoiceChangeCB();
}

void TimeAttackMenu::XPressCB_Details()
{
    String message = {};

    Localization::GetString(&message, Localization::ResetTimesWarning);

    Action<void> callbackYes;
    callbackYes.Set(&TimeAttackMenu::ResetTimes_YesCB);

    Action<void> callbackNo;
    callbackNo.Set(nullptr);

    UIDialog::CreateDialogYesNo(&message, callbackYes, callbackNo, true, true);
}

void TimeAttackMenu::TAZoneModule_ActionCB()
{
    UIButton *button = (UIButton *)this;

    MenuParam *param   = MenuParam::GetMenuParam();
    UIControl *control = sVars->taDetailsControl;

    control->selectionDisabled = true;
    param->actID               = control->buttons[0]->selection;

    MenuSetup::StartTransition(TimeAttackMenu::StartTAAttempt, 32);
}

void TimeAttackMenu::RankButton_ActionCB()
{
    UIRankButton *rankButton = (UIRankButton *)this;

    UIPopover *popover = UIPopover::CreatePopover();
    if (popover) {
        popover->storedEntity = (Entity *)this;

        UIPopover::AddButton(popover, UIPopover::POPOVER_WATCH_PIVOTED, TimeAttackMenu::WatchReplayCB_RanksMenu, false);
        UIPopover::AddButton(popover, UIPopover::POPOVER_CHALLENGE_PIVOTED, TimeAttackMenu::ChallengeReplayCB_RanksMenu, false);
        UIPopover::Setup(popover, rankButton->popoverPos.x - TO_FIXED(85), rankButton->popoverPos.y - TO_FIXED(68));
    }
}

void TimeAttackMenu::MenuSetupCB_Details() { TimeAttackMenu::TAZoneModule_ChoiceChangeCB(); }

void TimeAttackMenu::TAZoneModule_ChoiceChangeCB()
{
    MenuParam *param   = MenuParam::GetMenuParam();
    UIControl *control = sVars->taDetailsControl;

    int32 act = control->buttons[0]->selection;

    UITABanner::SetupDetails(sVars->detailsBanner, param->zoneID, act, param->characterID);
    TimeAttackData::ConfigureTableView(param->zoneID, act, param->characterID);

    int32 rowCount = 1;
    for (int32 rank = 1; rank < 4; ++rank) {
        UIRankButton *rankButton = (UIRankButton *)control->buttons[rank];

        int32 score    = TimeAttackData::GetScore(param->zoneID, act, param->characterID, rank);
        int32 replayID = TimeAttackData::GetReplayID(param->zoneID, act, param->characterID, rank);

        UIRankButton::SetTimeAttackRank(rankButton, rank, score, replayID);

        if (score)
            ++rowCount;

        if (replayID)
            rankButton->actionCB.Set(&TimeAttackMenu::RankButton_ActionCB);
        else
            rankButton->actionCB.Set(nullptr);
    }

    control->rowCount = rowCount;
}

void TimeAttackMenu::CharButton_ActionCB()
{
    UICharButton *charButton = (UICharButton *)this;

    MenuParam *param   = MenuParam::GetMenuParam();
    UIControl *control = sVars->taZoneSelControl;

    TimeAttackData::Clear();

    int32 characterID  = charButton->characterID + 1;
    param->characterID = characterID;

    APITable->InitLeaderboards();

    for (int32 i = 0; i < control->buttonCount; ++i) {
        UITAZoneModule *charButton = (UITAZoneModule *)control->buttons[i];
        charButton->characterID    = characterID;
    }

    UIControl::MatchMenuTag("Time Attack Zones");
}

void TimeAttackMenu::TransitionToDetailsCB() { UIControl::SetActiveMenu(sVars->taDetailsControl); }

void TimeAttackMenu::SetupLeaderboardsCarousel(UICarousel *carousel)
{
    UIControl *parent = (UIControl *)carousel->parent;

    LeaderboardAvail avail = APITable->LeaderboardEntryViewSize();
    carousel->minOffset    = MAX(avail.start, 1);
    carousel->maxOffset    = MAX(avail.start + avail.length, carousel->minOffset + 5);

    if (TimeAttackMenu::sVars->prevIsUser && avail.length) {
        int32 userID = 0;

        int32 end = avail.start + avail.length;
        for (int32 entryID = avail.start; entryID < end; ++entryID) {
            LeaderboardEntry *entry = APITable->ReadLeaderboardEntry(entryID);
            if (entry->isUser) {
                userID = entryID;
                break;
            }
        }

        carousel->scrollOffset = userID != parent->buttonCount >> 1 ? userID - (parent->buttonCount >> 1) : 0;
        carousel->virtualIndex = !userID ? avail.start : userID;
    }
    else {
        carousel->virtualIndex = 1;
        carousel->scrollOffset = 0;
    }

    for (int32 i = 0; i < parent->buttonCount; ++i) {
        int32 max = (i - carousel->scrollOffset) % parent->buttonCount;
        if (max < 0)
            max += parent->buttonCount;

        int32 vIndex = max + carousel->scrollOffset;
        LogHelpers::Print("i = %d, vIndex = %d", i, vIndex);

        if (vIndex == carousel->virtualIndex) {
            parent->buttonID = i;
            break;
        }
    }

    LogHelpers::Print("avail.start = %d, length = %d", avail.start, avail.length);
    LogHelpers::Print("scrollOffset = %d", carousel->scrollOffset);
    LogHelpers::Print("virtualIndex = %d", carousel->virtualIndex);
    LogHelpers::Print("minOffset = %d", carousel->minOffset);
    LogHelpers::Print("maxOffset = %d", carousel->maxOffset);

    carousel->buttonID = -1;
}

#if RETRO_INCLUDE_EDITOR
void TimeAttackMenu::EditorDraw(void) {}

void TimeAttackMenu::EditorLoad(void) {}
#endif

void TimeAttackMenu::Serialize(void) {}

} // namespace GameLogic