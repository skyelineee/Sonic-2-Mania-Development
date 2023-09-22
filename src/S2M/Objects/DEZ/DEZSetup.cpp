// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: DEZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "DEZSetup.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/Animals.hpp"
#include "Helpers/RPCHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(DEZSetup);

void DEZSetup::Update() {}
void DEZSetup::LateUpdate() {}
void DEZSetup::StaticUpdate()
{
    // checks for and destroys tails p2 since hes not supposed to be in this zone
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->sidekick) {
            player->Destroy();
        }
    }
}

void DEZSetup::Draw() {}

void DEZSetup::Create(void *data) {}

void DEZSetup::StageLoad()
{
    Animals::sVars->animalTypes[0] = Animals::Picky;
    Animals::sVars->animalTypes[1] = Animals::Cucky;

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
        SetPresence(playingAsText, "In Death Egg", "doggy", "doggy", characterImage, characterText);
    }
}

#if RETRO_INCLUDE_EDITOR
void DEZSetup::EditorDraw() {}

void DEZSetup::EditorLoad() {}
#endif

#if RETRO_REV0U
void DEZSetup::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(DEZSetup); }
#endif

void DEZSetup::Serialize() {}

} // namespace GameLogic