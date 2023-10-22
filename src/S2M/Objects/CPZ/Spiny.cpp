// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Spiny Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Spiny.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Spiny);

void Spiny::Update() { this->state.Run(this); }

void Spiny::LateUpdate() {}

void Spiny::StaticUpdate() {}

void Spiny::Draw() { this->animator.DrawSprite(nullptr, false); }

void Spiny::Create(void *data)
{
    this->visible       = true;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;

    if (data) {
        this->drawGroup = Zone::sVars->objectDrawGroup[0] - 1;
        this->animator.SetAnimation(sVars->aniFrames, 4, true, 0);
        this->state.Set(&Spiny::State_Shot);
    }
    else {
        this->startPos = this->position;
        this->startDir = this->direction;
        this->drawFX   = FX_FLIP;

        if (this->type == SPINY_FLOOR) {
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->state.Set(&Spiny::State_Init);

            if (!(this->direction & FLIP_X))
                this->velocity.x = -0x4000;
            else
                this->velocity.x = 0x4000;
        }
        else {
            this->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
            this->state.Set(&Spiny::State_Init);

            if (!(this->direction & FLIP_Y))
                this->velocity.y = -0x4000;
            else
                this->velocity.y = 0x4000;
        }
    }
}

void Spiny::StageLoad()
{
    sVars->aniFrames.Load("CPZ/Spiny.bin", SCOPE_STAGE);

    sVars->hitboxSpiny.left   = -12;
    sVars->hitboxSpiny.top    = -15;
    sVars->hitboxSpiny.right  = 12;
    sVars->hitboxSpiny.bottom = 15;

    sVars->hitboxRange.left   = -96;
    sVars->hitboxRange.top    = -256;
    sVars->hitboxRange.right  = 96;
    sVars->hitboxRange.bottom = 256;

    sVars->hitboxShot.left   = -4;
    sVars->hitboxShot.top    = -4;
    sVars->hitboxShot.right  = 4;
    sVars->hitboxShot.bottom = 4;

    DebugMode::AddObject(sVars->classID, &Spiny::DebugSpawn, &Spiny::DebugDraw);

    sVars->sfxShot.Get("Stage/Shot.wav");
}

void Spiny::DebugSpawn()
{
    Spiny *spiny = GameObject::Create<Spiny>(nullptr, this->position.x, this->position.y);
    spiny->direction   = this->direction;
}

void Spiny::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Spiny::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxSpiny))
            player->CheckBadnikBreak(this, true);
    }
}

void Spiny::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position       = this->startPos;
        this->dirChangeTimer = 0;
        this->moveTimer      = 128;
        this->direction      = this->startDir;
        Spiny::Create(nullptr);
    }
}

void Spiny::State_Init()
{
    this->active = ACTIVE_NORMAL;
    if (this->type == SPINY_FLOOR) {
        this->state.Set(&Spiny::State_Floor);
        Spiny::State_Floor();
    }
    else {
        this->state.Set(&Spiny::State_Wall);
        Spiny::State_Wall();
    }
}

void Spiny::State_Floor()
{
    this->animator.Process();

    if (++this->dirChangeTimer >= 256) {
        this->dirChangeTimer = 0;
        this->direction ^= FLIP_X;
        this->moveTimer  = 128;
        this->velocity.x = -this->velocity.x;
    }

    int32 distance = 0x7FFFFFFF;
    this->position.x += this->velocity.x;

    if (!this->moveTimer) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (player->CheckCollisionTouch(this, &sVars->hitboxRange)) {
                this->moveTimer = 40;
                this->state.Set(&Spiny::State_Shoot_Floor);
                this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);

                if (abs(player->position.x - this->position.x) < distance) {
                    distance = abs(player->position.x - this->position.x);
                    if (player->position.x > this->position.x)
                        this->shotVelocity = 0x10000;
                    else
                        this->shotVelocity = -0x10000;
                }
            }
        }
    }
    else {
        this->moveTimer--;
    }

    Spiny::CheckPlayerCollisions();
    Spiny::CheckOffScreen();
}

void Spiny::State_Shoot_Floor()
{
    this->animator.Process();

    if (--this->moveTimer == 20) {
        sVars->sfxShot.Play(false, 0xFF);

        Spiny *shot = GameObject::Create<Spiny>(INT_TO_VOID(true), this->position.x, this->position.y);
        shot->velocity.x  = this->shotVelocity;
        if (!(this->direction & 2))
            shot->velocity.y = -0x30000;
        else
            shot->velocity.y = 0x20000;
    }
    else {
        if (this->moveTimer < 0) {
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->moveTimer = 64;
            this->state.Set(&Spiny::State_Floor);
        }
    }

    Spiny::CheckPlayerCollisions();
    Spiny::CheckOffScreen();
}

void Spiny::State_Wall()
{
    this->animator.Process();

    if (++this->dirChangeTimer >= 256) {
        this->dirChangeTimer = 0;
        this->direction ^= FLIP_Y;
        this->moveTimer  = 128;
        this->velocity.y = -this->velocity.y;
    }

    this->position.y += this->velocity.y;

    if (!this->moveTimer) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (player->CheckCollisionTouch(this, &sVars->hitboxRange)) {
                this->moveTimer = 40;
                this->state.Set(&Spiny::State_Shoot_Wall);
                this->animator.SetAnimation(sVars->aniFrames, 3, true, 0);

                if (!(this->direction & 1))
                    this->shotVelocity = -0x30000;
                else
                    this->shotVelocity = 0x30000;
            }
        }
    }
    else {
        this->moveTimer--;
    }

    Spiny::CheckPlayerCollisions();
    Spiny::CheckOffScreen();
}

void Spiny::State_Shoot_Wall()
{
    this->animator.Process();

    if (--this->moveTimer == 20) {
        sVars->sfxShot.Play(false, 0xFF);
        GameObject::Create<Spiny>(INT_TO_VOID(true), this->position.x, this->position.y)->velocity.x = this->shotVelocity;
    }
    else {
        if (this->moveTimer < 0) {
            this->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
            this->moveTimer = 64;
            this->state.Set(&Spiny::State_Wall);
        }
    }

    Spiny::CheckPlayerCollisions();
    Spiny::CheckOffScreen();
}

void Spiny::State_Shot()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x2000;

    if (this->CheckOnScreen(&this->updateRange)) {
        this->animator.Process();

        if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0, true)) {
            this->inkEffect |= INK_ADD;
            this->alpha = 0x100;
            this->state.Set(&Spiny::State_ShotDisappear);
        }

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (player->CheckCollisionTouch(this, &sVars->hitboxShot))
                player->ProjectileHurt(this);
        }
    }
    else {
        this->Destroy();
    }
}

void Spiny::State_ShotDisappear()
{
    this->animator.Process();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->alpha -= 0x20;
    if (this->alpha <= 0)
        this->Destroy();
}

#if RETRO_INCLUDE_EDITOR
void Spiny::EditorDraw()
{
    this->drawFX = FX_FLIP;

    if (this->type == SPINY_FLOOR)
        this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
    else
        this->animator.SetAnimation(sVars->aniFrames, 2, false, 0);

    Spiny::Draw();
}

void Spiny::EditorLoad()
{
    sVars->aniFrames.Load("CPZ/Spiny.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Floor", SPINY_FLOOR);
    RSDK_ENUM_VAR("Wall", SPINY_WALL);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip", FLIP_NONE);
    RSDK_ENUM_VAR("Flip X", FLIP_X);
    RSDK_ENUM_VAR("Flip Y", FLIP_Y);
    RSDK_ENUM_VAR("Flip XY", FLIP_XY);
}
#endif

void Spiny::Serialize()
{
    RSDK_EDITABLE_VAR(Spiny, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(Spiny, VAR_UINT8, direction);
}

} // namespace GameLogic