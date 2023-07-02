// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIOptionPanel Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIOptionPanel.hpp"
#include "UIWidgets.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIOptionPanel);

void UIOptionPanel::Update() {}

void UIOptionPanel::LateUpdate() {}

void UIOptionPanel::StaticUpdate() {}

void UIOptionPanel::Draw() { this->panelAnimator.DrawSprite(nullptr, false); }

void UIOptionPanel::Create(void *data)
{
    this->active        = ACTIVE_BOUNDS;
    this->drawGroup     = 2;
    this->visible       = true;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;

    this->panelAnimator.SetAnimation(&sVars->aniFrames, 2, true, this->frameID);
}

void UIOptionPanel::StageLoad() { sVars->aniFrames.Load("UI/BGIcons.bin", SCOPE_STAGE); }

#if RETRO_INCLUDE_EDITOR
void UIOptionPanel::EditorDraw() { UIOptionPanel::Draw(); }

void UIOptionPanel::EditorLoad() {}
#endif

void UIOptionPanel::Serialize() { RSDK_EDITABLE_VAR(UIOptionPanel, VAR_INT32, frameID); }

} // namespace GameLogic