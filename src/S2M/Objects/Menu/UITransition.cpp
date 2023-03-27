// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UITransition Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "UITransition.hpp"
#include "UIControl.hpp"
#include "UIDialog.hpp"
#include "UIWidgets.hpp"
#include "MenuSetup.hpp"
#include "ManiaModeMenu.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UITransition);

void UITransition::Update()
{
    this->state.Run(this);
}

void UITransition::LateUpdate() {}

void UITransition::StaticUpdate() {}

void UITransition::Draw()
{
    if (this->isTransitioning)
        DrawShapes();
}

void UITransition::Create(void *data)
{
    this->active       = ACTIVE_ALWAYS;
    this->drawGroup    = 13;
    this->drawPos[0].x = 0;
    this->drawPos[0].y = 0;
    this->drawPos[1].x = 0;
    this->drawPos[1].y = 0;
    this->drawPos[2].x = 0;
    this->drawPos[2].y = 0;

    this->visible = true;
    this->state.Set(&UITransition::State_Init);
}

void UITransition::StageLoad()
{
    for (auto transition : GameObject::GetEntities<UITransition>(FOR_ALL_ENTITIES)) { sVars->activeTransition = (Entity *)transition; }
}

void UITransition::StartTransition(void (*callback)(), int32 delay)
{
    UITransition *transition = (UITransition *)sVars->activeTransition;

    if (transition->state.Matches(&UITransition::State_Init) && !UIDialog::sVars->activeDialog) {
        transition->state.Set(&UITransition::State_TransitionIn);
        transition->timer      = 0;
        transition->delay      = delay;
        transition->callback.Set(callback);
        transition->prevEntity = (Entity *)sceneInfo->entity;
    }

    UIControl *control = UIControl::GetUIControl();
    if (control)
        control->selectionDisabled = true;
}

void UITransition::StartTransition(Action<void> callback, int32 delay)
{
    UITransition *transition = (UITransition *)sVars->activeTransition;

    if (transition->state.Matches(&UITransition::State_Init) && !UIDialog::sVars->activeDialog) {
        transition->state.Set(&UITransition::State_TransitionIn);
        transition->timer = 0;
        transition->delay = delay;
        transition->callback = callback;
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

void UITransition::DrawShapes()
{
    Vector2 positions[3];
    uint32 colors[3];

    colors[0] = 0xE48E00;
    colors[1] = 0x1888F0;
    colors[2] = 0xE82858;

    int32 screenCenterX = (screenInfo->position.x + screenInfo->center.x) << 16;
    int32 screenCenterY = (screenInfo->position.y + screenInfo->center.y) << 16;
    positions[0].x      = screenCenterX + this->drawPos[0].x - 0xF00000;
    positions[0].y      = screenCenterY + this->drawPos[0].y;
    positions[1].x      = screenCenterX + this->drawPos[1].x;
    positions[1].y      = screenCenterY + this->drawPos[1].y;
    positions[2].x      = screenCenterX + this->drawPos[2].x + 0xF00000;
    positions[2].y      = screenCenterY + this->drawPos[2].y;

    for (int32 i = 0; i < 3; ++i) {
        UIWidgets::DrawParallelogram(positions[i].x, positions[i].y, 0, SCREEN_YSIZE, SCREEN_YSIZE, (colors[i] >> 16) & 0xFF, (colors[i] >> 8) & 0xFF,
                                    colors[i] & 0xFF);
    }
}

void UITransition::State_Init()
{
    this->isTransitioning = false;
    this->timer           = 0;
}

void UITransition::State_TransitionIn()
{
    if (this->timer < this->delay)
        this->isTransitioning = false;

    if (this->timer > this->delay + 16) {
        this->drawPos[0].y = 0;
        this->drawPos[1].x = 0;
        this->drawPos[1].y = 0;
        this->drawPos[2].y = 0;
        this->drawPos[0].x = 0;
        this->drawPos[2].x = 0;
        this->timer        = 0;
        this->state.Set(&UITransition::State_TransitionOut);
    }
    else {
        this->isTransitioning = true;

        int32 remain = this->timer - this->delay;
        if (!remain)
            UIWidgets::sVars->sfxWoosh.Play(false, 255);

        int32 offsets[3];
        offsets[0] = CLAMP(remain, 0, 8);
        offsets[1] = CLAMP(remain - 4, 0, 8);
        offsets[2] = CLAMP(remain - 8, 0, 8);

        int32 percent = 32 * offsets[0];
        if (percent > 0) {
            if (percent < 256) {
                this->drawPos[0].x = -0xF000 * percent + 0xF00000;
                this->drawPos[0].y = 0xF000 * percent - 0xF00000;
            }
            else {
                this->drawPos[0].x = 0;
                this->drawPos[0].y = 0;
            }
        }
        else {
            this->drawPos[0].x = 0xF00000;
            this->drawPos[0].y = -0xF00000;
        }

        percent = 32 * offsets[1];
        if (percent > 0) {
            if (percent < 256) {
                this->drawPos[1].x = 0xF000 * percent - 0xF00000;
                this->drawPos[1].y = -0xF000 * percent + 0xF00000;
            }
            else {
                this->drawPos[1].x = 0;
                this->drawPos[1].y = 0;
            }
        }
        else {
            this->drawPos[1].x = -0xF00000;
            this->drawPos[1].y = 0xF00000;
        }

        percent = 32 * offsets[2];
        if (percent > 0) {
            if (percent < 256) {
                this->drawPos[2].x = -0xF000 * percent + 0xF00000;
                this->drawPos[2].y = 0xF000 * percent - 0xF00000;
            }
            else {
                this->drawPos[2].x = 0;
                this->drawPos[2].y = 0;
            }
        }
        else {
            this->drawPos[2].x = 0xF00000;
            this->drawPos[2].y = -0xF00000;
        }

        ++this->timer;
    }
}

void UITransition::State_TransitionOut()
{
    if (this->timer >= 1) {
        if (this->timer > 16) {
            this->timer           = 0;
            this->isTransitioning = false;
            this->drawPos[0].x    = -0xF00000;
            this->drawPos[0].y    = 0xF00000;
            this->drawPos[1].x    = 0xF00000;
            this->drawPos[1].y    = -0xF00000;
            this->drawPos[2].x    = -0xF00000;
            this->drawPos[2].y    = 0xF00000;

            UIControl *control = UIControl::GetUIControl();
            if (control)
                control->selectionDisabled = false;

            this->state.Set(&UITransition::State_Init);
        }
        else {
            this->isTransitioning = true;
            int32 offsets[3];

            offsets[0] = CLAMP(this->timer - 1, 0, 8);
            offsets[1] = CLAMP(this->timer - 4, 0, 8);
            offsets[2] = CLAMP(this->timer - 1, 0, 8);

            int32 percent = 32 * offsets[0];
            if (percent > 0) {
                if (percent < 256) {
                    this->drawPos[0].x = -0xF000 * percent;
                    this->drawPos[0].y = 0xF000 * percent;
                }
                else {
                    this->drawPos[0].x = -0xF00000;
                    this->drawPos[0].y = 0xF00000;
                }
            }
            else {
                this->drawPos[0].x = 0;
                this->drawPos[0].y = 0;
            }

            percent = 32 * offsets[1];
            if (percent > 0) {
                if (percent < 256) {
                    this->drawPos[1].x = 0xF000 * percent;
                    this->drawPos[1].y = -0xF000 * percent;
                }
                else {
                    this->drawPos[1].x = 0xF00000;
                    this->drawPos[1].y = -0xF00000;
                }
            }
            else {
                this->drawPos[1].x = 0;
                this->drawPos[1].y = 0;
            }

            percent = 32 * offsets[2];
            if (percent > 0) {
                if (percent < 256) {
                    this->drawPos[2].x = -0xF000 * percent;
                    this->drawPos[2].y = 0x1E0000 * offsets[2];
                }
                else {
                    this->drawPos[2].x = -0xF00000;
                    this->drawPos[2].y = 0xF00000;
                }
            }
            else {
                this->drawPos[2].x = 0;
                this->drawPos[2].y = 0;
            }

            ++this->timer;
        }
    }
    else {
        this->isTransitioning                       = true;
        UIControl::GetUIControl()->selectionDisabled = false;

        if (!this->callback.Matches(nullptr) && !UIDialog::sVars->activeDialog) {
            Entity *storeEntity = (Entity *)sceneInfo->entity;
            sceneInfo->entity   = this->prevEntity;
#if RETRO_USE_MOD_LOADER
            this->callback.Run(this);
#else
            this->callback();
#endif
            this->callback.Set(nullptr);
            sceneInfo->entity = storeEntity;
        }

        if (MenuSetup::sVars) {
            ManiaModeMenu::ChangeMenuTrack();
            ManiaModeMenu::ChangeMenuBG();
        }

        UIControl *control   = UIControl::GetUIControl();
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