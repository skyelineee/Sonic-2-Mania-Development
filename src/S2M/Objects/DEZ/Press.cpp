// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Press Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Press.hpp"
#include "Crate.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Helpers/MathHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Press);

void Press::Update() { this->state.Run(this); }

void Press::LateUpdate() {}

void Press::StaticUpdate()
{
    sVars->canSuper      = true;
    Player *player = GameObject::Get<Player>(SLOT_PLAYER1);

    if (player->classID == Player::sVars->classID) {
        for (auto press : GameObject::GetEntities<Press>(FOR_ACTIVE_ENTITIES))
        {
            Hitbox hitboxRange;
            hitboxRange.top    = -4 * press->size;
            hitboxRange.bottom = 4 * press->size;
            hitboxRange.left   = -112;
            hitboxRange.right  = 112;
            if (player->CheckCollisionTouch(press, &hitboxRange)) {
                sVars->canSuper = false;
                break;
            }
        }
    }
    else
        sVars->canSuper = false;
}

void Press::Draw()
{
    if (this->rotation <= 0x100)
        Press::DrawHandle();

    // threads
    Vector2 drawPos    = this->drawPos;
    SpriteFrame *frame = sVars->aniFrames.GetFrame(1, this->threadAnimator.frameID);
    frame->height      = 56;
    frame->sprY        = (this->threadSprY >> 8) + 182;
    for (uint32 i = 0; i < this->threads; ++i) {
        this->threadAnimator.DrawSprite(&drawPos, false);
        drawPos.y += 0x380000;
    }
    frame->height = this->height;
    this->threadAnimator.DrawSprite(&drawPos, false);

    // crusher platforms
    drawPos = this->position;
    drawPos.y += this->offTop;
    this->crusherAnimator.frameID = 3;
    this->crusherAnimator.DrawSprite(&drawPos, false);

    this->crusherAnimator.frameID = 4;
    this->crusherAnimator.DrawSprite(&drawPos, false);

    drawPos.y -= this->offTop;
    drawPos.y += this->offBottom;
    this->crusherAnimator.frameID = 3;
    this->crusherAnimator.DrawSprite(&drawPos, false);

    this->crusherAnimator.frameID = 5;
    this->crusherAnimator.DrawSprite(&drawPos, false);

    // bumper
    drawPos.y = this->drawPos.y - 0x80000;
    this->bumperAnimator.DrawSprite(&drawPos, false);

    drawPos.y += (this->size + 16) << 16;
    this->bumperAnimator.DrawSprite(&drawPos, false);

    if (this->rotation > 0x100)
        Press::DrawHandle();
}

void Press::Create(void *data)
{
    this->active    = ACTIVE_BOUNDS;
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];

    if (!sceneInfo->inEditor) {
        this->size *= 8;
        int32 size = this->size;
        this->speed <<= 15;
        int32 step    = size / 7;
        int32 count   = (size - step) / 2;
        this->threads = (step + count) / 32;
        this->scale.y = 0x200;
        this->height  = size - 0x38 * this->threads;

        size <<= 15;
        this->updateRange.x = 0x1000000;
        this->updateRange.y = 0x380000 * this->threads + 0x1000000;
        this->drawPos.x     = this->position.x;
        this->drawPos.y     = this->position.y - size;
        this->offTop        = (this->offTop << 16) - size;
        this->offBottom     = (this->offBottom << 16) - size + 0xFFFF;

        this->crusherAnimator.SetAnimation(sVars->aniFrames, 0,true, 0);
        this->threadAnimator.SetAnimation(sVars->aniFrames, 1,true, 0);
        this->bumperAnimator.SetAnimation(sVars->aniFrames, 2,true, 0);
        this->state.Set(&Press::State_Crush);
    }
}

void Press::StageLoad()
{
    sVars->aniFrames.Load("DEZ/Press.bin", SCOPE_STAGE);

    sVars->hitbox.left   = -112;
    sVars->hitbox.top    = -16;
    sVars->hitbox.right  = 112;
    sVars->hitbox.bottom = 16;

    Player::sVars->canSuperCB = Press::CheckCanSuper;

    sVars->sfxImpact.Get("Stage/Impact2.wav");
    sVars->sfxPress.Get("DEZ/Press.wav");
}

bool32 Press::CheckCanSuper(bool32 isHUD) { return sVars->canSuper; }

void Press::DrawHandle()
{
    Vector2 drawPos = this->drawPos;

    this->scale.x = abs(Math::Cos512(this->rotation)) + 1;
    int32 scaleX  = abs(Math::Sin512(this->rotation)) + 1;

    drawPos.x += 0x2500 * Math::Cos512(this->rotation);
    drawPos.y -= 0x80000;
    this->crusherAnimator.frameID = 0;
    this->crusherAnimator.DrawSprite(&drawPos, false);

    this->drawFX |= FX_SCALE;
    drawPos.x = 0x1B80 * Math::Cos512(this->rotation) + this->drawPos.x;

    switch (this->rotation >> 7) {
        case 0:
        case 2:
            drawPos.x += (scaleX << 9);
            this->crusherAnimator.frameID = 2;
            this->crusherAnimator.DrawSprite(&drawPos, false);

            drawPos.x += -0xC00 * this->scale.x - (scaleX << 9);
            break;

        case 1:
        case 3:
            drawPos.x -= (scaleX << 9);
            this->crusherAnimator.frameID = 2;
            this->crusherAnimator.DrawSprite(&drawPos, false);

            drawPos.x += (scaleX + 2 * (3 * this->scale.x - 32)) << 9;
            break;

        default: break;
    }

    this->scale.x                 = scaleX;
    this->crusherAnimator.frameID = 1;
    this->crusherAnimator.DrawSprite(&drawPos, false);

    this->drawFX &= ~FX_SCALE;
}

void Press::Move()
{
    this->stoodPlayersRoof  = 0;
    this->stoodPlayersFloor = 0;

    int32 playerID = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        this->position.y += this->offBottom;

        if (player->CheckCollisionBox(this, &sVars->hitbox) == C_TOP) {
            if (this->state.Matches(&Press::State_Crush) && !player->sidekick) {
                if (abs(this->position.x - player->position.x) <= 0x600000) {
                    sVars->sfxPress.Play(false, 255);
                    this->state.Set(&Press::HandleMovement);
                    this->active = ACTIVE_NORMAL;
                }
            }

            this->stoodPlayersFloor |= 1 << playerID;
        }

        this->position.y += this->offTop - this->offBottom;

        int32 collide = player->CheckCollisionBox(this, &sVars->hitbox);
        if (collide == C_BOTTOM)
            player->collisionFlagV |= 2;
        else if (collide == C_TOP)
            this->stoodPlayersRoof |= 1 << playerID;

        ++playerID;
        this->position.y -= this->offTop;
    }
}

void Press::State_Crush()
{
    // fun press fact!
    Press::Move();
}
void Press::State_FinalCrush()
{
    // every second you don't move the press only gets closer
    Press::Move();
}
void Press::HandleMovement()
{
    this->threadAnimator.Process();
    this->bumperAnimator.Process();

    Press::Move();

    int32 oldBottom = this->offBottom;
    int32 newBottom = oldBottom - this->speed;
    this->offBottom = oldBottom - this->speed;

    this->threadSprY += (this->speed >> 11);
    this->threadSprY &= 0x7FF;

    int32 oldTop = this->offTop;
    int32 newTop = this->speed + this->offTop + this->topOffset;

    this->rotation  = (this->rotation - (this->speed >> 15)) & 0x1FF;
    this->offTop    = newTop;
    this->topOffset = 0;

    if (newTop + 0x100000 >= newBottom - 0x100000) {
        int32 diff = newTop - newBottom + 0x200000;
        if (diff > 0) {
            diff >>= 1;
            this->offTop    = newTop - diff;
            this->offBottom = newBottom + diff;
        }

        sVars->sfxImpact.Play(false, 255);
        this->active = ACTIVE_BOUNDS;
        Camera::ShakeScreen(0, 0, 5, 0, 0);
        this->state.Set(&Press::State_FinalCrush);
    }

    uint32 waitTime = 0;
    this->topOffset = this->offTop;

    bool32 top = false, bottom = false;
    int32 floorOffset = (oldBottom & 0xFFFF0000) - (this->offBottom & 0xFFFF0000);
    int32 actualPos   = this->position.y;

    for (auto crate : GameObject::GetEntities<Crate>(FOR_ACTIVE_ENTITIES))
    {
        this->position.y += this->offBottom;

        if (this->CheckCollisionBox(&sVars->hitbox, crate, &crate->hitbox) == C_TOP) {
            bottom = true;
            Crate::MoveY(crate, -floorOffset);
        }

        this->position.y += this->offTop - this->offBottom;
        if (crate->CheckCollisionBox(&crate->hitbox, this, &sVars->hitbox) == C_TOP) {
            top = true;

            switch (crate->frameID) {
                default: break;
                case 0: waitTime += 60; break;
                case 1: waitTime += 30; break;
                case 2: waitTime += 90; break;
                case 3: waitTime += 60; break;
            }
        }

        this->offTop     = this->position.y - actualPos;
        this->position.y = actualPos;
    }

    this->topOffset -= this->offTop;

    if (bottom && top) {
        sVars->sfxImpact.Play(false, 255);
        Camera::ShakeScreen(0, 0, 3, 0, 0);

        this->state.Set(&Press::State_HandleCrates);
        this->timerStart = waitTime;
        this->timer      = waitTime;
    }

    int32 playerID   = 1;
    int32 roofOffset = (oldTop & 0xFFFF0000) - (this->offTop & 0xFFFF0000);
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (playerID & this->stoodPlayersFloor)
            player->position.y -= floorOffset;
        if (playerID & this->stoodPlayersRoof)
            player->position.y -= roofOffset;

        playerID <<= 1;
    }
}

void Press::State_HandleCrates()
{
    Press::Move();

    if (this->timer <= 0) {
        for (auto crate : GameObject::GetEntities<Crate>(FOR_ACTIVE_ENTITIES))
        {
            this->position.y += this->offTop + 0x80000;

            if (this->CheckCollisionTouchBox(&sVars->hitbox, crate, &crate->hitbox))
                Crate::Break(crate);

            this->position.y += -0x80000 - this->offTop;
        }

        this->state.Set(&Press::HandleMovement);
    }
    else {
        int32 percentDone = ((this->timerStart - this->timer) << 16) / this->timerStart;
        int32 crateOff    = 0;

        for (auto crate : GameObject::GetEntities<Crate>(FOR_ACTIVE_ENTITIES))
        {
            this->position.y += this->offTop + 0x80000;
            if (this->CheckCollisionTouchBox(&sVars->hitbox, crate, &crate->hitbox) && percentDone > 0x8000) {
                int32 percent  = percentDone >> 9;
                int32 percent2 = percent * percent;
                int32 angle    = crateOff + ((percent * this->timerStart * (percent2 >> 8)) >> 8);

                crateOff += 0x100;
                crate->drawPos.x = (Math::Sin512(angle & 0x1FF) << 7) + crate->centerPos.x;
            }

            this->position.y += -0x80000 - this->offTop;
        }

        --this->timer;
    }
}

#if RETRO_INCLUDE_EDITOR
void Press::EditorDraw()
{
    int32 sizeStore = this->size;
    int32 offT      = this->offTop;
    int32 offB      = this->offBottom;

    this->size *= 8;
    int32 size = this->size;

    int32 step    = size / 7;
    int32 count   = (size - step) / 2;
    this->threads = (step + count) / 32;
    this->scale.y = 0x200;
    this->height  = size - 0x38 * this->threads;

    size <<= 15;
    this->updateRange.x = 0x1000000;
    this->updateRange.y = 0x380000 * this->threads + 0x1000000;
    this->drawPos.x     = this->position.x;
    this->drawPos.y     = this->position.y - size;
    this->offTop        = (this->offTop << 16) - size;
    this->offBottom     = (this->offBottom << 16) - size + 0xFFFF;

    this->crusherAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->threadAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    this->bumperAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);

    Press::Draw();

    this->size      = sizeStore;
    this->offTop    = offT;
    this->offBottom = offB;
}

void Press::EditorLoad() { sVars->aniFrames.Load("DEZ/Press.bin", SCOPE_STAGE); }
#endif

void Press::Serialize()
{
    RSDK_EDITABLE_VAR(Press, VAR_UINT16, size);
    RSDK_EDITABLE_VAR(Press, VAR_UINT32, speed);
    RSDK_EDITABLE_VAR(Press, VAR_ENUM, offTop);
    RSDK_EDITABLE_VAR(Press, VAR_ENUM, offBottom);
}
} // namespace GameLogic