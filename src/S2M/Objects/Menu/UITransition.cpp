// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UITransition Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UITransition.hpp"
#include "UIControl.hpp"
#include "UIDialog.hpp"
#include "UIWidgets.hpp"
#include "MenuSetup.hpp"
#include "ManiaModeMenu.hpp"
#include "Helpers/FXFade.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UITransition);

void UITransition::Update()
{
    this->state.Run(this);
    UITransition::DrawFade();
}

void UITransition::LateUpdate() {}

void UITransition::StaticUpdate() {}

void UITransition::Draw() {}

void UITransition::Create(void *data)
{
    this->active    = ACTIVE_ALWAYS;
    this->drawGroup = 13;

    this->visible = true;
    this->state.Set(&UITransition::State_Init);
}

void UITransition::StageLoad()
{
    for (auto transition : GameObject::GetEntities<UITransition>(FOR_ALL_ENTITIES)) {
        sVars->activeTransition = (Entity *)transition;
    }
}

void UITransition::StartTransition(Action<void> callback, int32 delay, bool32 back)
{
    UITransition *transition = (UITransition *)sVars->activeTransition;

    if (transition->state.Matches(&UITransition::State_Init) && !UIDialog::sVars->activeDialog) {
        transition->state.Set(&UITransition::State_TransitionIn);
        transition->timer      = 0;
        transition->fadeColor  = 0x000000;
        transition->delay      = delay;
        transition->callback   = callback;
        transition->back       = back;
        transition->prevEntity = (Entity *)sceneInfo->entity;
    }

    UIControl *control = UIControl::GetUIControl();
    if (control)
        control->selectionDisabled = true;
}

void UITransition::MatchNewTag()
{
    UIControl::MatchMenuTag(sVars->newTag);

    sVars->newTag = nullptr;
}

void UITransition::SetNewTag(const char *text)
{
    UITransition *transition = (UITransition *)sVars->activeTransition;

    if (transition->state.Matches(&UITransition::State_Init)) {
        if (!UIDialog::sVars->activeDialog) {
            sVars->newTag = (char *)text;
            UITransition::StartTransition(UITransition::MatchNewTag, 0);
        }
    }
}

void UITransition::DrawFade()
{
    if (FXFade::sVars) {
        if (this->isTransitioning) {
            FXFade *fxFade   = GameObject::Create<FXFade>(this->fadeColor, this->position.x, this->position.y);
            fxFade->active   = ACTIVE_ALWAYS;
            fxFade->speedIn  = 26;
            fxFade->speedOut = 26;
            fxFade->state.Set(&FXFade::State_FadeOut);
        }
    }
}

void UITransition::State_Init()
{
    this->isTransitioning = false;

    if (this->timer == 32) {
        UIControl *control = UIControl::GetUIControl();
        if (control)
            control->selectionDisabled = false;

        this->timer = 0;
    }
    else {
        ++this->timer;
    }
}

void UITransition::State_TransitionIn()
{
    if (this->timer < this->delay)
        this->isTransitioning = false;

    if (this->timer > this->delay + 17) {
        this->timer = 0;
        this->state.Set(&UITransition::State_TransitionOut);
    }
    else {
        this->isTransitioning = true;

        int32 remain = this->timer - this->delay;
        if (!remain && back)
            UIWidgets::sVars->sfxWoosh.Play(false, 255);

        ++this->timer;
    }
}

void UITransition::State_TransitionOut()
{
    if (this->timer >= 1) {
        if (this->timer > 17) {
            this->timer           = 0;
            this->isTransitioning = false;
            this->state.Set(&UITransition::State_Init);
        }
        else {
            this->isTransitioning = true;

            ++this->timer;
        }
    }
    else {
        this->isTransitioning                        = true;
        UIControl::GetUIControl()->selectionDisabled = false;

        if (!this->callback.Matches(nullptr) && !UIDialog::sVars->activeDialog) {
#if RETRO_USE_MOD_LOADER
            this->callback.Run(this->prevEntity);
#else
            this->callback();
#endif
            this->callback.Set(nullptr);
        }
        if (MenuSetup::sVars) {
            ManiaModeMenu::ChangeMenuTrack();
            ManiaModeMenu::ChangeMenuBG();
        }

        UIControl *control         = UIControl::GetUIControl();
        control->selectionDisabled = true;
        ++this->timer;
    }
}

#if RETRO_INCLUDE_EDITOR
void UITransition::EditorDraw() {}

void UITransition::EditorLoad() {}
#endif

void UITransition::Serialize() {}

} // namespace GameLogic