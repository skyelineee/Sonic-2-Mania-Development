// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Splats Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Splats.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Splats);

void Splats::Update() { this->state.Run(this); }

void Splats::LateUpdate() {}

void Splats::StaticUpdate() {}

void Splats::Draw()
{
    this->splashAnimator.DrawSprite(nullptr, false);
    this->mainAnimator.DrawSprite(nullptr, false);
}

void Splats::Create(void *data)
{
    this->startPos      = this->position;
    this->startDir      = this->direction;
    this->visible       = true;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;

    if (this->bounceCount < 1)
        this->bounceCount = 1;

    this->drawFX |= FX_FLIP;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];
    this->mainAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->state.Set(&Splats::State_Init);
}

void Splats::StageLoad()
{
    sVars->aniFrames.Load("SWZ/Splats.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left   = -10;
    sVars->hitboxBadnik.top    = -20;
    sVars->hitboxBadnik.right  = 6;
    sVars->hitboxBadnik.bottom = 20;

    DebugMode::AddObject(sVars->classID, &Splats::DebugSpawn, &Splats::DebugDraw);
}

void Splats::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Splats::DebugSpawn() { GameObject::Create<Splats>(nullptr, this->position.x, this->position.y); }

void Splats::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik)) {
            player->CheckBadnikBreak(this, true);
        }
    }
}

void Splats::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position   = this->startPos;
        this->direction  = this->startDir;
        this->isOnScreen = false;
        Splats::Create(nullptr);
    }
}

void Splats::State_Init()
{
    this->active     = ACTIVE_NORMAL;
    this->velocity.x = -0x10000;

    this->state.Set(&Splats::State_BounceAround);
    this->state.Run(this);
}

void Splats::State_BounceAround()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x3800;

    if (this->velocity.y > 0 && this->TileCollision(Zone::sVars->collisionLayers, 0, 0, 0, 0x100000, true)) {
        if (this->bounceCount && ++this->activeCount >= this->bounceCount) {
            this->activeCount = 0;
            this->direction ^= FLIP_X;
            this->velocity.x = -this->velocity.x;
        }
        this->position.y -= 0x80000;
        this->velocity.y = -0x40000;
    }

    this->mainAnimator.frameID = this->velocity.y < 0;

    Splats::CheckPlayerCollisions();
    Splats::CheckOffScreen();
}

#if RETRO_INCLUDE_EDITOR
void Splats::EditorDraw()
{
    this->drawFX |= FX_FLIP;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];
    this->mainAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    Splats::Draw();
}

void Splats::EditorLoad()
{
    sVars->aniFrames.Load("SWZ/Splats.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left", FLIP_NONE);
    RSDK_ENUM_VAR("Right", FLIP_X);
}
#endif

void Splats::Serialize()
{
    RSDK_EDITABLE_VAR(Splats, VAR_UINT8, bounceCount);
    RSDK_EDITABLE_VAR(Splats, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(Splats, VAR_UINT8, numActive);
    RSDK_EDITABLE_VAR(Splats, VAR_UINT16, minDelay);
}

} // namespace GameLogic