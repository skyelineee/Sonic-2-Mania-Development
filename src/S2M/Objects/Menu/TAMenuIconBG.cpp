// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TAMenuIconBG Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "TAMenuIconBG.hpp"
#include "UISaveSlot.hpp"
#include "UITAZoneModule.hpp"
#include "UIReplayCarousel.hpp"
#include "UIPopover.hpp"
#include "UIControl.hpp"
#include "TimeAttackMenu.hpp"
#include "Global/SaveGame.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TAMenuIconBG);

void TAMenuIconBG::Update() { this->staticAnimator.Process(); }
void TAMenuIconBG::LateUpdate() {}
void TAMenuIconBG::StaticUpdate() {}
void TAMenuIconBG::Draw()
{
    Vector2 drawPos;

    bool32 changeIcon = this->lastZoneIcon != this->zoneIconAnimator.frameID;

    drawPos.x = TO_FIXED(216);
    drawPos.y = TO_FIXED(16);

    UIControl *control         = TimeAttackMenu::sVars->taZoneSelControl;
    UITAZoneModule *module       = (UITAZoneModule *)control->buttons[control->lastButtonID];

    if (control && control->active) {

        if (module->disabled)
            this->timer = 0;

        if (!module->disabled) {
            this->zoneIconAnimator.DrawSprite(&drawPos, true);
            this->zoneIconAnimator.frameID = module->buttonZoneAnimator.frameID;
            if (changeIcon)
                this->timer = 0;
            if (this->timer <= 12) {
                this->timer++;
                this->staticAnimator.DrawSprite(&drawPos, true);
                this->lastZoneIcon = this->zoneIconAnimator.frameID;
            }
        }
        else {
            this->staticAnimator.DrawSprite(&drawPos, true);
        }
    }

    UIControl *replayControl     = TimeAttackMenu::sVars->replaysControl;

    if (replayControl && replayControl->active) {
        this->zoneIconAnimator.DrawSprite(&drawPos, true);
        if (changeIcon)
            this->timer = 0;
        if (this->timer <= 12) {
            this->timer++;
            this->staticAnimator.DrawSprite(&drawPos, true);
            this->lastZoneIcon = this->zoneIconAnimator.frameID;
        }
        for (auto carousel : GameObject::GetEntities<UIReplayCarousel>(FOR_ALL_ENTITIES)) {
            if (carousel->stateDraw.Matches(&UIReplayCarousel::Draw_Loading) || carousel->stateDraw.Matches(&UIReplayCarousel::Draw_NoReplays)) {
                this->staticAnimator.DrawSprite(&drawPos, true);
            }
        }
    }

    UIControl *storedControl = UIPopover::sVars->storedControl;
    if (UIDialog::sVars->activeDialog) {
        this->staticAnimator.DrawSprite(&drawPos, true);
    }
    if (storedControl) {
        if (storedControl->popoverHasFocus) {
            this->staticAnimator.DrawSprite(&drawPos, true);
        }
    }
}

void TAMenuIconBG::Create(void *data)
{
    this->staticAnimator.SetAnimation(&UISaveSlot::sVars->aniFrames, 22, true, 0);
    this->zoneIconAnimator.SetAnimation(&UISaveSlot::sVars->aniFrames, 23, true, 0);

    this->lastZoneIcon = -1;
    this->active       = ACTIVE_BOUNDS;
    this->visible      = true;
    this->drawGroup    = 1;
    this->timer        = 0;
}

void TAMenuIconBG::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void TAMenuIconBG::EditorDraw() {}

void TAMenuIconBG::EditorLoad() {}
#endif

void TAMenuIconBG::Serialize() {}

} // namespace GameLogic