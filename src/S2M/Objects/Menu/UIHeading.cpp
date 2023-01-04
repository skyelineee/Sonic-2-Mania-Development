// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIHeading Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIHeading.hpp"
#include "UIWidgets.hpp"
#include "Global/Localization.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIHeading);

void UIHeading::Update()
{
    if (!this->textFrames.Matches(&sVars->textFrames)) {
        this->textAnimator.SetAnimation(&sVars->textFrames, this->headingID, true, 0);
        this->textFrames = sVars->textFrames;
    }

    this->state.Run(this);
}

void UIHeading::LateUpdate(void) {}

void UIHeading::StaticUpdate(void) {}

void UIHeading::Draw()
{
    this->headingAnimator.DrawSprite(NULL, false);
    this->textAnimator.DrawSprite(NULL, false);
}

void UIHeading::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->startPos      = this->position;
        this->visible       = true;
        this->drawGroup     = 2;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x300000;

        this->headingAnimator.SetAnimation(&UIWidgets::sVars->uiFrames, 0, true, 0);
        this->textAnimator.SetAnimation(&sVars->textFrames, this->headingID, true, 0);
        this->textFrames = sVars->textFrames;
    }
}

void UIHeading::StageLoad() { UIHeading::LoadSprites(); }

void UIHeading::LoadSprites()
{
    switch (Localization::sVars->language) {
        case LANGUAGE_EN: sVars->textFrames.Load("UI/HeadingsEN.bin", SCOPE_STAGE); break;
        case LANGUAGE_FR: sVars->textFrames.Load("UI/HeadingsFR.bin", SCOPE_STAGE); break;
        case LANGUAGE_IT: sVars->textFrames.Load("UI/HeadingsIT.bin", SCOPE_STAGE); break;
        case LANGUAGE_GE: sVars->textFrames.Load("UI/HeadingsGE.bin", SCOPE_STAGE); break;
        case LANGUAGE_SP: sVars->textFrames.Load("UI/HeadingsSP.bin", SCOPE_STAGE); break;
        case LANGUAGE_JP: sVars->textFrames.Load("UI/HeadingsJP.bin", SCOPE_STAGE); break;

        default: break;
    }
}

#if RETRO_INCLUDE_EDITOR
void UIHeading::EditorDraw()
{
    this->startPos      = this->position;
    this->drawGroup     = 2;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x300000;

    this->headingAnimator.SetAnimation(&UIWidgets::sVars->uiFrames, 0, true, 0);
    this->textAnimator.SetAnimation(&sVars->textFrames, this->headingID, true, 0);
    this->textFrames = sVars->textFrames;

    UIHeading::Draw();
}

void UIHeading::EditorLoad()
{
    sVars->textFrames.Load("UI/HeadingsEN.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, headingID);
    RSDK_ENUM_VAR("Main Menu");
    RSDK_ENUM_VAR("Save Select");
    RSDK_ENUM_VAR("Time Attack");
    RSDK_ENUM_VAR("Results");
    RSDK_ENUM_VAR("Options");
    RSDK_ENUM_VAR("Extras");
    RSDK_ENUM_VAR("Player Select");
    RSDK_ENUM_VAR("Zone Select");
}
#endif

void UIHeading::Serialize() { RSDK_EDITABLE_VAR(UIHeading, VAR_ENUM, headingID); }
} // namespace GameLogic