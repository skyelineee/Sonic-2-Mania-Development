// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Ring Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Ring.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Ring);

void HP_Ring::Update() {}
void HP_Ring::LateUpdate() {}
void HP_Ring::StaticUpdate() {}
void HP_Ring::Draw() {}

void HP_Ring::Create(void *data) {}

void HP_Ring::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void HP_Ring::EditorDraw() {}

void HP_Ring::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_Ring::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(HP_Ring); }
#endif

void HP_Ring::Serialize() {}

} // namespace GameLogic