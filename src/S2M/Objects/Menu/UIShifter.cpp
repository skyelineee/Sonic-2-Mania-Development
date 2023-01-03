// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UICarousel Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "UIShifter.hpp"
#include "UIControl.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIShifter);

void UIShifter::Update() {}

void UIShifter::LateUpdate()
{
    if (this->parent) {
        int32 x = 0;
        int32 y = 0;
        if (this->parent->lastButtonID > 0) {
            x = this->parent->lastButtonID * this->shift.x;
            y = this->parent->lastButtonID * this->shift.y;
        }

        this->lastButtonShift.x = x;
        this->lastButtonShift.y = y;
        this->shiftOffset.x += (x - this->shiftOffset.x) >> 2;
        this->shiftOffset.y += (y - this->shiftOffset.y) >> 2;
        UIShifter::HandleShift();
    }
}

void UIShifter::StaticUpdate() {}

void UIShifter::Draw() {}

void UIShifter::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->startPos.x = this->position.x;
        this->startPos.y = this->position.y;

        this->active            = ACTIVE_BOUNDS;
        this->lastButtonShift.x = 0;
        this->lastButtonShift.y = 0;
        this->shiftOffset.x     = 0;
        this->shiftOffset.y     = 0;
        this->visible           = false;
        this->updateRange.x     = 0x800000;
        this->updateRange.y     = 0x800000;
    }
}

void UIShifter::StageLoad() {}

void UIShifter::HandleShift()
{
    int32 shiftX = -this->shift.x;
    int32 shiftY = -this->shift.y;
    int32 x      = this->startPos.x + (this->shiftOffset.x & 0xFFFF0000);
    int32 y      = this->startPos.y + (this->shiftOffset.y & 0xFFFF0000);

    for (int32 i = 0; i < this->parent->buttonCount; ++i) {
        this->parent->buttons[i]->position.x = x;
        this->parent->buttons[i]->position.y = y;
        x += shiftX;
        y += shiftY;
    }
}

#if RETRO_INCLUDE_EDITOR
void UIShifter::EditorDraw() {}

void UIShifter::EditorLoad() {}
#endif

void UIShifter::Serialize() { RSDK_EDITABLE_VAR(UIShifter, VAR_VECTOR2, shift); }
} // namespace GameLogic