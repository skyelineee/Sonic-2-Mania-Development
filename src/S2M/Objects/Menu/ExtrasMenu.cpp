// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ExtrasMenu Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "ExtrasMenu.hpp"
#include "MenuSetup.hpp"
#include "UIControl.hpp"
#include "UIChoice.hpp"
#include "UIWidgets.hpp"
#include "UIDiorama.hpp"
#include "MainMenu.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Helpers/GameProgress.hpp"
#include "Helpers/MenuParam.hpp"
#include "Helpers/MathHelpers.hpp"
#include "Helpers/TimeAttackData.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ExtrasMenu);

void ExtrasMenu::Update() {}
void ExtrasMenu::LateUpdate() {}
void ExtrasMenu::StaticUpdate() 
{
    UIControl *control = sVars->extrasControl;

    if (control && control->active) {
        UIDiorama *diorama = sVars->diorama;
        UIButton *button   = control->buttons[control->lastButtonID];

        if (button) {
            int32 selectedID = button->nameFrameID;

            switch (selectedID) {
                case 5: diorama->dioramaID = UIDiorama::UIDIORAMA_BOSSRUSH; break;
                case 6: diorama->dioramaID = UIDiorama::UIDIORAMA_MUSICPLAYER; break;
                case 7: diorama->dioramaID = UIDiorama::UIDIORAMA_LEVELSELECT; break;
                case 8: diorama->dioramaID = UIDiorama::UIDIORAMA_EXTRALEVELS; break;
                case 9: diorama->dioramaID = UIDiorama::UIDIORAMA_CREDITS; break;
                default: break;
            }

            if (button->disabled)
                diorama->timer = 12;

            for (int i = 0; i < control->buttonCount; ++i) {
                if (control->buttons[i]) {
                    UIButton *button = control->buttons[i];
                    if (button->nameFrameID > selectedID) {
                        button->position.y        = button->startPos.y + TO_FIXED(24);
                        button->descriptionListID = 5;
                        button->buttonListID      = 1;
                        button->nameListID        = 3;
                        button->buttonFrameID     = button->descriptionFrameID;
                    }
                    else if (button->nameFrameID == selectedID) {
                        button->buttonListID      = 0;
                        button->position.y        = button->startPos.y;
                        button->descriptionListID = 1;
                        button->nameListID        = 2;
                        button->buttonFrameID     = button->descriptionFrameID;
                        // big boy
                    }
                    else {
                        button->buttonListID      = 1;
                        button->nameListID        = 4;
                        button->descriptionListID = 5;
                        button->position.y        = button->startPos.y;
                        button->buttonFrameID     = button->descriptionFrameID + 5;
                    }
                }
            }
        }
    }
}

void ExtrasMenu::Draw() {}

void ExtrasMenu::Create(void *data) {}

void ExtrasMenu::StageLoad() {}

void ExtrasMenu::Initialize()
{
    String tag = {};
    tag.Set("Extras");

    LogHelpers::Print("ManiaModeMenu::Initialize()");

    for (auto control : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES))
    {
        if (tag.Compare(&tag, &control->tag, false))
            sVars->extrasControl = control;
    }

    UIControl *control = sVars->extrasControl;

    for (auto diorama : GameObject::GetEntities<UIDiorama>(FOR_ALL_ENTITIES)) {
        int32 x = control->startPos.x - control->cameraOffset.x;
        int32 y = control->startPos.y - control->cameraOffset.y;

        Hitbox hitbox;
        hitbox.right  = (control->size.x >> 17);
        hitbox.left   = -(control->size.x >> 17);
        hitbox.bottom = (control->size.y >> 17);
        hitbox.top    = -(control->size.y >> 17);

        if (MathHelpers::PointInHitbox(x, y, diorama->position.x, diorama->position.y, FLIP_NONE, &hitbox)) {
            sVars->diorama           = diorama;
            diorama->parent          = sVars->extrasControl;
        }
    }
}

void ExtrasMenu::HandleUnlocks()
{
    UIControl *control = (UIControl *)sVars->extrasControl;

    UIButton *bossRushButton = control->buttons[0];

    UIButton *musicPlayerButton = control->buttons[1];

    UIButton *levelSelectButton = control->buttons[2];

    UIButton *extraLevelsButton = control->buttons[3];

    UIButton *creditsButton = control->buttons[4];
}

void ExtrasMenu::SetupActions()
{
    UIControl *control = (UIControl *)sVars->extrasControl;

    control->processButtonInputCB.Set(&ExtrasMenu::ProcessInputs);

    for (auto button : GameObject::GetEntities<UIButton>(FOR_ALL_ENTITIES))
    {
        if (button->nameListID == 3) {
            switch (button->nameFrameID) {
                default: break;
                case 5:
                    button->actionCB.Set(&ExtrasMenu::BossRush_ActionCB);
                    button->clearParentState = true;
                    break;

                case 6:
                    button->actionCB.Set(&ExtrasMenu::MusicPlayer_ActionCB);
                    button->clearParentState = true;
                    break;

                case 7:
                    button->actionCB.Set(&ExtrasMenu::LevelSelect_ActionCB);
                    button->clearParentState = true;
                    break;

                case 8:
                    button->actionCB.Set(&ExtrasMenu::ExtraLevels_ActionCB);
                    button->clearParentState = true;
                    break;

                case 9:
                    button->actionCB.Set(&ExtrasMenu::Credits_ActionCB);
                    button->clearParentState = true;
                    break;
            }
        }
    }
}

void ExtrasMenu::ProcessInputs()
{
    UIControl *control = (UIControl *)this;
    control->ProcessButtonInput();
}

void ExtrasMenu::Start_BossRush()
{
    MenuParam *param = MenuParam::GetMenuParam();

    TimeAttackData::Clear();

    strcpy(param->menuTag, "Extras");
    param->menuSelection       = MenuParam::EXTRAS_SELECTION_BOSSRUSH;

    Stage::SetScene("Extras", "Egg Gauntlet Zone");
    Stage::LoadScene();
}

void ExtrasMenu::BossRush_ActionCB() { MenuSetup::StartTransition(ExtrasMenu::Start_BossRush, 32); }

void ExtrasMenu::Start_MusicPlayer()
{
    MenuParam *param = MenuParam::GetMenuParam();

    TimeAttackData::Clear();

    strcpy(param->menuTag, "Extras");
    param->menuSelection       = MenuParam::EXTRAS_SELECTION_MUSICPLAYER;

    Stage::SetScene("Extras", "Cabaret");
    Stage::LoadScene();
}

void ExtrasMenu::MusicPlayer_ActionCB() { MenuSetup::StartTransition(ExtrasMenu::Start_MusicPlayer, 32); }

void ExtrasMenu::Start_LevelSelect()
{
    MenuParam *param = MenuParam::GetMenuParam();

    TimeAttackData::Clear();

    strcpy(param->menuTag, "Extras");
    param->menuSelection       = MenuParam::EXTRAS_SELECTION_LEVELSELECT;

    Stage::SetScene("Presentation", "LSelect");
    Stage::LoadScene();
}

void ExtrasMenu::LevelSelect_ActionCB() { MenuSetup::StartTransition(ExtrasMenu::Start_LevelSelect, 32); }

void ExtrasMenu::Start_ExtraLevels()
{
    MenuParam *param = MenuParam::GetMenuParam();

    TimeAttackData::Clear();

    strcpy(param->menuTag, "Extras");
    param->menuSelection       = MenuParam::EXTRAS_SELECTION_EXTRALEVELS;

    Stage::SetScene("Presentation", "LSelect Extras");
    Stage::LoadScene();
}

void ExtrasMenu::ExtraLevels_ActionCB() { MenuSetup::StartTransition(ExtrasMenu::Start_ExtraLevels, 32); }

void ExtrasMenu::Start_Credits()
{
    MenuParam *param = MenuParam::GetMenuParam();

    TimeAttackData::Clear();

    strcpy(param->menuTag, "Extras");
    param->menuSelection       = MenuParam::EXTRAS_SELECTION_CREDITS;
    param->creditsReturnToMenu = true;

    Stage::SetScene("Presentation", "Credits");
    Stage::LoadScene();
}

void ExtrasMenu::Credits_ActionCB() { MenuSetup::StartTransition(ExtrasMenu::Start_Credits, 32); }

#if RETRO_INCLUDE_EDITOR
void ExtrasMenu::EditorDraw() {}

void ExtrasMenu::EditorLoad() {}
#endif

void ExtrasMenu::Serialize() {}

} // namespace GameLogic