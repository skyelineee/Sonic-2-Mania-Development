// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Buzzer Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "Buzzer.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Helpers/DrawHelpers.hpp"
#include "Global/Player.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Buzzer);

void Buzzer::Update() { 
    this->state.Run(this); 
    this->animator.Process();
}

void Buzzer::LateUpdate() {}

void Buzzer::StaticUpdate() {}

void Buzzer::Draw()
{
    this->animator.DrawSprite(NULL, false);
}

void Buzzer::Create(void *data)
{
    this->visible = true;
    this->drawFX |= FX_FLIP;
    this->drawGroup      = Zone::sVars->objectDrawGroup[0];
    this->startPos.x     = this->position.x;
    this->startPos.y     = this->position.y;
    this->startDir       = this->direction;
    this->timer          = 128;
    this->hasShot        = false;
    this->projectile     = NULL;

    if (!this->shotRange)
        this->shotRange = 32;

    this->hitboxRange.right = -this->shotRange;
    this->hitboxRange.left = -this->shotRange;
    this->hitboxRange.top = -256;
    this->hitboxRange.bottom = 256;

    if (data) {
        this->active = ACTIVE_NORMAL;
        this->updateRange.x = 0x200000;
        this->updateRange.y = 0x200000;
        this->animator.SetAnimation(sVars->aniFrames, 4, true, 0);
        this->state.Set(&Buzzer::State_ProjectileCharge);
    }
    else {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->animator.SetAnimation(sVars->aniFrames, Flying, true, 0);
        this->state.Set(&Buzzer::State_Init);
    }
}

void Buzzer::StageLoad()
{
    sVars->aniFrames.Load("EHZ/Buzzer.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left = -16;
    sVars->hitboxBadnik.top = -12;
    sVars->hitboxBadnik.right  = 16;
    sVars->hitboxBadnik.bottom = 12;

    sVars->hitboxProjectile.left = -6;
    sVars->hitboxProjectile.top   = -6;
    sVars->hitboxProjectile.right  = 6;
    sVars->hitboxProjectile.bottom = 6;

    DebugMode::AddObject(sVars->classID, &Buzzer::DebugSpawn, &Buzzer::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Buzzer::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
    DebugMode::sVars->animator.DrawSprite(NULL, false);
}

void Buzzer::DebugSpawn()
{
    Buzzer *buzzer = GameObject::Create<Buzzer>(nullptr, this->position.x, this->position.y);
    buzzer->direction = this->direction;
    buzzer->startDir  = this->startDir;
}

void Buzzer::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik)) {
            if (player->CheckBadnikBreak(this, true)) {
                if (this->projectile) {
                    projectile->Destroy();
                }
            }
        }
        else if (this->state.Matches(&Buzzer::State_Flying)) {
            if (player->CheckCollisionTouch(this, &this->hitboxRange)) {
                if (this->hasShot == false) {
                    this->shootTimer = 55;
                    this->state.Set(&Buzzer::State_Shooting);
                }
            }
        }
    }
}

void Buzzer::State_Init()
{
    if (this->CheckOnScreen(&this->updateRange)) {
        temp0            = this->position.x;
        this->position.x = this->startPos.x;
        if (this->shootTimer = 1) {
            this->state.Set(&Buzzer::State_Invisible);
        }
        if (this->CheckOnScreen(&this->updateRange)) {
            this->position.x = this->startPos.x;
            if (this->direction & FLIP_X) {
                this->velocity.x = 0x10000;
            }
            else {
                this->velocity.x = -0x10000;
            }
        }
        this->timer = 0;
        this->animator.SetAnimation(&sVars->aniFrames, Flying, false, 0);
        this->active = ACTIVE_BOUNDS;
        this->state.Set(&Buzzer::State_Flying);
    }
    else {
        this->position.x = temp0;
    }
}

void Buzzer::State_Flying()
{
    this->active = ACTIVE_NORMAL;
    if (this->timer < 256) {
        this->timer++;
        this->position.x += this->velocity.x;
    }
    else {
        this->timer = 0;
        this->state.Set(&Buzzer::State_Idle);
        this->animator.SetAnimation(&sVars->aniFrames, Idle, true, 0);
        this->velocity.x = -this->velocity.x;
        this->direction ^= FLIP_X;
    }
    Buzzer::CheckPlayerCollisions();
}

void Buzzer::State_Idle()
{
    if (this->shootTimer < 30) {
        this->shootTimer++;
    }
    else {
        this->shootTimer = 0;
        this->state.Set(&Buzzer::State_Flying);
        this->animator.SetAnimation(&sVars->aniFrames, Flying, false, 0);
        this->hasShot        = false;
    }
    Buzzer::CheckPlayerCollisions();
}

void Buzzer::State_Shooting()
{
    this->shootTimer--;
    if (this->shootTimer == 50) {
        this->animator.SetAnimation(&sVars->aniFrames, Shooting, false, 0);
    }
    else if (this->shootTimer == 30) {
        Buzzer *projectile    = GameObject::Create<Buzzer>(INT_TO_VOID(true), this->position.x, this->position.y);
            if (this->direction) {
                projectile->position.x -= 0xD0000;
                projectile->velocity.x = 0x18000;
            }
            else {
                projectile->position.x += 0xD0000;
                projectile->velocity.x = -0x18000;
            }
            projectile->position.y += 0x180000;
            projectile->velocity.y = 0x18000;
            projectile->groundVel  = 0;
            projectile->direction = this->direction;
            projectile->projectile = (Buzzer *)this;
            projectile->active     = ACTIVE_NORMAL;
            this->projectile       = (Buzzer *)projectile;
    }
    else if (!this->shootTimer) {
            this->shootTimer = 0;
            this->state.Set(&Buzzer::State_Flying);
            this->animator.SetAnimation(&sVars->aniFrames, Flying, false, 0);
            this->hasShot = true;
    }
    Buzzer::CheckPlayerCollisions();
}

void Buzzer::State_Invisible()
{
    this->animator.SetAnimation(&sVars->aniFrames, Invisible, true, 0);
}

void Buzzer::State_ProjectileCharge()
{
    this->animator.Process();

    if (this->animator.frameID == 6) {
        this->state.Set(&Buzzer::State_ProjectileShot);
        Buzzer *shot = (Buzzer *)this->projectile;
        shot->projectile = NULL;
    }
}

void Buzzer::State_ProjectileShot()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    if (this->CheckOnScreen(&this->updateRange)) {
        this->animator.Process();

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (player->CheckCollisionTouch(this, &sVars->hitboxProjectile)) {
                player->ProjectileHurt(this);
            }
        }
    }
    else {
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void Buzzer::EditorDraw(void)
{
    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        this->hitboxRange.right  = -this->shotRange;
        this->hitboxRange.left   = -this->shotRange;
        this->hitboxRange.top    = -256;
        this->hitboxRange.bottom = 256;

        DrawHelpers::DrawHitboxOutline(this->position.x, this->position.y, &this->hitboxRange, FLIP_NONE, 0xFF0000);
        this->animator.SetAnimation(&sVars->aniFrames, Flying, false, 0);

        RSDK_DRAWING_OVERLAY(false);
    }
}

void Buzzer::EditorLoad(void)
{
    sVars->aniFrames.Load("EHZ/Buzzer.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Right");
}
#endif

void Buzzer::Serialize()
{
    RSDK_EDITABLE_VAR(Buzzer, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(Buzzer, VAR_UINT8, shotRange);
}
} // namespace GameLogic