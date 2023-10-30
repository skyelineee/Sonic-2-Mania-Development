// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Dragonfly Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Dragonfly.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Dragonfly);

void Dragonfly::Update() { this->state.Run(this); }

void Dragonfly::LateUpdate() {}

void Dragonfly::StaticUpdate() {}

void Dragonfly::Draw()
{
    if (this->animator.animationID == 3) {
        this->animator.DrawSprite(nullptr, false);
    }
    else {
        for (int32 i = 0; i < DRAGONFLY_SPINE_COUNT; ++i) {
            this->bodyAnimator.frameID = i == 0;
            this->direction            = this->directions[i];
            this->bodyAnimator.DrawSprite(&this->positions[i], false);
        }

        this->direction = false;
        this->animator.DrawSprite(nullptr, false);

        this->inkEffect = INK_ALPHA;
        this->wingAnimator.DrawSprite(nullptr, false);

        this->inkEffect = INK_NONE;
    }
}

void Dragonfly::Create(void *data)
{
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];

    if (!sceneInfo->inEditor) {
        if (!this->speed) {
            this->dist  = 0x40;
            this->speed = 0x04;
        }
        this->drawFX |= FX_FLIP;

        if (data) {
            this->active        = ACTIVE_NORMAL;
            this->updateRange.x = 0x100000;
            this->updateRange.y = 0x100000;
            this->animator.SetAnimation(sVars->aniFrames, 3, true, 0);
            this->state.Set(&Dragonfly::State_Debris);
        }
        else {
            this->active        = ACTIVE_BOUNDS;
            this->updateRange.x = 0x800000;
            this->updateRange.y = (this->dist + 0x80) << 16;
            this->startPos      = this->position;
            this->alpha         = 0x80;

            this->wingAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
            this->bodyAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
            this->state.Set(&Dragonfly::State_Init);
        }
    }
}

void Dragonfly::StageLoad()
{
    sVars->aniFrames.Load("SWZ/Dragonfly.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left   = -8;
    sVars->hitboxBadnik.top    = -8;
    sVars->hitboxBadnik.right  = 8;
    sVars->hitboxBadnik.bottom = 8;

    sVars->hitboxSpine.left   = -5;
    sVars->hitboxSpine.top    = -3;
    sVars->hitboxSpine.right  = 5;
    sVars->hitboxSpine.bottom = 2;

    DebugMode::AddObject(sVars->classID, &Dragonfly::DebugSpawn, &Dragonfly::DebugDraw);
}

void Dragonfly::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

// rdc would probably not shut the fuck up if i didn't add this /hj
// hi chuli
void Dragonfly::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position.x = this->startPos.x;
        this->position.y = this->startPos.y;
        Dragonfly::Create(nullptr);
    }
}

void Dragonfly::DebugSpawn() { GameObject::Create<Dragonfly>(nullptr, this->position.x, this->position.y); }

void Dragonfly::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik)) {
            if (player->CheckBadnikBreak(this, false)) {
                for (int32 i = 0; i < DRAGONFLY_SPINE_COUNT; ++i) {
                    Dragonfly *child        = GameObject::Create<Dragonfly>(INT_TO_VOID(true), this->positions[i].x, this->positions[i].y);
                    child->animator.frameID = i == 0;
                    child->velocity.x       = Math::Rand(-4, 4) << 15;
                    child->velocity.y       = Math::Rand(-5, 1) << 15;
                }

                this->Destroy();
            }
        }
        else {
            Vector2 storePos = this->position;
            for (int32 i = 0; i < DRAGONFLY_SPINE_COUNT; ++i) {
                this->position = this->positions[i];
                if (player->CheckCollisionTouch(this, &sVars->hitboxSpine)) {
                    this->position = storePos;
                    player->Hurt(this);
                }
            }

            this->position = storePos;
        }
    }
}

void Dragonfly::State_Init()
{
    this->active = ACTIVE_NORMAL;

    if (!this->dir) {
        this->angle = 0;
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    }
    else {
        this->angle = 0x200;
        this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);
    }

    this->state.Set(&Dragonfly::State_Move);
    Dragonfly::State_Move();
}

void Dragonfly::State_Move()
{
    this->angle += this->speed;

    this->position.x = this->startPos.x + 0xC00 * Math::Cos256(this->angle + 0x40);
    this->position.y = this->startPos.y + (this->dist << 6) * Math::Sin1024(this->angle);

    int32 currentAngle = this->angle - (((DRAGONFLY_SPINE_COUNT * 13) + 13) - 1);
    for (int32 i = 0; i < DRAGONFLY_SPINE_COUNT; ++i) {
        this->directions[i] = ((currentAngle + 0x100) & 0x3FF) < 0x200 ? 2 : 0;

        this->positions[i].x = this->startPos.x + 0xC00 * Math::Cos256(currentAngle + 0x40);
        this->positions[i].y = this->startPos.y + (this->dist << 6) * Math::Sin1024(currentAngle);

        currentAngle += 13;
    }

    bool32 dir = ((this->angle + 0x100) & 0x3FF) < 0x200;
    if (this->animator.animationID == 1) {
        if (dir)
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    }
    else if (!dir)
        this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);

    this->animator.Process();
    this->wingAnimator.Process();

    Dragonfly::CheckPlayerCollisions();
    Dragonfly::CheckOffScreen();
}

void Dragonfly::State_Debris()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x3800;

    this->visible ^= true;

    if (!this->CheckOnScreen(nullptr))
        this->Destroy();
}

#if RETRO_INCLUDE_EDITOR
void Dragonfly::EditorDraw()
{
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = (this->dist + 0x80) << 16;
    this->startPos      = this->position;
    this->alpha         = 0x80;

    this->wingAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    this->bodyAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);

    if (!this->dir) {
        this->angle = 0;
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 4);
    }
    else {
        this->angle = 0x200;
        this->animator.SetAnimation(sVars->aniFrames, 1, true, 4);
    }

    this->position.x = this->startPos.x + 0xC00 * Math::Cos256(this->angle + 0x40);
    this->position.y = this->startPos.y + (this->dist << 6) * Math::Sin1024(this->angle);

    int32 currentAngle = this->angle - (((DRAGONFLY_SPINE_COUNT * 13) + 13) - 1);
    for (int32 i = 0; i < DRAGONFLY_SPINE_COUNT; ++i) {
        this->directions[i] = ((currentAngle + 0x100) & 0x3FF) < 0x200 ? 2 : 0;

        this->positions[i].x = this->startPos.x + 0xC00 * Math::Cos256(currentAngle + 0x40);
        this->positions[i].y = this->startPos.y + (this->dist << 6) * Math::Sin1024(currentAngle);
        currentAngle += 13;
    }

    Dragonfly::Draw();

    this->position = this->startPos;
}

void Dragonfly::EditorLoad()
{
    sVars->aniFrames.Load("SWZ/Dragonfly.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, dir);
    RSDK_ENUM_VAR("Down", FLIP_NONE);
    RSDK_ENUM_VAR("Up", FLIP_X);
}
#endif

void Dragonfly::Serialize()
{
    RSDK_EDITABLE_VAR(Dragonfly, VAR_UINT8, dir);
    RSDK_EDITABLE_VAR(Dragonfly, VAR_UINT8, dist);
    RSDK_EDITABLE_VAR(Dragonfly, VAR_UINT8, speed);
}

} // namespace GameLogic