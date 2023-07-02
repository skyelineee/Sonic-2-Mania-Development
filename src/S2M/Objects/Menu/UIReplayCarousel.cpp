// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIReplayCarousel Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "UIReplayCarousel.hpp"
#include "TAMenuIconBG.hpp"
#include "TimeAttackMenu.hpp"
#include "UIControl.hpp"
#include "UIWidgets.hpp"
#include "Global/Localization.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIReplayCarousel);

void UIReplayCarousel::Update()
{
    this->sortedRowCount = APITable->GetSortedUserDBRowCount(globals->replayTableID);
    UIReplayCarousel::SetupButtonCallbacks();
    UIReplayCarousel::HandleTouchPositions();
    this->active = ACTIVE_NORMAL;

    this->isMoving = this->curViewOffset != this->targetViewOffset;
    if (this->curViewOffset >= this->targetViewOffset) {
        if (this->curViewOffset > this->targetViewOffset) {
            this->curViewOffset -= 12;
            if (this->curViewOffset < this->targetViewOffset)
                this->curViewOffset = this->targetViewOffset;
        }
    }
    else {
        this->curViewOffset += 12;
        if (this->curViewOffset > this->targetViewOffset)
            this->curViewOffset = this->targetViewOffset;
    }
    UIReplayCarousel::HandlePositions();

    this->state.Run(this);

    if (this->visibleReplayOffset != this->prevReplayOffset) {
        UIReplayCarousel::SetupVisibleReplayButtons();
        this->prevReplayOffset = this->visibleReplayOffset;
    }

    UIControl *parent = (UIControl *)this->parent;
    if (this->curReplayID > 0) {
        UIControl::SetTargetPos(parent, 0, this->position.y + TO_FIXED(24));

        if (102 * this->curReplayID - 102 > 0)
            this->targetViewOffset = 102 * this->curReplayID - 102;
        else
            this->targetViewOffset = 0;
    }
    else {
        UIControl::SetTargetPos(parent, 0, parent->startPos.y);
        this->targetViewOffset = 0;
    }

    if (this->language != Localization::sVars->language && Localization::sVars->loaded) {
        this->language = Localization::sVars->language;
    }
    if (this->state.Matches(&UIReplayCarousel::State_Selected) && !parent->popoverHasFocus && !parent->dialogHasFocus
        && (parent->buttons[parent->buttonID] != (UIButton *)this || !parent->state.Matches(&UIControl::ProcessInputs))) {
        UIReplayCarousel::SetUnselected();
    }
}

void UIReplayCarousel::LateUpdate() {}

void UIReplayCarousel::StaticUpdate() {}

void UIReplayCarousel::Draw() { this->stateDraw.Run(this); }

void UIReplayCarousel::Create(void *data)
{
    this->startPos        = this->position;
    this->active          = ACTIVE_BOUNDS;
    this->drawGroup       = 6;
    this->visible         = true;
    this->drawFX          = FX_FLIP;
    this->updateRange.x   = 0x800000;
    this->updateRange.y   = 0x300000;
    this->processButtonCB.Set(&UIReplayCarousel::ProcessButtonCB);
    this->touchCB.Set(&UIButton::ProcessTouchCB_Multi);
    this->state.Set(&UIReplayCarousel::State_Init);
    this->sortedRowCount  = 0;
    UIReplayCarousel::HandlePositions();
    if (!sceneInfo->inEditor) {
        for (int32 i = 0; i < 4; ++i) {
            this->createdAtText[i].Init("", 0);
        }

        UIReplayCarousel::SetupVisibleReplayButtons();
        this->prevReplayOffset = this->visibleReplayOffset;
        this->language         = Localization::sVars->language;
        this->createTimeAnimator.SetAnimation(UIWidgets::sVars->fontFrames, 0, true, 0);
    }
    UIReplayCarousel::SetupButtonCallbacks();
}

void UIReplayCarousel::StageLoad() { sVars->aniFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE); }

void UIReplayCarousel::ProcessButtonCB()
{
    UIControl *parent = (UIControl *)this->parent;

    if (!this->isMoving && !this->state.Matches(&UIReplayCarousel::State_StartAction)) {
        int32 rowID = 0;
        if (parent->rowCount && parent->columnCount)
            rowID = parent->buttonID / parent->columnCount;

        int32 columnID = 0;
        if (parent->columnCount)
            columnID = parent->buttonID % parent->columnCount;

        int32 id         = this->curReplayID;
        bool32 movedUp   = false;
        bool32 movedDown = false;
        if (UIControl::sVars->anyUpPress) {
            if (this->curReplayID) {
                --id;
                movedDown = true;
            }
            else {
                --rowID;
                movedUp = true;
            }
        }

        if (UIControl::sVars->anyDownPress && this->curReplayID < this->sortedRowCount - 1) {
            movedDown = true;
            id++;
        }

        if (movedUp) {
            if (rowID < 0)
                rowID += parent->rowCount;
            if (rowID >= parent->rowCount)
                rowID -= parent->rowCount;

            int32 selection = (parent->buttonCount - 1);
            if (rowID * parent->columnCount + columnID < selection)
                selection = (rowID * parent->columnCount + columnID);

            if (parent->buttonID != selection) {
                parent->buttonID = selection;
                UIReplayCarousel::SetUnselected();
                UIWidgets::sVars->sfxBleep.Play(false, 255);
            }
        }
        else {
            if (movedDown) {
                UIWidgets::sVars->sfxBleep.Play(false, 255);
                this->curReplayID = id;
            }

            if (UIControl::sVars->anyConfirmPress) {
                if (this->disabled) {
                    this->failCB.Run(this);
                }
                else {
                    if (!this->actionCB.Matches(nullptr)) {
                        UIReplayCarousel::StartAction();
                        return;
                    }
                }
            }

            if (!this->currentlySelected) {
                if (parent->buttons[parent->lastButtonID] == (UIButton *)this && !parent->dialogHasFocus && !parent->popoverHasFocus) {
                    this->currentlySelected = true;
                    this->state.Set(&UIReplayCarousel::State_Selected);
                }
            }
        }
    }
}

void UIReplayCarousel::HandlePositions()
{
    if (this->curViewOffset) {
        this->visibleReplayOffset = this->curViewOffset / 102;

        int32 pos        = 102 * (this->curViewOffset / 102) - this->curViewOffset - 102;
        this->popoverPos = pos << 16;
        if (pos < 153) {
            this->visibleReplayCount = (200 - pos) / 0x66 + 1;
        }
        else {
            this->visibleReplayCount = 0;
        }
    }
    else {
        this->visibleReplayOffset = 0;
        this->visibleReplayCount  = MIN(this->sortedRowCount, 3);
        this->popoverPos          = -0x660000;
    }
}

void UIReplayCarousel::SetupButtonCallbacks()
{
    StateMachine<UIReplayCarousel> state = this->stateDraw;

    if (this->sortedRowCount) {
        if (!APITable->GetUserDBRowsChanged(globals->replayTableID) && globals->replayTableLoaded == STATUS_OK) {
            this->stateDraw.Set(&UIReplayCarousel::Draw_Carousel);
            this->processButtonCB.Set(&UIReplayCarousel::ProcessButtonCB);
            this->touchCB.Set(&UIButton::ProcessTouchCB_Multi);
        }
        else {
            this->stateDraw.Set(&UIReplayCarousel::Draw_Loading);
            this->touchCB.Set(nullptr);
            this->processButtonCB.Set(nullptr);
        }
    }
    else {
        this->stateDraw.Set(&UIReplayCarousel::Draw_NoReplays);
        this->touchCB.Set(nullptr);
        this->processButtonCB.Set(nullptr);
    }

    if (!this->stateDraw.Matches(state.state) && this->stateDraw.Matches(&UIReplayCarousel::Draw_Carousel))
        UIReplayCarousel::SetupVisibleReplayButtons();
}

void UIReplayCarousel::HandleTouchPositions()
{
    if (!this->touchCB.Matches(nullptr)) {
        this->touchPosCount = this->visibleReplayCount;
        int32 posX          = this->position.x;
        int32 posY          = this->popoverPos + this->position.y;

        int32 i = 0;
        for (; i < this->visibleReplayCount; ++i) {
            if (i + this->visibleReplayOffset >= this->sortedRowCount)
                break;

            this->touchPosOffsetM[i].x = posX - this->position.x;
            this->touchPosOffsetM[i].y = posY - this->position.y;
            this->touchPosSizeM[i].x   = 0x1320000;
            this->touchPosSizeM[i].y   = 0x550000;
            this->touchPosCallbacks[i].Set(&UIReplayCarousel::TouchedCB);
            posY += 0x660000;
        }

        for (; i < this->visibleReplayCount; ++i) {
            this->touchPosOffsetM[i].x = 0;
            this->touchPosOffsetM[i].y = 0;
            this->touchPosSizeM[i].x   = 0;
            this->touchPosSizeM[i].y   = 0;
            this->touchPosCallbacks[i].Set(nullptr);
        }
    }
}

void UIReplayCarousel::TouchedCB()
{
    int32 id = this->touchPosID + this->visibleReplayOffset;
    if (this->curReplayID == id)
        UIReplayCarousel::StartAction();
    else
        this->curReplayID = id;
}

void UIReplayCarousel::SetupVisibleReplayButtons()
{
    if (!sceneInfo->inEditor) {
        char buffer[0x20];
        for (int32 i = 0; i < 4; ++i) {
            int32 id = i + this->visibleReplayOffset;
            if (id >= this->sortedRowCount)
                break;
            int32 row    = APITable->GetSortedUserDBRowID(globals->replayTableID, id);
            uint8 zoneID = 0xFF;
            APITable->GetUserDBValue(globals->replayTableID, row, API::Storage::UserDB::UInt8, "zoneID", &zoneID);
            APITable->GetUserDBRowCreationTime(globals->replayTableID, row, buffer, sizeof(buffer) - 1, "%D");
            if (zoneID != 0xFF) {
                this->createdAtText[i].Init(buffer, 0);
                this->createdAtText[i].SetSpriteString(UIWidgets::sVars->fontFrames, 0);
            }
        }
    }
}

void UIReplayCarousel::DrawStageInfo(int32 drawX, int32 drawY, uint8 act, uint8 characterID, int32 score, int32 replayID)
{
    Vector2 drawPos;
    int32 widths[]  = { 60, 82, 52, 70 };

    drawPos.x = drawX - 0x990000;
    drawPos.y = drawY + 0x1D8000;

    Vector2 drawOffsets[4];
    for (int32 i = 0; i < 4; ++i) {
        drawOffsets[i].x = drawPos.x + 0x10000;
        drawOffsets[i].y = drawPos.y + 0xC0000;
        drawPos.x += (widths[i] << 16);
    }

    if ((!this->state.Matches(&UIReplayCarousel::State_StartAction) || this->curReplayID != replayID || !(this->timer & 2)) && !sceneInfo->inEditor) {
        if (!sceneInfo->inEditor) {
            int32 id    = replayID - this->visibleReplayOffset;
            drawPos.x   = drawOffsets[0].x + 0x1E0000;
            drawPos.y   = drawOffsets[0].y;
            int32 width = this->createdAtText[id].GetWidth(UIWidgets::sVars->fontFrames, 0, 0, this->createdAtText[id].length, 0);
            drawPos.x += TO_FIXED(40);
            drawPos.y -= TO_FIXED(3);
            this->createTimeAnimator.DrawString(&drawPos, &this->createdAtText[id], 0, this->createdAtText[id].length, UIButton::ALIGN_LEFT, 0,
                                                nullptr, false);
        }

        drawPos.x = drawOffsets[1].x - TO_FIXED(8);
        drawPos.y = drawOffsets[1].y + TO_FIXED(12);
        UIWidgets::DrawTime(drawPos.x, drawPos.y, score / 6000, score % 6000 / 100, score % 100);

        this->detailsAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, characterID + 5, true, act);
        drawPos.x = drawOffsets[2].x - TO_FIXED(78);
        drawPos.y = drawOffsets[2].y - TO_FIXED(40);
        this->detailsAnimator.DrawSprite(&drawPos, false);
    }
}

void UIReplayCarousel::DrawReplayInfo(int32 drawX, int32 drawY, uint8 zoneID, uint8 act, uint8 characterID, int32 score,
                                     int32 replayID)
{
    Vector2 drawPos;
    drawPos.x = drawX;
    drawPos.y = drawY - 0x160000;
    if ((!this->state.Matches(&UIReplayCarousel::State_StartAction) || this->curReplayID != replayID || !(this->timer & 2)) && !sceneInfo->inEditor) {
        int32 id  = replayID - this->visibleReplayOffset;
        drawPos.x = drawX - TO_FIXED(212);
        this->buttonAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, 0, true, characterID - 1);
        this->buttonZoneAnimator.SetAnimation(UIWidgets::sVars->timeAttackFrames, characterID + 1, true, zoneID);
        UIControl *replayControl = TimeAttackMenu::sVars->replaysControl;
        if (replayControl && replayControl->active) {
            for (auto taMenuBG : GameObject::GetEntities<TAMenuIconBG>(FOR_ACTIVE_ENTITIES)) {
                if (this->curReplayID == replayID)
                    taMenuBG->zoneIconAnimator.frameID = zoneID;
            }
        }

        this->buttonAnimator.DrawSprite(&drawPos, false);
        drawPos.x = drawX - TO_FIXED(197);
        drawPos.y = drawY - TO_FIXED(6);
        this->buttonZoneAnimator.DrawSprite(&drawPos, false);
    }


    UIReplayCarousel::DrawStageInfo(drawX, drawY, act, characterID, score, replayID);
}

void UIReplayCarousel::StartAction()
{
    UIControl *parent = (UIControl *)this->parent;

    this->timer               = 0;
    this->state.Set(&UIReplayCarousel::State_StartAction);
    this->processButtonCB.Set(nullptr);
    parent->selectionDisabled = true;
    UIWidgets::sVars->sfxAccept.Play(false, 255);
}

void UIReplayCarousel::SetUnselected()
{
    this->currentlySelected = false;
    this->state.Set(&UIReplayCarousel::State_Unselected);
    if (!UIControl::sVars->hasTouchInput)
        this->curReplayID = -1;
}

void UIReplayCarousel::Draw_Loading()
{
    UIControl *parent = (UIControl *)this->parent;

    Vector2 drawPos;
    drawPos.x = parent->position.x - TO_FIXED(106);
    drawPos.y = parent->position.y - TO_FIXED(4);
    this->buttonAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 20, true, 0);
    this->buttonAnimator.DrawSprite(&drawPos, false);
    drawPos.x = parent->position.x - TO_FIXED(158);
    drawPos.y = parent->position.y - TO_FIXED(9);
    this->buttonZoneAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 20, true, 4);
    this->buttonZoneAnimator.DrawSprite(&drawPos, false);
}

void UIReplayCarousel::Draw_NoReplays()
{
    UIControl *parent = (UIControl *)this->parent;

    Vector2 drawPos;
    drawPos.x = parent->position.x - TO_FIXED(106);
    drawPos.y = parent->position.y - TO_FIXED(4);
    this->buttonAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 20, true, 0);
    this->buttonAnimator.DrawSprite(&drawPos, false);
    drawPos.x = parent->position.x - TO_FIXED(174);
    drawPos.y = parent->position.y - TO_FIXED(9);
    this->buttonZoneAnimator.SetAnimation(UIWidgets::sVars->buttonFrames, 20, true, 3);
    this->buttonZoneAnimator.DrawSprite(&drawPos, false);
}

void UIReplayCarousel::Draw_Carousel()
{
    UIControl *parent = (UIControl *)this->parent;

    int32 posY = this->popoverPos + this->position.y;
    for (int32 i = 0; i < this->visibleReplayCount; ++i) {
        int32 id = i + this->visibleReplayOffset;
        if (id >= this->sortedRowCount)
            break;

        int32 score       = 0;
        uint8 zoneID      = 0;
        uint8 act         = 0;
        uint8 characterID = 0;

        int32 row = APITable->GetSortedUserDBRowID(globals->replayTableID, id);
        APITable->GetUserDBValue(globals->replayTableID, row, API::Storage::UserDB::UInt32, "score", &score);
        APITable->GetUserDBValue(globals->replayTableID, row, API::Storage::UserDB::UInt8, "zoneID", &zoneID);
        APITable->GetUserDBValue(globals->replayTableID, row, API::Storage::UserDB::UInt8, "act", &act);
        APITable->GetUserDBValue(globals->replayTableID, row, API::Storage::UserDB::UInt8, "characterID", &characterID);
        if (id == this->curReplayID && parent->active == ACTIVE_ALWAYS) {
            //int32 palRow = (zoneID % 12) >> 3;
            //int32 bankID = 0;
            //bankID = palRow + 1;
            //Palette *palette = (Palette *)this;
            //palette->Copy(bankID, (32 * (zoneID % 12)), 224, 32);
            Vector2 drawPos;
            drawPos.x                                = this->position.x - TO_FIXED(1);
            drawPos.y                                = posY + TO_FIXED(24);
            UIWidgets::sVars->arrowLeftAnimator.DrawSprite(&drawPos, false);
        }
        UIReplayCarousel::DrawReplayInfo(this->position.x, posY, zoneID, act, characterID, score, id);
        posY += TO_FIXED(96);
    }
}

void UIReplayCarousel::State_Init()
{
    UIControl *parent = (UIControl *)this->parent;

    for (auto prompt : GameObject::GetEntities<UIButtonPrompt>(FOR_ALL_ENTITIES))
    {
        if (UIControl::ContainsPos(parent, &prompt->position) && prompt->buttonID == 3) {
            sVars->prompt = prompt;
            break;
        }
    }

    this->state.Set(&UIReplayCarousel::State_Unselected);

    this->position.x = this->startPos.x;
}

void UIReplayCarousel::State_Unselected()
{
    this->position.x = this->startPos.x;
}

void UIReplayCarousel::State_Selected()
{
    if (this->curReplayID == -1)
        this->curReplayID = 0;
}

void UIReplayCarousel::State_StartAction()
{
    UIControl *parent = (UIControl *)this->parent;

    this->currentlySelected    = true;
    this->touchCB.Set(nullptr);

    if (this->timer >= 30) {
        this->timer               = 0;
        parent->selectionDisabled = false;
        this->actionCB.Run(this);
        this->state.Set(&UIReplayCarousel::State_Unselected);
    }
    else {
        this->timer++;
    }
}

#if RETRO_INCLUDE_EDITOR
void UIReplayCarousel::EditorDraw()
{
    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y + 0x100000;
    UIWidgets::DrawParallelogram(drawPos.x, drawPos.y, 96, 16, 16, 0x00, 0x00, 0x00);
}

void UIReplayCarousel::EditorLoad(void) { sVars->aniFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE); }
#endif

void UIReplayCarousel::Serialize(void) { RSDK_EDITABLE_VAR(UIReplayCarousel, VAR_BOOL, disabled); }

} // namespace GameLogic