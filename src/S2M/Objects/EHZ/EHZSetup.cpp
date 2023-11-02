// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: EHZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "EHZSetup.hpp"
#include "Helpers/CutsceneRules.hpp"
#include "Helpers/RPCHelpers.hpp"
#include "Helpers/FXFade.hpp"
#include "Global/Zone.hpp"
#include "Global/Animals.hpp"
#include "Global/EggPrison.hpp"
#include "Global/HUD.hpp"
#include "SWZ/Snowflakes.hpp"

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

    if (sVars->cutsceneActivated) {
        for (int32 p = 0; p < Player::sVars->playerCount; ++p) {
            Player *player                     = GameObject::Get<Player>(p);
            Zone::sVars->playerBoundActiveR[p] = false;
            for (auto prison : GameObject::GetEntities<EggPrison>(FOR_ALL_ENTITIES)) {
                prison->notSolid = true;
            }
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                player->right = true;
            }
        }
        if (++sVars->cutsceneTimer >= 180) {
            FXFade *fade    = GameObject::Create<FXFade>(0x000000, 0, 0);
            fade->drawGroup = Zone::sVars->hudDrawGroup + 1;
            fade->active    = ACTIVE_ALWAYS;
            fade->state.Set(&FXFade::State_FadeOut);
            fade->speedIn = 1;
            if (sVars->cutsceneTimer >= 260) {
                Stage::LoadScene();
            }
        }
    }
}
void EHZSetup::Draw() {}

void EHZSetup::Create(void *data) {}

void EHZSetup::StageLoad()
{
    Animals::sVars->animalTypes[0] = Animals::Flicky;
    Animals::sVars->animalTypes[1] = Animals::Ricky;

    sVars->cutsceneActivated = false;

    if (globals->gameMode != MODE_TIMEATTACK) {
        if (Zone::sVars->actID) {

            if (globals->atlEnabled) {
                if (!CutsceneRules::CheckStageReload())
                    EHZSetup::HandleActTransition();
            }

            if (CutsceneRules::CheckAct2()) {
                Zone::sVars->stageFinishCallback.Set(&EHZSetup::StageFinish_EndAct2);
            }
        }
        else {
            if (CutsceneRules::CheckAct1()) {
                Zone::sVars->shouldRecoverPlayers = true;
                Zone::sVars->stageFinishCallback.Set(&EHZSetup::StageFinish_EndAct1);
            }
        }
        // only does this set of rich presence if not in time attack, there is a different message for that
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
        SetPresence(playingAsText, "In Emerald Hill", "doggy", "doggy", characterImage, characterText);
    }

    sVars->background = SceneLayer::GetTileLayer(0);
    for (int32 i = 0; i < 1024; ++i) {
        sVars->background->deformationData[i] = sVars->deformation[i & 63];
    }
}

void EHZSetup::StageFinish_EndAct1()
{
    sVars->timerStorage = Zone::sVars->timer;
    Vector2 offset(TO_FIXED(Zone::sVars->cameraBoundsL[0] + screenInfo->center.x), TO_FIXED(Zone::sVars->cameraBoundsB[0]));
    Zone::StoreEntities(offset);

    if (Stage::CheckSceneFolder("HEHZ")) {
        // copy first Snowflakes (should be the only Snowflakes)
        Snowflakes *snowflake = *GameObject::GetEntities<Snowflakes>(RSDK::FOR_ACTIVE_ENTITIES).begin();
        sVars->snowflakeCount = Snowflakes::sVars->count;
        for (int i = 0; i < 0x40; ++i) {
            if (snowflake->positions[i].x || snowflake->positions[i].y) {
                snowflake->positions[i].y -= offset.y;
            }
        }

        sVars->snowflakeBasis = screenInfo->position.x;

        GameObject::Copy(sVars->snowflakeStorage, snowflake, false);
    }

    Stage::LoadScene();
}
void EHZSetup::HandleActTransition()
{
    Zone::sVars->cameraBoundsL[0] = 256 - screenInfo->center.x;
    Zone::sVars->cameraBoundsB[0] = 694;

    Zone::ReloadEntities(Vector2(TO_FIXED(256), TO_FIXED(694)), true);
    Zone::sVars->timer = sVars->timerStorage;

    if (Stage::CheckSceneFolder("HEHZ")) {
        Camera *camera = GameObject::Get<Camera>(SLOT_CAMERA1);
        camera->SetCameraBoundsXY(); // force screen pos, this doesn't cause any artifacts thankfully

        sVars->snowflakeAddend = screenInfo->position.x;
        sVars->snowflakeYOff   = TO_FIXED(694);
    }

    TileLayer *bg1 = SceneLayer::GetTileLayer(0);
    bg1->scrollPos *= bg1->parallaxFactor;
    for (int32 s = 0; s < bg1->scrollInfoCount; ++s) {
        bg1->scrollInfo[s].scrollPos += 0x29A000 * bg1->scrollInfo[s].parallaxFactor;
    }

    TileLayer *bg2 = SceneLayer::GetTileLayer(1);
    bg2->scrollPos *= bg2->parallaxFactor;
    for (int32 s = 0; s < bg2->scrollInfoCount; ++s) {
        bg2->scrollInfo[s].scrollPos += 0x299000 * bg2->scrollInfo[s].parallaxFactor;
    }

    TileLayer *bg3 = SceneLayer::GetTileLayer(2);
    bg3->scrollPos *= bg3->parallaxFactor;
    for (int32 s = 0; s < bg3->scrollInfoCount; ++s) {
        bg3->scrollInfo[s].scrollPos += 0x299000 * bg3->scrollInfo[s].parallaxFactor;
    }
}

void EHZSetup::StageFinish_EndAct2()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        player->state.Set(&Player::State_Ground);
        if (!player->sidekick) {
            player->stateInput.Set(nullptr);
        }
    }
    sVars->cutsceneActivated = true;
}

#if RETRO_REV0U
void EHZSetup::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(EHZSetup);

    int32 deformation[] = { 1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3,
                            1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3 };

    memcpy(sVars->deformation, deformation, sizeof(deformation));
}
#endif

#if RETRO_INCLUDE_EDITOR
void EHZSetup::EditorDraw() {}

void EHZSetup::EditorLoad() {}
#endif

void EHZSetup::Serialize() {}
} // namespace GameLogic
