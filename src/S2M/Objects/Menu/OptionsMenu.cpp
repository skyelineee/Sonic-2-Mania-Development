// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: OptionsMenu Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "OptionsMenu.hpp"
#include "UIWidgets.hpp"
#include "UISlider.hpp"
#include "UIWinSize.hpp"
#include "UIResPicker.hpp"
#include "UIDialog.hpp"
#include "Helpers/DialogRunner.hpp"
#include "UITransition.hpp"
#include "UISubHeading.hpp"
#include "UIKeyBinder.hpp"
#include "UILoadingIcon.hpp"
#include "MenuSetup.hpp"
#include "ManiaModeMenu.hpp"
#include "Helpers/MenuParam.hpp"
#include "Helpers/Options.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Helpers/GameProgress.hpp"
#include "Global/Localization.hpp"
#include "Global/SaveGame.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(OptionsMenu);

void OptionsMenu::Update() {}
void OptionsMenu::LateUpdate() {}
void OptionsMenu::StaticUpdate() 
{
    UIControl *control = sVars->optionsControl;
    UIControl *dataControl = sVars->dataOptionsControl;

    if (control && control->active) {
        UIDiorama *diorama = sVars->diorama;
        UIButton *button   = control->buttons[control->lastButtonID];

        if (button) {
            int32 selectedID = button->nameFrameID;

            switch (selectedID) {
                case 0: diorama->dioramaID = UIDiorama::UIDIORAMA_BOSSRUSH; break;
                case 1: diorama->dioramaID = UIDiorama::UIDIORAMA_MUSICPLAYER; break;
                case 2: diorama->dioramaID = UIDiorama::UIDIORAMA_LEVELSELECT; break;
                case 3: diorama->dioramaID = UIDiorama::UIDIORAMA_EXTRALEVELS; break;
                default: break;
            }

            if (button->disabled)
                diorama->timer = 12;

            for (int i = 0; i < control->buttonCount; ++i) {
                if (control->buttons[i]) {
                    UIButton *button = control->buttons[i];
                    if (button->nameFrameID > selectedID) {
                        button->position.y        = button->startPos.y + TO_FIXED(48);
                        button->descriptionListID = 4;
                        button->buttonListID      = 9;
                        button->nameListID        = 11;
                    }
                    else if (button->nameFrameID == selectedID) {
                        button->buttonListID      = 8;
                        button->position.y        = button->startPos.y;
                        button->descriptionListID = 2;
                        button->nameListID        = 10;
                        // big boy
                    }
                    else {
                        button->buttonListID      = 9;
                        button->nameListID        = 11;
                        button->descriptionListID = 4;
                        button->position.y        = button->startPos.y;
                    }
                }
            }
        }
    }

    if (dataControl && dataControl->active) {
        UIButton *button = dataControl->buttons[dataControl->lastButtonID];

        if (button) {
            int32 selectedID = button->nameFrameID;

            for (int i = 0; i < dataControl->buttonCount; ++i) {
                if (dataControl->buttons[i]) {
                    UIButton *button = dataControl->buttons[i];
                    if (button->nameFrameID > selectedID) {
                        button->position.y        = button->startPos.y + TO_FIXED(48);
                        button->descriptionListID = 4;
                        button->buttonListID      = 9;
                        button->nameListID        = 13;
                    }
                    else if (button->nameFrameID == selectedID) {
                        button->buttonListID      = 8;
                        button->position.y        = button->startPos.y;
                        button->descriptionListID = 3;
                        button->nameListID        = 12;
                        // big boy
                    }
                    else {
                        button->buttonListID      = 9;
                        button->nameListID        = 13;
                        button->descriptionListID = 4;
                        button->position.y        = button->startPos.y;
                    }
                }
            }
        }
    }
}

void OptionsMenu::Draw() {}

void OptionsMenu::Create(void *data) {}

void OptionsMenu::StageLoad() {}

void OptionsMenu::Initialize()
{
    LogHelpers::Print("ManiaModeMenu::Initialize()");

    String tag = {};

    for (auto control : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES))
    {
        tag.Set("Options");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->optionsControl = control;

        tag.Set("Video");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->videoControl = control;

        tag.Set("Video WIN");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->videoControl_Windows = control;

        tag.Set("Sound");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->soundControl = control;

        tag.Set("Controls WIN");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->controlsControl_Windows = control;

        tag.Set("Controls KB");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->controlsControl_KB = control;

        tag.Set("Controls PS4");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->controlsControl_PS4 = control;

        tag.Set("Controls XB1");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->controlsControl_XB1 = control;

        tag.Set("Controls NX");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->controlsControl_NX = control;

        tag.Set("Controls NX Grip");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->controlsControl_NXGrip = control;

        tag.Set("Controls NX Joycon");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->controlsControl_NXJoycon = control;

        tag.Set("Controls NX Pro");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->controlsControl_NXPro = control;

        tag.Set("Data Options");
        if (tag.Compare(&tag, &control->tag, false))
            sVars->dataOptionsControl = control;
    }

    for (auto prompt : GameObject::GetEntities<UIButtonPrompt>(FOR_ALL_ENTITIES))
    {
        UIControl *controller = sVars->optionsControl;

        if (UIControl::ContainsPos(controller, &prompt->position) && prompt->buttonID == 3)
            sVars->helpPrompt = prompt;
    }

    for (auto diorama : GameObject::GetEntities<UIDiorama>(FOR_ALL_ENTITIES))
    {
        UIControl *controller = sVars->optionsControl;

        if (UIControl::ContainsPos(controller, &diorama->position)) {
            sVars->diorama = diorama;
            diorama->parent      = sVars->optionsControl;
        }
    }
}

void OptionsMenu::HandleUnlocks()
{
    UIControl *control = sVars->dataOptionsControl;
}

void OptionsMenu::SetupActions()
{
    UIControl *optionsControl         = sVars->optionsControl;
    UIControl *videoControl           = sVars->videoControl;
    UIControl *controlsControl_Win    = sVars->controlsControl_Windows;
    UIControl *videoControl_Win       = sVars->videoControl_Windows;
    UIControl *soundControl           = sVars->soundControl;
    UIControl *dataControl            = sVars->dataOptionsControl;

    String string = {};

    for (auto button : GameObject::GetEntities<UIButton>(FOR_ALL_ENTITIES))
    {
        if (UIControl::ContainsPos(controlsControl_Win, &button->position))
            button->actionCB.Set(&OptionsMenu::KeyboardIDButton_Win_ActionCB);

        if (UIControl::ContainsPos(optionsControl, &button->position) && button->nameListID == 11) {

            switch (button->nameFrameID) {
                case 0: button->actionCB.Set(&OptionsMenu::VideoMenuButton_ActionCB); break;
                case 1: button->actionCB.Set(&OptionsMenu::SoundMenuButton_ActionCB); break;

                case 2:
                    button->actionCB.Set(&OptionsMenu::ControlsMenuButton_ActionCB);
                    if (SKU->platform == PLATFORM_DEV || SKU->platform == PLATFORM_PC)
                        button->transition = false;
                    break;
                case 3: button->actionCB.Set(&OptionsMenu::DataOptionsMenuButton_ActionCB); break;
            }
        }

        if (UIControl::ContainsPos(videoControl, &button->position) && button->nameListID == 3 && button->nameFrameID == 0)
            button->choiceChangeCB.Set(&OptionsMenu::ShaderButton_ActionCB);

        if (UIControl::ContainsPos(controlsControl_Win, &button->position) && button->nameListID == 17 && button->nameFrameID == 1)
            button->actionCB.Set(&OptionsMenu::SetDefaultMappings);

        if (UIControl::ContainsPos(videoControl_Win, &button->position) && button->nameListID == 14) {
            switch (button->nameFrameID) {
                case 7: button->choiceChangeCB.Set(&OptionsMenu::ShaderButton_ActionCB); break;
                case 8: button->choiceChangeCB.Set(&OptionsMenu::WindowScaleButton_ActionCB); break;
                case 9: button->choiceChangeCB.Set(&OptionsMenu::BorderlessButton_ActionCB); break;
                case 10: button->choiceChangeCB.Set(&OptionsMenu::FullScreenButton_ActionCB); break;
                case 11: button->choiceChangeCB.Set(&OptionsMenu::VSyncButton_ActionCB); break;
                case 12: button->choiceChangeCB.Set(&OptionsMenu::TripleBufferButton_ActionCB); break;
            }
        }

        if (UIControl::ContainsPos(dataControl, &button->position) && button->nameListID == 13) {
            switch (button->nameFrameID) {
                case 0: button->actionCB.Set(&OptionsMenu::EraseSaveGameButton_ActionCB); break;
                case 1: button->actionCB.Set(&OptionsMenu::EraseTimeAttackButton_ActionCB); break;
                case 2: button->actionCB.Set(&OptionsMenu::EraseReplaysButton_ActionCB); break;
                case 3: button->actionCB.Set(&OptionsMenu::EraseAllButton_ActionCB); break;
            }
        }
    }

    for (auto slider : GameObject::GetEntities<UISlider>(FOR_ALL_ENTITIES))
    {
        if (UIControl::ContainsPos(soundControl, &slider->position) && slider->listID == 17)
            slider->sliderChangedCB.Set(&OptionsMenu::UISlider_ChangedCB);
    }

    optionsControl->menuSetupCB.Set(&OptionsMenu::MenuSetupCB);

    videoControl_Win->menuUpdateCB.Set(&OptionsMenu::VideoControl_Win_MenuUpdateCB);
    videoControl_Win->yPressCB.Set(&OptionsMenu::VideoControl_Win_YPressCB);
    videoControl_Win->backPressCB.Set(&OptionsMenu::VideoControl_Win_BackPressCB);

    if (SKU->platform == PLATFORM_SWITCH || SKU->platform == PLATFORM_DEV) {
        optionsControl->yPressCB.Set(&OptionsMenu::ShowManual);
    }
    else {
        UIButtonPrompt *prompt = sVars->helpPrompt;
        prompt->visible              = false;
    }
}

void OptionsMenu::HandleMenuReturn()
{
    UIControl *videoControl        = sVars->videoControl;
    UIControl *soundControl        = sVars->soundControl;

    if (SKU->platform == PLATFORM_PC || SKU->platform == PLATFORM_DEV)
        OptionsMenu::InitVideoOptionsMenu();

    UIButton *button = videoControl->buttons[0];
    UIButton::SetChoiceSelection(button, Graphics::GetVideoSetting(VIDEOSETTING_SHADERID));

    UISlider *musSlider = (UISlider *)soundControl->buttons[0];
    musSlider->sliderPos = Graphics::GetVideoSetting(VIDEOSETTING_STREAM_VOL);

    UISlider *sfxSlider = (UISlider *)soundControl->buttons[1];
    sfxSlider->sliderPos = Graphics::GetVideoSetting(VIDEOSETTING_SFX_VOL);
}

void OptionsMenu::InitVideoOptionsMenu()
{
    if (SKU->platform == PLATFORM_PC || SKU->platform == PLATFORM_DEV) {
        UIControl *videoControl_Win = sVars->videoControl_Windows;
        Options *optionsRAM            = Options::GetOptionsRAM();

        Options::GetWinSize();

        int32 options[7];

        options[0] = Graphics::GetVideoSetting(VIDEOSETTING_SHADERID); // filter
        options[1] = optionsRAM->windowSize;                      // window size
        options[2] = Graphics::GetVideoSetting(VIDEOSETTING_BORDERED); // bordered

        // fullscreen
        options[3] = 0;
        if (!Graphics::GetVideoSetting(VIDEOSETTING_WINDOWED) || optionsRAM->windowSize == 4)
            options[3] = 1;

        options[4] = 0;                                                 // fullscreen res
        options[5] = Graphics::GetVideoSetting(VIDEOSETTING_VSYNC);     // vsync
        options[6] = Graphics::GetVideoSetting(VIDEOSETTING_TRIPLEBUFFERED); // triple buffered

        for (int32 i = 0; i < videoControl_Win->buttonCount; ++i) {
            UIButton *button = videoControl_Win->buttons[i];

            if (i == 4) {
                UIResPicker *resPicker = (UIResPicker *)UIButton::GetChoicePtr(button, button->selection);
                UIResPicker::GetDisplayInfo(resPicker);
            }
            else if (i == 1) {
                UIWinSize *winSize = (UIWinSize *)UIButton::GetChoicePtr(button, button->selection);
                winSize->selection       = Graphics::GetVideoSetting(VIDEOSETTING_WINDOW_HEIGHT) / SCREEN_YSIZE;
            }
            else if (button->selection != options[i]) {
                UIButton::SetChoiceSelection(button, options[i]);
            }
        }
    }
}

void OptionsMenu::VideoControl_Win_MenuUpdateCB() { OptionsMenu::InitVideoOptionsMenu(); }

void OptionsMenu::VideoControl_Win_YPressCB()
{
    String message = {};

    if (Graphics::GetVideoSetting(VIDEOSETTING_CHANGED)) {
        Graphics::UpdateWindow();

        Localization::GetString(&message, Localization::VideoChangesApplied);

        Action<void> callbackYes = {};
        callbackYes.Set(&OptionsMenu::ApplyChangesDlg_Win_YesCB);

        Action<void> callbackNo = {};
        callbackNo.Set(&OptionsMenu::ApplyChangesDlg_Win_NoCB);

        UIDialog *dialog =
            UIDialog::CreateDialogYesNo(&message, callbackYes, callbackNo, true, true);
        if (dialog)
            dialog->closeDelay = 15 * 60; // 15 seconds at 60 FPS
    }
}
void OptionsMenu::DlgRunnerCB_RevertVideoChanges()
{
    String message = {};

    Graphics::UpdateWindow();

    Localization::GetString(&message, Localization::VideoChangesApplied);
    // This is bugged! Using `OptionsMenu_ApplyChangesDlg_BackPress_NoCB` causes the settings to be reverted instead of saved!
    // This should have called a modified version of `OptionsMenu_ApplyChangesDlg_Win_YesCB` which also transitions to the previous menu!

    Action<void> callbackYes = {};
    callbackYes.Set(&OptionsMenu::ApplyChangesDlg_BackPress_NoCB);

    Action<void> callbackNo = {};
    callbackNo.Set(&OptionsMenu::ApplyChangesDlg_NoCB);

    UIDialog *dialog =
        UIDialog::CreateDialogYesNo(&message, callbackYes, callbackNo, true, true);
    if (dialog)
        dialog->closeDelay = 15 * 60; // 15 seconds at 60 FPS
}

bool32 OptionsMenu::VideoControl_Win_BackPressCB()
{
    String message = {};

    if (Graphics::GetVideoSetting(VIDEOSETTING_CHANGED)) {
        Localization::GetString(&message, Localization::ApplyChangedSettings);

        Action<void> callbackYes = {};
        callbackYes.Set(&OptionsMenu::ApplyChangesDlg_BackPress_YesCB);

        Action<void> callbackNo = {};
        callbackNo.Set(&OptionsMenu::ApplyChangesDlg_BackPress_NoCB);

        UIDialog *dialog =
            UIDialog::CreateDialogYesNo(&message, callbackYes, callbackNo, true, true);
        if (dialog)
            return true;
    }
    else {
        UITransition::StartTransition(UIControl::ReturnToParentMenu, 0);
    }

    return false;
}

void OptionsMenu::ApplyChangesDlg_BackPress_YesCB()
{
    Action<void> callback      = {};
    callback.Set(&DialogRunner::HandleCallback);
    DialogRunner *dialogRunner = GameObject::Create<DialogRunner>(&callback, 0, 0);

    dialogRunner->callback.Set(&OptionsMenu::DlgRunnerCB_RevertVideoChanges);
    dialogRunner->timer       = 0;
    dialogRunner->isPermanent = true;
}

void OptionsMenu::ApplyChangesDlg_Win_NoCB()
{
    Graphics::SetVideoSetting(VIDEOSETTING_RELOAD, false);
    Graphics::UpdateWindow();

    OptionsMenu::InitVideoOptionsMenu();

    Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, false);
}

void OptionsMenu::ApplyChangesDlg_NoCB()
{
    Graphics::SetVideoSetting(VIDEOSETTING_RELOAD, false);
    Graphics::UpdateWindow();

    OptionsMenu::InitVideoOptionsMenu();

    Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, false);
    UITransition::StartTransition(UIControl::ReturnToParentMenu, 0);
}

void OptionsMenu::ApplyChangesDlg_Win_YesCB()
{
    Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, false);
    Graphics::SetVideoSetting(VIDEOSETTING_WRITE, true);
    Graphics::SetVideoSetting(VIDEOSETTING_STORE, false);
}

void OptionsMenu::ApplyChangesDlg_BackPress_NoCB()
{
    Graphics::SetVideoSetting(VIDEOSETTING_RELOAD, false);

    OptionsMenu::InitVideoOptionsMenu();

    Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, false);
    UITransition::StartTransition(UIControl::ReturnToParentMenu, 0);
}

void OptionsMenu::VideoMenuButton_ActionCB()
{
    if (SKU->platform == PLATFORM_PC || SKU->platform == PLATFORM_DEV) {
        Graphics::SetVideoSetting(VIDEOSETTING_STORE, false);
        UIControl::MatchMenuTag("Video WIN");
    }
    else {
        UIControl::MatchMenuTag("Video");
    }
}
void OptionsMenu::SoundMenuButton_ActionCB() { UIControl::MatchMenuTag("Sound"); }
void OptionsMenu::DataOptionsMenuButton_ActionCB() { UIControl::MatchMenuTag("Data Options"); }
void OptionsMenu::ControlsMenuButton_DefaultKB_ActionCB() { UIControl::MatchMenuTag("Controls WIN"); }
void OptionsMenu::ControlsMenuButton_Default_ActionCB() { UIControl::MatchMenuTag("Controls XB1"); }

void OptionsMenu::ControlsMenuButton_ActionCB()
{
    int32 id          = Input::GetFilteredInputDeviceID(true, false, 5);
    int32 gamepadType = Input::GetInputDeviceType(id);

    String message = {};

    switch ((gamepadType >> 8) & 0xFF) {
        default:
        case Input::DEVICE_TYPE_CONTROLLER:
            switch (gamepadType & 0xFF) {
                case Input::DEVICE_XBOX: UITransition::SetNewTag("Controls XB1"); break;
                case Input::DEVICE_PS4: UITransition::SetNewTag("Controls PS4"); break;
                case Input::DEVICE_SATURN: UITransition::SetNewTag("Controls WIN"); break;
                case Input::DEVICE_SWITCH_HANDHELD: UITransition::SetNewTag("Controls NX"); break;
                case Input::DEVICE_SWITCH_JOY_GRIP: UITransition::SetNewTag("Controls NX Grip"); break;
                case Input::DEVICE_SWITCH_JOY_L:
                case Input::DEVICE_SWITCH_JOY_R: UITransition::SetNewTag("Controls NX Joycon"); break;
                case Input::DEVICE_SWITCH_PRO: UITransition::SetNewTag("Controls NX Pro"); break;
                default: UITransition::StartTransition(OptionsMenu::ControlsMenuButton_Default_ActionCB, 0); break;
            }
            break;

        case Input::DEVICE_TYPE_KEYBOARD: UITransition::StartTransition(OptionsMenu::ControlsMenuButton_DefaultKB_ActionCB, 0); break;

        case Input::DEVICE_TYPE_STEAMOVERLAY:
            if (!API::IsOverlayEnabled(id)) {
                Localization::GetString(&message, Localization::SteamOverlayUnavailable);

                Action<void> callback = {};
                callback.Set(nullptr);

                UIDialog::CreateDialogOk(&message, callback, true);
            }
            break;
    }
}

void OptionsMenu::SetDefaultMappings()
{
    controllerInfo[Input::CONT_P1].keyUp.keyMap     = Input::KEYMAP_UP;
    controllerInfo[Input::CONT_P1].keyDown.keyMap   = Input::KEYMAP_DOWN;
    controllerInfo[Input::CONT_P1].keyLeft.keyMap   = Input::KEYMAP_LEFT;
    controllerInfo[Input::CONT_P1].keyRight.keyMap  = Input::KEYMAP_RIGHT;
    controllerInfo[Input::CONT_P1].keyA.keyMap      = Input::KEYMAP_A;
    controllerInfo[Input::CONT_P1].keyB.keyMap      = Input::KEYMAP_S;
    controllerInfo[Input::CONT_P1].keyC.keyMap      = Input::KEYMAP_NO_MAPPING;
    controllerInfo[Input::CONT_P1].keyX.keyMap      = Input::KEYMAP_Q;
    controllerInfo[Input::CONT_P1].keyY.keyMap      = Input::KEYMAP_W;
    controllerInfo[Input::CONT_P1].keyZ.keyMap      = Input::KEYMAP_NO_MAPPING;
    controllerInfo[Input::CONT_P1].keyStart.keyMap  = Input::KEYMAP_RETURN;
    controllerInfo[Input::CONT_P1].keySelect.keyMap = Input::KEYMAP_TAB;

    controllerInfo[Input::CONT_P2].keyUp.keyMap     = Input::KEYMAP_NUMPAD8;
    controllerInfo[Input::CONT_P2].keyDown.keyMap   = Input::KEYMAP_NUMPAD5;
    controllerInfo[Input::CONT_P2].keyLeft.keyMap   = Input::KEYMAP_NUMPAD4;
    controllerInfo[Input::CONT_P2].keyRight.keyMap  = Input::KEYMAP_NUMPAD6;
    controllerInfo[Input::CONT_P2].keyA.keyMap      = Input::KEYMAP_J;
    controllerInfo[Input::CONT_P2].keyB.keyMap      = Input::KEYMAP_K;
    controllerInfo[Input::CONT_P2].keyC.keyMap      = Input::KEYMAP_NO_MAPPING;
    controllerInfo[Input::CONT_P2].keyX.keyMap      = Input::KEYMAP_U;
    controllerInfo[Input::CONT_P2].keyY.keyMap      = Input::KEYMAP_I;
    controllerInfo[Input::CONT_P2].keyZ.keyMap      = Input::KEYMAP_NO_MAPPING;
    controllerInfo[Input::CONT_P2].keyStart.keyMap  = Input::KEYMAP_OEM_4;
    controllerInfo[Input::CONT_P2].keySelect.keyMap = Input::KEYMAP_OEM_6;

    Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, true);
}

void OptionsMenu::SetupKBControlsMenu(int32 playerID)
{
    UIControl *control = sVars->controlsControl_KB;

    for (auto subHeading : GameObject::GetEntities<UISubHeading>(FOR_ALL_ENTITIES))
    {
        if (UIControl::ContainsPos(control, &subHeading->position)) {
            subHeading->frameID = 19 + playerID;
            break;
        }
    }

    for (int32 b = 0; b < control->buttonCount; ++b) {
        UIKeyBinder *binder = (UIKeyBinder *)control->buttons[b];

        if (binder->classID == UIKeyBinder::sVars->classID)
            binder->inputID = playerID;
    }
}

void OptionsMenu::KeyboardIDButton_Win_ActionCB()
{
    UIButton *button = (UIButton *)this;

    UIControl *control = sVars->controlsControl_Windows;

    for (int32 i = 0; i < control->buttonCount; ++i) {
        if (button == control->buttons[i]) {
            OptionsMenu::SetupKBControlsMenu(i);
            UIControl::MatchMenuTag("Controls KB");
        }
    }
}

void OptionsMenu::MenuSetupCB()
{
    if (Options::sVars->changed) {
        UILoadingIcon::StartWait();
        Options::SaveOptionsBin(OptionsMenu::SaveOptionsCB_Load);
    }

    UIControl *control = sVars->optionsControl;

    UIButton *eraseDataButton = control->buttons[4];
    if (eraseDataButton)
        eraseDataButton->disabled = API::Storage::GetNoSave();
}

void OptionsMenu::SaveOptionsCB_Load(bool32 success) { UILoadingIcon::FinishWait(); }

void OptionsMenu::TransitionCB_ReloadScene()
{
    globals->suppressAutoMusic = true;

    Stage::ForceHardReset(true);
    Stage::LoadScene();
}

void OptionsMenu::SaveOptionsCB_Action(bool32 success)
{
    MenuParam *param = MenuParam::GetMenuParam();

    UILoadingIcon::FinishWait();

    //TimeAttackData::Clear();

    strcpy(param->menuTag, "Options");
    MenuSetup::StartTransition(OptionsMenu::TransitionCB_ReloadScene, 32);
}

void OptionsMenu::ShaderButton_ActionCB()
{
    UIButton *button = (UIButton *)this;

    Options *options = Options::GetOptionsRAM();

    options->screenShader   = button->selection;
    options->overrideShader = true;

    Graphics::SetVideoSetting(VIDEOSETTING_SHADERID, button->selection);
    Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, false);
}

void OptionsMenu::WindowScaleButton_ActionCB()
{
    UIButton *button = (UIButton *)this;

    Options *options = Options::GetOptionsRAM();
    if (button->selection != 4) {
        Graphics::SetVideoSetting(VIDEOSETTING_WINDOW_WIDTH, WIDE_SCR_XSIZE * (button->selection + 1));
        Graphics::SetVideoSetting(VIDEOSETTING_WINDOW_HEIGHT, SCREEN_YSIZE * (button->selection + 1));

        options->windowSize = button->selection;
        Options::sVars->changed    = true;
    }
}

void OptionsMenu::BorderlessButton_ActionCB()
{
    UIButton *button = (UIButton *)this;

    Options *options = Options::GetOptionsRAM();

    options->windowBorder = button->selection;
    Graphics::SetVideoSetting(VIDEOSETTING_BORDERED, button->selection);

    Options::sVars->changed = true;
}

void OptionsMenu::FullScreenButton_ActionCB()
{
    UIButton *button = (UIButton *)this;

    Options *options = Options::GetOptionsRAM();

    options->windowed = button->selection ^ 1;
    Graphics::SetVideoSetting(VIDEOSETTING_WINDOWED, button->selection ^ 1);

    Options::sVars->changed = true;
}

void OptionsMenu::VSyncButton_ActionCB()
{
    UIButton *button = (UIButton *)this;

    Options *options = Options::GetOptionsRAM();

    options->vSync = button->selection;
    Graphics::SetVideoSetting(VIDEOSETTING_VSYNC, button->selection);

    Options::sVars->changed = true;
}

void OptionsMenu::TripleBufferButton_ActionCB()
{
    UIButton *button = (UIButton *)this;

    Options *options = Options::GetOptionsRAM();

    options->tripleBuffering = button->selection;
    Graphics::SetVideoSetting(VIDEOSETTING_TRIPLEBUFFERED, button->selection);

    Options::sVars->changed = true;
}

void OptionsMenu::UISlider_ChangedCB()
{
    UISlider *slider = (UISlider *)this;

    Options *options = Options::GetOptionsRAM();

    // Bug Details (?):
    // what the hell is up with this???????
    // it'd only ever be 0 or 1 why are 0xF1, 0xF2, 0xF4, 0xF5 & 0xFC options?????
    // this is a CB for the slider why are the boolean values here???

    bool32 value = slider->frameID != 1;
    switch (value) {
        case 0xF1:
            options->windowed = slider->sliderPos;
            Graphics::SetVideoSetting(VIDEOSETTING_WINDOWED, options->windowed);
            break;

        case 0xF2:
            options->windowBorder = slider->sliderPos;
            Graphics::SetVideoSetting(VIDEOSETTING_BORDERED, options->windowBorder);
            break;

        case 0xF4:
            options->vSync = slider->sliderPos;
            Graphics::SetVideoSetting(VIDEOSETTING_VSYNC, options->vSync);
            break;

        case 0xF5:
            options->tripleBuffering = slider->sliderPos;
            Graphics::SetVideoSetting(VIDEOSETTING_TRIPLEBUFFERED, options->tripleBuffering);
            break;
        case 0xFC:
            options->screenShader   = slider->sliderPos;
            options->overrideShader = true;

            Graphics::SetVideoSetting(VIDEOSETTING_SHADERID, options->screenShader);
            Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, false);
            break;

        case 0:
            options->volMusic         = slider->sliderPos;
            options->overrideMusicVol = true;
            Graphics::SetVideoSetting(VIDEOSETTING_STREAM_VOL, options->volMusic);
            break;

        case 1:
            options->volSfx         = slider->sliderPos;
            options->overrideSfxVol = true;
            Graphics::SetVideoSetting(VIDEOSETTING_SFX_VOL, options->volSfx);
            break;

        default: break;
    }

    Options::sVars->changed = true;
}

void OptionsMenu::ShowManual()
{
    UIWidgets::sVars->sfxAccept.Play(false, 0xFF);
    API::LaunchManual();
}

void OptionsMenu::EraseSaveDataCB(bool32 success)
{
    String message;
    UIControl *control = OptionsMenu::sVars->dataOptionsControl;

    UILoadingIcon::FinishWait();
    if (success) {
        ManiaModeMenu::StartReturnToTitle();
    }
    else {
        control->selectionDisabled = false;
        message.Init("ERROR ERASING DATA.", 0);

        Action<void> callback = {};
        callback.Set(nullptr);

        UIDialog::CreateDialogOk(&message, callback, true);
    }
}

void OptionsMenu::AreYouSureDlg_YesCB_EraseSaveGame()
{
    // Bug Details(?):
    // sizeof(globals->noSaveSlot) and sizeof(saveData) is 4096 (sizeof(int32) * 0x400)
    // but the memset size is only 1024 (sizeof(uint8) * 0x400)
    // so only about 1/4th of the save slot is cleared, though nothin uses the extra space so it's not a big deal

    for (int32 i = 0; i < 8; ++i) {
        SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveDataPtr(i);
        memset(saveRAM, 0, 0x400);
    }

    memset(globals->noSaveSlot, 0, 0x400);
    globals->continues = 0;

    SaveGame::SaveFile(OptionsMenu::EraseSaveDataCB);
}

void OptionsMenu::AreYouSureDlg_YesCB_EraseAllData()
{
    for (int32 i = 0; i < 8; ++i) {
        SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveDataPtr(i);
        memset(saveRAM, 0, 0x400);
    }

    memset(globals->noSaveSlot, 0, 0x400);
    globals->continues = 0;

    GameProgress::ClearProgress();

    APITable->RemoveAllDBRows(globals->taTableID);

    SaveGame::SaveFile(OptionsMenu::EraseSaveDataCB);
}

void OptionsMenu::EraseSaveGameButton_ActionCB()
{
    String message = {};

    Localization::GetString(&message, Localization::AreYouSureSave);

    Action<void> callbackYes = {};
    callbackYes.Set(&OptionsMenu::AreYouSureDlg_YesCB_EraseSaveGame);

    Action<void> callbackNo = {};
    callbackNo.Set(nullptr);

    UIDialog::CreateDialogYesNo(&message, callbackYes, callbackNo, true, true);
}

void OptionsMenu::AreYouSureDlg_YesCB_EraseTimeAttack()
{
    UIControl *control   = sVars->dataOptionsControl;
    control->selectionDisabled = true;

    UILoadingIcon::StartWait();
    APITable->RemoveAllDBRows(globals->taTableID);

    //TimeAttackData::SaveDB(OptionsMenu::EraseSaveDataCB);
    LogHelpers::Print("TimeAttack table ID = %d, status = %d", globals->taTableID, globals->taTableLoaded);
}

void OptionsMenu::EraseTimeAttackButton_ActionCB()
{
    String message = {};

    Localization::GetString(&message, Localization::AreYouSureSave);

    Action<void> callbackYes = {};
    callbackYes.Set(&OptionsMenu::AreYouSureDlg_YesCB_EraseTimeAttack);

    Action<void> callbackNo = {};
    callbackNo.Set(nullptr);

    UIDialog::CreateDialogYesNo(&message, callbackYes, callbackNo, true, true);
}

void OptionsMenu::AreYouSureDlg_YesCB_EraseReplays()
{
    UIControl *control = sVars->dataOptionsControl;

    if (!control->selectionDisabled) {
        control->selectionDisabled = true;
        UILoadingIcon::StartWait();
    }

    APITable->SetupUserDBRowSorting(globals->replayTableID);
    APITable->SetupUserDBRowSorting(globals->taTableID);

    if (APITable->GetSortedUserDBRowCount(globals->replayTableID) <= 0) {
        //ReplayDB::SaveDB(OptionsMenu::EraseReplaysCB);
    }
    else {
        int32 row = APITable->GetSortedUserDBRowID(globals->replayTableID, 0);
        //ReplayDB::DeleteReplay(row, OptionsMenu::EraseReplaysCB, true);
    }
}

void OptionsMenu_EraseReplaysCB(bool32 success) { //TimeAttackData::SaveDB(OptionsMenu::EraseSaveDataCB);
}

void OptionsMenu::EraseReplaysButton_ActionCB()
{
    String message = {};

    Localization::GetString(&message, Localization::AreYouSureSave);

    Action<void> callbackYes = {};
    callbackYes.Set(&OptionsMenu::AreYouSureDlg_YesCB_EraseReplays);

    Action<void> callbackNo = {};
    callbackNo.Set(nullptr);

    UIDialog::CreateDialogYesNo(&message, callbackYes, callbackNo, true, true);
}

void OptionsMenu::EraseAllButton_ActionCB()
{
    String message = {};

    Localization::GetString(&message, Localization::AreYouSureSave);

    Action<void> callbackYes = {};
    callbackYes.Set(&OptionsMenu::AreYouSureDlg_YesCB_EraseAllData);

    Action<void> callbackNo = {};
    callbackNo.Set(nullptr);

    UIDialog::CreateDialogYesNo(&message, callbackYes, callbackNo, true, true);
}

#if RETRO_INCLUDE_EDITOR
void OptionsMenu::EditorDraw() {}

void OptionsMenu::EditorLoad() {}
#endif

void OptionsMenu::Serialize() {}

} // namespace GameLogic