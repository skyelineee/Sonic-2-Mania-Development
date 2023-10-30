// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: OneWayDoor Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "OneWayDoor.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(OneWayDoor);

void OneWayDoor::Update()
{
    this->state.Run(this);

    this->animator.Process();
}

void OneWayDoor::LateUpdate() {}

void OneWayDoor::StaticUpdate() {}

void OneWayDoor::Draw() { this->stateDraw.Run(this); }

void OneWayDoor::Create(void *data)
{
    this->active        = ACTIVE_BOUNDS;
    this->visible       = true;
    this->updateRange.x = 0x1000000;
    this->updateRange.y = 0x1000000;
    this->state.Set(&OneWayDoor::State_MoveDown);
    this->drawGroup     = Zone::sVars->objectDrawGroup[1] - 2;

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    this->stateDraw.Set(&OneWayDoor::Draw_CPZ);
    this->groundVel = 0x80000;
}

void OneWayDoor::StageLoad()
{
    sVars->aniFrames.Load("CPZ/OneWayDoor.bin", SCOPE_STAGE);

    sVars->hitboxTop.left   = -8;
    sVars->hitboxTop.top    = -32;
    sVars->hitboxTop.right  = 8;
    sVars->hitboxTop.bottom = 0;

    sVars->hitboxBottom.left   = -8;
    sVars->hitboxBottom.top    = 0;
    sVars->hitboxBottom.right  = 8;
    sVars->hitboxBottom.bottom = 32;

    sVars->hitboxRange.left   = -64;
    sVars->hitboxRange.top    = -32;
    sVars->hitboxRange.right  = 8;
    sVars->hitboxRange.bottom = 32;
}

void OneWayDoor::HandlePlayerInteractions()
{
    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

    this->state.Set(&OneWayDoor::State_MoveDown);

    bool32 isBehind = false;
    if (this->direction == FLIP_X)
        isBehind = player1->position.x >= this->position.x;
    else if (!this->direction)
        isBehind = player1->position.x <= this->position.x;

    for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        this->position.y -= 2 * this->offsetY;

        currentPlayer->CheckCollisionBox(this, &sVars->hitboxTop);

        currentPlayer->CheckCollisionBox(this, &sVars->hitboxBottom);
        this->position.y += 2 * this->offsetY;

        if (currentPlayer->velocity.x < 0x60000)
            sVars->hitboxRange.left = -64;
        else
            sVars->hitboxRange.left = -88;

        if (currentPlayer->CheckCollisionTouch(this, &sVars->hitboxRange)) {
            this->state.Set(&OneWayDoor::State_MoveUp);
        }
    }
}

void OneWayDoor::State_MoveDown()
{
    if (this->offsetY > 0)
        this->offsetY -= this->groundVel;

    OneWayDoor::HandlePlayerInteractions();
}

void OneWayDoor::State_MoveUp()
{
    if (this->offsetY < 0x200000)
        this->offsetY += this->groundVel;

    OneWayDoor::HandlePlayerInteractions();
}

void OneWayDoor::Draw_CPZ()
{
    this->animator.frameID = 0;
    this->position.y -= 2 * this->offsetY;
    this->animator.DrawSprite(nullptr, false);

    this->animator.frameID = 1;
    this->animator.DrawSprite(nullptr, false);

    this->position.y += 2 * this->offsetY;
}

#if RETRO_INCLUDE_EDITOR
void OneWayDoor::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    OneWayDoor::Draw_CPZ();
}

void OneWayDoor::EditorLoad()
{
    sVars->aniFrames.Load("CPZ/OneWayDoor.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip", FLIP_NONE);
    RSDK_ENUM_VAR("Flipped", FLIP_X);
}
#endif

void OneWayDoor::Serialize() { RSDK_EDITABLE_VAR(OneWayDoor, VAR_UINT8, direction); }
} // namespace GameLogic