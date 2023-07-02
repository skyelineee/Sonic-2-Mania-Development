// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UITABanner Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UITABanner.hpp"
#include "UIWidgets.hpp"
#include "Global/Localization.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UITABanner);

void UITABanner::Update()
{
    this->active = ACTIVE_NORMAL;
}

void UITABanner::LateUpdate() {}

void UITABanner::StaticUpdate() {}

void UITABanner::Draw()
{
    UIControl *control = this->parent;
    if (control) {
        UITABanner::DrawBannerInfo(this->position.x, this->position.y, this->zoneID, this->actID, this->characterID);
    }
}

void UITABanner::Create(void *data)
{
    this->startPos.x    = this->position.x;
    this->startPos.y    = this->position.y;
    this->active        = ACTIVE_BOUNDS;
    this->drawGroup     = 2;
    this->visible       = true;
    this->drawFX        = FX_FLIP;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x300000;
}

void UITABanner::StageLoad() {}

void UITABanner::SetupDetails(UITABanner *banner, uint8 zoneID, uint8 actID, uint8 characterID)
{
    banner->actID       = actID;
    banner->characterID = characterID;
    banner->zoneID      = zoneID;
}

void UITABanner::DrawStageInfo(int32 drawX, int32 drawY, uint8 actID, uint8 characterID)
{
    Vector2 drawPos;

    int32 widths[3];
    widths[0] = 60;
    widths[1] = 52;
    widths[2] = 70;
    drawPos.x = drawX - 0x390000;
    drawPos.y = drawY + 0x118000;

    Vector2 drawOffsets[3];
    for (int32 i = 0; i < 3; ++i) {
        drawOffsets[i].x = drawPos.x + 0x10000;
        drawOffsets[i].y = drawPos.y + 0xC0000;
        drawPos.x += (widths[i] << 16);
    }

    this->textAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, characterID + 5, true, actID);
    drawPos.x = drawOffsets[1].x + TO_FIXED(90);
    drawPos.y = drawOffsets[1].y - TO_FIXED(39);
    this->textAnimator.DrawSprite(&drawPos, false);
}

void UITABanner::DrawBannerInfo(int32 drawX, int32 drawY, uint8 zoneID, uint8 actID, uint8 characterID)
{
    Vector2 drawPos;

    drawPos.x = drawX - TO_FIXED(160);
    drawPos.y = drawY - TO_FIXED(26);
    if (!sceneInfo->inEditor) {
        this->bannerAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, 15, true, characterID - 1);
        this->zoneNameAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, characterID + 11, true, zoneID);
        this->bannerAnimator.DrawSprite(&drawPos, false);
        drawPos.x = drawX - TO_FIXED(100);
        drawPos.y = drawY - TO_FIXED(10);
        this->zoneNameAnimator.DrawSprite(&drawPos, false);
    }
    UITABanner::DrawStageInfo(drawX, drawY, actID, characterID);
}

#if RETRO_INCLUDE_EDITOR
void UITABanner::EditorDraw()
{
    UITABanner::DrawBannerInfo(this->position.x, this->position.y, this->zoneID, this->actID, this->characterID);
}

void UITABanner::EditorLoad() {}
#endif

void UITABanner::Serialize() {}

} // namespace GameLogic