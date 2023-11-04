// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HTZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "HTZSetup.hpp"
#include "Global/Zone.hpp"
#include "Global/Animals.hpp"
#include "Helpers/RPCHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HTZSetup);

void HTZSetup::Update() {}
void HTZSetup::LateUpdate() {}
void HTZSetup::StaticUpdate()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->onGround) {
            Hitbox *hitbox = player->GetHitbox();

            int32 tileInfo  = 0;
            uint8 behaviour = HTZ_TFLAGS_NORMAL;
            HTZSetup::GetTileInfo(player->position.x, (hitbox->bottom << 16) + player->position.y, player->moveLayerPosition.x,
                                  player->moveLayerPosition.y, player->collisionPlane, &tileInfo, &behaviour);

            if (behaviour == HTZ_TFLAGS_LAVA && player->shield != Player::Shield_Fire && !sVars->fadeTimer) {
                player->HurtFlip();
            }
        }
    }
}

void HTZSetup::Draw() {}

void HTZSetup::Create(void *data) {}

void HTZSetup::StageLoad()
{
    Animals::sVars->animalTypes[0] = Animals::Locky;
    Animals::sVars->animalTypes[1] = Animals::Wocky;

    if (globals->gameMode != MODE_TIMEATTACK) {
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
        SetPresence(playingAsText, "In Hill Top", "htz", "Hill Top", characterImage, characterText);
    }
}

void HTZSetup::GetTileInfo(int32 x, int32 y, int32 moveOffsetX, int32 moveOffsetY, int32 cPlane, int32 *tile, uint8 *flags)
{
    int32 tileLow  = RSDKTable->GetTile(Zone::sVars->fgLayer[0].id, x >> 20, y >> 20);
    int32 tileHigh = RSDKTable->GetTile(Zone::sVars->fgLayer[1].id, x >> 20, y >> 20);

    int32 flagsLow  = RSDKTable->GetTileFlags(tileLow, cPlane);
    int32 flagsHigh = RSDKTable->GetTileFlags(tileHigh, cPlane);

    int32 tileMove  = 0;
    int32 flagsMove = 0;
    if (Zone::sVars->moveLayer.id) {
        tileMove  = RSDKTable->GetTile(Zone::sVars->moveLayer.id, (moveOffsetX + x) >> 20, (moveOffsetY + y) >> 20);
        flagsMove = RSDKTable->GetTileFlags(tileMove, cPlane);
    }

    int32 tileSolidLow  = 0;
    int32 tileSolidHigh = 0;
    int32 tileSolidMove = 0;
    if (cPlane) {
        tileSolidHigh = (tileHigh >> 14) & 3;
        tileSolidLow  = (tileLow >> 14) & 3;
    }
    else {
        tileSolidHigh = (tileHigh >> 12) & 3;
        tileSolidLow  = (tileLow >> 12) & 3;
    }

    if (Zone::sVars->moveLayer.id)
        tileSolidMove = (tileMove >> 12) & 3;

    *tile  = 0;
    *flags = HTZ_TFLAGS_NORMAL;
    if (flagsMove && tileSolidMove) {
        *tile  = tileMove;
        *flags = flagsMove;
    }
    else if (flagsHigh && tileSolidHigh) {
        *tile  = tileHigh;
        *flags = flagsHigh;
    }
    else if (flagsLow && tileSolidLow) {
        *tile  = tileLow;
        *flags = flagsLow;
    }
}

#if RETRO_INCLUDE_EDITOR
void HTZSetup::EditorDraw() {}

void HTZSetup::EditorLoad() {}
#endif

#if RETRO_REV0U
void HTZSetup::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(HTZSetup); }
#endif

void HTZSetup::Serialize() {}

} // namespace GameLogic