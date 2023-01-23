// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ManiaModeMenu Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "ManiaModeMenu.hpp"
#include "MainMenu.hpp"
#include "MenuSetup.hpp"
#include "UILoadingIcon.hpp"
#include "UIHeading.hpp"
#include "UIWidgets.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Helpers/MenuParam.hpp"
#include "Helpers/DialogRunner.hpp"
#include "Helpers/Options.hpp"
#include "Global/Localization.hpp"
#include "Global/SaveGame.hpp"
#include "Global/Music.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ManiaModeMenu);

void ManiaModeMenu::Update() {}

void ManiaModeMenu::LateUpdate() {}

void ManiaModeMenu::StaticUpdate() {}

void ManiaModeMenu::Draw() {}

void ManiaModeMenu::Create(void *data) {}

void ManiaModeMenu::StageLoad() {}

void ManiaModeMenu::Initialize()
{
    LogHelpers::Print("ManiaModeMenu::Initialize()");

    MainMenu::Initialize();
    // UISubHeading::Initialize(); i am NOT leaving it like this
    // TimeAttackMenu::Initialize();
    // CompetitionMenu::Initialize();
    // OptionsMenu::Initialize();
    // ExtrasMenu::Initialize();

    ManiaModeMenu::HandleUnlocks();
    ManiaModeMenu::SetupActions();
}

bool32 ManiaModeMenu::InitAPI()
{
    if (!MenuSetup::sVars->initializedAPI)
        MenuSetup::sVars->fxFade->timer = 512;

    int32 authStatus = API::Auth::GetUserAuthStatus();
    if (!authStatus) {
        API::Auth::TryAuth();
    }
    else if (authStatus != STATUS_CONTINUE) {
        int32 storageStatus = APITable->GetStorageStatus();
        if (!storageStatus) {
            API::Storage::TryInitStorage();
        }
        else if (storageStatus != STATUS_CONTINUE) {
            int32 saveStatus = API::Storage::GetSaveStatus();

            if (!API::Storage::GetNoSave() && (authStatus != STATUS_OK || storageStatus != STATUS_OK)) {
                if (saveStatus != STATUS_CONTINUE) {
                    if (saveStatus != STATUS_FORBIDDEN) {
                        DialogRunner::PromptSavePreference(storageStatus);
                    }
                    else {
                        Stage::SetScene("Presentation", "Title Screen");
                        Stage::LoadScene();
                    }
                }

                return false;
            }

            if (!MenuSetup::sVars->initializedSaves) {
                UILoadingIcon::StartWait();
                Options::LoadOptionsBin();
                SaveGame::LoadFile(&SaveGame::SaveLoadedCB);
                //ReplayRecorder::LoadReplayDB(nullptr);

                MenuSetup::sVars->initializedSaves = true;
            }

            if (MenuSetup::sVars->initializedAPI)
                return true;

            if (globals->optionsLoaded == STATUS_OK && globals->saveLoaded == STATUS_OK //&& globals->replayTableLoaded == STATUS_OK
                /*&& globals->taTableLoaded == STATUS_OK*/) {

                if (!API::Storage::GetNoSave() && DialogRunner::NotifyAutosave())
                    return false;

                UILoadingIcon::FinishWait();
                if (DialogRunner::CheckUnreadNotifs())
                    return false;

                MenuSetup::sVars->initializedAPI = true;
                return true;
            }

            if (API::Storage::GetNoSave()) {
                UILoadingIcon::FinishWait();
                return true;
            }
            else {
                if (globals->optionsLoaded == STATUS_ERROR || globals->saveLoaded == STATUS_ERROR //|| globals->replayTableLoaded == STATUS_ERROR
                    /*|| globals->taTableLoaded == STATUS_ERROR*/) {
                    int32 status = API::Storage::GetSaveStatus();

                    if (status != STATUS_CONTINUE) {
                        if (status == STATUS_FORBIDDEN) {
                            Stage::SetScene("Presentation", "Title Screen");
                            Stage::LoadScene();
                        }
                        else {
                            DialogRunner::PromptSavePreference(STATUS_CORRUPT);
                        }
                    }
                }
            }
        }
    }

    return false;
}

void ManiaModeMenu::InitLocalization(bool32 success)
{
    if (success) {
        Localization::sVars->loaded = false;

        Localization::LoadStrings();
        UIWidgets::ApplyLanguage();
        UIHeading::LoadSprites();
    }
}

int32 ManiaModeMenu::GetActiveMenu()
{
    UIControl *control = UIControl::GetUIControl();

    if (control == MainMenu::sVars->menuControl /*|| control == ExtrasMenu->extrasControl || control == OptionsMenu->optionsControl
        || control == OptionsMenu->videoControl || control == OptionsMenu->soundControl || control == OptionsMenu->dataOptionsControl
        || control == OptionsMenu->controlsControl_Windows || control == OptionsMenu->controlsControl_KB
        || control == OptionsMenu->controlsControl_PS4 || control == OptionsMenu->controlsControl_XB1 || control == OptionsMenu->controlsControl_NX
        || control == OptionsMenu->controlsControl_NXGrip || control == OptionsMenu->controlsControl_NXJoycon
        || control == OptionsMenu->controlsControl_NXPro*/) {
        return MenuSetup::Main;
    }

    /*if (control == TimeAttackMenu->timeAttackControl || control == TimeAttackMenu->timeAttackControl_Legacy
        || control == TimeAttackMenu->taZoneSelControl || control == TimeAttackMenu->taDetailsControl
        || control == TimeAttackMenu->leaderboardsControl || control == TimeAttackMenu->replaysControl {
        return MenuSetup::TimeAttack;
    }*/

    if (control == ManiaModeMenu::sVars->saveSelectMenu || control == ManiaModeMenu::sVars->noSaveMenu
        || control == ManiaModeMenu::sVars->secretsMenu)
        return MenuSetup::SaveSelect;

    return MenuSetup::Main;
}

void ManiaModeMenu::ChangeMenuTrack()
{
    int32 trackID = 0;

    switch (ManiaModeMenu::GetActiveMenu()) {
        default:
        case MenuSetup::Main: trackID = 0; break;
        case MenuSetup::TimeAttack: trackID = 1; break;
        //case MAINMENU_COMPETITION: trackID = 2; break;
        case MenuSetup::SaveSelect: trackID = 3; break;
        //case MAINMENU_SAVESELECT_ENCORE: trackID = 4; break;
    }

    if (!Music::IsPlaying())
        Music::PlayTrack(trackID);
    else if (Music::sVars->activeTrack != trackID)
        Music::PlayOnFade(trackID, 0.12f);
}

void ManiaModeMenu::StartReturnToTitle()
{
    UIControl *control = UIControl::GetUIControl();
    if (control)
        control->state.Set(nullptr);

    Music::FadeOut(0.05f);
    MenuSetup::StartTransition(ManiaModeMenu::ReturnToTitle, 32);
}

void ManiaModeMenu::ReturnToTitle()
{
    //TimeAttackData::Clear();

    Stage::SetScene("Presentation", "Title Screen");
    Stage::LoadScene();
}

void ManiaModeMenu::State_HandleTransition()
{
    MenuSetup *menuSetup = (MenuSetup *)this;
    menuSetup->fadeTimer = CLAMP(menuSetup->timer << ((menuSetup->fadeShift & 0xFF) - 1), 0, 0x200);
}

void ManiaModeMenu::HandleUnlocks()
{
    MainMenu::HandleUnlocks();
    //UISubHeading::HandleUnlocks(); changing
    //TimeAttackMenu::HandleUnlocks();
    //OptionsMenu::HandleUnlocks();
    //ExtrasMenu::HandleUnlocks();
}

void ManiaModeMenu::SetupActions()
{
    MainMenu::SetupActions();
    //UISubHeading::SetupActions(); changing
    //TimeAttackMenu::SetupActions();
    //OptionsMenu::SetupActions();
    //ExtrasMenu::SetupActions();
}

void ManiaModeMenu::HandleMenuReturn()
{
    MenuParam *param = (MenuParam *)globals->menuParam;

    char buffer[0x100];
    memset(buffer, 0, 0x100);
    for (auto control : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES))
    {
        if (strcmp(param->menuTag, "") != 0) {
            RSDKTable->GetCString(buffer, &control->tag);

            if (strcmp((const char *)buffer, param->menuTag) != 0) {
                control->SetInactiveMenu(control);
            }
            else {
                control->storedButtonID  = param->menuSelection;
                control->hasStoredButton = true;
                UIControl::SetActiveMenu(control);
                control->buttonID = param->menuSelection;
            }
        }
    }

    //UISubHeading::HandleMenuReturn(0); changing
    //TimeAttackMenu::HandleMenuReturn();
    //OptionsMenu::HandleMenuReturn();

    int32 characterID = 0, zoneID = 0, act = 0, isEncoreMode = false;
    if (param->inTimeAttack) {
        characterID  = param->characterID;
        zoneID       = param->zoneID;
        act          = param->actID;
        isEncoreMode = param->isEncoreMode;
    }

    //TimeAttackData::Clear();

    if (param->inTimeAttack) {
        param->characterID  = characterID;
        param->zoneID       = zoneID;
        param->actID        = act;
        param->isEncoreMode = isEncoreMode;
    }
}

#if RETRO_INCLUDE_EDITOR
void ManiaModeMenu::EditorDraw() {}

void ManiaModeMenu::EditorLoad() {}
#endif

void ManiaModeMenu::Serialize() {}
} // namespace GameLogic
