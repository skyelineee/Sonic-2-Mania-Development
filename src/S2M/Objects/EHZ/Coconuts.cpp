// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Coconuts Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "Coconuts.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Player.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Coconuts);

void Coconuts::Update()
{
    this->state.Run(this);
    this->animator.Process();
}

void Coconuts::LateUpdate() {}

void Coconuts::StaticUpdate() {}

void Coconuts::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Coconuts);

    int32 throwDelays[] = { 32, 24, 16, 40, 32, 16 };

    memcpy(sVars->throwDelays, throwDelays, sizeof(throwDelays));
}

void Coconuts::Draw()
{
    this->animator.DrawSprite(NULL, false);
}

void Coconuts::Create(void* data)
{
    this->visible = true;
    this->drawFX |= FX_FLIP;
    this->drawGroup  = Zone::sVars->objectDrawGroup[0];
    this->startPos.x = this->position.x;
    this->startPos.y = this->position.y;
    this->startDir   = this->direction;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->timer      = 16;
    this->coconut    = NULL;

    if (data) {
        this->active = ACTIVE_NORMAL;
        this->animator.SetAnimation(&sVars->aniFrames, Coconut, false, 0);
        this->state.Set(&Coconuts::State_Coconut);
    }
    else {
        this->active = ACTIVE_BOUNDS;
        this->animator.SetAnimation(sVars->aniFrames, Idle, true, 0);
        this->state.Set(&Coconuts::State_Init);
    }
}

void Coconuts::StageLoad()
{
    if (Stage::CheckSceneFolder("EHZ")) {
        sVars->aniFrames.Load("EHZ/Coconuts.bin", SCOPE_STAGE);
    }

    sVars->hitboxBadnik.left   = -8;
    sVars->hitboxBadnik.top    = -16;
    sVars->hitboxBadnik.right  = 16;
    sVars->hitboxBadnik.bottom = 16;

    sVars->hitboxCoconut.left   = -8;
    sVars->hitboxCoconut.top    = -8;
    sVars->hitboxCoconut.right  = 8;
    sVars->hitboxCoconut.bottom = 8;

    sVars->sfxDrop.Get("Stage/Drop.wav");

    DebugMode::AddObject(sVars->classID, &Coconuts::DebugSpawn, &Coconuts::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Coconuts::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, Idle, false, 0);
    DebugMode::sVars->animator.DrawSprite(NULL, false);
}

void Coconuts::DebugSpawn()
{
    Coconuts *coconuts    = GameObject::Create<Coconuts>(nullptr, this->position.x, this->position.y);
    coconuts->direction = this->direction;
    coconuts->startDir  = this->startDir;
}

void Coconuts::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik)) {
            if (player->CheckBadnikBreak(this, true)) {
                if (this->coconut) {
                    coconut->Destroy();
                }
            }
        }
    }
}

void Coconuts::State_Init()
{
    if (this->CheckOnScreen(&this->updateRange)) {
        Vector2 storePos = this->position;
        this->position = this->startPos;

        if (this->CheckOnScreen(&this->updateRange)) {
            this->targetDelay = 0;
            this->timer       = 16;
            this->state.Set(&Coconuts::State_AwaitPlayer);
            this->active = ACTIVE_BOUNDS;
        }
        else {
            this->position = storePos;
        }
    }
}

void Coconuts::State_AwaitPlayer()
{
    this->active         = ACTIVE_NORMAL;
    this->targetDistance = 0x7FFFFFFF;
    this->targetPlayer   = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 dir = FLIP_NONE;
        int32 targetPos = player->position.x - this->position.x;
        if (targetPos < 0) {
            dir = FLIP_NONE;
            targetPos = -targetPos;
        }
        else {
            dir = FLIP_X;
        }
        if (targetPos < this->targetDistance) {
            this->targetDistance = targetPos;
            this->targetPlayer   = player;
            this->direction      = dir;
        }
    }

    bool32 throwing = false;
    if (this->targetDelay == 0) {
        if (this->targetDistance > -0x600000) {
            if (this->targetDistance < 0x600000) {
                this->timer = 8;
                this->targetDelay = 32;
                this->animator.SetAnimation(&sVars->aniFrames, Throw, false, 0);
                this->state.Set(&Coconuts::State_Throwing);
                throwing = true;
            }
        }
    }
    else {
        this->targetDelay--;
    }

    if (throwing == false) {
        this->timer--;
        if (this->timer < 0) {
            int32 dir = GET_BIT(this->throwID, 0);
            if (dir == FLIP_NONE) {
                this->velocity.y = -0x10000;
            }
            else {
                this->velocity.y = 0x10000;
            }

            this->timer = this->throwID[sVars->throwDelays];

            this->throwID++;
            if (this->throwID > 5) {
                this->throwID = 0;
            }

            this->state.Set(&Coconuts::State_Moving);
        }
    }
    Coconuts::CheckPlayerCollisions();
}

void Coconuts::State_Moving()
{
    this->animator.SetAnimation(&sVars->aniFrames, Moving, false, 0);
    this->position.y += this->velocity.y;

    this->timer--;
    if (this->timer < 0) {
        this->timer = 16;
        this->animator.SetAnimation(&sVars->aniFrames, Idle, false, 0);
        this->state.Set(&Coconuts::State_AwaitPlayer);
    }
    Coconuts::CheckPlayerCollisions();
}

void Coconuts::State_Throwing()
{
    this->timer--;
    if (this->timer < 0) {
        Coconuts *coconut = GameObject::Create<Coconuts>(INT_TO_VOID(true), this->position.x, this->position.y);
        sVars->sfxDrop.Play();
        if (this->direction == FLIP_NONE) {
            coconut->position.x += 0xB0000;
            coconut->velocity.x = -0x10000;
        }
        else {
            coconut->position.x -= 0xB0000;
            coconut->velocity.x = 0x10000;
        }
        coconut->position.y -= 0xD0000;
        coconut->velocity.y = -0x10000;
        coconut->direction  = this->direction;
        coconut->active     = ACTIVE_NORMAL;
        this->state.Set(&Coconuts::State_HasThrown);
    }
    
    Coconuts::CheckPlayerCollisions();
}

void Coconuts::State_HasThrown()
{
    this->timer--;
    if (this->timer < 0) {
        int32 dir = GET_BIT(this->throwID, 0);
        if (dir == 0) {
            this->velocity.y = -0x10000;
        }
        else {
            this->velocity.y = 0x10000;
        }

        this->timer = this->throwID[sVars->throwDelays];
        this->throwID++;
        if (this->throwID > 5) {
            this->throwID = 0;
        }

        this->animator.SetAnimation(&sVars->aniFrames, Moving, false, 0);
        this->state.Set(&Coconuts::State_Moving);
    }
    Coconuts::CheckPlayerCollisions();
}

void Coconuts::State_Coconut()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x2000;

    if (this->CheckOnScreen(&this->updateRange)) {
        this->animator.Process();

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (player->CheckCollisionTouch(this, &sVars->hitboxCoconut)) {
                player->ProjectileHurt(this);
            }
        }
    }
    else {
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void Coconuts::EditorDraw()
{
    this->animator.SetAnimation(&sVars->aniFrames, Idle, false, 0);

}

void Coconuts::EditorLoad()
{
    if (Stage::CheckSceneFolder("EHZ"))
        sVars->aniFrames.Load("EHZ/Coconuts.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Right");
}
#endif

void Coconuts::Serialize() {}

} // namespace GameLogic