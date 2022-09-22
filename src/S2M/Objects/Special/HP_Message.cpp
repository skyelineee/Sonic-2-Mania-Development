// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Message Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Message.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Message);

void HP_Message::Update() {}
void HP_Message::LateUpdate() {}
void HP_Message::StaticUpdate() {}
void HP_Message::Draw() {}

void HP_Message::Create(void *data) {}

void HP_Message::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void HP_Message::EditorDraw() {}

void HP_Message::EditorLoad()
{

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Generic");
    RSDK_ENUM_VAR("Start Message");
    RSDK_ENUM_VAR("Ring Reminder");
}
#endif

#if RETRO_REV0U
void HP_Message::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(HP_Message); }
#endif

void HP_Message::Serialize()
{
    RSDK_EDITABLE_VAR(HP_Message, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(HP_Message, VAR_INT32, ringCountSonic);
    RSDK_EDITABLE_VAR(HP_Message, VAR_INT32, ringCountKnux);
    RSDK_EDITABLE_VAR(HP_Message, VAR_INT32, ringCountVS);
}

} // namespace GameLogic