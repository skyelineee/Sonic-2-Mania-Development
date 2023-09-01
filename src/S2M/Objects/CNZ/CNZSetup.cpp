// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: CNZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "CNZSetup.hpp"
#include "Global/Zone.hpp"
#include "Global/Animals.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(CNZSetup);

void CNZSetup::Update() {}
void CNZSetup::LateUpdate() {}
void CNZSetup::StaticUpdate() 
{
    sVars->deformTimer++;
    if (sVars->deformTimer > 7) {
        ++sVars->background->deformationOffset;
        sVars->deformTimer = 0;
    }

    sVars->paletteTimer++;
    if (sVars->paletteTimer == 8) {
        sVars->paletteTimer = 0;
        paletteBank[0].Rotate(232, 234, true);
        paletteBank[0].Rotate(181, 183, true);
        paletteBank[0].Rotate(185, 187, true);
        paletteBank[0].Rotate(197, 199, false);

        sVars->paletteIndex1++;
        sVars->paletteIndex1 %= 18;
        int32 currentColor = sVars->palCycleTable[sVars->paletteIndex1];
        // GetTableValue(temp0, object.paletteIndex1, CNZSetup_palCycleTable);
        paletteBank[0].SetEntry(237, currentColor);
        // SetPaletteEntry(0, 187, temp0);

        sVars->paletteIndex2++;
        sVars->paletteIndex2 %= 18;
        currentColor = sVars->palCycleTable[sVars->paletteIndex2];
        // GetTableValue(temp0, object.paletteIndex2, CNZSetup_palCycleTable);
        paletteBank[0].SetEntry(236, currentColor);
        // SetPaletteEntry(0, 186, temp0);

        sVars->paletteIndex3++;
        sVars->paletteIndex3 %= 18;
        currentColor = sVars->palCycleTable[sVars->paletteIndex3];
        // GetTableValue(temp0, object.paletteIndex3, CNZSetup_palCycleTable);
        paletteBank[0].SetEntry(235, currentColor);
        // SetPaletteEntry(0, 185, temp0);
    }
}

void CNZSetup::Draw() {}

void CNZSetup::Create(void *data) {}

void CNZSetup::StageLoad() 
{
    Animals::sVars->animalTypes[0] = Animals::Flicky;
    Animals::sVars->animalTypes[1] = Animals::Becky;

    // setting offets for each palette index
    sVars->paletteIndex1 = 0;
    sVars->paletteIndex2 = 1;
    sVars->paletteIndex3 = 2;
    sVars->background = SceneLayer::GetTileLayer(0);
    for (int32 i = 0; i < 1024; ++i) {
        sVars->background->deformationData[i] = sVars->deformationTable[i & 63];
    }
}

#if RETRO_INCLUDE_EDITOR
void CNZSetup::EditorDraw() {}

void CNZSetup::EditorLoad() {}
#endif

#if RETRO_REV0U
void CNZSetup::StaticLoad(Static *sVars) 
{ 
	RSDK_INIT_STATIC_VARS(CNZSetup);

    int32 deformationTable[] = { 1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3, 1,
                                       2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3 };

    memcpy(sVars->deformationTable, deformationTable, sizeof(deformationTable));

    int32 palCycleTable[] = { 0xE08000, 0xE0A000, 0xC0E000, 0xE0E0E0, 0xA0E000, 0x40E000, 0x00C060, 0x40C0C0, 0x0080E0, 0x0040E0,
                              0x4000E0, 0x8000C0, 0xE020C0, 0xE00080, 0xE00040, 0xE00000, 0xE04000, 0xE06000 };

    memcpy(sVars->palCycleTable, palCycleTable, sizeof(palCycleTable));
}
#endif

void CNZSetup::Serialize() {}

} // namespace GameLogic