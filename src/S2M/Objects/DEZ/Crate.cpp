// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Crate Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Crate.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/Shield.hpp"
#include "Global/Spikes.hpp"
#include "Global/ItemBox.hpp"
#include "Common/Platform.hpp"
#include "Helpers/MathHelpers.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Crate);

void Crate::Update()
{
    if (this->collision != Platform::C_Solid || !Crate::Collide()) {
        if (this->animator.animationID) {
            this->state.Run(this);
        }
        else {
            //Platform::Update();
            // this is all just platform update stuff
            if (Zone::sVars->teleportActionActive) {
                this->pushPlayersL = false;
                this->pushPlayersR = false;
            }

            this->position.x = this->drawPos.x & 0xFFFF0000;
            this->position.y = this->drawPos.y & 0xFFFF0000;

            if (this->hasTension) {
                if (this->stood && this->stoodAngle < 64)
                    this->stoodAngle += 4;
                else if (!this->stood && this->stoodAngle > 0)
                    this->stoodAngle -= 4;
            }

            this->collisionOffset.x = -this->position.x;
            this->collisionOffset.y = -this->position.y;

            this->state.Run(this);

            if (this->classID) {
                this->stood = false;
                this->collisionOffset.x += this->drawPos.x & 0xFFFF0000;
                this->collisionOffset.y += this->drawPos.y & 0xFFFF0000;
                if (!this->state.Matches(&Platform::State_Falling2) && !this->state.Matches(&Platform::State_Hold)) {
                    this->stateCollide.Run(this);
                }

                this->position.x = this->centerPos.x;
                this->position.y = this->centerPos.y;
                for (int32 s = sceneInfo->entitySlot + 1, i = 0; i < this->childCount; ++i) {
                    Entity *child = GameObject::Get(s++);
                    if (child->classID == ItemBox::sVars->classID) {
                        if (!child->scale.y) {
                            ItemBox *itemBox       = (ItemBox *)child;
                            itemBox->parent        = (Entity *)this;
                            itemBox->scale.x       = itemBox->position.x - this->centerPos.x;
                            itemBox->scale.y       = itemBox->position.y - this->centerPos.y;
                            itemBox->position.x    = itemBox->scale.x + this->drawPos.x;
                            itemBox->position.y    = itemBox->scale.y + this->drawPos.y;
                            itemBox->updateRange.x = this->updateRange.x;
                            itemBox->updateRange.y = this->updateRange.y;
                        }
                    }
                    else {
                        child->position.x += this->collisionOffset.x;
                        child->position.y += this->collisionOffset.y;

                        if (child->classID == Spikes::sVars->classID) {
                            Spikes *spikes            = (Spikes *)child;
                            spikes->collisionOffset.x = this->collisionOffset.x;
                            spikes->collisionOffset.y = this->collisionOffset.y;
                        }
                        else if (child->classID == sVars->classID) {
                            Platform *platform = (Platform *)child;
                            platform->centerPos.x += this->collisionOffset.x;
                            platform->centerPos.y += this->collisionOffset.y;
                        }
                    }
                }

                if (this->animator.frameDuration)
                    this->animator.Process();
            }
        }
    }
}

void Crate::LateUpdate() {}

void Crate::StaticUpdate() {}

void Crate::Draw() { this->animator.DrawSprite(&this->drawPos, false); }

void Crate::Create(void *data)
{
    int32 frameID = this->frameID;
    if (frameID == 3)
        this->collision = Platform::C_None;
    else
        this->collision = Platform::C_Solid;

    // this object uses platform as a base for a bunch of its stuff apparently but i cant just run the platform events directly since things work different here
    //Platform::Create(nullptr);
    this->amplitude.x >>= 10;
    this->amplitude.y >>= 10;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->active    = ACTIVE_BOUNDS;
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0] + 1;
    this->centerPos = this->position;
    this->drawPos   = this->position;
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->animator.frameID = this->frameID;
    this->frameID = frameID;

    if (!sceneInfo->inEditor) {
        this->hitbox.left   = -24;
        this->hitbox.top    = -24;
        this->hitbox.right  = 24;
        this->hitbox.bottom = 24;

        this->active    = ACTIVE_XBOUNDS;
        this->drawFX    = FX_SCALE | FX_FLIP;
        this->scale.x   = 0x200;
        this->scale.y   = 0x200;
        this->drawGroup = frameID == 3 ? Zone::sVars->objectDrawGroup[0] : Zone::sVars->objectDrawGroup[1];
        this->animator.SetAnimation(sVars->aniFrames, 0, true, frameID);
        this->state.Set(&Crate::State_None);
    }
}

void Crate::StageLoad()
{
    sVars->aniFrames.Load("DEZ/Crate.bin", SCOPE_STAGE);

    sVars->sfxExplosion2.Get("Stage/Explosion2.wav");
}

void Crate::Break(Crate *create)
{
    // im not porting ice lmao
    //for (int32 s = 0; s < 64; ++s) {
    //    // ice is used to create a shattering effect
    //    int32 x        = create->position.x + (Math::Rand(-24, 25) << 16);
    //    int32 y        = create->position.y + (Math::Rand(-24, 25) << 16);
    //    EntityIce *ice = CREATE_ENTITY(Ice, INT_TO_VOID(ICE_CHILD_SHARD), x, y);
    //
    //    ice->velocity.x          = Math::Rand(-6, 8) << 15;
    //    ice->velocity.y          = Math::Rand(-10, 2) << 15;
    //    ice->direction           = Math::Rand(0, 4);
    //    ice->blockAnimator.speed = Math::Rand(1, 4);
    //    ice->drawGroup           = Zone::sVars->objectDrawGroup[0] + 1;
    //
    //    switch (create->animator.frameID) {
    //        case 0:
    //        case 3: RSDK.SetAnimation(sVars->aniFrames, 1, &ice->blockAnimator, true, 0); break;
    //
    //        case 1:
    //        case 2:
    //            if (Math::Rand(0, 6) >= 2)
    //                RSDK.SetAnimation(sVars->aniFrames, 2, &ice->blockAnimator, true, 0);
    //            else
    //                RSDK.SetAnimation(sVars->aniFrames, 3, &ice->blockAnimator, true, 0);
    //            break;
    //
    //        default: break;
    //    }
    //}

    sVars->sfxExplosion2.Play(false, 255);
    create->position.y -= 0x10000;

    for (auto crate : GameObject::GetEntities<Crate>(FOR_ACTIVE_ENTITIES))
    {
        if (crate != create && crate->state.Matches(&Crate::State_None)
            && create->CheckCollisionTouchBox(&create->hitbox, crate, &crate->hitbox)) {
            crate->state.Set(&Crate::State_ApplyGravity);
        }
    }

    create->Destroy();
}
void Crate::MoveY(Crate *self, int32 offset)
{
    self->drawPos.y += offset;
    self->centerPos.x = self->drawPos.x;
    self->centerPos.y = self->drawPos.y;
    self->position.x  = self->drawPos.x;
    self->position.y  = self->drawPos.y;

    int32 start = (self->drawPos.y - 0x300000) & 0xFFFF0000;
    for (auto crate : GameObject::GetEntities<Crate>(FOR_ACTIVE_ENTITIES))
    {
        if (crate != self && self->CheckCollisionBox(&self->hitbox, crate, &crate->hitbox, true) == C_TOP)
            Crate::MoveY(crate, start - crate->drawPos.y);
    }
}
bool32 Crate::Collide()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        int32 storeX         = player->position.x;
        int32 storeY         = player->position.y;
        int32 storeXVel      = player->velocity.x;
        int32 storeYVel      = player->velocity.y;
        int32 storeVel       = player->groundVel;
        int32 storeGrounded  = player->onGround;
        Hitbox *playerHitbox = player->GetHitbox();

        int32 shieldAnim = GameObject::Get<Shield>(Player::sVars->playerCount + RSDKTable->GetEntitySlot(player))->shieldAnimator.animationID;

        switch (this->CheckCollisionBox(&this->hitbox, player, playerHitbox)) {
            default:
            case C_NONE:
                player->velocity.x = storeXVel;
                player->velocity.y = storeYVel;
                player->position.x = storeX;
                player->position.y = storeY;
                player->CheckCollisionBox(this, &this->hitbox);
                break;

            case C_TOP:
                if (this->frameID != 1) {
                    player->velocity.x = storeXVel;
                    player->velocity.y = storeYVel;
                    player->position.x = storeX;
                    player->position.y = storeY;
                    player->CheckCollisionBox(this, &this->hitbox);
                    break;
                }
                else if (player->shield == Player::Shield_Bubble && player->invincibleTimer <= 0) {
                    if (shieldAnim == 8 && player->velocity.y >= 0x80000) {
                        Crate::Break(this);
                        player->velocity.x = storeXVel;
                        player->velocity.y = storeYVel;
                        player->position.x = storeX;
                        player->position.y = storeY;
                        player->groundVel  = storeVel;
                        player->onGround   = storeGrounded;
                        return true;
                    }
                }

                player->velocity.x = storeXVel;
                player->velocity.y = storeYVel;
                player->position.x = storeX;
                player->position.y = storeY;
                player->CheckCollisionBox(this, &this->hitbox);
                break;

            case C_LEFT:
            case C_RIGHT:
                if (this->frameID != 1 || player->shield != Player::Shield_Fire || player->invincibleTimer > 0 || shieldAnim != 2) {
                    player->velocity.x = storeXVel;
                    player->velocity.y = storeYVel;
                    player->position.x = storeX;
                    player->position.y = storeY;
                    player->CheckCollisionBox(this, &this->hitbox);
                }
                else if (player->position.x < this->position.x) {
                    player->velocity.x = storeXVel;
                    player->velocity.y = storeYVel;
                    player->position.x = storeX;
                    player->position.y = storeY;

                    if (player->velocity.x >= 0x78000) {
                        Crate::Break(this);
                        player->groundVel = storeVel;
                        player->onGround  = storeGrounded;
                        return true;
                    }
                    else {
                        player->CheckCollisionBox(this, &this->hitbox);
                    }
                }
                else {
                    player->velocity.x = storeXVel;
                    player->velocity.y = storeYVel;
                    player->position.x = storeX;
                    player->position.y = storeY;

                    if (player->velocity.x <= -0x78000) {
                        Crate::Break(this);

                        player->groundVel = storeVel;
                        player->onGround  = storeGrounded;
                        return true;
                    }
                    else {
                        player->CheckCollisionBox(this, &this->hitbox);
                    }
                }
                break;

            case C_BOTTOM:
                if (this->collisionOffset.y >= 0)
                    player->collisionFlagV |= 2;

                player->velocity.x = storeXVel;
                player->velocity.y = storeYVel;
                player->position.x = storeX;
                player->position.y = storeY;
                player->CheckCollisionBox(this, &this->hitbox);
                break;
        }
    }

    return false;
}
void Crate::State_None()
{
    // hehe
}
void Crate::State_ApplyGravity()
{
    this->position.y -= 0x10000;

    for (auto crate : GameObject::GetEntities<Crate>(FOR_ACTIVE_ENTITIES))
    {
        if (crate != this && crate->state.Matches(&Crate::State_None) && this->CheckCollisionTouchBox(&this->hitbox, crate, &crate->hitbox)) {
            crate->state.Set(&Crate::State_ApplyGravity);
        }
    }

    this->position.y += 0x10000;
    this->timer = 15;
    this->state.Set(&Crate::State_WaitToFall);
}
void Crate::State_WaitToFall()
{
    if (--this->timer <= 0)
        this->state.Set(&Crate::State_Fall);
}
void Crate::State_Fall()
{
    this->drawPos.y += this->velocity.y;

    int32 x = this->position.x;
    int32 y = this->position.y;

    this->velocity.y += 0x3800;
    this->position.x = this->drawPos.x;
    this->position.y = this->drawPos.y;

    if (this->TileCollision(Zone::sVars->collisionLayers, 0, CMODE_FLOOR, 0, 0x180000, true)) {
        this->velocity.y = 0;
        this->state.Set(&Crate::State_None);
    }
    else {
        for (auto crate : GameObject::GetEntities<Crate>(FOR_ACTIVE_ENTITIES))
        {
            if (crate != this && !crate->velocity.y && crate->CheckCollisionBox(&crate->hitbox, this, &this->hitbox, true) == C_TOP) {
                this->velocity.y = 0;
                this->state.Set(&Crate::State_None);
            }
        }
    }

    this->drawPos.x   = this->position.x;
    this->drawPos.y   = this->position.y;
    this->centerPos.x = this->position.x;
    this->centerPos.y = this->position.y;

    this->position.x = x;
    this->position.y = y;
}

#if RETRO_INCLUDE_EDITOR
void Crate::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, this->frameID);
    this->drawPos = this->position;

    Crate::Draw();

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        for (int32 s = sceneInfo->entitySlot + 1, i = 0; i < this->childCount; ++i) {
            Entity *child = GameObject::Get(s + i);
            if (!child)
                continue;

            DrawHelpers::DrawArrow(this->position.x, this->position.y, child->position.x, child->position.y, 0xE0E0E0, INK_NONE, 0xFF);
        }

        RSDK_DRAWING_OVERLAY(false);
    }
}

void Crate::EditorLoad()
{
    sVars->aniFrames.Load("DEZ/Crate.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, frameID);
    RSDK_ENUM_VAR("Blue");
    RSDK_ENUM_VAR("Orange (Broken)");
    RSDK_ENUM_VAR("Orange");
    RSDK_ENUM_VAR("Blue (BG)");
}
#endif

void Crate::Serialize()
{
    RSDK_EDITABLE_VAR(Crate, VAR_UINT8, frameID);
    RSDK_EDITABLE_VAR(Crate, VAR_ENUM, childCount);
    RSDK_EDITABLE_VAR(Crate, VAR_BOOL, ignoreItemBox);
}

} // namespace GameLogic