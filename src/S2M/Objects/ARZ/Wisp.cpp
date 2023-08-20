// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Wisp Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Wisp.hpp"
#include "Global/Zone.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Wisp);

void Wisp::Update() { this->state.Run(this); }

void Wisp::LateUpdate() {}

void Wisp::StaticUpdate() {}

void Wisp::Draw()
{
    this->bodyAnimator.DrawSprite(nullptr, false);

    this->inkEffect = INK_ALPHA;
    this->wingAnimator.DrawSprite(nullptr, false);

    this->inkEffect = INK_NONE;
}

void Wisp::Create(void *data)
{
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];
    this->drawFX |= FX_FLIP;
    this->startPos      = this->position;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->direction     = FLIP_NONE;
    this->target        = NULL;
    this->alpha         = 0xC0;
    this->timer         = 16;
    this->buzzCount     = 4;
    this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->wingAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

    this->state.Set(&Wisp::State_Init);
}

void Wisp::StageLoad()
{
    sVars->aniFrames.Load("ARZ/Wisp.bin", SCOPE_STAGE);
    
    sVars->hitboxBadnik.left   = -8;
    sVars->hitboxBadnik.top    = -8;
    sVars->hitboxBadnik.right  = 8;
    sVars->hitboxBadnik.bottom = 8;

    DebugMode::AddObject(sVars->classID, &Wisp::DebugSpawn, &Wisp::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Wisp::DebugSpawn() { GameObject::Create<Wisp>(nullptr, this->position.x, this->position.y); }

void Wisp::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Wisp::HandlePlayerInteractions()
{;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        Player *target = this->target;
        if (target) {
            if (abs(player->position.x - this->position.x) < abs(target->position.x - this->position.x))
                this->target = player;
        }
        else {
            this->target = player;
        }

        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik))
            player->CheckBadnikBreak(this, true);
    }
}

void Wisp::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position.x = this->startPos.x;
        this->position.y = this->startPos.y;
        Wisp::Create(NULL);
    }
}

void Wisp::State_Init()
{
    this->active     = ACTIVE_NORMAL;
    this->velocity.x = 0;
    this->velocity.y = 0;

    this->state.Set(&Wisp::State_Idle);
    Wisp::State_Idle();
}

void Wisp::State_Idle()
{
    if (!--this->timer) {
        if (--this->buzzCount) {
            this->velocity.y = -0x10000;
            this->timer      = 96;
            this->state.Set(&Wisp::State_FlyTowardTarget);
        }
        else {
            this->velocity.x = -0x20000;
            this->velocity.y = -0x20000;
            this->state.Set(&Wisp::State_FlyAway);
        }
    }

    this->wingAnimator.Process();

    Wisp::HandlePlayerInteractions();
    Wisp::CheckOffScreen();
}

void Wisp::State_FlyTowardTarget()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    Player *target = this->target;
    if (target) {
        if (target->position.x >= this->position.x) {
            this->velocity.x += 0x1000;
            this->direction = FLIP_X;
            if (this->velocity.x > 0x20000)
                this->velocity.x = 0x20000;
        }
        else {
            this->velocity.x -= 0x1000;
            this->direction = FLIP_NONE;
            if (this->velocity.x < -0x20000)
                this->velocity.x = -0x20000;
        }

        if (target->position.y >= this->position.y) {
            this->velocity.y += 0x1000;
            if (this->velocity.y > 0x20000)
                this->velocity.y = 0x20000;
        }
        else {
            this->velocity.y -= 0x1000;
            if (this->velocity.y < -0x20000)
                this->velocity.y = -0x20000;
        }
    }

    if (!--this->timer) {
        this->timer      = Math::Rand(0, 32);
        this->state.Set(&Wisp::State_Idle);
        this->velocity.x = 0;
        this->velocity.y = 0;
        this->direction  = FLIP_NONE;
    }
    this->wingAnimator.Process();

    Wisp::HandlePlayerInteractions();
    Wisp::CheckOffScreen();
}

void Wisp::State_FlyAway()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->wingAnimator.Process();

    Wisp::HandlePlayerInteractions();
    Wisp::CheckOffScreen();
}

#if RETRO_INCLUDE_EDITOR
void Wisp::EditorDraw() { Wisp::Draw(); }

void Wisp::EditorLoad()
{
    sVars->aniFrames.Load("ARZ/Wisp.bin", SCOPE_STAGE);

    // despite being editable, this variable is set to FLIP_NONE on create, thereby making the editable var's value unused
    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("(Unused)", FLIP_NONE);
    // RSDK_ENUM_VAR("No Flip", FLIP_NONE);
    // RSDK_ENUM_VAR("Flip X", FLIP_X);
}
#endif

void Wisp::Serialize() { RSDK_EDITABLE_VAR(Wisp, VAR_UINT8, direction); }

} // namespace GameLogic