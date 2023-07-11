// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TitleLogo Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "TitleLogo.hpp"
#include "Global/Localization.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TitleLogo);

void TitleLogo::Update()
{
    this->state.Run(this);
}

void TitleLogo::LateUpdate() {}

void TitleLogo::StaticUpdate() {}

void TitleLogo::Draw()
{
    switch (this->type) {
        case TITLELOGO_EMBLEM: // drawing for emblem type
            Graphics::SetClipBounds(0, 0, 0, screenInfo->size.x, screenInfo->size.y);

            this->direction = FLIP_NONE;
            this->mainAnimator.DrawSprite(nullptr, false);
            break;

        case TITLELOGO_RIBBON: // drawing for ribbon type
            this->direction = FLIP_X;
            this->mainAnimator.DrawSprite(nullptr, false);

            this->direction = FLIP_NONE;
            this->mainAnimator.DrawSprite(nullptr, false);

            if (this->showRibbonCenter)
                this->ribbonCenterAnimator.DrawSprite(nullptr, false);
            break;

        case TITLELOGO_PRESSSTART: // drawing for press start text
            if (!(this->timer & 0x10))
                this->mainAnimator.DrawSprite(nullptr, false);
            break;

        default: this->mainAnimator.DrawSprite(nullptr, false); break; // drawing for everything else
    }
}

void TitleLogo::Create(void *data)
{
    this->drawFX = FX_FLIP;

    if (!sceneInfo->inEditor) {
        switch (this->type) {
            case TITLELOGO_EMBLEM: this->mainAnimator.SetAnimation(sVars->aniFrames, 0, true, 0); break; // animation for emblem

            case TITLELOGO_RIBBON: // animations for ribbons
                this->mainAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
                this->ribbonCenterAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
                this->state.Set(&TitleLogo::State_Ribbon);
                break;

            case TITLELOGO_PRESSSTART: // sets the state to press start if the type is press start
                this->state.Set(&TitleLogo::State_PressButton);
                break;

            default: this->mainAnimator.SetAnimation(sVars->aniFrames, this->type + 2, true, 0); break; // sets the animation for everything else
        }

        switch (this->type) {
            case TITLELOGO_EMBLEM: 
            case TITLELOGO_RIBBON:
                this->alpha = 0; // sets to 0 on create for emblem and ribbon
                this->inkEffect = INK_ALPHA; // set to alpha for emblem and ribbon to fade in
                this->visible   = false;
                this->active    = ACTIVE_NEVER;
                this->drawGroup = 4;
                break;
            case TITLELOGO_GAMETITLE:
            case TITLELOGO_COPYRIGHT:
            case TITLELOGO_PRESSSTART:
                this->visible   = false;
                this->active    = ACTIVE_NEVER;
                this->inkEffect = INK_NONE; // none for every other type
                this->alpha     = 255; // set to to opaque for every other type
                this->drawGroup = 4;
                break;

            default: // any other type beyond the normal list
                this->active    = ACTIVE_NORMAL;
                this->visible   = true;
                this->drawGroup = 4;
                break;
        }
    }
}

void TitleLogo::StageLoad()
{
    sVars->aniFrames.Load("Title/Logo.bin", SCOPE_STAGE);
}

void TitleLogo::SetupPressStart() { this->mainAnimator.SetAnimation(sVars->aniFrames, 6, true, 0); }

void TitleLogo::State_Ribbon()
{
    this->mainAnimator.Process();

    if (this->showRibbonCenter)
        this->ribbonCenterAnimator.Process();
}
void TitleLogo::State_PressButton() { ++this->timer; }

void TitleLogo::State_HandleSetup()
{
    this->position.y += this->velocity.y;
    this->velocity.y += 0x3800;

    this->mainAnimator.Process();

    if (this->showRibbonCenter)
        this->ribbonCenterAnimator.Process();

    if (this->timer <= 0) {
        this->timer = 0;
        this->state.Set(this->type == TITLELOGO_RIBBON ? &TitleLogo::State_Ribbon : nullptr);
    }
}

#if RETRO_INCLUDE_EDITOR
void TitleLogo::EditorDraw()
{
    this->drawFX = FX_FLIP;
    switch (this->type) {
        case TITLELOGO_EMBLEM:
            this->mainAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);

            this->direction = FLIP_NONE;
            this->mainAnimator.DrawSprite(nullptr, false);

            this->direction = FLIP_X;
            this->mainAnimator.DrawSprite(nullptr, false);

            this->direction = FLIP_NONE;
            break;

        case TITLELOGO_RIBBON:
            this->mainAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
            this->ribbonCenterAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);

            this->direction = FLIP_X;
            this->mainAnimator.DrawSprite(nullptr, false);

            this->direction = FLIP_NONE;
            this->mainAnimator.DrawSprite(nullptr, false);

            this->ribbonCenterAnimator.DrawSprite(nullptr, false);
            break;

        case TITLELOGO_PRESSSTART:
            this->mainAnimator.SetAnimation(sVars->aniFrames, 8, true, 0);
            this->mainAnimator.DrawSprite(nullptr, false);
            break;

        default:
            this->mainAnimator.SetAnimation(sVars->aniFrames, this->type + 2, true, 0);
            this->mainAnimator.DrawSprite(nullptr, false);
            break;
    }
}

void TitleLogo::EditorLoad()
{
    sVars->aniFrames.Load("Title/Logo.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Emblem", TITLELOGO_EMBLEM);
    RSDK_ENUM_VAR("Ribbon", TITLELOGO_RIBBON);
    RSDK_ENUM_VAR("Game Title", TITLELOGO_GAMETITLE);
    RSDK_ENUM_VAR("Copyright", TITLELOGO_COPYRIGHT);
    RSDK_ENUM_VAR("Press Start", TITLELOGO_PRESSSTART);
}
#endif

void TitleLogo::Serialize() { RSDK_EDITABLE_VAR(TitleLogo, VAR_ENUM, type); }


} // namespace GameLogic