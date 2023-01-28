// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: MainMenu Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "MainMenu.hpp"
#include "UIControl.hpp"
#include "UIShifter.hpp"
#include "UIButton.hpp"
#include "UIDiorama.hpp"
#include "UIDialog.hpp"
#include "UIWidgets.hpp"
#include "MenuSetup.hpp"
#include "ManiaModeMenu.hpp"
#include "Helpers/MathHelpers.hpp"
#include "Global/Localization.hpp"
#include "Global/Music.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(MainMenu);

void MainMenu::Update() {}

void MainMenu::LateUpdate() {}

void MainMenu::StaticUpdate()
{
    UIControl *control = MainMenu::sVars->menuControl;

    if (control && control->active) {
        UIDiorama *diorama = MainMenu::sVars->diorama;
        UIButton *button   = control->buttons[control->lastButtonID];

        if (button) {
            int32 selectedID = button->nameFrameID;

            switch (selectedID) {
                case 0: diorama->dioramaID = UIDiorama::UIDIORAMA_ADVENTURE; break;
                case 1: diorama->dioramaID = UIDiorama::UIDIORAMA_TIMEATTACK; break;
                case 2: diorama->dioramaID = UIDiorama::UIDIORAMA_OPTIONS; break;
                case 3: diorama->dioramaID = UIDiorama::UIDIORAMA_EXTRAS; break;
                case 4: diorama->dioramaID = UIDiorama::UIDIORAMA_EXIT; break;
                default: break;
            }

            if (button->disabled)
                diorama->timer = 12;

            for (int i = 0; i < control->buttonCount; ++i) {
                if (control->buttons[i]) {
                    UIButton* button = control->buttons[i]; 
                    if (button->nameFrameID > selectedID) {
                        button->position.y = button->startPos.y + TO_FIXED(24);
                        button->buttonFrameID = button->nameFrameID;
                        button->descriptionListID = 1;
                        button->buttonListID = 1;
                        button->nameListID   = 3;
                    }
                    else if (button->nameFrameID == selectedID) {
                        button->buttonListID = 0;
                        button->position.y = button->startPos.y;
                        button->buttonFrameID = button->nameFrameID;
                        button->descriptionListID = 0;
                        button->nameListID    = 2;
                        // big boy
                    }
                    else {
                        button->buttonListID = 1;
                        button->nameListID    = 4;
                        button->descriptionListID = 1;
                        button->position.y = button->startPos.y;
                        button->buttonFrameID = button->nameFrameID + 5;
                    }
                }
            }
        }
    }
}

void MainMenu::Draw() {}

void MainMenu::Create(void *data) {}

void MainMenu::StageLoad() {}

void MainMenu::Initialize()
{
    String text = {};

    for (auto control : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES))
    {
        text.Set("Main Menu");
        if (text.Compare(&text, &control->tag, false)) {
            MainMenu::sVars->menuControl = control;
            control->backPressCB.Set(&MainMenu::BackPressCB_ReturnToTitle);
        }
    }

   UIControl *menuControl = MainMenu::sVars->menuControl;

    for (auto prompt : GameObject::GetEntities<UIButtonPrompt>(FOR_ALL_ENTITIES))
    {
        int32 x = menuControl->startPos.x - menuControl->cameraOffset.x;
        int32 y = menuControl->startPos.y - menuControl->cameraOffset.y;

        Hitbox hitbox;
        hitbox.right  = (menuControl->size.x >> 17);
        hitbox.left   = -(menuControl->size.x >> 17);
        hitbox.bottom = (menuControl->size.y >> 17);
        hitbox.top    = -(menuControl->size.y >> 17);

        if (MathHelpers::PointInHitbox(x, y, prompt->position.x, prompt->position.y, FLIP_NONE, &hitbox) && !prompt->buttonID)
            MainMenu::sVars->confirmPrompt = prompt;
    }

    for (auto diorama : GameObject::GetEntities<UIDiorama>(FOR_ALL_ENTITIES))
    {
        int32 x = menuControl->startPos.x - menuControl->cameraOffset.x;
        int32 y = menuControl->startPos.y - menuControl->cameraOffset.y;

        Hitbox hitbox;
        hitbox.right  = (menuControl->size.x >> 17);
        hitbox.left   = -(menuControl->size.x >> 17);
        hitbox.bottom = (menuControl->size.y >> 17);
        hitbox.top    = -(menuControl->size.y >> 17);

        if (MathHelpers::PointInHitbox(x, y, diorama->position.x, diorama->position.y, FLIP_NONE, &hitbox)) {
            MainMenu::sVars->diorama = diorama;
            diorama->parent   = MainMenu::sVars->menuControl;
        }
    }

    /*int32 button1Frame = 1; // Time Attack
    int32 button2Frame = 2; // Competition
    int32 button3Frame = 3; // Options
    int32 button4Frame = 4; // Extras
    int32 button5Frame = 6; // Buy Plus

    bool32 button3StopMus    = false; // Options button does NOT stop music
    bool32 button5Transition = false; // Buy Plus Does NOT do a transition

        button1Frame = 5; // Encore Mode
        button2Frame = 1; // Time Attack
        button3Frame = 2; // Competition
        button4Frame = 3; // Options
        button5Frame = 4; // Extras

        button3StopMus    = true; // Competition button stops music
        button5Transition = true; // Extras Does a transition


    UIButton *buttonManiaMode = menuControl->buttons[0];
    buttonManiaMode->frameID        = 0;
    buttonManiaMode->transition     = true;
    buttonManiaMode->stopMusic      = true;

    UIButton *buttonEncoreMode = menuControl->buttons[1];
    buttonEncoreMode->frameID        = button1Frame;
    buttonEncoreMode->transition     = true;
    buttonEncoreMode->stopMusic      = true;

    UIButton *buttonTimeAttack = menuControl->buttons[2];
    buttonTimeAttack->frameID        = button2Frame;
    buttonTimeAttack->transition     = true;
    buttonTimeAttack->stopMusic      = true;

    UIButton *buttonCompetition = menuControl->buttons[3];
    buttonCompetition->frameID        = button3Frame;
    buttonCompetition->transition     = true;
    buttonCompetition->stopMusic      = button3StopMus;

    UIButton *buttonOptions = menuControl->buttons[4];
    buttonOptions->frameID        = button4Frame;
    buttonOptions->transition     = true;
    buttonOptions->stopMusic      = false;

    UIButton *buttonExtras = menuControl->buttons[5];
    buttonExtras->frameID        = button5Frame;
    buttonExtras->transition     = button5Transition;
    buttonExtras->stopMusic      = false;

    UIButton *buttonExit = menuControl->buttons[6];
    buttonExit->frameID        = 7;
    buttonExit->transition     = false;
    buttonExit->stopMusic      = false;*/

    int32 button1Frame = 1; // Time Attack
    int32 button2Frame = 2; // Options
    int32 button3Frame = 3; // Extras
    int32 button4Frame = 4; // Exit

    bool32 button2StopMus    = false; // Options button does NOT stop music

    UIButton *buttonAdventure = menuControl->buttons[0];
    buttonAdventure->nameFrameID        = 0;
    buttonAdventure->buttonFrameID      = 0;
    buttonAdventure->transition     = true;
    buttonAdventure->stopMusic      = true;

    UIButton *buttonTimeAttack = menuControl->buttons[1];
    buttonTimeAttack->nameFrameID    = button1Frame;
    buttonTimeAttack->buttonFrameID  = 1;
    buttonTimeAttack->transition     = true;
    buttonTimeAttack->stopMusic      = true;

    UIButton *buttonOptions = menuControl->buttons[2];
    buttonOptions->nameFrameID    = button2Frame;
    buttonOptions->buttonFrameID  = 2;
    buttonOptions->transition     = true;
    buttonOptions->stopMusic      = false;

    UIButton *buttonExtras = menuControl->buttons[3];
    buttonExtras->nameFrameID    = button3Frame;
    buttonExtras->buttonFrameID  = 3;
    buttonExtras->transition     = true;
    buttonExtras->stopMusic      = false;

    UIButton *buttonExit = menuControl->buttons[4];
    buttonExit->nameFrameID    = button4Frame;
    buttonExit->buttonFrameID  = 4;
    buttonExit->transition     = false;
    buttonExit->stopMusic      = false;
}

bool32 MainMenu::BackPressCB_ReturnToTitle()
{
    ManiaModeMenu::StartReturnToTitle();

    return true;
}

void MainMenu::ExitGame() { API::ExitGame(); }

void MainMenu::ExitButton_ActionCB()
{
    String msg;
    Localization::GetString(&msg, Localization::QuitWarning);

    Action<void> callbackYes = {};
    callbackYes.Set(&MainMenu::StartExitGame);

    Action<void> callbackNo = {};
    callbackNo.Set(nullptr);

    UIDialog::CreateDialogYesNo(&msg, callbackYes, callbackNo, true, true);
}

void MainMenu::StartExitGame()
{
    if (UIControl::GetUIControl())
        UIControl::GetUIControl()->state.Set(nullptr);

    Music::FadeOut(0.02f);

    MenuSetup::StartTransition(MainMenu::ExitGame, 64);
}

void MainMenu::MenuButton_ActionCB()
{
    // sneaky and stupid
    UIButton *button = (UIButton *)this;

    switch (button->nameFrameID) {
        case 0: //Mania Mode
            if (API::Storage::GetNoSave()) {
                UIControl::MatchMenuTag("No Save Mode");
            }
            else {
                UIControl *saveSelect = ManiaModeMenu::sVars->saveSelectMenu;
                saveSelect->buttonID        = 7;
                saveSelect->menuWasSetup           = false;
                ManiaModeMenu::sVars->saveSelLastButtonID = -1;

                for (int32 i = 0; i < saveSelect->buttonCount; ++i) {
                    Entity *store     = (Entity *)sceneInfo->entity;
                    sceneInfo->entity = (Entity *)saveSelect->buttons[i];
                    //UISaveSlot::HandleSaveIconChange();
                    sceneInfo->entity = store;
                }
                UIControl::MatchMenuTag("Save Select");
            }
            break;

        case 1: // Time Attack
                /*UIControl *control = TimeAttackMenu::sVars->timeAttackControl_Legacy;
                control->buttonID        = 0;
                control->menuWasSetup    = false;
                UIControl::MatchMenuTag("Time Attack Legacy");*/
            break;

        case 2: // Options
            UIControl::MatchMenuTag("Options");
            break;

        case 3: // Extras
            UIControl::MatchMenuTag("Extras");
            break;

        default: break;
    }
}

void MainMenu::HandleUnlocks()
{
    UIControl *control = MainMenu::sVars->menuControl;
}

void MainMenu::SetupActions()
{
    for (auto button : GameObject::GetEntities<UIButton>(FOR_ALL_ENTITIES))
    {
        if (button->nameListID == 3) {
            if (button->nameFrameID == 4) {
                if (SKU->platform != PLATFORM_PC && SKU->platform != PLATFORM_DEV) {
                    UIControl *control = MainMenu::sVars->menuControl;

                    // Remove "Exit" Button if not on PC or DEV platform
                    button->Destroy();
                    --control->buttonCount;
                    --control->rowCount;
                    control->buttons[4] = nullptr;
                }
                else {
                    button->actionCB.Set(&MainMenu::ExitButton_ActionCB);
                }
            }
            else {
                button->actionCB.Set(&MainMenu::MenuButton_ActionCB);
            }
        }
    }

    MainMenu::sVars->menuControl->menuSetupCB.Set(&MainMenu::MenuSetupCB);
}

void MainMenu::MenuSetupCB() { MainMenu::sVars->diorama->lastDioramaID = -1; }

#if RETRO_INCLUDE_EDITOR
void MainMenu::EditorDraw() {}

void MainMenu::EditorLoad() {}
#endif

void MainMenu::Serialize() {}
} // namespace GameLogic