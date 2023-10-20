// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: PushSpring Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "PushSpring.hpp"
#include "Global/Zone.hpp"
#include "Global/Player.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(PushSpring);

void PushSpring::Update()
{
    this->state.Run(this);

    this->beingPushed = false;
    this->stateCollide.Run(this);
}

void PushSpring::LateUpdate() {}

void PushSpring::StaticUpdate() {}

void PushSpring::Draw()
{
    if (this->pushOffset) {
        this->stateDraw.Run(this);
    }
    else {
        this->animator.frameID = 0;
        this->animator.DrawSprite(nullptr, false);
    }
}

void PushSpring::Create(void *data)
{
    this->drawFX = FX_FLIP;
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;

        this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);

        if (this->type != PUSHSPRING_V) {
            if (this->direction) {
                this->stateDraw.Set(&PushSpring::Draw_Left);
                this->stateCollide.Set(&PushSpring::Collide_Left);
            }
            else {
                this->stateDraw.Set(&PushSpring::Draw_Right);
                this->stateCollide.Set(&PushSpring::Collide_Right);
            }

            this->groundVel = 0x4000;

            this->hitbox.left   = -20;
            this->hitbox.top    = -16;
            this->hitbox.right  = 20;
            this->hitbox.bottom = 16;

            this->state.Set(&PushSpring::State_WaitForPushed);
        }
        else {
            if (this->direction) {
                this->direction    = FLIP_Y;
                this->stateDraw.Set(&PushSpring::Draw_Bottom);
                this->stateCollide.Set(&PushSpring::Collide_Bottom);
            }
            else {
                this->stateDraw.Set(&PushSpring::Draw_Top);
                this->stateCollide.Set(&PushSpring::Collide_Top);
            }

            this->groundVel = 0x20000;

            this->hitbox.left   = -16;
            this->hitbox.top    = -20;
            this->hitbox.right  = 16;
            this->hitbox.bottom = 20;

            this->state.Set(&PushSpring::State_WaitForPushed);
        }
    }
}

void PushSpring::StageLoad()
{
    sVars->aniFrames.Load("OOZ/PushSpring.bin", SCOPE_STAGE);

    sVars->sfxPush.Get("Stage/Push.wav");
    sVars->sfxSpring.Get("Global/Spring.wav");
}

void PushSpring::Collide_Top()
{
    this->hitbox.top = (this->pushOffset >> 16) - 22;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckCollisionBox(this, &this->hitbox) == C_TOP) {
            player->position.y += 0x20000;
            this->beingPushed |= true;

            if (this->pushOffset >= 0x120000) {
                player->collisionMode = 0;
                player->onGround      = false;
                player->state.Set(&Player::State_Air);
                player->velocity.y    = -0xA0000;

                int32 anim = player->animator.animationID;
                if (anim == Player::ANI_WALK || (anim > Player::ANI_FALL && anim <= Player::ANI_DASH))
                    player->animationReserve = player->animator.animationID;
                else
                    player->animationReserve = Player::ANI_WALK;

                player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                sVars->sfxSpring.Play(false, 255);
                this->state.Set(&PushSpring::State_PushRecoil);
            }
        }
    }
}

void PushSpring::Collide_Bottom()
{
    this->hitbox.top = (this->pushOffset >> 16) - 22;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        int32 yvel = player->velocity.y;

        if (player->CheckCollisionBox(this, &this->hitbox) == C_BOTTOM) {
            if (yvel < 0) {
                player->velocity.y = yvel + 0x3800;
                player->position.y -= 0x20000;
                this->beingPushed |= true;
            }

            if (this->pushOffset >= 0x120000) {
                player->collisionMode = 0;
                player->onGround      = false;
                player->state.Set(&Player::State_Air);
                player->velocity.y    = 0xA0000;
                sVars->sfxSpring.Play(false, 255);
                this->state.Set(&PushSpring::State_PushRecoil);
            }
        }
    }
}

void PushSpring::Collide_Left()
{
    this->hitbox.right = 22 - (this->pushOffset >> 16);

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckCollisionBox(this, &this->hitbox) == C_LEFT) {
            if (player->direction == FLIP_NONE)
                player->position.x += 0x20000;

            if (player->right)
                this->beingPushed |= true;

            if (this->state.Matches(&PushSpring::State_PushRecoil)) {
                if (this->pushOffset > 0x10000) {
                    player->groundVel     = -12 * this->pushOffset / 18;
                    player->velocity.x    = player->groundVel;
                    player->collisionMode = 0;
                    player->controlLock   = 16;
                    player->pushing       = false;
                    player->direction     = this->direction;
                    player->state.Set(&Player::State_Ground);
                    sVars->sfxSpring.Play(false, 255);
                }
            }
        }
    }
}

void PushSpring::Collide_Right()
{
    this->hitbox.right = 22 - (this->pushOffset >> 16);

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckCollisionBox(this, &this->hitbox) == C_RIGHT) {
            if (player->direction == FLIP_X)
                player->position.x -= 0x20000;

            if (player->left)
                this->beingPushed |= true;

            if (this->state.Matches(&PushSpring::State_PushRecoil)) {
                if (this->pushOffset > 0x10000) {
                    player->groundVel     = 12 * this->pushOffset / 18;
                    player->velocity.x    = player->groundVel;
                    player->collisionMode = 0;
                    player->controlLock   = 16;
                    player->pushing       = false;
                    player->direction     = this->direction;
                    player->state.Set(&Player::State_Ground);
                    sVars->sfxSpring.Play(false, 255);
                }
            }
        }
    }
}

void PushSpring::Draw_Top()
{
    Vector2 drawPos = this->position;
    drawPos.y += this->pushOffset;
    this->animator.frameID = 1;
    this->animator.DrawSprite(&drawPos, false);

    drawPos.y              = this->position.y + 2 * this->pushOffset / 3;
    this->animator.frameID = 2;
    this->animator.DrawSprite(&drawPos, false);

    drawPos.y              = this->position.y + this->pushOffset / 3;
    this->animator.frameID = 3;
    this->animator.DrawSprite(&drawPos, false);

    this->animator.frameID = 4;
    this->animator.DrawSprite(nullptr, false);
}

void PushSpring::Draw_Bottom()
{
    Vector2 drawPos = this->position;
    drawPos.y -= this->pushOffset;
    this->animator.frameID = 1;
    this->animator.DrawSprite(&drawPos, false);

    drawPos.y              = this->position.y - 2 * this->pushOffset / 3;
    this->animator.frameID = 2;
    this->animator.DrawSprite(&drawPos, false);

    drawPos.y              = this->position.y - this->pushOffset / 3;
    this->animator.frameID = 3;
    this->animator.DrawSprite(&drawPos, false);

    this->animator.frameID = 4;
    this->animator.DrawSprite(0, false);
}

void PushSpring::Draw_Left()
{
    Vector2 drawPos = this->position;
    drawPos.x += this->pushOffset;
    this->animator.frameID = 1;
    this->animator.DrawSprite(&drawPos, false);

    drawPos.x              = this->position.x + 2 * this->pushOffset / 3;
    this->animator.frameID = 2;
    this->animator.DrawSprite(&drawPos, false);

    drawPos.x              = this->position.x + this->pushOffset / 3;
    this->animator.frameID = 3;
    this->animator.DrawSprite(&drawPos, false);

    this->animator.frameID = 4;
    this->animator.DrawSprite(nullptr, false);
}

void PushSpring::Draw_Right(void)
{
    Vector2 drawPos = this->position;
    drawPos.x -= this->pushOffset;
    this->animator.frameID = 1;
    this->animator.DrawSprite(&drawPos, false);

    drawPos.x              = this->position.x - 2 * this->pushOffset / 3;
    this->animator.frameID = 2;
    this->animator.DrawSprite(&drawPos, false);

    drawPos.x              = this->position.x - this->pushOffset / 3;
    this->animator.frameID = 3;
    this->animator.DrawSprite(&drawPos, false);

    this->animator.frameID = 4;
    this->animator.DrawSprite(nullptr, false);
}

void PushSpring::State_WaitForPushed()
{
    if (this->beingPushed) {
        this->timer = 0;
        this->state.Set(&PushSpring::State_BeingPushed);
    }
}

void PushSpring::State_BeingPushed()
{
    if (this->beingPushed) {
        this->pushOffset += this->groundVel;

        if (this->pushOffset < 0x120000) {
            if (!(this->timer % 10))
                sVars->sfxPush.Play(false, 0xFF);
        }
        else {
            this->pushOffset = 0x120000;
        }

        ++this->timer;
    }
    else {
        this->state.Set(&PushSpring::State_PushRecoil);
    }
}

void PushSpring::State_PushRecoil()
{
    if (this->beingPushed) {
        this->timer = 0;
        this->state.Set(&PushSpring::State_BeingPushed);
    }
    else {
        this->pushOffset -= 0x20000;

        if (this->pushOffset <= 0) {
            this->pushOffset = 0;
            this->state.Set(&PushSpring::State_WaitForPushed);
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void PushSpring::EditorDraw()
{
    int32 dir = this->direction;

    this->animator.SetAnimation(sVars->aniFrames, this->type, false, 0);

    if (this->type == PUSHSPRING_V)
        this->direction *= FLIP_Y;

    this->animator.DrawSprite(nullptr, false);

    this->direction = dir;
}

void PushSpring::EditorLoad()
{
    sVars->aniFrames.Load("OOZ/PushSpring.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Vertical", PUSHSPRING_V);
    RSDK_ENUM_VAR("Horizontal", PUSHSPRING_H);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip", FLIP_NONE);
    RSDK_ENUM_VAR("Flipped", FLIP_X);
}
#endif

void PushSpring::Serialize()
{
    RSDK_EDITABLE_VAR(PushSpring, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(PushSpring, VAR_UINT8, direction);
}

} // namespace GameLogic