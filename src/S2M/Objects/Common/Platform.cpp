// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Platform Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Platform.hpp"
#include "Global/Zone.hpp"
#include "Common/ScreenWrap.hpp"
#include "PlatformNode.hpp"
#include "PlatformControl.hpp"
#include "Global/Shield.hpp"
#include "Global/ItemBox.hpp"
#include "Global/Spikes.hpp"
#include "Global/Spring.hpp"
#include "Global/Debris.hpp"
#include "Button.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Platform);

void Platform::Update()
{
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
void Platform::LateUpdate() {}
void Platform::StaticUpdate()
{
    for (int32 p = 0; p < PLAYER_COUNT; ++p) {
        sVars->stoodPos[p].x = 0;
        sVars->stoodPos[p].y = 0;
    }
}
void Platform::Draw()
{
    Vector2 drawPos;

    if (this->frameID >= 0) {

        if ((this->state.Matches(&Platform::State_Circular) && this->hasTension)
            || (this->state.Matches(&Platform::State_Swing) || this->state.Matches(&Platform::State_Clacker) || this->type == Platform::SwingReact)) {
            int32 ang = this->angle;
            if (this->state.Matches(&Platform::State_Circular) && this->hasTension)
                ang = this->speed * Zone::sVars->timer + 4 * this->angle;

            int32 fxStore = this->drawFX;
            this->drawFX |= FX_FLIP | FX_ROTATE;
            int32 cnt              = (this->amplitude.y >> 10) - 1;
            this->direction        = FLIP_NONE;
            this->animator.frameID = this->frameID + 1;

            int32 rot   = ang >> 1;
            int32 angle = 0x40 * 0x10;
            for (int32 i = 0; i < cnt; ++i) {
                drawPos.x = angle * Math::Cos1024(ang) + this->centerPos.x;
                drawPos.y = angle * Math::Sin1024(ang) + this->centerPos.y;
                this->animator.DrawSprite(&drawPos, false);
                angle += 0x400;
                this->direction ^= FLIP_X;
                this->rotation = rot;
            }

            this->drawFX           = fxStore;
            this->animator.frameID = this->frameID + 2;
            this->animator.DrawSprite(&this->centerPos, false);

            this->animator.frameID = this->frameID;
        }

        if (sVars->aniFrames.Matches(nullptr))
            Graphics::DrawRect(this->drawPos.x - 0x200000, this->drawPos.y - 0x100000, 64 << 16, 32 << 16, 0x8080A0, 0xFF, INK_NONE, false);
        else
            this->animator.DrawSprite(&this->drawPos, false);

        ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Platform::Draw), true);
    }
}

void Platform::Create(void *data)
{
    this->amplitude.x >>= 10;
    this->amplitude.y >>= 10;
    this->active    = ACTIVE_BOUNDS;
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0] + 1;
    this->centerPos = this->position;
    this->drawPos   = this->position;
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->animator.frameID = this->frameID;

    if (!sceneInfo->inEditor) {
        switch (this->animator.GetFrameID()) {
            case '1': this->drawGroup = Zone::sVars->objectDrawGroup[0]; break;
            case '2': this->drawGroup = Zone::sVars->objectDrawGroup[1]; break;
            case '3': this->drawGroup = Zone::sVars->objectDrawGroup[0] - 1; break;
            case '4': this->drawGroup = Zone::sVars->playerDrawGroup[0] + 1; break;
            default: break;
        }
    }

    switch (this->type) {
        default:
        case Platform::Fixed:
            this->state.Set(&Platform::State_Fixed);
            this->updateRange.x = 0x800000;
            this->updateRange.y = 0x800000;
            break;

        case Platform::Fall:
            this->state.Set(&Platform::State_Fall);
            this->updateRange.x = 0x800000;
            this->updateRange.y = (abs(this->amplitude.y) + 0x2000) << 10;
            break;

        case Platform::Linear:
            this->state.Set(&Platform::State_Linear);
            this->updateRange.x = (abs(this->amplitude.x) + 0x2000) << 10;
            this->updateRange.y = (abs(this->amplitude.y) + 0x2000) << 10;
            this->rotation      = this->angle;
            this->angle         = 0;
            break;

        case Platform::Circular:
            this->updateRange.x = (abs(this->amplitude.x) + 0x2000) << 10;
            this->updateRange.y = (abs(this->amplitude.y) + 0x2000) << 10;
            this->state.Set(&Platform::State_Circular);
            break;

        case Platform::Path:
        case Platform::PathReact:
            this->active = ACTIVE_BOUNDS;
            if (this->type == Platform::Path)
                this->state.Set(&Platform::State_PathStop);
            else
                this->state.Set(&Platform::State_PathReact);
            this->updateRange.x = 0x800000;
            this->updateRange.y = 0x800000;
            break;

        case Platform::Push:
            this->updateRange.x = 0x800000;
            this->updateRange.y = 0x800000;
            if (!sceneInfo->inEditor) {
                this->speed <<= 11;
                this->position.x += 0x8000;
            }
            this->state.Set(&Platform::State_Push_Init);
            break;

        case Platform::React:
            this->amplitude.x <<= 10;
            this->updateRange.x = 0x800000 + abs(this->amplitude.x);
            this->updateRange.y = 0x800000 + abs(this->amplitude.x);
            if (this->speed < 0)
                this->direction = FLIP_X;
            this->state.Set(&Platform::State_React);
            break;

        case Platform::HoverReact:
            this->amplitude.x <<= 10;
            this->updateRange.x = 0x800000 + abs(this->amplitude.x);
            this->updateRange.y = 0x800000 + abs(this->amplitude.x);
            if (this->speed < 0)
                this->direction = FLIP_X;
            this->state.Set(&Platform::State_Hover_React);
            break;

        case Platform::DoorSlide:
            this->amplitude.x <<= 10;
            this->updateRange.x = 0x800000 + abs(this->amplitude.x);
            this->updateRange.y = 0x800000 + abs(this->amplitude.x);

            if (this->speed < 0) {
                this->direction = FLIP_X;
                this->speed     = -this->speed;
            }
            this->state.Set(&Platform::State_DoorSlide);
            break;

        case Platform::SwingReact:
            this->timer = 0;
            this->drawGroup += (this->tileOrigin.y >> 16) & 0xFF;
            // [Fallthrough]
        case Platform::Swing:
            this->updateRange.x = (abs(this->amplitude.y) + 512) << 14;
            this->updateRange.y = (abs(this->amplitude.y) + 512) << 14;
            this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);

            this->amplitude.y <<= 4;
            this->groundVel = 4 * this->angle;
            this->angle     = this->groundVel + 0x100 + (this->amplitude.x * Math::Sin1024(this->speed * this->timer) >> 14);
            this->drawPos.x = this->amplitude.y * Math::Cos1024(this->angle) + this->centerPos.x;
            this->drawPos.y = this->amplitude.y * Math::Sin1024(this->angle) + this->centerPos.y;
            if (this->type == Platform::Swing) {
                this->state.Set(&Platform::State_Swing);
            }
            else {
                if (this->groundVel >= 0)
                    this->drawPos.x -= TO_FIXED(32);
                else
                    this->drawPos.x += TO_FIXED(32);
                this->state.Set(&Platform::State_SwingReact);
            }
            break;

        case Platform::TrackReact:
            if (this->direction) {
                this->drawPos.x = this->centerPos.x + (this->amplitude.x << 9);
                this->drawPos.y = this->centerPos.y + (this->amplitude.y << 9);
            }
            else {
                this->drawPos.x = this->centerPos.x - (this->amplitude.x << 9);
                this->drawPos.y = this->centerPos.y - (this->amplitude.y << 9);
            }
            // [Fallthrough]
        case Platform::Track:
            this->updateRange.x = (abs(this->amplitude.x) + 0x4000) << 9;
            this->updateRange.y = (abs(this->amplitude.y) + 0x4000) << 9;
            if (this->speed < 0)
                this->direction = FLIP_X;

            if (this->type == Platform::Track) {
                this->state.Set(&Platform::State_Track);
            }
            else {
                this->state.Set(&Platform::State_TrackReact);
            }
            break;

        case Platform::Clacker:
            this->updateRange.x = (abs(this->amplitude.y) + 0x200) << 14;
            this->updateRange.y = (abs(this->amplitude.y) + 0x200) << 14;
            this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);
            this->amplitude.y <<= 4;
            this->groundVel = 4 * this->angle;
            this->angle     = this->groundVel + 0x100 + (this->amplitude.x * Math::Sin1024(this->speed * this->timer) >> 14);
            this->drawPos.x = this->amplitude.y * Math::Cos1024(this->angle) + this->centerPos.x;
            this->drawPos.y = this->amplitude.y * Math::Sin1024(this->angle) + this->centerPos.y;
            this->state.Set(&Platform::State_Clacker);
            break;

        case Platform::Child:
            this->state.Set(&Platform::State_Child);
            this->updateRange.x = 0x800000;
            this->updateRange.y = 0x800000;
            break;

        case Platform::Null: break;
    }

    if (this->frameID >= 0) {
        int32 f    = this->frameID;
        int32 anim = 0;
        while (f >= this->animator.frameCount) {
            if (!this->animator.frameCount)
                break;
            f -= this->animator.frameCount;
            this->animator.SetAnimation(sVars->aniFrames, ++anim, true, 0);
        }
        this->frameID          = f;
        this->animator.frameID = f;
    }
    else {
        this->animator.SetAnimation(nullptr, 0, true, 0);
    }

    if (!sceneInfo->inEditor) {
        if (this->collision != Platform::C_None) {
            Hitbox *hitbox = this->animator.GetHitbox(this->collision != Platform::C_Platform);
            if (!sVars->aniFrames.Matches(nullptr) && hitbox) {
                this->hitbox.left   = hitbox->left;
                this->hitbox.top    = hitbox->top;
                this->hitbox.right  = hitbox->right;
                this->hitbox.bottom = hitbox->bottom;
            }
            else {
                this->hitbox.left   = -32;
                this->hitbox.top    = -16;
                this->hitbox.right  = -8;
                this->hitbox.bottom = 32;
            }
        }

        switch (this->collision) {
            case Platform::C_Platform:
                this->stateCollide.Set(&Platform::Collision_Platform);
                this->hitbox.bottom = this->hitbox.top + 8;
                break;

            default:
            case Platform::C_Solid: this->stateCollide.Set(&Platform::Collision_Solid); break;

            case Platform::C_Tiled: this->stateCollide.Set(&Platform::Collision_Tiles); break;
            case Platform::C_Hurt: this->stateCollide.Set(&Platform::Collision_Hurt); break;
            case Platform::C_None: this->stateCollide.Set(&Platform::Collision_None); break;
            case Platform::C_SolidHurtSides: this->stateCollide.Set(&Platform::Collision_Solid_Hurt_Sides); break;
            case Platform::C_SolidHurtBottom: this->stateCollide.Set(&Platform::Collision_Solid_Hurt_Bottom); break;
            case Platform::C_SolidHurtTop: this->stateCollide.Set(&Platform::Collision_Solid_Hurt_Top); break;
            case Platform::C_SolidHold: this->stateCollide.Set(&Platform::Collision_Solid_Hold); break;

            case Platform::C_SolidSticky:
            case Platform::C_StickyTop:
            case Platform::C_StickyLeft:
            case Platform::C_StickyRight:
            case Platform::C_StickyBottom: this->stateCollide.Set(&Platform::Collision_Sticky); break;

            case Platform::C_SolidBarrel: this->stateCollide.Set(&Platform::Collision_Solid_Barrel); break;
            case Platform::C_SolidNoCrush: this->stateCollide.Set(&Platform::Collision_Solid_NoCrush); break;
            case Platform::C_SolidHurtAll: this->stateCollide.Set(&Platform::Collision_Solid_Hurt_All); break;
            case Platform::C_SolidHurtNoCrush: this->stateCollide.Set(&Platform::Collision_Solid_Hurt_NoCrush); break;
        }

        for (int32 i = 0; i < this->childCount; ++i) {
            Platform *child       = GameObject::Get<Platform>(this->Slot() + 1 + i);
            child->tileCollisions = TILECOLLISION_NONE;
            if (this->updateRange.y < 0x800000 + abs(this->position.y - child->position.y))
                this->updateRange.y = 0x800000 + abs(this->position.y - child->position.y);
        }

        if (ScreenWrap::CheckCompetitionWrap())
            this->active = ACTIVE_NORMAL;
    }
}

void Platform::StageLoad()
{
    RSDK_DYNAMIC_PATH_ACTID("Platform");
    sVars->aniFrames.Load(dynamicPath, SCOPE_STAGE);

    sVars->sfxClacker.Get("Stage/Clacker.wav");
    sVars->sfxClang.Get("Stage/Clang.wav");
    sVars->sfxPush.Get("Global/Push.wav");
    sVars->sfxClack.Get("Stage/Clack3.wav");

    if (sVars->playingPushSfx) {
        sVars->sfxPush.Stop();
        sVars->playingPushSfx = false;
    }

    sVars->active = ACTIVE_ALWAYS;
}

void Platform::State_PathStop()
{
    SET_CURRENT_STATE();

    this->velocity.x = 0;
    this->velocity.y = 0;
}

void Platform::State_Fixed()
{
    SET_CURRENT_STATE();

    this->drawPos.x = this->centerPos.x;
    this->drawPos.y = this->centerPos.y;
    this->drawPos.y += Math::Sin256(this->stoodAngle) << 10;

    this->velocity.x = 0;
    this->velocity.y = 0;
}

void Platform::State_Linear()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;

    this->drawPos.x = this->amplitude.x * Math::Sin1024(this->speed * (this->rotation + Zone::sVars->timer)) + this->centerPos.x;
    this->drawPos.y = this->amplitude.y * Math::Sin1024(this->speed * (this->rotation + Zone::sVars->timer)) + this->centerPos.y;
    this->drawPos.y += Math::Sin256(this->stoodAngle) << 10;

    this->velocity.x = this->drawPos.x + drawX;
    this->velocity.y = this->drawPos.y + drawY;
}

void Platform::State_Swing()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;

    this->angle     = this->groundVel + 0x100 + ((this->amplitude.x * Math::Sin1024(this->speed * Zone::sVars->timer) + 0x200) >> 14);
    this->drawPos.x = this->amplitude.y * Math::Cos1024(this->angle) + this->centerPos.x;
    this->drawPos.y = this->amplitude.y * Math::Sin1024(this->angle) + this->centerPos.y;

    this->velocity.x = this->drawPos.x + drawX;
    this->velocity.y = this->drawPos.y + drawY;
}

void Platform::State_React()
{
    SET_CURRENT_STATE();

    if (this->stood) {
        if (this->hasTension)
            this->timer = 120;
        this->rotation     = 0;
        this->tileOrigin.x = 0;
        this->tileOrigin.y = 0;
        this->active       = ACTIVE_NORMAL;
        this->state.Set(&Platform::State_ReactMove);
    }

    this->velocity.x = 0;
    this->velocity.y = 0;
}

void Platform::State_Hover_React()
{
    SET_CURRENT_STATE();

    this->rotation += 4;
    this->drawPos.y = this->centerPos.y + (Math::Sin1024(this->rotation) << 9);

    if (this->timer) {
        this->timer--;
        if (!this->timer) {
            this->tileOrigin.x = this->drawPos.x - this->centerPos.x;
            this->tileOrigin.y = Math::Sin1024(this->rotation) << 9;
            this->centerPos.x  = this->drawPos.x;
            this->centerPos.y  = this->drawPos.y;
            this->active       = ACTIVE_NORMAL;
            this->state.Set(&Platform::State_ReactMove);
        }
    }
    else if (this->stood) {
        this->timer = 60;
    }

    this->velocity.x = 0;
    this->velocity.y = 0;
}

void Platform::State_Push_Init()
{
    SET_CURRENT_STATE();

    this->active        = ACTIVE_NORMAL;
    this->updateRange.x = 0x2000000;
    this->updateRange.y = 0x2000000;
    this->state.Set(&Platform::State_Push);
    this->velocity.x = 0;
    this->velocity.y = 0;
}

void Platform::State_Falling()
{
    SET_CURRENT_STATE();

    if (--this->timer <= 0) {
        this->timer = 0;
        this->state.Set(&Platform::State_Falling2);
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if ((1 << player->Slot()) & this->stoodPlayers)
                player->velocity.y = this->velocity.y - 0x10000;
        }
    }

    this->drawPos.y += this->velocity.y;
    this->velocity.y += 0x3800;
    this->velocity.x = 0;
}

void Platform::State_DoorSlide()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;

    bool32 activated = false;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (!player->sidekick) {
            Vector2 pivotPos = player->position;
            Zone::RotateOnPivot(&pivotPos, &this->centerPos, ((this->angle & 0xFF) + 0x80) & 0xFF);

            if (abs(player->position.x - this->centerPos.x) <= TO_FIXED(1024)) {
                if (abs(player->position.y - this->centerPos.y) <= TO_FIXED(1024)) {
                    if (pivotPos.y < this->centerPos.y && this->centerPos.y - pivotPos.y < TO_FIXED(256))
                        activated = true;
                }
            }
        }
    }

    if (!activated) {
        if (this->amplitude.y > 0) {
            this->amplitude.y -= (this->speed << 16);
            if (this->amplitude.y <= 0) {
                this->amplitude.y = 0;

                if (this->onScreen && sVars->useClack)
                    sVars->sfxClack.Play();
            }
        }
    }
    else {
        if (this->amplitude.y < this->amplitude.x) {
            this->amplitude.y += (this->speed << 16);
            if (this->amplitude.y >= this->amplitude.x) {
                this->amplitude.y = this->amplitude.x;

                if (this->onScreen && sVars->useClack)
                    sVars->sfxClack.Play();
            }
        }
    }

    if (this->direction) {
        this->drawPos.x = (-this->amplitude.y >> 8) * Math::Cos256(this->angle) + this->centerPos.x;
        this->drawPos.y = (-this->amplitude.y >> 8) * Math::Sin256(this->angle) + this->centerPos.y;
    }
    else {
        this->drawPos.x = (this->amplitude.y >> 8) * Math::Cos256(this->angle) + this->centerPos.x;
        this->drawPos.y = (this->amplitude.y >> 8) * Math::Sin256(this->angle) + this->centerPos.y;
    }

    this->velocity.x = this->drawPos.x + drawX;
    this->velocity.y = this->drawPos.y + drawY;
}

void Platform::State_ReactMove()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;
    this->amplitude.y += this->groundVel;

    this->drawPos.x = (this->amplitude.y >> 8) * Math::Cos256(this->angle) + this->centerPos.x;
    this->drawPos.y = (this->amplitude.y >> 8) * Math::Sin256(this->angle) + this->centerPos.y;

    int32 speed16 = this->speed << 16;
    if (this->groundVel == speed16) {
        if (this->amplitude.y >= this->amplitude.x) {
            this->amplitude.y = this->amplitude.x;
            this->groundVel   = this->groundVel - (this->speed << 11);
            this->state.Set(&Platform::State_ReactSlow);
        }
    }
    else {
        this->groundVel += (this->speed << 10);
        if (this->groundVel >= speed16) {
            this->groundVel = speed16;
            this->centerPos.x += ((this->amplitude.y + this->groundVel) >> 8) * Math::Cos256(this->angle);
            this->centerPos.y += ((this->groundVel + this->amplitude.y) >> 8) * Math::Sin256(this->angle);
            this->amplitude.y = -this->groundVel;
        }
    }

    this->velocity.x = this->drawPos.x + drawX;
    this->velocity.y = this->drawPos.y + drawY;
}

void Platform::State_Push()
{
    SET_CURRENT_STATE();

    this->velocity.x = 0;
    this->velocity.y = 0;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        uint8 id = 1 << player->Slot();
        if (id & this->pushPlayersL)
            this->velocity.x += this->speed;
        if (id & this->pushPlayersR)
            this->velocity.x -= this->speed;
    }

    if (this->velocity.x > 0 || this->velocity.x < 0) {
        if (!sVars->playingPushSfx) {
            sVars->sfxPush.Play();
            sVars->playingPushSfx = true;
        }
    }

    if (this->velocity.x) {
        if (this->timer > 0) {
            this->timer--;
            this->velocity.x = 0;
        }
    }
    else {
        if (sVars->playingPushSfx) {
            sVars->sfxPush.Play();
            sVars->playingPushSfx = false;
        }

        if (this->timer < 4)
            this->timer++;
    }

    this->drawPos.x += this->velocity.x;
    this->position.x = this->drawPos.x;
    this->position.y = this->drawPos.y;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        Hitbox *playerHitbox = player->GetHitbox();
        int32 playerID       = 1 << player->Slot();

        if (playerID & this->pushPlayersL)
            player->position.x = this->drawPos.x + ((this->hitbox.left - playerHitbox->right) << 16);
        if (playerID & this->pushPlayersR)
            player->position.x = this->drawPos.x + ((this->hitbox.right - playerHitbox->left - 1) << 16);
    }

    bool32 collided = this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, (this->hitbox.left + 16) << 16, this->hitbox.bottom << 16, 4);
    int32 y         = this->position.y;

    collided |= this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, this->hitbox.bottom << 16, 4);
    if (this->position.y < y)
        y = this->position.y;

    collided |= this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, (this->hitbox.right - 16) << 16, this->hitbox.bottom << 16, 4);
    if (this->position.y < y)
        y = this->position.y;

    this->position.y = y;
    this->drawPos.y  = y;

    if (!collided) {
        this->velocity.x <<= 1;
        if (!this->velocity.x) {
            this->state.Set(&Platform::State_Push_Fall);
        }
        else {
            if (this->velocity.x > 0)
                this->state.Set(&Platform::State_Push_SlideOffL);
            else
                this->state.Set(&Platform::State_Push_SlideOffR);
        }
    }

    this->TileCollision(Zone::sVars->collisionLayers, CMODE_RWALL, 0, this->hitbox.left << 16, (this->hitbox.bottom - 8) << 16, true);
    this->TileCollision(Zone::sVars->collisionLayers, CMODE_LWALL, 0, this->hitbox.right << 16, (this->hitbox.bottom - 8) << 16, true);
    this->TileCollision(Zone::sVars->collisionLayers, CMODE_RWALL, 0, this->hitbox.left << 16, (this->hitbox.top + 8) << 16, true);
    this->TileCollision(Zone::sVars->collisionLayers, CMODE_LWALL, 0, this->hitbox.right << 16, (this->hitbox.top + 8) << 16, true);

    this->drawPos.x = this->position.x;
    this->drawPos.y = this->position.y;
    if (!this->CheckOnScreen(&this->updateRange)) {
        this->speed >>= 11;
        this->drawPos.x  = this->centerPos.x;
        this->drawPos.y  = this->centerPos.y;
        this->velocity.x = 0;
        this->velocity.y = 0;
        this->visible    = false;
        if (sVars->playingPushSfx) {
            sVars->sfxPush.Stop();
            sVars->playingPushSfx = false;
        }
        this->state.Set(&Platform::State_Hold);
    }
}

void Platform::State_Hold()
{
    SET_CURRENT_STATE();

    if (!this->CheckOnScreen(&this->updateRange))
        Platform::Create(nullptr);

    this->velocity.x = 0;
    this->velocity.y = 0;
}

void Platform::State_Track()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;

    int32 move      = Zone::sVars->timer * (this->speed << 7);
    int32 direction = ((move >> 16) & 1);
    if (this->angle) {
        if (this->stoodPlayers & 1) {
            this->timer++;
        }
        else if (this->timer >= 1) {
            if (this->timer == 1)
                this->timer = 0;
            else
                this->timer -= 2;
        }

        move = this->timer * (this->speed << 7);
        if (!(move & 0x10000) && (this->stoodPlayers & 1) && direction) {
            direction = FLIP_X;
            this->timer--;
        }
    }

    if (direction == this->direction) {
        this->drawPos.x = this->centerPos.x + ((move & 0xFFFF) * this->amplitude.x >> 6) - (this->amplitude.x << 9);
        this->drawPos.y = this->centerPos.y + ((move & 0xFFFF) * this->amplitude.y >> 6) - (this->amplitude.y << 9);
    }
    else {
        this->drawPos.x = this->centerPos.x + (this->amplitude.x << 9) - ((move & 0xFFFF) * this->amplitude.x >> 6);
        this->drawPos.y = this->centerPos.y + (this->amplitude.y << 9) - ((move & 0xFFFF) * this->amplitude.y >> 6);
    }

    if (((move >> 16) & 1) != this->hasTension) {
        if (this->onScreen)
            sVars->sfxClang.Play();

        this->hasTension = (move >> 16) & 1;
    }

    this->velocity.x = this->drawPos.x + drawX;
    this->velocity.y = this->drawPos.y + drawY;

    if (this->zdepth == 999) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if ((1 << player->playerID) & stoodPlayers)
                this->collisionOffset.x = -0x10000 * (this->drawPos.x >> 16);
        }
    }
}

void Platform::State_Falling2()
{
    SET_CURRENT_STATE();

    this->drawPos.y += this->velocity.y;
    this->velocity.y += 0x3800;

    this->position.x = this->drawPos.x;
    this->position.y = this->drawPos.y;

    if (this->CheckOnScreen(&this->updateRange)) {
        this->position.x = this->centerPos.x;
        this->position.y = this->centerPos.y;
        this->velocity.x = 0;
    }
    else {
        if (this->amplitude.x) {
            this->Destroy();
        }
        else {
            this->drawPos.x  = this->centerPos.x;
            this->drawPos.y  = this->centerPos.y;
            this->velocity.y = 0;
            this->visible    = false;
            this->state.Set(&Platform::State_Hold);
        }

        this->velocity.x = 0;
    }
}

void Platform::State_Fall()
{
    SET_CURRENT_STATE();

    if (this->timer) {
        if (!--this->timer || this->angle) {
            this->active = ACTIVE_NORMAL;
            this->state.Set(&Platform::State_Falling);
            this->timer = 30;
        }
    }

    this->drawPos.x = this->centerPos.x;
    this->drawPos.y = this->centerPos.y;
    this->drawPos.y += Math::Sin256(this->stoodAngle) << 10;

    this->velocity.x = 0;
}

void Platform::State_Circular()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;

    this->drawPos.x = this->amplitude.x * Math::Cos1024(this->speed * Zone::sVars->timer + 4 * this->angle) + this->centerPos.x;
    this->drawPos.y = this->amplitude.y * Math::Sin1024(this->speed * Zone::sVars->timer + 4 * this->angle) + this->centerPos.y;

    this->velocity.x = this->drawPos.x + drawX;
    this->velocity.y = this->drawPos.y + drawY;
}

void Platform::State_SwingReact()
{
    SET_CURRENT_STATE();

    this->velocity.x = 0;
    this->velocity.y = 0;

    if (this->stood) {
        this->state.Set(&Platform::State_Swing2);
    }
}

void Platform::State_TrackReact()
{
    SET_CURRENT_STATE();

    this->velocity.x = 0;
    this->velocity.y = 0;

    if (this->stood)
        this->state.Set(&Platform::State_Track2);
}

void Platform::State_Clacker()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;

    int32 prevAngle = this->angle;
    int32 maxAngle  = this->groundVel + 0x100;
    this->angle     = this->groundVel + ((this->amplitude.x * Math::Sin1024(this->speed * Zone::sVars->timer) + 0x200) >> 14) + 0x100;

    if (!this->hasTension) {
        if (this->angle >= maxAngle) {
            if (!this->onScreen || prevAngle >= maxAngle) {
                this->angle = this->groundVel + 0x100;
            }
            else {
                sVars->sfxClacker.Play();
                this->angle = this->groundVel + 0x100;
            }
        }
    }
    else if (this->angle <= maxAngle) {
        if (!this->onScreen || prevAngle <= maxAngle) {
            this->angle = this->groundVel + 0x100;
        }
        else {
            sVars->sfxClacker.Play();
            this->angle = this->groundVel + 0x100;
        }
    }

    this->drawPos.x  = this->amplitude.y * Math::Cos1024(this->angle) + this->centerPos.x;
    this->drawPos.y  = this->amplitude.y * Math::Sin1024(this->angle) + this->centerPos.y;
    this->velocity.x = this->drawPos.x + drawX;
    this->velocity.y = this->drawPos.y + drawY;
}

void Platform::State_Child()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;

    this->drawPos.x = this->centerPos.x;
    this->drawPos.y = this->centerPos.y;

    this->velocity.x = this->drawPos.x + drawX;
    this->velocity.y = this->drawPos.y + drawY;
}

void Platform::State_Push_SlideOffL()
{
    SET_CURRENT_STATE();

    this->drawPos.x += this->velocity.x;

    int32 storeX = this->position.x;
    int32 storeY = this->position.y;

    this->position.x = this->drawPos.x;
    this->position.y = this->drawPos.y;
    if (!this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, this->hitbox.left << 16, this->hitbox.bottom << 16, false))
        this->state.Set(&Platform::State_Push_Fall);

    this->position.x = storeX;
    this->position.y = storeY;
}

void Platform::State_Push_SlideOffR()
{
    SET_CURRENT_STATE();

    this->drawPos.x += this->velocity.x;

    int32 storeX = this->position.x;
    int32 storeY = this->position.y;

    this->position.x = this->drawPos.x;
    this->position.y = this->drawPos.y;

    if (!this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, this->hitbox.right << 16, this->hitbox.bottom << 16, false))
        this->state.Set(&Platform::State_Push_Fall);

    this->position.x = storeX;
    this->position.y = storeY;
}

void Platform::State_Push_Fall()
{
    SET_CURRENT_STATE();

    int32 storeX = this->position.x;
    int32 storeY = this->position.y;

    this->drawPos.y += this->velocity.y;
    this->velocity.y += 0x3800;

    this->position.x = this->drawPos.x;
    this->position.y = this->drawPos.y;

    bool32 collided = this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, (this->hitbox.left + 16) << 16, this->hitbox.bottom << 16, 4);
    int32 y         = this->position.y;

    collided |= this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, this->hitbox.bottom << 16, 4);
    if (this->position.y < y)
        y = this->position.y;

    collided |= this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, (this->hitbox.right - 16) << 16, this->hitbox.bottom << 16, 4);
    if (this->position.y < y)
        y = this->position.y;

    if (collided) {
        this->velocity.y = 0;
        this->state.Set(&Platform::State_Push);
    }
    else {
        this->position.y = y;
        this->drawPos.y  = y;
    }

    this->position.x = storeX;
    this->position.y = storeY;

    this->velocity.x = 0;
}

void Platform::State_Path()
{
    SET_CURRENT_STATE();

    this->drawPos.x += this->velocity.x;
    this->drawPos.y += this->velocity.y;
    PlatformNode *node = GameObject::Get<PlatformNode>(this->speed);

    if (this->velocity.y <= 0) {
        if (this->drawPos.y < node->position.y)
            this->drawPos.y = node->position.y;
    }
    else {
        if (this->drawPos.y > node->position.y)
            this->drawPos.y = node->position.y;
    }

    if (this->velocity.x <= 0) {
        if (this->drawPos.x < node->position.x)
            this->drawPos.x = node->position.x;
    }
    else {
        if (this->drawPos.x > node->position.x)
            this->drawPos.x = node->position.x;
    }
}

void Platform::State_ReactSlow()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;

    this->amplitude.y += this->groundVel;
    this->drawPos.x = (this->amplitude.y >> 8) * Math::Cos256(this->angle) + this->centerPos.x;
    this->drawPos.y = (this->amplitude.y >> 8) * Math::Sin256(this->angle) + this->centerPos.y;

    if (this->groundVel <= 0) {
        this->centerPos.x = this->drawPos.x - this->tileOrigin.x;
        this->centerPos.y = this->drawPos.y - this->tileOrigin.y;
        this->groundVel   = 0;
        this->amplitude.y = 0;
        this->angle       = -this->angle;

        if (this->timer == -1) {
            this->timer = 0;
            if (this->type == Platform::HoverReact)
                this->state.Set(&Platform::State_Hover_React);
            else
                this->state.Set(&Platform::State_React);
        }
        else {
            if (this->hasTension) {
                this->state.Set(&Platform::State_ReactWait);
            }
            else {
                this->active = ACTIVE_BOUNDS;
                if (this->type == Platform::HoverReact)
                    this->state.Set(&Platform::State_Hover);
                else
                    this->state.Set(&Platform::State_Fixed);
            }
        }
    }
    else {
        this->groundVel -= this->speed << 10;
    }

    this->velocity.x = drawX + this->drawPos.x;
    this->velocity.y = drawY + this->drawPos.y;
}

void Platform::State_Hover()
{
    SET_CURRENT_STATE();

    this->rotation += 4;
    this->drawPos.y  = (Math::Sin1024(this->rotation) << 9) + this->centerPos.y;
    this->velocity.x = 0;
    this->velocity.y = 0;
}

void Platform::State_ReactWait()
{
    SET_CURRENT_STATE();

    if (this->type == Platform::HoverReact) {
        this->rotation += 4;
        this->drawPos.y = (Math::Sin1024(this->rotation) << 9) + this->centerPos.y;
    }

    if (this->stood) {
        this->timer      = 120;
        this->velocity.x = 0;
        this->velocity.y = 0;
    }
    else {
        if (--this->timer) {
            this->tileOrigin.x = this->drawPos.x - this->centerPos.x;
            this->tileOrigin.y = this->drawPos.y - this->centerPos.y;
            this->centerPos.x  = this->drawPos.x;
            this->timer        = -1;
            this->centerPos.y  = this->drawPos.y;
            this->active       = ACTIVE_NORMAL;
            this->state.Set(&Platform::State_ReactMove);
        }
        this->velocity.x = 0;
        this->velocity.y = 0;
    }
}

void Platform::State_PathReact()
{
    SET_CURRENT_STATE();

    if (!this->stood) {
        this->velocity.x = 0;
        this->velocity.y = 0;
    }
    else {
        int32 slot                  = sceneInfo->entitySlot - 1;
        PlatformControl *controller = GameObject::Get<PlatformControl>(slot);
        if (controller->classID == PlatformControl::sVars->classID) {
            controller->setActive = true;
            this->state.Set(&Platform::State_Path);
            this->velocity.x = 0;
            this->velocity.y = 0;
        }
        else {
            while (controller->classID == sVars->classID || controller->classID == PlatformNode::sVars->classID) {
                controller = GameObject::Get<PlatformControl>(slot--);
                if (controller->classID == PlatformControl::sVars->classID) {
                    controller->setActive = true;
                    this->state.Set(&Platform::State_Path);
                    this->velocity.x = 0;
                    this->velocity.y = 0;
                    break;
                }
            }
        }
    }
}

void Platform::State_Swing2()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;
    this->angle = this->groundVel + 0x100 + ((this->amplitude.x * Math::Sin1024(this->timer * this->speed) + 0x200) >> 14);

    if (this->timer++ == 256 && this->hasTension) {
        this->timer = 119;
        this->state.Set(&Platform::State_SwingWait);
    }
    this->drawPos.x = this->amplitude.y * Math::Cos1024(this->angle) + this->centerPos.x;
    this->drawPos.y = this->amplitude.y * Math::Sin1024(this->angle) + this->centerPos.y;

    if (this->groundVel < 0)
        this->drawPos.x += 0x200000;
    else
        this->drawPos.x -= 0x200000;

    this->velocity.x = drawX + this->drawPos.x;
    this->velocity.y = drawY + this->drawPos.y;
}

void Platform::State_SwingWait()
{
    SET_CURRENT_STATE();

    if (this->stood) {
        this->timer      = 120;
        this->velocity.x = 0;
        this->velocity.y = 0;
    }
    else {
        if (!--this->timer) {
            this->timer  = 257;
            this->active = ACTIVE_NORMAL;
            this->state.Set(&Platform::State_SwingReturn);
        }
        this->velocity.x = 0;
        this->velocity.y = 0;
    }
}

void Platform::State_SwingReturn()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;
    int32 amp   = this->amplitude.x * Math::Sin1024(this->timer-- * this->speed);
    this->angle = this->groundVel + 0x100 + ((amp + 0x200) >> 14);

    if (this->timer == 88)
        this->state.Set(&Platform::State_SwingReact);

    this->drawPos.x = this->amplitude.y * Math::Cos1024(this->angle) + this->centerPos.x;
    this->drawPos.y = this->amplitude.y * Math::Sin1024(this->angle) + this->centerPos.y;
    if (this->groundVel < 0)
        this->drawPos.x += 0x200000;
    else
        this->drawPos.x -= 0x200000;
    this->velocity.x = drawX + this->drawPos.x;
    this->velocity.y = drawY + this->drawPos.y;
}

void Platform::State_Track2()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;
    int32 move  = this->angle++ * (this->speed << 7);

    if (this->hasTension) {
        if (move < 0x10000) {
            if (move <= 0)
                move = 0;
        }
        else {
            move        = 0x10000;
            this->timer = 120;
            this->state.Set(&Platform::State_TrackWait);
        }
    }

    if (((move >> 16) & 1) == this->direction) {
        this->drawPos.x = this->centerPos.x + ((move & 0xFFFF) * this->amplitude.x >> 6) - (this->amplitude.x << 9);
        this->drawPos.y = this->centerPos.y + ((move & 0xFFFF) * this->amplitude.y >> 6) - (this->amplitude.y << 9);
    }
    else {
        this->drawPos.x = this->centerPos.x + (this->amplitude.x << 9) - ((move & 0xFFFF) * this->amplitude.x >> 6);
        this->drawPos.y = this->centerPos.y + (this->amplitude.y << 9) - ((move & 0xFFFF) * this->amplitude.y >> 6);
    }

    this->velocity.x = drawX + this->drawPos.x;
    this->velocity.y = drawY + this->drawPos.y;
}

void Platform::State_TrackWait()
{
    SET_CURRENT_STATE();

    if (this->stoodPlayers) {
        this->timer      = 120;
        this->velocity.x = 0;
        this->velocity.y = 0;
    }
    else {
        if (!--this->timer) {
            this->active = ACTIVE_NORMAL;
            this->state.Set(&Platform::State_TrackReturn);
        }
        this->velocity.x = 0;
        this->velocity.y = 0;
    }
}

void Platform::State_TrackReturn()
{
    SET_CURRENT_STATE();

    int32 drawX = -this->drawPos.x;
    int32 drawY = -this->drawPos.y;
    int32 move  = this->angle-- * (this->speed << 7);

    if (move > 0) {
        if (move >= 0x10000)
            move = 0x10000;
    }
    else {
        move = 0;
        this->state.Set(&Platform::State_TrackReact);
    }

    if (((move >> 16) & 1) == this->direction) {
        this->drawPos.x = this->centerPos.x + ((move & 0xFFFF) * this->amplitude.x >> 6) - (this->amplitude.x << 9);
        this->drawPos.y = this->centerPos.y + ((move & 0xFFFF) * this->amplitude.y >> 6) - (this->amplitude.y << 9);
    }
    else {
        this->drawPos.x = this->centerPos.x + (this->amplitude.x << 9) - ((move & 0xFFFF) * this->amplitude.x >> 6);
        this->drawPos.y = this->centerPos.y + (this->amplitude.y << 9) - ((move & 0xFFFF) * this->amplitude.y >> 6);
    }

    this->velocity.x = drawX + this->drawPos.x;
    this->velocity.y = drawY + this->drawPos.y;
}

// Collision States
void Platform::Collision_Solid()
{
    Hitbox *solidHitbox    = this->animator.GetHitbox(1);
    Hitbox *platformHitbox = this->animator.GetHitbox(0);
    int32 stoodPlayers     = this->stoodPlayers;

    if (!ScreenWrap::CheckCompetitionWrap() || !ScreenWrap::sVars->handlingWrap) {
        this->stoodPlayers = 0;
        this->pushPlayersL = 0;
        this->pushPlayersR = 0;
    }

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 playerID = player->Slot();

        if (!ScreenWrap::Unknown1(player, this))
            continue;

        player->CheckCollisionPlatform(this, platformHitbox);

        switch (player->CheckCollisionBox(this, solidHitbox)) {
            case C_TOP:
                if (globals->gravityDir == CMODE_FLOOR)
                    HandleStood(player, playerID, stoodPlayers);

                switch (globals->gravityDir) {
                    default: break;

                    case CMODE_FLOOR:
                        if (this->velocity.y <= 0)
                            player->collisionFlagV |= 1;
                        break;

                    case CMODE_ROOF:
                        if (this->velocity.y >= 0)
                            player->collisionFlagV |= 1;
                        break;
                }

                break;

            case C_LEFT:
                if (player->onGround && player->right)
                    this->pushPlayersL |= 1 << playerID;

                if (this->velocity.x <= 0) {
                    if (player->left) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x -= player->acceleration;
                    }
                    player->collisionFlagH |= 1;
                }
                break;

            case C_RIGHT:
                if (player->onGround && player->left)
                    this->pushPlayersR |= 1 << playerID;

                if (this->velocity.x >= 0) {
                    if (player->right) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x += player->acceleration;
                    }
                    player->collisionFlagH |= 2;
                }
                break;

            case C_BOTTOM:
                if (globals->gravityDir == CMODE_ROOF)
                    HandleStood(player, playerID, stoodPlayers);

                if (this->velocity.y >= 0)
                    player->collisionFlagV |= 2;
                break;

            default: break;
        }
    }
}
void Platform::Collision_Hurt()
{
    if (this->timer)
        this->timer--;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->CheckCollisionBox(this, &this->hitbox)) {
            if (!player->CheckMightyUnspin(0x400, this->type == Platform::Circular, &player->uncurlTimer))
                player->Hurt(this);
        }
    }
}
void Platform::Collision_Solid_Hurt_Bottom()
{
    Hitbox *solidHitbox    = this->animator.GetHitbox(1);
    Hitbox *platformHitbox = this->animator.GetHitbox(0);
    int32 stoodPlayers     = this->stoodPlayers;

    this->stoodPlayers = 0;
    this->pushPlayersL = 0;
    this->pushPlayersR = 0;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 playerID = player->Slot();
        player->CheckCollisionPlatform(this, platformHitbox);

        switch (player->CheckCollisionBox(this, solidHitbox)) {
            case C_TOP:
                if (globals->gravityDir == CMODE_FLOOR)
                    HandleStood(player, playerID, stoodPlayers);

                switch (globals->gravityDir) {
                    default: break;

                    case CMODE_FLOOR:
                        if (this->velocity.y <= 0)
                            player->collisionFlagV |= 1;
                        break;

                    case CMODE_ROOF:
                        if (this->velocity.y >= 0)
                            player->collisionFlagV |= 1;
                        break;
                }
                break;

            case C_LEFT:
                if (player->onGround && player->right)
                    this->pushPlayersL |= 1 << playerID;

                if (this->velocity.x <= 0) {
                    if (player->left) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x -= player->acceleration;
                    }
                    player->collisionFlagH |= 1;
                }
                break;

            case C_RIGHT:
                if (player->onGround && player->left)
                    this->pushPlayersR |= 1 << playerID;

                if (this->velocity.x >= 0) {
                    if (player->right) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x += player->acceleration;
                    }
                    player->collisionFlagH |= 2;
                }
                break;

            case C_BOTTOM:
                if (globals->gravityDir == CMODE_ROOF)
                    HandleStood(player, playerID, stoodPlayers);

                if (this->velocity.y >= 0)
                    player->collisionFlagV |= 2;

                if (!player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer))
                    player->Hurt(this);
                break;

            default: break;
        }
    }
}
void Platform::Collision_Solid_Hurt_Sides()
{
    Hitbox *solidHitbox = this->animator.GetHitbox(1);
    int32 stoodPlayers  = this->stoodPlayers;

    this->stoodPlayers = 0;
    this->pushPlayersL = 0;
    this->pushPlayersR = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        uint16 playerID = player->Slot();

        switch (player->CheckCollisionBox(this, solidHitbox)) {
            case C_TOP:
                if (globals->gravityDir == CMODE_ROOF)
                    HandleStood(player, playerID, stoodPlayers);

                switch (globals->gravityDir) {
                    default:

                    case CMODE_FLOOR:
                        if (this->velocity.y <= 0)
                            player->collisionFlagV |= 1;
                        break;

                    case CMODE_ROOF:
                        if (this->velocity.y >= 0)
                            player->collisionFlagV |= 1;
                        break;
                }

                break;

            case C_LEFT:
                if (player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer) || (player->Hurt(this) && this->type == Platform::Fixed)) {
                    player->velocity.x += this->velocity.x;
                }

                if (!player->blinkTimer || player->velocity.x >= this->velocity.x) {
                    if (player->onGround && player->right)
                        this->pushPlayersL |= 1 << playerID;

                    if (this->velocity.x < 0 && player->velocity.x >= this->velocity.x && player->left) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x -= player->acceleration;
                    }
                }

                if (this->velocity.x <= 0)
                    player->collisionFlagH |= 1;
                break;

            case C_RIGHT:
                if (player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer) || (player->Hurt(this) && this->type == Platform::Fixed)) {
                    player->velocity.x += this->velocity.x;
                }

                if (!player->blinkTimer || player->velocity.x <= this->velocity.x) {
                    if (player->onGround && player->left)
                        this->pushPlayersR |= 1 << playerID;

                    if (this->velocity.x > 0 && player->velocity.x <= this->velocity.x && player->right) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x += player->acceleration;
                    }
                }

                if (this->velocity.x >= 0)
                    player->collisionFlagH |= 2;
                break;

            case C_BOTTOM:
                if (globals->gravityDir == CMODE_ROOF)
                    HandleStood(player, playerID, stoodPlayers);

                if (this->velocity.y >= 0)
                    player->collisionFlagV |= 2;
                break;

            default: break;
        }
    }
}
void Platform::Collision_Tiles()
{
    this->stoodPlayers = 0;
    this->pushPlayersL = 0;
    this->pushPlayersR = 0;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 playerID = player->Slot();

        Hitbox hitbox;
        hitbox.left   = this->hitbox.left - 16;
        hitbox.top    = this->hitbox.top - 16;
        hitbox.right  = this->hitbox.right + 16;
        hitbox.bottom = this->hitbox.bottom + 16;

        if (player->CheckCollisionTouch(this, &hitbox)) {
            player->collisionLayers |= Zone::sVars->moveLayerMask;
            player->moveLayerPosition.x = this->tileOrigin.x - this->drawPos.x;
            player->moveLayerPosition.y = this->tileOrigin.y - this->drawPos.y;

            if (player->animator.animationID == Player::ANI_PUSH && player->onGround) {
                if (player->right)
                    this->pushPlayersL |= 1 << playerID;

                if (player->onGround && player->left)
                    this->pushPlayersR |= 1 << playerID;
            }

            bool32 isClimbing = false;
            if (player->state.Matches(&Player::State_KnuxWallClimb) || player->state.Matches(&Player::State_KnuxLedgePullUp)) {
                isClimbing = true;

                if (player->state.Matches(&Player::State_KnuxLedgePullUp))
                    hitbox.top -= 16;

                if (player->position.x >= this->position.x) {
                    hitbox.top += 16;
                    hitbox.left += 16;
                }
                else {
                    hitbox.top += 16;
                    hitbox.right -= 16;
                }

                hitbox.bottom -= 16;
            }
            else {
                switch (player->collisionMode) {
                    case CMODE_FLOOR:
                        hitbox.right -= 16;
                        hitbox.left += 16;
                        hitbox.bottom -= 16;
                        break;

                    case CMODE_LWALL:
                        hitbox.top += 16;
                        hitbox.right -= 16;
                        hitbox.bottom -= 16;
                        break;

                    case CMODE_ROOF:
                        hitbox.top += 16;
                        hitbox.left += 16;
                        hitbox.right -= 16;
                        break;

                    case CMODE_RWALL:
                        hitbox.top += 16;
                        hitbox.left += 16;
                        hitbox.bottom -= 16;
                        break;

                    default: break;
                }
            }

            if (player->CheckCollisionTouch(this, &hitbox) && (player->onGround || isClimbing))
                HandleStood_Tiles(player, playerID);
        }
    }
}
void Platform::Collision_Sticky()
{
    Hitbox *solidHitbox    = this->animator.GetHitbox(1);
    Hitbox *platformHitbox = this->animator.GetHitbox(0);

    this->pushPlayersL = 0;
    this->pushPlayersR = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        uint16 playerID = player->Slot();
        player->CheckCollisionPlatform(this, platformHitbox);

        int32 side = player->CheckCollisionBox(this, solidHitbox);

        HandleStood_Sticky(player, playerID, side);
    }
}
void Platform::Collision_Solid_Hurt_Top()
{
    Hitbox *solidHitbox = this->animator.GetHitbox(1);
    int32 stoodPlayers  = this->stoodPlayers;

    this->stoodPlayers = 0;
    this->pushPlayersL = 0;
    this->pushPlayersR = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        uint16 playerID = player->Slot();

        switch (player->CheckCollisionBox(this, solidHitbox)) {
            case C_TOP:
                if (globals->gravityDir == CMODE_FLOOR)
                    HandleStood(player, playerID, stoodPlayers);

                if (!player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer))
                    player->Hurt(this);

                switch (globals->gravityDir) {
                    default: break;

                    case CMODE_FLOOR:
                        if (this->velocity.y <= 0)
                            player->collisionFlagV |= 1;
                        break;

                    case CMODE_ROOF:
                        if (this->velocity.y >= 0)
                            player->collisionFlagV |= 1;
                        break;
                }
                break;

            case C_LEFT:
                if (player->onGround && player->right)
                    this->pushPlayersL |= 1 << playerID;

                if (this->velocity.x <= 0) {
                    if (player->left) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x -= player->acceleration;
                    }
                    player->collisionFlagH |= 1;
                }
                break;

            case C_RIGHT:
                if (player->onGround && player->left)
                    this->pushPlayersR |= 1 << playerID;

                if (this->velocity.x >= 0) {
                    if (player->right) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x += player->acceleration;
                    }
                    player->collisionFlagH |= 2;
                }
                break;

            case C_BOTTOM:
                if (globals->gravityDir == CMODE_ROOF)
                    HandleStood(player, playerID, stoodPlayers);

                if (this->velocity.y >= 0)
                    player->collisionFlagV |= 2;
                break;

            default: break;
        }
    }
}
void Platform::Collision_Platform()
{
    SET_CURRENT_STATE();

    Hitbox *platformHitbox = this->animator.GetHitbox(0);
    int32 stoodPlayers     = this->stoodPlayers;

    this->stoodPlayers = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->shield >= Shield::Unknown)
            continue;

        uint16 playerID = player->Slot();
        int32 yVel      = player->velocity.y;

        switch (globals->gravityDir) {
            default: break;

            case CMODE_FLOOR:
                if (this->collisionOffset.y < 0)
                    player->velocity.y -= this->collisionOffset.y;
                break;

            case CMODE_ROOF:
                if (this->collisionOffset.y > 0)
                    player->velocity.y -= this->collisionOffset.y;
                break;
        }

        if (player->CheckCollisionPlatform(this, platformHitbox))
            HandleStood(player, playerID, stoodPlayers);
        else
            player->velocity.y = yVel;
    }
}
void Platform::Collision_Solid_Barrel()
{
    Hitbox *solidHitbox    = this->animator.GetHitbox(1);
    Hitbox *platformHitbox = this->animator.GetHitbox(0);
    int32 stoodPlayers     = this->stoodPlayers;

    this->stoodPlayers = 0;
    this->pushPlayersL = 0;
    this->pushPlayersR = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        uint16 playerID = player->Slot();
        player->CheckCollisionPlatform(this, platformHitbox);

        switch (player->CheckCollisionBox(this, solidHitbox)) {
            case C_TOP:
                if (player->characterID != ID_MIGHTY || !player->state.Matches(&Player::State_MightyHammerDrop)) {
                    if (globals->gravityDir == CMODE_FLOOR)
                        HandleStood_Barrel(player, playerID, stoodPlayers);

                    switch (globals->gravityDir) {
                        default: break;

                        case CMODE_FLOOR:
                            if (this->velocity.y <= 0)
                                player->collisionFlagV |= 1;
                            break;

                        case CMODE_ROOF:
                            if (this->velocity.y >= 0)
                                player->collisionFlagV |= 1;
                            break;
                    }
                }
                break;

            case C_LEFT:
                if (player->onGround && player->right)
                    this->pushPlayersL |= 1 << playerID;

                if (this->velocity.x <= 0) {
                    if (player->left) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x -= player->acceleration;
                    }

                    player->collisionFlagH |= 1;
                }
                break;

            case C_RIGHT:
                if (player->onGround && player->left)
                    this->pushPlayersR |= 1 << playerID;

                if (this->velocity.x >= 0) {
                    if (player->right) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x += player->acceleration;
                    }

                    player->collisionFlagH |= 2;
                }
                break;

            case C_BOTTOM:
                if (player->characterID != ID_MIGHTY || !player->state.Matches(&Player::State_MightyHammerDrop)) {
                    if (globals->gravityDir == CMODE_ROOF)
                        HandleStood_Barrel(player, playerID, stoodPlayers);

                    if (this->velocity.y >= 0)
                        player->collisionFlagV |= 2;
                }
                break;

            default: break;
        }
    }
}
void Platform::Collision_Solid_Hold()
{
    Hitbox *solidHitbox    = this->animator.GetHitbox(1);
    Hitbox *platformHitbox = this->animator.GetHitbox(0);
    int32 stoodPlayers     = this->stoodPlayers;

    this->stoodPlayers = 0;
    this->pushPlayersL = 0;
    this->pushPlayersR = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        uint16 playerID = player->Slot();
        player->CheckCollisionPlatform(this, platformHitbox);

        switch (player->CheckCollisionBox(this, solidHitbox)) {
            case C_NONE:
                if ((1 << playerID) & stoodPlayers) {
                    if (player->state.Matches(&Player::State_Static)) {
                        player->state.Set(&Player::State_Air);
                        player->animator.SetAnimation(player->aniFrames, Player::ANI_FALL, false, 0);
                    }
                }
                break;

            case C_TOP:
                if (player->characterID != ID_MIGHTY || !player->state.Matches(&Player::State_MightyHammerDrop)) {
                    if (globals->gravityDir == CMODE_FLOOR)
                        HandleStood_Hold(player, playerID, stoodPlayers);

                    switch (globals->gravityDir) {
                        default: break;

                        case CMODE_FLOOR:
                            if (this->velocity.y <= 0)
                                player->collisionFlagV |= 1;
                            break;

                        case CMODE_ROOF:
                            if (this->velocity.y >= 0)
                                player->collisionFlagV |= 1;
                            break;
                    }
                }
                break;

            case C_LEFT:
                if (player->onGround && player->right)
                    this->pushPlayersL |= 1 << playerID;

                if (this->velocity.x <= 0) {
                    if (player->left) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x -= player->acceleration;
                    }

                    player->collisionFlagH |= 1;
                }
                break;

            case C_RIGHT:
                if (player->onGround && player->left)
                    this->pushPlayersR |= 1 << playerID;

                if (this->velocity.x >= 0) {
                    if (player->right) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x += player->acceleration;
                    }

                    player->collisionFlagH |= 2;
                }

                break;

            case C_BOTTOM:
                if (player->characterID != ID_MIGHTY || !player->state.Matches(&Player::State_MightyHammerDrop)) {
                    if (globals->gravityDir == CMODE_ROOF)
                        HandleStood_Hold(player, playerID, stoodPlayers);

                    if (this->velocity.y >= 0)
                        player->collisionFlagV |= 2;
                }
                break;

            default: break;
        }
    }
}
void Platform::Collision_None()
{
    // hehe
}
void Platform::Collision_Solid_NoCrush()
{
    Hitbox *solidHitbox    = this->animator.GetHitbox(1);
    Hitbox *platformHitbox = this->animator.GetHitbox(0);
    int32 stoodPlayers     = this->stoodPlayers;

    this->stoodPlayers = 0;
    this->pushPlayersL = 0;
    this->pushPlayersR = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        uint16 playerID = player->Slot();
        player->CheckCollisionPlatform(this, platformHitbox);

        switch (player->CheckCollisionBox(this, solidHitbox)) {
            default:
            case C_NONE: break;

            case C_TOP:
                if (globals->gravityDir == CMODE_FLOOR)
                    HandleStood(player, playerID, stoodPlayers);
                break;

            case C_LEFT:
                if (player->onGround && player->right)
                    this->pushPlayersL |= 1 << playerID;

                if (this->velocity.x <= 0 && player->left) {
                    player->groundVel  = this->velocity.x;
                    player->velocity.x = player->groundVel;
                    player->velocity.x -= player->acceleration;
                }
                break;

            case C_RIGHT:
                if (player->onGround && player->left)
                    this->pushPlayersR |= 1 << playerID;

                if (this->velocity.x >= 0) {
                    if (player->right) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x += player->acceleration;
                    }
                }
                break;

            case C_BOTTOM:
                if (globals->gravityDir == CMODE_ROOF)
                    HandleStood(player, playerID, stoodPlayers);
                break;
        }
    }
}
void Platform::Collision_Solid_Hurt_All()
{
    Hitbox *solidHitbox = this->animator.GetHitbox(1);
    int32 stoodPlayers  = this->stoodPlayers;

    this->stoodPlayers = 0;
    this->pushPlayersL = 0;
    this->pushPlayersR = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        uint16 playerID = player->Slot();

        switch (player->CheckCollisionBox(this, solidHitbox)) {
            case C_TOP:
                if (globals->gravityDir == CMODE_FLOOR)
                    HandleStood(player, playerID, stoodPlayers);

                if (!player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer))
                    player->Hurt(this);

                switch (globals->gravityDir) {
                    default: break;

                    case CMODE_FLOOR:
                        if (this->velocity.y <= 0)
                            player->collisionFlagV |= 1;
                        break;

                    case CMODE_ROOF:
                        if (this->velocity.y >= 0)
                            player->collisionFlagV |= 1;
                        break;
                }
                break;

            case C_LEFT:
                if (player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer) || (player->Hurt(this) && this->type == Platform::Fixed)) {
                    player->velocity.x += this->velocity.x;
                }

                if (!player->blinkTimer || player->velocity.x >= this->velocity.x) {
                    if (player->onGround && player->right)
                        this->pushPlayersL |= 1 << playerID;

                    if (this->velocity.x < 0 && player->velocity.x >= this->velocity.x && player->left) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x -= player->acceleration;
                    }
                }

                if (this->velocity.x <= 0)
                    player->collisionFlagH |= 1;
                break;

            case C_RIGHT:
                if (player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer) || (player->Hurt(this) && this->type == Platform::Fixed)) {
                    player->velocity.x += this->velocity.x;
                }

                if (!player->blinkTimer || player->velocity.x <= this->velocity.x) {
                    if (player->onGround && player->left)
                        this->pushPlayersR |= 1 << playerID;

                    if (this->velocity.x > 0 && player->velocity.x <= this->velocity.x && player->right) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x += player->acceleration;
                    }
                }

                if (this->velocity.x >= 0)
                    player->collisionFlagH |= 2;
                break;

            case C_BOTTOM:
                if (globals->gravityDir == CMODE_ROOF)
                    HandleStood(player, playerID, stoodPlayers);

                if (this->velocity.y >= 0)
                    player->collisionFlagV |= 2;

                if (!player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer))
                    player->Hurt(this);
                break;

            default: break;
        }
    }
}
void Platform::Collision_Solid_Hurt_NoCrush()
{
    Hitbox *solidHitbox = this->animator.GetHitbox(1);
    int32 stoodPlayers  = this->stoodPlayers;

    this->stoodPlayers = 0;
    this->pushPlayersL = 0;
    this->pushPlayersR = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        uint16 playerID = player->Slot();

        switch (player->CheckCollisionBox(this, solidHitbox)) {
            case C_TOP:
                if (globals->gravityDir == CMODE_FLOOR)
                    HandleStood(player, playerID, stoodPlayers);

                if (!player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer))
                    player->Hurt(this);
                break;

            case C_LEFT:
                if (player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer) || (player->Hurt(this) && this->type == Platform::Fixed)) {
                    player->velocity.x += this->velocity.x;
                }

                if (!player->blinkTimer || player->velocity.x >= this->velocity.x) {
                    if (player->onGround && player->right)
                        this->pushPlayersL |= 1 << playerID;

                    if (this->velocity.x < 0 && player->velocity.x >= this->velocity.x && player->left) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x -= player->acceleration;
                    }
                }
                break;

            case C_RIGHT:
                if (player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer) || (player->Hurt(this) && this->type == Platform::Fixed)) {
                    player->velocity.x += this->velocity.x;
                }

                if (!player->blinkTimer || player->velocity.x <= this->velocity.x) {
                    if (player->onGround && player->left)
                        this->pushPlayersR |= 1 << playerID;

                    if (this->velocity.x > 0 && player->velocity.x <= this->velocity.x && player->right) {
                        player->groundVel  = this->velocity.x;
                        player->velocity.x = player->groundVel;
                        player->velocity.x += player->acceleration;
                    }
                }
                break;

            case C_BOTTOM:
                if (globals->gravityDir == CMODE_FLOOR)
                    HandleStood(player, playerID, stoodPlayers);

                if (!player->CheckMightyUnspin(0x400, 0, &player->uncurlTimer))
                    player->Hurt(this);
                break;

            default: break;
        }
    }
}

void Platform::HandleStood(Player *player, int32 playerID, int32 stoodPlayers)
{
    this->stood = true;
    if (!((1 << playerID) & stoodPlayers) && !player->sidekick && this->state.Matches(&Platform::State_Fall) && !this->timer) {
        if (player->characterID == ID_MIGHTY && player->state.Matches(&Player::State_MightyHammerDrop))
            this->timer = 1;
        else
            this->timer = 30;
    }

    this->stoodPlayers |= 1 << playerID;

    if (sVars->stoodPos[playerID].x) {
        player->position.x = sVars->stoodPos[playerID].x;
        player->position.y = sVars->stoodPos[playerID].y;
    }
    else {
        sVars->stoodPos[playerID].x = player->position.x;
        sVars->stoodPos[playerID].y = player->position.y;
    }

    player->position.x += this->collisionOffset.x;
    player->position.y += this->collisionOffset.y;
    player->position.y &= 0xFFFF0000;
}
void Platform::HandleStood_Tiles(Player *player, int32 playerID)
{
    this->stoodPlayers |= 1 << playerID;

    if (!player->sidekick) {
        this->stood = true;
        if (this->state.Matches(&Platform::State_Fall) && !this->timer) {
            if (player->characterID == ID_MIGHTY && player->state.Matches(&Player::State_MightyHammerDrop))
                this->timer = 1;
            else
                this->timer = 30;
        }
    }

    player->position.x += this->collisionOffset.x;
    player->position.y += this->collisionOffset.y;
    player->position.y &= 0xFFFF0000;
}
void Platform::HandleStood_Hold(Player *player, int32 playerID, int32 stoodPlayers)
{
    if (player->characterID != ID_MIGHTY || !player->state.Matches(&Player::State_MightyHammerDrop)) {
        if (!((1 << playerID) & stoodPlayers)) {
            player->state.Set(&Player::State_Static);
            player->nextGroundState.Set(nullptr);
            player->nextAirState.Set(nullptr);
            player->stateGravity.Set(&Player::Gravity_NULL);
            player->velocity.x = 0;
            player->velocity.y = 0;
            player->groundVel  = 0;

            if (this->classID == sVars->classID) {
                int32 prevBottom = player->GetHitbox()->bottom;
                player->animator.SetAnimation(player->aniFrames, Player::ANI_TWISTER, false, (player->direction << 31 >> 31) & 6);

                player->position.y += (prevBottom - player->GetHitbox()->bottom) << 16;
            }

            player->animator.speed = 64;
            player->direction      = FLIP_X;
        }

        if (globals->gravityDir == CMODE_FLOOR)
            HandleStood(player, playerID, stoodPlayers);

        if ((player->position.x ^ this->position.x) & 0xFFFF0000) {
            if (player->position.x >= this->position.x)
                player->position.x -= 0x10000;
            else
                player->position.x += 0x10000;
        }

        if (player->jumpPress)
            player->Action_Jump();
    }
}
void Platform::HandleStood_Barrel(Player *player, int32 playerID, int32 stoodPlayers)
{
    if (!((1 << playerID) & stoodPlayers)) {
        player->state.Set(&Player::State_Static);
        player->nextGroundState.Set(nullptr);
        player->nextAirState.Set(nullptr);
        player->stateGravity.Set(&Player::Gravity_NULL);
        player->velocity.x = 0;
        player->velocity.y = 0;
        player->groundVel  = 0;

        if (this->classID == sVars->classID)
            player->animator.SetAnimation(player->aniFrames, Player::ANI_TURNTABLE, false, 0);

        player->animator.speed = 64;
        player->direction      = FLIP_NONE;

        if (!player->sidekick && this->state.Matches(&Platform::State_Fall) && !this->timer) {
            if (player->characterID == ID_MIGHTY && player->state.Matches(&Player::State_MightyHammerDrop))
                this->timer = 1;
            else
                this->timer = 30;
        }
    }

    HandleStood(player, playerID, stoodPlayers);

    if ((player->position.x ^ this->position.x) & 0xFFFF0000) {
        if (player->position.x >= this->position.x)
            player->position.x -= 0x10000;
        else
            player->position.x += 0x10000;
    }

    if (player->jumpPress)
        player->Action_Jump();
}
void Platform::HandleStood_Sticky(Player *player, int32 playerID, uint8 cSide)
{
    bool32 isStuck = false;
    if ((this->collision == Platform::C_SolidSticky || this->collision == cSide + Platform::C_SolidSticky) && cSide)
        isStuck = true;

    if (!player->state.Matches(&Player::State_Static) && isStuck) {
        player->state.Set(&Player::State_Static);
        player->nextGroundState.Set(nullptr);
        player->nextAirState.Set(nullptr);
        player->stateGravity.Set(&Player::Gravity_NULL);
        player->velocity.x     = 0;
        player->velocity.y     = 0;
        player->groundVel      = 0;
        player->animator.speed = 0;

        switch (cSide) {
            case C_TOP: player->angle = 0x00; break;

            case C_LEFT:
                player->angle    = 0xC0;
                player->onGround = false;
                break;

            case C_RIGHT: player->angle = 0x40; break;

            case C_BOTTOM: player->angle = 0x80; break;

            default: break;
        }

        player->tileCollisions = TILECOLLISION_NONE;
        if (!player->sidekick) {
            this->stood = true;
            if (this->state.Matches(&Platform::State_Fall) && !this->timer) {
                if (player->characterID == ID_MIGHTY && player->state.Matches(&Player::State_MightyHammerDrop))
                    this->timer = 1;
                else
                    this->timer = 30;
            }
        }

        this->stoodPlayers |= 1 << playerID;
    }
    else if ((1 << playerID) & this->stoodPlayers) {
        if (player->state.Matches(&Player::State_Static)) {
            if (sVars->stoodPos[playerID].x) {
                player->position.x = sVars->stoodPos[playerID].x;
                player->position.y = sVars->stoodPos[playerID].y;
            }
            else {
                sVars->stoodPos[playerID].x = player->position.x;
                sVars->stoodPos[playerID].y = player->position.y;
            }

            player->position.x += this->collisionOffset.x;
            player->position.y += this->collisionOffset.y;
            player->position.y &= 0xFFFF0000;

            if (player->jumpPress) {
                player->tileCollisions = globals->tileCollisionMode;
                player->Action_Jump();
            }
        }
        else {
            this->stoodPlayers = this->stoodPlayers & ~(1 << playerID);
        }
    }
    else if (cSide == C_TOP) {
        if (sVars->stoodPos[playerID].x) {
            player->position.x = sVars->stoodPos[playerID].x;
            player->position.y = sVars->stoodPos[playerID].y;
        }
        else {
            sVars->stoodPos[playerID].x = player->position.x;
            sVars->stoodPos[playerID].y = player->position.y;
        }

        player->position.x += this->collisionOffset.x;
        player->position.y += this->collisionOffset.y;
        player->position.y &= 0xFFFF0000;
    }
}

#if RETRO_INCLUDE_EDITOR
void Platform::EditorDraw()
{
    int32 frameID = this->frameID;
    Vector2 drawPos;

    switch (this->type) {
        case Platform::Linear:
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->animator.frameID = frameID;
            this->animator.DrawSprite(nullptr, false);

            drawPos.x = this->amplitude.x + this->position.x;
            drawPos.y = this->amplitude.y + this->position.y;
            this->animator.DrawSprite(&drawPos, false);

            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0x00, INK_NONE, false);

            drawPos.x = this->position.x - this->amplitude.x;
            drawPos.y = this->position.y - this->amplitude.y;
            this->animator.DrawSprite(&drawPos, false);

            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0x00, INK_NONE, false);
            break;

        case Platform::Circular:
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->animator.frameID = frameID;
            this->animator.DrawSprite(nullptr, false);

            drawPos.x = this->position.x + (this->amplitude.x >> 8) * Math::Cos256(this->angle);
            drawPos.y = this->position.y + (this->amplitude.y >> 8) * Math::Sin256(this->angle);
            this->animator.DrawSprite(&drawPos, false);

            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0x00, INK_NONE, false);
            break;

        case Platform::Swing:
        case Platform::Clacker: {
            int32 amplitude = abs(this->amplitude.x >> 16);
            if (!amplitude)
                amplitude = 1;

            this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);

            int32 angle       = 0x40 - amplitude + this->angle;
            int32 targetAngle = amplitude + 0x40 + this->angle;
            while (angle <= targetAngle) {
                int32 cnt = (this->amplitude.y >> 16) - 1;
                int32 amp = 0x400;
                for (int32 i = 0; i < cnt; ++i) {
                    this->animator.frameID = frameID + 1;
                    drawPos.x              = this->position.x + Math::Cos256(angle) * amp;
                    drawPos.y              = this->position.y + Math::Sin256(angle) * amp;
                    this->animator.DrawSprite(&drawPos, false);
                    angle += 0x400;
                }

                this->animator.frameID = frameID + 2;
                this->animator.DrawSprite(&this->position, false);

                this->animator.frameID = frameID;
                drawPos.x              = this->position.x + Math::Cos256(angle) * amp;
                drawPos.y              = this->position.y + Math::Sin256(angle) * amp;
                this->animator.DrawSprite(&drawPos, false);

                angle += amplitude;
            }
            break;
        }

        case Platform::Push:
            if (!this->speed)
                this->speed = 12;

            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            if (sVars->aniFrames.Matches(nullptr)) {
                Graphics::DrawRect(this->drawPos.x - 0x200000, this->drawPos.y - 0x100000, 0x400000, 0x200000, 0x8080A0, 0xFF, INK_NONE, false);
            }
            else {
                this->animator.frameID = frameID;
                this->animator.DrawSprite(nullptr, false);
            }
            break;

        case Platform::Track:
        case Platform::TrackReact:
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->animator.frameID = frameID;
            this->animator.DrawSprite(nullptr, false);

            drawPos.x = this->position.x + (this->amplitude.x >> 1);
            drawPos.y = this->position.y + (this->amplitude.y >> 1);
            this->animator.DrawSprite(&drawPos, false);

            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0xFF, INK_NONE, false);

            drawPos.x = this->position.x - (this->amplitude.x >> 1);
            drawPos.y = this->position.y - (this->amplitude.y >> 1);
            this->animator.DrawSprite(&drawPos, false);

            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0x00, INK_NONE, false);
            break;

        case Platform::React:
        case Platform::HoverReact: {
            if (this->speed) {
                if (this->speed < 0)
                    this->direction = FLIP_X;
            }
            else {
                this->speed = 1;
            }

            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->animator.frameID = frameID;
            this->animator.DrawSprite(nullptr, false);

            this->centerPos.x = this->position.x;
            this->centerPos.y = this->position.y;
            drawPos.x         = this->position.x;
            drawPos.y         = this->position.y;

            while (this->groundVel < (this->speed << 10)) {
                this->groundVel += this->speed << 10;
                this->amplitude.y += this->groundVel;
            }

            this->groundVel = this->speed << 16;

            drawPos.x += (this->amplitude.y >> 8) * Math::Cos256(this->angle) + (this->amplitude.x >> 8) * Math::Cos256(this->angle);
            drawPos.y += (this->amplitude.y >> 8) * Math::Sin256(this->angle) + (this->amplitude.x >> 8) * Math::Sin256(this->angle);

            this->amplitude.y = 0;
            this->groundVel -= this->speed << 11;

            while (this->groundVel > 0) {
                this->groundVel += this->speed << 10;
                this->amplitude.y += this->groundVel;
            }

            drawPos.x += (this->amplitude.y >> 8) * Math::Cos256(this->angle);
            drawPos.y += (this->amplitude.y >> 8) * Math::Sin256(this->angle);
            this->groundVel   = 0;
            this->amplitude.y = 0;
            this->animator.DrawSprite(&drawPos, false);

            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0x00, INK_NONE, false);
            break;
        }

        case Platform::DoorSlide:
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->animator.frameID = frameID;
            this->animator.DrawSprite(nullptr, false);

            if (this->speed < 0) {
                drawPos.x = this->position.x + -(this->amplitude.x >> 8) * Math::Cos256(this->angle);
                drawPos.y = this->position.y + -(this->amplitude.x >> 8) * Math::Sin256(this->angle);
            }
            else {
                drawPos.x = this->position.x + (this->amplitude.x >> 8) * Math::Cos256(this->angle);
                drawPos.y = this->position.y + (this->amplitude.x >> 8) * Math::Sin256(this->angle);
            }
            this->animator.DrawSprite(&drawPos, false);

            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0x00, INK_NONE, false);

            drawPos.x = this->position.x + (Math::Cos256(this->angle + 64) << 14);
            drawPos.y = this->position.y + (Math::Sin256(this->angle + 64) << 14);
            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0x00, INK_NONE, false);
            break;

        case Platform::SwingReact: {
            int32 amplitude = abs(this->amplitude.x >> 16);
            if (!amplitude)
                amplitude = 1;

            this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);

            int32 angle       = 0;
            int32 targetAngle = 0;
            if (this->amplitude.x < 0) {
                angle       = this->angle - amplitude + 64;
                targetAngle = this->angle + 64;
            }
            else {
                angle       = this->angle + 64;
                targetAngle = angle + amplitude;
            }

            while (angle <= targetAngle) {
                int32 cnt = (this->amplitude.y >> 16) - 1;
                int32 amp = 0x400;
                for (int32 i = 0; i < cnt; ++i) {
                    this->animator.frameID = frameID + 1;
                    drawPos.x              = this->position.x + Math::Cos256(angle) * amp;
                    drawPos.y              = this->position.y + Math::Sin256(angle) * amp;
                    this->animator.DrawSprite(&drawPos, false);
                    amp += 0x400;
                }

                this->animator.frameID = frameID + 2;
                this->animator.DrawSprite(&this->position, false);

                this->animator.frameID = frameID;
                drawPos.x              = this->position.x + Math::Cos256(angle) * amp;
                drawPos.y              = this->position.y + Math::Sin256(angle) * amp;
                if (this->angle < 0)
                    drawPos.x += 0x200000;
                else
                    drawPos.x += -0x200000;

                this->animator.DrawSprite(&drawPos, false);

                angle += amplitude;
            }
            break;
        }

        default:
        case Fixed:
        case Fall:
        case Path:
        case PathReact:
        case Child:
        case Null:
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            if (sVars->aniFrames.Matches(nullptr)) {
                Graphics::DrawRect(this->drawPos.x - 0x200000, this->drawPos.y - 0x100000, 0x400000, 0x200000, 0x8080A0u, 0xFF, INK_NONE, false);
            }
            else {
                this->animator.frameID = frameID;
                this->animator.DrawSprite(nullptr, false);
            }
            break;
    }

    this->frameID = frameID;
}

void Platform::EditorLoad()
{
    RSDK_DYNAMIC_PATH_ACTID("Platform");
    sVars->aniFrames.Load(dynamicPath, SCOPE_STAGE);

    RSDK::SpriteAnimation ticMarkFrames;
    ticMarkFrames.Load("Global/TicMark.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Fixed");
    RSDK_ENUM_VAR("Fall");
    RSDK_ENUM_VAR("Linear");
    RSDK_ENUM_VAR("Circular");
    RSDK_ENUM_VAR("Swing");
    RSDK_ENUM_VAR("Path");
    RSDK_ENUM_VAR("Push");
    RSDK_ENUM_VAR("Track");
    RSDK_ENUM_VAR("React");
    RSDK_ENUM_VAR("Hover-React");
    RSDK_ENUM_VAR("Door Slide");
    RSDK_ENUM_VAR("Path-React");
    RSDK_ENUM_VAR("Swing-React");
    RSDK_ENUM_VAR("Track-React");
    RSDK_ENUM_VAR("Clacker");
    RSDK_ENUM_VAR("Child");

    RSDK_ACTIVE_VAR(sVars, collision);
    RSDK_ENUM_VAR("Platform");
    RSDK_ENUM_VAR("Solid");
    RSDK_ENUM_VAR("Tiled");
    RSDK_ENUM_VAR("Hurt");
    RSDK_ENUM_VAR("None");
    RSDK_ENUM_VAR("Solid Hurt Sides");
    RSDK_ENUM_VAR("Solid Hurt Bottom");
    RSDK_ENUM_VAR("Solid Hurt Top");
    RSDK_ENUM_VAR("Solid Hold");
    RSDK_ENUM_VAR("Solid Sticky");
    RSDK_ENUM_VAR("Sticky Top");
    RSDK_ENUM_VAR("Sticky Left");
    RSDK_ENUM_VAR("Sticky Right");
    RSDK_ENUM_VAR("Sticky Bottom");
    RSDK_ENUM_VAR("Solid Barrel");
    RSDK_ENUM_VAR("Solid No Crush");
    RSDK_ENUM_VAR("Solid Hurt All");
    RSDK_ENUM_VAR("Solid Hurt No Crush");
}
#endif

#if RETRO_REV0U
void Platform::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Platform);

    sVars->aniFrames.Init();

    sVars->sfxClacker.Init();
    sVars->sfxClang.Init();
    sVars->sfxPush.Init();
    sVars->sfxClack.Init();
}
#endif

void Platform::Serialize()
{
    RSDK_EDITABLE_VAR(Platform, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(Platform, VAR_VECTOR2, amplitude);
    RSDK_EDITABLE_VAR(Platform, VAR_ENUM, speed);
    RSDK_EDITABLE_VAR(Platform, VAR_BOOL, hasTension);
    RSDK_EDITABLE_VAR(Platform, VAR_INT8, frameID);
    RSDK_EDITABLE_VAR(Platform, VAR_UINT8, collision);
    RSDK_EDITABLE_VAR(Platform, VAR_VECTOR2, tileOrigin);
    RSDK_EDITABLE_VAR(Platform, VAR_ENUM, childCount);
    RSDK_EDITABLE_VAR(Platform, VAR_INT32, angle);
}

} // namespace GameLogic
