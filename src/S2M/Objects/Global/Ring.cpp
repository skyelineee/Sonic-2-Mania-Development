// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Ring Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Ring.hpp"
#include "Zone.hpp"
#include "DebugMode.hpp"
#include "Shield.hpp"
#include "Spikes.hpp"
// #include "Common/Bridge.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Ring);

void Ring::Update() { state.Run(this); }
void Ring::LateUpdate() {}
void Ring::StaticUpdate() {}
void Ring::Draw() { stateDraw.Run(this); }

void Ring::Create(void *data)
{
    this->visible = true;

    this->drawGroup = Zone::sVars->objectDrawGroup[0] + 1;
    if (this->planeFilter > 0 && ((uint8)this->planeFilter - 1) & 2)
        this->drawGroup = Zone::sVars->objectDrawGroup[1] + 1;

    if (this->type == Ring::Combi) {
        this->drawFX |= FX_FLIP;
        this->ringAmount = 5;
    }

    if (!sceneInfo->inEditor) {
        if (!data) {
            this->animator.SetAnimation(sVars->aniFrames, type, true, 0);

            this->amplitude.x >>= 10;
            this->amplitude.y >>= 10;
            this->active = ACTIVE_BOUNDS;

            switch (this->moveType) {
                case Ring::Linear:
                    this->updateRange.x = (abs(this->amplitude.x) + 0x1000) << 10;
                    this->updateRange.y = (abs(this->amplitude.y) + 0x1000) << 10;
                    this->state.Set(&Ring::State_Linear);
                    this->stateDraw.Set(&Ring::Draw_Oscillating);
                    break;

                case Ring::Circle:
                    this->updateRange.x = (abs(this->amplitude.x) + 0x1000) << 10;
                    this->updateRange.y = (abs(this->amplitude.y) + 0x1000) << 10;
                    this->state.Set(&Ring::State_Circular);
                    this->stateDraw.Set(&Ring::Draw_Oscillating);
                    break;

                case Ring::Track:
                    this->updateRange.x = (abs(this->amplitude.x) + 0x2000) << 9;
                    this->updateRange.y = (abs(this->amplitude.y) + 0x2000) << 9;
                    this->state.Set(&Ring::State_Track);
                    this->stateDraw.Set(&Ring::Draw_Oscillating);
                    break;

                case Ring::Path:
                    this->updateRange.x = 0x400000;
                    this->updateRange.y = 0x400000;
                    this->active        = ACTIVE_NEVER;
                    this->state.Set(&Ring::State_Path);
                    this->stateDraw.Set(&Ring::Draw_Oscillating);
                    break;

                default:
                case Ring::Fixed:
                    this->updateRange.x = 0x400000;
                    this->updateRange.y = 0x400000;
                    this->state.Set(&Ring::State_Normal);
                    this->stateDraw.Set(&Ring::Draw_Normal);
                    break;
            }
        }
        else {
            this->active   = ACTIVE_NORMAL;
            this->startPos = this->position;
            this->animator.SetAnimation(sVars->aniFrames, Ring::Normal, true, 0);
        }
    }
}

void Ring::StageLoad()
{
    sVars->aniFrames.Load("Global/Ring.bin", SCOPE_STAGE);

    sVars->hitbox.left   = -8;
    sVars->hitbox.top    = -8;
    sVars->hitbox.right  = 8;
    sVars->hitbox.bottom = 8;

    if (globals->secrets & SECRET_NOITEMS) {
        for (auto ring : GameObject::GetEntities<Ring>(FOR_ALL_ENTITIES)) ring->Destroy();
    }

    DebugMode::AddObject(sVars->classID, &Ring::DebugSpawn, &Ring::DebugDraw);

    sVars->sfxRing.Get("Global/Ring.wav");
}

void Ring::DebugSpawn() { GameObject::Create<Ring>(nullptr, this->position.x, this->position.y); }
void Ring::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Ring::Collect()
{
    int32 storeX = this->position.x;
    int32 storeY = this->position.y;

    if (this->drawPos.x)
        this->position = this->drawPos;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->CheckCollisionTouch(this, &sVars->hitbox)) {
            if (!this->planeFilter || player->collisionPlane == (((uint8)this->planeFilter - 1) & 1)) {
                if (player->sidekick)
                    player = GameObject::Get<Player>(SLOT_PLAYER1);

                int32 ringAmount = 1;
                if (this->type == Ring::Combi) {
                    player->ringExtraLife += 100 * (this->ringAmount / 100);
                    ringAmount = this->ringAmount;
                }
                player->GiveRings(ringAmount, true);

                int32 max = 0x100000;
                if (this->type != Ring::Combi)
                    max = 0x80000;

                if (globals->useManiaBehavior) {
                    int32 cnt = 4 * (this->type == Ring::Combi) + 4;
                    int32 min = -max;
                    for (int32 i = 0; i < cnt; ++i) {
                        int32 x       = this->position.x + Math::Rand(min, max);
                        int32 y       = this->position.y + Math::Rand(min, max);
                        Ring *sparkle = GameObject::Create<Ring>(nullptr, x, y);

                        sparkle->state.Set(&Ring::State_Sparkle);
                        sparkle->stateDraw.Set(&Ring::Draw_Sparkle);
                        sparkle->active  = ACTIVE_NORMAL;
                        sparkle->visible = false;
                        if (this->drawGroup == 1)
                            sparkle->drawGroup = 1;
                        else
                            sparkle->drawGroup = Zone::sVars->objectDrawGroup[1];

                        sparkle->animator.SetAnimation(sVars->aniFrames, Ring::Sparkle1 + (i % 3), true, 0);
                        int32 frameCount = sparkle->animator.frameCount;
                        if (sparkle->animator.animationID == 2) {
                            sparkle->alpha = 0xE0;
                            frameCount >>= 1;
                        }
                        sparkle->maxFrameCount  = frameCount - 1;
                        sparkle->animator.speed = Math::Rand(6, 8);
                        sparkle->timer          = 2 * i++;
                    }
                }
                else {
                    Ring *sparkle = GameObject::Create<Ring>(nullptr, this->position.x, this->position.y);
                    sparkle->state.Set(&Ring::State_Sparkle);
                    sparkle->stateDraw.Set(&Ring::Draw_Sparkle);
                    sparkle->active  = ACTIVE_NORMAL;
                    sparkle->visible = false;
                    if (this->drawGroup == 1)
                        sparkle->drawGroup = 1;
                    else
                        sparkle->drawGroup = Zone::sVars->objectDrawGroup[1];

                    sparkle->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
                    sparkle->timer         = 0;
                    sparkle->maxFrameCount = sparkle->animator.frameCount - 1;
                }

                this->Destroy();
                this->active = ACTIVE_DISABLED; // not sure what the purpose of this is but sure

                return;
            }
        }
        else if (!this->state.Matches(&Ring::State_Attracted) && player->shield == Shield::Lightning
                 && this->CheckCollisionTouchCircle(0x500000, player, 0x10000)) {
            this->drawPos.x = 0;
            this->state.Set(&Ring::State_Attracted);
            this->stateDraw.Set(&Ring::Draw_Normal);
            this->active       = ACTIVE_NORMAL;
            this->storedPlayer = player;
            return;
        }
    }

    this->position.x = storeX;
    this->position.y = storeY;
}

void Ring::LoseRings(RSDK::Vector2 *position, int32 rings, uint8 cPlane, uint8 drawGroup)
{
    int32 outerRingCount = CLAMP(rings, 0, 16);
    int32 innerRingCount = CLAMP(rings - 16, 0, 16);
    int32 bigRingCount   = CLAMP(rings - 32, 0, 16);

    int32 angleStart = 0xC0 - 8 * (innerRingCount & -2);
    int32 angle      = angleStart + 8;
    if (!(innerRingCount & 1))
        angle = angleStart - 8;

    for (int32 i = 0; i < innerRingCount; ++i) {
        Ring *ring           = GameObject::Create<Ring>(position, position->x, position->y);
        ring->velocity.x     = Math::Cos256(angle) << 9;
        ring->velocity.y     = Math::Sin256(angle) << 9;
        ring->animator.speed = 0x200;
        ring->collisionPlane = cPlane;
        ring->inkEffect      = INK_ALPHA;
        ring->alpha          = 0x100;
        ring->isPermanent    = true;
        ring->state.Set(&Ring::State_Lost);
        ring->stateDraw.Set(&Ring::Draw_Normal);
        ring->drawGroup = drawGroup;
        ring->moveType  = Ring::Fixed;
        ring->velocity.x += Zone::sVars->autoScrollSpeed / 2;
        angle += 0x10;
    }

    angleStart = 0xC0 - 8 * (outerRingCount & -2);
    angle      = angleStart + 8;
    if (!(outerRingCount & 1))
        angle = angleStart - 8;

    for (int32 i = 0; i < outerRingCount; ++i) {
        Ring *ring           = GameObject::Create<Ring>(position, position->x, position->y);
        ring->velocity.x     = Math::Cos256(angle) << 10;
        ring->velocity.y     = Math::Sin256(angle) << 10;
        ring->animator.speed = 512;
        ring->collisionPlane = cPlane;
        ring->inkEffect      = INK_ALPHA;
        ring->alpha          = 0x100;
        ring->isPermanent    = true;
        ring->state.Set(&Ring::State_Lost);
        ring->stateDraw.Set(&Ring::Draw_Normal);
        ring->drawGroup = drawGroup;
        ring->moveType  = Ring::Fixed;
        ring->velocity.x += Zone::sVars->autoScrollSpeed / 2;
        angle += 0x10;
    }

    if (globals->useManiaBehavior) {
        angleStart = 0xC0 - 8 * (bigRingCount & -2);
        angle      = angleStart + 8;
        if (!(bigRingCount & 1))
            angle = angleStart - 8;

        for (int32 i = 0; i < bigRingCount; ++i) {
            Ring *ringGrow       = GameObject::Create<Ring>(position, position->x, position->y);
            ringGrow->velocity.x = Math::Cos256(angle) << 11;
            ringGrow->velocity.y = Math::Sin256(angle) << 11;
            ringGrow->animator.SetAnimation(sVars->aniFrames, Ring::Combi, true, 0);
            ringGrow->scale.x        = 0x100;
            ringGrow->scale.y        = 0x100;
            ringGrow->animator.speed = 0x200;
            ringGrow->drawFX         = FX_FLIP | FX_ROTATE | FX_SCALE;
            ringGrow->state.Set(&Ring::State_LostFX);
            ringGrow->stateDraw.Set(&Ring::Draw_Normal);
            angle += 0x10;
        }
    }
}
void Ring::LoseHyperRings(RSDK::Vector2 *position, int32 rings, uint8 cPlane)
{
    int32 ringCount = CLAMP(rings >> 2, 1, 8);
    int32 ringValue = rings / ringCount;

    int32 startAngle = 0xC0 - 0x10 * (ringCount & 0xFFFFFFFE);
    int32 angle      = startAngle + 0x10;
    if (!(ringCount & 1))
        angle = startAngle - 0x10;

    for (int32 i = 0; i < ringCount; ++i) {
        Ring *hyperRing = GameObject::Create<Ring>(position, position->x, position->y);
        hyperRing->animator.SetAnimation(sVars->aniFrames, Ring::Combi, true, 0);
        hyperRing->type           = Ring::Combi;
        hyperRing->velocity.x     = 0x300 * Math::Cos256(angle);
        hyperRing->isPermanent    = true;
        hyperRing->drawFX         = FX_FLIP | FX_SCALE;
        hyperRing->alpha          = 0x100;
        hyperRing->velocity.y     = 0x300 * Math::Sin256(angle);
        hyperRing->animator.speed = 0x180;
        hyperRing->collisionPlane = cPlane;
        hyperRing->angle          = i << 6;
        hyperRing->ringAmount     = ringValue;
        hyperRing->state.Set(&Ring::State_Combi);
        hyperRing->stateDraw.Set(&Ring::Draw_Normal);
        angle += 0x20;
    }
}
void Ring::FakeLoseRings(Vector2 *position, int32 ringCount, uint8 drawGroup)
{
    int32 ringCount2 = ringCount - 16;
    int32 ringCount3 = ringCount2 - 16;
    int32 radius     = 0x800;
    int32 scale      = 0x100;

    if (ringCount > 16) {
        ringCount = 16;
        radius    = 0x600;
        scale     = 0xC0;
    }

    if (ringCount2 <= 16) {
        if (ringCount2 < 0)
            ringCount2 = 0;
    }
    else {
        radius -= 0x200;
        ringCount2 = 16;
        scale -= 0x40;
    }

    ringCount3 = CLAMP(ringCount3, 0, 16);

    int32 angleVal = 0xC0 - 8 * (ringCount2 & 0xFFFFFFFE);
    int32 angle    = angleVal + 8;
    if (!(ringCount2 & 1))
        angle = angleVal - 8;

    for (int32 r = 0; r < ringCount2; ++r) {
        Ring *ring       = GameObject::Create<Ring>(position, position->x, position->y);
        ring->velocity.x = radius * Math::Cos256(angle);
        ring->velocity.y = radius * Math::Sin256(angle);
        ring->animator.SetAnimation(sVars->aniFrames, Ring::Combi, true, 0);
        ring->animator.speed = 0x200;
        ring->scale.x        = scale;
        ring->scale.y        = scale;
        ring->drawFX         = FX_FLIP | FX_SCALE;
        ring->drawGroup      = drawGroup;
        ring->state.Set(&Ring::State_LostFX);
        ring->stateDraw.Set(&Ring::Draw_Normal);
        angle += 0x10;
    }

    angleVal = 0xC0 - 8 * (ringCount & 0xFFFFFFFE);
    angle    = angleVal + 8;
    if (!(ringCount & 1))
        angle = angleVal - 8;

    for (int32 r = 0; r < ringCount; ++r) {
        Ring *ring       = GameObject::Create<Ring>(position, position->x, position->y);
        ring->velocity.x = (radius + 0x200) * Math::Cos256(angle);
        ring->velocity.y = (radius + 0x200) * Math::Sin256(angle);
        ring->animator.SetAnimation(sVars->aniFrames, Ring::Combi, true, 0);
        ring->animator.speed = 0x200;
        ring->scale.x        = scale + 0x40;
        ring->scale.y        = scale + 0x40;
        ring->drawFX         = FX_FLIP | FX_SCALE;
        ring->drawGroup      = drawGroup;
        ring->state.Set(&Ring::State_LostFX);
        ring->stateDraw.Set(&Ring::Draw_Normal);
        angle += 0x10;
    }

    angleVal = 0xC0 - 8 * (ringCount3 & 0xFFFFFFFE);
    angle    = angleVal + 8;
    if (!(ringCount3 & 1))
        angle = angleVal - 8;

    for (int32 r = 0; r < ringCount3; ++r) {
        Ring *ringGrow       = GameObject::Create<Ring>(position, position->x, position->y);
        ringGrow->velocity.x = (radius + 0x400) * Math::Cos256(angle);
        ringGrow->velocity.y = (radius + 0x400) * Math::Sin256(angle);
        ringGrow->animator.SetAnimation(sVars->aniFrames, Ring::Combi, true, 0);
        ringGrow->animator.speed = 0x200;
        ringGrow->scale.x        = scale + 0x80;
        ringGrow->scale.y        = scale + 0x80;
        ringGrow->drawFX         = FX_FLIP | FX_SCALE;
        ringGrow->drawGroup      = drawGroup;
        ringGrow->state.Set(&Ring::State_LostFX);
        ringGrow->stateDraw.Set(&Ring::Draw_Normal);
        angle += 0x10;
    }
}

int32 Ring::CheckPlatformCollisions(Platform *platform)
{
    int32 side = C_NONE;
    if (!platform->state.Matches(&Platform::State_Falling2) && !platform->state.Matches(&Platform::State_Hold)) {
        platform->position.x = platform->drawPos.x - platform->collisionOffset.x;
        platform->position.y = platform->drawPos.y - platform->collisionOffset.y;

        switch (platform->collision) {
            case Platform::C_Platform: {
                Hitbox *hitbox = platform->animator.GetHitbox(0);
                if (this->velocity.y >= 0 && (hitbox->top || hitbox->bottom || hitbox->left || hitbox->right))
                    side = platform->CheckCollisionPlatform(hitbox, this, &sVars->hitbox, true);
                break;
            }

            case Platform::C_Solid:
            case Platform::C_SolidHurtSides:
            case Platform::C_SolidHurtBottom:
            case Platform::C_SolidHurtTop:
            case Platform::C_SolidHold:
            case Platform::C_SolidSticky:
            case Platform::C_StickyTop:
            case Platform::C_StickyLeft:
            case Platform::C_StickyRight:
            case Platform::C_StickyBottom:
            case Platform::C_SolidBarrel:
            case Platform::C_SolidNoCrush:
            case Platform::C_SolidHurtAll:
            case Platform::C_SolidHurtNoCrush:
            case Platform::C_Null: {
                Hitbox *hitbox = platform->animator.GetHitbox(1);
                if (hitbox->top || hitbox->bottom || hitbox->left || hitbox->right)
                    side = platform->CheckCollisionBox(hitbox, this, &sVars->hitbox, true);
                break;
            }

            case Platform::C_Tiled:
                side = C_NONE;
                if (platform->CheckCollisionTouchBox(&platform->hitbox, this, &sVars->hitbox) && this->collisionLayers & Zone::sVars->moveLayerMask) {
                    TileLayer *moveLayer  = Zone::sVars->moveLayer.GetTileLayer();
                    moveLayer->position.x = -(platform->drawPos.x + platform->tileOrigin.x) >> 16;
                    moveLayer->position.y = -(platform->drawPos.y + platform->tileOrigin.y) >> 16;
                }
                break;

            default: break;
        }

        platform->position.x = platform->centerPos.x;
        platform->position.y = platform->centerPos.y;
        if (side == C_LEFT || side == C_RIGHT)
            this->velocity.x -= platform->velocity.x;
    }

    return side;
}
void Ring::CheckObjectCollisions(int32 x, int32 y)
{
    int32 collisionSides = 0;
    int32 xVel           = this->velocity.x;
    int32 yVel           = this->velocity.y;

    if (Platform::sVars) {
        for (auto platform : GameObject::GetEntities<Platform>(FOR_ACTIVE_ENTITIES)) {
            collisionSides |= 1 << CheckPlatformCollisions(platform);
        }
    }
    
    // if (Bridge::sVars) {
    //     for (auto bridge : GameObject::GetEntities<Bridge>(FOR_ACTIVE_ENTITIES)) {
    //         bool32 collided = bridge->HandleCollisions(this, &sVars->hitbox, false, false);
    //         if (collided)
    //             collisionSides |= 2;
    //     }
    // }

    if (Spikes::sVars) {
        for (auto spikes : GameObject::GetEntities<Spikes>(FOR_ACTIVE_ENTITIES)) {
            collisionSides |= 1 << spikes->CheckCollisionBox(&spikes->hitbox, this, &sVars->hitbox, true);
        }
    }

    if (xVel <= 0) {
        if (!(collisionSides & 8) && this->TileCollision(Zone::sVars->collisionLayers, CMODE_RWALL, this->collisionPlane, -x, 0, true))
            this->velocity.x = -xVel;
    }
    else if (!(collisionSides & 4) && this->TileCollision(Zone::sVars->collisionLayers, CMODE_LWALL, this->collisionPlane, x, 0, true)) {
        this->velocity.x = -xVel;
    }

    if (yVel <= 0) {
        if (collisionSides & 0x10 || this->TileCollision(Zone::sVars->collisionLayers, CMODE_ROOF, this->collisionPlane, 0, -y, true))
            this->velocity.y = -yVel;
    }
    else if (collisionSides & 2 || this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, y, true)) {
        this->velocity.y = (yVel >> 2) - yVel;
        if (this->velocity.y > -0x10000)
            this->velocity.y = -0x10000;
    }
}

void Ring::State_Normal()
{
    SET_CURRENT_STATE();

    Collect();

    this->animator.frameID = Zone::sVars->ringFrame;
}
void Ring::State_Linear()
{
    SET_CURRENT_STATE();

    this->drawPos.x = this->position.x + this->amplitude.x * Math::Sin1024(this->speed * Zone::sVars->timer);
    this->drawPos.y = this->position.y + this->amplitude.y * Math::Sin1024(this->speed * Zone::sVars->timer);

    Collect();

    this->animator.frameID = Zone::sVars->ringFrame;
}
void Ring::State_Circular()
{
    SET_CURRENT_STATE();

    this->drawPos.x = this->position.x + this->amplitude.x * Math::Cos1024(this->speed * Zone::sVars->timer + 4 * this->angle);
    this->drawPos.y = this->position.y + this->amplitude.y * Math::Sin1024(this->speed * Zone::sVars->timer + 4 * this->angle);

    Collect();

    this->animator.frameID = Zone::sVars->ringFrame;
}
void Ring::State_Path()
{
    SET_CURRENT_STATE();

    this->drawPos.x += this->velocity.x;
    this->drawPos.y += this->velocity.y;

    Entity *node = GameObject::Get(this->speed);

    if (this->velocity.x <= 0) {
        if (this->drawPos.x < node->position.x) {
            this->drawPos.x = node->position.x;
        }
    }
    else if (this->drawPos.x > node->position.x) {
        this->drawPos.x = node->position.x;
    }

    if (this->velocity.y <= 0) {
        if (this->drawPos.y < node->position.y) {
            this->drawPos.y = node->position.y;
        }
    }
    else if (this->drawPos.y > node->position.y) {
        this->drawPos.y = node->position.y;
    }

    Collect();

    this->animator.frameID = Zone::sVars->ringFrame;
}
void Ring::State_Track()
{
    SET_CURRENT_STATE();

    uint16 timeVal = Zone::sVars->timer << 7;
    if ((timeVal & 0x100) == this->direction) {
        this->drawPos.x = this->position.x + (timeVal * this->amplitude.x >> 6) - (this->amplitude.x << 15);
        this->drawPos.y = this->position.y + (timeVal * this->amplitude.y >> 6) - (this->amplitude.y << 15);
    }
    else {
        this->drawPos.x = this->position.x + (this->amplitude.x << 15) - (timeVal * this->amplitude.x >> 6);
        this->drawPos.y = this->position.y + (this->amplitude.y << 15) - (timeVal * this->amplitude.y >> 6);
    }

    Collect();

    this->animator.frameID = Zone::sVars->ringFrame;
}
void Ring::State_Lost()
{
    SET_CURRENT_STATE();

    switch (globals->gravityDir) {
        default: break;
        case CMODE_FLOOR: this->velocity.y += 0x1800; break;
        case CMODE_ROOF: this->velocity.y -= 0x1800; break;
    }

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    if (this->moveType != Ring::Fixed) {

        switch (globals->gravityDir) {
            default: break;

            case CMODE_FLOOR:
                if (this->velocity.y > 0 && this->TileCollision(Zone::sVars->collisionLayers, CMODE_ROOF, this->collisionPlane, 0, -0x80000, true)) {
                    int32 yvel = this->velocity.y - (this->velocity.y >> 2);
                    if (yvel < 0x10000)
                        yvel = 0x10000;
                    this->velocity.y = yvel;
                }
                break;

            case CMODE_ROOF:
                if (this->velocity.y > 0 && this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 0x80000, true)) {
                    int32 yvel = (this->velocity.y >> 2) - this->velocity.y;
                    if (yvel > -0x10000)
                        yvel = -0x10000;
                    this->velocity.y = yvel;
                }
                break;
        }
    }
    else {
        CheckObjectCollisions(0x80000, 0x80000);
    }

    this->animator.Process();

    if (!(++this->timer & 7)) {
        if (this->animator.speed > 0x40)
            this->animator.speed -= 0x10;
    }

    if (this->timer > 0x3F)
        Collect();

    if (this->timer > 0xFF)
        this->Destroy();
    else if (this->timer >= 0xF0)
        this->alpha -= 0x10;
}
void Ring::State_LostFX()
{
    SET_CURRENT_STATE();

    this->velocity.y += 0x1800;
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->animator.Process();

    this->scale.x += 0x10;
    this->scale.y += 0x10;

    if (++this->timer > 64)
        this->Destroy();
}
void Ring::State_Combi()
{
    SET_CURRENT_STATE();

    this->velocity.y += 0x1200;
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->angle += this->animator.speed >> 6;
    if (this->timer <= 0xF0) {
        this->scale.x = (-Math::Sin256(this->angle) >> 1) + 0x180;
        this->scale.y = (Math::Sin256(this->angle) >> 1) + 0x180;
    }
    else {
        if (this->scale.x > this->scale.y) {
            this->scale.x += 8;
            this->scale.y -= (this->scale.y >> 3);
        }
        else {
            this->scale.y += 8;
            this->scale.x -= (this->scale.x >> 3);
        }
    }

    int32 x = this->scale.x << 11;
    int32 y = this->scale.y << 11;
    this->CheckObjectCollisions(x, y);

    if (!(this->angle & 0xF)) {
        Ring *sparkle = GameObject::Create<Ring>(nullptr, this->position.x + Math::Rand(-x, x), this->position.y + Math::Rand(-y, y));
        sparkle->state.Set(&Ring::State_Sparkle);
        sparkle->stateDraw.Set(&Ring::Draw_Sparkle);
        sparkle->active  = ACTIVE_NORMAL;
        sparkle->visible = false;
        sparkle->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
        sparkle->maxFrameCount  = sparkle->animator.frameCount - 1;
        sparkle->animator.speed = 4;
        this->sparkleType       = (this->sparkleType + 1) % 3;
    }

    this->animator.Process();

    if (!(++this->timer & 7)) {
        if (this->animator.speed > 0x80) {
            this->animator.speed -= 8;
        }
    }

    if (this->timer >= 72)
        Collect();

    if (this->timer > 0xFF)
        this->Destroy();
}
void Ring::State_Attracted()
{
    SET_CURRENT_STATE();

    Player *player = this->storedPlayer;
    if (player->shield == Shield::Lightning) {
        int32 startX = this->position.x;
        int32 startY = this->position.y;

        if (this->position.x <= player->position.x) {
            if (this->velocity.x >= 0)
                this->velocity.x += 0x3000;
            else
                this->velocity.x += 0xC000;
        }
        else {
            if (this->velocity.x <= 0)
                this->velocity.x -= 0x3000;
            else
                this->velocity.x -= 0xC000;
        }

        if (startY <= player->position.y) {
            if (this->velocity.y >= 0)
                this->velocity.y += 0x3000;
            else
                this->velocity.y += 0xC000;
        }
        else {
            if (this->velocity.y <= 0)
                this->velocity.y -= 0x3000;
            else
                this->velocity.y -= 0xC000;
        }

        this->position.x = startX + this->velocity.x;
        this->position.y = startY + this->velocity.y;
    }
    else {
        this->state.Set(&Ring::State_Lost);
        this->animator.speed = 0x80;
        this->alpha          = 0x100;
        this->inkEffect      = INK_ALPHA;
    }

    Collect();

    this->animator.frameID = Zone::sVars->ringFrame;
}
void Ring::State_Sparkle()
{
    SET_CURRENT_STATE();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    if (this->timer <= 0) {
        this->visible = true;

        this->animator.Process();

        if (this->animator.frameID >= this->maxFrameCount)
            this->Destroy();
    }
    else {
        this->visible = false;
        this->timer--;
    }
}

void Ring::Draw_Normal()
{
    this->direction = this->animator.frameID > 8;
    this->animator.DrawSprite(nullptr, false);
}
void Ring::Draw_Oscillating()
{
    this->direction = this->animator.frameID > 8;
    this->animator.DrawSprite(&this->drawPos, false);
}
void Ring::Draw_Sparkle()
{

    if (this->alpha == 0xE0) {
        this->animator.frameID += 16;
        this->inkEffect = INK_ADD;
        this->animator.DrawSprite(nullptr, false);

        this->inkEffect = INK_NONE;
        this->animator.frameID -= 16;
    }
    this->animator.DrawSprite(nullptr, false);
}

#if RETRO_INCLUDE_EDITOR
void Ring::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, this->type & 1, true, 0);

    Vector2 drawPos;
    switch (this->moveType) {
        case Ring::Path:
            this->animator.DrawSprite(nullptr, false);

            drawPos.x = this->amplitude.x + this->position.x;
            drawPos.y = this->amplitude.y + this->position.y;
            this->animator.DrawSprite(&drawPos, false);

            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0LL, 0x00, INK_NONE, false);

            drawPos.x = this->position.x - this->amplitude.x;
            drawPos.y = this->position.y - this->amplitude.y;
            this->animator.DrawSprite(&drawPos, false);

            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0x00, INK_NONE, false);
            break;

        // unknown lol
        case Ring::Unknown_5:
            this->animator.DrawSprite(nullptr, false);
            drawPos.x = this->position.x + (this->amplitude.x >> 8) * Math::Cos256(this->angle);
            drawPos.y = this->position.y + (this->amplitude.y >> 8) * Math::Sin256(this->angle);

            this->animator.DrawSprite(&drawPos, false);
            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0x00, INK_NONE, false);
            break;

        // also unknown lol
        case Ring::Unknown_6:
            this->animator.DrawSprite(nullptr, false);

            drawPos.x = this->position.x + (this->amplitude.x >> 1);
            drawPos.y = this->position.y + (this->amplitude.y >> 1);
            this->animator.DrawSprite(&drawPos, false);

            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0x00, INK_NONE, false);

            drawPos.x = this->position.x - (this->amplitude.x >> 1);
            drawPos.y = this->position.y - (this->amplitude.y >> 1);
            this->animator.DrawSprite(&drawPos, false);

            Graphics::DrawLine(this->position.x, this->position.y, drawPos.x, drawPos.y, 0xE0E0E0, 0x00, INK_NONE, false);
            break;

        case Ring::Fixed:
        case Ring::Linear:
        case Ring::Circle:
        case Ring::Track:
        default: this->animator.DrawSprite(nullptr, false); break;
    }
}

void Ring::EditorLoad()
{
    sVars->aniFrames.Load("Global/Ring.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Normal");
    RSDK_ENUM_VAR("Combi");

    RSDK_ACTIVE_VAR(sVars, planeFilter);
    RSDK_ENUM_VAR("All");
    RSDK_ENUM_VAR("AL");
    RSDK_ENUM_VAR("BL");
    RSDK_ENUM_VAR("AH");
    RSDK_ENUM_VAR("BH");

    RSDK_ACTIVE_VAR(sVars, moveType);
    RSDK_ENUM_VAR("Fixed");
    RSDK_ENUM_VAR("Linear");
    RSDK_ENUM_VAR("Circular");
    RSDK_ENUM_VAR("Track");
    RSDK_ENUM_VAR("Path");
}
#endif

#if RETRO_REV0U
void Ring::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Ring);

    sVars->aniFrames.Init();
    sVars->sfxRing.Init();
}
#endif

void Ring::Serialize()
{
    RSDK_EDITABLE_VAR(Ring, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(Ring, VAR_ENUM, planeFilter);
    RSDK_EDITABLE_VAR(Ring, VAR_ENUM, moveType);
    RSDK_EDITABLE_VAR(Ring, VAR_VECTOR2, amplitude);
    RSDK_EDITABLE_VAR(Ring, VAR_ENUM, speed);
    RSDK_EDITABLE_VAR(Ring, VAR_INT32, angle);
}

} // namespace GameLogic