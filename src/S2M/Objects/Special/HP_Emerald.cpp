// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Emerald Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Emerald.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Emerald);

void HP_Emerald::Update() {}
void HP_Emerald::LateUpdate() {}
void HP_Emerald::StaticUpdate() {}
void HP_Emerald::Draw() {}

void HP_Emerald::Create(void *data) {}

void HP_Emerald::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void HP_Emerald::EditorDraw() {}

void HP_Emerald::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_Emerald::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(HP_Emerald); }
#endif

void HP_Emerald::Serialize() {}

} // namespace GameLogic