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

void UIOptionPanel::Update() 
{
    if (!this->textFrames.Matches(&UIWidgets::sVars->textFrames)) {
        this->topAnimator.SetAnimation(&UIWidgets::sVars->textFrames, this->topListID, true, this->topFrameID);
        this->botAnimator.SetAnimation(&UIWidgets::sVars->textFrames, this->botListID, true, this->botFrameID);

        this->textFrames = UIWidgets::sVars->textFrames;
    }
}

void UIOptionPanel::LateUpdate() {}

void UIOptionPanel::StaticUpdate() {}

void UIOptionPanel::Draw()
{
    UIOptionPanel::DrawBG();

    this->topAnimator.DrawSprite(&this->drawPosTop, false);

    if (!this->botHidden)
        this->botAnimator.DrawSprite(&this->drawPosBottom, false);
}

void UIOptionPanel::Create(void *data)
{
    this->startPos      = this->position;
    this->active        = ACTIVE_BOUNDS;
    this->drawGroup     = 2;
    this->visible       = true;
    this->drawFX        = FX_FLIP;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;

    this->topAnimator.SetAnimation(&UIWidgets::sVars->textFrames, this->topListID, true, this->topFrameID);
    this->botAnimator.SetAnimation(&UIWidgets::sVars->textFrames, this->botListID, true, this->botFrameID);
    this->textFrames = UIWidgets::sVars->textFrames;

    if (this->panelSize < 136)
        this->panelSize = 136;

    this->size.x       = WIDE_SCR_XSIZE << 16;
    this->size.y       = this->panelSize << 16;
    this->drawPosTop.x = this->position.x - 0x920000;
    this->drawPosTop.y = this->position.y + (-0x370000 - (((this->panelSize << 15) - 0x438001) & 0xFFFF0000));

    this->hitbox.left   = -212;
    this->hitbox.top    = -68;
    this->hitbox.right  = 212;
    this->hitbox.bottom = 68;

    this->drawPosBottom = this->position;
    if (!this->botAlignRight)
        this->drawPosBottom.x -= 0xA0000;
    else
        this->drawPosBottom.x += 0xA20000;
    this->drawPosBottom.y += 0x390000;
}

void UIOptionPanel::StageLoad() {}

void UIOptionPanel::DrawBG()
{
    Graphics::DrawRect(this->position.x - (this->size.x >> 1), this->position.y - (this->size.y >> 1), this->size.x, this->size.y, 0x30A0F0, 255, INK_NONE,
                  false);

    int32 x = this->position.x - (this->size.x >> 1);
    int32 y = this->position.y - (this->size.y >> 1);
    Graphics::DrawRect(x, y, 0xE60000, 0x180000, 0, 255, INK_NONE, false);

    UIWidgets::DrawRightTriangle(0xE60000 + x, y, 23, 0x00, 0x00, 0x00);

    if (!this->botHidden) {
        x = (this->size.x >> 1) + this->position.x - 0xE60000;
        y = (this->size.y >> 1) + this->position.y - 0x180000;
        Graphics::DrawRect(x, y, 0xE60000, 0x180000, 0, 255, INK_NONE, false);

        UIWidgets::DrawRightTriangle(x, y + 0x170000, -23, 0x00, 0x00, 0x00);
    }
}

#if RETRO_INCLUDE_EDITOR
void UIOptionPanel::EditorDraw()
{
    this->startPos      = this->position;
    this->active        = ACTIVE_BOUNDS;
    this->drawGroup     = 2;
    this->visible       = true;
    this->drawFX        = FX_FLIP;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;

    this->topAnimator.SetAnimation(&UIWidgets::sVars->textFrames, this->topListID, true, this->topFrameID);
    this->botAnimator.SetAnimation(&UIWidgets::sVars->textFrames, this->botListID, true, this->botFrameID);
    this->textFrames = UIWidgets::sVars->textFrames;

    if (this->panelSize < 136)
        this->panelSize = 136;

    this->size.x = WIDE_SCR_XSIZE << 16;
    this->size.y = this->panelSize << 16;

    this->drawPosTop.x = this->position.x - 0x920000;
    this->drawPosTop.y = this->position.y + (-0x370000 - (((this->panelSize << 15) - 0x438001) & 0xFFFF0000));

    this->drawPosBottom = this->position;
    if (!this->botAlignRight)
        this->drawPosBottom.x -= 0xA0000;
    else
        this->drawPosBottom.x += 0xA20000;
    this->drawPosBottom.y += 0x390000;

    UIOptionPanel::Draw();
}

void UIOptionPanel::EditorLoad() {}
#endif

void UIOptionPanel::Serialize() 
{
    RSDK_EDITABLE_VAR(UIOptionPanel, VAR_ENUM, panelSize);
    RSDK_EDITABLE_VAR(UIOptionPanel, VAR_ENUM, topListID);
    RSDK_EDITABLE_VAR(UIOptionPanel, VAR_ENUM, topFrameID);
    RSDK_EDITABLE_VAR(UIOptionPanel, VAR_ENUM, botListID);
    RSDK_EDITABLE_VAR(UIOptionPanel, VAR_ENUM, botFrameID);
    RSDK_EDITABLE_VAR(UIOptionPanel, VAR_BOOL, botAlignRight);
    RSDK_EDITABLE_VAR(UIOptionPanel, VAR_BOOL, botHidden);
}

} // namespace GameLogic