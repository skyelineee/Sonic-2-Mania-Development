// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: RisingLava Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "RisingLava.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(RisingLava);

void RisingLava::Update()
{
    this->state.Run(this);
}

void RisingLava::LateUpdate() {}

void RisingLava::StaticUpdate() {}

void RisingLava::Draw() {}

void RisingLava::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->hitbox.left   = -(this->size.x >> 17);
        this->hitbox.top    = -(this->size.y >> 17);
        this->hitbox.right  = this->size.x >> 17;
        this->hitbox.bottom = this->size.y >> 17;

        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->limit *= -0x10000;
        this->state.Set(&RisingLava::State_CheckRiseStart);
    }
}

void RisingLava::StageLoad() { sVars->sfxRumble.Get("Stage/Rumble.wav"); }

void RisingLava::State_CheckRiseStart()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->CheckCollisionTouch(this, &this->hitbox) && !player->sidekick) {
            TileLayer *move               = SceneLayer::GetTileLayer(Zone::sVars->moveLayer.id);
            move->drawGroup[0]            = 5;
            move->scrollInfo[0].scrollPos = -this->offset.x;
            move->scrollPos               = -this->offset.y;
            this->active                  = ACTIVE_NORMAL;
            this->state.Set(&RisingLava::State_RiseShake);
        }
    }
}

void RisingLava::State_RiseShake()
{
    TileLayer *move = RSDKTable->GetTileLayer(Zone::sVars->moveLayer.id);

    move->scrollPos += 0x8000;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        player->collisionLayers |= Zone::sVars->moveLayerMask;
        player->moveLayerPosition.x = move->scrollInfo[0].scrollPos;
        player->moveLayerPosition.y = move->scrollPos;
    }

    if (!(Zone::sVars->timer & 3))
        Camera::ShakeScreen(0, 0, 2, 0, 0);

    if (!(Zone::sVars->timer & 7))
        sVars->sfxRumble.Play(false, 255);

    if (move->scrollPos > this->limit)
        this->state.Set(&RisingLava::State_StoppedRising);
}

void RisingLava::State_StoppedRising()
{
    TileLayer *move = SceneLayer::GetTileLayer(Zone::sVars->moveLayer.id);

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        player->collisionLayers |= Zone::sVars->moveLayerMask;
        player->moveLayerPosition.x = move->scrollInfo[0].scrollPos;
        player->moveLayerPosition.y = move->scrollPos;
    }
}

#if RETRO_INCLUDE_EDITOR
void RisingLava::EditorDraw()
{
    DrawHelpers::DrawRectOutline(this->position.x, this->position.y, this->size.x, this->size.y, 0xFFFF00);

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        DrawHelpers::DrawRectOutline(this->offset.x, this->offset.y, this->size.x, this->size.y, 0xFFFF00);

        DrawHelpers::DrawArrow(this->position.x, this->position.y, this->offset.x, this->offset.y, 0xFFFF00, INK_NONE, 0xFF);

        RSDK_DRAWING_OVERLAY(false);
    }
}

void RisingLava::EditorLoad() {}
#endif

void RisingLava::Serialize()
{
    RSDK_EDITABLE_VAR(RisingLava, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(RisingLava, VAR_VECTOR2, offset);
    RSDK_EDITABLE_VAR(RisingLava, VAR_ENUM, limit);
}

} // namespace GameLogic