// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Bridge Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "S2M.hpp"
#include "Bridge.hpp"
#include "BurningLog.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Bridge);

void Bridge::Update()
{
    if (this->stoodEntityCount) {
        if (this->timer < 0x80)
            this->timer += 8;
    }
    else {
        if (this->timer) {
            this->stoodEntity = (Entity *)-1;
            this->timer -= 8;
        }
        else {
            this->depression = 0;
        }
    }

    this->stoodEntityCount = 0;
    this->bridgeDepth      = (this->depression * this->timer) >> 7;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        Hitbox *playerHitbox = player->GetHitbox();
        if (player->state.Matches(&Player::State_KnuxLedgePullUp))
            continue;

        Bridge::HandleCollisions(player, this, playerHitbox, true, true);
    }

    if (this->burnOffset != 0xFF)
        Bridge::Burn(this->burnOffset);
}

void Bridge::LateUpdate() {}

void Bridge::StaticUpdate() {}

void Bridge::Draw()
{
    int32 id = 0;
    Vector2 drawPos;

    int32 size = this->stoodPos >> 20;
    int32 ang  = 0x80000;
    drawPos.x  = this->startPos + 0x80000;
    for (int32 i = 0; i < size; ++i) {
        drawPos.y = (this->bridgeDepth * Math::Sin512((ang << 7) / this->stoodPos) >> 9) + this->position.y;
        this->animator.DrawSprite(&drawPos, false);
        drawPos.x += 0x100000;
        ang += 0x100000;
    }
    id = size;

    drawPos.y = this->bridgeDepth + this->position.y;
    this->animator.DrawSprite(&drawPos, false);
    drawPos.x += 0x100000;
    ++id;

    ang           = 0x80000;
    int32 divisor = this->endPos - this->startPos - this->stoodPos;
    drawPos.x     = this->endPos - 0x80000;
    for (; id < this->length; ++id) {
        drawPos.y = (this->bridgeDepth * Math::Sin512((ang << 7) / divisor) >> 9) + this->position.y;
        this->animator.DrawSprite(&drawPos, false);
        drawPos.x -= 0x100000;
        ang += 0x100000;
    }
}

void Bridge::Create(void *data)
{
    this->visible = true;
    ++this->length;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->active        = ACTIVE_BOUNDS;
    int32 len           = this->length << 19;
    this->startPos      = this->position.x - len;
    this->endPos        = len + this->position.x;
    this->updateRange.x = len;
    this->updateRange.y = 0x800000;
    this->stoodEntity   = (Entity *)-1;
    this->burnOffset    = 0xFF;
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
}

void Bridge::StageLoad()
{
    if (Stage::CheckSceneFolder("EHZ")) {
        sVars->aniFrames.Load("EHZ/Bridge.bin", SCOPE_STAGE);
    }

    DebugMode::AddObject(sVars->classID, &Bridge::DebugSpawn, &Bridge::DebugDraw);
}

void Bridge::DebugDraw()
{
    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(&drawPos, false);
}

void Bridge::DebugSpawn()
{
    Bridge *bridge = GameObject::Create<Bridge>(nullptr, this->position.x, this->position.y);
}

void Bridge::Burn(int32 offset)
{
    int32 size   = this->stoodPos >> 20;
    int32 spawnX = this->startPos + 0x80000;
    int32 off    = -offset;
    int32 ang    = 0x80000;
    for (int32 i = 0; i < size; ++i) {
        int32 sine = Math::Sin512((ang << 7) / this->stoodPos);
        GameObject::Create<BurningLog>(INT_TO_VOID(8 * abs(off++) + 16), spawnX, (this->bridgeDepth * sine >> 9) + this->position.y);
        ang += 0x100000;
        spawnX += 0x100000;
    }

    int32 id = size;
    GameObject::Create<BurningLog>(INT_TO_VOID(8 * abs(id++ - offset) + 16), spawnX, this->bridgeDepth + this->position.y);

    spawnX        = this->endPos - 0x80000;
    int32 divisor = this->endPos - this->startPos - this->stoodPos;
    ang           = 0x80000;
    if (id < this->length) {
        off = offset - id;
        for (; id < this->length; ++id, --off) {
            int32 spawnY = (this->bridgeDepth * Math::Sin512((ang << 7) / divisor) >> 9) + this->position.y;
            GameObject::Create<BurningLog>(INT_TO_VOID(8 * abs(this->length - abs(off) - offset) + 16), spawnX, spawnY);
            ang += 0x100000;
            spawnX -= 0x100000;
        }
    }
    this->Destroy();
}

bool32 Bridge::HandleCollisions(void *e, Bridge *self, Hitbox *entityHitbox, bool32 updateVars, bool32 isPlayer)
{
    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

    // use EntityPlayer as the type so we can access player variables if needed
    // if `isPlayer` is false, then only base entity variables will be accessed
    Player *entity = (Player *)e;

    bool32 bridgeCollided = false;

    if (entity->position.x > self->startPos && entity->position.x < self->endPos) {
        if (entity != self->stoodEntity) {
            if (updateVars && !self->stoodEntityCount)
                self->stoodPos = entity->position.x - self->startPos;

            if (entity->velocity.y >= 0) {
                Hitbox hitboxBridge;
                hitboxBridge.left  = -0x400;
                hitboxBridge.right = 0x400;

                int32 divisor = 0;
                int32 ang     = 0;
                if (entity->position.x - self->startPos <= self->stoodPos) {
                    divisor = self->stoodPos;
                    ang     = (entity->position.x - self->startPos) << 7;
                }
                else {
                    divisor = self->endPos - self->startPos - self->stoodPos;
                    ang     = (self->endPos - entity->position.x) << 7;
                }

                int32 hitY = (self->bridgeDepth * Math::Sin512(ang / divisor) >> 9) - 0x80000;
                if (entity->velocity.y >= 0x8000) {
                    hitboxBridge.top    = (hitY >> 16);
                    hitboxBridge.bottom = hitboxBridge.top + 8;
                }
                else {
                    hitboxBridge.bottom = (hitY >> 16);
                    hitboxBridge.top    = hitboxBridge.bottom - 8;
                }

                bool32 collided = false;
                if (isPlayer)
                    collided = entity->CheckCollisionTouch(self, &hitboxBridge);
                else
                    collided = self->CheckCollisionTouchBox(&hitboxBridge, entity, entityHitbox);

                if (collided) {
                    entity->position.y = hitY + self->position.y - (entityHitbox->bottom << 16);

                    if (updateVars) {
                        ++self->stoodEntityCount;
                        if (!entity->onGround) {
                            entity->onGround  = true;
                            entity->groundVel = entity->velocity.x;
                        }

                        if (isPlayer)
                            entity->flailing = false;

                        if (entity == player1) {
                            if (self->stoodEntity != (void *)-1 && self->stoodEntity != (void *)-2) {
                                int32 distance    = self->endPos - self->startPos;
                                self->stoodPos    = entity->position.x - self->startPos;
                                self->depression  = (distance >> 13) * Math::Sin512((self->stoodPos >> 8) / (distance >> 16));
                                self->bridgeDepth = (self->depression * self->timer) >> 7;
                            }

                            self->stoodEntity = entity;
                            if (entity->velocity.y < 0x10000)
                                self->timer = 0x80;
                        }
                        else {
                            if (self->stoodEntity == (void *)-1) {
                                self->stoodEntity = entity;
                                if (entity->velocity.y < 0x10000)
                                    self->timer = 0x80;
                            }

                            if (self->stoodEntity == (void *)-2)
                                self->stoodEntity = entity;
                        }

                        if (!entity->onGround) {
                            entity->onGround  = true;
                            entity->groundVel = entity->velocity.x;
                        }

                        entity->velocity.y = 0;

                        if (isPlayer) {
                            if (entity->shield == Player::Shield_Fire && self->burnable)
                                Bridge::Burn((entity->position.x - self->startPos) >> 20);
                        }
                    }

                    bridgeCollided = true;
                }
            }
        }
        else if (updateVars) {
            self->stoodPos   = entity->position.x - self->startPos;
            int32 distance   = (self->endPos - self->startPos);
            self->depression = Math::Sin512((self->stoodPos >> 8) / (distance >> 16)) * (distance >> 13);

            if (entity->position.y > self->position.y - 0x300000) {
                if (entity->velocity.y >= 0) {
                    ++self->stoodEntityCount;
                    entity->position.y = self->position.y + self->bridgeDepth - ((entityHitbox->bottom + 8) << 16);

                    if (!entity->onGround) {
                        entity->onGround  = true;
                        entity->groundVel = entity->velocity.x;
                    }

                    entity->velocity.y = 0;
                    if (isPlayer) {
                        entity->flailing = false;
                        if (entity->shield == Player::Shield_Fire && self->burnable)
                            Bridge::Burn((entity->position.x - self->startPos) >> 20);
                    }

                    bridgeCollided = true;
                }
                else {
                    self->stoodEntity = (void *)-2;
                }
            }
        }
    }
    else if (entity == self->stoodEntity) {
        self->timer       = 32;
        self->stoodEntity = (void *)-2;
    }

    return bridgeCollided;
}

#if RETRO_INCLUDE_EDITOR
void Bridge::EditorDraw()
{
    int32 length = this->length++;

    int32 len           = this->length << 19;
    this->startPos      = this->position.x - len;
    this->endPos        = len + this->position.x;
    this->updateRange.x = len;
    this->updateRange.y = 0x800000;

    Bridge::Draw();

    this->length = length;
}

void Bridge::EditorLoad()
{
    if (Stage::CheckSceneFolder("EHZ")) {
        sVars->aniFrames.Load("EHZ/Bridge.bin", SCOPE_STAGE);
    }
}
#endif

void Bridge::Serialize()
{
    RSDK_EDITABLE_VAR(Bridge, VAR_UINT8, length);
    RSDK_EDITABLE_VAR(Bridge, VAR_BOOL, burnable);
}


} // namespace GameLogic