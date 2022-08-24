// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: PlatformNode Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "PlatformNode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(PlatformNode);

void PlatformNode::Update() {}
void PlatformNode::LateUpdate() {}
void PlatformNode::StaticUpdate() {}
void PlatformNode::Draw() {}

void PlatformNode::Create(void *data) {}

void PlatformNode::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void PlatformNode::EditorDraw()
{
    if (showGizmos()) {
        PlatformNode *next = GameObject::Get<PlatformNode>(sceneInfo->entitySlot + 1);
        if (next && next->classID == sVars->classID) {
            RSDK_DRAWING_OVERLAY(true);
            Graphics::DrawLine(this->position.x, this->position.y, next->position.x, next->position.y, 0xFFFF00, 0xFF, INK_NONE, false);
            RSDK_DRAWING_OVERLAY(false);
        }
    }

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->animator.DrawSprite(nullptr, false);
}

void PlatformNode::EditorLoad() { sVars->aniFrames.Load("Global/PlaneSwitch.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void PlatformNode::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(PlatformNode);

    sVars->aniFrames.Init();
}
#endif

void PlatformNode::Serialize() {}

} // namespace GameLogic