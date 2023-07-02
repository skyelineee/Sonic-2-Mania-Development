// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIRankButton Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIRankButton.hpp"
#include "UIPopover.hpp"
#include "UIControl.hpp"
#include "UICarousel.hpp"
#include "UIWidgets.hpp"
#include "TimeAttackMenu.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIRankButton);

void UIRankButton::Update()
{
    this->touchPosSizeS.x   = this->size.x;
    this->touchPosOffsetS.x = 0;
    this->touchPosOffsetS.y = 0;
    this->touchPosSizeS.x   = this->size.x + 3 * this->size.y;
    this->touchPosSizeS.y   = this->size.y + 0x60000;

    this->state.Run(this);

    if (this->leaderboardEntry) {
        if (this->leaderboardEntry != this->prevLeaderboardEntry || this->leaderboardEntry->status != this->prevLeaderboardEntryStatus) {
            UIRankButton::SetupLeaderboardRank(this, this->leaderboardEntry);
            this->prevLeaderboardEntry       = this->leaderboardEntry;
            this->prevLeaderboardEntryStatus = this->leaderboardEntry->status;
        }
    }

    UIControl *control = (UIControl *)this->parent;
    if (control && this->state.Matches(&UIRankButton::State_HandleButtonEnter) && !control->popoverHasFocus
        && (!control->state.Matches(&UIControl::ProcessInputs) || control->buttons[control->buttonID] != (UIButton *)this)) {
        this->isSelected = false;
        this->state.Set(&UIRankButton::State_HandleButtonLeave);
    }
}

void UIRankButton::LateUpdate() {}

void UIRankButton::StaticUpdate() {}

void UIRankButton::Draw()
{
    UIControl *control   = (UIControl *)this->parent;
    UICarousel *carousel = control->carousel;

    bool32 setClip = false;

    int32 newClipX1 = 0;
    int32 newClipY1 = 0;
    int32 newClipX2 = 0;
    int32 newClipY2 = 0;
    if (carousel && (carousel->clipSize.x > 0 || carousel->clipSize.y > 0)) {
        setClip   = true;
        newClipX2 = carousel->clipSize.x;
        newClipY2 = carousel->clipSize.y;
        newClipX1 = carousel->clipOffset.x + carousel->position.x;
        newClipY1 = carousel->clipOffset.y + carousel->position.y;
    }

    if (this->buttonBounceOffset)
        setClip = false;

    int32 clipX1 = screenInfo->clipBound_X1;
    int32 clipY1 = screenInfo->clipBound_Y1;
    int32 clipX2 = screenInfo->clipBound_X2;
    int32 clipY2 = screenInfo->clipBound_Y2;

    if (setClip)
        Graphics::SetClipBounds(sceneInfo->currentScreenID, (newClipX1 >> 16) - screenInfo->position.x - (newClipX2 >> 17),
                           (newClipY1 >> 16) - screenInfo->position.y - (newClipY2 >> 17),
                           (newClipX1 >> 16) - screenInfo->position.x + (newClipX2 >> 17),
                           (newClipX2 >> 17) + (newClipY1 >> 16) - screenInfo->position.y);

    UIRankButton::DrawSprites();

    if (setClip)
        Graphics::SetClipBounds(sceneInfo->currentScreenID, clipX1, clipY1, clipX2, clipY2);
}

void UIRankButton::Create(void *data)
{
    this->startPos.x    = this->position.x;
    this->startPos.y    = this->position.y;
    this->visible       = true;
    this->drawGroup     = 2;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x400000;
    this->size.x        = 0x180000;
    this->size.y        = 0x100000;
    this->bgEdgeSize    = 16;
    this->hasChanged    = false;
    this->rank          = 0;
    this->score         = 0;
    this->replayID      = 0;

    if (!sceneInfo->inEditor) {
        this->rankText.Init("", 0);
        this->nameTimeText.Init("-----", 0);
        this->nameTimeText.SetSpriteString(UIWidgets::sVars->fontFrames, 0);
    }

    this->processButtonCB.Set(&UIButton::ProcessButtonCB_Scroll);
    this->touchCB.Set(&UIButton::ProcessTouchCB_Single);
    this->selectedCB.Set(&UIRankButton::SelectedCB);
    this->failCB.Set(&UIRankButton::FailCB);
    this->buttonEnterCB.Set(&UIRankButton::ButtonEnterCB);
    this->buttonLeaveCB.Set(&UIRankButton::ButtonLeaveCB);
    this->checkButtonEnterCB.Set(&UIRankButton::CheckButtonEnterCB);
    this->checkSelectedCB.Set(&UIRankButton::CheckSelectedCB);

    this->textVisible = true;
    this->textAnimator.SetAnimation(UIWidgets::sVars->textFrames, 0, true, 0);
    this->fontAnimator.SetAnimation(UIWidgets::sVars->fontFrames, 0, true, 0);
    this->replayIconAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, 10, true, 1);
    this->rankingAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, 10, true, 2);

    this->textFrames = UIWidgets::sVars->textFrames;
}

void UIRankButton::StageLoad() {}

void UIRankButton::SetRankText(UIRankButton *button, int32 rank)
{
    char buffer[8];
    if (rank)
        sprintf_s(buffer, (int32)sizeof(buffer), "%d", rank);
    else
        sprintf_s(buffer, (int32)sizeof(buffer), "-");

    button->rankText.Set(buffer);
    button->rankText.SetSpriteString(UIWidgets::sVars->fontFrames, 0);
}

void UIRankButton::SetTimeAttackRank(UIRankButton *button, int32 rank, int32 score, int32 replayID)
{
    button->rank     = rank;
    button->score    = score;
    button->replayID = 0;

    if (replayID && globals->replayTableID != -1 && APITable->GetUserDBRowByID(globals->replayTableID, replayID) != (uint16)-1)
        button->replayID = replayID;

    button->leaderboardEntry = NULL;
    button->hasChanged       = false;

    if (!sceneInfo->inEditor)
        UIRankButton::SetRankText(button, rank);
}

void UIRankButton::SetupLeaderboardRank(UIRankButton *button, LeaderboardEntry *entry)
{
    button->leaderboardEntry = entry;

    if (entry) {
        button->rank = entry->globalRank;

        if (entry->status == STATUS_OK) {
            button->score      = entry->score;
            button->hasChanged = false;

            button->nameTimeText.Copy(&button->nameTimeText, &entry->username);
            button->nameTimeText.SetSpriteString(UIWidgets::sVars->fontFrames, 0);

            UIRankButton::SetRankText(button, entry->globalRank);
        }
        else {
            button->score      = 0;
            button->hasChanged = false;

            button->nameTimeText.Set("-----");
            button->nameTimeText.SetSpriteString(UIWidgets::sVars->fontFrames, 0);

            UIRankButton::SetRankText(button, button->rank);
        }
    }
    else {
        button->rank       = 0;
        button->score      = 0;
        button->hasChanged = false;

        button->nameTimeText.Set("-----");
        button->nameTimeText.SetSpriteString(UIWidgets::sVars->fontFrames, 0);

        UIRankButton::SetRankText(button, button->rank);
    }
}

void UIRankButton::DrawSprites()
{
    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y;

    int32 startX = 0xA40000;
    if (!this->showsName)
        startX = 0x840000;

    int32 width = 0xB20000;
    if (this->showsName)
        width = 0xF20000;
    
    Vector2 detailPos;
    detailPos.x = 0x110000 - ((width + 0x2A0000) >> 1) + drawPos.x + this->buttonBounceOffset;
    detailPos.y = drawPos.y;

    this->detailIcon.SetAnimation(UIWidgets::sVars->buttonFrames, 23, true, 0);
    drawPos.x = 0x110000 - ((width + 0x2A0000) >> 1) + this->position.x;
    this->detailIcon.DrawSprite(&detailPos, false);

    this->detailIcon.SetAnimation(UIWidgets::sVars->buttonFrames, 23, true, 1);
    int32 drawX = startX + drawPos.x;
    int32 drawY = drawPos.y;
    this->detailIcon.DrawSprite(&detailPos, false);

    UIControl *control = TimeAttackMenu::sVars->taDetailsControl;
    if (control && control->active && this->isSelected) {
        Vector2 arrowPos;
        arrowPos.x = drawPos.x + TO_FIXED(250);
        arrowPos.y = drawPos.y;
        UIWidgets::sVars->arrowLeftAnimator.DrawSprite(&arrowPos, false);
    }

    if (!this->state.Matches(&UIRankButton::State_Selected) || !((this->timer >> 1) % 2)) {
        if (!sceneInfo->inEditor) {
            drawPos.x = drawPos.x + this->buttonBounceOffset;
            drawPos.y = drawPos.y;

            if (this->rank && this->rank <= 3) {
                this->rankingAnimator.frameID = this->rank + 1;
                this->rankingAnimator.DrawSprite(&drawPos, false);
            }

            drawPos.x -= this->rankText.GetWidth(UIWidgets::sVars->fontFrames, 0, 0, this->rankText.length, 0) << 15;
            this->fontAnimator.DrawString(&drawPos, &this->rankText, 0, this->rankText.length, UIButton::ALIGN_LEFT, 0, nullptr,
                                        false);
        }

        if (this->showsName && !sceneInfo->inEditor) {
            drawPos.x = drawX - (width >> 1) + 0x20000 + this->buttonBounceOffset;
            drawPos.y = drawY;
            int32 len = MIN(this->nameTimeText.length, 20);
            this->fontAnimator.DrawString(&drawPos, &this->nameTimeText, 0, len, UIButton::ALIGN_LEFT, 0, nullptr, false);
        }

        drawPos.x = (drawX + this->buttonBounceOffset) - 0x200000;
        drawPos.y = drawY;
        if (this->showsName)
            drawPos.x += 0x3C0000;

        UIWidgets::DrawTime(drawPos.x, drawPos.y - TO_FIXED(2), this->score / 6000, this->score % 6000 / 100, this->score % 100);

        if (!sceneInfo->inEditor && this->replayID) {
            drawPos.x = this->position.x + ((width + 0x2A0000) >> 1);
            drawPos.y = this->position.y;

            if (this->showsName)
                drawPos.x += 0xC0000;

            drawPos.x += this->buttonBounceOffset;;
            this->replayIconAnimator.DrawSprite(&drawPos, false);

            this->popoverPos.x = drawPos.x;
            this->popoverPos.y = drawPos.y - 0x80000;
        }
    }
}

bool32 UIRankButton::CheckButtonEnterCB()
{
    return this->state.Matches(&UIRankButton::State_HandleButtonEnter);
}

bool32 UIRankButton::CheckSelectedCB()
{
    return this->state.Matches(&UIRankButton::State_Selected);
}

void UIRankButton::ButtonEnterCB()
{
    if (!this->state.Matches(&UIRankButton::State_HandleButtonEnter) && !this->state.Matches(&UIRankButton::State_Selected)) {
        this->buttonBounceOffset   = 0;
        this->buttonBounceVelocity = -0x25000;

        this->state.Set(&UIRankButton::State_HandleButtonEnter);
    }
}

void UIRankButton::ButtonLeaveCB()
{
    this->state.Set(&UIRankButton::State_HandleButtonLeave);
}

void UIRankButton::FailCB() { UIWidgets::sVars->sfxFail.Play(false, 255); }

void UIRankButton::SelectedCB()
{
    UIControl *control = (UIControl *)this->parent;
    control->backoutTimer    = 30;
    this->timer              = 0;
    this->state.Set(&UIRankButton::State_Selected);

    UIWidgets::sVars->sfxAccept.Play(false, 255);
}

void UIRankButton::State_HandleButtonLeave()
{
    if (this->buttonBounceOffset) {
        int32 offset = -(this->buttonBounceOffset / abs(this->buttonBounceOffset));
        this->buttonBounceOffset += offset << 16;

        if (offset < 0 && this->buttonBounceOffset < 0)
            this->buttonBounceOffset = 0;
        else if (offset > 0 && this->buttonBounceOffset > 0)
            this->buttonBounceOffset = 0;
    }

    this->isSelected = false;
}

void UIRankButton::State_HandleButtonEnter()
{
    this->buttonBounceOffset += this->buttonBounceVelocity;
    if (this->buttonBounceOffset >= -0x20000 && this->buttonBounceVelocity > 0) {
        this->buttonBounceOffset   = 0;
        this->buttonBounceVelocity = 0;
    }

    this->isSelected = true;
    this->buttonBounceVelocity += 0x4800;
}

void UIRankButton::State_Selected()
{
    UIRankButton::State_HandleButtonEnter();

    if (++this->timer == 30) {
        this->timer = 0;
        this->actionCB.Run(this);
        this->state.Set(&UIRankButton::State_HandleButtonEnter);
    }

    this->textVisible = !((this->timer >> 1) & 1);
}

#if RETRO_INCLUDE_EDITOR
void UIRankButton::EditorDraw() { UIRankButton::DrawSprites(); }

void UIRankButton::EditorLoad() {}
#endif

void UIRankButton::Serialize() { RSDK_EDITABLE_VAR(UIRankButton, VAR_BOOL, showsName); }
} // namespace GameLogic