// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ConveyorBelt Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "ConveyorBelt.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ConveyorBelt);

void ConveyorBelt::Update()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckCollisionTouch(this, &this->hitbox)) {
            if (player->onGround)
                player->position.x += this->speed << 14;
        }
    }
}

void ConveyorBelt::LateUpdate() {}

void ConveyorBelt::StaticUpdate() {}

void ConveyorBelt::Draw() {}

void ConveyorBelt::Create(void *data)
{
    this->active        = ACTIVE_BOUNDS;
    this->visible       = false;
    this->updateRange.x = 0x400000;
    this->updateRange.y = 0x400000;
    this->drawGroup     = Zone::sVars->objectDrawGroup[1];

    if (sceneInfo->inEditor) {
        if (!this->speed)
            this->speed = -8;

        if (!this->size.x)
            this->size.x = 0xC00000;

        if (!this->size.y)
            this->size.y = 0x200000;
    }

    this->hitbox.left   = -(this->size.x >> 17) - 16;
    this->hitbox.top    = -(this->size.y >> 17) - 16;
    this->hitbox.right  = (this->size.x >> 17) + 16;
    this->hitbox.bottom = (this->size.y >> 17) + 16;
}

void ConveyorBelt::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void ConveyorBelt::EditorDraw()
{
    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        DrawHelpers::DrawHitboxOutline(this->position.x, this->position.y, &this->hitbox, FLIP_NONE, 0xFFFF00);

        RSDK_DRAWING_OVERLAY(false);
    }
}

void ConveyorBelt::EditorLoad() {}
#endif

void ConveyorBelt::Serialize()
{
    RSDK_EDITABLE_VAR(ConveyorBelt, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(ConveyorBelt, VAR_INT32, speed);
}

} // namespace GameLogic