// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Blastoid Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Blastoid.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Common/CollapsingPlatform.hpp"
#include "Common/Water.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Blastoid);

void Blastoid::Update() { this->state.Run(this); }

void Blastoid::LateUpdate() {}

void Blastoid::StaticUpdate() {}

void Blastoid::Draw() { this->animator.DrawSprite(nullptr, false); }

void Blastoid::Create(void *data)
{
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];
    this->drawFX |= FX_FLIP;

    if (!sceneInfo->inEditor) {
        if (data) {
            this->active        = ACTIVE_NORMAL;
            this->updateRange.x = 0x200000;
            this->updateRange.y = 0x200000;
            this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);
            this->state.Set(&Blastoid::State_Projectile);
        }
        else {
            this->active        = ACTIVE_BOUNDS;
            this->updateRange.x = 0x800000;
            this->updateRange.y = 0x800000;
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 1);
            this->state.Set(&Blastoid::State_Init);
        }
    }
}

void Blastoid::StageLoad()
{
    sVars->aniFrames.Load("DEZ/Blastoid.bin", SCOPE_STAGE);

    sVars->hitboxBody.left   = -12;
    sVars->hitboxBody.top    = -9;
    sVars->hitboxBody.right  = 12;
    sVars->hitboxBody.bottom = 12;

    sVars->hitboxProjectile.left   = -3;
    sVars->hitboxProjectile.top    = -3;
    sVars->hitboxProjectile.right  = 3;
    sVars->hitboxProjectile.bottom = 3;

    sVars->sfxShot.Get("Stage/Shot.wav");

    DebugMode::AddObject(sVars->classID, &Blastoid::DebugSpawn, &Blastoid::DebugDraw);
}

void Blastoid::DebugSpawn()
{
    GameObject::Create<Blastoid>(nullptr, this->position.x, this->position.y);
}

void Blastoid::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Blastoid::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBody) && player->CheckBadnikBreak(this, false)) {
            CollapsingPlatform *platform = GameObject::Get<CollapsingPlatform>(sceneInfo->entitySlot - 1);
            if (platform->classID == CollapsingPlatform::sVars->classID) {
                platform->active        = ACTIVE_NORMAL;
                platform->collapseDelay = 30;
                //platform->stoodPos.x  = this->position.x;
                platform->stoodPos      = this->position.x;
            }
            this->Destroy();
        }
    }
}

void Blastoid::State_Init()
{
    this->active = ACTIVE_NORMAL;

    CollapsingPlatform *platform = GameObject::Get<CollapsingPlatform>(sceneInfo->entitySlot - 1);
    if (platform->classID == CollapsingPlatform::sVars->classID)
        platform->active = ACTIVE_NEVER;

    this->state.Set(&Blastoid::State_Body);
}

void Blastoid::State_Body()
{
    this->animator.Process();

    switch (++this->timer) {
        case 1:
        case 16:
        case 31: {
            Blastoid *projectile = GameObject::Create<Blastoid>(INT_TO_VOID(true), this->position.x, this->position.y);

            if (this->direction & FLIP_X) {
                projectile->position.x += 0x100000;
                projectile->velocity.x = 0x20000;
            }
            else {
                projectile->position.x -= 0x100000;
                projectile->velocity.x = -0x20000;
            }

            if (this->direction & FLIP_Y) {
                projectile->position.y += 0x60000;
                projectile->velocity.y = 0x10000;
            }
            else {
                projectile->position.y -= 0x60000;
                projectile->velocity.y = -0x10000;
            }

            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            sVars->sfxShot.Play(false, 0xFF);
            break;
        }

        case 121: this->timer = 0; break;

        default: break;
    }

    Blastoid::CheckPlayerCollisions();

    if (!this->CheckOnScreen(nullptr)) {
        this->timer  = 0;
        this->active = ACTIVE_BOUNDS;
    }
}

void Blastoid::State_Projectile()
{
    if (!this->CheckOnScreen(nullptr)) {
        this->Destroy();
    }
    else {
        this->position.x += this->velocity.x;
        this->position.y += this->velocity.y;

        this->animator.Process();

        // Bug(?): setPos value is 4??????
        // This implies these used to be "ObjectTileGrip" at some point, and were then changed to "ObjectTileCollision" without changing the last
        // parameter Not that it *really* matters, since 4 will evaluate to true since it's non-zero

        if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, sVars->hitboxProjectile.top << 13, 4)
            || this->TileCollision(Zone::sVars->collisionLayers, CMODE_LWALL, 0, sVars->hitboxProjectile.left << 13, 0, 4)
            || this->TileCollision(Zone::sVars->collisionLayers, CMODE_ROOF, 0, 0, sVars->hitboxProjectile.bottom << 13, 4)
            || this->TileCollision(Zone::sVars->collisionLayers, CMODE_RWALL, 0, sVars->hitboxProjectile.right << 13, 0, 4)) {
            this->Destroy();
        }
        else {
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
            {
                int32 shield = player->shield;
                // HCZBubble overrides active shields for collision
                //if (Water_GetPlayerBubble(player))
                //    player->shield = SHIELD_BUBBLE;

                if (player->CheckCollisionTouch(this, &sVars->hitboxProjectile))
                    player->ProjectileHurt(this);
                player->shield = shield;
            }
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void Blastoid::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);

    Blastoid::Draw();

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        CollapsingPlatform *platform = GameObject::Get<CollapsingPlatform>(sceneInfo->entitySlot - 1);
        if (CollapsingPlatform::sVars && platform->classID == CollapsingPlatform::sVars->classID)
            DrawHelpers::DrawArrow(this->position.x, this->position.y, platform->position.x, platform->position.y, 0xFFFF00, INK_NONE, 0xFF);

        RSDK_DRAWING_OVERLAY(false);
    }
}

void Blastoid::EditorLoad()
{
    sVars->aniFrames.Load("DEZ/Blastoid.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left", FLIP_NONE);
    RSDK_ENUM_VAR("Right", FLIP_X);
    RSDK_ENUM_VAR("Left (Flipped)", FLIP_Y);
    RSDK_ENUM_VAR("Right (Flipped)", FLIP_XY);
}
#endif

void Blastoid::Serialize() { RSDK_EDITABLE_VAR(Blastoid, VAR_UINT8, direction); }

} // namespace GameLogic