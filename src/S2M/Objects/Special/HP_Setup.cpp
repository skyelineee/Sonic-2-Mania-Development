// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Setup Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Setup.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Setup);

void HP_Setup::Update() {}
void HP_Setup::LateUpdate() {}
void HP_Setup::StaticUpdate() {}
void HP_Setup::Draw() {}

void HP_Setup::Create(void *data) {}

void HP_Setup::StageLoad()
{
    if (!globals->playerID)
        globals->playerID = ID_DEFAULT_PLAYER;

    int32 start = 0;
    if (globals->gameMode == MODE_COMPETITION) {
        start = Math::Rand(0, 12) << 3;
    }
    else {
        // start = stage.actNum << 3; // TODO: get actID
        start = Math::Rand(0, 12) << 3;
    }

    paletteBank[0].Copy(1, start, 192, 8);

    sVars->stageColor1 = paletteBank[0].GetEntry(197) | 0xFF000000;
    sVars->stageColor2 = paletteBank[0].GetEntry(192) | 0xFF000000;
}

#if RETRO_INCLUDE_EDITOR
void HP_Setup::EditorDraw() {}

void HP_Setup::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_Setup::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(HP_Setup); }
#endif

void HP_Setup::Serialize() {}

} // namespace GameLogic