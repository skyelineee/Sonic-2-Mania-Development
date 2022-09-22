// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Checkpoint Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Checkpoint.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Checkpoint);

void HP_Checkpoint::Update() {}
void HP_Checkpoint::LateUpdate() {}
void HP_Checkpoint::StaticUpdate() {}
void HP_Checkpoint::Draw() {}

void HP_Checkpoint::Create(void *data) {}

void HP_Checkpoint::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void HP_Checkpoint::EditorDraw() {}

void HP_Checkpoint::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_Checkpoint::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(HP_Checkpoint); }
#endif

void HP_Checkpoint::Serialize()
{
    RSDK_EDITABLE_VAR(HP_Checkpoint, VAR_INT32, ringCountSonic);
    RSDK_EDITABLE_VAR(HP_Checkpoint, VAR_INT32, ringCountKnux);
    RSDK_EDITABLE_VAR(HP_Checkpoint, VAR_INT32, ringCountVS);
}

} // namespace GameLogic