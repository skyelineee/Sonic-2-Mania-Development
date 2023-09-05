// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HPZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "HPZSetup.hpp"
#include "Helpers/CutsceneRules.hpp"
#include "Global/Zone.hpp"
#include "Global/Animals.hpp"
#include "Common/Water.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HPZSetup);

void HPZSetup::Update() {}

void HPZSetup::LateUpdate() {}

void HPZSetup::StaticUpdate()
{
    sVars->paletteTimer += 42;
    if (sVars->paletteTimer >= 256) {
        sVars->paletteTimer -= 256;
        paletteBank[0].Rotate(185, 188, true);
    }

    if (!(Zone::sVars->timer & 1)) {
        for (int32 layerID = Zone::sVars->fgLayer[0].id; layerID <= Zone::sVars->fgLayer[1].id; ++layerID)
            SceneLayer::GetTileLayer(layerID)->deformationOffsetW++;
    }

    sVars->waterslidingPlayerCount = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->onGround) {
            Hitbox *hitbox = player->GetHitbox();

            int32 tileInfo  = 0;
            uint8 behaviour = HPZ_TFLAGS_NORMAL;
            HPZSetup::GetTileInfo(player->position.x, (hitbox->bottom << 16) + player->position.y, player->moveLayerPosition.x,
                                  player->moveLayerPosition.y, player->collisionPlane, &tileInfo, &behaviour);

            if (behaviour == HPZ_TFLAGS_SLIDE) {
                if (!player->state.Matches(&Player::State_BubbleBounce)) {
                    if (player->onGround) {
                        if (!player->state.Matches(&Player::State_WaterSlide)) {
                            player->interaction    = true;
                            player->tileCollisions = TILECOLLISION_DOWN;
                            player->position.y += player->jumpOffset;
                            player->nextGroundState.Set(nullptr);
                            player->state.Set(&Player::State_WaterSlide);
                        }
                
                        if (player->onGround)
                            ++sVars->waterslidingPlayerCount;
                    }
                    else if (player->animator.animationID == Player::ANI_FLUME)
                        ++sVars->waterslidingPlayerCount;
                }
            }
        }
    }

    //HCZSetup->waterslidingPlayerCount = 0;
    //foreach_active(Player, player)
    //{
    //    RSDK.GetEntitySlot(player);
    //    if (player->state != Player_State_Static) {
    //        Hitbox *hitbox = Player_GetHitbox(player);
    //        uint16 tile =
    //            RSDK.GetTile(Zone->fgLayer[1], player->position.x >> 20, ((hitbox->bottom << 16) + player->position.y - 0x10000) >> 20) & 0x3FF;
    //
    //        if (((tile >= 226 && tile <= 244) || (tile >= 880 && tile <= 888)) && player->collisionPlane == 1) {
    //            if (player->state != Player_State_BubbleBounce && player->state != Player_State_MightyHammerDrop) {
    //                if (player->onGround) {
    //                    if (player->state != Player_State_WaterSlide) {
    //                        player->interaction    = true;
    //                        player->tileCollisions = TILECOLLISION_DOWN;
    //                        player->position.y += player->jumpOffset;
    //                        player->nextGroundState = StateMachine_None;
    //                        player->state           = Player_State_WaterSlide;
    //                    }
    //
    //                    if (player->onGround)
    //                        ++HCZSetup->waterslidingPlayerCount;
    //                }
    //                else if (player->animator.animationID == ANI_FLUME)
    //                    ++HCZSetup->waterslidingPlayerCount;
    //            }
    //        }
    //    }
    //}
}

void HPZSetup::Draw() {}

void HPZSetup::Create(void *data) {}

void HPZSetup::StageLoad()
{
    Animals::sVars->animalTypes[0] = Animals::Rocky;
    Animals::sVars->animalTypes[1] = Animals::Micky;

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

void HPZSetup::GetTileInfo(int32 x, int32 y, int32 moveOffsetX, int32 moveOffsetY, int32 cPlane, int32 *tile, uint8 *flags)
{
    int32 tileLow  = RSDKTable->GetTile(Zone::sVars->fgLayer[0].id, x >> 20, y >> 20);
    int32 tileHigh = RSDKTable->GetTile(Zone::sVars->fgLayer[1].id, x >> 20, y >> 20);

    int32 flagsLow  = RSDKTable->GetTileFlags(tileLow, cPlane);
    int32 flagsHigh = RSDKTable->GetTileFlags(tileHigh, cPlane);

    int32 tileSolidLow  = 0;
    int32 tileSolidHigh = 0;
    if (cPlane) {
        tileSolidHigh = (tileHigh >> 14) & 3;
        tileSolidLow  = (tileLow >> 14) & 3;
    }
    else {
        tileSolidHigh = (tileHigh >> 12) & 3;
        tileSolidLow  = (tileLow >> 12) & 3;
    }

    *tile  = 0;
    *flags = HPZ_TFLAGS_NORMAL;
    if (flagsHigh && tileSolidHigh) {
        *tile  = tileHigh;
        *flags = flagsHigh;
    }
    else if (flagsLow && tileSolidLow) {
        *tile  = tileLow;
        *flags = flagsLow;
    }
}

#if RETRO_INCLUDE_EDITOR
void HPZSetup::EditorDraw() {}

void HPZSetup::EditorLoad() {}
#endif

void HPZSetup::Serialize() {}
} // namespace GameLogic
