// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SSZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SSZSetup.hpp"
#include "Global/Zone.hpp"
#include "Global/Animals.hpp"
#include "Helpers/RPCHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SSZSetup);

void SSZSetup::Update() {}
void SSZSetup::LateUpdate() {}
void SSZSetup::StaticUpdate() {}
void SSZSetup::Draw() {}

void SSZSetup::Create(void *data) {}

void SSZSetup::StageLoad()
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
        SetPresence(playingAsText, "In Sand Shower", "doggy", "doggy", characterImage, characterText);
    }
}

#if RETRO_INCLUDE_EDITOR
void SSZSetup::EditorDraw() {}

void SSZSetup::EditorLoad() {}
#endif

#if RETRO_REV0U
void SSZSetup::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(SSZSetup); }
#endif

void SSZSetup::Serialize() {}

} // namespace GameLogic