// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TitleSonic Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "TitleSonic.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TitleSonic);

void TitleSonic::Update()
{
    this->animatorSonic.Process();
    this->animatorTails.Process();
}

void TitleSonic::LateUpdate() {}

void TitleSonic::StaticUpdate() {}

void TitleSonic::Draw()
{
    Graphics::SetClipBounds(0, 0, 0, screenInfo->size.x, 160);
    if (this->type == Sonic) {
        this->animatorSonic.DrawSprite(nullptr, false); // putting nullptr here just draws the sprite at the objects position i think
    }
    else {
        this->animatorTails.DrawSprite(nullptr, false);
    }

    Graphics::SetClipBounds(0, 0, 0, screenInfo->size.x, screenInfo->size.y); // does this just cut off things at the screen border so nothing draws outside of it??
}

void TitleSonic::Create(void *data)
{
    if (this->type == Sonic) { // adding types to this for both sonic and tails (also removing the finger as its not needed)
        this->animatorSonic.SetAnimation(sVars->aniFrames, 0, true, 0);
    }
    else {
        this->animatorTails.SetAnimation(sVars->aniFrames, 1, true, 0);
    }

    if (!sceneInfo->inEditor) {
        this->visible   = false;
        this->active    = ACTIVE_NEVER;
        this->drawGroup = 4;
    }
}

void TitleSonic::StageLoad() { sVars->aniFrames.Load("Title/TitleCharacters.bin", SCOPE_STAGE); }

#if RETRO_INCLUDE_EDITOR
void TitleSonic::EditorDraw()
{
    this->animatorSonic.frameID = this->animatorSonic.frameCount - 1;

    this->animatorSonic.DrawSprite(nullptr, false);
}

void TitleSonic::EditorLoad() { sVars->aniFrames.Load("Title/TitleCharacters.bin", SCOPE_STAGE); }
#endif

void TitleSonic::Serialize() { RSDK_EDITABLE_VAR(TitleSonic, VAR_INT32, type); }

} // namespace GameLogic