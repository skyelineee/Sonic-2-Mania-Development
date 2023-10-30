// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SWZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "SWZSetup.hpp"
#include "Helpers/CutsceneRules.hpp"
#include "Helpers/RPCHelpers.hpp"
#include "Helpers/FXFade.hpp"
#include "Global/Zone.hpp"
#include "Global/Animals.hpp"
#include "Global/EggPrison.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SWZSetup);

void SWZSetup::Update() {}
void SWZSetup::LateUpdate() {}

void SWZSetup::StaticUpdate() 
{
    sVars->paletteTimer++;
    if (sVars->paletteTimer == 7) {
        sVars->paletteTimer = 0;
        paletteBank[0].Rotate(176, 179, true);
    }
}

void SWZSetup::Draw() {}

void SWZSetup::Create(void *data) {}

void SWZSetup::StageLoad()
{
    Animals::sVars->animalTypes[0] = Animals::Pocky;
    Animals::sVars->animalTypes[1] = Animals::Becky;

    if (globals->gameMode != MODE_TIMEATTACK) {
        if (Zone::sVars->actID) {
            if (globals->atlEnabled) {
                if (!CutsceneRules::CheckStageReload())
                    SWZSetup::HandleActTransition();
            }

            if (CutsceneRules::CheckAct2()) {
                //Zone::sVars->stageFinishCallback.Set(&SWZSetup::StageFinish_EndAct2);
            }
        }
        else {
            if (CutsceneRules::CheckAct1()) {
                Zone::sVars->shouldRecoverPlayers = true;
                Zone::sVars->stageFinishCallback.Set(&SWZSetup::StageFinish_EndAct1);
            }
        }
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
        SetPresence(playingAsText, "In Secret Woods", "doggy", "doggy", characterImage, characterText);
    }
}

void SWZSetup::StageFinish_EndAct1()
{
    Zone::StoreEntities(Vector2(TO_FIXED(Zone::sVars->cameraBoundsL[0] + screenInfo->center.x), TO_FIXED(Zone::sVars->cameraBoundsB[0])));
    Stage::LoadScene();
}

void SWZSetup::HandleActTransition()
{
    Zone::sVars->cameraBoundsL[0] = 256 - screenInfo->center.x;
    Zone::sVars->cameraBoundsB[0] = 816;

    Zone::ReloadEntities(Vector2(TO_FIXED(256), TO_FIXED(816)), true);

    TileLayer *bg1 = SceneLayer::GetTileLayer(0);
    bg1->scrollPos * bg1->parallaxFactor;
    for (int32 s = 0; s < bg1->scrollInfoCount; ++s) {
        bg1->scrollInfo[s].scrollPos += 0x16000 * bg1->scrollInfo[s].parallaxFactor;
    }

    TileLayer *bg2 = SceneLayer::GetTileLayer(1);
    bg2->scrollPos * bg2->parallaxFactor;
    for (int32 s = 0; s < bg2->scrollInfoCount; ++s) {
        bg2->scrollInfo[s].scrollPos += 0xC3500 * bg2->scrollInfo[s].parallaxFactor;
    }

    TileLayer *bg3 = SceneLayer::GetTileLayer(2);
    bg3->scrollPos * bg3->parallaxFactor;
    for (int32 s = 0; s < bg3->scrollInfoCount; ++s) {
        bg3->scrollInfo[s].scrollPos += 0x16000 * bg3->scrollInfo[s].parallaxFactor;
    }
}

void SWZSetup::StageFinish_EndAct2() {}

#if RETRO_REV0U
void SWZSetup::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(SWZSetup); }
#endif

#if RETRO_INCLUDE_EDITOR
void SWZSetup::EditorDraw() {}

void SWZSetup::EditorLoad() {}
#endif

void SWZSetup::Serialize() {}
} // namespace GameLogic
