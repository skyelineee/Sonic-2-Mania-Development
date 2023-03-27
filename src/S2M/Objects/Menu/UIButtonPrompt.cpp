// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIButtonPrompt Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIButtonPrompt.hpp"
#include "UIControl.hpp"
#include "UIHeading.hpp"
#include "UIWidgets.hpp"
#include "Global/Localization.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIButtonPrompt);

void UIButtonPrompt::Update()
{
    bool32 textChanged = false;
    if (!this->textSprite.Matches(&UIWidgets::sVars->textFrames)) {
        this->promptAnimator.SetAnimation(&UIWidgets::sVars->textFrames, 0, true, this->promptID);
        textChanged      = true;
        this->textSprite = UIWidgets::sVars->textFrames;
    }

    if (this->scale.x == 0x200 && this->scaleMax == 0x200 && this->scaleSpeed)
        this->scaleSpeed = 0;

    this->state.Run(this);

    if (this->scale.x >= this->scaleMax) {
        if (this->scale.x > this->scaleMax) {
            this->scale.x -= this->scaleSpeed;
            if (this->scale.x < this->scaleMax)
                this->scale.x = this->scaleMax;
        }
    }
    else {
        this->scale.x += this->scaleSpeed;
        if (this->scale.x > this->scaleMax)
            this->scale.x = this->scaleMax;
    }

    this->scale.y = this->scale.x;

    if (this->prevPrompt != this->promptID) {
        this->promptAnimator.SetAnimation(&UIWidgets::sVars->textFrames, 0, true, this->promptID);
        this->prevPrompt = this->promptID;
    }

    int32 button = this->buttonID;
    if (this->prevButton != button) {
        UIButtonPrompt::SetButtonSprites();
        button           = this->buttonID;
        this->prevButton = button;
    }

    if (SKU->platform == PLATFORM_PC || SKU->platform == PLATFORM_DEV) {
        int32 mappings = UIButtonPrompt::GetButtonMappings(UIButtonPrompt::sVars->inputSlot, button);
        if (textChanged || this->mappings != mappings) {
            UIButtonPrompt::SetButtonSprites();
            this->mappings = mappings;
        }
    }
}
void UIButtonPrompt::LateUpdate()
{
    UIControl *control = (UIControl *)this->parent;
    if (control && this->headingAnchor) {

        UIHeading *heading = (UIHeading *)control->heading;
        if (heading) {
            switch (this->headingAnchor) {
                default:
                case UIBUTTONPROMPT_ANCHOR_NONE: break;

                case UIBUTTONPROMPT_ANCHOR_TOPLEFT:
                    this->position.x = heading->position.x - TO_FIXED(195);
                    this->position.y = heading->position.y - TO_FIXED(5);
                    break;

                case UIBUTTONPROMPT_ANCHOR_TOPRIGHT:
                    this->position.x = heading->position.x - TO_FIXED(90);
                    this->position.y = heading->position.y - TO_FIXED(5);
                    break;

                case UIBUTTONPROMPT_ANCHOR_BOTTOMRIGHT:
                    this->position.x = heading->position.x - TO_FIXED(195);
                    this->position.y = heading->position.y + 0x100000;
                    break;

                case UIBUTTONPROMPT_ANCHOR_BOTTOMLEFT:
                    this->position.x = heading->position.x - TO_FIXED(90);
                    this->position.y = heading->position.y + 0x100000;
                    break;
            }
        }
    }
}

void UIButtonPrompt::StaticUpdate()
{
    sVars->type = GetGamepadType();

    int32 id = Input::GetFilteredInputDeviceID(false, false, 0);

    int32 gamepadType = Input::GetInputDeviceType(id);
    int32 deviceType  = (gamepadType >> 8) & 0xFF;

    UIButtonPrompt::sVars->inputSlot = deviceType == Input::DEVICE_TYPE_KEYBOARD ? (gamepadType & 0xFF) : Input::CONT_P1;
}

void UIButtonPrompt::Draw()
{
    UIButtonPrompt::SetButtonSprites();

    this->decorAnimator.DrawSprite(NULL, false);

    this->drawFX = FX_SCALE;
    this->buttonAnimator.DrawSprite(NULL, false);

    this->drawFX = FX_NONE;
    if (this->textVisible)
        this->promptAnimator.DrawSprite(NULL, false);
}

void UIButtonPrompt::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->startPos      = this->position;
        this->visible       = true;
        this->drawGroup     = 2;
        this->scaleMax      = 0x200;
        this->scaleSpeed    = 0x10;
        this->scale.x       = 0x200;
        this->scale.y       = 0x200;
        this->disableScale  = false;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x2000000;
        this->updateRange.y = 0x800000;
        this->textVisible   = true;

        this->decorAnimator.SetAnimation(&sVars->aniFrames, 0, true, 0);
        this->promptAnimator.SetAnimation(&UIWidgets::sVars->textFrames, 0, true, this->promptID);

        UIButtonPrompt::SetButtonSprites();

        this->textSprite = UIWidgets::sVars->textFrames;
        this->state.Set(&UIButtonPrompt::State_CheckIfSelected);
        this->parent      = NULL;
        this->touchSize.x = 0x580000;
        this->touchSize.y = 0x100000;
        this->touchPos.x  = 0x200000;
    }
}

void UIButtonPrompt::StageLoad() 
{
    UIButtonPrompt::sVars->type      = UIBUTTONPROMPT_KEYBOARD;
    UIButtonPrompt::sVars->inputSlot = Input::CONT_P1;

    sVars->aniFrames.Load("UI/Buttons.bin", SCOPE_STAGE);
}

int32 UIButtonPrompt::GetButtonMappings(int32 input, int32 button)
{
    switch (button) {
        case UIBUTTONPROMPT_BUTTON_A: return controllerInfo[input].keyA.keyMap;
        case UIBUTTONPROMPT_BUTTON_B: return controllerInfo[input].keyB.keyMap;
        case UIBUTTONPROMPT_BUTTON_X: return controllerInfo[input].keyX.keyMap;
        case UIBUTTONPROMPT_BUTTON_Y: return controllerInfo[input].keyY.keyMap;
        case UIBUTTONPROMPT_BUTTON_START: return controllerInfo[input].keyStart.keyMap;
        case UIBUTTONPROMPT_BUTTON_SELECT: return controllerInfo[input].keySelect.keyMap;
        default: break;
    }

    return 0;
}

int32 UIButtonPrompt::GetGamepadType()
{
    int32 id = Input::GetFilteredInputDeviceID(false, false, 0);

    int32 gamepadType = Input::GetInputDeviceType(id);

    int32 deviceType = (gamepadType >> 8) & 0xFF;
    if (deviceType == Input::DEVICE_TYPE_KEYBOARD) {
        switch (Localization::sVars->language) {
            case LANGUAGE_FR: return UIBUTTONPROMPT_KEYBOARD_FR;
            case LANGUAGE_IT: return UIBUTTONPROMPT_KEYBOARD_IT;
            case LANGUAGE_GE: return UIBUTTONPROMPT_KEYBOARD_GE;
            case LANGUAGE_SP: return UIBUTTONPROMPT_KEYBOARD_SP;
            default: return UIBUTTONPROMPT_KEYBOARD;
        }
    }
    else if (deviceType == Input::DEVICE_TYPE_CONTROLLER) {
        // I don't actually think saturn type is ever set in-engine, neat that it exists though

        switch (gamepadType & 0xFF) {
            case Input::DEVICE_XBOX: return UIBUTTONPROMPT_XBOX;
            case Input::DEVICE_PS4: return UIBUTTONPROMPT_PS4;
            case Input::DEVICE_SATURN: return UIBUTTONPROMPT_SATURN_WHITE;
            case Input::DEVICE_SWITCH_HANDHELD:
            case Input::DEVICE_SWITCH_JOY_GRIP:
            case Input::DEVICE_SWITCH_PRO: return UIBUTTONPROMPT_SWITCH;
            case Input::DEVICE_SWITCH_JOY_L: return UIBUTTONPROMPT_JOYCON_L;
            case Input::DEVICE_SWITCH_JOY_R: return UIBUTTONPROMPT_JOYCON_R;
            default: break;
        }
    }
    return UIBUTTONPROMPT_XBOX;
}

uint8 UIButtonPrompt::MappingsToFrame(int32 mappings)
{
    // case values: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    switch (mappings) {
        case 8: return 14;
        case 9: return 15;
        case 13: return 41;
        case 16:
        case 160: return 42;
        case 17:
        case 162: return 54;
        case 18:
        case 164: return 55;
        case 20: return 29;
        case 32: return 56;
        case 33: return 91;
        case 34: return 92;
        case 35: return 93;
        case 36: return 94;
        case 37: return 61;
        case 38: return 59;
        case 39: return 62;
        case 40: return 60;
        case 45: return 95;
        case 46: return 96;
        case 48: return 11;
        case 49: return 2;
        case 50: return 3;
        case 51: return 4;
        case 52: return 5;
        case 53: return 6;
        case 54: return 7;
        case 55: return 8;
        case 56: return 9;
        case 57: return 10;
        case 65: return 30;
        case 66: return 47;
        case 67: return 45;
        case 68: return 32;
        case 69: return 18;
        case 70: return 33;
        case 71: return 34;
        case 72: return 35;
        case 73: return 23;
        case 74: return 36;
        case 75: return 37;
        case 76: return 38;
        case 77: return 49;
        case 78: return 48;
        case 79: return 24;
        case 80: return 25;
        case 81: return 16;
        case 82: return 19;
        case 83: return 31;
        case 84: return 20;
        case 85: return 22;
        case 86: return 46;
        case 87: return 17;
        case 88: return 44;
        case 89: return 21;
        case 90: return 43;
        case 96: return 77;
        case 97: return 73;
        case 98: return 74;
        case 99: return 75;
        case 100: return 70;
        case 101: return 71;
        case 102: return 72;
        case 103: return 66;
        case 104: return 67;
        case 105: return 68;
        case 106: return 64;
        case 107: return 69;
        case 109: return 65;
        case 110: return 78;
        case 111: return 63;
        case 161: return 53;
        case 163: return 58;
        case 165: return 57;
        case 186: return 79;
        case 187: return 87;
        case 188: return 88;
        case 189: return 89;
        case 190: return 90;
        case 191: return 80;
        case 192: return 81;
        case 219: return 82;
        case 220: return 83;
        case 221: return 84;
        case 222: return 85;
        case 223: return 86;
        default: break;
    }
    return 0;
}

void UIButtonPrompt::SetButtonSprites()
{
    if (sceneInfo->inEditor) {
        this->buttonAnimator.SetAnimation(&sVars->aniFrames, UIBUTTONPROMPT_XBOX, true, this->buttonID);
    }
    else {
        int32 buttonID = this->buttonID;
        if (API::GetConfirmButtonFlip() && buttonID <= 1)
            buttonID ^= 1;

        if (UIButtonPrompt::sVars->type != UIBUTTONPROMPT_KEYBOARD
            && (UIButtonPrompt::sVars->type < UIBUTTONPROMPT_KEYBOARD_FR || UIButtonPrompt::sVars->type > UIBUTTONPROMPT_KEYBOARD_SP)) {
            this->buttonAnimator.SetAnimation(&sVars->aniFrames, UIButtonPrompt::sVars->type, true, buttonID);
        }
        else {
            // despite different languages existing here, the english one is always the one used
            int32 mappings = UIButtonPrompt::GetButtonMappings(UIButtonPrompt::sVars->inputSlot, buttonID);
            int32 frame    = UIButtonPrompt::MappingsToFrame(mappings);
            this->buttonAnimator.SetAnimation(&sVars->aniFrames, UIBUTTONPROMPT_KEYBOARD, true, frame);
        }
    }
}

bool32 UIButtonPrompt::CheckTouch()
{
    UIControl *control = (UIControl *)this->parent;
    if (control && !control->dialogHasFocus && !control->selectionDisabled) {
        if (touchInfo->count) {
            int32 screenX = (screenInfo->position.x << 16);
            int32 screenY = (screenInfo->position.y << 16);
            int32 sizeX   = this->touchSize.x >> 1;
            int32 sizeY   = this->touchSize.y >> 1;

            bool32 wasTouched = false;
            for (int32 i = 0; i < touchInfo->count; ++i) {
                int32 x = screenX - (int32)((touchInfo->x[i] * screenInfo->size.x) * -65536.0f);
                int32 y = screenY - (int32)((touchInfo->y[i] * screenInfo->size.y) * -65536.0f);

                int32 touchX = abs(this->touchPos.x + this->position.x - x);
                int32 touchY = abs(this->touchPos.y + this->position.y - y);
                if (touchX < sizeX && touchY < sizeY) {
                    wasTouched = true;
                }
            }

            this->touched = wasTouched;
            return true;
        }
        else {
            if (this->touched) {
                this->timer = 0;
                this->state.Set(&UIButtonPrompt::State_Selected);
            }
            this->touched = false;
        }
    }

    return false;
}

void UIButtonPrompt::State_CheckIfSelected()
{
    if (this->visible) {
        if (UIButtonPrompt::CheckTouch()) {
            this->scaleMax = 0x280;

            if (this->scaleSpeed < 0x10)
                this->scaleSpeed = 0x10;
        }
        else if (!this->disableScale) {
            this->scaleMax = 0x200;
        }
    }
}

void UIButtonPrompt::State_Selected()
{
    this->scaleMax = 0x280;

    if (++this->timer == 16) {
        this->timer       = 0;
        this->textVisible = true;
        this->state.Set(&UIButtonPrompt::State_CheckIfSelected);

        int32 buttonID = this->buttonID;
        if (API::GetConfirmButtonFlip() && buttonID <= 1)
            buttonID ^= 1;
        
        UIControl::ClearInputs(buttonID);
    }

    this->textVisible = !((this->timer >> 1) & 1);
}

#if RETRO_INCLUDE_EDITOR
void UIButtonPrompt::EditorDraw()
{
    this->startPos      = this->position;
    this->drawGroup     = 2;
    this->disableScale  = false;
    this->updateRange.x = 0x2000000;
    this->updateRange.y = 0x800000;
    this->textVisible   = true;
    this->decorAnimator.SetAnimation(&sVars->aniFrames, 0, true, 0);
    this->promptAnimator.SetAnimation(&UIWidgets::sVars->textFrames, 0, true, this->promptID);

    UIButtonPrompt::SetButtonSprites();
    this->textSprite = UIWidgets::sVars->textFrames;

    UIButtonPrompt::Draw();
}

void UIButtonPrompt::EditorLoad(void)
{
    sVars->aniFrames.Load("UI/Buttons.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, buttonID);
    RSDK_ENUM_VAR("A");
    RSDK_ENUM_VAR("B");
    RSDK_ENUM_VAR("X");
    RSDK_ENUM_VAR("Y");
    RSDK_ENUM_VAR("Start");
    RSDK_ENUM_VAR("Select");

    RSDK_ACTIVE_VAR(sVars, headingAnchor);
    RSDK_ENUM_VAR("None");
    RSDK_ENUM_VAR("Top-Left");
    RSDK_ENUM_VAR("Top-Right");
    RSDK_ENUM_VAR("Bottom-Right");
    RSDK_ENUM_VAR("Bottom-Left");
}
#endif

void UIButtonPrompt::Serialize()
{
    RSDK_EDITABLE_VAR(UIButtonPrompt, VAR_ENUM, promptID);
    RSDK_EDITABLE_VAR(UIButtonPrompt, VAR_ENUM, buttonID);
    RSDK_EDITABLE_VAR(UIButtonPrompt, VAR_UINT8, headingAnchor);
}

} // namespace GameLogic