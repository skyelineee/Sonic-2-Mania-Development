// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Bomb Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Bomb.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Bomb);

void HP_Bomb::Update() {}
void HP_Bomb::LateUpdate() {}
void HP_Bomb::StaticUpdate() {}
void HP_Bomb::Draw() {}

void HP_Bomb::Create(void *data) {}

void HP_Bomb::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void HP_Bomb::EditorDraw() {}

void HP_Bomb::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_Bomb::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(HP_Bomb); }
#endif

void HP_Bomb::Serialize() {}

} // namespace GameLogic