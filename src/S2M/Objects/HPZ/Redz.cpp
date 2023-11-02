// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Redz Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Redz.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Redz);

void Redz::Update()
{
    this->animator.Process();

    this->state.Run(this);

    if (!this->state.Matches(&Redz::State_Init) && !this->state.Matches(&Redz::Flame_State) && !this->state.Matches(&Redz::Flame_Setup)) {
        Redz::CheckPlayerCollisions();
        if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
            this->direction  = this->startDir;
            this->position.x = this->startPos.x;
            this->position.y = this->startPos.y;
            Redz::Create(nullptr);
        }
    }
}

void Redz::LateUpdate() {}

void Redz::StaticUpdate() {}

void Redz::Draw()
{
    this->animator.DrawSprite(nullptr, false);
}

void Redz::Create(void *data)
{
    this->visible       = true;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->startPos      = this->position;
    this->startDir      = this->direction;
    this->drawFX        = FX_FLIP;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->state.Set(&Redz::State_Init);
}

void Redz::StageLoad()
{
    sVars->aniFrames.Load("HPZ/Redz.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left   = -16;
    sVars->hitboxBadnik.top    = -16;
    sVars->hitboxBadnik.right  = 16;
    sVars->hitboxBadnik.bottom = 16;

    sVars->attackbox.left   = -64;
    sVars->attackbox.top    = -96;
    sVars->attackbox.right  = 0;
    sVars->attackbox.bottom = -8;

    sVars->hitboxFlame.left   = -7;
    sVars->hitboxFlame.top    = -7;
    sVars->hitboxFlame.right  = 7;
    sVars->hitboxFlame.bottom = -7;

    sVars->hitboxRange.left   = 0;
    sVars->hitboxRange.top    = 0;
    sVars->hitboxRange.right  = 0;
    sVars->hitboxRange.bottom = 0;

    sVars->sfxFlame.Get("Stage/Flame.wav");

    DebugMode::AddObject(sVars->classID, &Redz::DebugSpawn, &Redz::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Redz::DebugSpawn()
{
    Redz *redz = GameObject::Create<Redz>(nullptr, this->position.x, this->position.y);
    redz->direction  = this->direction;
    redz->startDir   = this->direction;
}

void Redz::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Redz::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik))
            player->CheckBadnikBreak(this, true);
    }
}

void Redz::State_Init()
{
    this->active = ACTIVE_NORMAL;
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->animator.frameID = 0;
    this->animator.speed   = 1;
    this->state.Set(&Redz::State_Walk);
    this->velocity.x       = (2 * (this->direction != FLIP_NONE) - 1) << 15;
    Redz::State_Walk();
}

void Redz::State_Walk()
{
    this->position.x += this->velocity.x;
    if (!this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, (2 * (this->direction != FLIP_NONE) - 1) << 19, 0x100000, 8)) {
        this->state.Set(&Redz::State_Turn);
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->animator.frameID = 0;
        this->animator.speed   = 0;
    }

    if (this->attackDelay <= 0) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
        {
            if (player->CheckCollisionTouchBox(&sVars->hitboxRange, this, &sVars->attackbox)) {
                this->state.Set(&Redz::State_PrepareAttack);
                this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
                this->animator.frameID = 0;
                this->attackDelay      = 60;
                this->animator.speed   = 0;
            }
        }
    }
    else {
        this->attackDelay--;
    }
}

void Redz::State_Turn()
{
    if (this->timer < 59) {
        this->timer++;
    }
    else {
        this->timer = 0;
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->animator.frameID = 0;
        this->animator.speed   = 1;
        this->direction        = this->direction == FLIP_NONE;
        this->velocity.x       = -this->velocity.x;
        this->state.Set(&Redz::State_Walk);
    }
}

void Redz::State_PrepareAttack()
{
    if (this->timer >= 30) {
        this->state.Set(&Redz::State_Attack);
        this->timer = 0;
        this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);
        sVars->sfxFlame.Play(false, 255);
    }
    else {
        this->timer++;
    }
}

void Redz::State_Attack()
{
    if (this->timer >= 90) {
        this->timer = 0;
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->animator.frameID = 0;
        this->animator.speed   = 1;
        this->state.Set(&Redz::State_Walk);
    }

    if (!(this->timer & 3)) {
        Redz *flame = GameObject::Create<Redz>(this, this->position.x, this->position.y);
        flame->state.Set(&Redz::Flame_Setup);
        flame->position.y -= 0x40000;
        flame->position.x += (2 * (this->direction != FLIP_NONE) - 1) << 19;
        flame->velocity.x =
            (2 * (this->direction != FLIP_NONE) - 1) * (Math::Cos512(((Math::Sin512(8 * (this->timer & 0x3F)) >> 5) - 48) & 0x1FF) << 8);
        flame->velocity.y = Math::Sin512(((Math::Sin512(8 * (this->timer & 0x3F)) >> 5) - 48) & 0x1FF) << 8;
    }
    ++this->timer;
}

void Redz::Flame_Setup()
{
    this->active = ACTIVE_NORMAL;
    this->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
    this->animator.frameID = 0;
    this->state.Set(&Redz::Flame_State);
    this->animator.speed   = 1;
    Redz::Flame_State();
}

void Redz::Flame_State()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->CheckCollisionTouch(this, &sVars->hitboxFlame)) {
            if (player->shield != Player::Shield_Fire) {
                player->Hurt(this);
                // player->ElementHurt(this, Player::Shield_Fire);
            }
        }
    }

    if (++this->timer > 20)
        this->Destroy();
}

#if RETRO_INCLUDE_EDITOR
void Redz::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    Redz::Draw();
}

void Redz::EditorLoad()
{
    sVars->aniFrames.Load("HPZ/Redz.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip");
    RSDK_ENUM_VAR("Flip X");
}
#endif

void Redz::Serialize() { RSDK_EDITABLE_VAR(Redz, VAR_UINT8, direction); }

} // namespace GameLogic