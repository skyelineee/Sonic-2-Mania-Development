// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Aquis Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Aquis.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Aquis);

void Aquis::Update() { this->state.Run(this); }

void Aquis::LateUpdate() {}

void Aquis::StaticUpdate() {}

void Aquis::Draw()
{
    if (this->mainAnimator.animationID == 4) {
        this->mainAnimator.DrawSprite(nullptr, false);
    }
    else {
        this->mainAnimator.DrawSprite(nullptr, false);
        this->wingAnimator.DrawSprite(nullptr, false);
    }
}

void Aquis::Create(void *data)
{
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];
    this->drawFX |= FX_FLIP;
    this->startPos      = this->position;
    this->startDir      = this->direction;
    this->timer         = 128;
    this->playerInRange = 0;

    if (data) {
        this->active        = ACTIVE_NORMAL;
        this->updateRange.x = 0x200000;
        this->updateRange.y = 0x200000;
        this->mainAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
        this->state.Set(&Aquis::State_Shot);
    }
    else {
        this->active         = ACTIVE_BOUNDS;
        this->updateRange.x  = 0x800000;
        this->updateRange.y  = 0x800000;
        this->remainingTurns = 3;
        this->mainAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->wingAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
        this->state.Set(&Aquis::State_Init);
    }
}

void Aquis::StageLoad()
{
    sVars->aniFrames.Load("OOZ/Aquis.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left   = -10;
    sVars->hitboxBadnik.top    = -16;
    sVars->hitboxBadnik.right  = 10;
    sVars->hitboxBadnik.bottom = 16;

    sVars->hitboxRange.left   = -96;
    sVars->hitboxRange.top    = -64;
    sVars->hitboxRange.right  = 64;
    sVars->hitboxRange.bottom = 128;

    sVars->hitboxProjectile.left   = -3;
    sVars->hitboxProjectile.top    = -3;
    sVars->hitboxProjectile.right  = 3;
    sVars->hitboxProjectile.bottom = 3;

    sVars->sfxShot.Get("Stage/Shot.wav");

    DebugMode::AddObject(sVars->classID, &Aquis::DebugSpawn, &Aquis::DebugDraw);
}

void Aquis::DebugSpawn()
{
    Aquis *aquis       = GameObject::Create<Aquis>(nullptr, this->position.x, this->position.y);
    aquis->direction   = this->direction;
    aquis->startDir    = this->direction;
}

void Aquis::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Aquis::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik))
            player->CheckBadnikBreak(this, true);
    }
}

void Aquis::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position  = this->startPos;
        this->direction = this->startDir;
        Aquis::Create(nullptr);
    }
}

void Aquis::State_Init()
{
    this->active = ACTIVE_NORMAL;
    this->timer  = 32;

    this->state.Set(&Aquis::State_Idle);
    Aquis::State_Idle();
}

void Aquis::State_Idle()
{
    if (!--this->timer) {
        if (this->remainingTurns--) {
            this->velocity.y    = -0x10000;
            this->timer         = 128;
            this->playerInRange = false;
            this->state.Set(&Aquis::State_Moving);
        }
        else {
            this->velocity.y = 0;

            if (this->direction == FLIP_X) {
                this->velocity.x = 0;
                this->state.Set(&Aquis::State_Turning);
                this->mainAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
            }
            else {
                this->velocity.x = -0x20000;
                this->state.Set(&Aquis::State_Flee);
            }
        }
    }

    this->mainAnimator.Process();
    this->wingAnimator.Process();

    Aquis::CheckPlayerCollisions();
    Aquis::CheckOffScreen();
}

void Aquis::State_Moving()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    Player *playerPtr = Player::GetNearestPlayerXY();
    if (playerPtr) {
        bool32 changeDir = false;
        if (playerPtr->position.x >= this->position.x) {
            this->velocity.x += 0x1000;

            if (this->velocity.x > 0x10000)
                this->velocity.x = 0x10000;

            changeDir = this->direction == FLIP_NONE;
        }
        else {
            this->velocity.x -= 0x1000;

            if (this->velocity.x < -0x10000)
                this->velocity.x = -0x10000;

            changeDir = this->direction == FLIP_X;
        }

        if (changeDir) {
            this->state.Set(&Aquis::State_Turning);
            this->mainAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
        }

        if (playerPtr->position.y >= this->position.y) {
            this->velocity.y += 0x1000;

            if (this->velocity.y > 0x10000)
                this->velocity.y = 0x10000;
        }
        else {
            this->velocity.y -= 0x1000;

            if (this->velocity.y < -0x10000)
                this->velocity.y = -0x10000;
        }
    }

    if (!this->state.Matches(&Aquis::State_Turning)) {
        if (!this->timer--) {
            this->timer      = 32;
            this->state.Set(&Aquis::State_Idle);
            this->velocity.x = 0;
            this->velocity.y = 0;
        }

        if (!this->playerInRange) {
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
            {
                if (player->velocity.y < 0 && !player->onGround)
                    continue;

                if (player->CheckCollisionTouch(this, &sVars->hitboxRange)) {
                    this->playerInRange = true;
                    this->timer         = 64;
                    this->mainAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
                    this->state.Set(&Aquis::State_Shoot);

                    if (player->position.x >= this->position.x) {
                        if (player->velocity.x <= (this->position.x - player->position.x) >> 5) {
                            this->mainAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
                            this->state.Set(&Aquis::State_Turning);
                        }
                    }
                    else if (player->velocity.x >= (this->position.x - player->position.x) >> 5) {
                        this->mainAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
                        this->state.Set(&Aquis::State_Turning);
                    }
                }
            }
        }
    }

    this->mainAnimator.Process();
    this->wingAnimator.Process();

    Aquis::CheckPlayerCollisions();
    Aquis::CheckOffScreen();
}

void Aquis::State_Shoot()
{
    if (--this->timer) {
        if (this->timer == 33) {
            Aquis *shot = GameObject::Create<Aquis>(INT_TO_VOID(true), this->position.x, this->position.y);

            if (this->direction) {
                shot->position.x += 0x100000;
                shot->velocity.x = 0x30000;
            }
            else {
                shot->position.x -= 0x100000;
                shot->velocity.x = -0x30000;
            }

            shot->position.y -= 0xA0000;
            shot->velocity.y = 0x20000;
            shot->direction  = this->direction;
            shot->active     = ACTIVE_NORMAL;
            sVars->sfxShot.Play(false, 255);
        }
    }
    else {
        this->timer          = 32;
        this->remainingTurns = 0;
        this->state.Set(&Aquis::State_Idle);
        this->velocity.x     = 0;
        this->velocity.y     = 0;
    }

    this->mainAnimator.Process();
    this->wingAnimator.Process();

    Aquis::CheckPlayerCollisions();
    Aquis::CheckOffScreen();
}

void Aquis::State_Turning()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->mainAnimator.Process();

    if (this->mainAnimator.frameID == this->mainAnimator.frameCount - 1) {
        this->mainAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->direction ^= FLIP_X;

        if (this->remainingTurns < 0) {
            this->velocity.x = -0x20000;

            this->state.Set(&Aquis::State_Flee);
            Aquis::State_Flee();
        }
        else if (this->playerInRange) {
            this->timer = 64;
            this->mainAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

            this->state.Set(&Aquis::State_Shoot);
            Aquis::State_Shoot();
        }
        else {
            this->state.Set(&Aquis::State_Moving);
            Aquis::State_Moving();
        }
    }
    else {
        if (this->timer > 1)
            this->timer--;

        Aquis::CheckPlayerCollisions();
        Aquis::CheckOffScreen();
    }
}

void Aquis::State_Flee()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->wingAnimator.Process();

    Aquis::CheckPlayerCollisions();
    Aquis::CheckOffScreen();
}

void Aquis::State_Shot()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    if (this->CheckOnScreen(nullptr)) {
        this->mainAnimator.Process();

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (player->CheckCollisionTouch(this, &sVars->hitboxProjectile))
                player->ProjectileHurt(this);
        }
    }
    else {
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void Aquis::EditorDraw()
{
    this->mainAnimator.DrawSprite(nullptr, false);
    this->wingAnimator.DrawSprite(nullptr, false);
}

void Aquis::EditorLoad() { sVars->aniFrames.Load("OOZ/Aquis.bin", SCOPE_STAGE); }
#endif

void Aquis::Serialize() {}


} // namespace GameLogic