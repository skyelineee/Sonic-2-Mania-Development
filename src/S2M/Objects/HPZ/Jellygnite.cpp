// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Jellygnite Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Jellygnite.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"
#include "Global/Explosion.hpp"
#include "Common/Water.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Jellygnite);

void Jellygnite::Update()
{
    this->bodyAnimator.Process();
    this->backTentacleAnimator.Process();
    this->frontTentacleAnimator.Process();

    this->state.Run(this);

    if (!this->state.Matches(&Jellygnite::State_Init) && this->classID == sVars->classID) {
        if (this->grabDelay > 0)
            this->grabDelay--;

        Jellygnite::CheckPlayerCollisions();

        if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
            this->position           = this->startPos;
            this->direction          = this->startDir;
            this->visible            = false;
            this->timer              = 0;
            this->angle              = 0;
            this->frontTentacleAngle = 0;
            this->oscillateAngle     = 0;
            this->grabbedPlayer      = 0;
            this->shakeTimer         = 0;
            this->shakeCount         = 0;
            this->lastShakeFlags     = 0;

            Jellygnite::Create(nullptr);
        }
    }
}

void Jellygnite::LateUpdate() {}

void Jellygnite::StaticUpdate()
{
    for (auto jellygnite : GameObject::GetEntities<Jellygnite>(FOR_ACTIVE_ENTITIES))  { RSDKTable->AddDrawListRef(Zone::sVars->objectDrawGroup[1], RSDKTable->GetEntitySlot(jellygnite)); }
}

void Jellygnite::Draw()
{
    Jellygnite::DrawBackTentacle();
    Jellygnite::DrawFrontTentacle();

    this->bodyAnimator.DrawSprite(nullptr, false);
}

void Jellygnite::Create(void *data)
{
    this->visible       = true;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->startPos      = this->position;
    this->startDir      = this->direction;
    this->drawFX        = FX_FLIP;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->state.Set(&Jellygnite::State_Init);
}

void Jellygnite::StageLoad()
{
    sVars->aniFrames.Load("HPZ/Jellygnite.bin", SCOPE_STAGE);

    sVars->hitbox.left   = -14;
    sVars->hitbox.top    = -14;
    sVars->hitbox.right  = 14;
    sVars->hitbox.bottom = 14;

    sVars->sfxGrab.Get("Global/Grab.wav");
    sVars->sfxElectrify.Get("Stage/Electrify2.wav");

    DebugMode::AddObject(sVars->classID, &Jellygnite::DebugSpawn, &Jellygnite::DebugDraw);
}

void Jellygnite::DebugSpawn()
{
    Jellygnite *jellygnite       = GameObject::Create<Jellygnite>(nullptr, this->position.x, this->position.y);
    jellygnite->direction        = this->direction;
    jellygnite->startDir         = this->direction;
}

void Jellygnite::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Jellygnite::SetupAnimations(uint8 animationID)
{
    switch (animationID) {
        case JELLYGNITE_ANI_FLOATING:
            this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->frontTentacleAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
            this->backTentacleAnimator.SetAnimation(sVars->aniFrames, 5, true, 0);
            break;

        case JELLYGNITE_ANI_ANGRY:
            this->bodyAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
            this->frontTentacleAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
            this->backTentacleAnimator.SetAnimation(sVars->aniFrames, 5, true, 0);
            break;

        case JELLYGNITE_ANI_FLASHING:
            this->bodyAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
            this->frontTentacleAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
            this->backTentacleAnimator.SetAnimation(sVars->aniFrames, 6, true, 0);
            break;
    }

    this->prevAnimationID = animationID;
}

void Jellygnite::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player != this->grabbedPlayer) {
            if (player->CheckCollisionTouch(this, &sVars->hitbox) && this->state.Matches(&Jellygnite::State_Swimming)
                && player->position.y >= this->position.y) {

                this->state.Set(&Jellygnite::State_GrabbedPlayer);
                Jellygnite::SetupAnimations(JELLYGNITE_ANI_ANGRY);
                this->grabbedPlayer = player;

                this->isPermanent = true;
                sVars->sfxGrab.Play(false, 255);
                player->velocity.x      = 0;
                player->velocity.y      = 0;
                player->groundVel       = 0;
                player->state.Set(&Player::State_Static);
                player->nextAirState.Set(nullptr);
                player->nextGroundState.Set(nullptr);
                player->onGround        = false;
                player->direction       = this->direction;
                player->animator.SetAnimation(player->aniFrames, Player::ANI_FAN, true, 0);
                player->animator.speed = 0;
                player->direction      = this->direction ^ FLIP_X;
            }

            if (player != this->grabbedPlayer && player->position.y < this->position.y && player->CheckBadnikTouch(this, &sVars->hitbox)
                && player->CheckBadnikBreak(this, false)) {
                if (this->grabbedPlayer)
                    this->grabbedPlayer->state.Set(&Player::State_Air);

                this->Destroy();
            }
        }
    }
}

void Jellygnite::HandlePlayerStruggle()
{
    Player *player = this->grabbedPlayer;

    if (player) {
        if (this->lastShakeFlags) {
            if (!--this->shakeTimer) {
                this->shakeCount     = 0;
                this->lastShakeFlags = 0;
            }

            uint8 shakeFlags = 0;
            if (player->left)
                shakeFlags = 1;
            if (player->right)
                shakeFlags |= 2;

            if (shakeFlags && shakeFlags != 3 && shakeFlags != this->lastShakeFlags) {
                this->lastShakeFlags = shakeFlags;
                if (++this->shakeCount >= 4) {
                    player->state.Set(&Player::State_Air);
                    this->grabDelay     = 16;
                    this->grabbedPlayer = nullptr;
                }
                else {
                    this->shakeTimer = 64;
                }
            }
        }
        else {
            if (player->left) {
                this->lastShakeFlags = 1;
                this->shakeTimer     = 32;
            }

            if (player->right) {
                this->lastShakeFlags |= 2;
                this->shakeTimer = 32;
            }
        }

        player->position.x = this->position.x;
        player->position.y = this->position.y + 0xC0000;
    }
}

bool32 Jellygnite::CheckInWater(Player *player)
{
    if (player->position.y > Water::sVars->waterLevel)
        return true;

    for (auto water : GameObject::GetEntities<Water>(FOR_ACTIVE_ENTITIES)) 
    {
        if (water->type == Water::Pool) {
            if (player->CheckCollisionTouch(this, &water->hitbox)
                && this->CheckCollisionTouchBox(&sVars->hitbox, water, &water->hitbox)) {
                return true;
            }
        }
    }
    return false;
}

void Jellygnite::DrawBackTentacle()
{
    int32 angle = this->angle & 0x1FF;
    int32 y     = this->position.y + 0x70000;

    for (int32 i = 0; i < 4; ++i) {
        Vector2 drawPos;
        drawPos.x = this->position.x + (Math::Cos512(angle) << 9);
        drawPos.y = y + (Math::Sin512(angle) << 8);
        this->backTentacleAnimator.DrawSprite(&drawPos, false);

        angle = (angle + 0x20) & 0x1FF;
        y += 0x60000;
    }
}

void Jellygnite::DrawFrontTentacle()
{
    int32 x   = 0;
    int32 y   = 0;
    int32 ang = this->frontTentacleAngle;
    for (int32 i = 0; i < 4; ++i) {
        int32 angle = (ang >> 7) & 0x1FF;
        x += 0x312 * Math::Sin512(angle);
        y += 0x312 * Math::Cos512(angle);

        Vector2 drawPos;
        drawPos.x = x + this->position.x - 0xD0000;
        drawPos.y = y + this->position.y + 0x10000;
        this->frontTentacleAnimator.DrawSprite(&drawPos, false);

        drawPos.x = -x;
        drawPos.x = this->position.x + 0xD0000 - x;
        this->frontTentacleAnimator.DrawSprite(&drawPos, false);

        ang <<= 1;
    }
}

void Jellygnite::State_Init()
{
    if (this->position.y >= Water::sVars->waterLevel) {
        this->active             = ACTIVE_NORMAL;
        this->timer              = 0;
        this->angle              = 0;
        this->frontTentacleAngle = 0;
        this->oscillateAngle     = 0;
        this->grabbedPlayer      = 0;
        this->shakeTimer         = 0;
        this->shakeCount         = 0;
        this->lastShakeFlags     = 0;
        Jellygnite::SetupAnimations(JELLYGNITE_ANI_FLOATING);

        this->state.Set(&Jellygnite::State_Swimming);
        Jellygnite::State_Swimming();
    }
    else {
        this->Destroy();
    }
}

void Jellygnite::State_Swimming()
{
    this->angle              = (this->angle + 4) & 0x1FF;
    this->frontTentacleAngle = Math::Sin512(this->angle) << 1;

    this->oscillateAngle = (this->oscillateAngle + 1) & 0x1FF;
    this->position.x     = (Math::Sin512(this->oscillateAngle) << 11) + this->startPos.x;

    Player *playerPtr = Player::GetNearestPlayerX();
    if (playerPtr) {
        if (Jellygnite::CheckInWater(playerPtr)) {
            if (this->position.y <= playerPtr->position.y - 0x200000) {
                this->velocity.y += 0x800;

                if (this->velocity.y >= 0xC000)
                    this->velocity.y = 0xC000;
            }
            else {
                this->velocity.y -= 0x800;

                if (this->velocity.y <= -0xC000)
                    this->velocity.y = -0xC000;
            }
        }
        else {
            this->velocity.y >>= 1;
        }
    }

    this->position.y += this->velocity.y;

    if (this->position.y - 0x100000 < Water::sVars->waterLevel && this->velocity.y < 0) {
        this->position.y = this->position.y - this->velocity.y;
        this->velocity.y = -this->velocity.y;
    }
}

void Jellygnite::State_GrabbedPlayer()
{
    if (this->frontTentacleAngle >= 0x600) {
        this->state.Set(&Jellygnite::State_Explode);
        Jellygnite::SetupAnimations(JELLYGNITE_ANI_FLASHING);
        sVars->sfxElectrify.Play(false, 255);
    }
    else {
        this->frontTentacleAngle += 0x80;
    }

    Player *player = this->grabbedPlayer;
    player->position.x   = this->position.x;
    player->position.y   = this->position.y + 0xC0000;
}

void Jellygnite::State_Explode()
{
    Jellygnite::HandlePlayerStruggle();

    if (++this->timer == 60) {
        Player *player = this->grabbedPlayer;
        if (player && player->state.Matches(&Player::State_Static)) {
            player->Hurt(this);

            if (!player->state.Matches(&Player::State_Hurt) && player->CheckValidState())
                player->state.Set(&Player::State_Air);

            this->grabbedPlayer = nullptr;
        }

        GameObject::Create<Explosion>(INT_TO_VOID(Explosion::Type1), this->position.x, this->position.y)->drawGroup = Zone::sVars->objectDrawGroup[1];
        Explosion::sVars->sfxDestroy.Play(false, 255);
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void Jellygnite::EditorDraw()
{
    Jellygnite::SetupAnimations(JELLYGNITE_ANI_FLOATING);

    Jellygnite::Draw();
}

void Jellygnite::EditorLoad()
{
    sVars->aniFrames.Load("HPZ/Jellygnite.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Right");
}
#endif

void Jellygnite::Serialize(void) { RSDK_EDITABLE_VAR(Jellygnite, VAR_UINT8, direction); }

} // namespace GameLogic