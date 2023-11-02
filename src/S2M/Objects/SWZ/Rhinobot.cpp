// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Rhinobot Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Rhinobot.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Rhinobot);

void Rhinobot::Update() { this->state.Run(this); }

void Rhinobot::LateUpdate() {}

void Rhinobot::StaticUpdate() {}

void Rhinobot::Draw()
{
    if (this->drawDust) {
        Vector2 drawPos;
        drawPos.x = this->position.x;
        drawPos.y = this->position.y;
        if (this->state.Matches(&Rhinobot::State_Moving)) {
            if (!this->moveDir)
                drawPos.x -= 0xD0000;
            else
                drawPos.x += 0xD0000;
        }
        this->dustAnimator.DrawSprite(&drawPos, false);
    }
    this->bodyAnimator.DrawSprite(nullptr, false);
}

void Rhinobot::Create(void *data)
{
    this->visible = true;
    this->drawFX |= FX_FLIP;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->startPos      = this->position;
    this->startDir      = this->direction;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->dustAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    this->state.Set(& Rhinobot::State_Init);
}

void Rhinobot::StageLoad()
{
    sVars->aniFrames.Load("SWZ/Rhinobot.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left   = -14;
    sVars->hitboxBadnik.top    = -9;
    sVars->hitboxBadnik.right  = 23;
    sVars->hitboxBadnik.bottom = 15;

    sVars->sfxHuff.Get("Stage/Huff.wav");

    DebugMode::AddObject(sVars->classID, &Rhinobot::DebugSpawn, &Rhinobot::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Rhinobot::DebugSpawn() { GameObject::Create<Rhinobot>(nullptr, this->position.x, this->position.y); }

void Rhinobot::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

bool32 Rhinobot::CheckTileCollisions()
{
    if (this->moveDir) {
        if (!this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0x40000, 0xF0000, 8)) {
            if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_LWALL, 0, 0x100000, 0, false))
                return false;
            if (this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0xF0000, 8))
                return false;
            this->state.Set(& Rhinobot::State_Fall);
        }
    }
    else if (!this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, -0x40000, 0xF0000, 8)) {
        if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_RWALL, 0, -0x100000, 0, false))
            return false;
        if (this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0xF0000, 8))
            return false;
        this->state.Set(& Rhinobot::State_Fall);
    }

    return true;
}

void Rhinobot::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik))
            player->CheckBadnikBreak(this, true);
    }
}

void Rhinobot::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position  = this->startPos;
        this->direction = this->startDir;
        Rhinobot::Create(nullptr);
    }
}

void Rhinobot::Delay_Charge()
{
    this->drawDust = false;
    if (!this->moveDir)
        this->velocity.x = -0x40000;
    else
        this->velocity.x = 0x40000;

    this->state.Set(&Rhinobot::State_Skidding);
    this->stateDelay.Set(&Rhinobot::Delay_Skidding);
}

void Rhinobot::Delay_Skidding()
{
    this->bodyAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    this->skidDir ^= FLIP_X;
    this->acceleration = -this->acceleration;
    this->topSpeed     = -this->topSpeed;
    this->stateDelay.Set(&Rhinobot::Delay_SkidFinish);
}

void Rhinobot::Delay_SkidFinish()
{
    this->moveDir ^= FLIP_X;
    this->direction ^= FLIP_X;
    this->drawDust = false;
    this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->stateDelay.Set(&Rhinobot::Delay_Skidding);
}

void Rhinobot::State_Init()
{
    this->active     = ACTIVE_NORMAL;
    this->stateDelay.Set(&Rhinobot::Delay_Skidding);
    this->moveDir    = this->startDir;
    this->skidDir    = this->startDir;

    if (this->direction == FLIP_NONE) {
        this->acceleration = -0x1000;
        this->topSpeed     = -0x30000;
    }
    else {
        this->acceleration = 0x1000;
        this->topSpeed     = 0x30000;
    }

    this->state.Set(& Rhinobot::State_Moving);
    Rhinobot::State_Moving();
}

void Rhinobot::State_Moving()
{
    Player *player = Player::GetNearestPlayerXY();

    bool32 isRevingUp = false;
    if (abs(player->position.y - this->position.y) <= 0x200000) {
        if (abs(player->position.x - this->position.x) <= 0x600000) {
            if (this->skidDir == this->moveDir && (player->position.x > this->position.x) == this->skidDir) {
                this->state.Set(&Rhinobot::State_Idle);
                this->velocity.x = 0;
                this->timer      = 32;
                this->stateDelay.Set(&Rhinobot::Delay_Charge);
                this->drawDust   = true;
                if (this->onScreen)
                    sVars->sfxHuff.Play(false, 255);
                isRevingUp = true;
            }
        }
    }

    if (!isRevingUp) {
        if (Rhinobot::CheckTileCollisions()) {
            if (this->state.Matches(&Rhinobot::State_Moving)) {
                this->velocity.x += this->acceleration;
                this->position.x += this->velocity.x;

                if (!this->velocity.x || (this->skidDir && this->velocity.x >= this->topSpeed)
                    || (!this->skidDir && this->velocity.x <= this->topSpeed)) {
#if RETRO_USE_MOD_LOADER
                    this->stateDelay.Run(this);
#else
                    this->stateDelay();
#endif
                }
            }
        }
        else {
            this->velocity.x = 0;
            this->skidDir    = this->moveDir ^ FLIP_X;
            this->moveDir ^= FLIP_X;
            this->direction ^= FLIP_X;
            this->drawDust = false;
            this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->stateDelay.Set(&Rhinobot::Delay_Skidding);
        }

        if (((this->moveDir && !this->skidDir && this->velocity.x < 0x28000) || (!this->moveDir && this->skidDir && this->velocity.x > -0x28000))
            && !this->drawDust) {
            this->drawDust = true;
            if (this->onScreen)
                sVars->sfxHuff.Play(false, 0xFF);
        }
    }

    this->bodyAnimator.Process();
    this->dustAnimator.Process();

    Rhinobot::CheckPlayerCollisions();
    Rhinobot::CheckOffScreen();
}

void Rhinobot::State_Skidding()
{
    this->position.x += this->velocity.x;

    if (!Rhinobot::CheckTileCollisions()) {
        this->velocity.x = 0;
        this->skidDir    = this->moveDir ^ FLIP_X;
        this->moveDir ^= FLIP_X;
        this->direction ^= FLIP_X;
        this->drawDust = false;
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->stateDelay.Set(&Rhinobot::Delay_Skidding);
        this->state.Set(&Rhinobot::State_Moving);
    }

    this->bodyAnimator.Process();
    this->dustAnimator.Process();

    Rhinobot::CheckPlayerCollisions();
    Rhinobot::CheckOffScreen();
}

void Rhinobot::State_Idle()
{
    this->dustAnimator.Process();

    if (--this->timer <= 0) {
#if RETRO_USE_MOD_LOADER
        this->stateDelay.Run(this);
#else
        this->stateDelay();
#endif
    }

    Rhinobot::CheckPlayerCollisions();
    Rhinobot::CheckOffScreen();
}

void Rhinobot::State_Fall()
{
    this->dustAnimator.Process();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x3800;

    if (this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x100000, 8)) {
        this->velocity.y = 0;
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->drawDust = true;
        if (this->onScreen)
            sVars->sfxHuff.Play(false, 255);
        this->state.Set(& Rhinobot::State_Moving);
        Rhinobot::State_Moving();
    }
    else {
        Rhinobot::CheckPlayerCollisions();
        Rhinobot::CheckOffScreen();
    }
}

#if RETRO_INCLUDE_EDITOR
void Rhinobot::EditorDraw(void) { Rhinobot::Draw(); }

void Rhinobot::EditorLoad(void) { sVars->aniFrames.Load("SWZ/Rhinobot.bin", SCOPE_STAGE); }
#endif

void Rhinobot::Serialize(void) {}

} // namespace GameLogic