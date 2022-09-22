// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Background Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Background.hpp"
#include "HP_Halfpipe.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Background);

void HP_Background::Update()
{
    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);

    this->drawPos.x = 0;
    this->drawPos.y = 0;
    switch (((halfpipe->halfpipeRotation.y + 0x40) & 0x1FF) >> 7) {
        default:
        case 0:
            this->drawPos.x = halfpipe->halfpipeRotation.y;
            this->drawPos.y = -halfpipe->halfpipeRotation.x;
            this->rotation  = halfpipe->halfpipeRotation.z;
            break;

        case 1:
            this->drawPos.x = halfpipe->halfpipeRotation.y;
            this->drawPos.y = -halfpipe->halfpipeRotation.z;
            this->rotation  = -halfpipe->halfpipeRotation.x;
            break;

        case 2:
            this->drawPos.x = halfpipe->halfpipeRotation.y;
            this->drawPos.y = halfpipe->halfpipeRotation.x;
            this->rotation  = halfpipe->halfpipeRotation.z;
            break;

        case 3:
            this->drawPos.x = halfpipe->halfpipeRotation.y;
            this->drawPos.y = halfpipe->halfpipeRotation.z;
            this->rotation  = -halfpipe->halfpipeRotation.x;
            break;
    }

    this->drawPos.x = (this->drawPos.x << 1) & 0xFF;
    this->drawPos.y = (this->drawPos.y << 1) & 0xFF;
    this->rotation &= 0x1FF;

    int32 temp2 = (halfpipe->halfpipeRotation.x + halfpipe->halfpipeRotation.y + halfpipe->halfpipeRotation.z) & 0x7F;

    if (temp2 == 0) {
        this->viewRotation = Math::Sin256(this->angle);
        this->angle        = (this->angle + 8) & 0xFF;
    }
    else {
        this->angle = 0;
        this->viewRotation >>= 1;
    }

    this->drawPos.y += (this->viewRotation * 3) >> 9;
}
void HP_Background::LateUpdate() {}
void HP_Background::StaticUpdate() {}
void HP_Background::Draw()
{
    Vector2 drawPos;
    drawPos.x = TO_FIXED(this->drawPos.x + screenInfo[sceneInfo->currentScreenID].center.x);
    drawPos.y = TO_FIXED(this->drawPos.y + screenInfo[sceneInfo->currentScreenID].center.y);

    int32 start = 0;
    int32 end   = 12;
    if (this->rotation) {
        start = 1;
        end   = 11;
    }

    for (int32 f = start; f < end; ++f) {
        this->animator.frameID = f;
        this->animator.DrawSprite(&drawPos, true);
    }
}

void HP_Background::Create(void *data)
{
    this->active  = ACTIVE_NORMAL;
    this->visible = true;

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
}

void HP_Background::StageLoad()
{
    sVars->aniFrames.Load("Special/Background.bin", SCOPE_STAGE);

    // Destroy any bg entities placed in the scene
    for (auto zone : GameObject::GetEntities<HP_Background>(FOR_ALL_ENTITIES)) zone->Destroy();
    // ... and ensure we have a bg entity in the correct reserved slot
    GameObject::Reset(SLOT_HP_BG, sVars->classID, nullptr);
}

#if RETRO_INCLUDE_EDITOR
void HP_Background::EditorDraw() {}

void HP_Background::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_Background::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(HP_Background);

    sVars->aniFrames.Init();
}
#endif

void HP_Background::Serialize() {}

} // namespace GameLogic