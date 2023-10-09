// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Bomb Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Bomb.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Explosion.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Bomb);

void Bomb::Update() { this->state.Run(this); }

void Bomb::LateUpdate() {}

void Bomb::StaticUpdate() {}

void Bomb::Draw()
{
    Vector2 drawPos;

    if (this->state.Matches(&Bomb::State_Explode)) {
        drawPos.x = this->position.x;
        drawPos.y = this->fuseOffset + this->position.y;
        this->fuseAnimator.DrawSprite(&drawPos, false);
    }
    this->mainAnimator.DrawSprite(nullptr, false);
}

void Bomb::Create(void *data)
{
    this->visible = true;
    if (this->planeFilter > 0 && (((uint8)this->planeFilter - 1) & 2))
        this->drawGroup = Zone::sVars->objectDrawGroup[1];
    else
        this->drawGroup = Zone::sVars->objectDrawGroup[0];
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    if (data) {
        this->mainAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
        this->state.Set(&Bomb::State_Shrapnel);
    }
    else {
        this->startDir = this->direction;
        this->startPos = this->position;
        if (this->direction == FLIP_NONE)
            this->velocity.x = -0x1000;
        else
            this->velocity.x = 0x1000;
        this->drawFX |= FX_FLIP;
        this->timer = 1536;
        this->mainAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->fuseAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
        this->state.Set(&Bomb::State_Init);
    }
}

void Bomb::StageLoad()
{
    sVars->aniFrames.Load("SSZ/Bomb.bin", SCOPE_STAGE);

    sVars->hitboxHurt.left   = -12;
    sVars->hitboxHurt.top    = -18;
    sVars->hitboxHurt.right  = 12;
    sVars->hitboxHurt.bottom = 18;

    sVars->hitboxRange.left   = -96;
    sVars->hitboxRange.top    = -96;
    sVars->hitboxRange.right  = 96;
    sVars->hitboxRange.bottom = 96;

    sVars->hitboxShrapnel.left   = -6;
    sVars->hitboxShrapnel.top    = -6;
    sVars->hitboxShrapnel.right  = 6;
    sVars->hitboxShrapnel.bottom = 6;

    sVars->sfxExplosion.Get("Stage/Explosion.wav");

    DebugMode::AddObject(sVars->classID, &Bomb::DebugSpawn, &Bomb::DebugDraw);
}

void Bomb::DebugSpawn()
{
    GameObject::Create<Bomb>(nullptr, this->position.x, this->position.y);
}

void Bomb::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(0, false);
}

void Bomb::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position  = this->startPos;
        this->direction = this->startDir;
        Bomb::Create(nullptr);
    }
}

void Bomb::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (this->planeFilter <= 0 || player->collisionPlane == ((this->planeFilter - 1) & 1)) {
            if (!this->state.Matches(&Bomb::State_Explode)) {
                if (player->CheckCollisionTouch(this, &sVars->hitboxRange)) {
                    this->mainAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
                    this->timer = 144;
                    this->state.Set(&Bomb::State_Explode);
                }
            }
            if (player->CheckCollisionTouch(this, &sVars->hitboxHurt)) {
                player->Hurt(this);
            }
        }
    }
}

void Bomb::State_Init()
{
    this->active = ACTIVE_NORMAL;
    this->state.Set(&Bomb::State_Walk);
    Bomb::State_Walk();
}

void Bomb::State_Walk()
{
    this->position.x += this->velocity.x;

    if (!--this->timer) {
        this->timer = 180;
        this->state.Set(&Bomb::State_Idle);
    }
    else {
        bool32 groundCollided = false;
        if ((this->direction & FLIP_Y)) // RWall??? it should be roof right???
            groundCollided = this->TileGrip(Zone::sVars->collisionLayers, CMODE_RWALL, 0, 0, -0x100000, 2);
        else       
            groundCollided = this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x100000, 2);

        if (!groundCollided) {
            this->timer = 180;
            this->mainAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->state.Set(&Bomb::State_Idle);
        }
    }

    this->mainAnimator.Process();

    Bomb::CheckPlayerCollisions();
    Bomb::CheckOffScreen();
}

void Bomb::State_Idle()
{
    if (!--this->timer) {
        this->direction ^= FLIP_X;
        this->velocity.x = -this->velocity.x;
        this->timer      = 0x600;
        this->mainAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->state.Set(&Bomb::State_Walk);
    }

    this->mainAnimator.Process();

    Bomb::CheckPlayerCollisions();
    Bomb::CheckOffScreen();
}

void Bomb::State_Explode()
{
    if ((this->direction & FLIP_Y))
        this->fuseOffset -= 0x1000;
    else
        this->fuseOffset += 0x1000;

    if (--this->timer > 0) {
        this->mainAnimator.Process();
        this->fuseAnimator.Process();
        Bomb::CheckPlayerCollisions();
        Bomb::CheckOffScreen();
    }
    else {
        sVars->sfxExplosion.Play(false, 255);
        Bomb *debris        = GameObject::Create<Bomb>(INT_TO_VOID(true), this->position.x, this->position.y);
        debris->velocity.x  = -0x20000;
        debris->velocity.y  = -0x30000;
        debris->planeFilter = this->planeFilter;
        debris->drawGroup   = this->drawGroup;

        debris              = GameObject::Create<Bomb>(INT_TO_VOID(true), this->position.x, this->position.y);
        debris->velocity.x  = -0x10000;
        debris->velocity.y  = -0x20000;
        debris->planeFilter = this->planeFilter;
        debris->drawGroup   = this->drawGroup;

        debris              = GameObject::Create<Bomb>(INT_TO_VOID(true), this->position.x, this->position.y);
        debris->velocity.x  = 0x20000;
        debris->velocity.y  = -0x30000;
        debris->planeFilter = this->planeFilter;
        debris->drawGroup   = this->drawGroup;

        debris              = GameObject::Create<Bomb>(INT_TO_VOID(true), this->position.x, this->position.y);
        debris->velocity.x  = 0x10000;
        debris->velocity.y  = -0x20000;
        debris->planeFilter = this->planeFilter;
        debris->drawGroup   = this->drawGroup;

        Explosion *explosion       = GameObject::Create<Explosion>(INT_TO_VOID(Explosion::Type1), this->position.x, this->position.y);
        explosion->planeFilter     = this->planeFilter;
        explosion->drawGroup       = this->drawGroup + 1;

        this->Destroy();
    }
}

void Bomb::State_Shrapnel()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x1800;
    if (this->CheckOnScreen(&this->updateRange)) {
        this->mainAnimator.Process();

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (this->planeFilter <= 0 || player->collisionPlane == ((this->planeFilter - 1) & 1)) {
                if (player->CheckCollisionTouch(this, &sVars->hitboxShrapnel)) {
                    player->ProjectileHurt(this);
                }
            }
        }
    }
    else {
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void Bomb::EditorDraw() { Bomb::Draw(); }

void Bomb::EditorLoad()
{
    sVars->aniFrames.Load("SSZ/Bomb.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, planeFilter);
    RSDK_ENUM_VAR("None");
    RSDK_ENUM_VAR("AL");
    RSDK_ENUM_VAR("BL");
    RSDK_ENUM_VAR("AH");
    RSDK_ENUM_VAR("BH");

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Right");
    RSDK_ENUM_VAR("Left (Flip Y)");
    RSDK_ENUM_VAR("Right (Flip Y)");
}
#endif

void Bomb::Serialize()
{
    RSDK_EDITABLE_VAR(Bomb, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(Bomb, VAR_ENUM, planeFilter);
}

} // namespace GameLogic