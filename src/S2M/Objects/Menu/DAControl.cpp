// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: DAControl Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "DAControl.hpp"
#include "UIButton.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(DAControl);

void DAControl::Update() { this->state.Run(this); }

void DAControl::LateUpdate() {}

void DAControl::StaticUpdate() {}

void DAControl::Draw()
{
    Vector2 drawPos;

    this->direction = FLIP_NONE;
    for (int32 i = 0; i < 2; ++i) {
        drawPos                         = this->position;
        this->backPlateAnimator.frameID = 0;
        this->backPlateAnimator.DrawSprite(&drawPos, false);

        this->backPlateAnimator.frameID = 1;
        this->backPlateAnimator.DrawSprite(&drawPos, false);

        drawPos.y += 0x80000;
        this->backPlateAnimator.DrawSprite(&drawPos, false);

        drawPos.y += 0x80000;
        this->backPlateAnimator.DrawSprite(&drawPos, false);

        drawPos.y += 0x80000;
        this->backPlateAnimator.frameID = 2;
        this->backPlateAnimator.DrawSprite(&drawPos, false);

        drawPos.y -= 0x170000;
        this->backPlateAnimator.frameID = 3;
        this->backPlateAnimator.DrawSprite(&drawPos, false);
        ++this->direction;
    }

    this->direction = FLIP_NONE;
    drawPos.y -= 0x130000;
    this->backPlateAnimator.frameID = 4;
    this->backPlateAnimator.DrawSprite(&drawPos, false);

    drawPos.x = this->position.x - 0x700000;
    drawPos.y = this->position.y + 0x1E0000;
    for (int32 i = 0; i < 5; ++i) {
        this->buttonAnimator.DrawSprite(&drawPos, false);
        this->optionsAnimator[i].DrawSprite(&drawPos, false);
        drawPos.x += 0x380000;
    }

    drawPos.x = this->position.x - 0x800000;
    drawPos.y = this->position.y;
    if (!sceneInfo->inEditor)
        this->textAnimator.DrawString(&drawPos, &this->text, 0, this->text.length, UIButton::ALIGN_LEFT, 0, 0, false);
}

void DAControl::Create(void *data)
{
    this->drawFX = FX_FLIP;
    if (!sceneInfo->inEditor) {
        this->visible       = true;
        this->drawGroup     = 2;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->backPlateAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->buttonAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->textAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
        for (int32 i = 0; i < 5; ++i) this->optionsAnimator[i].SetAnimation(sVars->aniFrames, 2,true, i);

        this->text.Init("SELECT A TRACK...", 0);
        this->textAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
        this->text.SetSpriteString(sVars->aniFrames, 4);
    }
}

void DAControl::StageLoad() { sVars->aniFrames.Load("UI/DAGarden.bin", SCOPE_STAGE); }

#if RETRO_INCLUDE_EDITOR
void DAControl::EditorDraw()
{
    this->visible       = true;
    this->drawGroup     = 2;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->backPlateAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->buttonAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    this->textAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
    for (int32 i = 0; i < 5; ++i) this->optionsAnimator[i].SetAnimation(sVars->aniFrames, 2, true, i);

    DAControl::Draw();
}

void DAControl::EditorLoad(void) { sVars->aniFrames.Load("UI/DAGarden.bin", SCOPE_STAGE); }
#endif

void DAControl::Serialize(void) {}

} // namespace GameLogic