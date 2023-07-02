// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIButtonLabel Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIButtonLabel.hpp"
#include "UIWidgets.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIButtonLabel);

void UIButtonLabel::Update()
{
    if (!this->textSprite.Matches(UIWidgets::sVars->textFrames)) {
        this->textAnimator.SetAnimation(UIWidgets::sVars->textFrames, this->listID, true, this->frameID);
        this->textSprite = UIWidgets::sVars->textFrames;
    }
}

void UIButtonLabel::LateUpdate() {}

void UIButtonLabel::StaticUpdate() {}

void UIButtonLabel::Draw()
{
    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y;

    this->frameAnimator.DrawSprite(&drawPos, false);
    this->textAnimator.DrawSprite(&drawPos, false);
}

void UIButtonLabel::Create(void *data)
{
    this->active        = ACTIVE_BOUNDS;
    this->drawGroup     = 2;
    this->visible       = true;
    this->drawFX        = FX_FLIP;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x300000;

    this->frameAnimator.SetAnimation(sVars->aniFrames, 0, true, this->type);
    this->textAnimator.SetAnimation(UIWidgets::sVars->textFrames, this->listID, true, this->frameID);

    this->textSprite = UIWidgets::sVars->textFrames;
}

void UIButtonLabel::StageLoad() { sVars->aniFrames.Load("UI/ButtonLabel.bin", SCOPE_STAGE); }

#if RETRO_INCLUDE_EDITOR
void UIButtonLabel::EditorDraw()
{
    this->drawGroup     = 2;
    this->drawFX        = FX_FLIP;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x300000;

    this->frameAnimator.SetAnimation(sVars->aniFrames, 0, true, this->type);
    this->textAnimator.SetAnimation(UIWidgets::sVars->textFrames, this->listID, true, this->frameID);

    this->textSprite = UIWidgets::sVars->textFrames;

    UIButtonLabel::Draw();
}

void UIButtonLabel::EditorLoad() { sVars->aniFrames.Load("UI/ButtonLabel.bin", SCOPE_STAGE); }
#endif

void UIButtonLabel::Serialize()
{
    RSDK_EDITABLE_VAR(UIButtonLabel, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(UIButtonLabel, VAR_ENUM, listID);
    RSDK_EDITABLE_VAR(UIButtonLabel, VAR_ENUM, frameID);
}

} // namespace GameLogic