// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_SpecialClear Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_SpecialClear.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_SpecialClear);

void HP_SpecialClear::Update() {}
void HP_SpecialClear::LateUpdate() {}
void HP_SpecialClear::StaticUpdate() {}
void HP_SpecialClear::Draw() {}

void HP_SpecialClear::Create(void *data) {}

void HP_SpecialClear::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void HP_SpecialClear::EditorDraw() {}

void HP_SpecialClear::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_SpecialClear::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(HP_SpecialClear); }
#endif

void HP_SpecialClear::Serialize() {}

} // namespace GameLogic