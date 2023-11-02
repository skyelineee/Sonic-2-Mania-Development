// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Grabber Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Grabber.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Explosion.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Grabber);

void Grabber::Update() { this->state.Run(this); }

void Grabber::LateUpdate(void) {}

void Grabber::StaticUpdate(void) {}

void Grabber::Draw(void)
{
    if (sceneInfo->currentDrawGroup == this->drawGroup) {
        Graphics::DrawLine(this->position.x, this->startPos.y - 0x100000, this->position.x, this->position.y, 0x202020, 0x00, INK_NONE, false);
        Graphics::DrawLine(this->position.x - 0x10000, this->startPos.y - 0x100000, this->position.x - 0x10000, this->position.y, 0xE0E0E0, 0x00, INK_NONE,
                      false);

        Vector2 drawPos;
        int32 storeDir  = this->direction;
        drawPos.x       = this->position.x;
        drawPos.y       = this->startPos.y;
        this->direction = 0;
        this->wheelAnimator.DrawSprite(&drawPos, false);

        this->direction = storeDir;
        this->bodyAnimator.DrawSprite(nullptr, false);

        if (!sceneInfo->currentScreenID)
            RSDKTable->AddDrawListRef(Zone::sVars->objectDrawGroup[1], sceneInfo->entitySlot);
    }
    else {
        this->clampAnimator.DrawSprite(nullptr, false);
    }
}

void Grabber::Create(void *data)
{
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];
    this->startPos  = this->position;
    this->startDir  = this->direction;
    this->active    = ACTIVE_BOUNDS;

    this->velocity.x    = !this->direction ? -0x4000 : 0x4000;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->drawFX        = FX_FLIP;
    this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->clampAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    this->wheelAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
    this->state.Set(&Grabber::State_Init);
}

void Grabber::StageLoad()
{
    sVars->aniFrames.Load("CPZ/Grabber.bin", SCOPE_STAGE);
    sVars->nullFrames.Load("", SCOPE_STAGE);

    sVars->hitboxBadnik.left   = -8;
    sVars->hitboxBadnik.top    = -8;
    sVars->hitboxBadnik.right  = 8;
    sVars->hitboxBadnik.bottom = 8;

    sVars->hitboxRange.left   = -64;
    sVars->hitboxRange.top    = 0;
    sVars->hitboxRange.right  = 64;
    sVars->hitboxRange.bottom = 128;

    sVars->hitboxGrab.left   = -8;
    sVars->hitboxGrab.top    = 0;
    sVars->hitboxGrab.right  = 8;
    sVars->hitboxGrab.bottom = 16;

    DebugMode::AddObject(sVars->classID, &Grabber::DebugSpawn, &Grabber::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);

    sVars->sfxGrab.Get("Global/Grab.wav");
    sVars->sfxDestroy.Get("Global/Destroy.wav");
}

void Grabber::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Grabber::DebugSpawn()
{
    Grabber *grabber       = GameObject::Create<Grabber>(nullptr, this->position.x, this->position.y);
    grabber->direction     = this->direction;
    grabber->startDir      = this->direction;
    grabber->velocity.x    = !this->direction ? -0x4000 : 0x4000;
}

void Grabber::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player != this->grabbedPlayer) {
            if (this->state.Matches(&Grabber::State_TryToGrab)) {
                if (player->CheckCollisionTouch(this, &sVars->hitboxGrab)) {
                    sVars->sfxGrab.Play(false, 255);
                    this->state.Set(&Grabber::State_GrabbedPlayer);
                    this->isPermanent       = true;
                    this->grabbedPlayer     = player;
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
            }

            if (player != this->grabbedPlayer && !this->grabDelay && player->CheckBadnikTouch(this, &sVars->hitboxBadnik)
                && player->CheckBadnikBreak(this, false)) {
                Player *playerPtr = this->grabbedPlayer;
                if (playerPtr)
                    playerPtr->state.Set(&Player::State_Air);
                this->Destroy();
            }
        }
    }
}

void Grabber::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position      = this->startPos;
        this->struggleDelay = 0;
        this->struggleTimer = 0;
        this->struggleFlags = 0;
        this->direction     = this->startDir;
        this->grabbedPlayer = nullptr;
        this->turnTimer     = 0;
        this->timer         = 0;
        Grabber::Create(nullptr);
    }
}

void Grabber::HandleExplode()
{
    if (!--this->turnTimer) {
        this->bodyAnimator.frameID ^= 1;
        this->turnTimer = this->timer;

        if (!--this->timer) {
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
}

void Grabber::State_Init()
{
    this->active = ACTIVE_NORMAL;
    this->state.Set(&Grabber::State_CheckForGrab);
    Grabber::State_CheckForGrab();
}

void Grabber::State_CheckForGrab()
{
    this->wheelAnimator.Process();

    if (++this->turnTimer >= 256) {
        this->turnTimer = 0;
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
        this->clampAnimator.SetAnimation(sVars->nullFrames, 0, true, 0);
        this->state.Set(&Grabber::State_HandleTurn);
    }
    this->position.x += this->velocity.x;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckCollisionTouch(this, &sVars->hitboxRange)) {
            this->timer = 16;
            this->state.Set(&Grabber::State_GrabDelay);
        }
    }

    Grabber::CheckPlayerCollisions();
    Grabber::CheckOffScreen();
}

void Grabber::State_GrabDelay()
{
    if (--this->timer < 0) {
        this->timer = 32;
        this->state.Set(&Grabber::State_TryToGrab);
    }

    Grabber::CheckPlayerCollisions();
    Grabber::CheckOffScreen();
}

void Grabber::State_HandleTurn()
{
    this->bodyAnimator.Process();

    if (this->bodyAnimator.frameID == this->bodyAnimator.frameCount - 1) {
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->clampAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        if (this->direction == FLIP_X)
            this->position.x -= 0x10000;
        else
            this->position.x += 0x10000;

        this->state.Set(&Grabber::State_CheckForGrab);
        this->velocity.x = -this->velocity.x;
        this->direction ^= FLIP_X;
        Grabber::State_CheckForGrab();
    }
    else {
        Grabber::CheckPlayerCollisions();
        Grabber::CheckOffScreen();
    }
}

void Grabber::State_TryToGrab()
{
    if (--this->timer < 0)
        this->state.Set(&Grabber::State_RiseUp);

    this->position.y += 0x20000;

    Grabber::CheckPlayerCollisions();
    Grabber::CheckOffScreen();
}

void Grabber::State_RiseUp()
{
    if (++this->timer >= 32) {
        this->timer = 0;
        this->state.Set(&Grabber::State_CheckForGrab);
    }

    this->position.y -= 0x20000;
    Grabber::CheckPlayerCollisions();
    Grabber::CheckOffScreen();
}

void Grabber::State_GrabbedPlayer()
{
    this->bodyAnimator.Process();
    this->clampAnimator.Process();

    if (++this->timer < 32) {
        this->position.y -= 0x20000;

        Player *player = this->grabbedPlayer;
        if (player) {
            player->animator.speed = 0;
            player->position.x     = this->position.x;
            player->position.y     = this->position.y + 0x100000;
            player->velocity.x     = 0;
            player->velocity.y     = 0;
            player->direction      = this->direction ^ FLIP_X;
        }
        Grabber::CheckPlayerCollisions();
    }
    else if (this->bodyAnimator.frameID != this->bodyAnimator.frameCount - 1) {
        Grabber::CheckPlayerCollisions();
    }
    else {
        this->turnTimer = 1;
        this->timer     = 16;
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
        this->state.Set(&Grabber::State_Struggle);
        Grabber::CheckPlayerCollisions();
    }

    Grabber::CheckOffScreen();
}

void Grabber::State_Struggle()
{
    Player *player = this->grabbedPlayer;
    if (player) {
        player->animator.speed = 0;
        if (this->struggleFlags) {
            player->velocity.x = 0;
            player->velocity.y = 0;
            if (this->struggleDelay) {
                this->struggleDelay--;

                uint8 struggleFlags = 0;
                if (player->left)
                    struggleFlags = 1;

                if (player->right)
                    struggleFlags |= 2;

                if (struggleFlags) {
                    if (struggleFlags != 3 && struggleFlags != this->struggleFlags) {
                        this->struggleFlags = struggleFlags;
                        if (++this->struggleTimer >= 4) {
                            player->state.Set(&Player::State_Air);
                            this->grabbedPlayer = 0;
                            this->grabDelay     = 16;
                            this->state.Set(&Grabber::State_PlayerEscaped);
                        }
                    }
                }
            }
            else {
                this->struggleTimer = 0;
                this->struggleFlags = 0;
            }
        }
        else {
            if (player->left) {
                this->struggleFlags = 1;
                this->struggleDelay = 64;
            }
            else if (player->right) {
                this->struggleFlags = 2;
                this->struggleDelay = 64;
            }
        }

        player->position.x = this->position.x;
        player->position.y = this->position.y + 0x100000;
        player->direction  = this->direction ^ FLIP_X;
    }

    Grabber::CheckPlayerCollisions();
    Grabber::HandleExplode();
    Grabber::CheckOffScreen();
}

void Grabber::State_PlayerEscaped()
{
    if (this->grabDelay)
        this->grabDelay--;

    Grabber::CheckPlayerCollisions();
    Grabber::HandleExplode();
    Grabber::CheckOffScreen();
}

#if RETRO_INCLUDE_EDITOR
void Grabber::EditorDraw()
{
    this->startPos = this->position;

    Graphics::DrawLine(this->position.x, this->startPos.y - 0x100000, this->position.x, this->position.y, 0x202020, 0, INK_NONE, false);
    Graphics::DrawLine(this->position.x - 0x10000, this->startPos.y - 0x100000, this->position.x - 0x10000, this->position.y, 0xE0E0E0, 0, INK_NONE,
                  false);

    Vector2 drawPos;
    int32 dir       = this->direction;
    drawPos.x       = this->position.x;
    drawPos.y       = this->startPos.y;
    this->direction = FLIP_NONE;
    this->wheelAnimator.DrawSprite(&drawPos, false);

    this->direction = dir;
    this->bodyAnimator.DrawSprite(nullptr, false);
    this->clampAnimator.DrawSprite(nullptr, false);
}

void Grabber::EditorLoad()
{
    sVars->aniFrames.Load("CPZ/Grabber.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left", FLIP_NONE);
    RSDK_ENUM_VAR("Right", FLIP_X);
}
#endif

void Grabber::Serialize() { RSDK_EDITABLE_VAR(Grabber, VAR_UINT8, direction); }
} // namespace GameLogic