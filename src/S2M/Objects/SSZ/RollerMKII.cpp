// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: RollerMKII Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "RollerMKII.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Global/PlaneSwitch.hpp"
#include "Global/Dust.hpp"
#include "Global/Spikes.hpp"
#include "Common/Platform.hpp"
#include "Common/BreakableWall.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(RollerMKII);

void RollerMKII::Update() { this->state.Run(this); }

void RollerMKII::LateUpdate() {}

void RollerMKII::StaticUpdate() {}

void RollerMKII::Draw() { this->animator.DrawSprite(nullptr, false); }

void RollerMKII::Create(void *data)
{
    this->visible         = true;
    this->drawGroup       = Zone::sVars->objectDrawGroup[0];
    this->startPos        = this->position;
    this->startDir        = this->direction;
    this->drawFX          = FX_FLIP;
    this->active          = ACTIVE_BOUNDS;
    this->updateRange.x   = 0x1000000;
    this->updateRange.y   = 0x1000000;
    this->onGround        = false;
    this->tileCollisions  = TILECOLLISION_DOWN;
    this->collisionLayers = Zone::sVars->collisionLayers;
    this->collisionPlane  = 0;
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 5);
    this->state.Set(&RollerMKII::State_Init);
}

void RollerMKII::StageLoad()
{
    sVars->aniFrames.Load("SSZ/RollerMKII.bin", SCOPE_STAGE);

    sVars->hitboxOuter_Rolling.left   = -9;
    sVars->hitboxOuter_Rolling.top    = -14;
    sVars->hitboxOuter_Rolling.right  = 9;
    sVars->hitboxOuter_Rolling.bottom = 14;

    sVars->hitboxInner_Rolling.left   = -8;
    sVars->hitboxInner_Rolling.top    = -14;
    sVars->hitboxInner_Rolling.right  = 8;
    sVars->hitboxInner_Rolling.bottom = 14;

    sVars->hitboxOuter_Idle.left   = -9;
    sVars->hitboxOuter_Idle.top    = -14;
    sVars->hitboxOuter_Idle.right  = 9;
    sVars->hitboxOuter_Idle.bottom = 24;

    sVars->hitboxInner_Idle.left   = -8;
    sVars->hitboxInner_Idle.top    = -14;
    sVars->hitboxInner_Idle.right  = 8;
    sVars->hitboxInner_Idle.bottom = 24;

    // Hitbox for interacting with player
    sVars->hitboxBadnik.left   = -14;
    sVars->hitboxBadnik.top    = -14;
    sVars->hitboxBadnik.right  = 14;
    sVars->hitboxBadnik.bottom = 14;

    // Hitbox for interacting with other objects
    // It's.... the same... as above???
    sVars->hitboxObject.left   = -14;
    sVars->hitboxObject.top    = -14;
    sVars->hitboxObject.right  = 14;
    sVars->hitboxObject.bottom = 14;

    sVars->sfxBumper.Get("Stage/Bumper3.wav");
    sVars->sfxJump.Get("Stage/Jump2.wav");
    sVars->sfxDropDash.Get("Global/DropDash.wav");
    sVars->sfxRelease.Get("Global/Release.wav");
    sVars->sfxSkidding.Get("Global/Skidding.wav");

    DebugMode::AddObject(sVars->classID, &RollerMKII::DebugSpawn, &RollerMKII::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void RollerMKII::DebugSpawn()
{
    RollerMKII *rollerMKII       = GameObject::Create<RollerMKII>(nullptr, this->position.x, this->position.y);
    rollerMKII->direction        = this->direction;
    rollerMKII->startDir         = this->direction;
}

void RollerMKII::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 1);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void RollerMKII::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position  = this->startPos;
        this->direction = this->startDir;
        RollerMKII::Create(nullptr);
    }
}

void RollerMKII::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik))
            player->CheckBadnikBreak(this, true);
    }
}

void RollerMKII::CheckPlayerCollisions_Rolling()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik)) {

            int32 anim        = player->animator.animationID;
            bool32 shouldBump = false;

            if (player->CheckAttacking(this) || shouldBump) {
                sVars->sfxBumper.Play(false, 255);
                int32 angle = Math::ATan2(player->position.x - this->position.x, player->position.y - this->position.y);
                int32 velX  = 0x380 * Math::Cos256(angle);
                int32 velY  = 0x380 * Math::Sin256(angle);
            
                if (anim != Player::ANI_FLY && anim != Player::ANI_FLY_LIFT_TIRED) {
                    if (!player->state.Matches(&Player::State_TailsFlight)) {
                        if (!player->state.Matches(&Player::State_DropDash))
                            player->state.Set(&Player::State_Air);
                        if (anim != Player::ANI_JUMP && anim != Player::ANI_DASH)
                            player->animator.animationID = Player::ANI_WALK;
                    }
                }
               player->velocity.x   = velX;
               player->groundVel    = velX;
               player->applyJumpCap = false;
               if (player->characterID == ID_KNUCKLES && player->animator.animationID == Player::ANI_GLIDE) {
                    player->animator.SetAnimation(player->aniFrames, Player::ANI_GLIDE_DROP, false, 0);
                   player->state.Set(&Player::State_KnuxGlideDrop);
               }
               player->velocity.y     = velY;
               player->onGround       = false;
               player->tileCollisions = TILECOLLISION_DOWN;
               sVars->sfxBumper.Play(false, 255);
               this->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
               this->velocity.y    = -0x40000;
               this->velocity.x    = -velX;
               this->onGround      = false;
               this->touchedGround = 0;
               this->timer         = 45;
               this->state.Set(&RollerMKII::State_Bumped);
               this->direction     = player->position.x < this->position.x;
            }
            else {
               player->Hurt(this);
            }
        }
    }
}

int32 RollerMKII::HandleObjectCollisions(Entity *otherEntity, Hitbox *hitbox)
{
    int32 velX = this->velocity.x;
    // review
    int32 side = this->CheckCollisionBox(hitbox, otherEntity, &sVars->hitboxObject, true);

    if ((side == C_LEFT && velX > 0) || (side == C_RIGHT && velX < 0)) {
        if (!this->state.Matches(&RollerMKII::State_Bumped)) {
            sVars->sfxBumper.Play(false, 255);
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->velocity.y    = -0x40000;
            this->onGround      = false;
            this->touchedGround = 0;
            this->timer         = 30;
            this->velocity.x    = velX < 0 ? 0x20000 : -0x20000;
            this->state.Set(&RollerMKII::State_Bumped);
            this->direction     = this->position.x < Player::GetNearestPlayerXY()->position.x;
        }
    }
    return side;
}

bool32 RollerMKII::HandlePlatformCollisions(Platform *platform)
{
    bool32 collided = false;

    if (!platform->state.Matches(&Platform::State_Falling2) && !platform->state.Matches(&Platform::State_Hold)) {
        platform->position.x = platform->drawPos.x - platform->collisionOffset.x;
        platform->position.y = platform->drawPos.y - platform->collisionOffset.y;
        if (platform->collision) {
            if (platform->collision != Platform::C_Solid) {
                if (platform->collision == Platform::C_Tiled
                    && platform->CheckCollisionTouchBox(&platform->hitbox, this, &sVars->hitboxObject)) {
                    if (this->collisionLayers & Zone::sVars->moveLayerMask) {
                        TileLayer *move  = Zone::sVars->moveLayer.GetTileLayer();
                        move->position.x = -(platform->drawPos.x + platform->tileOrigin.x) >> 16;
                        move->position.y = -(platform->drawPos.y + platform->tileOrigin.y) >> 16;
                    }

                    if (this->velocity.y >= 0x3800)
                        collided = true;
                }
            }
            else {
                Hitbox *hitbox = platform->animator.GetHitbox(1);
                collided       = RollerMKII::HandleObjectCollisions((Entity *)platform, hitbox);
            }
        }
        else {
            Hitbox *hitbox = platform->animator.GetHitbox(0);
            RSDKTable->CheckObjectCollisionPlatform(platform, hitbox, this, &sVars->hitboxObject, true);
        }

        platform->position = platform->centerPos;
    }

    return collided;
}

void RollerMKII::HandleCollisions()
{
    for (auto planeSwitch : GameObject::GetEntities<PlaneSwitch>(FOR_ALL_ENTITIES))
    {
        planeSwitch->CheckCollisions(this, planeSwitch->flags, planeSwitch->size, true, Zone::sVars->playerDrawGroup[0],
                                    Zone::sVars->playerDrawGroup[1]);
    }

    for (auto platform : GameObject::GetEntities<Platform>(FOR_ALL_ENTITIES)) { RollerMKII::HandlePlatformCollisions(platform); }
    for (auto spikes : GameObject::GetEntities<Spikes>(FOR_ALL_ENTITIES)) { RollerMKII::HandleObjectCollisions((Entity *)spikes, &spikes->hitbox); }
    for (auto wall : GameObject::GetEntities<BreakableWall>(FOR_ALL_ENTITIES)) { RollerMKII::HandleObjectCollisions((Entity *)wall, &wall->hitbox); }

    if (!this->collisionMode && !this->state.Matches(&RollerMKII::State_Bumped)) {
        bool32 collided = this->direction == FLIP_X
                              ? this->TileCollision(Zone::sVars->collisionLayers, CMODE_RWALL, this->collisionPlane, -0xA0000, 0, false)
                              : this->TileCollision(Zone::sVars->collisionLayers, CMODE_LWALL, this->collisionPlane, 0xA0000, 0, false);
        if (collided) {
            sVars->sfxBumper.Play(false, 255);
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->onGround      = false;
            this->velocity.x    = this->direction == FLIP_X ? 0x20000 : -0x20000;
            this->velocity.y    = -0x40000;
            this->touchedGround = 0;
            this->timer         = 30;
            this->state.Set(&RollerMKII::State_Bumped);
            this->direction     = this->position.x < Player::GetNearestPlayerXY()->position.x;
        }
    }
}

void RollerMKII::State_Init()
{
    this->active        = ACTIVE_NORMAL;
    this->velocity.x    = 0;
    this->velocity.y    = 0;
    this->groundVel     = 0;
    this->touchedGround = 0;

    this->state.Set(&RollerMKII::State_Idle);
    RollerMKII::State_Idle();
}

void RollerMKII::State_Idle()
{
    this->animator.Process();

    Vector2 range = { 0x200000, 0x200000 };

    if (this->timer) {
        this->timer--;
    }
    else if (this->CheckOnScreen(&range)) {
        bool32 wallCollided = false;
        if (this->direction == FLIP_X)
            wallCollided = this->TileCollision(Zone::sVars->collisionLayers, CMODE_RWALL, this->collisionPlane, -0x180000, 0, false);
        else
            wallCollided = this->TileCollision(Zone::sVars->collisionLayers, CMODE_LWALL, this->collisionPlane, 0x180000, 0, false);

        if (wallCollided)
            this->direction ^= FLIP_X;

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (this->direction) {
                if (player->position.x < this->position.x) {
                    if (this->position.x - player->position.x < 0x800000) {
                        this->velocity.y = -0x40000;
                        this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);

                        this->playerPtr = player;
                        sVars->sfxJump.Play(false, 0xFF);

                        this->state.Set(&RollerMKII::State_SpinUp);
                    }
                }
            }
            else {
                if (player->position.x > this->position.x) {
                    if (player->position.x - this->position.x < 0x800000) {
                        this->velocity.y = -0x40000;
                        this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);

                        this->playerPtr = player;
                        sVars->sfxJump.Play(false, 0xFF);

                        this->state.Set(&RollerMKII::State_SpinUp);
                    }
                }
            }
        }
    }

    RollerMKII::CheckPlayerCollisions();
    RollerMKII::CheckOffScreen();
}

void RollerMKII::State_SpinUp()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x3800;

    this->animator.Process();

    if (this->velocity.y > 0) {
        this->timer = 15;
        sVars->sfxDropDash.Play(false, 255);
        this->state.Set(&RollerMKII::State_RollDelay);
    }

    if (this->animator.frameID <= 4)
        RollerMKII::CheckPlayerCollisions();
    else
        RollerMKII::CheckPlayerCollisions_Rolling();

    RollerMKII::CheckOffScreen();
}

void RollerMKII::State_RollDelay()
{
    this->animator.Process();

    if (!--this->timer)
        this->state.Set(&RollerMKII::State_Rolling_Air);

    RollerMKII::CheckPlayerCollisions_Rolling();
    RollerMKII::CheckOffScreen();
}

void RollerMKII::State_Rolling_Air()
{
    if (!this->touchedGround) {
        if (this->position.x <= this->playerPtr->position.x) {
            this->direction = FLIP_NONE;

            if (this->velocity.x < 0x80000)
                this->velocity.x += 0x3800;
        }
        else {
            this->direction = FLIP_X;

            if (this->velocity.x > -0x80000)
                this->velocity.x -= 0x3800;
        }
    }

    this->ProcessMovement(&sVars->hitboxOuter_Rolling, &sVars->hitboxInner_Rolling);

    this->velocity.y += 0x3800;

    RollerMKII::HandleCollisions();

    if (this->onGround) {
        if (abs(this->angle) & 0x60) {
            this->state.Set(&RollerMKII::State_Rolling_Ground);
        }
        else {
            if (this->velocity.x)
                this->groundVel = this->velocity.x;

            if (!this->touchedGround) {
                this->touchedGround = true;

                if (this->direction == FLIP_NONE) {
                    this->groundVel  = 0x80000;
                    this->velocity.x = 0x80000;
                }
                else {
                    this->groundVel  = -0x80000;
                    this->velocity.x = -0x80000;
                }
                this->velocity.y = -0x20000;

                this->onGround = false;
                sVars->sfxDropDash.Stop();
                sVars->sfxRelease.Play(false, 255);
                Hitbox *hitbox = this->animator.GetHitbox(0);

                Dust *dust = GameObject::Create<Dust>(this, this->position.x, this->position.y + 0xE0000);
                dust->animator.SetAnimation(Dust::sVars->aniFrames, 2, true, 0);
                dust->state.Set(&Dust::State_DustPuff);
                dust->position.y += hitbox->bottom << 16;
                dust->direction = this->direction;
                dust->drawGroup = this->drawGroup;
                dust->position.x += this->direction ? 0x90000 : -0x90000;
            }
            else if (this->velocity.y <= 0x10000) {
                this->state.Set(&RollerMKII::State_Rolling_Ground);
            }
            else {
                this->velocity.y = -0x20000;
                this->onGround   = false;
            }
        }

        this->direction = this->groundVel < 0;
    }

    this->animator.Process();

    RollerMKII::CheckPlayerCollisions_Rolling();
    RollerMKII::CheckOffScreen();
}

void RollerMKII::State_Rolling_Ground()
{
    if (!this->collisionMode) {
        if (this->position.x <= this->playerPtr->position.x) {
            this->direction = FLIP_NONE;

            if (this->groundVel < 0x80000) {
                this->groundVel += 0x3800;

                if (this->groundVel > 0) {
                    this->groundVel += 0x1400;

                    if (this->groundVel >= -0x40000) {
                        this->timer = 0;
                    }
                    else {
                        sVars->sfxSkidding.Play(false, 255);
                        this->timer = (this->timer + 1) & 3;
                        if (!this->timer)
                            GameObject::Create<Dust>(nullptr, this->position.x, this->position.y + 0xE0000)->state.Set(&Dust::State_DustPuff);
                    }
                }
            }
        }
        else {
            this->direction = FLIP_X;

            if (this->groundVel > -0x80000) {
                this->groundVel -= 0x3800;

                if (this->groundVel > 0) {
                    this->groundVel += 0x1400;

                    if (this->groundVel <= 0x40000) {
                        this->timer = 0;
                    }
                    else {
                        sVars->sfxSkidding.Play(false, 255);
                        this->timer = (this->timer + 1) & 3;
                        if (!this->timer)
                            GameObject::Create<Dust>(nullptr, this->position.x, this->position.y + 0xE0000)->state.Set(&Dust::State_DustPuff);
                    }
                }
            }
        }
    }

    this->ProcessMovement(&sVars->hitboxOuter_Rolling, &sVars->hitboxInner_Rolling);

    this->groundVel += (Math::Sin256(this->angle) << 13 >> 8);
    if (this->collisionMode != CMODE_FLOOR) {
        if (this->angle >= 0x40 && this->angle <= 0xC0 && this->groundVel <= 0x20000) {
            this->onGround      = false;
            this->angle         = 0;
            this->collisionMode = 0;
        }
    }

    RollerMKII::HandleCollisions();

    if (!this->onGround && !this->state.Matches(&RollerMKII::State_Bumped))
        this->state.Set(&RollerMKII::State_Rolling_Air);

    this->animator.Process();

    RollerMKII::CheckPlayerCollisions_Rolling();
    RollerMKII::CheckOffScreen();
}

void RollerMKII::State_Bumped()
{
    this->ProcessMovement(&sVars->hitboxOuter_Idle, &sVars->hitboxInner_Idle);

    this->velocity.y += 0x3800;
    RollerMKII::HandleCollisions();

    if (this->onGround) {
        this->groundVel  = 0;
        this->velocity.x = 0;
        this->velocity.y = 0;
        this->state.Set(&RollerMKII::State_Idle);
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    }

    this->animator.Process();
    RollerMKII::CheckOffScreen();
}

#if RETRO_INCLUDE_EDITOR
void RollerMKII::EditorDraw() { RollerMKII::Draw(); }

void RollerMKII::EditorLoad()
{
    sVars->aniFrames.Load("SSZ/RollerMKII.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Right");
    RSDK_ENUM_VAR("Left");
}
#endif

void RollerMKII::Serialize() { RSDK_EDITABLE_VAR(RollerMKII, VAR_UINT8, direction); }

} // namespace GameLogic