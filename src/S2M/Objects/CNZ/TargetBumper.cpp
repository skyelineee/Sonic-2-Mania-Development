// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TargetBumper Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "TargetBumper.hpp"
#include "Global/Zone.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/ScoreBonus.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TargetBumper);

void TargetBumper::Update() { this->state.Run(this); }

void TargetBumper::LateUpdate() {}

void TargetBumper::StaticUpdate() {}

void TargetBumper::Draw() { this->animator.DrawSprite(nullptr, false); }

void TargetBumper::Create(void *data)
{
    this->visible = true;
    this->drawFX |= FX_FLIP;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->startPos      = this->position;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x400000;
    this->updateRange.y = 0x400000;
    this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);
    this->animator.frameID = this->hitCount;
    this->state.Set(&TargetBumper::State_Idle);
}

void TargetBumper::StageLoad()
{
    sVars->aniFrames.Load("CNZ/TargetBumper.bin", SCOPE_STAGE);

    sVars->sfxTargetBumper.Get("Stage/TargetBumper.wav");

    DebugMode::AddObject(sVars->classID, &TargetBumper::DebugSpawn, &TargetBumper::DebugDraw);
}

void TargetBumper::DebugSpawn() { GameObject::Create<TargetBumper>(nullptr, this->position.x, this->position.y); }

void TargetBumper::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void TargetBumper::CheckPlayerCollisions()
{
    Hitbox hitboxBumper;
    switch (this->type) {
        case TARGETBUMP_HORIZONTAL:
            hitboxBumper.left   = -14;
            hitboxBumper.top    = -4;
            hitboxBumper.right  = 14;
            hitboxBumper.bottom = 4;
            break;

        case TARGETBUMP_VERTICAL:
            hitboxBumper.left   = -4;
            hitboxBumper.top    = -14;
            hitboxBumper.right  = 4;
            hitboxBumper.bottom = 14;
            break;

        case TARGETBUMP_DIAGONAL:
            hitboxBumper.left   = -8;
            hitboxBumper.top    = -8;
            hitboxBumper.right  = 8;
            hitboxBumper.bottom = 8;
            break;
    }

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckCollisionTouch(this, &hitboxBumper) && player->animator.animationID != Player::ANI_HURT) {
            this->curPos = this->startPos;
            this->state.Set(&TargetBumper::State_Hit);
            this->active = ACTIVE_NORMAL;

            switch (this->type) {
                case TARGETBUMP_HORIZONTAL:
                    if (player->position.y <= this->position.y) {
                        player->velocity.y = -0x70000;
                        this->curPos.y += 0x20000;
                    }
                    else {
                        player->velocity.y = 0x70000;
                        this->curPos.y -= 0x20000;
                    }
                    break;

                case TARGETBUMP_VERTICAL:
                    if (player->position.x <= this->position.x) {
                        player->velocity.x = -0x70000;
                        this->curPos.x += 0x20000;
                    }
                    else {
                        player->velocity.x = 0x70000;
                        this->curPos.x -= 0x20000;
                    }
                    break;

                case TARGETBUMP_DIAGONAL: {
                    int32 angle = 96;
                    if (this->direction)
                        angle = 32;

                    int32 ang2 = 0;
                    uint8 atan = Math::ATan2(player->velocity.x, player->velocity.y);
                    int32 ang  = atan - angle;
                    if (atan - angle >= 0)
                        ang2 = ang;
                    else
                        ang2 = -ang;

                    if (ang2 < 0x40) {
                        if (ang2 < 0x38) {
                            angle -= ang;
                            angle &= 0xFF;
                        }

                        if ((this->direction & FLIP_X))
                            this->curPos.x += 0x20000;
                        else
                            this->curPos.x -= 0x20000;

                        this->curPos.y += 0x20000;
                    }
                    else {
                        angle += 0x80;
                        if ((0x80 - ang2) < 0x38) {
                            angle -= ang;
                            angle &= 0xFF;
                        }

                        if ((this->direction & FLIP_X))
                            this->curPos.x -= 0x20000;
                        else
                            this->curPos.x += 0x20000;

                        this->curPos.y -= 0x20000;
                    }
                    player->velocity.x = -0x700 * Math::Cos256(angle);
                    player->velocity.y = -0x700 * Math::Sin256(angle);
                    break;
                }
            }

            if (player->state.Matches(&Player::State_FlyCarried))
                GameObject::Get<Player>(SLOT_PLAYER2)->flyCarryTimer = 30;

            int32 anim = player->animator.animationID;
            if (anim != Player::ANI_FLY && anim != Player::ANI_FLY_LIFT_TIRED && !player->state.Matches(&Player::State_TailsFlight)) {
                player->state.Set(&Player::State_Air);
                if (anim != Player::ANI_JUMP && anim != Player::ANI_JOG && anim != Player::ANI_RUN && anim != Player::ANI_DASH)
                    player->animator.animationID = Player::ANI_WALK;
            }

            if (player->animator.animationID != Player::ANI_FLY)
                player->groundVel = player->velocity.x;

            player->onGround       = false;
            player->tileCollisions = TILECOLLISION_DOWN;
            if (this->hitCount < 3) {
                this->hitTimer = 0;

                ScoreBonus *bonus = GameObject::Create<ScoreBonus>(nullptr, this->position.x, this->position.y);
                bonus->animator.frameID = 16;
                player->GiveScore(10);
                if (++this->hitCount < 3)
                    this->animator.frameID = this->hitCount;
            }
        }
    }
}

void TargetBumper::State_Idle() { TargetBumper::CheckPlayerCollisions(); }

void TargetBumper::State_Hit()
{
    TargetBumper::CheckPlayerCollisions();

    if (this->hitTimer == 0) {
        sVars->sfxTargetBumper.Play(false, 255);
    }

    if ((this->hitTimer & 4)) {
        this->position.x = this->startPos.x;
        this->position.y = this->startPos.y;
    }
    else {
        this->position.x = this->curPos.x;
        this->position.y = this->curPos.y;
    }

    if (++this->hitTimer == 12) {
        if (this->hitCount < 3) {
            this->position.x = this->startPos.x;
            this->position.y = this->startPos.y;
            this->hitTimer   = 0;
            this->state.Set(&TargetBumper::State_Idle);
        }
        else {
            this->Destroy();
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void TargetBumper::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);
    this->animator.frameID = this->hitCount;

    this->animator.DrawSprite(nullptr, false);
}

void TargetBumper::EditorLoad()
{
    sVars->aniFrames.Load("CNZ/TargetBumper.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Horizontal", TARGETBUMP_HORIZONTAL);
    RSDK_ENUM_VAR("Vertical", TARGETBUMP_VERTICAL);
    RSDK_ENUM_VAR("Diagonal", TARGETBUMP_DIAGONAL);

    RSDK_ACTIVE_VAR(sVars, hitCount);
    RSDK_ENUM_VAR("Three Hits", TARGETBUMP_THREE_HIT);
    RSDK_ENUM_VAR("Two Hits", TARGETBUMP_TWO_HIT);
    RSDK_ENUM_VAR("One Hit", TARGETBUMP_ONE_HIT);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip", FLIP_NONE);
    RSDK_ENUM_VAR("Flipped", FLIP_X);
}
#endif

void TargetBumper::Serialize()
{
    RSDK_EDITABLE_VAR(TargetBumper, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(TargetBumper, VAR_ENUM, hitCount);
    RSDK_EDITABLE_VAR(TargetBumper, VAR_UINT8, direction);
}

} // namespace GameLogic