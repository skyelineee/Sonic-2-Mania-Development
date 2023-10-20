// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Octus Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Octus.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Octus);

void Octus::Update() { this->state.Run(this); }

void Octus::LateUpdate() {}

void Octus::StaticUpdate() {}

void Octus::Draw() { this->animator.DrawSprite(nullptr, false); }

void Octus::Create(void *data)
{
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];
    this->drawFX |= FX_FLIP;
    this->startPos = this->position;
    this->startDir = this->direction;
    this->timer    = 128;
    this->unused1  = 0;
    this->unused2  = 0;

    if (data) {
        this->inkEffect     = INK_ADD;
        this->alpha         = 0xC0;
        this->active        = ACTIVE_NORMAL;
        this->updateRange.x = 0x200000;
        this->updateRange.y = 0x200000;
        this->animator.SetAnimation(sVars->aniFrames, 3, true, 0);
        this->state.Set(&Octus::State_Shot);
    }
    else {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->state.Set(&Octus::State_Init);
        this->alpha = 0xC0;
    }
}

void Octus::StageLoad()
{
    sVars->aniFrames.Load("OOZ/Octus.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left   = -16;
    sVars->hitboxBadnik.top    = -12;
    sVars->hitboxBadnik.right  = 16;
    sVars->hitboxBadnik.bottom = 12;

    sVars->hitboxRange.left   = -128;
    sVars->hitboxRange.top    = -512;
    sVars->hitboxRange.right  = 128;
    sVars->hitboxRange.bottom = 512;

    sVars->hitboxProjectile.left   = -4;
    sVars->hitboxProjectile.top    = -4;
    sVars->hitboxProjectile.right  = 4;
    sVars->hitboxProjectile.bottom = 4;

    sVars->sfxShot.Get("Stage/Shot.wav");

    DebugMode::AddObject(sVars->classID, &Octus::DebugSpawn, &Octus::DebugDraw);
}

void Octus::DebugSpawn()
{
    Octus *octus       = GameObject::Create<Octus>(nullptr, this->position.x, this->position.y);
    octus->direction   = this->direction;
    octus->startDir    = this->direction;
}

void Octus::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Octus::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik))
            player->CheckBadnikBreak(this, true);
    }
}

void Octus::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position  = this->startPos;
        this->direction = this->startDir;
        Octus::Create(nullptr);
    }
}

void Octus::State_Init()
{
    this->active = ACTIVE_NORMAL;

    this->state.Set(&Octus::State_CheckPlayerInRange);
    Octus::State_CheckPlayerInRange();
}

void Octus::State_CheckPlayerInRange()
{
    this->animator.Process();

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->CheckCollisionTouch(this, &sVars->hitboxRange)) {
            this->timer = 32;
            this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);
            this->state.Set(&Octus::State_JumpDelay);
            break;
        }
    }

    Octus::CheckPlayerCollisions();
    Octus::CheckOffScreen();
}

void Octus::State_JumpDelay()
{
    this->animator.Process();

    if (--this->timer <= 0) {
        this->velocity.y = -0x20000;
        this->state.Set(&Octus::State_Jump);
    }

    Octus::CheckPlayerCollisions();
    Octus::CheckOffScreen();
}

void Octus::State_Jump()
{
    this->animator.Process();

    this->position.y += this->velocity.y;
    this->velocity.y += 0x1000;

    if (this->velocity.y >= 0) {
        this->timer = 60;
        this->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
        this->state.Set(&Octus::State_Shoot);
    }

    Octus::CheckPlayerCollisions();
    Octus::CheckOffScreen();
}

void Octus::State_Shoot()
{
    this->animator.Process();

    if (--this->timer == 51) {
        Octus *shot = GameObject::Create<Octus>(INT_TO_VOID(true), this->position.x, this->position.y);
        if (this->direction) {
            shot->position.x += 0xE0000;
            shot->velocity.x = 0x20000;
        }
        else {
            shot->position.x -= 0xE0000;
            shot->velocity.x = -0x20000;
        }

        sVars->sfxShot.Play(false, 255);
    }
    else if (this->timer <= 0) {
        this->state.Set(&Octus::State_Fall);
    }

    Octus::CheckPlayerCollisions();
    Octus::CheckOffScreen();
}

void Octus::State_Fall()
{
    this->animator.Process();

    this->position.y += this->velocity.y;
    this->velocity.y += 0x1000;

    if (this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0xD0000, 1)) {
        this->velocity.y = 0;
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->state.Set(&Octus::State_CheckPlayerInRange);
    }

    Octus::CheckPlayerCollisions();
    Octus::CheckOffScreen();
}

void Octus::State_Shot()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    if (this->CheckOnScreen(nullptr)) {
        this->animator.Process();
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
void Octus::EditorDraw() { Octus::Draw(); }

void Octus::EditorLoad()
{
    sVars->aniFrames.Load("OOZ/Octus.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left", FLIP_NONE);
    RSDK_ENUM_VAR("Right", FLIP_X);
}
#endif

void Octus::Serialize() { RSDK_EDITABLE_VAR(Octus, VAR_UINT8, direction); }
} // namespace GameLogic