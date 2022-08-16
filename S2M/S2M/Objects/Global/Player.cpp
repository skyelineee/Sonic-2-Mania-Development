// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Player Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Player.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Player);

void Player::Update() {}
void Player::LateUpdate() {}
void Player::StaticUpdate() {}
void Player::Draw() {}

void Player::Create(void *data) {}

void Player::StageLoad()
{
    Dev::Print(Dev::PRINT_NORMAL, "Player");

    if (!globals->playerID)
        globals->playerID = ID_DEFAULT_PLAYER;

    sceneInfo->debugMode = globals->medalMods & MEDAL_DEBUGMODE;
    Dev::AddViewableVariable("Debug Mode", &sceneInfo->debugMode, Dev::VIEWVAR_BOOL, false, true);

    // TEMP
    sceneInfo->debugMode = true;
}

#if RETRO_INCLUDE_EDITOR
void Player::EditorDraw() {}

void Player::EditorLoad() {}
#endif

#if RETRO_REV0U
void Player::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(Player); }
#endif

void Player::Serialize() {}

} // namespace GameLogic