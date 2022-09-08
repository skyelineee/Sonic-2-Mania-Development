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
}

void EHZSetup::StageFinish_EndAct1()
{
    Zone::StoreEntities(Vector2(TO_FIXED(15792), TO_FIXED(1588)));
    Stage::LoadScene();
}

void EHZSetup::HandleActTransition()
{ 
    Zone::sVars->cameraBoundsL[0] = 256 - screenInfo->center.x;
    Zone::sVars->cameraBoundsB[0] = 1412;

    this->storePos.x = TO_FIXED(256);
    this->storePos.y = TO_FIXED(1412);

    Zone::ReloadEntities(storePos, true);
}

#if RETRO_INCLUDE_EDITOR
void EHZSetup::EditorDraw() {}

void EHZSetup::EditorLoad() {}
#endif

void EHZSetup::Serialize() {}
} // namespace GameLogic
