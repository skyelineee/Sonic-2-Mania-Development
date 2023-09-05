// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Stegway Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Stegway.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"
#include "Global/Dust.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Stegway);

void Stegway::Update()
{
    this->mainAnimator.Process();
    this->wheelAnimator.Process();
    if (this->showJet)
        this->jetAnimator.Process();

    this->state.Run(this);

    Stegway::HandlePlayerInteractions();
    if (!this->state.Matches(&Stegway::State_Init))
        Stegway::CheckOffScreen();
}

void Stegway::LateUpdate() {}

void Stegway::StaticUpdate() {}

void Stegway::Draw()
{
    this->mainAnimator.DrawSprite(nullptr, false);
    this->wheelAnimator.DrawSprite(nullptr, false);
    if (this->showJet)
        this->jetAnimator.DrawSprite(nullptr, false);
}

void Stegway::Create(void *data)
{
    this->visible       = true;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->startPos      = this->position;
    this->startDir      = this->direction;
    this->drawFX        = FX_FLIP;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->state.Set(&Stegway::State_Init);
}

void Stegway::StageLoad()
{
    sVars->aniFrames.Load("HPZ/Stegway.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left   = -20;
    sVars->hitboxBadnik.top    = -14;
    sVars->hitboxBadnik.right  = 20;
    sVars->hitboxBadnik.bottom = 14;

    sVars->hitboxRange.left   = -96;
    sVars->hitboxRange.top    = -32;
    sVars->hitboxRange.right  = 0;
    sVars->hitboxRange.bottom = 14;

    sVars->sfxRev.Get("Stage/Rev.wav");
    sVars->sfxRelease.Get("Global/Release.wav");

    DebugMode::AddObject(sVars->classID, &Stegway::DebugSpawn, &Stegway::DebugDraw);
}

void Stegway::DebugSpawn()
{
    Stegway *stegway = GameObject::Create<Stegway>(nullptr, this->position.x, this->position.y);
    stegway->direction     = this->direction;
    stegway->startDir      = this->direction;
}

void Stegway::DebugDraw(void)
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Stegway::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->direction = this->startDir;
        this->position  = this->startPos;
        Stegway::Create(nullptr);
    }
}

void Stegway::HandlePlayerInteractions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik))
            player->CheckBadnikBreak(this, true);
    }
}

void Stegway::SetupAnims(char type, bool32 force)
{
    switch (type) {
        case 0:
            if (force || this->mainAnimator.animationID)
                this->mainAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->mainAnimator.speed  = 1;
            this->wheelAnimator.speed = 6;
            break;

        case 1:
            if (force || this->mainAnimator.animationID != 1)
                this->mainAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
            this->wheelAnimator.speed = 16;
            break;

        case 2:
            if (force || this->mainAnimator.animationID)
                this->mainAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
            if (this->mainAnimator.frameID == 2)
                this->mainAnimator.frameID = 1;
            this->mainAnimator.speed  = 0;
            this->wheelAnimator.speed = 24;
            break;

        case 3:
            if (force || this->mainAnimator.animationID)
                this->mainAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
            if (this->mainAnimator.frameID == 2)
                this->mainAnimator.frameID = 1;
            this->mainAnimator.speed  = 0;
            this->wheelAnimator.speed = 0;
            break;
        default: break;
    }
}

void Stegway::State_Init()
{
    this->active = ACTIVE_NORMAL;
    if (this->direction == FLIP_NONE)
        this->velocity.x = -0x4000;
    else
        this->velocity.x = 0x4000;
    this->wheelAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    this->jetAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
    Stegway::SetupAnims(0, true);
    this->state.Set(&Stegway::State_Moving);
    Stegway::State_Moving();
}

void Stegway::State_Moving()
{
    this->position.x += this->velocity.x;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->CheckCollisionTouch(this, &sVars->hitboxRange)) {
            this->state.Set(&Stegway::State_RevUp);
            Stegway::SetupAnims(3, false);
        }
    }

    bool32 collided = false;
    if (this->direction)
        collided = this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0xC0000, 0x100000, 8);
    else           
        collided = this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, -0xC0000, 0x100000, 8);

    if (!collided) {
        this->state.Set(&Stegway::State_Turn);
        Stegway::SetupAnims(3, false);
        this->showJet = false;
        this->noFloor = false;
    }
    Stegway::CheckOffScreen();
}

void Stegway::State_Turn()
{
    if (this->timer >= 29) {
        this->timer = 0;
        this->state.Set(&Stegway::State_Moving);
        Stegway::SetupAnims(0, false);

        this->direction ^= FLIP_X;
        if (this->direction == FLIP_NONE)
            this->velocity.x = -0x4000;
        else
            this->velocity.x = 0x4000;
    }
    else {
        this->timer++;
    }
}

void Stegway::State_RevUp()
{
    if (++this->timer == 8) {
        this->timer = 0;
        this->state.Set(&Stegway::State_RevRelease);
        Stegway::SetupAnims(2, false);
        sVars->sfxRev.Play(false, 255);
    }
}

void Stegway::State_RevRelease()
{
    if (++this->timer == 32) {
        this->timer = 0;
        this->state.Set(&Stegway::State_Dash);
        Dust *dust = GameObject::Create<Dust>(this, this->position.x - 0xA0000 * (2 * (this->direction != FLIP_NONE) - 1), this->position.y + 0x100000);
        dust->animator.SetAnimation(Dust::sVars->aniFrames, 2, true, 0);
        dust->state.Set(&Dust::State_DustPuff);
        dust->direction = FLIP_X - this->direction;
        dust->drawGroup = this->drawGroup;
        Stegway::SetupAnims(1, false);
        this->showJet = true;
        this->velocity.x *= 12;
        sVars->sfxRelease.Play(false, 255);
    }
}

void Stegway::State_Dash()
{
    this->position.x += this->velocity.x;
    int32 dir = 2 * (this->direction != FLIP_NONE) - 1;

    if (!this->noFloor) {
        int32 storeX = this->position.x;
        int32 storeY = this->position.y;
        if (!this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, dir << 22, 0x100000, 8))
            this->noFloor = true;
        this->position.x = storeX;
        this->position.y = storeY;
    }

    bool32 collided = this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0xC0000 * dir, 0x100000, 8);

    if (this->noFloor) {
        if (this->velocity.x * dir >= 0x4000) {
            this->velocity.x -= 0xA00 * dir;
            if (dir * this->velocity.x < 0x4000) {
                this->velocity.x = dir << 14;
                this->state.Set(&Stegway::State_Moving);
                Stegway::SetupAnims(0, false);
                this->mainAnimator.frameID = 0;
                this->showJet              = false;
                this->noFloor              = false;
            }
        }
    }

    if (!collided) {
        this->state.Set(&Stegway::State_Turn);
        Stegway::SetupAnims(3, false);
        this->showJet = false;
        this->noFloor = false;
    }
}

#if RETRO_INCLUDE_EDITOR
void Stegway::EditorDraw()
{
    this->wheelAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    this->jetAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
    Stegway::SetupAnims(0, true);

    Stegway::Draw();
}

void Stegway::EditorLoad()
{
    sVars->aniFrames.Load("HPZ/Stegway.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip");
    RSDK_ENUM_VAR("Flip X");
}
#endif

void Stegway::Serialize() { RSDK_EDITABLE_VAR(Stegway, VAR_UINT8, direction); }

} // namespace GameLogic