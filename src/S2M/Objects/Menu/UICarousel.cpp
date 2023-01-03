// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UICarousel Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UICarousel.hpp"
#include "UIControl.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UICarousel);

void UICarousel::Update() {}

void UICarousel::LateUpdate()
{
    if (this->parent) {
        UICarousel::HandleScrolling();
        UICarousel::HandleButtonPositions();

        this->offset.x -= this->offset.x >> 2;
        this->offset.y -= this->offset.y >> 2;

        if (abs(this->offset.y) <= 0x10000)
            this->offset.y = 0;
    }
}

void UICarousel::StaticUpdate() {}

void UICarousel::Draw() {}

void UICarousel::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->startPos.x = this->position.x;
        this->startPos.y = this->position.y;
        this->active     = ACTIVE_BOUNDS;

        this->unused1 = 0;
        this->unused2 = 0;
        this->unused3 = 0;
        this->unused4 = 0;

        this->visible       = false;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->scrollOffset  = -1;
        this->buttonID      = -1;
        this->minOffset     = 0;
        this->maxOffset     = 20;
    }
}

void UICarousel::StageLoad() {}

void UICarousel::HandleScrolling()
{
    UIControl *control = (UIControl *)this->parent;
    int32 buttonID     = this->buttonID;

    if (control->buttonID != buttonID) {
        int32 max        = control->buttonCount - 1;
        bool32 movedUp   = false;
        bool32 movedDown = false;

        if (buttonID != -1) {
            if (!buttonID && control->buttonID == max) {
                movedUp = true;
            }
            else if (buttonID == max && !control->buttonID) {
                movedDown = true;
            }
            else {
                if (control->buttonID < buttonID)
                    movedUp = true;
                else if (control->buttonID > buttonID)
                    movedDown = true;
            }
        }

        // Bug Details:
        // This is HORRIBLY broken when using touch controls
        // the virtual scrolling expects buttons to only be incremented one at a time
        // HOWEVER, using touch you can jump to any visible button, which can be jumping 2 or more buttons at a time!!
        // as a result of that, the scrollOffset & virtualIndex will become misaligned from the visual buttonID and mess up controls and visuals
        // Fix:
        // find a way to increment by however many buttons we jumped, rather than just by 1
        // this is slightly harder than it sounds due to the virtual scroll & wrapping being applied

        int32 scrollOffset = this->scrollOffset;
        if (movedUp) {
            --this->virtualIndex;
            this->scrollOffset = this->virtualIndex - (control->buttonCount >> 1);
        }
        else if (movedDown) {
            ++this->virtualIndex;
            this->scrollOffset = this->virtualIndex - (control->buttonCount >> 1);
        }

        if (this->maxOffset != -1) {
            if (this->scrollOffset + control->buttonCount > this->maxOffset + 1)
                this->scrollOffset = this->maxOffset - control->buttonCount + 1;

            if (this->virtualIndex > this->maxOffset - 1) {
                this->virtualIndex = this->maxOffset - 1;
                control->buttonID  = buttonID;
            }
        }

        if (this->minOffset != -1) {
            if (this->scrollOffset < this->minOffset - 1)
                this->scrollOffset = this->minOffset - 1;

            if (this->virtualIndex < this->minOffset) {
                this->virtualIndex = this->minOffset;
                control->buttonID  = buttonID;
            }
        }

        if (this->scrollOffset >= scrollOffset) {
            if (this->scrollOffset > scrollOffset)
                this->offset.y += abs(this->shift.y);

            this->buttonID = control->buttonID;
        }
        else {
            this->offset.y -= abs(this->shift.y);
            this->buttonID = control->buttonID;
        }
    }
}

void UICarousel::HandleButtonPositions()
{
    UIControl *control = (UIControl *)this->parent;

    Vector2 positions[0x10];
    memset(positions, 0, sizeof(positions));

    for (int32 i = 0; i < control->buttonCount; ++i) {
        positions[i].x = this->position.x;
        positions[i].y = this->position.y - (this->shift.y * i);
    }

    for (int32 i = 0; i < control->buttonCount; ++i) {
        UIButton *item = control->buttons[i];

        int32 id = (i - this->scrollOffset) % control->buttonCount;
        if (id < 0)
            id += control->buttonCount;

        item->position.x = positions[id].x + this->offset.x;
        item->position.y = positions[id].y + this->offset.y;
        item->drawGroup  = id - control->buttonCount + 12;
    }
}

#if RETRO_INCLUDE_EDITOR
void UICarousel::EditorDraw()
{
    DrawHelpers::DrawRectOutline(this->position.x + this->clipOffset.x, this->position.y + this->clipOffset.y, this->clipSize.x, this->clipSize.y,
                                 0xFF0000);
}

void UICarousel::EditorLoad() {}
#endif

void UICarousel::Serialize()
{
    RSDK_EDITABLE_VAR(UICarousel, VAR_VECTOR2, shift);
    RSDK_EDITABLE_VAR(UICarousel, VAR_VECTOR2, clipSize);
    RSDK_EDITABLE_VAR(UICarousel, VAR_VECTOR2, clipOffset);
}
} // namespace GameLogic