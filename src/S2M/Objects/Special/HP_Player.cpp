// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Player Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Player.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Player);

void HP_Player::Update() {}
void HP_Player::LateUpdate() {}
void HP_Player::StaticUpdate() {}
void HP_Player::Draw() {}

void HP_Player::Create(void *data) {}

void HP_Player::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void HP_Player::EditorDraw() {}

void HP_Player::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_Player::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(HP_Player); }
#endif

void HP_Player::Serialize() { RSDK_EDITABLE_VAR(HP_Player, VAR_ENUM, characterID); }

} // namespace GameLogic