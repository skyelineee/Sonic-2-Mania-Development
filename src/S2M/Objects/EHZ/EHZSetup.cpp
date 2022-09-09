// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: EHZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "EHZSetup.hpp"
#include "Helpers/CutsceneRules.hpp"
#include "Global/Zone.hpp"
#include "Global/Animals.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(EHZSetup);

void EHZSetup::Update() {}

void EHZSetup::LateUpdate() {}

void EHZSetup::StaticUpdate()
{
    if (!(Zone::sVars->timer & 7)) {
        ++sVars->background->deformationOffset;
    }
    sVars->paletteTimer += 42;
    if (sVars->paletteTimer >= 256) {
        sVars->paletteTimer -= 256;
        paletteBank[0].Rotate(170, 173, true);
    }
}
void EHZSetup::Draw() {}

void EHZSetup::Create(void *data) {}

void EHZSetup::StageLoad()
{
    Animals::sVars->animalTypes[0] = Animals::Flicky;
    Animals::sVars->animalTypes[1] = Animals::Pocky;

    if (Zone::sVars->actID) {

        if (globals->atlEnabled) {
            if (!CutsceneRules::CheckStageReload())
                &EHZSetup::HandleActTransition;
        }

        if (CutsceneRules::CheckAct1()) {
            Zone::sVars->stageFinishCallback.Set(&EHZSetup::StageFinish_EndAct1);
        }
    }
    sVars->background = SceneLayer::GetTileLayer(0);
    for (int32 i = 0; i < 1024; ++i) {
        sVars->background->deformationData[i] = sVars->deformation[i & 63];
    }
}

void EHZSetup::StageFinish_EndAct1()
{
    Zone::StoreEntities(Vector2(TO_FIXED(10880), TO_FIXED(822)));
    Stage::LoadScene();
}

void EHZSetup::HandleActTransition()
{ 
    Vector2 storePos;

    Zone::sVars->cameraBoundsL[0] = 256 - screenInfo->center.x;
    Zone::sVars->cameraBoundsB[0] = 1412;

    storePos.x = TO_FIXED(256);
    storePos.y = TO_FIXED(690);

    Zone::ReloadEntities(storePos, true);
}

void EHZSetup::StaticLoad(Static* sVars)
{
    RSDK_INIT_STATIC_VARS(EHZSetup);

    int32 deformation[] = { 1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3,
                            1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3 };

    memcpy(sVars->deformation, deformation, sizeof(deformation));

}

#if RETRO_INCLUDE_EDITOR
void EHZSetup::EditorDraw() {}

void EHZSetup::EditorLoad() {}
#endif

void EHZSetup::Serialize() {}
} // namespace GameLogic
