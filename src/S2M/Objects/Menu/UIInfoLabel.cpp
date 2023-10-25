// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIInfoLabel Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIInfoLabel.hpp"
#include "UIWidgets.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIInfoLabel);

void UIInfoLabel::Update() {}

void UIInfoLabel::LateUpdate() {}

void UIInfoLabel::StaticUpdate() {}

void UIInfoLabel::Draw() { UIInfoLabel::DrawSprites(); }

void UIInfoLabel::Create(void *data)
{
    this->active        = ACTIVE_BOUNDS;
    this->drawGroup     = 2;
    this->visible       = true;
    this->drawFX        = FX_FLIP;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x300000;
    this->width         = FROM_FIXED(this->size.y);

    if (!sceneInfo->inEditor) {
        this->textAnimator.SetAnimation(UIWidgets::sVars->fontFrames, 0, true, 0);
        this->text.SetSpriteString(UIWidgets::sVars->fontFrames, 0);
    }
}

void UIInfoLabel::StageLoad(void) {}

void UIInfoLabel::SetText(UIInfoLabel *label, char *text)
{
    if (!sceneInfo->inEditor) {
        label->text.Init(text, 0);
        if (!sceneInfo->inEditor) {
            label->textAnimator.SetAnimation(UIWidgets::sVars->fontFrames, 0, true, 0);
            label->text.SetSpriteString(UIWidgets::sVars->fontFrames, 0);
        }
    }
}
void UIInfoLabel::SetString(UIInfoLabel *entity, String *text)
{
    if (!sceneInfo->inEditor) {
        entity->text.Copy(&entity->text, text);
        if (!sceneInfo->inEditor) {
            entity->textAnimator.SetAnimation(UIWidgets::sVars->fontFrames, 0, true, 0);
            entity->text.SetSpriteString(UIWidgets::sVars->fontFrames, 0);
        }
    }
}

void UIInfoLabel::DrawSprites()
{
    Vector2 drawPos;
    int32 size = (this->size.y + this->size.x) >> 16;

    UIWidgets::DrawParallelogram(this->position.x, this->position.y, size, FROM_FIXED(this->size.y), this->width, 0xF0, 0xF0, 0xF0);

    UIWidgets::DrawParallelogram(this->position.x, this->position.y, size, FROM_FIXED(this->size.y), this->width, 0x00, 0x00, 0x00);

    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    if (sceneInfo->inEditor) {
        this->textAnimator.SetAnimation(sVars->aniFrames, 12, true, 2);
        drawPos.y -= 0x40000;
        this->textAnimator.DrawSprite(&drawPos, false);
    }
    else {
        drawPos.y -= 0x10000;
        drawPos.x -= this->text.GetWidth(UIWidgets::sVars->fontFrames, 0, 0, this->text.length, 0) << 15;
        this->textAnimator.DrawString(&drawPos, &this->text, 0, this->text.length, 0, 0, 0, 0);
    }
}

#if RETRO_INCLUDE_EDITOR
void UIInfoLabel::EditorDraw() { UIInfoLabel::DrawSprites(); }

void UIInfoLabel::EditorLoad() { sVars->aniFrames.Load("UI/TextEN.bin", SCOPE_STAGE); }
#endif

void UIInfoLabel::Serialize()
{
    RSDK_EDITABLE_VAR(UIInfoLabel, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(UIInfoLabel, VAR_STRING, text);
}

} // namespace GameLogic