// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UILoadingIcon Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UILoadingIcon.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UILoadingIcon);

void UILoadingIcon::Update() 
{
    this->state.Run(this);

    this->visible = true;

    switch (this->timer) {
        case 0: this->visible = false; break;

        case 8:
            // overwritten just below lmao
            this->inkEffect = INK_BLEND;
            break;

        default:
            if (this->timer >= 16) // overwritten just below lmao
                this->inkEffect = INK_NONE;
            break;
    }

    this->inkEffect = INK_ALPHA;
    this->alpha     = 16 * this->timer;
    if (this->alpha >= 0x100)
        this->alpha = 0xFF;

    this->animator.Process();
}
void UILoadingIcon::LateUpdate() {}

void UILoadingIcon::StaticUpdate() {}

void UILoadingIcon::Draw()
{
    Vector2 drawPos;
    drawPos.x = (screenInfo->size.x - 24) << 16;
    drawPos.y = (screenInfo->size.y - 24) << 16;
    this->animator.DrawSprite(&drawPos, true);
}

void UILoadingIcon::Create(void *data)
{
    this->active    = ACTIVE_ALWAYS;
    this->visible   = true;
    this->drawGroup = 15;

    this->animator.SetAnimation(&UILoadingIcon::sVars->aniFrames, 0, true, 0);
    this->state.Set(&UILoadingIcon::State_Show);
}

void UILoadingIcon::StageLoad()
{
    sVars->timer         = 0;
    sVars->activeSpinner = nullptr;

    sVars->aniFrames.Load("UI/LoadingIcon.bin", SCOPE_STAGE);
}

void UILoadingIcon::StartWait()
{
    ++UILoadingIcon::sVars->timer;

    UILoadingIcon *spinner = UILoadingIcon::sVars->activeSpinner;

    if (UILoadingIcon::sVars->timer <= 0) {
        if (spinner)
            spinner->state.Set(&UILoadingIcon::State_Hide);
    }
    else {
        if (!spinner) {
            spinner              = GameObject::Create<UILoadingIcon>(nullptr, 0, 0);
            spinner->isPermanent = true;

            UILoadingIcon::sVars->activeSpinner = spinner;
        }

        spinner->state.Set(&UILoadingIcon::State_Show);
    }
}
void UILoadingIcon::FinishWait()
{
    if (UILoadingIcon::sVars->timer > 0)
        UILoadingIcon::sVars->timer--;

    UILoadingIcon *spinner = UILoadingIcon::sVars->activeSpinner;

    if (UILoadingIcon::sVars->timer <= 0) {
        if (spinner)
            spinner->state.Set(&UILoadingIcon::State_Hide);
    }
    else {
        if (!spinner) {
            spinner              = GameObject::Create<UILoadingIcon>(nullptr, 0, 0);
            spinner->isPermanent = true;

            UILoadingIcon::sVars->activeSpinner = spinner;
        }

        spinner->state.Set(&UILoadingIcon::State_Show);
    }
}
void UILoadingIcon::State_Show()
{
    if (this->timer >= 16) {
        this->timer   = 16;
        this->fadedIn = true;
    }
    else {
        this->timer += 3;
    }
}
void UILoadingIcon::State_Hide()
{
    if (this->timer <= 0) {
        UILoadingIcon::sVars->activeSpinner = nullptr;
        this->Destroy();
    }
    else if (this->fadedIn) {
        this->timer -= 3;
    }
    else {
        UILoadingIcon::State_Show();
    }
}

#if RETRO_INCLUDE_EDITOR
void UILoadingIcon::EditorDraw() {}

void UILoadingIcon::EditorLoad() {}
#endif

void UILoadingIcon::Serialize() {}

} // namespace GameLogic