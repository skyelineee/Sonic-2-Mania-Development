// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIControl Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIControl.hpp"
#include "UISaveSlot.hpp"
#include "Helpers/MathHelpers.hpp"
#include "Helpers/DrawHelpers.hpp"
#include "Helpers/LogHelpers.hpp"
#include "UIButtonPrompt.hpp"
#include "UIKeyBinder.hpp"
#include "UIChoice.hpp"
#include "UICarousel.hpp"
#include "UIShifter.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIControl);

void UIControl::Update()
{
    if (this->buttonID >= 0 && this->buttonID != this->lastButtonID)
        this->lastButtonID = this->buttonID;

    if (!sVars->hasTouchInput && this->buttonID == -1)
        this->buttonID = this->lastButtonID;

    this->state.Run(this);

    if (this->backoutTimer > 0)
        this->backoutTimer--;

    this->menuUpdateCB.Run(this);
}
void UIControl::LateUpdate() {}

void UIControl::StaticUpdate()
{
    if (sVars->lockInput) {
        sVars->lockInput   = false;
        sVars->inputLocked = true;
    }
    else {
        sVars->inputLocked = false;
    }

    sVars->forceBackPress = false;

    ++sVars->timer;
    sVars->timer &= 0x7FFF;
}

void UIControl::Draw()
{
    screenInfo->position.x = FROM_FIXED(this->position.x) - screenInfo->center.x;
    screenInfo->position.y = FROM_FIXED(this->position.y) - screenInfo->center.y;
}

void UIControl::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        if (data) {
            Vector2 *size = (Vector2 *)data;
            this->size.x  = size->x;
            this->size.y  = size->y;
        }

        this->updateRange.x = this->size.x >> 1;
        this->updateRange.y = this->size.y >> 1;

        this->promptCount = 0;

        if (this->rowCount <= 1)
            this->rowCount = 1;

        if (this->columnCount <= 1)
            this->columnCount = 1;

        if (!this->hasStoredButton)
            this->buttonID = this->startingID;
        else
            this->buttonID = this->storedButtonID;

        this->position.x += this->cameraOffset.x;
        this->position.y += this->cameraOffset.y;
        this->startPos.x = this->position.x;
        this->startPos.y = this->position.y;

        int32 slotID = this->Slot();
        if (UIButtonPrompt::sVars && slotID != SLOT_DIALOG_UICONTROL) {
            for (auto prompt : GameObject::GetEntities<UIButtonPrompt>(FOR_ALL_ENTITIES))
            {
                if (this->promptCount < UICONTROL_PROMPT_COUNT) {
                    int32 x = this->startPos.x - this->cameraOffset.x;
                    int32 y = this->startPos.y - this->cameraOffset.y;

                    Hitbox hitbox;
                    hitbox.right  = this->size.x >> 17;
                    hitbox.left   = -(this->size.x >> 17);
                    hitbox.bottom = this->size.y >> 17;
                    hitbox.top    = -(this->size.y >> 17);
                    if (MathHelpers::PointInHitbox(x, y, prompt->position.x, prompt->position.y, FLIP_NONE, &hitbox)) {
                        prompt->parent                     = (Entity *)this;
                        this->prompts[this->promptCount++] = prompt;
                    }
                }
            }
        }

        analogStickInfoL[1].deadzone = 0.75f;
        analogStickInfoL[2].deadzone = 0.75f;
        analogStickInfoL[3].deadzone = 0.75f;
        analogStickInfoL[4].deadzone = 0.75f;

        UIControl::SetupButtons();

        if (this->noWidgets) {
            this->active  = ACTIVE_NORMAL;
            this->visible = true;
        }
        else {
            this->menuWasSetup = false;

            if (this->activeOnLoad)
                UIControl::SetActiveMenu(this);
            else
                this->active = ACTIVE_NEVER;
        }
    }
}


void UIControl::StageLoad()
{
    sVars->inputLocked       = false;
    sVars->lockInput         = false;
    sVars->active            = ACTIVE_ALWAYS;
    sVars->isProcessingInput = false;
}

UIControl *UIControl::GetUIControl()
{
    for (auto control : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES)) {
        if (control->active == ACTIVE_ALWAYS) {
            return control;
        }
    }

    return nullptr;
}

void UIControl::ClearInputs(uint8 buttonID)
{
    for (int32 i = 0; i < PLAYER_COUNT; ++i) {
        sVars->upPress[i]      = false;
        sVars->downPress[i]    = false;
        sVars->leftPress[i]    = false;
        sVars->rightPress[i]   = false;
        sVars->yPress[i]       = false;
        sVars->xPress[i]       = false;
        sVars->backPress[i]    = false;
        sVars->confirmPress[i] = false;
        sVars->startPress[i]   = false;
    }

    sVars->anyUpPress    = false;
    sVars->anyDownPress  = false;
    sVars->anyLeftPress  = false;
    sVars->anyRightPress = false;
    sVars->anyYPress     = buttonID == UIButtonPrompt::UIBUTTONPROMPT_BUTTON_Y;
    sVars->anyXPress     = buttonID == UIButtonPrompt::UIBUTTONPROMPT_BUTTON_X;
    sVars->anyStartPress = buttonID == UIButtonPrompt::UIBUTTONPROMPT_BUTTON_SELECT;

    if (API::GetConfirmButtonFlip()) {
        sVars->anyConfirmPress = buttonID == UIButtonPrompt::UIBUTTONPROMPT_BUTTON_B;
        sVars->anyBackPress    = buttonID == UIButtonPrompt::UIBUTTONPROMPT_BUTTON_A;
        sVars->forceBackPress  = buttonID == UIButtonPrompt::UIBUTTONPROMPT_BUTTON_A;
    }
    else {
        sVars->anyConfirmPress = buttonID == UIButtonPrompt::UIBUTTONPROMPT_BUTTON_A;
        sVars->anyBackPress    = buttonID == UIButtonPrompt::UIBUTTONPROMPT_BUTTON_B;
        sVars->forceBackPress  = buttonID == UIButtonPrompt::UIBUTTONPROMPT_BUTTON_B;
    }

    sVars->lockInput   = true;
    sVars->inputLocked = true;
}

void UIControl::ProcessInputs()
{
    UIControl::HandlePosition();

    if (!sVars->inputLocked) {
        for (int32 i = 0; i < PLAYER_COUNT; ++i) {
            sVars->upPress[i]    = controllerInfo[Input::CONT_P1 + i].keyUp.press | analogStickInfoL[Input::CONT_P1 + i].keyUp.press;
            sVars->downPress[i]  = controllerInfo[Input::CONT_P1 + i].keyDown.press | analogStickInfoL[Input::CONT_P1 + i].keyDown.press;
            sVars->leftPress[i]  = controllerInfo[Input::CONT_P1 + i].keyLeft.press | analogStickInfoL[Input::CONT_P1 + i].keyLeft.press;
            sVars->rightPress[i] = controllerInfo[Input::CONT_P1 + i].keyRight.press | analogStickInfoL[Input::CONT_P1 + i].keyRight.press;

            if (sVars->upPress[i] && sVars->downPress[i]) {
                sVars->upPress[i] = false;
                sVars->downPress[i] = false;
            }

            if (sVars->leftPress[i] && sVars->rightPress[i]) {
                sVars->leftPress[i] = false;
                sVars->rightPress[i] = false;
            }

            sVars->yPress[i] = controllerInfo[Input::CONT_P1 + i].keyY.press;
            sVars->xPress[i] = controllerInfo[Input::CONT_P1 + i].keyX.press;
            sVars->startPress[i] = controllerInfo[Input::CONT_P1 + i].keyStart.press;

            sVars->confirmPress[i] = controllerInfo[Input::CONT_P1 + i].keyStart.press;
            if (API::GetConfirmButtonFlip()) {
                sVars->confirmPress[i] |= controllerInfo[Input::CONT_P1 + i].keyB.press;
                sVars->backPress[i] = controllerInfo[Input::CONT_P1 + i].keyA.press;
            }
            else {
                sVars->confirmPress[i] |= controllerInfo[Input::CONT_P1 + i].keyA.press;
                sVars->backPress[i] = controllerInfo[Input::CONT_P1 + i].keyB.press;
            }
        }

        sVars->anyUpPress    = controllerInfo->keyUp.press | analogStickInfoL->keyUp.press;
        sVars->anyDownPress  = controllerInfo->keyDown.press | analogStickInfoL->keyDown.press;
        sVars->anyLeftPress  = controllerInfo->keyLeft.press | analogStickInfoL->keyLeft.press;
        sVars->anyRightPress = controllerInfo->keyRight.press | analogStickInfoL->keyRight.press;
        sVars->anyYPress     = controllerInfo->keyY.press;
        sVars->anyXPress     = controllerInfo->keyX.press;
        sVars->anyStartPress = controllerInfo->keyStart.press;

        sVars->anyConfirmPress = controllerInfo->keyStart.press;
        if (API::GetConfirmButtonFlip()) {
            sVars->anyConfirmPress |= controllerInfo->keyB.press;
            sVars->anyBackPress = controllerInfo->keyA.press;
        }
        else {
            sVars->anyConfirmPress |= controllerInfo->keyA.press;
            sVars->anyBackPress = controllerInfo->keyB.press;
        }

        sVars->anyBackPress |= unknownInfo->pausePress;
        sVars->anyBackPress |= sVars->forceBackPress;

        if (sVars->anyBackPress) {
            sVars->anyConfirmPress = false;
            sVars->anyYPress       = false;
        }

        if (sVars->anyConfirmPress) {
            sVars->anyYPress = false;
        }

        sVars->inputLocked = true;
    }

    if (!this->selectionDisabled) {
        bool32 backPressed = false;

        if (sVars->anyBackPress) {
            if (!this->childHasFocus && !this->dialogHasFocus
                && !this->popoverHasFocus
                && this->backoutTimer <= 0) {
                if (!this->backPressCB.Matches(nullptr)) {
                    backPressed = this->backPressCB.Run(this);

                    if (!backPressed) {
                        sVars->anyBackPress = false;
                    }
                    else {
                        if (this->buttons[this->buttonID])
                            this->buttons[this->buttonID]->isSelected = false;
                    }
                }
                else {
                    if (this->parentTag.length <= 0) {
                        sVars->anyBackPress = false;
                    }
                    else {
                        this->selectionDisabled = true;
                        // UITransition::StartTransition(UIControl::ReturnToParentMenu, 0); //
                        backPressed = false;

                        if (this->buttons[this->buttonID])
                            this->buttons[this->buttonID]->isSelected = false;
                    }
                }

                if (backPressed)
                    return;
            }
            else {
                LogHelpers::Print("Backout prevented");
                LogHelpers::Print("childHasFocus = %d", this->childHasFocus);
                LogHelpers::Print("dialogHasFocus = %d", this->dialogHasFocus);
                LogHelpers::Print("popoverHasFocus = %d", this->popoverHasFocus);
                LogHelpers::Print("backoutTimer = %d", this->backoutTimer);
            }
        }

        if (!this->processButtonInputCB.Matches(nullptr)) {
            this->processButtonInputCB.Run(this);
        }
        else
            UIControl::ProcessButtonInput();

        if (!this->selectionDisabled) {
            if (sVars->anyYPress) {
                if (!this->childHasFocus && !this->dialogHasFocus
                    && !this->popoverHasFocus
                    && this->backoutTimer <= 0) {
                    this->yPressCB.Run(this);
                }

                sVars->anyYPress = false;
            }

            if (sVars->anyXPress) {
                if (!this->childHasFocus && !this->dialogHasFocus
                    && !this->popoverHasFocus
                    && this->backoutTimer <= 0) {
                    this->xPressCB.Run(this);
                }

                sVars->anyXPress = false;
            }
        }
    }
}

int32 UIControl::GetButtonID(UIControl *control, UIButton *entity)
{
    for (int32 i = 0; i < control->buttonCount; ++i) {
        if (entity == control->buttons[i])
            return i;
    }

    return -1;
}

void UIControl::MenuChangeButtonInit(UIControl *control)
{
    Entity *storeEntity = (Entity *)sceneInfo->entity;
    for (int32 i = 0; i < SCENEENTITY_COUNT; ++i) {
        UIButton *entity = GameObject::Get<UIButton>(i);

        if (entity) {
            int32 left   = MIN(-screenInfo->size.x >> 1, screenInfo->size.x >> 1) << 16;
            int32 right  = MAX(-screenInfo->size.x >> 1, screenInfo->size.x >> 1) << 16;
            int32 top    = MIN(-screenInfo->size.y >> 1, screenInfo->size.y >> 1) << 16;
            int32 bottom = MAX(-screenInfo->size.y >> 1, screenInfo->size.y >> 1) << 16;

            if (entity->position.x >= control->position.x + left && entity->position.x <= control->position.x + right) {
                if (entity->position.y >= control->position.y + top && entity->position.y <= control->position.y + bottom) {
                    int32 slot = entity->Slot();

                    sceneInfo->entity = (Entity *)entity;
                    if (UIButton::sVars && entity->classID == UIButton::sVars->classID) {
                        UIButton *button = (UIButton *)entity;

                        UIButton::ManageChoices(button);
                        button->Update();
                    }

                    else if (UIChoice::sVars && entity->classID == UIChoice::sVars->classID) {
                        UIChoice *choice = (UIChoice *)entity;

                        choice->Update();
                    }

                    // TODO: uncomment this when its done //
                    /*else if (UITAZoneModule::sVars && entity->classID == UITAZoneModule::sVars->classID) {
                        UITAZoneModule *zoneModule = (UITAZoneModule *)entity;

                        zoneModule->Update();
                    }*/

                    else if (UIHeading::sVars && entity->classID == UIHeading::sVars->classID) {
                        UIHeading *heading = (UIHeading *)entity;

                        heading->Update();
                    }

                    if (entity->visible)
                        Graphics::AddDrawListRef(entity->drawGroup, slot);

                    sceneInfo->entity = storeEntity;
                }
            }
        }
    }
}

void UIControl::SetActiveMenuButtonPrompts(UIControl *entity)
{
    for (int32 i = 0; i < entity->promptCount; ++i) entity->prompts[i]->active = ACTIVE_NORMAL;
}

void UIControl::SetActiveMenu(UIControl *entity)
{
    LogHelpers::PrintString(&entity->tag);

    entity->active  = ACTIVE_ALWAYS;
    entity->visible = true;

    if (entity->hasStoredButton) {
        entity->buttonID        = entity->storedButtonID;
        entity->storedButtonID  = 0;
        entity->hasStoredButton = false;
    }
    else if (entity->resetSelection) {
        entity->buttonID = entity->startingID;
    }

    Graphics::ClearCameras();
    Graphics::AddCamera(&entity->position, screenInfo->size.x << 16, screenInfo->size.y << 16, true);

    UIControl::MenuChangeButtonInit(entity);

    if (!entity->childHasFocus && (entity->resetSelection || !entity->menuWasSetup)) {

        entity->position.x  = entity->startPos.x;
        entity->position.y  = entity->startPos.y;
        entity->targetPos.x = entity->startPos.x;
        entity->targetPos.y = entity->startPos.y;
    }

    entity->state.Set(&UIControl::ProcessInputs);
    entity->childHasFocus = false;

    entity->menuWasSetup = true;

    for (int32 p = 0; p < entity->promptCount; ++p) entity->prompts[p]->active = ACTIVE_NORMAL;

    if (!entity->menuSetupCB.Matches(nullptr)) {
        Entity *storeEntity = (Entity *)sceneInfo->entity;
        sceneInfo->entity   = (Entity *)entity;
#if RETRO_USE_MOD_LOADER
        entity->menuSetupCB.Run(entity);
#else
        entity->menuSetupCB();
#endif
        sceneInfo->entity = storeEntity;
    }
}

void UIControl::SetMenuLostFocus(UIControl *entity)
{
    entity->active  = ACTIVE_ALWAYS;
    entity->visible = true;

    if (!entity->dialogHasFocus && !entity->childHasFocus
        && !entity->popoverHasFocus)
    {
        if (entity->hasStoredButton) {
            entity->buttonID        = entity->storedButtonID;
            entity->storedButtonID  = 0;
            entity->hasStoredButton = false;
        }
        else if (entity->resetSelection) {
            entity->buttonID = entity->startingID;
        }
        if (entity->resetSelection || !entity->menuWasSetup) {
            entity->position.x  = entity->startPos.x;
            entity->position.y  = entity->startPos.y;
            entity->targetPos.x = entity->startPos.x;
            entity->targetPos.y = entity->startPos.y;
        }

        entity->menuWasSetup = true;
        UIControl::SetActiveMenuButtonPrompts(entity);
        entity->state.Set(&UIControl::ProcessInputs);
    }
}

void UIControl::SetInactiveMenu(UIControl *control)
{
    sVars->hasTouchInput = false;
    control->active          = ACTIVE_NEVER;
    control->visible         = false;
    control->state.Set(nullptr);

    if (this->promptCount) {
        for (int32 p = 0; p < control->promptCount; ++p) control->prompts[p]->active = ACTIVE_BOUNDS;
    }
}

void UIControl::SetupButtons()
{
    int32 slotID = this->Slot();

    Hitbox hitboxRange;
    if (UIHeading::sVars && slotID != SLOT_DIALOG_UICONTROL) {
        for (auto heading : GameObject::GetEntities<UIHeading>(FOR_ALL_ENTITIES))
        {
            int32 x            = this->startPos.x - this->cameraOffset.x;
            int32 y            = this->startPos.y - this->cameraOffset.y;
            hitboxRange.left   = -(this->size.x >> 17);
            hitboxRange.top    = -(this->size.y >> 17);
            hitboxRange.right  = this->size.x >> 17;
            hitboxRange.bottom = this->size.y >> 17;

            if (MathHelpers::PointInHitbox(x, y, heading->position.x, heading->position.y, FLIP_NONE, &hitboxRange))
                this->heading = heading;
        }
    }

    if (UIShifter::sVars && slotID != SLOT_DIALOG_UICONTROL) {
        for (auto shifter : GameObject::GetEntities<UIShifter>(FOR_ALL_ENTITIES))
        {
            int32 x            = this->startPos.x - this->cameraOffset.x;
            int32 y            = this->startPos.y - this->cameraOffset.y;
            hitboxRange.right  = this->size.x >> 17;
            hitboxRange.left   = -(this->size.x >> 17);
            hitboxRange.bottom = (this->size.y >> 17);
            hitboxRange.top    = -(this->size.y >> 17);

            if (MathHelpers::PointInHitbox(x, y, shifter->position.x, shifter->position.y, FLIP_NONE, &hitboxRange)) {
                this->shifter   = shifter;
                shifter->parent = this;
            }
        }
    }

    if (UICarousel::sVars && slotID != SLOT_DIALOG_UICONTROL) {
        for (auto carousel : GameObject::GetEntities<UICarousel>(FOR_ALL_ENTITIES))
        {
            int32 x            = this->startPos.x - this->cameraOffset.x;
            int32 y            = this->startPos.y - this->cameraOffset.y;
            hitboxRange.right  = this->size.x >> 17;
            hitboxRange.left   = -(this->size.x >> 17);
            hitboxRange.bottom = this->size.y >> 17;
            hitboxRange.top    = -(this->size.y >> 17);

            if (MathHelpers::PointInHitbox(x, y, carousel->position.x, carousel->position.y, FLIP_NONE, &hitboxRange)) {
                this->carousel   = carousel;
                carousel->parent = this;
            }
        }
    }

    for (int32 i = 0; i < SCENEENTITY_COUNT; ++i) {
        UIButton *button = GameObject::Get<UIButton>(i);

        if (button) {
            int32 classID = button->classID;
            if (classID != UIButton::sVars->classID && (!UISaveSlot::sVars || classID != UISaveSlot::sVars->classID)
                //&& (!UITAZoneModule || classID != UITAZoneModule->classID)
                /*&& (!UISlider || classID != UISlider->classID)*/ && (!UIKeyBinder::sVars || classID != UIKeyBinder::sVars->classID)) {
            }
            else {
                int32 x            = this->startPos.x - this->cameraOffset.x;
                int32 y            = this->startPos.y - this->cameraOffset.y;
                hitboxRange.left   = -(this->size.x >> 17);
                hitboxRange.top    = -(this->size.y >> 17);
                hitboxRange.right  = this->size.x >> 17;
                hitboxRange.bottom = this->size.y >> 17;

                if (MathHelpers::PointInHitbox(x, y, button->position.x, button->position.y, FLIP_NONE, &hitboxRange)) {
                    if (this->buttonCount < UICONTROL_BUTTON_COUNT) {
                        if (!button->parent)
                            button->parent = (Entity *)this;

                        this->buttons[this->buttonCount++] = button;
                    }
                }
            }
        }
    }
}

bool32 UIControl::isMoving(UIControl *entity)
{
    if (entity->scrollSpeed.x && entity->scrollSpeed.y) {
        return entity->position.x != entity->targetPos.x || entity->position.y != entity->targetPos.y;
    }
    else {
        if (entity->scrollSpeed.x) {
            return entity->position.x != entity->targetPos.x;
        }
        else if (entity->scrollSpeed.y) {
            return entity->position.y != entity->targetPos.y;
        }
    }

    return false;
}

void UIControl::MatchMenuTag(const char *text)
{
    String string;
    string.Init(nullptr);

    string.Set(text);
    for (auto entity : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES))
    {
        if (entity->active == ACTIVE_ALWAYS || !string.Compare(&string, &entity->tag, false))
            entity->SetInactiveMenu(entity);
        else
            UIControl::SetActiveMenu(entity);
    }
}

void UIControl::HandleMenuChange(String *newMenuTag)
{
    if (newMenuTag->length) {
        for (auto entity : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES))
        {
            if (entity->active == ACTIVE_ALWAYS || !newMenuTag->Compare(newMenuTag, &entity->tag, false))
                entity->SetInactiveMenu(entity);
            else
                UIControl::SetActiveMenu(entity);
        }
    }
}

void UIControl::HandleMenuLoseFocus(UIControl *parent)
{
    for (auto entity : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES))
    {
        if (entity->active == ACTIVE_ALWAYS || entity != parent)
            entity->SetInactiveMenu(entity);
        else
            UIControl::SetMenuLostFocus(entity);
    }
}

void UIControl::ReturnToParentMenu()
{
    UIControl *entity   = UIControl::GetUIControl();
    entity->selectionDisabled = false;

    UIControl::HandleMenuChange(&entity->parentTag);
}

void UIControl::SetTargetPos(UIControl *entity, int32 x, int32 y)
{
    int32 targetX = x;
    if (!x) {
        targetX = entity->position.x;
        x       = entity->position.x;
    }

    int32 targetY = y;
    if (!y) {
        targetY = entity->position.y;
        y       = entity->position.y;
    }

    if (!entity->noClamp) {
        int32 startX = entity->startPos.x - entity->cameraOffset.x;
        int32 startY = entity->startPos.y - entity->cameraOffset.y;
        int32 x1     = startX + (screenInfo->size.x << 15) - (entity->size.x >> 1);
        int32 x2     = startX + (entity->size.x >> 1) - (screenInfo->size.x << 15);
        int32 y1     = startY + (screenInfo->size.y << 15) - (entity->size.y >> 1);
        int32 y2     = startY + (entity->size.y >> 1) - (screenInfo->size.y << 15);

        if (x < x2)
            x2 = x;
        targetX = x2;

        if (x1 > x2)
            targetX = x1;

        if (y < y2)
            y2 = y;
        targetY = y2;

        if (y1 > y2)
            targetY = y1;
    }

    entity->targetPos.x = targetX;
    entity->targetPos.y = targetY;
}

void UIControl::HandlePosition()
{
    if (this->position.x < this->targetPos.x) {
        this->position.x += this->scrollSpeed.x;
        if (this->position.x > this->targetPos.x)
            this->position.x = this->targetPos.x;
    }
    else if (this->position.x > this->targetPos.x) {
        this->position.x -= this->scrollSpeed.x;
        if (this->position.x < this->targetPos.x)
            this->position.x = this->targetPos.x;
    }

    if (this->position.y < this->targetPos.y) {
        this->position.y += this->scrollSpeed.y;
        if (this->position.y > this->targetPos.y)
            this->position.y = this->targetPos.y;
    }
    else if (this->position.y > this->targetPos.y) {
        this->position.y -= this->scrollSpeed.y;
        if (this->position.y < this->targetPos.y)
            this->position.y = this->targetPos.y;
    }

    if (this->heading)
        this->heading->position.x = this->position.x;
}

void UIControl::ProcessButtonInput()
{
    bool32 allowAction = false;
    if (touchInfo->count || sVars->hasTouchInput) {
        UIButton *activeButton = 0;
        sVars->hasTouchInput     = touchInfo->count != 0;
        sVars->isProcessingInput = true;

        for (int32 i = 0; i < this->buttonCount; ++i) {
            if (this->buttons[i]) {
                UIButton *button = this->buttons[i];

                Entity *storeEntity = (Entity *)sceneInfo->entity;
                sceneInfo->entity   = (Entity *)button;
                if (!button->touchCB.Matches(nullptr) && !this->dialogHasFocus
                    && !this->popoverHasFocus)
                {
                    if (button->checkSelectedCB.Matches(nullptr) || button->checkSelectedCB.Matches(nullptr)) {
                        if (allowAction || !button->touchCB.Matches(nullptr)) {
                            allowAction = true;
                            if (!button->touchCB.Matches(nullptr) && !activeButton)
                                activeButton = button;
                        }
                        else {
                            allowAction = false;
                        }
                    }
                }

                sceneInfo->entity = storeEntity;
            }
        }

        if (touchInfo->count) {
            if (allowAction) {
                int32 id = -1;

                for (int32 i = 0; i < this->buttonCount; ++i) {
                    if (activeButton == this->buttons[i]) {
                        id = i;
                        break;
                    }
                }

                this->buttonID = id;
                if (activeButton->isSelected) {
                    Entity *storeEntity      = (Entity *)sceneInfo->entity;
                    sceneInfo->entity        = (Entity *)activeButton;
                    activeButton->isSelected = true;
                    activeButton->buttonEnterCB.Run(activeButton);
                    sceneInfo->entity = storeEntity;
                }
            }
            else {
                this->buttonID = -1;
            }
        }

        sVars->isProcessingInput = false;
    }

    if (this->buttonID >= 0) {
        if (this->buttonID < this->buttonCount) {
            UIButton *button = this->buttons[this->buttonID];

            if (button) {
                Entity *storeEntity = (Entity *)sceneInfo->entity;
                sceneInfo->entity   = (Entity *)button;
                if (!button->processButtonCB.Matches(nullptr)) {
                    if (button->checkSelectedCB.Matches(nullptr) || !button->checkSelectedCB.Run(this)) {
#if RETRO_USE_MOD_LOADER
                        button->processButtonCB.Run(button);
#else
                        button->processButtonCB();
#endif
                    }
                }
                sceneInfo->entity = storeEntity;
            }
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void UIControl::EditorDraw()
{
    this->updateRange.x = this->size.x >> 1;
    this->updateRange.y = this->size.y >> 1;

    DrawHelpers::DrawRectOutline(this->position.x, this->position.y, this->size.x, this->size.y, 0xFFFF00);

    this->animator.SetAnimation(&sVars->aniFrames, 0, false, 0);
    this->animator.DrawSprite(NULL, false);
}

void UIControl::EditorLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }
#endif

void UIControl::Serialize()
{
    RSDK_EDITABLE_VAR(UIControl, VAR_STRING, tag);
    RSDK_EDITABLE_VAR(UIControl, VAR_STRING, parentTag);
    RSDK_EDITABLE_VAR(UIControl, VAR_BOOL, activeOnLoad);
    RSDK_EDITABLE_VAR(UIControl, VAR_BOOL, noWidgets);
    RSDK_EDITABLE_VAR(UIControl, VAR_BOOL, resetSelection);
    RSDK_EDITABLE_VAR(UIControl, VAR_UINT8, rowCount);
    RSDK_EDITABLE_VAR(UIControl, VAR_UINT8, columnCount);
    RSDK_EDITABLE_VAR(UIControl, VAR_UINT8, startingID);
    RSDK_EDITABLE_VAR(UIControl, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(UIControl, VAR_VECTOR2, cameraOffset);
    RSDK_EDITABLE_VAR(UIControl, VAR_VECTOR2, scrollSpeed);
    RSDK_EDITABLE_VAR(UIControl, VAR_BOOL, noClamp);
    RSDK_EDITABLE_VAR(UIControl, VAR_BOOL, noWrap);
}
} // namespace GameLogic