// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: CPZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "CPZSetup.hpp"
#include "Global/Animals.hpp"
#include "Global/Zone.hpp"
#include "Common/Water.hpp"
#include "Common/Decoration.hpp"
#include "Common/ParallaxSprite.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(CPZSetup);

void CPZSetup::Update(void) {}

void CPZSetup::LateUpdate(void) {}

void CPZSetup::StaticUpdate(void)
{
    // Animate the converyor belt thingys in the background
    if (!(Zone::sVars->timer & 3)) {
        --sVars->aniTileFrame;
        sVars->aniTileFrame &= 0xF;
        RSDKTable->DrawAniTiles(sVars->aniTiles, 762, 109, sVars->aniTileFrame + 48, 16, 16);
    }

    if (!(Zone::sVars->timer & 7)) {
        ++sVars->background->deformationOffset;

        ++sVars->bgTowerLightPalIndex;
        sVars->bgTowerLightPalIndex &= 15;
        paletteBank[0].SetEntry(159, paletteBank[3].GetEntry(sVars->bgTowerLightPalIndex));

        ++sVars->rainbowLightPalIndex;
        sVars->rainbowLightPalIndex %= 21;
        paletteBank[0].SetEntry(158, paletteBank[3].GetEntry(sVars->rainbowLightPalIndex + 16));

        sVars->chemLiquidPalIndex1 += 3;
        sVars->chemLiquidPalIndex1 %= 27;
        paletteBank[0].SetEntry(141, paletteBank[3].GetEntry(sVars->chemLiquidPalIndex1 + 48));

        sVars->chemLiquidPalIndex2 += 3;
        sVars->chemLiquidPalIndex2 %= 27;
        paletteBank[0].SetEntry(142, paletteBank[3].GetEntry(sVars->chemLiquidPalIndex2 + 48));

        sVars->chemLiquidPalIndex3 += 3;
        sVars->chemLiquidPalIndex3 %= 27;
        paletteBank[0].SetEntry(143, paletteBank[3].GetEntry(sVars->chemLiquidPalIndex3 + 48));
    }
}

void CPZSetup::Draw() {}

void CPZSetup::Create(void *data) {}

void CPZSetup::StageLoad()
{
    sVars->aniTiles = RSDKTable->LoadSpriteSheet("CPZ/Objects.gif", SCOPE_STAGE);

    RSDKTable->SetDrawGroupProperties(0, false, Water::DrawHook_ApplyWaterPalette);
    RSDKTable->SetDrawGroupProperties(Zone::sVars->hudDrawGroup, false, Water::DrawHook_RemoveWaterPalette);
    Water::sVars->waterPalette = 2;

    sVars->chemLiquidPalIndex1 = 0;
    sVars->chemLiquidPalIndex2 = 1;
    sVars->chemLiquidPalIndex3 = 2;

    sVars->background = SceneLayer::GetTileLayer(0);
    for (int32 i = 0; i < 0x400; ++i) {
        sVars->background->deformationData[i] = sVars->deformation[i & 0x3F];
    }

    Animals::sVars->animalTypes[0] = Animals::Locky;
    Animals::sVars->animalTypes[1] = Animals::Pocky;

    if (Zone::sVars->actID) {
    }
    else {
    }
}

void CPZSetup::StageFinish_EndAct1()
{
    SceneLayer::GetTileLayer(0);
    Zone::StoreEntities(Vector2(TO_FIXED(screenInfo->position.x + screenInfo->center.x), TO_FIXED(screenInfo->size.y + screenInfo->position.y)));
    Stage::LoadScene();
}

void CPZSetup::StageFinish_EndAct2() {}

#if RETRO_REV0U
void CPZSetup::StaticLoad(Static *sVars) 
{ 
    RSDK_INIT_STATIC_VARS(CPZSetup);

    int32 deformation[] = { 1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3,
                            1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3 };

    memcpy(sVars->deformation, deformation, sizeof(deformation));
}
#endif

#if RETRO_INCLUDE_EDITOR
void CPZSetup::EditorDraw() {}

void CPZSetup::EditorLoad()
{
    RSDK_ACTIVE_VAR(Decoration::sVars, type);
    RSDK_ENUM_VAR("Warning Sign", CPZ_DECORATION_WARNSIGN);

    RSDK_ACTIVE_VAR(ParallaxSprite::sVars, aniID);
    RSDK_ENUM_VAR("Girder", CPZ_PARALLAXSPRITE_GIRDER);
    RSDK_ENUM_VAR("Chemical 1", CPZ_PARALLAXSPRITE_CHEMICAL1);
    RSDK_ENUM_VAR("Chemical 2", CPZ_PARALLAXSPRITE_CHEMICAL2);
}
#endif

void CPZSetup::Serialize() {}
} // namespace GameLogic