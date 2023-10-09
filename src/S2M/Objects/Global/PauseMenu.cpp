// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: PauseMenu Object
// Object Author: AChickMcNuggie
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

    //sets the pause menu position to the center of the screen
    this->position.x = (screenInfo->position.x + screenInfo->center.x) << 16;
    this->position.y = (screenInfo->position.y + screenInfo->center.y) << 16;

    //sets the button and control positions every frame
    if (this->manager) {
        this->manager->position.x = this->position.x;
        this->manager->position.y = this->position.y;
        PauseMenu::HandleButtonPositions();
    }

    this->trianglesLeftAnimator.Process();
    this->trianglesRightAnimator.Process();
    this->checkerboardBGAnimator.Process();
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
        // sets these all to 0 on create
        this->timer       = 0;
        this->pauseTimer  = 0;
        this->inkEffect   = INK_NONE;
        this->alpha       = 0;
        //sets the triangleSpeed to 10 on create, will slowly go down or go up when sliding in or away respectively
        this->triangleSpeed  = TO_FIXED(10);
        // sets the centerSpeed to 10 on create, will slowly go down or go up when sliding in or away respectively
        this->centerSpeed  = TO_FIXED(21);

        //grabs current screen
        ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

        // i would draw all of these relative to the entity rather than the screen, however it seems the pausemenus position is set after create so setting stuff to this->position here doesnt work
        // i tried to solve it by setting all of these to the screeninfo position and center like the uicontrol does, however if i do it like that, the visuals can be placed wrong as
        // the center of the screen changes in the stage due to what the player is doing, so screen relative it is
        // base positions for sprites for sliding in
        triangleLeftPos.x        = -TO_FIXED(46);
        triangleLeftPos.y        = TO_FIXED(0);
        triangleRightPos.x       = TO_FIXED(424);
        triangleRightPos.y       = TO_FIXED(0);
        bgCirclePos.x            = screen->center.x << 16;
        bgCirclePos.y            = screen->center.y << 16;
        // kept getting a warning when trying to do these circlepos.y on the same line so just subtracted it afterwards lol
        bgCirclePos.y            -= TO_FIXED(210);
        pauseTextPos.x           = screen->center.x << 16;
        pauseTextPos.y           = screen->center.y << 16;
        // same position as circlepos so it moves at the same speed and position, while also offsetted by negative 70 pixels as the pause text is 70 pixels above
        pauseTextPos.y           -= TO_FIXED(280);
        emeraldsPos.x            = screen->center.x << 16;
        emeraldsPos.y            = screen->center.y << 16;
        // same position as circlepos so it moves at the same speed and position, while also offsetted by positive 62 pixels as the emeralds are 62 pixels below
        emeraldsPos.y            -= TO_FIXED(148);

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
    //resets the pause menu control, i assume to not leave anything leftover from previous pauses
    RSDKTable->ResetEntitySlot(SLOT_PAUSEMENU_UICONTROL, UIControl::sVars->classID, &size);

    //sets the position to the center of the screen
    UIControl *control = GameObject::Get<UIControl>(SLOT_PAUSEMENU_UICONTROL);

    control->position.x = (screenInfo->position.x + screenInfo->center.x) << 16;
    control->position.y = (screenInfo->position.y + screenInfo->center.y) << 16;

    // these are set here instead of create so buttonPos can be properly set to the object position
    // position before sliding in
    this->buttonPos.x = this->position.x;
    // same position as circlepos so they move at the same rate and stay together
    this->buttonPos.y = this->position.y - TO_FIXED(210);

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
    //sets the tint color table, tho im gonna be honest i have no idea what its actually doing with all of these operators and hex numbers
    for (int32 i = 0; i < 0x10000; ++i) {
        uint32 r = (0x20F * (i >> 11) + 23) >> 6;
        uint32 g = (0x103 * ((i >> 5) & 0x3F) + 33) >> 6;
        uint32 b = (0x20F * (i & 0x1F) + 23) >> 6;

        int32 brightness = MIN(((b + g + r) << 8) / 680, 0xFF);

        sVars->tintLookupTable[i] = (brightness >> 3) | ((brightness >> 2) << 5) | ((brightness >> 6) << 14);
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
        button->actionCB.Set(&PauseMenu::ActionCB_Button);
        button->size.x             = 0;
        button->size.y             = 0;
        button->buttonListID       = 3;
        button->buttonFrameID      = 0;
        button->nameListID         = 5;
        button->nameFrameID        = id + 1;
        button->descriptionListID  = 7;
        button->descriptionFrameID = 0; // empty frame (no descriptions are needed)
        button->visibleArrowHorizontal = true; // lol
        button->align              = UIButton::ALIGN_LEFT;
        button->drawGroup          = this->drawGroup;
        button->active             = ACTIVE_ALWAYS;
        this->buttonPtrs[buttonID] = button;
        ++this->buttonCount;
    }
}

void PauseMenu::ClearButtons(PauseMenu *entity)
{
    //destroys the pause menu control and buttons if there are any present, destroys the actual pause menu entity itself at the end
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
    pos = this->position;
    // button y positions are now based on the buttonPos position which is changed rather than the pausemenu entity position, which is great as now it follows the rest of the pause menu
    pos.y = this->buttonPos.y - TO_FIXED(36);

    for (int32 i = 0; i < this->buttonCount; ++i) {
        if (!this->buttonPtrs[i])
            break;

        UIButton *button = this->buttonPtrs[i];
        button->startPos.x     = pos.x;
        button->startPos.y     = pos.y;
        button->position.x     = pos.x;
        button->position.y     = pos.y;
        //adds 30 pixels to the y position for every button in the control
        pos.y += TO_FIXED(30);
    }
}

void PauseMenu::PauseSound()
{
    for (int32 i = 0; i < CHANNEL_COUNT; ++i) {
        if (RSDKTable->ChannelActive(i)) {
            RSDKTable->PauseChannel(i);
            sVars->activeChannels[i] = true;
        }
    }
}

void PauseMenu::ResumeSound()
{
    for (int32 i = 0; i < CHANNEL_COUNT; ++i) {
        if (sVars->activeChannels[i]) {
            RSDKTable->ResumeChannel(i);
            sVars->activeChannels[i] = false;
        }
    }
}

void PauseMenu::StopSound()
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
    // i think this sets the pause camera bounds to whichever player pauses? this would only happen in competition i think which isnt here so shouldnt ever be used
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

    //looks for an currently assigned input device, if it cant find one it sets forced disconnect to true
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
    //loads the lose progress warning string if youre not currently watching a replay, otherwise loads the previous string without the warning
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
    // this resets the checkpoints i think so you dont end up loading your last touched checkpoint upon restarting
    if (StarPost::sVars) {
        StarPost::sVars->postIDs[0] = 0;
        StarPost::sVars->postIDs[1] = 0;
        StarPost::sVars->postIDs[2] = 0;
        StarPost::sVars->postIDs[3] = 0;
    }
    Music::Stop();

    //creates another pause entity with it set to fading out? i would think itd just be easier to create an fxfade object instead unless im wrong
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

    // this resets the checkpoints i think so you dont end up loading in a random one when picking another stage after exiting
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
    // this resets the checkpoints i think so you dont end up loading in a random one when picking another stage after exiting
    // tho this was also done in ExitDialog_YesCB which then calls this function, is there a reason its done twice?
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
    //gets the uicontrol of the pause menu
    PauseMenu *pauseMenu = GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);
    UIControl *manager   = pauseMenu->manager;

    //if the button id is 0 or higher and its less than the total buttoncount
    //then run the action of whatever button is pressed
    if (manager->buttonID >= 0 && manager->buttonID < manager->buttonCount) {
        pauseMenu->buttonActions[manager->buttonID].Run(pauseMenu);
    }
}

void PauseMenu::State_SetupButtons()
{
    this->timer                 = 0;
    sVars->forcedDisconnect = false;

    //if the controller is suddenly disconnected, force a pause (which is empty i believe it just freezes the game with the bg tint so things dont happen while its disconnected)
    if (sVars->controllerDisconnect || sVars->signOutDetected) {
        if (sVars->controllerDisconnect)
            this->disconnectCheck.Set(&PauseMenu::IsDisconnected);

        this->state.Set(&PauseMenu::State_ForcedPause);

        this->stateDraw.Set(&PauseMenu::Draw_ForcePause);
    }
    else {
        sVars->sfxAccept.Play(false, 255);
        
        //sets the action of each added button
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

        //assigns p2 to the pause menu if they start it i think?
        if (globals->gameMode < MODE_TIMEATTACK && Input::GetInputDeviceID(Input::CONT_P2) == (uint32)Input::INPUT_AUTOASSIGN)
            Input::AssignInputSlotToDevice(Input::CONT_P2, Input::INPUT_NONE);
    }

    this->state.Run(this);
}

void PauseMenu::State_StartPause()
{
    if (this->timer >= 32) {
        this->timer               = 0;
        UIControl::sVars->inputLocked = false;
        UIControl::SetMenuLostFocus(this->manager);
        this->state.Set(&PauseMenu::State_Paused);
    }
    else {
        Vector2 pos;
       
        //i would think this is supposed to make the tinted background fade in when paused due to the timer, but it seems to just pop in with no easing so idk
        int32 alpha = 32 * this->timer;
        MathHelpers::Lerp2Sin1024(&pos, MAX(0, alpha), -TO_FIXED(240), 0, 0, 0);

        MathHelpers::Lerp2Sin1024(&pos, MAX(0, alpha), TO_FIXED(232), 0, 0, 0);

        ++this->timer;
        this->tintAlpha           = alpha;
    }
}

void PauseMenu::State_Paused()
{
    //sets tintalpha to max bc at this point everything should have come in
    this->tintAlpha           = 0xFF;

    //if you press the pause button again and no dialog is up then the game unpauses
    UIControl *manager = this->manager;
    if (Unknown_pausePress && !manager->dialogHasFocus) {
        PauseMenu *pauseMenu = GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);
        pauseMenu->state.Set(&PauseMenu::State_Resume);
    }
}

void PauseMenu::State_ForcedPause()
{
    String textBuffer;

    // this is currently bugged
    // after forcepausing, the game unpauses for every frame anyway, but since the controller is still disconnected, it pauses again, resulting in a loop
    // this isnt like gamebreaking bc if you reconnect the controller during this it unpauses just fine and you can play, but still annoying and needs to be fixed
    if (this->timer == 1) {
        UIControl::sVars->inputLocked = false;
        if (sVars->controllerDisconnect) {
            //loads the strings for reconnecting the controller on the uidialog
            //idk why if its switch it doesnt say wireless tho wouldnt most be using joycons which are wireless???
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
            //unpauses the game when the dialog is gone and its force paused, which would happen after you press the dialog button which means a controller is reconnected
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

void PauseMenu::State_Resume()
{
    if (!this->timer && globals->gameMode < MODE_TIMEATTACK && !Input::GetInputDeviceID(Input::CONT_P2))
        Input::AssignInputSlotToDevice(Input::CONT_P2, Input::INPUT_AUTOASSIGN);

    //locks the control once a button is pressed and starts its action
    UIControl::sVars->inputLocked = true;

    if (this->timer >= 32) {
        //resumes everything as normal and destroys the pause menu with the clear buttons function
        this->timer               = 0;
        Stage::SetEngineState(ENGINESTATE_REGULAR);
        PauseMenu::ClearButtons(GameObject::Get<PauseMenu>(SLOT_PAUSEMENU));
        PauseMenu::ResumeSound();
    }
    else {
        Vector2 pos;

        //again i assume this is spuposed to make the bg tint fade away but it just kinda disappears with no easing so idk
        int32 percent = 0x20 * this->timer;
        MathHelpers::Lerp2Sin1024(&pos, MAX(0, percent), 0, 0, -TO_FIXED(240), 0);

        MathHelpers::Lerp2Sin1024(&pos, MAX(0, percent), 0, 0, TO_FIXED(232), 0);

        this->tintAlpha           = ((0x100 - this->timer) << 32) / 32;
        ++this->timer;
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
    //i think this happens when the user is signed out and it returns to the title screen, as the exit button goes to the main menu
    //however this should never happen here as there arent any users LOL
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

void PauseMenu::DrawStartPause()
{
    // if the speed is above 0, subtract it by 1 every frame (this makes it slowly descend which looks nice)
    if (this->triangleSpeed > 0) {
        this->triangleSpeed -= TO_FIXED(1);
    }
    // makes sure it never gets below 0 and start moving in the wrong direction
    if (this->triangleSpeed < 0) {
        this->triangleSpeed = 0;
    }

    // this is probably ugly but i need a different speed value for the sprites in the middle, as they have a far greater distance to travel and the previous one ends too soon
    // i tried just adding a constant to_fixed value to the original triangle speed for the middle objects, but it still reaches 0 at the same time and just starts to move by a fixed 8 pixels alone every frame
    // which makes the movement look very ugly and unnatural
    if (this->centerSpeed > 0) {
        this->centerSpeed -= TO_FIXED(1);
    }
    if (this->centerSpeed < 0) {
        this->centerSpeed = 0;
    }

    // grabs current screen
    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    Vector2 drawPos;

    // checkerboard bg
    this->inkEffect = INK_ALPHA;
    // increases alpha for the checkerboard
    if (this->alpha < 96) {
        this->alpha += 8;
    }
    drawPos.x = 0;
    drawPos.y = 0;
    this->checkerboardBGAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 8, false, 0);
    this->checkerboardBGAnimator.DrawSprite(&drawPos, true);

    // side triangles
    // have to use triangleLeftPos which is set in create unfortunately
    // i would rather use local drawing variables like in drawpausemenu but cant bc it would just be set back to its pase position after every attempt of moving as a result of it going every frame
    // i dont know if its possible to get it to stop at exactly 0 with this method of easing, adding or subtracting a fixed amount if its
    // under or above a certain position usually makes it end either a little too far or not far enough from whatever exact position is wanted
    // for example, if you want it to stop moving at position fixed(50), and it increases by 8 every frame, you could have it be at 48, and then it adds another 8 bc it is still currently under 50
    // however that would set its position to fixed(56) and itd be farther than whats wanted which is annoying
    // in this case the speed reaches 0 a little before it reaches its end, so it stops moving a little before but id rather have that then it going too far, which would happen
    // if i increased the speed anymore as a result of what ive mentioned above, ill just have to settle for the closest i can get unfortunately
    this->inkEffect = INK_NONE;
    drawPos.x = triangleLeftPos.x;
    drawPos.y = triangleLeftPos.y;
    this->trianglesLeftAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 1, false, 0);
    this->trianglesLeftAnimator.DrawSprite(&drawPos, true);
    // adds to the left triangle position by whatever the triangle speed currently is if its below 0 (0 should be the final resting position)
    if (triangleLeftPos.x < TO_FIXED(0)) {
        triangleLeftPos.x += this->triangleSpeed;
    }

    drawPos.x = triangleRightPos.x;
    drawPos.y = triangleRightPos.y;
    this->trianglesRightAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 2, false, 0);
    this->trianglesRightAnimator.DrawSprite(&drawPos, true);
    // subtracts the right triangle position by whatever the trianglespeed currently is if its above 378 (378 should be the final resting position)
    if (triangleRightPos.x > TO_FIXED(378)) {
        triangleRightPos.x -= this->triangleSpeed;
    }

    // these 2 are drawn together and use the same animator and position so these can be done in the same spot fortunately
    drawPos.x = bgCirclePos.x;
    drawPos.y = bgCirclePos.y;
    this->bgCircleAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 0, true, 0);
    this->bgCircleAnimator.DrawSprite(&drawPos, true);
    drawPos.y = bgCirclePos.y - TO_FIXED(11);
    this->bgCircleAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 0, true, 1);
    this->bgCircleAnimator.DrawSprite(&drawPos, true);
    // adds to the bg circles positions by whatever the centerspeed currently is if its above the current pausemenu.y position (that should be the final resting position)
    if (bgCirclePos.y < TO_FIXED(screen->center.y)) {
        bgCirclePos.y += this->centerSpeed;
    }

    drawPos.x = pauseTextPos.x;
    drawPos.y = pauseTextPos.y;
    this->pauseTextAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 5, true, 0);
    this->pauseTextAnimator.DrawSprite(&drawPos, true);
    // adds to the pause text position by whatever the centerspeed currently is if its 70 pixels or higher above the pausemenu.y position (that should be the final resting position)
    if (pauseTextPos.y < TO_FIXED(screen->center.y) - TO_FIXED(70)) {
        pauseTextPos.y += this->centerSpeed;
    }

    drawPos.x = emeraldsPos.x;
    drawPos.y = emeraldsPos.y;
    SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();
    this->emeraldsAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 6, true, 0);
    // i still dont know how get_bit works or what exactly this code is doing but i do know this is how to get the correct amount of emeralds displaying, so it is what it is
    for (int32 e = 0; e < 7; ++e) {
        if (GET_BIT(saveRAM->collectedEmeralds, e)) {
            this->emeraldsAnimator.frameID = e;
        }
        else if (saveRAM->collectedEmeralds == 0) {
            this->emeraldsAnimator.frameID = 7;
        }
    }
    this->emeraldsAnimator.DrawSprite(&drawPos, true);
    // adds to the emerald count position by whatever the centerspeed currently is if its higher above 62 pixels below the pausemenu.y position (that should be the final resting position)
    if (emeraldsPos.y < TO_FIXED(screen->center.y) + TO_FIXED(62)) {
        emeraldsPos.y += this->centerSpeed;
    }

    // adds to the button position by whatever the centerspeed currently is if its above the pausemenu.y position (that should be the final resting position)
    if (this->buttonPos.y < this->position.y) {
        this->buttonPos.y += this->centerSpeed;
    }
}

void PauseMenu::DrawPauseMenu()
{
    // idle resting positions
    Vector2 drawPos;

    // checkerboard bg
    this->inkEffect = INK_ALPHA;
    drawPos.x = 0;
    drawPos.y = 0;
    this->checkerboardBGAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 8, false, 0);
    this->checkerboardBGAnimator.DrawSprite(&drawPos, true);

    this->inkEffect = INK_NONE;
    // side triangles
    drawPos.x = -TO_FIXED(1);
    drawPos.y = TO_FIXED(0);
    // leaving this as a note for myself, set forceApply to FALSE if you want the animation to actually play properly LMAO, otherwise it just seems to reset every frame
    this->trianglesLeftAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 1, false, 0);
    this->trianglesLeftAnimator.DrawSprite(&drawPos, true);

    drawPos.x = TO_FIXED(379);

    //i would use just one animator for these but apparently the animation cant play if i do that so two different triangle animators it is
    this->trianglesRightAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 2, false, 0);
    this->trianglesRightAnimator.DrawSprite(&drawPos, true);

    drawPos = this->position;

    //bg circles
    this->bgCircleAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 0, true, 0);
    this->bgCircleAnimator.DrawSprite(&drawPos, false);

    drawPos.y = this->position.y - TO_FIXED(11);

    this->bgCircleAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 0, true, 1);
    this->bgCircleAnimator.DrawSprite(&drawPos, false);

    // "PAUSED" text, stole your comment hehe
    drawPos.y = this->position.y - TO_FIXED(70);
    this->pauseTextAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 5, true, 0);
    this->pauseTextAnimator.DrawSprite(&drawPos, false);

    //emeralds
    drawPos.y = this->position.y + TO_FIXED(62);
    SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();
    this->emeraldsAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 6, true, 0);
    // i still dont know how get_bit works or what exactly this code is doing but i do know this is how to get the correct amount of emeralds displaying, so it is what it is
    for (int32 e = 0; e < 7; ++e) {
        if (GET_BIT(saveRAM->collectedEmeralds, e)) {
            this->emeraldsAnimator.frameID = e;
        }
        else if (saveRAM->collectedEmeralds == 0) {
            this->emeraldsAnimator.frameID = 7;
        }
    }
    this->emeraldsAnimator.DrawSprite(&drawPos, false);

    //changes the button animation depending on if its selected or not
    for (int32 i = 0; i < this->buttonCount; ++i) {
        if (!this->buttonPtrs[i])
            break;

        UIButton *button = this->buttonPtrs[i];
        if (button->isSelected) {
            button->buttonListID = 4;
        }
        else {
            button->buttonListID = 3;
        }
    }

    this->triangleSpeed = 0;
    this->centerSpeed   = 0;
}

void PauseMenu::DrawEndPause()
{ 
    // pretty much just a reverse of drawstartpause lol
    // increases the speed instead of decreasing it
    // if checks arent needed for the positions as theyre all supposed to go off screen and not visible anyway, and the entity is destroyed moments later
    if (this->triangleSpeed >= 0) {
        this->triangleSpeed += TO_FIXED(1);
    }
    if (this->triangleSpeed < 0) {
        this->triangleSpeed = 0;
    }

    if (this->centerSpeed >= 0) {
        this->centerSpeed += TO_FIXED(1);
    }
    if (this->centerSpeed < 0) {
        this->centerSpeed = 0;
    }

    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    Vector2 drawPos;

    // checkerboard bg
    this->inkEffect = INK_ALPHA;
    // decreases alpha for the checkerboard
    if (this->alpha <= 96) {
        this->alpha -= 8;
    }
    drawPos.x = 0;
    drawPos.y = 0;
    this->checkerboardBGAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 8, false, 0);
    this->checkerboardBGAnimator.DrawSprite(&drawPos, true);

    this->inkEffect = INK_NONE;
    drawPos.x = triangleLeftPos.x;
    drawPos.y = triangleLeftPos.y;
    this->trianglesLeftAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 1, false, 0);
    this->trianglesLeftAnimator.DrawSprite(&drawPos, true);
    triangleLeftPos.x -= this->triangleSpeed;

    drawPos.x = triangleRightPos.x;
    drawPos.y = triangleRightPos.y;
    this->trianglesRightAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 2, false, 0);
    this->trianglesRightAnimator.DrawSprite(&drawPos, true);
    triangleRightPos.x += this->triangleSpeed;

    drawPos.x = bgCirclePos.x;
    drawPos.y = bgCirclePos.y;
    this->bgCircleAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 0, true, 0);
    this->bgCircleAnimator.DrawSprite(&drawPos, true);
    drawPos.y = bgCirclePos.y - TO_FIXED(11);
    this->bgCircleAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 0, true, 1);
    this->bgCircleAnimator.DrawSprite(&drawPos, true);
    bgCirclePos.y += this->centerSpeed;

    drawPos.x = pauseTextPos.x;
    drawPos.y = pauseTextPos.y;
    this->pauseTextAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 5, true, 0);
    this->pauseTextAnimator.DrawSprite(&drawPos, true);
    pauseTextPos.y += this->centerSpeed;

    drawPos.x = emeraldsPos.x;
    drawPos.y = emeraldsPos.y;
    SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();
    this->emeraldsAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 6, true, 0);
    for (int32 e = 0; e < 7; ++e) {
        if (GET_BIT(saveRAM->collectedEmeralds, e)) {
            this->emeraldsAnimator.frameID = e;
        }
        else if (saveRAM->collectedEmeralds == 0) {
            this->emeraldsAnimator.frameID = 7;
        }
    }
    this->emeraldsAnimator.DrawSprite(&drawPos, true);
    emeraldsPos.y += this->centerSpeed;

    this->buttonPos.y += this->centerSpeed;
}

void PauseMenu::Draw_RegularPause()
{
    //actually draws the bg tint and sets it up with the lookup table
    if (!this->state.Matches(&PauseMenu::State_HandleFadeout)) {
        RSDKTable->SetTintLookupTable(sVars->tintLookupTable);
        Graphics::DrawRect(0, 0, screenInfo->size.x, screenInfo->size.y, 0, this->tintAlpha, INK_TINT, true);

        // calls the start draw function whenever first paused, and after 32 frames it will be set to the idle function if the current state isnt resuming
        if (this->pauseTimer >= 32) {
            if (!this->state.Matches(&PauseMenu::State_Resume)) {
                PauseMenu::DrawPauseMenu();
            }
        }
        else {
            PauseMenu::DrawStartPause();
            this->pauseTimer++;
        }

        // if the state of the object is currently resuming, then it calls the end of the drawing function where everything slides away
        if (this->state.Matches(&PauseMenu::State_Resume)) {
            PauseMenu::DrawEndPause();
        }
    }
}

void PauseMenu::Draw_ForcePause()
{
    //actually draws the bg tint and sets it up with the lookup table
    //ONLY draws the bg tint with none of the other pause menu stuff, as a forced pause's only purpose is to have the controller be reconnected
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