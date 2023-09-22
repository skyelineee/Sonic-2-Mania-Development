// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Setup Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Setup.hpp"
#include "Global/Zone.hpp"
#include "Helpers/RPCHelpers.hpp"
#include "HP_Message.hpp"
#include "HP_Checkpoint.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Setup);

void HP_Setup::Update() {}
void HP_Setup::LateUpdate() {}
void HP_Setup::StaticUpdate() {
	if (++sVars->paletteTimer == 4) {
        sVars->paletteTimer = 0;

		sVars->paletteIndex1 = (sVars->paletteIndex1 + 1) & 15;
        paletteBank[0].SetEntry(142, sVars->starPalCycle[sVars->paletteIndex1]);

        sVars->paletteIndex2 = (sVars->paletteIndex2 + 1) & 15;
        paletteBank[0].SetEntry(143, sVars->starPalCycle[sVars->paletteIndex2]);
	}
}
void HP_Setup::Draw() {}

void HP_Setup::Create(void *data) {}

void HP_Setup::StageLoad()
{
    if (!globals->playerID)
        globals->playerID = ID_DEFAULT_PLAYER;

    sVars->gotEmerald = false;
    sceneInfo->timeEnabled = true;

    uint8 paletteID = 0;

    // grab the actID & destroy any halfpipe entities placed in the scene
    for (auto setup : GameObject::GetEntities<HP_Setup>(FOR_ALL_ENTITIES)) {
        Zone::sVars->actID = setup->stageID;
        paletteID          = setup->paletteID;
        setup->Destroy();
    }

    int32 start = 0;
    start = paletteID << 3;

    paletteBank[0].Copy(1, start, 192, 8);

    sVars->stageColor1 = paletteBank[0].GetEntry(197) | (0xFF << 24);
    sVars->stageColor2   = paletteBank[0].GetEntry(192) | (0xFF << 24);
    sVars->arrowColor    = 0xE0E000 | (0xFF << 24);
    sVars->railEdgeColor = 0xE0A020 | (0xFF << 24);

    sVars->checkpointID = 0;
    int32 count = 0;
    for (auto checkpoint : GameObject::GetEntities<HP_Checkpoint>(FOR_ALL_ENTITIES)) {
        if (count < 3) {
            sVars->ringCountSonic[count] = checkpoint->ringCountSonic;
            sVars->ringCountKnux[count]  = checkpoint->ringCountKnux;
            sVars->ringCount2P[count]    = checkpoint->ringCount2P;
            count++;
        }
        checkpoint->ringCount2P    = 0;
        checkpoint->ringCountSonic = 0;
        checkpoint->ringCountKnux  = 0;

        if (count == 1) {
            checkpoint->Destroy(); // first checkpoint is just for initial ring count
        }
    }

    if (GET_CHARACTER_ID(2)) {
        sVars->ringCounts = sVars->ringCount2P;
    }
    else {
        if (CHECK_CHARACTER_ID(ID_KNUCKLES, 1))
            sVars->ringCounts = sVars->ringCountKnux;
        else
            sVars->ringCounts = sVars->ringCountSonic;
    }

    for (int32 i = RESERVE_ENTITY_COUNT; i < ENTITY_COUNT; ++i) {
        Entity *entity = GameObject::Get(i);
        if (!entity->classID)
            entity->position.y = 0x7FFF0000;
        else
            entity->active = ACTIVE_NEVER;
    }

    Entity *storage = GameObject::Get(ENTITY_COUNT - 1);
    int32 sortCount = ENTITY_COUNT;
    for (int32 i = RESERVE_ENTITY_COUNT; i < sortCount; ++i) {

        int32 slot1 = sortCount - 1;
        int32 slot2 = sortCount - 2;
        while (slot1 > i) {
            Entity *entity1 = GameObject::Get(slot1);
            Entity *entity2 = GameObject::Get(slot2);

            if (entity1->position.y < entity2->position.y) {
                GameObject::Copy(storage, entity1, false);
                GameObject::Copy(entity1, entity2, false);
                GameObject::Copy(entity2, storage, false);
            }

            slot1--;
            slot2--;
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
    SetPresence(playingAsText, "In a Special Stage", "doggy", "doggy", characterImage, characterText);
}

#if RETRO_INCLUDE_EDITOR
void HP_Setup::EditorDraw() {}

void HP_Setup::EditorLoad()
{
    RSDK_ACTIVE_VAR(sVars, stageID);
    RSDK_ENUM_VAR("Act 1");
    RSDK_ENUM_VAR("Act 2");
    RSDK_ENUM_VAR("Act 3");
    RSDK_ENUM_VAR("Act 4");
    RSDK_ENUM_VAR("Act 5");
    RSDK_ENUM_VAR("Act 6");
    RSDK_ENUM_VAR("Act 7");
    RSDK_ENUM_VAR("None");
}
#endif

#if RETRO_REV0U
void HP_Setup::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(HP_Setup);

    color starPalCycle[] = { 0xE0E0E0, 0xD0D0D0, 0xC0C0C0, 0xB0B0B0, 0xA0A0A0, 0x909090, 0x808080, 0x707070,
                             0x606060, 0x707070, 0x808080, 0x909090, 0xA0A0A0, 0xB0B0B0, 0xC0C0C0, 0xD0D0D0 };
    memcpy(sVars->starPalCycle, starPalCycle, sizeof(starPalCycle));
}
#endif

void HP_Setup::Serialize()
{
    RSDK_EDITABLE_VAR(HP_Setup, VAR_UINT8, stageID);
    RSDK_EDITABLE_VAR(HP_Setup, VAR_UINT8, paletteID);
}

} // namespace GameLogic