// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: CutsceneRules Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "CutsceneRules.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/StarPost.hpp"
#include "Global/SpecialRing.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(CutsceneRules);

void CutsceneRules::Update() {}
void CutsceneRules::LateUpdate() {}
void CutsceneRules::StaticUpdate() {}
void CutsceneRules::Draw() {}

void CutsceneRules::Create(void *data) {}

void CutsceneRules::StageLoad() {}

bool32 CutsceneRules::CheckAct1() { return stageList[Zone::sVars->folderListPos].actID == 0; }

bool32 CutsceneRules::CheckAct2() { return stageList[Zone::sVars->folderListPos].actID != 0; }

bool32 CutsceneRules::CheckOutroAct2() { return stageList[Zone::sVars->folderListPos].actID != 0; }

bool32 CutsceneRules::CheckOutroAct1() { return stageList[Zone::sVars->folderListPos].actID == 0; }

bool32 CutsceneRules::CheckStageReload()
{
    if (StarPost::sVars) {
        for (int32 p = 0; p < Player::sVars->playerCount; ++p) {
            if (StarPost::sVars->postIDs[p])
                return true;
        }
    }

    if (SpecialRing::sVars && globals->specialRingID > 0) {
        for (auto specialRing : GameObject::GetEntities<SpecialRing>(FOR_ALL_ENTITIES))
        {
            if (specialRing->id > 0 && globals->specialRingID == specialRing->id)
                return true;
        }
    }

    return false;
}

#if RETRO_INCLUDE_EDITOR
void CutsceneRules::EditorDraw() {}

void CutsceneRules::EditorLoad() {}
#endif

#if RETRO_REV0U
void CutsceneRules::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(CutsceneRules); }
#endif

void CutsceneRules::Serialize() {}

} // namespace GameLogic