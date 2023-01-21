// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: DialogRunner Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "DialogRunner.hpp"
#include "Global/SaveGame.hpp"
#include "LogHelpers.hpp"
#include "Menu/UILoadingIcon.hpp"
#include "Menu/UIDialog.hpp"
#include "Menu/MenuSetup.hpp"
#include "Menu/ManiaModeMenu.hpp"
#include "Global/Music.hpp"
#include "Global/Localization.hpp"
#include "Global/Zone.hpp"
#include "Helpers/GameProgress.hpp"
#include "Options.hpp"
#include "TimeAttackData.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(DialogRunner);

void DialogRunner::Update() { this->callback.Run(this); }
void DialogRunner::Create(void *data)
{
    this->active         = ACTIVE_ALWAYS;
    this->visible        = 0;
    this->callback.Copy((Action<void>*)data);
    this->timer          = 0;
    this->useGenericText = false;
}

void DialogRunner::StageLoad()
{
    sVars->authForbidden = false;
    sVars->signedOut     = false;
    sVars->unused2       = 0;
    sVars->unused1       = 0;
    sVars->isAutoSaving  = false;

    sVars->activeCallback = NULL;

    SaveGame::LoadSaveData();

    TimeAttackData::sVars->loaded          = false;
    TimeAttackData::sVars->uuid            = 0;
    TimeAttackData::sVars->rowID           = -1;
    TimeAttackData::sVars->personalRank    = 0;
    TimeAttackData::sVars->leaderboardRank = 0;
    TimeAttackData::sVars->isMigratingData = false;

    Options::sVars->changed = false;
    if (SKU->platform && SKU->platform != PLATFORM_DEV) {
        Options *options         = Options::GetOptionsRAM();
        options->vSync           = false;
        options->windowed        = false;
        options->windowBorder    = false;
        options->tripleBuffering = false;
    }
    else {
        Options::Reload();
    }
}

#if RETRO_REV0U
void DialogRunner::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(DialogRunner); }
#endif

void DialogRunner::HandleCallback()
{
    if (this->timer <= 0) {
        LogHelpers::Print("Callback: %x", this->callback);
        this->callback.Run(this);
        this->Destroy();
    }
    else {
        this->timer--;
    }
}

void DialogRunner::NotifyAutoSave_CB()
{
    sVars->isAutoSaving = false;
    globals->notifiedAutosave  = true;
    UILoadingIcon::FinishWait();
}

void DialogRunner::NotifyAutoSave()
{
    String string;
    string.Init(nullptr);

    if (sVars->isAutoSaving) {
        if (!UIDialog::sVars->activeDialog) {
            Localization::GetString(&string, Localization::AutoSaveNotif);
            Action<void> callback;
            callback.Set(&DialogRunner::NotifyAutoSave_CB);
            UIDialog *dialog = UIDialog::CreateDialogOk(&string, callback, true);
            dialog->useAltColor    = true;
        }
    }
    else {
        sVars->activeCallback = nullptr;
        this->Destroy();
    }
}

void DialogRunner::SetNoSaveDisabled()
{
    APITable->SetSaveStatusForbidden();
    APITable->SetNoSave(false);
}

void DialogRunner::SetNoSaveEnabled()
{
    APITable->SetSaveStatusError();
    APITable->SetNoSave(true);
}

void DialogRunner::PromptSavePreference_CB()
{
    String string;
    string.Init(nullptr);

    if (APITable->GetSaveStatus() == STATUS_CONTINUE) {
        if (!UIDialog::sVars->activeDialog) {
            int32 stringID = Localization::SaveLoadFail;
            switch (this->status) {
                case STATUS_ERROR:
                    stringID = Localization::NoXBoxProfile;
                    if (SKU->platform != PLATFORM_XB1)
                        stringID = Localization::SaveLoadFail;
                    break;
                case STATUS_CORRUPT: stringID = Localization::CorruptSave; break;
                case STATUS_NOSPACE: stringID = (SKU->platform == PLATFORM_XB1) + Localization::NoSaveSpace; break;
            }
            Localization::GetString(&string, stringID);

            Action<void> callbackYes;
            callbackYes.Set(&DialogRunner::SetNoSaveEnabled);

            Action<void> callbackNo;
            callbackNo.Set(&DialogRunner::SetNoSaveDisabled);

            UIDialog *dialog = UIDialog::CreateDialogYesNo(&string, callbackYes, callbackNo, true, true);
            dialog->useAltColor    = true;
        }
    }
    else {
        sVars->activeCallback = nullptr;
        this->Destroy();
    }
}
void DialogRunner::CheckUserAuth_CB()
{
    if (this->timer) {
        if (sVars->signedOut) {
            if (!UIDialog::sVars->activeDialog) {
                if (Zone::sVars && Zone::GetZoneID() != Zone::Invalid) {
                    Stage::SetScene("Presentation", "Title Screen");
                    Zone::StartFadeOut(10, 0x000000);
                }
                else if (MenuSetup::sVars) {
                    ManiaModeMenu::StartReturnToTitle();
                }
                else if (FXFade::sVars) {
                    Stage::SetScene("Presentation", "Title Screen");
                    FXFade *fxFade    = GameObject::Create<FXFade>(nullptr, this->position.x, this->position.y);
                    fxFade->active          = ACTIVE_ALWAYS;
                    fxFade->timer           = 0;
                    fxFade->speedIn         = 16;
                    fxFade->speedOut        = 16;
                    fxFade->state.Set(&FXFade::State_FadeOut);
                    fxFade->drawGroup       = DRAWGROUP_COUNT - 1;
                    fxFade->isPermanent     = true;
                    fxFade->oneWay          = true;
                    fxFade->transitionScene = true;
                }

                sVars->activeCallback = nullptr;
                this->Destroy();
            }
        }
        else if (!UIDialog::sVars->activeDialog) {
            String string;
            int32 id = Localization::SignOutDetected;
            if (this->useGenericText)
                id = Localization::ReturningToTitle;
            Localization::GetString(&string, id);
            Action<void> callback = {};
            callback.Set(&DialogRunner::CheckUserAuth_OK);
            UIDialog *dialog = UIDialog::CreateDialogOk(&string, callback, true);
            dialog->useAltColor    = true;
        }
    }
    else {
        UIDialog *dialog = UIDialog::sVars->activeDialog;
        if (dialog) {
            Action<void> callback;
            callback.Set(nullptr);
            UIDialog::CloseOnSel_HandleSelection(dialog, callback);
        }
        else {
            if (UIControl::sVars) {
                UIControl *control;
                control = UIControl::GetUIControl();
                if (UIControl::GetUIControl())
                    control->SetInactiveMenu(UIControl::GetUIControl());
            }

            Stage::SetEngineState(ENGINESTATE_FROZEN);
            Music::Stop();
            this->timer = 1;
        }
    }
}
void DialogRunner::ManageNotifs()
{
    if (GameProgress::CountUnreadNotifs()) {
        String string;
        string.Init(nullptr);
        if (!UIDialog::sVars->activeDialog) {
            int32 str = GameProgress::GetNotifStringID(GameProgress::GetNextNotif());
            Localization::GetString(&string, str);
            Action<void> callback;
            callback.Set(&DialogRunner::GetNextNotif);
            UIDialog *dialog = UIDialog::CreateDialogOk(&string, callback, true);
            dialog->playEventSfx   = true;
            dialog->useAltColor    = true;
        }
    }
    else {
        sVars->activeCallback = nullptr;
        UILoadingIcon::StartWait();
        GameProgress::Save(DialogRunner::TrackGameProgressCB);
        this->Destroy();
    }
}
void DialogRunner::TrackGameProgressCB(bool32 success) { UILoadingIcon::FinishWait(); }

void DialogRunner::GetNextNotif()
{
    if (sceneInfo->inEditor || APITable->GetNoSave() || globals->saveLoaded != STATUS_OK) {
        LogHelpers::Print("WARNING GameProgress Attempted to save before loading SaveGame file");
        return;
    }
    else {
        GameProgress *progressRAM = GameProgress::GetProgressRAM();
        int32 id              = GameProgress::GetNextNotif();
        if (id >= 0)
            progressRAM->unreadNotifs[id] = true;
    }
}
bool32 DialogRunner::CheckUnreadNotifs()
{
    if (!GameProgress::CountUnreadNotifs())
        return false;

    Action<void> data;

    if (!sVars->activeCallback)
        data.Set(&DialogRunner::ManageNotifs);
        sVars->activeCallback = GameObject::Create<DialogRunner>(&data, 0, 0);

    return true;
}
bool32 DialogRunner::NotifyAutosave()
{
    if (globals->notifiedAutosave) {
        if (!sVars->isAutoSaving && !sVars->activeCallback)
            return false;
    }
    else if (!sVars->isAutoSaving || !sVars->activeCallback) {
        UILoadingIcon::StartWait();
        sVars->isAutoSaving = true;
        globals->notifiedAutosave  = false;
        LogHelpers::Print("DUMMY NotifyAutosave()");
        Action<void> data;
        data.Set(&DialogRunner::NotifyAutoSave);
        DialogRunner *dialogRunner = GameObject::Create<DialogRunner>(&data, 0, 0);
        dialogRunner->active             = ACTIVE_ALWAYS;
        sVars->activeCallback     = dialogRunner;
    }

    return true;
}
void DialogRunner::GetUserAuthStatus()
{
    if (APITable->GetUserAuthStatus() == STATUS_FORBIDDEN) {
        if (sVars->authForbidden)
            return;

        Action<void> data;
        data.Set(&DialogRunner::CheckUserAuth_CB);
        DialogRunner *dialogRunner = GameObject::Create<DialogRunner>(&data, 0, 0);
        dialogRunner->active       = ACTIVE_ALWAYS;
        sVars->activeCallback      = dialogRunner;
        sVars->authForbidden       = true;
    }
}
void DialogRunner::PromptSavePreference(int32 id)
{
    if (APITable->GetNoSave()) {
        LogHelpers::Print("PromptSavePreference() returning due to noSave");
        return;
    }

    LogHelpers::Print("PromptSavePreference()");

    if (APITable->GetSaveStatus() == STATUS_CONTINUE)
        LogHelpers::Print("WARNING PromptSavePreference() when prompt already in progress.");

    APITable->ClearSaveStatus();

    Action<void> data;
    data.Set(&DialogRunner::PromptSavePreference_CB);
    DialogRunner *dialogRunner = GameObject::Create<DialogRunner>(&data, 0, 0);
    dialogRunner->status             = id;
    sVars->activeCallback     = dialogRunner;
}
void DialogRunner::CheckUserAuth_OK() { sVars->signedOut = true; }

} // namespace GameLogic