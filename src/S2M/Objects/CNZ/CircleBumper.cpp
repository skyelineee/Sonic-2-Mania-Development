// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: CircleBumper Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "CircleBumper.hpp"
#include "Global/Zone.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/ScoreBonus.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(CircleBumper);

void CircleBumper::Update()
{
    this->stateMove.Run(this);
    this->stateCollide.Run(this);
}

void CircleBumper::LateUpdate() {}

void CircleBumper::StaticUpdate() {}

void CircleBumper::Draw() { this->animator.DrawSprite(&this->drawPos, false); }

void CircleBumper::Create(void *data)
{
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[1];
    this->drawPos   = this->position;
    this->active    = ACTIVE_BOUNDS;
    this->amplitude.x >>= 10;
    this->amplitude.y >>= 10;

    switch (this->type) {
        default:
        case CIRCLEBUMPER_FIXED:
            this->updateRange.x = 0x400000;
            this->updateRange.y = 0x400000;
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->stateMove.Set(&CircleBumper::Move_Fixed);
            break;

        case CIRCLEBUMPER_LINEAR:
            this->updateRange.x = (abs(this->amplitude.x) + 0x1000) << 10;
            this->updateRange.y = (abs(this->amplitude.y) + 0x1000) << 10;
            this->stateMove.Set(&CircleBumper::Move_Linear);
            break;

        case CIRCLEBUMPER_CIRCLE:
            this->updateRange.x = (abs(this->amplitude.x) + 0x1000) << 10;
            this->updateRange.y = (abs(this->amplitude.y) + 0x1000) << 10;
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->stateMove.Set(&CircleBumper::Move_Circular);
            break;

        case CIRCLEBUMPER_TRACK:
            this->updateRange.x = (abs(this->amplitude.x) + 0x2000) << 9;
            this->updateRange.y = (abs(this->amplitude.y) + 0x2000) << 9;
            this->stateMove.Set(&CircleBumper::Move_Track);
            break;

        case CIRCLEBUMPER_PATH:
            this->updateRange.x = 0x400000;
            this->updateRange.y = 0x400000;
            this->active        = ACTIVE_NEVER;
            this->stateMove.Set(&CircleBumper::Move_Path);
            break;
    }

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->stateCollide.Set(&CircleBumper::Collide_Normal);
    if (!this->hitCount)
        this->hitCount = 5;
}

void CircleBumper::StageLoad()
{
    sVars->aniFrames.Load("CNZ/CircleBumper.bin", SCOPE_STAGE);

    sVars->hitboxBumper.left   = -7;
    sVars->hitboxBumper.top    = -6;
    sVars->hitboxBumper.right  = 7;
    sVars->hitboxBumper.bottom = 6;

    sVars->sfxBumper.Get("Stage/Bumper.wav");

    DebugMode::AddObject(sVars->classID, &CircleBumper::DebugSpawn, &CircleBumper::DebugDraw);
}

void CircleBumper::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void CircleBumper::DebugSpawn() { GameObject::Create<CircleBumper>(nullptr, this->position.x, this->position.y); }

void CircleBumper::CheckPlayerCollisions()
{
    int32 storeX = this->position.x;
    int32 storeY = this->position.y;

    this->position.x = this->drawPos.x;
    this->position.y = this->drawPos.y;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->animator.animationID != Player::ANI_HURT && player->CheckBadnikTouch(this, &sVars->hitboxBumper)) {
            this->animator.frameID = 0;
            this->stateCollide.Set(&CircleBumper::Collide_Bumped);
            sVars->sfxBumper.Play(false, 0xFF);
            this->active = ACTIVE_NORMAL;

            int32 angle = Math::ATan2(player->position.x - this->position.x, player->position.y - this->position.y);
            int32 xVel  = 0x700 * Math::Cos256(angle);
            int32 yVel  = 0x700 * Math::Sin256(angle);
            if (player->state.Matches(&Player::State_FlyCarried))
                GameObject::Get<Player>(SLOT_PLAYER2)->flyCarryTimer = 30;

            int32 anim = player->animator.animationID;
            if (anim != Player::ANI_FLY && anim != Player::ANI_FLY_LIFT_TIRED && !player->state.Matches(&Player::State_TailsFlight)) {
                if (!player->state.Matches(&Player::State_DropDash))
                    player->state.Set(&Player::State_Air);
                if (anim != Player::ANI_JUMP && anim != Player::ANI_JOG && anim != Player::ANI_RUN && anim != Player::ANI_DASH)
                    player->animator.animationID = Player::ANI_WALK;
            }
            if (player->animator.animationID != Player::ANI_FLY) {
                player->velocity.x   = xVel;
                player->groundVel    = xVel;
                player->applyJumpCap = false;
            }

            player->velocity.y     = yVel;
            player->onGround       = false;
            player->tileCollisions = TILECOLLISION_DOWN;

            if (this->hitCount) {
                ScoreBonus *bonus = GameObject::Create<ScoreBonus>(nullptr, this->position.x, this->position.y);
                bonus->animator.frameID = 16;
                player->GiveScore(10);
                --this->hitCount;
            }
        }
    }

    this->position.x = storeX;
    this->position.y = storeY;
}

void CircleBumper::Collide_Normal() { CircleBumper::CheckPlayerCollisions(); }

void CircleBumper::Collide_Bumped()
{
    CircleBumper::CheckPlayerCollisions();

    this->animator.Process();

    if (this->animator.frameID == this->animator.frameCount - 1) {
        this->animator.frameID = 0;
        this->active           = ACTIVE_BOUNDS;
        this->stateCollide.Set(&CircleBumper::Collide_Normal);
    }
}

void CircleBumper::Move_Fixed()
{
    this->drawPos.x = this->position.x;
    this->drawPos.y = this->position.y;
}

void CircleBumper::Move_Linear()
{
    this->drawPos.x = this->position.x + this->amplitude.x * Math::Sin1024(this->speed * Zone::sVars->timer);
    this->drawPos.y = this->position.y + this->amplitude.y * Math::Sin1024(this->speed * Zone::sVars->timer);
}

void CircleBumper::Move_Circular()
{
    this->drawPos.x = this->position.x + this->amplitude.x * Math::Cos1024(this->speed * Zone::sVars->timer + 4 * this->angle);
    this->drawPos.y = this->position.y + this->amplitude.y * Math::Sin1024(this->speed * Zone::sVars->timer + 4 * this->angle);
}

void CircleBumper::Move_Path()
{
    this->drawPos.x += this->velocity.x;
    this->drawPos.y += this->velocity.y;
    //Entity *node = RSDK_GET_ENTITY_GEN(this->speed);
    Entity *node = GameObject::Get<CircleBumper>(this->speed);

    if (this->velocity.x <= 0) {
        if (this->drawPos.x < node->position.x)
            this->drawPos.x = node->position.x;
    }
    else if (this->drawPos.x > node->position.x) {
        this->drawPos.x = node->position.x;
    }

    if (this->velocity.y <= 0) {
        if (this->drawPos.y < node->position.y)
            this->drawPos.y = node->position.y;
    }
    else if (this->drawPos.y > node->position.y) {
        this->drawPos.y = node->position.y;
    }
}

void CircleBumper::Move_Track()
{
    int32 timer = Zone::sVars->timer << 7;
    if (((timer >> 16) & 1) == this->direction) {
        this->drawPos.x = this->position.x + (timer * this->amplitude.x >> 6) - (this->amplitude.x << 15);
        this->drawPos.y = this->position.y + (timer * this->amplitude.y >> 6) - (this->amplitude.y << 15);
    }
    else {
        this->drawPos.x = this->position.x + (this->amplitude.x << 15) - (timer * this->amplitude.x >> 6);
        this->drawPos.y = this->position.y + (this->amplitude.y << 15) - (timer * this->amplitude.y >> 6);
    }
}

#if RETRO_INCLUDE_EDITOR
void CircleBumper::EditorDraw()
{
    Vector2 amplitude;
    amplitude.x = this->amplitude.x >> 10;
    amplitude.y = this->amplitude.y >> 10;
    switch (this->type) {
        default:
        case CIRCLEBUMPER_FIXED:
            this->updateRange.x = 0x400000;
            this->updateRange.y = 0x400000;
            break;

        case CIRCLEBUMPER_LINEAR:
            this->updateRange.x = (abs(amplitude.x) + 0x1000) << 10;
            this->updateRange.y = (abs(amplitude.y) + 0x1000) << 10;
            break;

        case CIRCLEBUMPER_CIRCLE:
            this->updateRange.x = (abs(amplitude.x) + 0x1000) << 10;
            this->updateRange.y = (abs(amplitude.y) + 0x1000) << 10;
            break;

        case CIRCLEBUMPER_TRACK:
            this->updateRange.x = (abs(amplitude.x) + 0x2000) << 9;
            this->updateRange.y = (abs(amplitude.y) + 0x2000) << 9;
            break;

        case CIRCLEBUMPER_PATH:
            this->updateRange.x = 0x400000;
            this->updateRange.y = 0x400000;
            break;
    }

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->drawPos = this->position;

    CircleBumper::Draw();

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        Vector2 pos;
        switch (this->type) {
            case CIRCLEBUMPER_LINEAR:
                this->updateRange.x = (abs(amplitude.x) + 0x1000) << 10;
                this->updateRange.y = (abs(amplitude.y) + 0x1000) << 10;

                // draw distance previews
                this->inkEffect = INK_BLEND;
                this->drawPos.x = amplitude.x * Math::Sin1024(0x100) + this->position.x;
                this->drawPos.y = amplitude.y * Math::Sin1024(0x100) + this->position.y;
                pos             = this->drawPos;
                CircleBumper::Draw();

                this->drawPos.x = amplitude.x * Math::Sin1024(0x300) + this->position.x;
                this->drawPos.y = amplitude.y * Math::Sin1024(0x300) + this->position.y;
                CircleBumper::Draw();

                Graphics::DrawLine(pos.x, pos.y, this->drawPos.x, this->drawPos.y, 0x00FF00, 0, INK_NONE, false);
                break;

            case CIRCLEBUMPER_CIRCLE:
                this->updateRange.x = (abs(amplitude.x) + 0x1000) << 10;
                this->updateRange.y = (abs(amplitude.y) + 0x1000) << 10;

                // draw distance preview
                this->inkEffect = INK_BLEND;
                this->drawPos.x = amplitude.x * Math::Cos1024(4 * this->angle) + this->position.x;
                this->drawPos.y = amplitude.y * Math::Sin1024(4 * this->angle) + this->position.y;

                CircleBumper::Draw();
                break;

            case CIRCLEBUMPER_TRACK:
                this->updateRange.x = (abs(amplitude.x) + 0x2000) << 9;
                this->updateRange.y = (abs(amplitude.y) + 0x2000) << 9;

                // draw distance preview
                this->inkEffect = INK_BLEND;

                // draw distance previews
                this->inkEffect = INK_BLEND;
                this->drawPos.x = this->position.x + (0x100 * amplitude.x >> 6) - (amplitude.x << 15);
                this->drawPos.y = this->position.y + (0x100 * amplitude.y >> 6) - (amplitude.y << 15);
                pos             = this->drawPos;
                CircleBumper::Draw();

                this->drawPos.x = this->position.x + (amplitude.x << 15) - (0x000 * amplitude.x >> 6);
                this->drawPos.y = this->position.y + (amplitude.y << 15) - (0x000 * amplitude.y >> 6);
                CircleBumper::Draw();

                Graphics::DrawLine(pos.x, pos.y, this->drawPos.x, this->drawPos.y, 0x00FF00, 0x00, INK_NONE, false);
                break;

            case CIRCLEBUMPER_PATH: {
                //Entity *target = RSDK_GET_ENTITY_GEN(this->speed);
                Entity *target = GameObject::Get<CircleBumper>(this->speed);

                if (target)
                    DrawHelpers::DrawArrow(this->position.x, this->position.y, target->position.x, target->position.y, 0x00FF00, INK_NONE, 0xFF);
                break;
            }

            default:
            case CIRCLEBUMPER_FIXED:
                // nothin'
                break;
        }

        this->inkEffect = INK_NONE;
        RSDK_DRAWING_OVERLAY(false);
    }
}

void CircleBumper::EditorLoad()
{
    sVars->aniFrames.Load("CNZ/CircleBumper.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Fixed", CIRCLEBUMPER_FIXED);
    RSDK_ENUM_VAR("Linear", CIRCLEBUMPER_LINEAR);
    RSDK_ENUM_VAR("Circle", CIRCLEBUMPER_CIRCLE);
    RSDK_ENUM_VAR("Track", CIRCLEBUMPER_TRACK);
    RSDK_ENUM_VAR("Path", CIRCLEBUMPER_PATH);
}
#endif

void CircleBumper::Serialize()
{
    RSDK_EDITABLE_VAR(CircleBumper, VAR_ENUM, hitCount);
    RSDK_EDITABLE_VAR(CircleBumper, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(CircleBumper, VAR_VECTOR2, amplitude);
    RSDK_EDITABLE_VAR(CircleBumper, VAR_ENUM, speed);
    RSDK_EDITABLE_VAR(CircleBumper, VAR_INT32, angle);
}

} // namespace GameLogic