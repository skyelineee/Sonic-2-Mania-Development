// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ARZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "ARZSetup.hpp"
#include "Helpers/CutsceneRules.hpp"
#include "Global/Zone.hpp"
#include "Global/Animals.hpp"
#include "Common/Water.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ARZSetup);

void ARZSetup::Update() {}

void ARZSetup::LateUpdate() {}

void ARZSetup::StaticUpdate()
{
    sVars->paletteTimer += 42;
    if (sVars->paletteTimer >= 256) {
        sVars->paletteTimer -= 256;
        paletteBank[0].Rotate(177, 180, true);
    }

    if (!(Zone::sVars->timer & 1)) {
        for (int32 layerID = Zone::sVars->fgLayer[0].id; layerID <= Zone::sVars->fgLayer[1].id; ++layerID) SceneLayer::GetTileLayer(layerID)->deformationOffsetW++;
    }
}

void ARZSetup::Draw() {}

void ARZSetup::Create(void *data) {}

void ARZSetup::StageLoad()
{
    Animals::sVars->animalTypes[0] = Animals::Flicky;
    Animals::sVars->animalTypes[1] = Animals::Pecky;

    RSDKTable->SetDrawGroupProperties(0, false, Water::DrawHook_ApplyWaterPalette);
    RSDKTable->SetDrawGroupProperties(Zone::sVars->hudDrawGroup, false, Water::DrawHook_RemoveWaterPalette);

    Water::sVars->waterPalette = 1;

    // All Layers between FG Low & FG High get foreground water deformation applied 
    for (int32 layerID = Zone::sVars->fgLayer[0].id; layerID <= Zone::sVars->fgLayer[1].id; ++layerID) {
        TileLayer *layer   = SceneLayer::GetTileLayer(layerID);
        int32 *deformDataW = layer->deformationDataW;

        // FG underwater deformation values
        for (int32 i = 0; i < 4; ++i) {
            deformDataW[0]  = 1;
            deformDataW[1]  = 1;
            deformDataW[2]  = 2;
            deformDataW[3]  = 2;
            deformDataW[4]  = 3;
            deformDataW[5]  = 3;
            deformDataW[6]  = 3;
            deformDataW[7]  = 3;
            deformDataW[8]  = 2;
            deformDataW[9]  = 2;
            deformDataW[10] = 1;
            deformDataW[11] = 1;

            deformDataW[128] = 1;
            deformDataW[129] = 1;
            deformDataW[130] = 2;
            deformDataW[131] = 2;
            deformDataW[132] = 3;
            deformDataW[133] = 3;
            deformDataW[134] = 3;
            deformDataW[135] = 3;
            deformDataW[136] = 2;
            deformDataW[137] = 2;
            deformDataW[138] = 1;
            deformDataW[139] = 1;

            deformDataW[160] = -1;
            deformDataW[161] = -1;
            deformDataW[162] = -2;
            deformDataW[163] = -2;
            deformDataW[164] = -3;
            deformDataW[165] = -3;
            deformDataW[166] = -3;
            deformDataW[167] = -3;
            deformDataW[168] = -2;
            deformDataW[169] = -2;
            deformDataW[170] = -1;
            deformDataW[171] = -1;

            deformDataW += 0x100; // ?
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void ARZSetup::EditorDraw() {}

void ARZSetup::EditorLoad() {}
#endif

void ARZSetup::Serialize() {}
} // namespace GameLogic
