// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: MetalArm Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "MetalArm.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Helpers/Soundboard.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(MetalArm);

void MetalArm::Update()
{
    this->moveOffset.x = -this->armPosition.x;
    this->moveOffset.y = -this->armPosition.y;

    int32 timerA       = MIN(this->durationA, this->moveTimer);
    int32 interpolateA = ((this->endAngleA - this->startAngleA) << 16) / this->durationA;
    this->armAngle.x   = (this->startAngleA << 16) + interpolateA * timerA;

    int32 timerB       = MIN(this->durationB, this->moveTimer);
    int32 interpolateB = ((this->endAngleB - this->startAngleB) << 16) / this->durationB;
    this->armAngle.y   = (this->startAngleB << 16) + interpolateB * timerB;

    this->armPosition = MetalArm::GetArmPosition();
    this->armPosition.x &= 0xFFFF0000;
    this->armPosition.y &= 0xFFFF0000;

    this->position.x = this->armPosition.x;
    this->position.y = this->armPosition.y;
    this->moveOffset.x += this->position.x;
    this->moveOffset.y += this->position.y;

    MetalArm::CheckPlayerCollisions();

    this->position.x = this->startPos.x;
    this->position.y = this->startPos.y;

    this->state.Run(this);
}

void MetalArm::LateUpdate() {}

void MetalArm::StaticUpdate() {}

void MetalArm::Draw()
{
    this->rotation = 0;
    this->baseAnimator.DrawSprite(nullptr, false);

    int32 x = 0x2400 * Math::Cos512((this->armAngle.x >> 16)) + this->position.x;
    int32 y = 0x2400 * Math::Sin512((this->armAngle.x >> 16)) + this->position.y;

    Vector2 drawPos  = MetalArm::GetArmPosition();
    this->position.x = x;
    this->position.y = y;
    this->position.x &= 0xFFFF0000;
    this->position.y &= 0xFFFF0000;
    this->rotation = (this->armAngle.x + this->armAngle.y) >> 16;
    this->armBAnimator.DrawSprite(nullptr, false);

    this->position.x = this->startPos.x;
    this->position.y = this->startPos.y;
    this->position.x &= 0xFFFF0000;
    this->position.y &= 0xFFFF0000;
    this->rotation = (this->armAngle.x >> 16);
    this->armAAnimator.DrawSprite(nullptr, false);

    this->rotation = 0;
    this->position = drawPos;
    this->position.x &= 0xFFFF0000;
    this->position.y &= 0xFFFF0000;
    this->platformAnimator.DrawSprite(&drawPos, false);

    this->position.x = this->startPos.x;
    this->position.y = this->startPos.y;
}

void MetalArm::Create(void *data)
{
    this->active        = ACTIVE_BOUNDS;
    this->drawGroup     = Zone::sVars->objectDrawGroup[1];
    this->startPos.x    = this->position.x;
    this->startPos.y    = this->position.y;
    this->visible       = true;
    this->drawFX        = FX_ROTATE | FX_FLIP;
    this->updateRange.x = 0x1000000;
    this->updateRange.y = 0x1000000;

    if (!this->durationA)
        this->durationA = 60;

    if (!this->durationB)
        this->durationB = 40;

    if (!this->holdDuration)
        this->holdDuration = 60;

    this->hitbox.left   = -56;
    this->hitbox.top    = -27;
    this->hitbox.right  = 56;
    this->hitbox.bottom = -7;

    this->baseAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->armAAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    this->armBAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    this->platformAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);

    this->armAngle.x  = this->startAngleA << 16;
    this->armAngle.y  = this->startAngleB << 16;
    this->moveTimer   = 0;
    this->armPosition = MetalArm::GetArmPosition();
    this->state.Set(&MetalArm::State_Idle);
}

void MetalArm::StageLoad()
{
    sVars->aniFrames.Load("DEZ/MetalArm.bin", SCOPE_STAGE);
}

Vector2 MetalArm::GetArmPosition()
{
    int32 x = 0x2400 * Math::Cos512((this->armAngle.x >> 16)) + this->position.x;
    int32 y = 0x2400 * Math::Sin512((this->armAngle.x >> 16)) + this->position.y;

    Vector2 armPos;
    armPos.x = x + 0x3800 * Math::Cos512((this->armAngle.x + this->armAngle.y) >> 16);
    armPos.y = y + 0x3800 * Math::Sin512((this->armAngle.x + this->armAngle.y) >> 16);

    return armPos;
}

void MetalArm::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        int32 playerID = RSDKTable->GetEntitySlot(player);

        if ((1 << playerID) & this->stoodPlayers) {
            player->position.x += this->moveOffset.x;
            player->position.y += this->moveOffset.y;
            player->position.y += 0x10000;
        }

        if (!player->CheckCollisionPlatform(this, &this->hitbox))
            this->stoodPlayers &= ~(1 << playerID);
        else
            this->stoodPlayers |= 1 << playerID;
    }
}

void MetalArm::State_Idle()
{
    if ((this->stoodPlayers & 1)) {
        this->moveTimer = 0;
        this->holdTimer = 0;
        this->active    = ACTIVE_NORMAL;
        this->state.Set(&MetalArm::State_MoveToHold);
    }
}

void MetalArm::State_MoveToHold()
{
    if (this->moveTimer >= MAX(this->durationA, this->durationB)) {
        this->holdTimer = 0;
        this->state.Set(&MetalArm::State_Holding);
    }
    else {
        this->moveTimer++;
    }
}

void MetalArm::State_Holding()
{
    if (this->stoodPlayers & 1) {
        this->holdTimer = 0;
    }
    else {
        if (this->holdTimer >= this->holdDuration) {
            this->state.Set(&MetalArm::State_MoveToStart);
        }
        else {
            this->holdTimer++;
        }
    }
}

void MetalArm::State_MoveToStart()
{
    if (this->moveTimer <= 0) {
        this->state.Set(&MetalArm::State_Idle);
        this->active = ACTIVE_BOUNDS;
    }
    else {
        this->moveTimer--;
    }
}

#if RETRO_INCLUDE_EDITOR
void MetalArm::EditorDraw()
{
    MetalArm::Create(nullptr);

    this->startPos.x  = this->position.x;
    this->startPos.y  = this->position.y;
    this->armAngle.x  = this->startAngleA << 16;
    this->armAngle.y  = this->startAngleB << 16;
    this->armPosition = MetalArm::GetArmPosition();

    MetalArm::Draw();
}

void MetalArm::EditorLoad() { sVars->aniFrames.Load("DEZ/MetalArm.bin", SCOPE_STAGE); }
#endif

void MetalArm::Serialize()
{
    RSDK_EDITABLE_VAR(MetalArm, VAR_INT32, startAngleA);
    RSDK_EDITABLE_VAR(MetalArm, VAR_INT32, startAngleB);
    RSDK_EDITABLE_VAR(MetalArm, VAR_INT32, endAngleA);
    RSDK_EDITABLE_VAR(MetalArm, VAR_INT32, endAngleB);
    RSDK_EDITABLE_VAR(MetalArm, VAR_INT32, durationA);
    RSDK_EDITABLE_VAR(MetalArm, VAR_INT32, durationB);
    RSDK_EDITABLE_VAR(MetalArm, VAR_INT32, holdDuration);
}
} // namespace GameLogic