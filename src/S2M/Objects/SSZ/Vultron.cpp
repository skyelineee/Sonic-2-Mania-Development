// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Vultron Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Vultron.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Vultron);

void Vultron::Update() { this->state.Run(this); }

void Vultron::LateUpdate(void) {}

void Vultron::StaticUpdate(void) {}

void Vultron::Draw()
{
    this->bodyAnimator.DrawSprite(nullptr, false);
    this->flameAnimator.DrawSprite(nullptr, false);
}

void Vultron::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->visible   = true;
        this->drawGroup = Zone::sVars->objectDrawGroup[0];
        this->drawFX    = FX_FLIP;

        if (data)
            this->type = VOID_TO_INT(data);

        this->active = ACTIVE_BOUNDS;

        switch (this->type) {
            case VULTRON_DIVE:
                this->updateRange.x = (this->dist + 16) << 19;
                this->updateRange.y = 0x800000;
                this->startPos      = this->position;
                this->startDir      = this->direction;

                this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
                this->state.Set(&Vultron::State_Init);
                break;

            case VULTRON_TARGET:
                this->updateRange.x = 0x800000;
                this->updateRange.y = 0x800000;
                this->drawFX        = FX_ROTATE;
                this->rotation      = 0xC0 * (sceneInfo->createSlot & 1) + 0x140;

                this->hitboxBadnik.left   = -12;
                this->hitboxBadnik.top    = -8;
                this->hitboxBadnik.right  = 12;
                this->hitboxBadnik.bottom = 8;

                this->bodyAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
                this->flameAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
                this->state.Set(&Vultron::State_Targeting);
                break;
        }
    }
}

void Vultron::StageLoad()
{
    sVars->sfxVultron.Get("Stage/Vultron.wav");

    sVars->aniFrames.Load("SSZ/Vultron.bin", SCOPE_STAGE);
    sVars->nullFrames.Load("", SCOPE_STAGE);

    sVars->hitboxRange.left   = 0;
    sVars->hitboxRange.top    = -64;
    sVars->hitboxRange.right  = 256;
    sVars->hitboxRange.bottom = 128;

    DebugMode::AddObject(sVars->classID, &Vultron::DebugSpawn, &Vultron::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Vultron::DebugSpawn()
{
    Vultron *vultron = GameObject::Create<Vultron>(nullptr, this->position.x, this->position.y);
    vultron->direction     = this->direction;
    vultron->startDir      = this->direction;
    vultron->dist          = 64;
    vultron->updateRange.x = 0x2800000;
}

void Vultron::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Vultron::CheckPlayerCollisions()
{
    Hitbox *hitbox = this->bodyAnimator.GetHitbox(0);
    int32 left     = (hitbox->left << 16) + (((hitbox->right - hitbox->left) << 15) & 0xFFFF0000);
    int32 top      = (hitbox->top << 16) + (((hitbox->bottom - hitbox->top) << 15) & 0xFFFF0000);
    if (this->direction == FLIP_X)
        left = -left;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckBadnikTouch(this, hitbox)) {
            this->position.x += left;
            this->position.y += top;

            if (!player->CheckBadnikBreak(this, true)) {
                this->position.x -= left;
                this->position.y -= top;
            }
        }
    }
}

void Vultron::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position  = this->startPos;
        this->direction = this->startDir;
        this->flameAnimator.SetAnimation(sVars->nullFrames, 0, true, 0); // -1 (null frames)

        Vultron::Create(nullptr);
    }
}

void Vultron::State_Init()
{
    this->active     = ACTIVE_NORMAL;
    this->velocity.x = 0;

    this->state.Set(&Vultron::State_CheckPlayerInRange);
    Vultron::State_CheckPlayerInRange();
}

void Vultron::State_CheckPlayerInRange()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckCollisionTouch(this, &sVars->hitboxRange)) {
            this->velocity.x = this->direction == FLIP_NONE ? 0x8000 : -0x8000;
            this->velocity.y = -0xD800;
            this->storeY     = this->position.y;
            sVars->sfxVultron.Play(false, 255);
            this->state.Set(&Vultron::State_Hop);
        }
    }

    Vultron::CheckPlayerCollisions();
    Vultron::CheckOffScreen();
}

void Vultron::State_Hop()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x1800;

    this->bodyAnimator.Process();

    if (this->bodyAnimator.frameID == 5) {
        this->flameAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
        this->state.Set(&Vultron::State_Dive);
        this->velocity.x *= 8;
    }

    Vultron::CheckPlayerCollisions();
    Vultron::CheckOffScreen();
}

void Vultron::State_Dive()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y -= 0x1800;

    if (this->velocity.y <= 0) {
        this->distRemain = this->dist;
        this->velocity.y = 0;
        this->state.Set(&Vultron::State_Flying);
    }

    this->flameAnimator.Process();

    Vultron::CheckPlayerCollisions();
    Vultron::CheckOffScreen();
}

void Vultron::State_Flying()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    if (!--this->distRemain) {
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->flameAnimator.SetAnimation(sVars->nullFrames, 0, true, 0); // -1 (null frames)
        this->state.Set(&Vultron::State_Rise);
    }

    this->flameAnimator.Process();
    Vultron::CheckPlayerCollisions();
    Vultron::CheckOffScreen();
}

void Vultron::State_Rise()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->velocity.x += this->direction ? 0x1800 : -0x1800;
    this->velocity.y -= 0x4000;

    if (this->velocity.y < -0x38000) {
        this->velocity.y = -0x38000;
        this->state.Set(&Vultron::State_PrepareDive);
    }

    this->bodyAnimator.Process();

    Vultron::CheckPlayerCollisions();
    Vultron::CheckOffScreen();
}

void Vultron::State_PrepareDive()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    if (this->direction) {
        this->velocity.x += 0x1800;

        if (this->velocity.x > 0)
            this->velocity.x = 0;
    }
    else {
        this->velocity.x -= 0x1800;

        if (this->velocity.x < 0)
            this->velocity.x = 0;
    }

    this->velocity.y += 0x1800;
    if (this->velocity.y > -0xC800) {
        this->direction ^= FLIP_X;
        this->position.y = this->storeY;
        this->velocity.x = this->direction == FLIP_NONE ? 0x8000 : -0x8000;
        this->velocity.y = -0xC800;
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->state.Set(&Vultron::State_Hop);
    }

    this->bodyAnimator.Process();

    Vultron::CheckPlayerCollisions();
    Vultron::CheckOffScreen();
}

void Vultron::State_Targeting()
{
    Player *targetPlayer = Player::GetNearestPlayerX();

    this->flameAnimator.Process();

    int32 angle = Math::ATan2((targetPlayer->position.x - this->position.x) >> 16, (targetPlayer->position.y - this->position.y) >> 16);
    int32 rot   = (angle << 1) - this->rotation;

    if (abs(2 * angle - this->rotation) >= abs(rot - 0x200)) {
        if (abs(rot - 0x200) < abs(rot + 0x200))
            this->rotation += ((rot - 0x200) >> 5);
        else
            this->rotation += ((rot + 0x200) >> 5);
    }
    else {
        if (abs(2 * angle - this->rotation) < abs(rot + 0x200))
            this->rotation += (rot >> 5);
        else
            this->rotation += ((rot + 0x200) >> 5);
    }

    this->rotation &= 0x1FF;
    this->position.x += Math::Cos512(this->rotation) << 9;
    this->position.y += Math::Sin512(this->rotation) << 9;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckBadnikTouch(this, &this->hitboxBadnik))
            player->CheckBadnikBreak(this, true);
    }
}

#if RETRO_INCLUDE_EDITOR
void Vultron::EditorDraw()
{
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;

    switch (this->type) {
        case VULTRON_DIVE:
            this->drawFX = FX_NONE;
            this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, false, 0);
            this->flameAnimator.SetAnimation(sVars->nullFrames, 2, true, 0);
            break;

        case VULTRON_TARGET:
            this->drawFX = FX_ROTATE;
            this->bodyAnimator.SetAnimation(sVars->aniFrames, 1, false, 0);
            this->flameAnimator.SetAnimation(sVars->aniFrames, 2, false, 0);
            break;
    }

    Vultron::Draw();
}

void Vultron::EditorLoad()
{
    sVars->aniFrames.Load("SSZ/Vultron.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Dive");
    RSDK_ENUM_VAR("Target Player");

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Right");
    RSDK_ENUM_VAR("Left");
}
#endif

void Vultron::Serialize()
{
    RSDK_EDITABLE_VAR(Vultron, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(Vultron, VAR_UINT16, dist);
    RSDK_EDITABLE_VAR(Vultron, VAR_UINT8, direction);
}

} // namespace GameLogic