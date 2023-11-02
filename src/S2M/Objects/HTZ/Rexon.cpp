// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Rexon Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Rexon.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"
#include "Global/Explosion.hpp"
#include "Helpers/BadnikHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Rexon);

void Rexon::Update(void)
{
    this->state.Run(this);

    this->positions[REXON_SEGMENT_COUNT].x = this->position.x;
    this->positions[REXON_SEGMENT_COUNT].y = this->position.y;

    if (this->direction)
        this->positions[REXON_SEGMENT_COUNT].x += 0x1C0000;
    else
        this->positions[REXON_SEGMENT_COUNT].x -= 0x1C0000;
    this->positions[REXON_SEGMENT_COUNT].y += 0x110000;

    for (int32 i = REXON_SEGMENT_COUNT - 1; i >= 0; --i) {
        int32 y = this->segmentAmplitude[i] >> 1;
        if (this->direction == FLIP_X)
            y = (0x80 - y) & 0xFF;

        this->positions[i].x = this->segmentMagnitude[i] * Math::Cos256(y) + this->positions[i + 1].x;
        this->positions[i].y = this->segmentMagnitude[i] * Math::Sin256(y) + this->positions[i + 1].y;
    }
}

void Rexon::LateUpdate() {}

void Rexon::StaticUpdate() {}

void Rexon::Draw()
{
    this->bodyAnimator.DrawSprite(nullptr, false);

    if (!this->state.Matches(&Rexon::State_Destroyed) && !this->state.Matches(&Rexon::State_Explode)) {
        this->headAnimator.DrawSprite(this->positions, false);
        for (int32 i = 1; i < REXON_SEGMENT_COUNT; ++i) this->neckAnimator.DrawSprite(&this->positions[i], false);
    }
}

void Rexon::Create(void *data)
{
    this->drawFX |= FX_FLIP;
    this->startPos      = this->position;
    this->startDir      = this->direction;
    this->visible       = true;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->type          = VOID_TO_INT(data);

    switch (this->type) {
        case REXON_MAIN:
            this->timer = 128;
            if (!this->noMove)
                this->velocity.x = -0x2000;

            for (int32 i = 0; i < REXON_SEGMENT_COUNT; ++i) {
                this->segmentMagnitude[i]  = 0;
                this->segmentAmplitude[i]  = 0x170;
                this->segmentDirections[i] = FLIP_NONE;
            }

            this->segmentID = 0;
            this->bodyAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
            this->headAnimator.SetAnimation(sVars->aniFrames, 1, true, 2);
            this->neckAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
            this->state.Set(&Rexon::State_Init);
            break;

        case REXON_DEBRIS:
            this->active = ACTIVE_NORMAL;
            this->bodyAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
            this->state.Set(&Rexon::State_Debris);
            break;

        case REXON_SHOT:
            this->active = ACTIVE_NORMAL;
            this->bodyAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
            this->state.Set(&Rexon::State_Projectile);
            break;
    }
}

void Rexon::StageLoad(void)
{
    sVars->aniFrames.Load("HTZ/Rexon.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left   = -8;
    sVars->hitboxBadnik.top    = -8;
    sVars->hitboxBadnik.right  = 8;
    sVars->hitboxBadnik.bottom = 8;

    sVars->hitboxShell.left   = -16;
    sVars->hitboxShell.top    = -8;
    sVars->hitboxShell.right  = 16;
    sVars->hitboxShell.bottom = 8;

    sVars->hitboxRange.left   = -160;
    sVars->hitboxRange.top    = -256;
    sVars->hitboxRange.right  = 96;
    sVars->hitboxRange.bottom = 256;

    sVars->hitboxProjectile.left   = -4;
    sVars->hitboxProjectile.top    = -4;
    sVars->hitboxProjectile.right  = 4;
    sVars->hitboxProjectile.bottom = 4;

    sVars->sfxShot.Get("Stage/Shot.wav");
    sVars->sfxExplosion.Get("Stage/Explosion2.wav");

    DebugMode::AddObject(sVars->classID, &Rexon::DebugSpawn, &Rexon::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Rexon::DebugSpawn()
{
    GameObject::Create<Rexon>(nullptr, this->position.x, this->position.y);
}

void Rexon::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 3, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Rexon::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position  = this->startPos;
        this->direction = this->startDir;
        Rexon::Create(nullptr);
    }
}

void Rexon::CheckPlayerCollisions()
{
    if (!this->state.Matches(&Rexon::State_Explode)) {
        int32 storeX = this->position.x;
        int32 storeY = this->position.y;

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
        {
            // Provide a shell box for players to stand on
            player->CheckCollisionBox(this, &sVars->hitboxShell);

            if (!this->state.Matches(&Rexon::State_Destroyed)) {

                // Check if the head's been hit
                this->position.x = this->positions[0].x;
                this->position.y = this->positions[0].y;
                if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik)) {
                    if (player->CheckBadnikBreak(this, false))
                        Rexon::DestroyRexon(this, false);
                }
                else {
                    // Check hitboxes for all body parts individually, too
                    for (int32 i = 1; i < REXON_SEGMENT_COUNT; ++i) {
                        this->position.x = this->positions[i].x;
                        this->position.y = this->positions[i].y;
                        if (player->CheckCollisionTouch(this, &sVars->hitboxBadnik))
                            player->Hurt(this);
                    }
                }

                this->position.x = storeX;
                this->position.y = storeY;
            }
        }
    }
}

void Rexon::DestroyRexon(Rexon *rexon, bool32 crushed)
{
    if (!rexon->state.Matches(&Rexon::State_Destroyed) && !rexon->state.Matches(&Rexon::State_Explode)) {
        int32 x = rexon->position.x;
        int32 y = rexon->position.y;

        rexon->position.x = rexon->positions[0].x;
        rexon->position.y = rexon->positions[0].y;

        BadnikHelpers::BadnikBreak(rexon, false, true);

        rexon->position.x = x;
        rexon->position.y = y;

        int32 velocities[] = { -0x8000, 0x10000, -0x10000, 0x8000 };
        for (int32 i = 1; i < REXON_SEGMENT_COUNT; ++i) {
            Rexon *debris = GameObject::Create<Rexon>(INT_TO_VOID(REXON_DEBRIS), rexon->positions[i].x, rexon->positions[i].y);
            debris->direction   = rexon->direction;
            debris->velocity.x  = velocities[(i - 1) & 3];
        }
    }

    if (crushed) {
        rexon->active       = ACTIVE_NORMAL;
        rexon->destroyDelay = 60;
        rexon->state.Set(&Rexon::State_Explode);
    }
    else {
        rexon->state.Set(&Rexon::State_Destroyed);
    }
}

void Rexon::State_Init()
{
    this->active = ACTIVE_NORMAL;

    this->state.Set(&Rexon::State_Hidden);
    Rexon::State_Hidden();
}

void Rexon::State_Hidden()
{
    if (this->noMove) {
        this->direction = Player::GetNearestPlayerXY()->position.x >= this->position.x;
    }
    else {
        this->position.x += this->velocity.x;
        if (--this->timer <= 0) {
            this->direction ^= FLIP_X;
            this->timer      = 128;
            this->velocity.x = -this->velocity.x;
        }
    }

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->CheckCollisionTouch(this, &sVars->hitboxRange)) {
            this->state.Set(&Rexon::State_Rising);
            this->direction = this->direction = player->position.x >= this->position.x;
            break;
        }
    }

    Rexon::CheckPlayerCollisions();
    Rexon::CheckOffScreen();
}

void Rexon::State_Rising()
{
    this->segmentMagnitude[this->segmentID] += 640;
    if (this->segmentMagnitude[this->segmentID] >= 0xF00) {
        this->segmentMagnitude[this->segmentID] = 0xF00;
        ++this->segmentID;
    }

    for (int32 i = this->segmentID - 1; i >= 0; --i) {
        this->segmentAmplitude[i] += 2;
    }

    if (this->segmentID == REXON_SEGMENT_COUNT)
        this->state.Set(&Rexon::State_Shooting);

    Rexon::CheckPlayerCollisions();
    Rexon::CheckOffScreen();
}

void Rexon::State_Shooting()
{
    this->headAnimator.Process();

    for (int32 i = 0; i < REXON_SEGMENT_COUNT; ++i) {
        if (this->segmentDirections[i]) {
            if (--this->segmentAmplitude[i] <= 0x160)
                this->segmentDirections[i] = FLIP_NONE;
        }
        else if (++this->segmentAmplitude[i] >= 0x1A0) {
            this->segmentDirections[i] = FLIP_X;
        }
    }

    if (this->segmentAmplitude[0] == 0x160) {
        sVars->sfxShot.Play(false, 255);

        Rexon *shot = GameObject::Create<Rexon>(INT_TO_VOID(REXON_SHOT), this->positions[0].x, this->positions[0].y);
        if (this->direction) {
            shot->position.x += 0xE0000;
            shot->velocity.x = 0x10000;
        }
        else {
            shot->position.x -= 0xE0000;
            shot->velocity.x = -0x10000;
        }

        shot->position.y += 0x60000;
        shot->velocity.y = 0x8000;
    }
    else if (this->segmentAmplitude[0] == 0x170) {
        if (this->segmentDirections[0] == FLIP_X)
            this->headAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    }
    else if (this->segmentAmplitude[0] == 0x180 && this->segmentDirections[0] == FLIP_NONE) {
        this->headAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    }

    Rexon::CheckPlayerCollisions();
    Rexon::CheckOffScreen();
}

void Rexon::State_Destroyed()
{
    Rexon::CheckPlayerCollisions();
    Rexon::CheckOffScreen();
}

void Rexon::State_Explode()
{
    this->position.y += 0x4000;
    this->velocity.y = 0x4000;

    if (!(Zone::sVars->timer % 3)) {
        sVars->sfxExplosion.Play(false, 255);

        if (Zone::sVars->timer & 4) {
            int32 x                    = this->position.x + (Math::Rand(sVars->hitboxShell.left, sVars->hitboxShell.right) << 16);
            int32 y                    = this->position.y + (Math::Rand(sVars->hitboxShell.top, sVars->hitboxShell.bottom) << 16);
            Explosion *explosion       = GameObject::Create<Explosion>(INT_TO_VOID((Math::Rand(0, 256) > 192) + Explosion::Type2), x, y);
            explosion->drawGroup       = Zone::sVars->objectDrawGroup[1];
        }
    }

    if (--this->destroyDelay <= 0)
        this->Destroy();
}

void Rexon::State_Debris()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x3800;

    this->visible ^= true;

    if (!this->CheckOnScreen(nullptr))
        this->Destroy();
}

void Rexon::State_Projectile()
{
    this->bodyAnimator.Process();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->CheckCollisionTouch(this, &sVars->hitboxProjectile)) {
            player->ProjectileHurt(this);
        }
    }

    if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, sVars->hitboxProjectile.bottom << 13, 4)
        || this->TileCollision(Zone::sVars->collisionLayers, CMODE_LWALL, 0, sVars->hitboxProjectile.left << 13, 0, 4)
        || this->TileCollision(Zone::sVars->collisionLayers, CMODE_ROOF, 0, 0, sVars->hitboxProjectile.top << 13, 4)
        || this->TileCollision(Zone::sVars->collisionLayers, CMODE_RWALL, 0, sVars->hitboxProjectile.right << 13, 0, 4)
        || !this->CheckOnScreen(nullptr)) {
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void Rexon::EditorDraw()
{
    for (int32 i = 0; i < REXON_SEGMENT_COUNT; ++i) {
        this->segmentMagnitude[i] = 0xF00;
        this->segmentAmplitude[i] = 0x170;
    }

    this->segmentID = 0;
    this->bodyAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
    this->headAnimator.SetAnimation(sVars->aniFrames, 1, true, 2);
    this->neckAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);

    this->positions[REXON_SEGMENT_COUNT].x = this->position.x;
    this->positions[REXON_SEGMENT_COUNT].y = this->position.y;

    if (this->direction)
        this->positions[REXON_SEGMENT_COUNT].x += 0x1C0000;
    else
        this->positions[REXON_SEGMENT_COUNT].x -= 0x1C0000;
    this->positions[REXON_SEGMENT_COUNT].y += 0x110000;

    for (int32 i = REXON_SEGMENT_COUNT - 1; i >= 0; --i) {
        int32 y = this->segmentAmplitude[i] >> 1;
        if (this->direction == FLIP_X)
            y = (0x80 - y) & 0xFF;

        this->positions[i].x = this->segmentMagnitude[i] * Math::Cos256(y) + this->positions[i + 1].x;
        this->positions[i].y = this->segmentMagnitude[i] * Math::Sin256(y) + this->positions[i + 1].y;
    }

    Rexon::Draw();
}

void Rexon::EditorLoad()
{
    sVars->aniFrames.Load("HTZ/Rexon.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left", FLIP_NONE);
    RSDK_ENUM_VAR("Right", FLIP_X);
}
#endif

void Rexon::Serialize()
{
    RSDK_EDITABLE_VAR(Rexon, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(Rexon, VAR_BOOL, noMove);
}

} // namespace GameLogic