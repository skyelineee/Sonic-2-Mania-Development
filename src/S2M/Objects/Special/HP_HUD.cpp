// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_HUD Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_HUD.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_HUD);

void HP_HUD::Update() {}
void HP_HUD::LateUpdate() {}
void HP_HUD::StaticUpdate() {}
void HP_HUD::Draw() {}

void HP_HUD::Create(void *data) {}

void HP_HUD::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void HP_HUD::EditorDraw() {}

void HP_HUD::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_HUD::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(HP_HUD); }
#endif

void HP_HUD::Serialize() {}

} // namespace GameLogic