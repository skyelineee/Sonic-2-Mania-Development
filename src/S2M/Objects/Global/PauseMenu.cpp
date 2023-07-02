// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: PauseMenu Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "PauseMenu.hpp"
#include "Zone.hpp"
#include "Music.hpp"
#include "Player.hpp"
#include "TitleCard.hpp"
#include "ActClear.hpp"
#include "Localization.hpp"
#include "ReplayRecorder.hpp"
#include "StarPost.hpp"
#include "SaveGame.hpp"
#include "Menu/UIWidgets.hpp"
#include "Menu/UIButton.hpp"
#include "Menu/UIDialog.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Helpers/MenuParam.hpp"
#include "Helpers/MathHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(PauseMenu);

void PauseMenu::Update()
{
    this->state.Run(this);

    this->position.x = (screenInfo->position.x + screenInfo->center.x) << 16;
    this->position.y = (screenInfo->position.y + screenInfo->center.y) << 16;

    if (this->manager) {
        this->manager->position.x = this->position.x;
        this->manager->position.y = this->position.y;
        PauseMenu::HandleButtonPositions();
    }
}

void PauseMenu::LateUpdate()
{
    if (!this->state.Matches(nullptr)) {
        if (Music::IsPlaying())
            Music::Pause();
    }
    else {
        StateMachine<Player> state = GameObject::Get<Player>(this->triggerPlayer)->state;

        if (state.Matches(&Player::State_Death) || state.Matches(&Player::State_Drown)) {
            this->Destroy();
        }
        else {
            this->visible   = true;
            this->drawGroup = DRAWGROUP_COUNT - 1;
            Stage::SetEngineState(ENGINESTATE_FROZEN);
            this->animator.SetAnimation(UIWidgets::sVars->textFrames, 10, true, 3);
            PauseMenu::PauseSound();
            this->state.Set(&PauseMenu::State_SetupButtons);
        }
    }
}

void PauseMenu::StaticUpdate()
{
    if (sceneInfo->state == ENGINESTATE_REGULAR) {
        int32 cnt = 0;
        if (TitleCard::sVars)
            cnt = RSDKTable->GetEntityCount(TitleCard::sVars->classID, true);
        if (ActClear::sVars)
            cnt += RSDKTable->GetEntityCount(ActClear::sVars->classID, true);

        PauseMenu *pauseMenu = GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);

        bool32 allowEvents = true;
        if (Zone::sVars)
            allowEvents = Zone::sVars->timer > 1;

        if (!cnt && pauseMenu->classID == 0 && allowEvents && !sVars->disableEvents) {
            if (API::Auth::GetUserAuthStatus() == STATUS_FORBIDDEN) {
                sVars->signOutDetected = true;
                RSDKTable->ResetEntitySlot(SLOT_PAUSEMENU, sVars->classID, nullptr);
                pauseMenu->triggerPlayer = 0;
            }
            //else if (API.CheckDLC(DLC_PLUS) != (bool32)globals->lastHasPlus) {
            //    sVars->plusChanged = true;
            //    globals->lastHasPlus   = API.CheckDLC(DLC_PLUS);
            //    RSDKTable->ResetEntitySlot(SLOT_PAUSEMENU, sVars->classID, nullptr);
            //   pauseMenu->triggerPlayer = 0;
            //}
            else {
                for (int32 i = 0; i < PauseMenu::GetPlayerCount(); ++i) {
                    int32 id = Input::GetInputDeviceID(Input::CONT_P1 + i);
                    if (!Input::IsInputDeviceAssigned(id) && id != Input::INPUT_AUTOASSIGN) {
                        sVars->controllerDisconnect = true;
                        RSDKTable->ResetEntitySlot(SLOT_PAUSEMENU, sVars->classID, nullptr);
                        pauseMenu->triggerPlayer = i;
                    }
                    if (!Input::IsInputSlotAssigned(Input::CONT_P1 + i)) {
                        sVars->controllerDisconnect = true;
                        RSDKTable->ResetEntitySlot(SLOT_PAUSEMENU, sVars->classID, nullptr);
                        pauseMenu->triggerPlayer = i;
                    }
                }
            }
        }
    }
}

void PauseMenu::Draw()
{
    if (this->paused)
        Graphics::FillScreen(0x000000, this->fadeTimer, this->fadeTimer - 128, this->fadeTimer - 256);

    if (Graphics::GetVideoSetting(VIDEOSETTING_SCREENCOUNT) <= 1) {
        this->stateDraw.Run(this);
    }
}

void PauseMenu::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active = ACTIVE_ALWAYS;

        if (data == INT_TO_VOID(true)) {
            this->visible   = true;
            this->drawGroup = DRAWGROUP_COUNT - 1;
            this->state.Set(&PauseMenu::State_HandleFadeout);
        }
        else {
            this->state.Set(nullptr);
            this->stateDraw.Set(nullptr);
        }
    }
}

void PauseMenu::StageLoad()
{
    sVars->active = ACTIVE_ALWAYS;

    sVars->sfxBleep.Get("Global/MenuBleep.wav");
    sVars->sfxAccept.Get("Global/MenuAccept.wav");

    sVars->disableEvents        = false;
    sVars->controllerDisconnect = false;
    sVars->forcedDisconnect     = false;
    sVars->signOutDetected      = false;
    //sVars->plusChanged = false;

    //if (!globals->hasPlusInitial) {
    //    globals->lastHasPlus    = API.CheckDLC(DLC_PLUS);
    //   globals->hasPlusInitial = true;
    //}

    for (int32 i = 0; i < CHANNEL_COUNT; ++i) {
        sVars->activeChannels[i] = false;
    }

    PauseMenu::SetupTintTable();
}

void PauseMenu::SetupMenu()
{
    Vector2 size;
    size.x = screenInfo->size.x << 16;
    size.y = screenInfo->size.y << 16;
    RSDKTable->ResetEntitySlot(SLOT_PAUSEMENU_UICONTROL, UIControl::sVars->classID, &size);

    UIControl *control = GameObject::Get<UIControl>(SLOT_PAUSEMENU_UICONTROL);

    control->position.x = (screenInfo->position.x + screenInfo->center.x) << 16;
    control->position.y = (screenInfo->position.y + screenInfo->center.y) << 16;

    // Bug Details:
    // control->rowCount is slightly bugged, if `pauseMenu->disableRestart` is enabled then wrapping by pressing down is broken and wont work
    // this is due to rowCount being 3, while control->buttonCount is only 2
    // Fix:
    // set control->rowCount to control->buttonCount once it's been initialized, instead of using a constant value
    control->rowCount    = PAUSEMENU_BUTTON_COUNT;
    control->columnCount = 1;
    control->buttonID    = 0;
    this->manager        = control;

    int32 i = 0;
    for (; i < PAUSEMENU_BUTTON_COUNT; ++i) {
        if (!this->buttonPtrs[i])
            break;
        UIButton *button = this->buttonPtrs[i];
        button->parent         = (Entity *)control;
        control->buttons[i]    = button;
    }
    control->buttonCount = i;
}

void PauseMenu::SetupTintTable()
{
    for (int32 i = 0; i < 0x10000; ++i) {
        uint32 r = (0x20F * (i >> 11) + 23) >> 6;
        uint32 g = (0x103 * ((i >> 5) & 0x3F) + 33) >> 6;
        uint32 b = (0x20F * (i & 0x1F) + 23) >> 6;

        int32 brightness = MIN(((b + g + r) << 8) / 680, 0xFF);

        sVars->tintLookupTable[i] = (brightness >> 3) | ((brightness >> 2) << 5) | ((brightness >> 3) << 11);
        //sVars->tintLookupTable[i] = ColorHelpers::PackRGB(brightness, brightness, brightness);
    }
}

void PauseMenu::AddButton(uint8 id, Action<void> action)
{
    int32 buttonID = this->buttonCount;
    if (buttonID < PAUSEMENU_BUTTON_COUNT) {
        this->buttonIDs[buttonID]     = id;
        this->buttonActions[buttonID] = action;

        int32 buttonSlot = this->buttonCount + 18;
        RSDKTable->ResetEntitySlot(buttonSlot, UIButton::sVars->classID, nullptr);
        UIButton *button = GameObject::Get<UIButton>(buttonSlot);

        button->position.x = (screenInfo->position.x + screenInfo->center.x) << 16;
        button->position.y = (screenInfo->position.y + screenInfo->center.y) << 16;
        button->nameAnimator.SetAnimation(UIWidgets::sVars->textFrames, 10, true, id);
        button->actionCB.Set(&PauseMenu::ActionCB_Button);
        button->size.x             = 60 << 16;
        button->size.y             = 21 << 16;
        button->bgEdgeSize         = 21;
        button->align              = UIButton::ALIGN_LEFT;
        button->drawGroup          = this->drawGroup;
        button->active             = ACTIVE_ALWAYS;
        this->buttonPtrs[buttonID] = button;
        ++this->buttonCount;
    }
}

void PauseMenu::ClearButtons(PauseMenu *entity)
{
    if (entity->manager)
        entity->manager->Destroy();

    for (int32 i = 0; i < PAUSEMENU_BUTTON_COUNT; ++i) {
        if (entity->buttonPtrs[i])
            entity->buttonPtrs[i]->Destroy();
    }

    entity->Destroy();
}

void PauseMenu::HandleButtonPositions()
{
    Vector2 pos;
    pos.x = TO_FIXED(screenInfo->center.x - 69) + this->position.x + this->yellowTrianglePos.x;
    pos.y = (this->position.y + TO_FIXED(56)) + this->yellowTrianglePos.y - TO_FIXED(36);
    if (this->buttonCount == (PAUSEMENU_BUTTON_COUNT - 1)) {
        pos.x -= TO_FIXED(36);
        pos.y += TO_FIXED(36);
    }

    for (int32 i = 0; i < this->buttonCount; ++i) {
        if (!this->buttonPtrs[i])
            break;

        UIButton *button = this->buttonPtrs[i];
        button->startPos.x     = pos.x;
        button->startPos.y     = pos.y;
        button->position.x     = pos.x;
        button->position.y     = pos.y;
        pos.x -= TO_FIXED(36);
        pos.y += TO_FIXED(36);
    }
}

void PauseMenu::PauseSound(void)
{
    for (int32 i = 0; i < CHANNEL_COUNT; ++i) {
        if (RSDKTable->ChannelActive(i)) {
            RSDKTable->PauseChannel(i);
            sVars->activeChannels[i] = true;
        }
    }
}

void PauseMenu::ResumeSound(void)
{
    for (int32 i = 0; i < CHANNEL_COUNT; ++i) {
        if (sVars->activeChannels[i]) {
            RSDKTable->ResumeChannel(i);
            sVars->activeChannels[i] = false;
        }
    }
}

void PauseMenu::StopSound(void)
{
    for (int32 i = 0; i < CHANNEL_COUNT; ++i) {
        if (sVars->activeChannels[i]) {
            RSDKTable->StopChannel(i);
            sVars->activeChannels[i] = false;
        }
    }
}

void PauseMenu::FocusCamera()
{
    if (!Camera::sVars)
        return;

    LogHelpers::Print("FocusCamera(): triggerPlayer = %d", this->triggerPlayer);
    for (auto cameraPtr : GameObject::GetEntities<Camera>(FOR_ALL_ENTITIES))
    {
        int32 id         = RSDKTable->GetEntitySlot(cameraPtr);
        int32 prevScreen = cameraPtr->screenID;
        if (id - SLOT_CAMERA1 == this->triggerPlayer) {
            cameraPtr->screenID = 0;
            cameraPtr->SetCameraBoundsXY();
        }
        else {
            cameraPtr->screenID = 1;
        }
        LogHelpers::Print("cameraPtr->screenID %d => %d", prevScreen, cameraPtr->screenID);
    }
}

void PauseMenu::UpdateCameras()
{
    if (!Camera::sVars)
        return;

    for (auto camera : GameObject::GetEntities<Camera>(FOR_ALL_ENTITIES))
    {
        camera->screenID = RSDKTable->GetEntitySlot(camera) - SLOT_CAMERA1;
        camera->SetCameraBoundsXY();
    }
}

void PauseMenu::CheckAndReassignControllers()
{
    PauseMenu *entity = GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);

    // prolly a leftover from pre-plus
    int32 deviceID = Input::GetInputDeviceID(Input::CONT_P1 + (entity->triggerPlayer ^ 1));

    UNUSED(deviceID); // be quiet compiler I know it aint used!!

    int32 id = Input::GetFilteredInputDeviceID(true, true, 5);

    if (id)
        Input::AssignInputSlotToDevice(Input::CONT_P1 + entity->triggerPlayer, id);
    else
        Input::AssignInputSlotToDevice(Input::CONT_P1 + entity->triggerPlayer, Input::INPUT_AUTOASSIGN);

    if (globals->gameMode < MODE_TIMEATTACK && !Input::GetInputDeviceID(Input::CONT_P2))
        Input::AssignInputSlotToDevice(Input::CONT_P2, Input::INPUT_AUTOASSIGN);

    sVars->forcedDisconnect = true;
}

bool32 PauseMenu::IsDisconnected()
{
    int32 id = Input::GetInputDeviceID(Input::CONT_P1 + this->triggerPlayer);

    return Input::IsInputDeviceAssigned(id) || sVars->forcedDisconnect;
}

uint8 PauseMenu::GetPlayerCount()
{
    MenuParam *param            = MenuParam::GetMenuParam();

    return 1;
}

void PauseMenu::ResumeButtonCB()
{
    PauseMenu *pauseMenu = GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);
    pauseMenu->state.Set(&PauseMenu::State_Resume);
}

void PauseMenu::RestartButtonCB()
{
    GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);

    String msg;
    int32 strID = Localization::AreYouSure;
    if (!ReplayRecorder::sVars || !ReplayRecorder::sVars->isReplaying)
        strID = Localization::RestartWarning;
    Localization::GetString(&msg, strID);

    Action<void> callbackYes;
    callbackYes.Set(&PauseMenu::RestartDialog_YesCB);

    Action<void> callbackNo;
    callbackNo.Set(nullptr);

    UIDialog::CreateDialogYesNo(&msg, callbackYes, callbackNo, false, true);
}

void PauseMenu::ExitButtonCB()
{
    GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);

    String msg;
    int32 strID = Localization::AreYouSure;
    if (!ReplayRecorder::sVars || !ReplayRecorder::sVars->isReplaying)
        strID = Localization::QuitWarningLoseProgress;
    Localization::GetString(&msg, strID);

    Action<void> callbackYes;
    callbackYes.Set(&PauseMenu::ExitDialog_YesCB);

    Action<void> callbackNo;
    callbackNo.Set(nullptr);

    UIDialog::CreateDialogYesNo(&msg, callbackYes, callbackNo, false, true);
}

void PauseMenu::RestartDialog_YesCB()
{
    GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);

    UIDialog::sVars->activeDialog->parent->state.Set(nullptr);
    if (StarPost::sVars) {
        StarPost::sVars->postIDs[0] = 0;
        StarPost::sVars->postIDs[1] = 0;
        StarPost::sVars->postIDs[2] = 0;
        StarPost::sVars->postIDs[3] = 0;
    }
    Music::Stop();

    int32 x                  = (screenInfo->position.x + screenInfo->center.x) << 16;
    int32 y                  = (screenInfo->position.y + screenInfo->center.y) << 16;
    PauseMenu *fadeout       = GameObject::Create<PauseMenu>(INT_TO_VOID(true), x, y);
    fadeout->fadeoutCB.Set(&PauseMenu::RestartFadeCB);
    fadeout->state.Set(&PauseMenu::State_HandleFadeout);
}

void PauseMenu::ExitDialog_YesCB()
{
    GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);

    UIDialog::sVars->activeDialog->parent->state.Set(nullptr);
    globals->recallEntities               = false;
    globals->initCoolBonus                = false;

    if (StarPost::sVars) {
        StarPost::sVars->postIDs[0] = 0;
        StarPost::sVars->postIDs[1] = 0;
        StarPost::sVars->postIDs[2] = 0;
        StarPost::sVars->postIDs[3] = 0;
    }
    Music::Stop();

    int32 x                  = (screenInfo->position.x + screenInfo->center.x) << 16;
    int32 y                  = (screenInfo->position.y + screenInfo->center.y) << 16;
    PauseMenu *fadeout       = GameObject::Create<PauseMenu>(INT_TO_VOID(true), x, y);
    fadeout->fadeoutCB.Set(&PauseMenu::ExitFadeCB);
    fadeout->state.Set(&PauseMenu::State_HandleFadeout);
}

void PauseMenu::RestartFadeCB()
{
    globals->specialRingID = 0;
    PauseMenu::StopSound();
    Stage::LoadScene();
}

void PauseMenu::ExitFadeCB()
{
    if (StarPost::sVars) {
        StarPost::sVars->postIDs[0] = 0;
        StarPost::sVars->postIDs[1] = 0;
        StarPost::sVars->postIDs[2] = 0;
        StarPost::sVars->postIDs[3] = 0;
    }
    globals->specialRingID = 0;

    Stage::SetScene("Presentation", "Menu");
    PauseMenu::StopSound();
    Stage::LoadScene();
}

void PauseMenu::ActionCB_Button()
{
    PauseMenu *pauseMenu = GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);
    UIControl *manager   = pauseMenu->manager;

    if (manager->buttonID >= 0 && manager->buttonID < manager->buttonCount) {
        pauseMenu->buttonActions[manager->buttonID].Run(pauseMenu);
        //StateMachine_Run(pauseMenu->buttonActions[manager->buttonID]);
    }
}

void PauseMenu::State_SetupButtons()
{
    this->timer                 = 0;
    sVars->forcedDisconnect = false;

    if (sVars->controllerDisconnect || sVars->signOutDetected) {
        if (sVars->controllerDisconnect)
            this->disconnectCheck.Set(&PauseMenu::IsDisconnected);

        this->state.Set(&PauseMenu::State_ForcedPause);

        this->stateDraw.Set(&PauseMenu::Draw_ForcePause);
    }
    else {
        sVars->sfxAccept.Play(false, 255);

        Action<void> callbackResume;
        callbackResume.Set(&PauseMenu::ResumeButtonCB);
        PauseMenu::AddButton(0, callbackResume);

        if (!this->disableRestart) {
            Action<void> callbackRestart;
            callbackRestart.Set(&PauseMenu::RestartButtonCB);
            PauseMenu::AddButton(1, callbackRestart);
        }

        Action<void> callbackExit;
        callbackExit.Set(&PauseMenu::ExitButtonCB);
        PauseMenu::AddButton(2, callbackExit);

        PauseMenu::HandleButtonPositions();
        PauseMenu::SetupMenu();

        this->state.Set(&PauseMenu::State_StartPause);
        this->stateDraw.Set(&PauseMenu::Draw_RegularPause);

        if (globals->gameMode < MODE_TIMEATTACK && Input::GetInputDeviceID(Input::CONT_P2) == (uint32)Input::INPUT_AUTOASSIGN)
            Input::AssignInputSlotToDevice(Input::CONT_P2, Input::INPUT_NONE);
    }

    this->state.Run(this);
}

void PauseMenu::State_StartPause()
{
    if (this->timer == 1) {
        UIControl::sVars->inputLocked = false;
        UIControl::SetMenuLostFocus(this->manager);
    }

    if (this->timer >= 8) {
        this->headerPos.x         = 0;
        this->headerPos.y         = 0;
        this->yellowTrianglePos.x = 0;
        this->yellowTrianglePos.y = 0;
        this->timer               = 0;
        this->state.Set(&PauseMenu::State_Paused);
    }
    else {
        Vector2 pos;

        int32 alpha = 32 * this->timer;
        MathHelpers::Lerp2Sin1024(&pos, MAX(0, alpha), -TO_FIXED(240), 0, 0, 0);

        this->headerPos.x = pos.x;
        this->headerPos.y = pos.y;
        MathHelpers::Lerp2Sin1024(&pos, MAX(0, alpha), TO_FIXED(232), 0, 0, 0);

        ++this->timer;
        this->yellowTrianglePos.x = pos.x;
        this->yellowTrianglePos.y = pos.y;
        this->tintAlpha           = alpha;
    }
}

void PauseMenu::State_StartPauseCompetition()
{
    if (this->timer == 1) {
        UIControl::sVars->inputLocked = false;
        UIControl::SetMenuLostFocus(this->manager);
    }

    if (this->timer >= 8) {
        this->headerPos.x         = TO_FIXED(0);
        this->headerPos.y         = TO_FIXED(0);
        this->yellowTrianglePos.x = TO_FIXED(0);
        this->yellowTrianglePos.y = TO_FIXED(0);
        if (this->timer >= 16) {
            this->paused = false;
            this->timer  = 0;
            this->state.Set(&PauseMenu::State_Paused);
        }
        else {
            int32 t      = this->timer - 8;
            this->paused = true;
            if (this->timer == 8) {
                Graphics::SetVideoSetting(VIDEOSETTING_SCREENCOUNT, 1);
                PauseMenu::FocusCamera();
            }
            ++this->timer;
            this->fadeTimer = (8 - t) << 6;
        }
    }
    else {
        this->headerPos.x         = TO_FIXED(0xFFF);
        this->headerPos.y         = TO_FIXED(0xFFF);
        this->yellowTrianglePos.x = TO_FIXED(0xFFF);
        this->yellowTrianglePos.y = TO_FIXED(0xFFF);
        this->paused              = true;
        this->fadeTimer           = this->timer << 6;
        this->timer               = this->timer + 1;
    }
}

void PauseMenu::State_Paused()
{
    this->tintAlpha           = 0xFF;
    this->headerPos.x         = 0;
    this->headerPos.y         = 0;
    this->yellowTrianglePos.x = 0;
    this->yellowTrianglePos.y = 0;

    UIControl *manager = this->manager;
    if (Unknown_pausePress && !manager->dialogHasFocus) {
        PauseMenu *pauseMenu = GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);
        pauseMenu->state.Set(&PauseMenu::State_Resume);
    }
}

void PauseMenu::State_ForcedPause()
{
    String textBuffer;

    if (this->timer == 1) {
        UIControl::sVars->inputLocked = false;
        if (sVars->controllerDisconnect) {
            int32 strID = Localization::ReconnectWirelessController;
            if (SKU->platform == PLATFORM_SWITCH)
                strID = Localization::ReconnectController;
            Localization::GetString(&textBuffer, strID);

            UIDialog *dialog = UIDialog::CreateActiveDialog(&textBuffer);

            Action<void> callback;
            callback.Set(&PauseMenu::CheckAndReassignControllers);

            UIDialog::AddButton(UIDialog::DIALOG_CONTINUE, dialog, callback, 0);
            UIDialog::Setup(dialog);

            if (globals->gameMode < MODE_TIMEATTACK && Input::GetInputDeviceID(2) == (uint32)Input::INPUT_AUTOASSIGN)
                Input::AssignInputSlotToDevice(Input::CONT_P2, Input::INPUT_NONE);
        }
        else if (sVars->signOutDetected) {
            int32 strID = Localization::TestString;
            if (sVars->signOutDetected)
                strID = Localization::SignOutDetected;

            Localization::GetString(&textBuffer, strID);

            UIDialog *dialog = UIDialog::CreateActiveDialog(&textBuffer);

            Action<void> callback;
            callback.Set(&PauseMenu::State_SetupTitleFade);

            UIDialog::AddButton(UIDialog::DIALOG_OK, dialog, callback, 1);
            UIDialog::Setup(dialog);
        }
    }

    ++this->timer;
    if (!UIDialog::sVars->activeDialog) {
        if (this->forcePaused) {
            Stage::SetEngineState(ENGINESTATE_REGULAR);
            PauseMenu::ClearButtons(GameObject::Get<PauseMenu>(SLOT_PAUSEMENU));
            PauseMenu::ResumeSound();
        }
    }

    if (!this->forcePaused && !this->disconnectCheck.Matches(nullptr)) {
        if (!this->disconnectCheck.Matches(nullptr)) {
            if (sVars->controllerDisconnect)
                sVars->controllerDisconnect = false;

            UIDialog *dialog = UIDialog::sVars->activeDialog;
            if (dialog) {
                Action<void> callback;
                callback.Set(nullptr);
                UIDialog::CloseOnSel_HandleSelection(dialog, callback);
            }

            this->forcePaused = true;
        }
    }
}

void PauseMenu::State_ForcedPauseCompetition()
{
    if (this->timer == 1)
        UIControl::sVars->inputLocked = 0;

    if (this->timer >= 8) {
        if (this->timer >= 16) {
            this->paused    = false;
            this->fadeTimer = 0;
            this->timer     = 0;
            this->state.Set(&PauseMenu::State_ForcedPause);
        }
        else {
            int32 t      = this->timer - 8;
            this->paused = true;

            if (this->timer == 8) {
                Graphics::SetVideoSetting(VIDEOSETTING_SCREENCOUNT, 1);
                PauseMenu::FocusCamera();
            }

            ++this->timer;
            this->fadeTimer = (8 - t) << 6;
        }
    }
    else {
        this->paused    = true;
        this->fadeTimer = this->timer << 6;
        this->timer++;
    }
}

void PauseMenu::State_Resume()
{
    if (!this->timer && globals->gameMode < MODE_TIMEATTACK && !Input::GetInputDeviceID(Input::CONT_P2))
        Input::AssignInputSlotToDevice(Input::CONT_P2, Input::INPUT_AUTOASSIGN);

    if (this->timer >= 8) {
        this->headerPos.x         = -TO_FIXED(240);
        this->headerPos.y         = 0;
        this->yellowTrianglePos.x = TO_FIXED(232);
        this->yellowTrianglePos.y = 0;
        this->timer               = 0;
        Stage::SetEngineState(ENGINESTATE_REGULAR);
        PauseMenu::ClearButtons(GameObject::Get<PauseMenu>(SLOT_PAUSEMENU));
        PauseMenu::ResumeSound();
    }
    else {
        Vector2 pos;

        int32 percent = 0x20 * this->timer;
        MathHelpers::Lerp2Sin1024(&pos, MAX(0, percent), 0, 0, -TO_FIXED(240), 0);

        this->headerPos.x = pos.x;
        this->headerPos.y = pos.y;
        MathHelpers::Lerp2Sin1024(&pos, MAX(0, percent), 0, 0, TO_FIXED(232), 0);

        this->yellowTrianglePos.x = pos.x;
        this->yellowTrianglePos.y = pos.y;
        this->tintAlpha           = ((0x100 - this->timer) << 8) / 8;
        ++this->timer;
    }
}

void PauseMenu::State_ResumeCompetition()
{
    if (this->timer == 1) {
        UIControl::sVars->inputLocked = 0;
        UIControl::SetMenuLostFocus(this->manager);
    }

    if (this->timer >= 8) {
        if (this->timer >= 16) {
            this->paused = false;
            this->timer  = 0;
            Stage::SetEngineState(ENGINESTATE_REGULAR);
            PauseMenu::ClearButtons(GameObject::Get<PauseMenu>(SLOT_PAUSEMENU));
            PauseMenu::ResumeSound();
        }
        else {
            int32 t = this->timer - 8;

            if (this->timer == 8) {
                PauseMenu::UpdateCameras();
            }

            ++this->timer;
            this->fadeTimer = (8 - t) << 6;
        }
    }
    else {
        this->headerPos.x         = 0;
        this->headerPos.y         = 0;
        this->yellowTrianglePos.x = 0;
        this->yellowTrianglePos.y = 0;
        this->paused              = true;
        this->fadeTimer           = this->timer << 6;
        this->timer               = this->timer + 1;
    }
}

void PauseMenu::State_ForcedResumeCompetition()
{
    if (this->timer >= 8) {
        if (this->timer >= 16) {
            this->paused = false;
            this->timer  = 0;
            Stage::SetEngineState(ENGINESTATE_REGULAR);
            PauseMenu::ClearButtons(GameObject::Get<PauseMenu>(SLOT_PAUSEMENU));
            PauseMenu::ResumeSound();
        }
        else {
            int32 t      = this->timer - 8;
            this->paused = true;

            if (this->timer == 8) {
                PauseMenu::UpdateCameras();
            }

            ++this->timer;
            this->fadeTimer = (8 - t) << 6;
        }
    }
    else {
        this->paused    = true;
        this->fadeTimer = this->timer << 6;
        this->timer++;
    }
}

void PauseMenu::State_SetupTitleFade()
{
    PauseMenu *pauseMenu       = GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);
    pauseMenu->timer           = 0;
    pauseMenu->fadeTimer       = 0;
    pauseMenu->state.Set(&PauseMenu::State_FadeToTitle);
}

void PauseMenu::State_FadeToTitle()
{
    if (!UIDialog::sVars->activeDialog) {
        if (!this->timer) {
            this->paused = true;
            Music::FadeOut(0.2);
        }

        if (this->timer >= 60) {
            this->fadeTimer = 512;
            SaveGame::ClearRestartData();
            Stage::SetScene("Presentation", "Title Screen");
            Stage::LoadScene();
        }
        else {
            this->fadeTimer = (this->timer << 9) / 60;
            this->timer++;
        }
    }
}

void PauseMenu::State_HandleFadeout()
{
    this->fadeTimer += 12;
    this->paused = true;

    if (this->fadeTimer >= 1024) {
        this->fadeoutCB.Run(this);
    }
}

void PauseMenu::DrawPauseMenu()
{
    Vector2 drawPos;
    drawPos.x = this->position.x + TO_FIXED(100) + this->headerPos.x + -TO_FIXED(1) * screenInfo->center.x;
    drawPos.y = this->position.y - TO_FIXED(96) + this->headerPos.y;
    UIWidgets::DrawParallelogram(drawPos.x, drawPos.y, 200, 68, 68, 0xE8, 0x28, 0x58);

    drawPos.x += TO_FIXED(10);
    drawPos.y += TO_FIXED(6);
    UIWidgets::DrawParallelogram(drawPos.x, drawPos.y, 115, 24, 24, 0x00, 0x00, 0x00);

    // "PAUSED" text
    this->animator.DrawSprite(&drawPos, false);

    UIWidgets::DrawRightTriangle(this->yellowTrianglePos.x + TO_FIXED(screenInfo->center.x) + this->position.x,
                                this->yellowTrianglePos.y + TO_FIXED(screenInfo->center.y) + this->position.y, -232, 0xF0, 0xD8, 0x08);
}

void PauseMenu::Draw_RegularPause()
{
    if (!this->state.Matches(&PauseMenu::State_HandleFadeout)) {
        RSDKTable->SetTintLookupTable(sVars->tintLookupTable);
        Graphics::DrawRect(0, 0, screenInfo->size.x, screenInfo->size.y, 0, this->tintAlpha, INK_TINT, true);

        PauseMenu::DrawPauseMenu();
    }
}

void PauseMenu::Draw_ForcePause()
{
    if (!this->state.Matches(&PauseMenu::State_HandleFadeout)) {
        RSDKTable->SetTintLookupTable(sVars->tintLookupTable);
        Graphics::DrawRect(0, 0, screenInfo->size.x, screenInfo->size.y, 0, this->tintAlpha, INK_TINT, true);
    }
}

#if RETRO_INCLUDE_EDITOR
void PauseMenu::EditorDraw() {}

void PauseMenu::EditorLoad() {}
#endif

void PauseMenu::Serialize() {}

} // namespace GameLogic