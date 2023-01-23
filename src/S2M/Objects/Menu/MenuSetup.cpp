// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: MenuSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "MenuSetup.hpp"
#include "Helpers/MenuParam.hpp"
#include "UIControl.hpp"
#include "UILoadingIcon.hpp"
#include "ManiaModeMenu.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Global/Localization.hpp"
#include "Global/Music.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(MenuSetup);

void MenuSetup::Update()
{
    this->state.Run(this);

    if (this->timer >= this->delay) {
        if (this->callback)
            this->callback();

        this->Destroy();
    }
    else {
        this->timer++;
    }
}

void MenuSetup::LateUpdate() {}

void MenuSetup::StaticUpdate()
{
    if (!sVars->initializedAPI) {
        sVars->fxFade->speedOut = 0;

        String tag = {};
        tag.Set("Main Menu");

        UIControl *mainMenu = nullptr; // this will crash if no entities have "Main Menu" as the tag, make sure one does!!!
        for (auto control : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES)) {
            if (tag.Compare(&tag, &control->tag, false)) {
                mainMenu = control;
                break;
            }
        }

        if (mainMenu) {
            if (!ManiaModeMenu::InitAPI()) {
                mainMenu->selectionDisabled = true;
                return;
            }
            else {
                mainMenu->selectionDisabled = false;
                sVars->initializedAPI       = true;

                String message;
                Localization::GetString(&message, Localization::RPC_Menu);
                API::RichPresence::Set(PRESENCE_MENU, &message);
            }
        }
        else {
            Dev::Print(Dev::PRINT_NORMAL, "NO MAIN MENU CONTROL FOUND.\n");
        }
    }

    if (!sVars->initializedMenu) {
        ManiaModeMenu::Initialize();
        sVars->initializedMenu = true;
    }

    if (!sVars->initializedMenuReturn) {
        ManiaModeMenu::HandleMenuReturn();
        sVars->initializedMenuReturn = true;

        if (!globals->suppressAutoMusic)
            ManiaModeMenu::ChangeMenuTrack();

            globals->suppressAutoMusic = false;
    }

    sVars->fxFade->speedOut = 12;
    API::Auth::GetUserAuthStatus();
}

void MenuSetup::Draw() { Graphics::FillScreen(this->fadeColor, this->fadeTimer, this->fadeTimer - 128, this->fadeTimer - 256); }

void MenuSetup::Create(void *data)
{
    this->active    = ACTIVE_ALWAYS;
    this->visible   = true;
    this->drawGroup = 14;
}

void MenuSetup::StageLoad()
{
    MenuParam *param = (MenuParam *)globals->menuParam;
    LogHelpers::Print("Menu recall ctrl: %s", param->menuTag);

    sVars->initializedMenuReturn = false;
    sVars->initializedMenu       = false;
    sVars->initializedAPI        = false;
    sVars->initializedSaves      = false;
    sVars->fxFade                = NULL;

    if (!globals->suppressAutoMusic) {
        Music::Stop();
        Music::sVars->activeTrack = Music::TRACK_NONE;
    }

    if (!sceneInfo->inEditor) {
        switch (SKU->platform) {
            case PLATFORM_PC: LogHelpers::Print("PC SKU"); break;
            case PLATFORM_PS4: LogHelpers::Print("PS4 SKU"); break;
            case PLATFORM_XB1: LogHelpers::Print("XB1 SKU"); break;
            case PLATFORM_SWITCH: LogHelpers::Print("NX SKU"); break;
            case PLATFORM_DEV: LogHelpers::Print("DEV SKU"); break;
            default: LogHelpers::Print("INVALID PLATFORM: %d", SKU->platform); break;
        }

        switch (SKU->region) {
            case REGION_US: LogHelpers::Print("US REGION"); break;
            case REGION_JP: LogHelpers::Print("JP REGION"); break;
            case REGION_EU: LogHelpers::Print("EU REGION"); break;
            default: LogHelpers::Print("INVALID REGION: %d", SKU->region); break;
        }
    }

    // Bug Details(?):
    // sizeof(globals->noSaveSlot) and sizeof(saveData) is 4096 (sizeof(int32) * 0x400)
    // but the memset size is only 1024 (sizeof(uint8) * 0x400)
    // so only about 1/4th of the save slot is cleared, though nothin uses the extra space so it's not a big deal
    memset(globals->noSaveSlot, 0, 0x400);

    globals->continues      = 0;
    globals->recallEntities = false;
    Graphics::SetVideoSetting(VIDEOSETTING_SCREENCOUNT, 1);

    for (auto fade : GameObject::GetEntities<FXFade>(FOR_ALL_ENTITIES)) {
        MenuSetup::sVars->fxFade = fade;
    }
}

void MenuSetup::StartTransition(void (*callback)(), int32 delay)
{
    MenuSetup *menuSetup = GameObject::Create<MenuSetup>(nullptr, -0x100000, -0x100000);

    menuSetup->active    = ACTIVE_ALWAYS;
    menuSetup->fadeColor = 0x000000;
    menuSetup->fadeShift = 5;
    menuSetup->delay     = delay;
    menuSetup->state.Set(&ManiaModeMenu::State_HandleTransition);
    menuSetup->callback  = callback;
}

void MenuSetup::SaveFileCB(bool32 success)
{
    UILoadingIcon::FinishWait();
    Stage::LoadScene();
}

#if RETRO_INCLUDE_EDITOR
void MenuSetup::EditorDraw() {}

void MenuSetup::EditorLoad() {}
#endif

void MenuSetup::Serialize() {}
} // namespace GameLogic