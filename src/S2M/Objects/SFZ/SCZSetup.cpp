// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SCZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SCZSetup.hpp"
#include "Global/Zone.hpp"
#include "Global/Animals.hpp"
#include "Helpers/RPCHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SCZSetup);

void SCZSetup::Update() {}
void SCZSetup::LateUpdate() {}
void SCZSetup::StaticUpdate() {}
void SCZSetup::Draw() {}

void SCZSetup::Create(void *data) {}

void SCZSetup::StageLoad()
{
    Animals::sVars->animalTypes[0] = Animals::Flicky;
    Animals::sVars->animalTypes[1] = Animals::Becky;

    if (globals->gameMode != MODE_TIMEATTACK) {
        const char *playingAsText  = "";
        const char *characterImage = "";
        const char *characterText  = "";
        switch (GET_CHARACTER_ID(1)) {
            case ID_SONIC:
                playingAsText  = "Playing as Sonic";
                characterImage = "sonic";
                characterText  = "Sonic";
                break;
            case ID_TAILS:
                playingAsText  = "Playing as Tails";
                characterImage = "tails";
                characterText  = "Tails";
                break;
            case ID_KNUCKLES:
                playingAsText  = "Playing as Knuckles";
                characterImage = "knuckles";
                characterText  = "Knuckles";
                break;
        }
        if (Stage::CheckSceneFolder("SCZ")) {
            SetPresence(playingAsText, "In Sky Chase", "doggy", "doggy", characterImage, characterText);
        }
        else if (Stage::CheckSceneFolder("WFZ")) {
            SetPresence(playingAsText, "In Wing Fortress", "doggy", "doggy", characterImage, characterText);
        }
    }
}

void SCZSetup::PlayerState_Pilot()
{
    Player *player = (Player *)this;
    //RSDK_THIS(Player);

    player->position.x     = 0;
    player->position.y     = 0;
    player->active         = ACTIVE_NEVER;
    player->visible        = false;
    player->stateInput.Set(nullptr);
    player->position.x     = screenInfo->position.x << 16;
    player->position.y     = screenInfo->position.y << 16;
    Player::sVars->respawnTimer = 0;
}

#if RETRO_INCLUDE_EDITOR
void SCZSetup::EditorDraw() {}

void SCZSetup::EditorLoad() {}
#endif

#if RETRO_REV0U
void SCZSetup::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(SCZSetup); }
#endif

void SCZSetup::Serialize() {}

} // namespace GameLogic