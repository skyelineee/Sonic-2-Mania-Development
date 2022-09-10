// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Buzzer Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "Buzzer.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Buzzer);

void Buzzer::Update() { this->state.Run(this); }

void Buzzer::LateUpdate() {}

void Buzzer::StaticUpdate() {}

void Buzzer::Draw()
{
    this->animator.DrawSprite(NULL, false);
    this->thrustAnimator.DrawSprite(NULL, false);
}

void Buzzer::Create(void *data)
{
    this->visible = true;
    this->drawFX |= FX_FLIP;
    this->drawGroup      = Zone::sVars->objectDrawGroup[0];
    this->startPos.x     = this->position.x;
    this->startPos.y     = this->position.y;
    this->startDir       = this->direction;
    this->timer          = 128;
    this->detectedPlayer = false;
    this->projectile     = NULL;

    if (!this->shotRange)
        this->shotRange = 96;

    this->hitboxRange.right = this->shotRange;
    this->hitboxRange.left = this->shotRange;
    this->hitboxRange.top = -256;
    this->hitboxRange.bottom = 256;

    if (data) {
        this->active = ACTIVE_NORMAL;
        this->updateRange.x = 2;
        this->updateRange.y = 2;
        this->animator.SetAnimation(sVars->aniFrames, 3, true, 0);
        this->thrustAnimator.SetAnimation(sVars->aniFrames, -1, true, 0);
    }
    else {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 8;
        this->updateRange.y = 8;
        this->animator.SetAnimation(sVars->aniFrames, Flying, true, 0);
        this->thrustAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
        this->state.Set(&Buzzer::State_Init);
    }
}

void Buzzer::StageLoad()
{
    if (Stage::CheckSceneFolder("EHZ"))
        sVars->aniFrames.Load("EHZ/Buzzer.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left = -16;
    sVars->hitboxBadnik.top = -12;
    sVars->hitboxBadnik.right  = 16;
    sVars->hitboxBadnik.bottom = 12;

    sVars->hitboxProjectile.left = -6;
    sVars->hitboxProjectile.top   = -6;
    sVars->hitboxProjectile.right  = 6;
    sVars->hitboxProjectile.bottom = 6;

    DebugMode::AddObject(sVars->classID, &Buzzer::DebugDraw, &Buzzer::DebugSpawn);

}

void Buzzer::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
    DebugMode::sVars->animator.DrawSprite(NULL, false);
}

void Buzzer::DebugSpawn()
{
    Buzzer *buzzer = GameObject::Create<Buzzer>(nullptr, this->position.x, this->position.y);
    buzzer->direction = this->direction;
    buzzer->startDir  = this->startDir;
}

void Buzzer::State_Init()
{
    this->active = ACTIVE_NORMAL;
    if (!(this->direction & FLIP_X)) {
        this->velocity.x = -0x10000;
    }
    else {
        this->velocity.x = 0x10000;
    }

    this->state.Set(&Buzzer::State_Flying);
}

void Buzzer::State_Flying()
{
    this->active = ACTIVE_NORMAL;
    if (this->timer < 256) {
        this->timer++;
        this->position.x += this->velocity.x;
    }
    else {
        this->timer = 0;
        this->state.Set(&Buzzer::State_Idle);
        this->animator.SetAnimation(&sVars->aniFrames, Flying, true, 0);
        this->direction ^= 1;
    }
}

void Buzzer::State_Idle()
{
    if (this->shootTimer < 30) {
        this->shootTimer++;
    }
    else {
        this->shootTimer = 0;
        this->state.Set(&Buzzer::State_Flying);
        this->animator.SetAnimation(&sVars->aniFrames, Flying, true, 0);
        this->hasShot = false;
    }
}

void Buzzer::State_Shooting()
{
    if (this->shootTimer < 50) {
        this->shootTimer++;
        if (this->shootTimer == 30) {
            Buzzer *projectile    = GameObject::Create<Buzzer>(nullptr, this->position.x, this->position.y);
            projectile->direction = this->direction;
            if (this->direction == 0) {
                projectile->position.x += 0xD0000;
                projectile->velocity.x = -0x18000;
            }
            else {
                projectile->position.x += 0xD0000;
                projectile->velocity.x = -0x18000;
            }
            projectile->position.y += 0x180000;
            projectile->velocity.y = 0x18000;
        }
        else {
            this->shootTimer = 0;
            this->state.Set(&Buzzer::State_Flying);
            this->animator.SetAnimation(&sVars->aniFrames, Flying, true, 0);
            this->hasShot = true;
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void Buzzer::EditorDraw(void)
{
    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        this->hitboxRange.right  = this->shotRange;
        this->hitboxRange.left   = -this->shotRange;
        this->hitboxRange.top    = -256;
        this->hitboxRange.bottom = 256;

        DrawHelpers::DrawHitboxOutline(this->position.x, this->position.y, &this->hitboxRange, FLIP_NONE, 0xFF0000);

        RSDK_DRAWING_OVERLAY(false);
    }
}

void Buzzer::EditorLoad(void)
{
    if (Stage::CheckSceneFolder("EHZ"))
        sVars->aniFrames.Load("EHZ/Buzzer.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left", FLIP_NONE);
    RSDK_ENUM_VAR("Right", FLIP_X);
}
#endif

void Buzzer::Serialize()
{
    RSDK_EDITABLE_VAR(Buzzer, VAR_UINT8, direction);
}
} // namespace GameLogic