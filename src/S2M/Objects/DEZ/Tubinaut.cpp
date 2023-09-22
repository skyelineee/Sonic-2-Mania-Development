// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Tubinaut Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Tubinaut.hpp"
#include "Global/Player.hpp"
#include "Global/Dust.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Tubinaut);

void Tubinaut::Update() { this->state.Run(this); }

void Tubinaut::LateUpdate() {}

void Tubinaut::StaticUpdate() {}

void Tubinaut::Draw()
{
    if (this->attackTimer < 256 || this->orbCount <= 1) {
        for (int32 i = 0; i < TUBINAUT_ORB_COUNT; ++i) {
            if (this->ballsVisible[i]) {
                this->ballAnimators[i].DrawSprite(&this->orbPositions[i], false);
            }
        }
    }
    else {
        this->fieldAnimator.DrawSprite(nullptr, false);
    }

    this->drawFX = FX_FLIP;
    this->bodyAnimator.DrawSprite(nullptr, false);
    this->drawFX = FX_NONE;
}

void Tubinaut::Create(void *data)
{
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];

    if (data) {
        this->active        = ACTIVE_NORMAL;
        this->updateRange.x = 0x400000;
        this->updateRange.y = 0x400000;
        this->bodyAnimator.SetAnimation(sVars->aniFrames, VOID_TO_INT(data), true, 0);
        this->state.Set(&Tubinaut::Orb_BodyDeath);
    }
    else {
        this->startPos      = this->position;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;

        this->ballAnimators[0].SetAnimation(sVars->aniFrames, 1, true, 0);
        this->ballAnimators[1].SetAnimation(sVars->aniFrames, 2, true, 0);
        this->ballAnimators[2].SetAnimation(sVars->aniFrames, 3, true, 0);

        int32 angle = 0x000;
        for (int32 o = 0; o < TUBINAUT_ORB_COUNT; ++o) {
            this->orbAngles[o]    = angle;
            this->ballsVisible[o] = true;

            angle += 0x500;
        }

        this->activeOrbs  = 0b111;
        this->orbCount    = 3;
        this->attackTimer = 64;
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->fieldAnimator.SetAnimation(sVars->aniFrames, 7, true, 0);

        this->state.Set(&Tubinaut::State_Init);
        this->orbState.Set(&Tubinaut::Orb_Relax);
    }
}

void Tubinaut::StageLoad()
{
    sVars->aniFrames.Load("DEZ/Tubinaut.bin", SCOPE_STAGE);

    sVars->hitboxFace.left   = -10;
    sVars->hitboxFace.top    = -10;
    sVars->hitboxFace.right  = 10;
    sVars->hitboxFace.bottom = 10;

    sVars->hitboxOrb.left   = -7;
    sVars->hitboxOrb.top    = -7;
    sVars->hitboxOrb.right  = 7;
    sVars->hitboxOrb.bottom = 7;

    sVars->hitboxSpin.left   = -20;
    sVars->hitboxSpin.top    = -20;
    sVars->hitboxSpin.right  = 20;
    sVars->hitboxSpin.bottom = 20;

    DebugMode::AddObject(sVars->classID, &Tubinaut::DebugSpawn, &Tubinaut::DebugDraw);

    sVars->sfxPowerdown.Get("Stage/PowerDown.wav");
    sVars->sfxRepel.Get("Stage/Repel.wav");
}

void Tubinaut::DebugSpawn()
{
    Tubinaut *tubinaut = GameObject::Create<Tubinaut>(nullptr, this->position.x, this->position.y);
    tubinaut->distanceRemain = 256;
    tubinaut->distance       = 256;
}

void Tubinaut::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Tubinaut::CheckPlayerCollisions()
{
    int32 storeX   = this->position.x;
    int32 storeY   = this->position.y;
    int32 playerID = 0;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (this->playerTimers[playerID]) {
            --this->playerTimers[playerID];
        }
        else if (this->orbState.Matches(&Tubinaut::Orb_Attack) && this->orbCount > 1) {
            if (player->CheckCollisionTouch(this, &sVars->hitboxSpin)
                && (this->orbCount == 2 || (this->orbCount == 3 && !player->Hurt(this)))) {
                Tubinaut::HandleRepel(player, playerID);
            }
        }
        else {
            bool32 repelled = false;
            for (int32 i = 0; i < TUBINAUT_ORB_COUNT; ++i) {
                if (this->ballsVisible[i]) {
                    this->position.x = this->orbPositions[i].x;
                    this->position.y = this->orbPositions[i].y;

                    if (player->CheckCollisionTouch(this, &sVars->hitboxOrb)) {
                        Tubinaut::OrbHit(player, i);
                        repelled                     = true;
                        this->playerTimers[playerID] = 15;

                        if (this->orbCount) {
                            if (this->orbCount == 1) {
                                this->bodyAnimator.frameID = 2;
                            }
                            else if (this->orbCount == 2) {
                                switch (this->activeOrbs) {
                                    case 0b011: this->fieldAnimator.SetAnimation(sVars->aniFrames, 4, true, 0); break;
                                    case 0b101: this->fieldAnimator.SetAnimation(sVars->aniFrames, 6, true, 0); break;
                                    case 0b110: this->fieldAnimator.SetAnimation(sVars->aniFrames, 5, true, 0); break;
                                }

                                this->bodyAnimator.frameID = 0;
                            }
                        }
                        else {
                            this->bodyAnimator.frameID = 3;
                        }
                    }
                }
            }

            this->position.x = storeX;
            this->position.y = storeY;
            if (!repelled && player->CheckBadnikTouch(this, &sVars->hitboxFace) && player->CheckBadnikBreak(this, false)) {
                for (int32 i = 0; i < TUBINAUT_ORB_COUNT; ++i) {
                    if (this->ballsVisible[i]) {
                        Tubinaut *orb = GameObject::Create<Tubinaut>(INT_TO_VOID(i + 1), this->orbPositions[i].x, this->orbPositions[i].y);
                        orb->velocity.x     = 0x380 * Math::Cos256(this->orbAngles[i] >> 4);
                        orb->velocity.y     = 0x380 * Math::Sin256(this->orbAngles[i] >> 4);
                    }
                }

                this->Destroy();
                this->active = ACTIVE_DISABLED;
            }
        }

        ++playerID;
    }
}

bool32 Tubinaut::CheckAttacking(Player *player)
{
    bool32 isAttacking = player->CheckAttacking(this);

    if (!isAttacking)
        player->Hurt(this);

    return isAttacking;
}

void Tubinaut::OrbHit(Player *player, int32 orbID)
{
    if (Tubinaut::CheckAttacking(player)) {
        Dust *dust       = GameObject::Create<Dust>(this, this->position.x, this->position.y);
        dust->state.Set(&Dust::State_DustPuff);
        dust->drawGroup  = player->drawGroup;
        sVars->sfxPowerdown.Play(false, 255);
        int32 angle = Math::ATan2(player->position.x - this->position.x, player->position.y - this->position.y);

        int32 velX = 0, velY = 0;
        velX = 0x540 * Math::Cos256(angle);
        velY = 0x540 * Math::Sin256(angle);

        if (player->characterID != ID_KNUCKLES || player->animator.animationID != Player::ANI_GLIDE) {
            player->velocity.x   = velX;
            player->groundVel    = velX;
            player->applyJumpCap = false;
        }

        player->velocity.y     = velY;
        player->onGround       = false;
        player->tileCollisions = TILECOLLISION_DOWN;

        this->ballsVisible[orbID] = false;
        --this->orbCount;
        this->activeOrbs &= ~(1 << orbID);

        if (this->orbCount == 2) {
            switch (orbID) {
                case 0:
                    this->reAdjustTimers[1] = -24;
                    this->reAdjustTimers[2] = 24;
                    break;

                case 1:
                    this->reAdjustTimers[2] = -24;
                    this->reAdjustTimers[0] = 24;
                    break;

                case 2:
                    this->reAdjustTimers[0] = -24;
                    this->reAdjustTimers[1] = 24;
                    break;
            }
        }
    }
}

void Tubinaut::HandleRepel(Player *player, int32 playerID)
{
    if (Tubinaut::CheckAttacking(player)) {
        int32 angle = Math::ATan2(player->position.x - this->position.x, player->position.y - this->position.y);
        int32 velX  = 0x700 * Math::Cos256(angle);
        int32 velY  = 0x700 * Math::Sin256(angle);
    
        if (player->characterID != ID_KNUCKLES || player->animator.animationID != Player::ANI_GLIDE) {
            player->velocity.x   = velX;
            player->groundVel    = velX;
            player->applyJumpCap = false;
        }
    
        player->velocity.y           = velY;
        player->onGround             = false;
        player->tileCollisions       = TILECOLLISION_DOWN;
        this->playerTimers[playerID] = 15;
        sVars->sfxRepel.Play(false, 0xFF);
    }
}

void Tubinaut::HandleOrbs()
{
    this->position.y = (Math::Sin256(this->angle) << 10) + this->startPos.y;
    this->angle      = (this->angle + 4) & 0xFF;

    for (int32 i = 0; i < TUBINAUT_ORB_COUNT; ++i) {
        this->orbPositions[i].x = 0x1400 * Math::Cos256(this->orbAngles[i] >> 4) + this->position.x;
        this->orbPositions[i].y = 0x1400 * Math::Sin256(this->orbAngles[i] >> 4) + this->position.y;
        this->orbAngles[i] -= this->attackTimer;

        if (this->reAdjustTimers[i]) {
            if (this->reAdjustTimers[i] <= 0) {
                this->reAdjustTimers[i]++;
                this->orbAngles[i] -= 16;
            }
            else {
                this->reAdjustTimers[i]--;
                this->orbAngles[i] += 16;
            }
        }
        this->orbAngles[i] &= 0xFFF;

        if (this->orbState.Matches(&Tubinaut::Orb_Cooldown) || this->orbCount <= 1 || this->attackTimer < 192) {
            this->ballAnimators[i].frameID = (2 * (7 - ((((this->orbAngles[i] >> 4) + 112) >> 5) & 7)));
            this->ballAnimators[i].timer   = (this->ballAnimators[i].timer + 1) & 3;
            this->ballAnimators[i].frameID += (this->ballAnimators[i].timer >> 1);
        }
        else {
            int32 id                     = ((this->ballAnimators[i].timer + 1) & 7) >> 1;
            this->ballAnimators[i].timer = (this->ballAnimators[i].timer + 1) & 7;

            switch (id) {
                case 0:
                case 2: this->ballAnimators[i].frameID = (id + 2 * (7 - ((((this->orbAngles[i] >> 4) + 112) >> 5) & 7))); break;

                case 1: this->ballAnimators[i].frameID = 17; break;
                case 3: this->ballAnimators[i].frameID = 16; break;

                default: break;
            }
        }
    }
}

void Tubinaut::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position = this->startPos;
        this->visible  = false;
        this->state.Set(&Tubinaut::State_Init);
        this->active   = ACTIVE_BOUNDS;

        if (this->orbCount > 1) {
            this->attackTimer = 64;
            this->orbState.Set(&Tubinaut::Orb_Relax);
        }
    }
}

void Tubinaut::State_Init()
{
    this->timer          = 0;
    this->distanceRemain = this->distance;
    this->direction      = FLIP_NONE;
    this->velocity.x     = -0x8000;
    this->visible        = true;
    this->active         = ACTIVE_NORMAL;

    this->state.Set(&Tubinaut::State_Move);
    Tubinaut::State_Move();
}

void Tubinaut::State_Move()
{
    if (this->distance) {
        this->position.x += this->velocity.x;

        if (!--this->distanceRemain) {
            this->distanceRemain = this->distance;
            this->direction ^= FLIP_X;
            this->velocity.x = -this->velocity.x;
        }
    }
    else {
        Player *playerPtr = nullptr;
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (playerPtr) {
                if (abs(player->position.x - this->position.x) < abs(playerPtr->position.x - this->position.x))
                    playerPtr = player;
            }
            else {
                playerPtr = player;
            }
        }

        if (playerPtr)
            this->direction = playerPtr->position.x >= this->position.x;
    }

    Tubinaut::HandleOrbs();

    if (!this->orbCount)
        this->bodyAnimator.Process();

    this->fieldAnimator.Process();

#if RETRO_USE_MOD_LOADER
    this->orbState.Run(this);
#else
    // Explitly called, no StateMachine call for some reason
    this->orbState();
#endif

    Tubinaut::CheckPlayerCollisions();
    Tubinaut::CheckOffScreen();
}

void Tubinaut::Orb_Relax()
{
    if (++this->timer > 120) {
        this->orbState.Set(&Tubinaut::Orb_PrepareAttack);
        if (this->orbCount > 2)
            this->bodyAnimator.frameID = 1;
        this->timer = 0;
    }
}

void Tubinaut::Orb_PrepareAttack()
{
    if (this->attackTimer >= 256) {
        if (this->orbCount > 1)
            this->orbState.Set(&Tubinaut::Orb_Attack);
    }
    else {
        this->attackTimer++;
    }
}

void Tubinaut::Orb_Attack()
{
    if (++this->timer > 120) {
        this->orbState.Set(&Tubinaut::Orb_Cooldown);
        switch (this->orbCount) {
            case 0: this->bodyAnimator.frameID = 3; break;
            case 1: this->bodyAnimator.frameID = 2; break;

            case 2:
            case 3: this->bodyAnimator.frameID = 0; break;

            default: break;
        }

        this->timer = 0;
    }
}

void Tubinaut::Orb_Cooldown()
{
    if (this->orbCount > 1) {
        if (--this->attackTimer <= 64)
            this->orbState.Set(&Tubinaut::Orb_Relax);
    }
    else {
        this->orbState.Set(&Tubinaut::Orb_PrepareAttack);
    }
}

void Tubinaut::Orb_BodyDeath()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x3800;

    if (!this->CheckOnScreen(nullptr))
        this->Destroy();
}

#if RETRO_INCLUDE_EDITOR
void Tubinaut::EditorDraw()
{
    this->ballAnimators[0].SetAnimation(sVars->aniFrames, 1, true, 0);
    this->ballAnimators[1].SetAnimation(sVars->aniFrames, 2, true, 0);
    this->ballAnimators[2].SetAnimation(sVars->aniFrames, 3, true, 0);

    int32 angle = 0x000;
    for (int32 o = 0; o < TUBINAUT_ORB_COUNT; ++o) {
        this->orbAngles[o]    = angle;
        this->ballsVisible[o] = true;

        angle += 0x500;
    }

    this->activeOrbs  = 0b111;
    this->orbCount    = 3;
    this->attackTimer = 64;
    this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->fieldAnimator.SetAnimation(sVars->aniFrames, 7, true, 0);
    this->state.Set(&Tubinaut::State_Init);
    this->orbState.Set(&Tubinaut::Orb_Relax);

    for (int32 i = 0; i < TUBINAUT_ORB_COUNT; ++i) {
        this->orbPositions[i].x = 0x1400 * Math::Cos256(this->orbAngles[i] >> 4) + this->position.x;
        this->orbPositions[i].y = 0x1400 * Math::Sin256(this->orbAngles[i] >> 4) + this->position.y;
    }

    Tubinaut::Draw();

    if (showGizmos()) {
        int32 x = this->position.x;

        this->position.x += this->distance * -0x8000;

        for (int32 i = 0; i < TUBINAUT_ORB_COUNT; ++i) {
            this->orbPositions[i].x = this->position.x + 0x1400 * Math::Cos256(this->orbAngles[i] >> 4);
            this->orbPositions[i].y = this->position.y + 0x1400 * Math::Sin256(this->orbAngles[i] >> 4);
        }

        this->inkEffect = INK_BLEND;
        Tubinaut::Draw();
        this->inkEffect = INK_NONE;

        Graphics::DrawLine(x, this->position.y, this->position.x, this->position.y, 0x00FF00, 0xFF, INK_NONE, false);

        this->position.x = x;
    }
}

void Tubinaut::EditorLoad()
{
    sVars->aniFrames.Load("DEZ/Tubinaut.bin", SCOPE_STAGE);
}
#endif

void Tubinaut::Serialize(void) { RSDK_EDITABLE_VAR(Tubinaut, VAR_UINT16, distance); }

} // namespace GameLogic