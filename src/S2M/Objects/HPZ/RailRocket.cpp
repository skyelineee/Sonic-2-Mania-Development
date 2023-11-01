// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: RailRocket Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "RailRocket.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"
#include "Global/Dust.hpp"
#include "Global/Dust.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(RailRocket);

void RailRocket::Update()
{
    int32 chainPos       = 0;
    int32 handleAnglePos = this->handleAngle;
    this->handlePos.x    = this->position.x;
    this->handlePos.y    = this->position.y;
    while (chainPos < 8) {
        int32 chainAnglePos = handleAnglePos;
        chainAnglePos >>= 7;
        chainAnglePos &= 0x1FF;

        int32 handleAngleX = Math::Sin256(chainAnglePos);
        handleAngleX *= 0x312;
        this->handlePos.x += handleAngleX;

        int32 handleAngleY = Math::Cos256(chainAnglePos);
        handleAngleY *= 0x312;
        this->handlePos.y += handleAngleY;

        chainPos++;
        handleAnglePos += this->handleAngle;
    }

    if (this->grabDelay > 0) {
        this->grabDelay--;
    }
    this->hasPlayer = false;

    handleAnglePos   = this->position.x;
    int32 rocketPosY = this->position.y;
    this->position.x = this->handlePos.x;
    this->position.y = this->handlePos.y;
    for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (!currentPlayer->state.Matches(&Player::State_Static)) {
            if (this->grabDelay == 0) {
                if (currentPlayer->CheckCollisionTouch(this, &sVars->idleHandleHitbox)) {
                    if (currentPlayer->onGround == false) {
                        currentPlayer->state.Set(&Player::State_Static);
                        currentPlayer->nextAirState.Set(nullptr);
                        currentPlayer->nextGroundState.Set(nullptr);
                        currentPlayer->stateGravity.Set(&Player::Gravity_False);
                        currentPlayer->animator.SetAnimation(currentPlayer->aniFrames, Player::ANI_HANG, false, 0);
                        currentPlayer->groundVel  = 0;
                        currentPlayer->velocity.x = 0;
                        currentPlayer->velocity.y = 0;
                        currentPlayer->position.x = this->position.x;
                        currentPlayer->position.y = currentPlayer->GetHitbox()->top;
                        currentPlayer->position.y = -currentPlayer->position.y;
                        currentPlayer->position.y += 8;
                        currentPlayer->position.y <<= 16;
                        currentPlayer->position.y += this->position.y;

                        Player::sVars->sfxGrab.Play(false, 255);
                        this->hasPlayer = true;
                    }
                }
            }
        }
        else {
            if (currentPlayer->CheckCollisionTouch(this, &sVars->activeHandleHitbox)) {
                if (currentPlayer->jumpPress == true) {
                    currentPlayer->animator.SetAnimation(currentPlayer->aniFrames, Player::ANI_JUMP, false, 0);
                    currentPlayer->state.Set(&Player::State_Air);
                    currentPlayer->velocity.y = -0x40000;
                    this->grabDelay           = 30;
                }
                else {
                    currentPlayer->position.x = this->position.x;
                    currentPlayer->position.y = currentPlayer->GetHitbox()->top;
                    currentPlayer->position.y = -currentPlayer->position.y;
                    currentPlayer->position.y += 8;
                    currentPlayer->position.y <<= 16;
                    currentPlayer->position.y += this->position.y;
                    this->hasPlayer = true;
                }
            }
        }
    }

    this->position.x = handleAnglePos;
    this->position.y = rocketPosY;
    this->state.Run(this);
}

void RailRocket::LateUpdate() {}
void RailRocket::StaticUpdate() {}
void RailRocket::Draw()
{
    this->drawFX         = FX_NONE;
    int32 chainPos       = 0;
    int32 handleAnglePos = this->handleAngle;
    this->chainPos.x     = this->position.x;
    this->chainPos.y     = this->position.y;

    // Draw Handle Chain
    while (chainPos < 7) {
        int32 chainAnglePos = handleAnglePos;
        chainAnglePos >>= 7;
        chainAnglePos &= 0x1FF;

        int32 chainAngleX = Math::Sin256(chainAnglePos);
        chainAngleX *= 0x312;
        this->chainPos.x += chainAngleX;

        int32 chainAngleY = Math::Cos256(chainAnglePos);
        chainAngleY *= 0x312;
        this->chainPos.y += chainAngleY;

        this->animator.SetAnimation(sVars->aniFrames, 0, true, 2);
        this->animator.DrawSprite(&this->chainPos, false);

        chainPos++;
        handleAnglePos += this->handleAngle;
    }

    // Draw Handle
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 3);
    this->animator.DrawSprite(&this->handlePos, false);

    this->drawFX = FX_FLIP;
    // Draw Flame
    if (this->state.Matches(&RailRocket::State_MoveToTarget) || this->state.Matches(&RailRocket::State_ReachedTarget)) {
        this->direction = this->rocketDir;
        this->flameAnimator.Process();
        this->flameAnimator.SetAnimation(sVars->aniFrames, 1, false, 0);
        this->flameAnimator.DrawSprite(nullptr, false);
    }

    this->direction = this->rocketDir;
    this->direction ^= this->frame;
    this->animator.SetAnimation(
        sVars->aniFrames, 0, true,
        this->frame); // this is still animated like v4 so it can retain the animation speed changing on depending on the objects speed
    this->animator.DrawSprite(nullptr, false);
}

void RailRocket::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[1];
        this->state.Set(&RailRocket::State_AwaitPlayer);

        this->startPos       = this->position;
        int32 targetDistance = this->distance;
        targetDistance &= 0x7F;
        if (targetDistance == 0) {
            this->targetPosY = this->position.y;
            this->targetPosY -= 0x1000000;
        }
        else {
            targetDistance <<= 21;
            this->targetPosY = this->position.y;
            this->targetPosY -= targetDistance;
        }
    }
}

void RailRocket::StageLoad()
{
    sVars->aniFrames.Load("HPZ/RailRocket.bin", SCOPE_STAGE);
    sVars->sfxLaunch.Get("Stage/Launch.wav");

    sVars->idleHandleHitbox.left   = -10;
    sVars->idleHandleHitbox.top    = 16;
    sVars->idleHandleHitbox.right  = 10;
    sVars->idleHandleHitbox.bottom = 24;

    sVars->activeHandleHitbox.left   = -8;
    sVars->activeHandleHitbox.top    = -4;
    sVars->activeHandleHitbox.right  = 8;
    sVars->activeHandleHitbox.bottom = 20;

    DebugMode::AddObject(sVars->classID, &RailRocket::DebugSpawn, &RailRocket::DebugDraw);
}

void RailRocket::DebugSpawn()
{
    RailRocket *rocket = GameObject::Create<RailRocket>(nullptr, this->position.x, this->position.y);
    rocket->rocketDir  = this->direction;
}

void RailRocket::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void RailRocket::State_AwaitPlayer()
{
    if (this->handleAngle > 0) {
        if (this->handleAngleVel > 0) {
            this->handleAngleVel -= 16;
        }
        else {
            this->handleAngleVel -= 8;
        }
    }
    else {
        if (this->handleAngleVel < 0) {
            this->handleAngleVel += 16;
        }
        else {
            this->handleAngleVel += 8;
        }
    }
    this->handleAngle += this->handleAngleVel;

    if (this->hasPlayer == true) {
        this->groundVel = -0x80000;
        this->state.Set(&RailRocket::State_MoveToTarget);
        sVars->sfxLaunch.Play(false, 255);
    }
}

void RailRocket::State_MoveToTarget()
{
    this->position.y += this->groundVel;
    int32 speed = this->groundVel;
    speed >>= 1;
    if (this->rocketDir == 0) {
        this->position.x += speed;
        this->handleAngle += 0x20;
        if (this->handleAngle >= 0x200) {
            this->handleAngle = 0x200;
        }
    }
    else {
        this->position.x -= speed;
        this->handleAngle -= 0x20;
        if (this->handleAngle <= -0x200) {
            this->handleAngle = -0x200;
        }
    }

    if (this->position.y <= this->targetPosY) {
        this->groundVel  = 0;
        this->position.y = this->targetPosY;
        if (this->rocketDir == 0) {
            this->handleAngleVel = -0x80;
        }
        else {
            this->handleAngleVel = 0x80;
        }
        this->state.Set(&RailRocket::State_ReachedTarget);
    }

    this->frame = this->animationTimer;
    this->frame >>= 3;
    speed = abs(this->groundVel);
    speed >>= 16;
    this->animationTimer += speed;
    this->animationTimer &= 15;

    if (!(Zone::sVars->timer & 7)) {
        Dust *dust = GameObject::Create<Dust>(nullptr, this->position.x, this->position.y);
        dust->state.Set(&Dust::State_DustPuff);
        if (this->rocketDir == FLIP_NONE)
            dust->position.x += TO_FIXED(16);
        else
            dust->position.x -= TO_FIXED(16);
        dust->position.y += TO_FIXED(22);
    }
}

void RailRocket::State_ReachedTarget()
{
    if (this->handleAngle > 0) {
        if (this->handleAngleVel > 0) {
            this->handleAngleVel -= 16;
        }
        else {
            this->handleAngleVel -= 8;
        }
    }
    else {
        if (this->handleAngleVel < 0) {
            this->handleAngleVel += 16;
        }
        else {
            this->handleAngleVel += 8;
        }
    }

    this->handleAngle += this->handleAngleVel;
    this->timer++;
    if (this->timer == 60) {
        this->timer = 0;
        this->state.Set(&RailRocket::State_ReturnToStart);
    }
}

void RailRocket::State_ReturnToStart()
{
    this->groundVel += 0x1800;
    if (this->groundVel > 0x80000) {
        this->groundVel = 0x80000;
    }
    this->position.y += this->groundVel;

    int32 speed = this->groundVel;
    speed >>= 1;
    if (this->rocketDir == 0) {
        this->position.x += speed;
        this->handleAngle -= 0x20;
        if (this->handleAngle <= -0x200) {
            this->handleAngle = -0x200;
        }
    }
    else {
        this->position.x -= speed;
        this->handleAngle += 0x20;
        if (this->handleAngle >= 0x200) {
            this->handleAngle = 0x200;
        }
    }

    if (this->position.y >= this->startPos.y) {
        this->groundVel  = 0;
        this->position.x = this->startPos.x;
        this->position.y = this->startPos.y;
        if (this->rocketDir == 0) {
            this->handleAngleVel = 0x80;
        }
        else {
            this->handleAngleVel = -0x80;
        }
        this->state.Set(&RailRocket::State_Finished);
    }

    this->frame = this->animationTimer;
    this->frame >>= 3;
    speed = abs(this->groundVel);
    speed >>= 16;
    this->animationTimer += speed;
    this->animationTimer &= 15;
}

void RailRocket::State_Finished()
{
    if (this->handleAngle > 0) {
        if (this->handleAngleVel > 0) {
            this->handleAngleVel -= 16;
        }
        else {
            this->handleAngleVel -= 8;
        }
    }
    else {
        if (this->handleAngleVel < 0) {
            this->handleAngleVel += 16;
        }
        else {
            this->handleAngleVel += 8;
        }
    }
    this->handleAngle += this->handleAngleVel;

    if (this->hasPlayer == false) {
        this->state.Set(&RailRocket::State_AwaitPlayer);
    }
}

#if RETRO_INCLUDE_EDITOR
void RailRocket::EditorDraw() {}

void RailRocket::EditorLoad()
{
    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Right");
}
#endif

void RailRocket::Serialize()
{
    RSDK_EDITABLE_VAR(RailRocket, VAR_INT32, rocketDir);
    RSDK_EDITABLE_VAR(RailRocket, VAR_UINT8, distance);
}

} // namespace GameLogic