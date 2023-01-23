// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIKeyBinder Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIKeyBinder.hpp"
#include "UIWidgets.hpp"
#include "UIControl.hpp"
#include "UIButtonPrompt.hpp"
#include "Global/Localization.hpp"
#include "Helpers/LogHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIKeyBinder);

void UIKeyBinder::Update()
{
    this->touchPosSizeS.x   = this->size.x;
    this->touchPosOffsetS.x = 0;
    this->touchPosOffsetS.y = 0;
    this->touchPosSizeS.x += 3 * this->size.y;
    this->touchPosSizeS.y = this->size.y + 0x60000;

    if (!this->textFrames.Matches(&UIWidgets::sVars->textFrames)) {
        this->labelAnimator.SetAnimation(&UIWidgets::sVars->textFrames, this->listID, true, this->frameID);
        this->textFrames = UIWidgets::sVars->textFrames;
    }

    UIControl *parent = (UIControl *)this->parent;
    int32 inputID           = this->inputID + 1;
    int32 keyMap            = UIKeyBinder::GetMappings(inputID, this->type);

    String string;
    bool32 keyMapChanged = true;

    int32 frameID = -1;
    if (this->lasyKeyMap == keyMap) {
        keyMapChanged = false;
    }
    else if (keyMap == KEYMAP_AUTO_MAPPING) {
        this->keyAnimator.SetAnimation(&sVars->aniFrames, UIKeyBinder::GetButtonListID(), true, 0);
        this->lasyKeyMap = KEYMAP_AUTO_MAPPING;
        keyMapChanged    = false;
    }
    else {
        frameID = UIButtonPrompt::MappingsToFrame(keyMap);
    }

    for (int32 buttonID = 0; buttonID <= 8 && keyMapChanged; ++buttonID) {
        for (int32 inputSlot = 1; inputSlot <= 2 && keyMapChanged; ++inputSlot) {
            if ((buttonID != this->type || inputSlot != inputID) && keyMap) {
                if (UIKeyBinder::GetMappings(inputSlot, buttonID) != keyMap)
                    continue;

                if (this->state.Set(&UIKeyBinder::State_Selected)) {
                    sVars->activeInputID  = inputSlot;
                    sVars->activeButtonID = buttonID;

                    int32 str = -1;
                    if (inputSlot == inputID)
                        str = Localization::KeyAlreadyBound;
                    else if (inputSlot == Input::CONT_P1)
                        str = Localization::KeyAlreadyBoundP1;
                    else if (inputSlot == Input::CONT_P2)
                        str = Localization::KeyAlreadyBoundP2;

                    if (str != -1)
                        Localization::GetString(&string, str);

                    UIKeyBinder::SetMappings(this->type, inputID, KEYMAP_NO_MAPPING);
                    this->lasyKeyMap = KEYMAP_NO_MAPPING;
                    keyMapChanged = false;
                }
            }
        }
    }

    if (keyMapChanged) {
        if (frameID || this->state.Set(&UIKeyBinder::State_Selected)) {
            this->keyAnimator.SetAnimation(&sVars->aniFrames, UIKeyBinder::GetButtonListID(), true, frameID);
            this->lasyKeyMap = keyMap;

            if (this->state.Set(&UIKeyBinder::State_Selected)) {
                LogHelpers::Print("bind = %d 0x%02x", keyMap, keyMap);

                sVars->isSelected   = false;
                parent->selectionDisabled = false;
                this->processButtonCB.Set(&UIButton::ProcessButtonCB_Scroll);
                this->state.Set(&UIKeyBinder::State_HandleButtonEnter);

                sVars->activeBinder = NULL;
                parent->childHasFocus     = false;

               
                Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, true);
                UIWidgets::sVars->sfxAccept.Play(false, 255);
            }
        }
        else {
            LogHelpers::Print("bind = %d 0x%02x", keyMap, keyMap);

            int32 frame = UIButtonPrompt::MappingsToFrame(this->lasyKeyMap);
            this->keyAnimator.SetAnimation(sVars->aniFrames, UIKeyBinder::GetButtonListID(), true, frame);
            UIKeyBinder::SetMappings(inputID, this->type, KEYMAP_AUTO_MAPPING);

            sVars->sfxFail.Play(false, 255);
        }
    }

    this->state.Run(this);

    int32 id = -1;
    for (int32 i = 0; i < parent->buttonCount; ++i) {
        if (this == (UIKeyBinder *)parent->buttons[i]) {
            id = i;
            break;
        }
    }

    if (this->state.Matches(&UIKeyBinder::State_HandleButtonEnter)) {
        if ((!parent->state.Matches(&UIControl::ProcessInputs) || parent->buttonID != id)) {
            this->isSelected = false;
            this->state.Set(&UIKeyBinder::State_HandleButtonLeave);
        }
    }
}
void UIKeyBinder::LateUpdate() {}
void UIKeyBinder::StaticUpdate() {}
void UIKeyBinder::Draw() {}

void UIKeyBinder::Create(void *data)
{
    this->visible       = true;
    this->drawGroup     = 2;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x400000;
    this->size.x        = 0x400000;
    this->size.y        = 0xC0000;
    this->bgEdgeSize    = 12;

    this->processButtonCB.Set(&UIButton::ProcessButtonCB_Scroll);
    this->touchCB.Set(&UIButton::ProcessTouchCB_Single);
    this->actionCB.Set(&UIKeyBinder::ActionCB);
    this->selectedCB.Set(&UIKeyBinder::SelectedCB);
    this->failCB.Set(NULL);
    this->buttonEnterCB.Set(&UIKeyBinder::ButtonEnterCB);
    this->buttonLeaveCB.Set(&UIKeyBinder::ButtonLeaveCB);
    this->checkButtonEnterCB.Set(&UIKeyBinder::CheckButtonEnterCB);
    this->checkSelectedCB.Set(&UIKeyBinder::CheckSelectedCB);
    this->textVisible = true;

    this->unused1 = 512;
    this->listID  = 0; // this prolly could've been a constant, I don't think its ever set to anything but 0
    this->frameID = UIKeyBinder::GetKeyNameFrameID(this->type);

    this->labelAnimator.SetAnimation(&UIWidgets::sVars->textFrames, this->listID, true, this->frameID);
    this->textFrames = UIWidgets::sVars->textFrames;

    if (!sceneInfo->inEditor) {
        int32 mappings = UIKeyBinder::GetMappings(this->inputID + 1, this->type);
        int32 frame    = UIButtonPrompt::MappingsToFrame(mappings);
        this->keyAnimator.SetAnimation(&sVars->aniFrames, UIKeyBinder::GetButtonListID(), true, frame);
    }
}

void UIKeyBinder::StageLoad() 
{ 
    sVars->aniFrames.Load("UI/Buttons.bin", SCOPE_STAGE);

    sVars->sfxFail.Get("Stage/Fail.wav");
}

int32 UIKeyBinder::GetButtonListID()
{
    switch (Localization::sVars->language) {
        default:
        case LANGUAGE_EN: return UIButtonPrompt::UIBUTTONPROMPT_KEYBOARD;
        case LANGUAGE_FR: return UIButtonPrompt::UIBUTTONPROMPT_KEYBOARD_FR;
        case LANGUAGE_IT: return UIButtonPrompt::UIBUTTONPROMPT_KEYBOARD_IT;
        case LANGUAGE_GE: return UIButtonPrompt::UIBUTTONPROMPT_KEYBOARD_GE;
        case LANGUAGE_SP: return UIButtonPrompt::UIBUTTONPROMPT_KEYBOARD_SP;
    }
}

int32 UIKeyBinder::GetMappings(int32 input, int32 button)
{
    switch (button) {
        case UIKEYBINDER_UP: return controllerInfo[input].keyUp.keyMap; break;
        case UIKEYBINDER_DOWN: return controllerInfo[input].keyDown.keyMap; break;
        case UIKEYBINDER_LEFT: return controllerInfo[input].keyLeft.keyMap; break;
        case UIKEYBINDER_RIGHT: return controllerInfo[input].keyRight.keyMap; break;
        case UIKEYBINDER_A: return controllerInfo[input].keyA.keyMap; break;
        case UIKEYBINDER_B: return controllerInfo[input].keyB.keyMap; break;
        case UIKEYBINDER_X: return controllerInfo[input].keyX.keyMap; break;
        case UIKEYBINDER_Y: return controllerInfo[input].keyY.keyMap; break;
        case UIKEYBINDER_START: return controllerInfo[input].keyStart.keyMap; break;
        default: break;
    }

    return KEYMAP_NO_MAPPING;
}

void UIKeyBinder::SetMappings(int32 input, int32 button, int32 keyMap)
{
    switch (button) {
        case UIKEYBINDER_UP: controllerInfo[input].keyUp.keyMap = keyMap; break;
        case UIKEYBINDER_DOWN: controllerInfo[input].keyDown.keyMap = keyMap; break;
        case UIKEYBINDER_LEFT: controllerInfo[input].keyLeft.keyMap = keyMap; break;
        case UIKEYBINDER_RIGHT: controllerInfo[input].keyRight.keyMap = keyMap; break;
        case UIKEYBINDER_A: controllerInfo[input].keyA.keyMap = keyMap; break;
        case UIKEYBINDER_B: controllerInfo[input].keyB.keyMap = keyMap; break;
        case UIKEYBINDER_X: controllerInfo[input].keyX.keyMap = keyMap; break;
        case UIKEYBINDER_Y: controllerInfo[input].keyY.keyMap = keyMap; break;
        case UIKEYBINDER_START: controllerInfo[input].keyStart.keyMap = keyMap; break;
        default: break;
    }
}

int32 UIKeyBinder::GetKeyNameFrameID(int32 id)
{
    switch (id) {
        case UIKEYBINDER_UP: return UIKEYBINDER_FRAME_UP;
        case UIKEYBINDER_DOWN: return UIKEYBINDER_FRAME_DOWN;
        case UIKEYBINDER_LEFT: return UIKEYBINDER_FRAME_LEFT;
        case UIKEYBINDER_RIGHT: return UIKEYBINDER_FRAME_RIGHT;
        case UIKEYBINDER_A: return UIKEYBINDER_FRAME_A;
        case UIKEYBINDER_B: return UIKEYBINDER_FRAME_B;
        case UIKEYBINDER_X: return UIKEYBINDER_FRAME_X;
        case UIKEYBINDER_Y: return UIKEYBINDER_FRAME_Y;
        case UIKEYBINDER_START: return UIKEYBINDER_FRAME_START;
        default: break;
    }

    return 0;
}

void UIKeyBinder::DrawSprites()
{
    Vector2 drawPos;
    drawPos.x = this->position.x - this->buttonBounceOffset;
    drawPos.y = this->position.y - this->buttonBounceOffset;

    drawPos.x += this->buttonBounceOffset;
    drawPos.y += this->buttonBounceOffset;
    drawPos.x += this->buttonBounceOffset;
    drawPos.y += this->buttonBounceOffset;
 
    drawPos.x = this->position.x + this->buttonBounceOffset;
    drawPos.y = this->position.y + this->buttonBounceOffset;
    drawPos.x += 0xB0000 - (this->size.x >> 1);
    drawPos.y += this->textBounceOffset;
    this->keyAnimator.DrawSprite(&drawPos, false);

    if (this->textVisible) {
        drawPos.x += 0x60000;
        this->labelAnimator.DrawSprite(&drawPos, false);
    }
}

void UIKeyBinder::ActionCB() {}

bool32 UIKeyBinder::CheckButtonEnterCB()
{
    return this->state.Set(&UIKeyBinder::State_HandleButtonEnter);
}

bool32 UIKeyBinder::CheckSelectedCB()
{
    return this->state.Set(&UIKeyBinder::State_Selected);
}

void UIKeyBinder::ButtonEnterCB()
{
    if (this->state.Set(&UIKeyBinder::State_HandleButtonEnter)) {
        this->textBounceOffset     = 0;
        this->buttonBounceOffset   = 0;
        this->textBounceVelocity   = -0x20000;
        this->buttonBounceVelocity = -0x20000;

        this->state.Set(&UIKeyBinder::State_HandleButtonEnter);
    }
}

void UIKeyBinder::ButtonLeaveCB()
{
    this->state.Set(&UIKeyBinder::State_HandleButtonLeave);
}

void UIKeyBinder::SelectedCB()
{
    if (!sVars->isSelected) {
        sVars->isSelected = true;

        UIControl *parent   = (UIControl *)this->parent;
        parent->childHasFocus     = true;
        parent->selectionDisabled = true;

        sVars->activeBinder = this;
        this->state.Set(&UIKeyBinder::State_Selected);

        UIWidgets::sVars->sfxAccept.Play(false, 255);

        UIKeyBinder::SetMappings(this->inputID + 1, this->type, KEYMAP_AUTO_MAPPING);
    }
}

void UIKeyBinder::State_HandleButtonLeave()
{
    this->textVisible = true;

    if (this->textBounceOffset) {
        int32 dist = -(this->textBounceOffset / abs(this->textBounceOffset));
        this->textBounceOffset += dist << 15;

        if (dist < 0) {
            if (this->textBounceOffset < 0)
                this->textBounceOffset = 0;
            else if (dist > 0 && this->textBounceOffset > 0)
                this->textBounceOffset = 0;
        }
        else if (dist > 0 && this->textBounceOffset > 0)
            this->textBounceOffset = 0;
    }

    if (this->buttonBounceOffset) {
        int32 dist = -(this->buttonBounceOffset / abs(this->buttonBounceOffset));
        this->buttonBounceOffset += dist << 16;

        if (dist < 0) {
            if (this->buttonBounceOffset < 0)
                this->buttonBounceOffset = 0;
            else if (dist > 0 && this->buttonBounceOffset > 0)
                this->buttonBounceOffset = 0;
        }
        else if (dist > 0 && this->buttonBounceOffset > 0)
            this->buttonBounceOffset = 0;
    }
}

void UIKeyBinder::State_HandleButtonEnter()
{
    this->textBounceVelocity += 0x4000;
    this->textBounceOffset += this->textBounceVelocity;

    this->textVisible = true;
    if (this->textBounceOffset >= 0 && this->textBounceVelocity > 0) {
        this->textBounceOffset   = 0;
        this->textBounceVelocity = 0;
    }

    this->buttonBounceVelocity += 0x4800;
    this->buttonBounceOffset += this->buttonBounceVelocity;

    if (this->buttonBounceOffset >= -0x20000 && this->buttonBounceVelocity > 0) {
        this->buttonBounceOffset   = -0x20000;
        this->buttonBounceVelocity = 0;
    }
}

void UIKeyBinder::State_Selected()
{
    UIKeyBinder::State_HandleButtonEnter();

    this->processButtonCB.Set(&UIKeyBinder::ActionCB);
    this->textVisible     = !((this->timer >> 1) & 1);
    this->timer++;
}

void UIKeyBinder::MoveKeyToActionCB_No()
{
    UIKeyBinder *binder = sVars->activeBinder;

    if (binder->state.Set(&UIKeyBinder::State_Selected)) {
        UIKeyBinder::SetMappings(binder->inputID + 1, binder->type, KEYMAP_AUTO_MAPPING);
        sVars->activeInputID  = KEYMAP_AUTO_MAPPING;
        sVars->activeButtonID = KEYMAP_AUTO_MAPPING;
    }
}

void UIKeyBinder::MoveKeyToActionCB_Yes()
{
    UIKeyBinder *binder = sVars->activeBinder;

    if (binder->state.Set(&UIKeyBinder::State_Selected)) {
        // Store the keyMap from the other button
        int32 keyMap = UIKeyBinder::GetMappings(sVars->activeInputID, sVars->activeButtonID);

        // Remove other button's keyMap
        UIKeyBinder::SetMappings(sVars->activeInputID, sVars->activeButtonID, KEYMAP_NO_MAPPING);

        // Give the keyMap to the new button
        UIKeyBinder::SetMappings(binder->inputID + 1, binder->type, keyMap);

        UIControl *parent   = (UIControl *)binder->parent;
        parent->selectionDisabled = false;
        parent->childHasFocus     = false;

        sVars->isSelected   = false;
        binder->processButtonCB.Set(&UIButton::ProcessButtonCB_Scroll);
        binder->state.Set(&UIKeyBinder::State_HandleButtonEnter);
        sVars->activeBinder = NULL;

        Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, false);
        sVars->activeInputID  = KEYMAP_AUTO_MAPPING;
        sVars->activeButtonID = KEYMAP_AUTO_MAPPING;
    }
}

#if RETRO_INCLUDE_EDITOR
void UIKeyBinder::EditorDraw()
{
    this->inkEffect = this->disabled ? INK_BLEND : INK_NONE;
    UIKeyBinder::DrawSprites();
}

void UIKeyBinder::EditorLoad()
{
    sVars->aniFrames.Load("UI/Buttons.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Up");
    RSDK_ENUM_VAR("Down");
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Right");
    RSDK_ENUM_VAR("A");
    RSDK_ENUM_VAR("B");
    RSDK_ENUM_VAR("X");
    RSDK_ENUM_VAR("Y");
    RSDK_ENUM_VAR("Start");
}
#endif

void UIKeyBinder::Serialize()
{
    RSDK_EDITABLE_VAR(UIKeyBinder, VAR_BOOL, disabled);
    RSDK_EDITABLE_VAR(UIKeyBinder, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(UIKeyBinder, VAR_UINT8, inputID);
}

} // namespace GameLogic