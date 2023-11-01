// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SlotDisplay Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SlotDisplay.hpp"
#include "SlotMachine.hpp"
#include "SlotPrize.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SlotDisplay);

void SlotDisplay::Update() {}
void SlotDisplay::LateUpdate() {}
void SlotDisplay::StaticUpdate() {}
void SlotDisplay::Draw()
{
    if (!sVars->needsSetup) { // placeholder
        int32 slotPrizeFrame = 0;
        int32 sheetYPos = 0;
        int32 frameHeight = 0;
        // sprite frame stuff here was EditFrame in the v4 script

        // Slot L - Upper
        slotPrizeFrame = SlotMachine::sVars->slotPrizeL >> 5;
        sheetYPos = sVars->sheetY_L[slotPrizeFrame];
        slotPrizeFrame = SlotMachine::sVars->slotPrizeL & 31;
        frameHeight = 32 - slotPrizeFrame;
        this->prizePivotY = slotPrizeFrame - 16;
        SpriteFrame *frame = sVars->aniFrames.GetFrame(0, 0);
        frame->pivotX      = -48;
        frame->pivotY      = this->prizePivotY;
        frame->width       = 32;
        frame->height      = frameHeight;
        frame->sprX        = 1;
        frame->sprY        = sheetYPos;

        // Slot L - Lower
        slotPrizeFrame = SlotMachine::sVars->slotPrizeL >> 5;
        slotPrizeFrame++;
        slotPrizeFrame &= 7;
        sheetYPos = sVars->sheetY_L[slotPrizeFrame];
        slotPrizeFrame = SlotMachine::sVars->slotPrizeL & 31;
        frameHeight = 32 - slotPrizeFrame;
        sheetYPos += frameHeight;
        SpriteFrame *frame2 = sVars->aniFrames.GetFrame(0, 1);
        frame2->pivotX      = -48;
        frame2->pivotY      = -16;
        frame2->width       = 32;
        frame2->height      = slotPrizeFrame;
        frame2->sprX        = 1;
        frame2->sprY        = sheetYPos;

        // Slot M - Upper
        slotPrizeFrame = SlotMachine::sVars->slotPrizeM >> 5;
        sheetYPos = sVars->sheetY_M[slotPrizeFrame];
        slotPrizeFrame = SlotMachine::sVars->slotPrizeM & 31;
        frameHeight = 32 - slotPrizeFrame;
        this->prizePivotY = slotPrizeFrame - 16;
        SpriteFrame *frame3 = sVars->aniFrames.GetFrame(0, 2);
        frame3->pivotX      = -16;
        frame3->pivotY      = this->prizePivotY;
        frame3->width       = 32;
        frame3->height      = frameHeight;
        frame3->sprX        = 1;
        frame3->sprY        = sheetYPos;

        // Slot M - Lower
        slotPrizeFrame = SlotMachine::sVars->slotPrizeM >> 5;
        slotPrizeFrame++;
        slotPrizeFrame &= 7;
        sheetYPos = sVars->sheetY_M[slotPrizeFrame];
        slotPrizeFrame = SlotMachine::sVars->slotPrizeM & 31;
        frameHeight = 32 - slotPrizeFrame;
        sheetYPos += frameHeight;
        SpriteFrame *frame4 = sVars->aniFrames.GetFrame(0, 3);
        frame4->pivotX      = -16;
        frame4->pivotY      = -16;
        frame4->width       = 32;
        frame4->height      = slotPrizeFrame;
        frame4->sprX        = 1;
        frame4->sprY        = sheetYPos;

        // Slot R - Upper
        slotPrizeFrame = SlotMachine::sVars->slotPrizeR >> 5;
        sheetYPos = sVars->sheetY_R[slotPrizeFrame];
        slotPrizeFrame = SlotMachine::sVars->slotPrizeR & 31;
        frameHeight = 32 - slotPrizeFrame;
        this->prizePivotY = slotPrizeFrame - 16;
        SpriteFrame *frame5 = sVars->aniFrames.GetFrame(0, 4);
        frame5->pivotX      = 16;
        frame5->pivotY      = this->prizePivotY;
        frame5->width       = 32;
        frame5->height      = frameHeight;
        frame5->sprX        = 1;
        frame5->sprY        = sheetYPos;

        // Slot R - Lower
        slotPrizeFrame = SlotMachine::sVars->slotPrizeR >> 5;
        slotPrizeFrame++;
        slotPrizeFrame &= 7;
        sheetYPos = sVars->sheetY_R[slotPrizeFrame];
        slotPrizeFrame = SlotMachine::sVars->slotPrizeR & 31;
        frameHeight = 32 - slotPrizeFrame;
        sheetYPos += frameHeight;
        SpriteFrame *frame6 = sVars->aniFrames.GetFrame(0, 5);
        frame6->pivotX      = 16;
        frame6->pivotY      = -16;
        frame6->width       = 32;
        frame6->height      = slotPrizeFrame;
        frame6->sprX        = 1;
        frame6->sprY        = sheetYPos;

        this->animator.SetAnimation(&sVars->aniFrames, 0, true, 0);
        this->animator.DrawSprite(nullptr, false);
        this->animator.SetAnimation(&sVars->aniFrames, 0, true, 1);
        this->animator.DrawSprite(nullptr, false);
        this->animator.SetAnimation(&sVars->aniFrames, 0, true, 2);
        this->animator.DrawSprite(nullptr, false);
        this->animator.SetAnimation(&sVars->aniFrames, 0, true, 3);
        this->animator.DrawSprite(nullptr, false);
        this->animator.SetAnimation(&sVars->aniFrames, 0, true, 4);
        this->animator.DrawSprite(nullptr, false);
        this->animator.SetAnimation(&sVars->aniFrames, 0, true, 5);
        this->animator.DrawSprite(nullptr, false);
    }
    else {
        this->animator.SetAnimation(&sVars->aniFrames, 0, true, 0);
        this->animator.DrawSprite(nullptr, false);
        this->animator.SetAnimation(&sVars->aniFrames, 0, true, 1);
        this->animator.DrawSprite(nullptr, false);
        this->animator.SetAnimation(&sVars->aniFrames, 0, true, 2);
        this->animator.DrawSprite(nullptr, false);
    }
}

void SlotDisplay::Create(void *data)
{
	if (!sceneInfo->inEditor) {
		this->active = ACTIVE_BOUNDS;
		this->visible = true;
		this->drawGroup = 2;

        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
	}   
}

void SlotDisplay::StageLoad()
{
    sVars->aniFrames.Load("CNZ/SlotDisplay.bin", SCOPE_STAGE);
	sVars->needsSetup = true;
}

#if RETRO_INCLUDE_EDITOR
void SlotDisplay::EditorDraw() {}

void SlotDisplay::EditorLoad() {}
#endif

#if RETRO_REV0U
void SlotDisplay::StaticLoad(Static *sVars)
{
	RSDK_INIT_STATIC_VARS(SlotDisplay);

	int32 sheetY_L[] = {
        303, // SLOTPRIZE_BAR
        367, // SLOTPRIZE_EGGMAN
        271, // SLOTPRIZE_RING
        239, // SLOTPRIZE_TAILS
        207, // SLOTPRIZE_SONIC
        335, // SLOTPRIZE_JACKPOT
        239, // SLOTPRIZE_TAILS
        271, // SLOTPRIZE_RING
    };
    memcpy(sVars->sheetY_L, sheetY_L, sizeof(sheetY_L));
	
    int32 sheetY_M[] = {
        303, // SLOTPRIZE_BAR
        367, // SLOTPRIZE_EGGMAN
        271, // SLOTPRIZE_RING
        239, // SLOTPRIZE_TAILS
        207, // SLOTPRIZE_SONIC
        335, // SLOTPRIZE_JACKPOT
        367, // SLOTPRIZE_EGGMAN
        207, // SLOTPRIZE_SONIC
    };
    memcpy(sVars->sheetY_M, sheetY_M, sizeof(sheetY_M));
	
    int32 sheetY_R[] = {
        303, // SLOTPRIZE_BAR
        367, // SLOTPRIZE_EGGMAN
        271, // SLOTPRIZE_RING
        239, // SLOTPRIZE_TAILS
        207, // SLOTPRIZE_SONIC
        335, // SLOTPRIZE_JACKPOT
        239, // SLOTPRIZE_TAILS
        271, // SLOTPRIZE_RING
    };
    memcpy(sVars->sheetY_R, sheetY_R, sizeof(sheetY_R));
}
#endif

void SlotDisplay::Serialize() {}

} // namespace GameLogic