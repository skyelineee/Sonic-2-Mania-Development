// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIText Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIText.hpp"
#include "UIWidgets.hpp"
#include "UIButton.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIText);

void UIText::Update()
{
    if (this->animator.animationID != (int32)this->highlighted + this->listID) {
        if (!UIWidgets::sVars || !sVars->aniFrames.Matches(nullptr))
            this->animator.SetAnimation(sVars->aniFrames, this->highlighted + this->listID, true, 0);
        else
            this->animator.SetAnimation(UIWidgets::sVars->uiFrames, this->highlighted + this->listID, true, 0);
    }
}

void UIText::LateUpdate() {}

void UIText::StaticUpdate() {}

void UIText::Draw()
{
    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y;

    SpriteAnimation frames = (!UIWidgets::sVars || !sVars->aniFrames.Matches(nullptr)) ? sVars->aniFrames : UIWidgets::sVars->uiFrames;

    int32 width = this->text.GetWidth(frames, this->listID, 0, this->text.length, 0);
    switch (this->align) {
        default:
        case UITEXT_ALIGN_LEFT: break;

        case UITEXT_ALIGN_CENTER: drawPos.x -= width << 15; break;

        case UITEXT_ALIGN_RIGHT: drawPos.x -= width << 16; break;
    }

    this->animator.DrawString(&drawPos, &this->text, 0, 0, UIButton::ALIGN_LEFT, 0, nullptr, false);
}

void UIText::Create(void *data)
{
    if (!UIWidgets::sVars || !sVars->aniFrames.Matches(nullptr))
        this->animator.SetAnimation(sVars->aniFrames, this->listID, true, 0);
    else
        this->animator.SetAnimation(UIWidgets::sVars->uiFrames, this->listID, true, 0);

    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawGroup = 2;

        if (!this->text.chars)
            this->text.Init("UNTITLED", 0);

        if (!UIWidgets::sVars || !sVars->aniFrames.Matches(nullptr))
            this->text.SetSpriteString(sVars->aniFrames, this->listID);
        else
            this->text.SetSpriteString(UIWidgets::sVars->uiFrames, this->listID);
    }
}

void UIText::StageLoad()
{
    if (Stage::CheckSceneFolder("Credits")) {
        sVars->aniFrames.Load("LSelect/Text.bin", SCOPE_STAGE);
    }
}

#if RETRO_INCLUDE_EDITOR
void UIText::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, this->listID, true, 0);
    this->animator.DrawSprite(&this->position, false);
}

void UIText::EditorLoad()
{
    if (sVars->aniFrames.Matches(nullptr))
        sVars->aniFrames.Load("UI/TextEN.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, align);
    RSDK_ENUM_VAR("Left", UITEXT_ALIGN_LEFT);
    RSDK_ENUM_VAR("Center", UITEXT_ALIGN_CENTER);
    RSDK_ENUM_VAR("Right", UITEXT_ALIGN_RIGHT);
}
#endif

void UIText::Serialize()
{
    RSDK_EDITABLE_VAR(UIText, VAR_STRING, text);
    RSDK_EDITABLE_VAR(UIText, VAR_STRING, tag);
    RSDK_EDITABLE_VAR(UIText, VAR_BOOL, selectable);
    RSDK_EDITABLE_VAR(UIText, VAR_ENUM, listID);
    RSDK_EDITABLE_VAR(UIText, VAR_ENUM, align);
    RSDK_EDITABLE_VAR(UIText, VAR_ENUM, data0);
    RSDK_EDITABLE_VAR(UIText, VAR_ENUM, data1);
    RSDK_EDITABLE_VAR(UIText, VAR_ENUM, data2);
    RSDK_EDITABLE_VAR(UIText, VAR_ENUM, data3);
    RSDK_EDITABLE_VAR(UIText, VAR_BOOL, highlighted);
}
} // namespace GameLogic