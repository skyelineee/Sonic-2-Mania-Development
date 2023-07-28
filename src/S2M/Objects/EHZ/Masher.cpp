// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Masher Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "Masher.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Masher);

void Masher::Update()
{
    this->state.Run(this);
    this->animator.Process();
}

void Masher::LateUpdate() {}

void Masher::StaticUpdate() {}

void Masher::Draw() { this->animator.DrawSprite(NULL, false); }

void Masher::Create(void* data)
{
    this->visible    = true;
    this->drawFX    |= FX_FLIP;
    this->active     = ACTIVE_NORMAL;
    this->drawGroup  = Zone::sVars->objectDrawGroup[0];
    this->startPos.x = this->position.x;
    this->startPos.y = this->position.y;
    this->velocity.y = -0x50000;
    this->direction  = FLIP_NONE;
    this->state.Set(&Masher::State_Jumping);
}

void Masher::StageLoad()
{
    if (Stage::CheckSceneFolder("EHZ")) {
        sVars->aniFrames.Load("EHZ/Masher.bin", SCOPE_STAGE);
    }

    sVars->hitboxBadnik.left   = -12;
    sVars->hitboxBadnik.top    = -14;
    sVars->hitboxBadnik.right  = 12;
    sVars->hitboxBadnik.bottom = 14;

    DebugMode::AddObject(sVars->classID, &Masher::DebugSpawn, &Masher::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Masher::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, Idle, true, 0);
    DebugMode::sVars->animator.DrawSprite(NULL, false);
}

void Masher::DebugSpawn()
{
    Masher *masher  = GameObject::Create<Masher>(nullptr, this->position.x, this->position.y);
}

void Masher::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik)) {
            player->CheckBadnikBreak(this, true);
        }
    }
}

void Masher::State_Jumping()
{
    this->position.y += this->velocity.y;
    this->velocity.y += 0x1800;

    if (this->position.y > this->startPos.y) {
        this->position.y = this->startPos.y;
        this->velocity.y = -0x50000;
    }

    if (this->velocity.y < 0) {
        this->animator.SetAnimation(&sVars->aniFrames, Biting, false, 0);
    }
    else {
        this->animator.SetAnimation(&sVars->aniFrames, Idle, false, 0);
    }
    
    Masher::CheckPlayerCollisions();
}

#if RETRO_INCLUDE_EDITOR
void Masher::EditorDraw()
{
    this->animator.SetAnimation(&sVars->aniFrames, Idle, true, 0);
}

void Masher::EditorLoad()
{
    if (Stage::CheckSceneFolder("EHZ"))
        sVars->aniFrames.Load("EHZ/Masher.bin", SCOPE_STAGE);
}
#endif

void Masher::Serialize() {}

} // namespace GameLogic
