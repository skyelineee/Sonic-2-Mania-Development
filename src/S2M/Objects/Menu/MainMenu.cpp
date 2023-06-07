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
#include "Common/BGSwitch.hpp"

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
                        button->buttonFrameID = button->descriptionFrameID;
                        button->descriptionListID = 4;
                        button->buttonListID = 1;
                        button->nameListID   = 3;
                    }
                    else if (button->nameFrameID == selectedID) {
                        button->buttonListID = 0;
                        button->position.y = button->startPos.y;
                        button->buttonFrameID = button->descriptionFrameID;
                        button->descriptionListID = 0;
                        button->nameListID    = 2;
                        // big boy
                    }
                    else {
                        button->buttonListID = 1;
                        button->nameListID    = 4;
                        button->descriptionListID = 4;
                        button->position.y = button->startPos.y;
                        button->buttonFrameID = button->descriptionFrameID + 5;
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

    RSDKTable->GetTileLayer(1)->drawGroup[BGSwitch::sVars->screenID] = 0;
    RSDKTable->GetTileLayer(2)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
    RSDKTable->GetTileLayer(3)->drawGroup[BGSwitch::sVars->screenID] = 1;
    RSDKTable->GetTileLayer(4)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
    RSDKTable->GetTileLayer(5)->drawGroup[BGSwitch::sVars->screenID] = 1;
    RSDKTable->GetTileLayer(6)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;
    RSDKTable->GetTileLayer(7)->drawGroup[BGSwitch::sVars->screenID] = DRAWGROUP_COUNT;

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

    int32 button1Frame = 1; // Time Attack
    int32 button2Frame = 2; // Extras
    int32 button3Frame = 3; // Options
    int32 button4Frame = 4; // Exit

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

    UIButton *buttonExtras      = menuControl->buttons[2];
    buttonExtras->nameFrameID   = button2Frame;
    buttonExtras->buttonFrameID = 2;
    buttonExtras->transition    = true;
    buttonExtras->stopMusic     = false;

    UIButton *buttonOptions = menuControl->buttons[3];
    buttonOptions->nameFrameID    = button3Frame;
    buttonOptions->buttonFrameID  = 3;
    buttonOptions->transition     = true;
    buttonOptions->stopMusic      = false;

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
        case 0: // Adventure
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
                    sceneInfo->entity = store;
                }
                UIControl::MatchMenuTag("Save Select");
            }
            break;

        case 1: // Time Attack
                //UIControl *control = TimeAttackMenu::sVars->timeAttackControl_Legacy;
                //control->buttonID        = 0;
                //control->menuWasSetup    = false;
                UIControl::MatchMenuTag("Time Attack Legacy");
            break;

        case 2: // Extras
            UIControl::MatchMenuTag("Extras");
            break;

        case 3: // Options
            UIControl::MatchMenuTag("Options");
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