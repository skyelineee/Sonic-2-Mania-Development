// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Animals Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Animals.hpp"
#include "Player.hpp"
#include "Zone.hpp"
// #include "Common/Bridge.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Animals);

void Animals::Update()
{
    this->state.Run(this);

    if (!this->behaviour && !this->CheckOnScreen(nullptr)) {
        this->Destroy();
    }
}
void Animals::LateUpdate() {}
void Animals::StaticUpdate() {}
void Animals::Draw()
{
    uint8 dirStore = this->direction;
    if (globals->gravityDir == CMODE_ROOF)
        this->direction ^= FLIP_Y;

    this->animator.DrawSprite(&this->position, false);

    this->direction = dirStore;
}

void Animals::Create(void *data)
{
    if (this->behaviour == Animals::BehaveFixed)
        this->active = ACTIVE_BOUNDS;
    else
        this->active = ACTIVE_NORMAL;

    this->drawFX |= FX_FLIP;
    this->visible       = true;
    this->updateRange.x = 0x400000;
    this->updateRange.y = 0x400000;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];

    int32 type = Animals::Pocky;
    if (!(globals->secrets & SECRET_RICKYMODE))
        type = VOID_TO_INT(data);

    if (!this->type && Math::RandSeeded(0, 256, &Zone::sVars->randSeed) == 21) {
        type = Animals::Pocky;
        if (globals->gravityDir == CMODE_ROOF)
            this->velocity.y = 0x40000;
        else
            this->velocity.y = -0x40000;
        this->type = type - 1;
        this->state.Set(&Animals::State_Fall);
        this->hitboxAnimal.top    = -sVars->hitboxes[this->type] >> 16;
        this->hitboxAnimal.left   = -4;
        this->hitboxAnimal.right  = 4;
        this->hitboxAnimal.bottom = sVars->hitboxes[this->type] >> 16;
        this->animator.SetAnimation(sVars->aniFrames, 2 * this->type, true, 0);
    }
    else if (type) {
        if (globals->gravityDir == CMODE_ROOF)
            this->velocity.y = 0x40000;
        else
            this->velocity.y = -0x40000;
        this->type = type - 1;
        this->state.Set(&Animals::State_Fall);

        this->hitboxAnimal.top    = -sVars->hitboxes[this->type] >> 16;
        this->hitboxAnimal.left   = -4;
        this->hitboxAnimal.right  = 4;
        this->hitboxAnimal.bottom = sVars->hitboxes[this->type] >> 16;

        this->animator.SetAnimation(sVars->aniFrames, 2 * this->type, true, 0);
    }
    else if (this->behaviour == Animals::BehaveFollow) {
        this->active = ACTIVE_BOUNDS;
        switch (this->type) {
            case Animals::Flicky:
            case Animals::Cucky:
            case Animals::Locky: this->state.Set(&Animals::State_Fly); break;

            case Animals::Ricky:
            case Animals::Pocky:
            case Animals::Pecky:
            case Animals::Picky:
            case Animals::Rocky:
            case Animals::Becky:
            case Animals::Tocky:
            case Animals::Wocky:
            case Animals::Micky: this->state.Set(&Animals::State_Bounce); break;

            default: break;
        }

        Animals::CheckDirection();

        this->velocity.y = sVars->yVelocity[this->type];
        if (this->direction == FLIP_NONE)
            this->velocity.x = -sVars->xVelocity[this->type];
        else
            this->velocity.x = sVars->xVelocity[this->type];
        this->animator.SetAnimation(sVars->aniFrames, 2 * this->type + 1, true, 0);
    }
    else if (this->behaviour == Animals::BehaveIntro) {
        switch (this->type) {
            case Animals::Flicky:
            case Animals::Cucky:
            case Animals::Locky: this->state.Set(&Animals::State_IntroFly); break;

            case Animals::Ricky:
            case Animals::Pocky:
            case Animals::Pecky:
            case Animals::Picky:
            case Animals::Rocky:
            case Animals::Becky:
            case Animals::Tocky:
            case Animals::Wocky:
            case Animals::Micky: this->state.Set(&Animals::State_IntroBounce); break;

            default: break;
        }

        this->animator.SetAnimation(sVars->aniFrames, 2 * this->type + 1, true, 0);
        this->drawGroup = Zone::sVars->objectDrawGroup[0];
    }
    else {
        this->state.Set(&Animals::State_Placed);
    }
}

void Animals::StageLoad()
{
    sVars->aniFrames.Load("Global/Animals.bin", SCOPE_STAGE);

    // if (Platform::sVars)
    //     sVars->hasPlatform = true;
    // if (Bridge::sVars)
    //     sVars->hasBridge = true;
}

void Animals::CheckDirection()
{
    switch (this->behaviour) {
        default:
        case Animals::BehaveFree: this->direction = FLIP_X; break;

        case Animals::BehaveFollow: {
            Player *player = GameObject::Get<Player>(SLOT_PLAYER1);
            if (!player) {
                this->direction = FLIP_NONE;
            }
            else if (this->position.x < player->position.x) {
                this->direction = FLIP_NONE;
            }
            else {
                this->direction = FLIP_X;
            }
            break;
        }

        case Animals::BehaveFixed: this->direction = Math::RandSeeded(0, 2, &Zone::sVars->randSeed); break;
    }

    if (!this->direction)
        this->velocity.x = -sVars->xVelocity[this->type];
    else
        this->velocity.x = sVars->xVelocity[this->type];
}

/*bool32 Animals::CheckPlatformCollision(Platform *platform)
{
    bool32 collided = false;
    if (!platform->state.Matches(&Platform::State_Falling2) && !platform->state.Matches(&Platform::State_Hold)) {
        platform->position.x = platform->drawPos.x - platform->collisionOffset.x;
        platform->position.y = platform->drawPos.y - platform->collisionOffset.y;

        switch (platform->collision) {
            case Platform::C_Hurt:
            case Platform::C_None: 
            case Platform::C_SolidSpecial: 
            default: break;

            case Platform::C_Platform:
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
            case Platform::C_SolidConveyor:
            case Platform::C_SolidConveyorSwap:
                collided = platform->CheckCollisionPlatform(platform->animator.GetHitbox(0), this, &this->hitboxAnimal, true);
                break;

            case Platform::C_Tiled:
                if (platform->CheckCollisionTouchBox(&platform->hitbox, this, &this->hitboxAnimal)) {
                    if (this->collisionLayers & Zone::sVars->moveLayerMask) {
                        TileLayer *move  = Zone::sVars->moveLayer.GetTileLayer();
                        move->position.x = -(platform->drawPos.x + platform->tileOrigin.x) >> 16;
                        move->position.y = -(platform->drawPos.y + platform->tileOrigin.y) >> 16;
                    }

                    switch (globals->gravityDir) {
                        default: break;

                        case CMODE_FLOOR:
                            if (this->velocity.y >= 0x3800)
                                collided = true;
                            break;

                        case CMODE_ROOF:
                            if (this->velocity.y <= 0x3800)
                                collided = true;
                            break;
                    }
                }
                break;
        }

        platform->position.x = platform->centerPos.x;
        platform->position.y = platform->centerPos.y;
    }

    return collided;
}*/

bool32 Animals::CheckGroundCollision()
{
    switch (globals->gravityDir) {
        default: break;
        case CMODE_FLOOR:
            if (this->velocity.y <= 0)
                return false;

            if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, sVars->hitboxes[this->type], false))
                return true;
            break;

        case CMODE_ROOF:
            if (this->velocity.y >= 0)
                return false;

            if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, -sVars->hitboxes[this->type], false))
                return true;
            break;
    }

    if (sVars->hasPlatform) {
        // foreach_active(Platform, platform)
        // {
        //     if (CheckPlatformCollision(platform))
        //         return true;
        // }
    }

    if (sVars->hasBridge) {
        // for (auto bridge : GameObject::GetEntities<Bridge>(FOR_ACTIVE_ENTITIES))
        // {
        //     bool32 collided = bridge->HandleCollisions(this, &this->hitboxAnimal, false, false);
        //     if (collided) {
        //         return true;
        //     }
        // }
    }
    return false;
}

void Animals::State_Fall()
{
    SET_CURRENT_STATE();

    this->position.y += this->velocity.y;

    switch (globals->gravityDir) {
        default: break;

        case CMODE_FLOOR: this->velocity.y += 0x3800; break;

        case CMODE_ROOF: this->velocity.y -= 0x3800; break;
    }

    this->animator.Process();

    if (CheckGroundCollision()) {
        this->animator.SetAnimation(sVars->aniFrames, 2 * this->type + 1, true, 0);

        switch (this->type) {
            case Animals::Flicky:
            case Animals::Cucky:
            case Animals::Locky: this->state.Set(&Animals::State_Fly); break;

            case Animals::Ricky:
            case Animals::Pocky:
            case Animals::Pecky:
            case Animals::Picky:
            case Animals::Rocky:
            case Animals::Becky:
            case Animals::Tocky:
            case Animals::Wocky:
            case Animals::Micky: this->state.Set(&Animals::State_Bounce); break;

            default: break;
        }

        CheckDirection();
        this->velocity.y = sVars->yVelocity[this->type];
    }
}

void Animals::State_Fly()
{
    SET_CURRENT_STATE();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    switch (globals->gravityDir) {
        default: break;
        case CMODE_FLOOR: this->velocity.y += sVars->gravityStrength[this->type]; break;
        case CMODE_ROOF: this->velocity.y -= sVars->gravityStrength[this->type]; break;
    }

    if (CheckGroundCollision()) {
        CheckDirection();

        switch (globals->gravityDir) {
            default: break;
            case CMODE_FLOOR: this->velocity.y = sVars->yVelocity[this->type]; break;
            case CMODE_ROOF: this->velocity.y = -sVars->yVelocity[this->type]; break;
        }
    }

    this->animator.Process();
}

void Animals::State_Bounce()
{
    SET_CURRENT_STATE();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    switch (globals->gravityDir) {
        default: break;
        case CMODE_FLOOR: this->velocity.y += sVars->gravityStrength[this->type]; break;
        case CMODE_ROOF: this->velocity.y -= sVars->gravityStrength[this->type]; break;
    }

    if (CheckGroundCollision()) {
        CheckDirection();

        switch (globals->gravityDir) {
            default: break;
            case CMODE_FLOOR: this->velocity.y = sVars->yVelocity[this->type]; break;
            case CMODE_ROOF: this->velocity.y = -sVars->yVelocity[this->type]; break;
        }
    }

    switch (globals->gravityDir) {
        default: break;
        case CMODE_FLOOR: this->animator.frameID = this->velocity.y < 0; break;
        case CMODE_ROOF: this->animator.frameID = this->velocity.y > 0; break;
    }
}

void Animals::State_Placed()
{
    SET_CURRENT_STATE();

    if (this->timer <= 0) {
        this->drawGroup  = Zone::sVars->objectDrawGroup[0];
        this->startPos.x = 0;
        this->startPos.y = 0;

        if (this->behaviour == BehaveFlyFollow) {
            this->startPos.x = this->position.x;
            this->startPos.y = this->position.y;
            this->velocity.y = 0;
            this->angle      = 0xF8;

            this->animator.SetAnimation(sVars->aniFrames, 2 * this->type + 1, true, 0);
            this->state.Set(&Animals::State_FlyAround);
            this->position.y -= 1 << 16;
            this->drawGroup = Zone::sVars->objectDrawGroup[1];
        }
        else {
            this->drawGroup = Zone::sVars->objectDrawGroup[1];
            this->state.Set(&Animals::State_Fall);
        }
    }
    else {
        this->timer--;

        if (this->zdepth) {
            GameObject::Entity *parent = GameObject::Get<GameObject::Entity>(this->zdepth);

            this->position.x = this->startPos.x + parent->position.x;
            this->position.y = this->startPos.y + parent->position.y;
        }
    }
}

void Animals::State_IntroFly()
{
    SET_CURRENT_STATE();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->velocity.y += 0x1800;

    if (this->velocity.y >= 0) {
        if (this->TileCollision(Zone::sVars->collisionLayers, 0, 0, 0, 0x80000, true)) {
            this->velocity.y = -0x10000 - (Math::RandSeeded(0, 513, &Zone::sVars->randSeed) << 8);

            if (Math::RandSeeded(0, 3, &Zone::sVars->randSeed)) {
                this->velocity.x = -0x10000;
                this->direction  = FLIP_X;
            }
            else {
                if (!this->zdepth) {
                    this->velocity.x = 0x10000;
                    this->direction  = FLIP_NONE;
                }
                else {
                    this->velocity.x = -0x10000;
                    this->direction  = FLIP_X;
                }
            }
        }
    }

    this->animator.frameID = this->timer >> 1;
    this->timer            = (this->timer + 1) & 3;
}

void Animals::State_IntroBounce()
{
    SET_CURRENT_STATE();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->velocity.y += 0x3800;

    if (this->velocity.y >= 0) {
        if (this->TileCollision(Zone::sVars->collisionLayers, 0, 0, 0, 0x80000, true)) {
            this->velocity.y = -0x10000 - (Math::RandSeeded(0, 513, &Zone::sVars->randSeed) << 8);

            if (Math::RandSeeded(0, 3, &Zone::sVars->randSeed)) {
                this->velocity.x = -0x10000;
                this->direction  = FLIP_X;
            }
            else {
                if (!this->zdepth) {
                    this->velocity.x = 0x10000;
                    this->direction  = FLIP_NONE;
                }
                else {
                    this->velocity.x = -0x10000;
                    this->direction  = FLIP_X;
                }
            }
        }
    }

    this->animator.frameID = this->velocity.y >= 0;
}

void Animals::State_FlyAround()
{
    SET_CURRENT_STATE();

    Player *player = GameObject::Get<Player>(SLOT_PLAYER1);

    if (this->position.x <= player->position.x) {
        this->velocity.x += 0xC00;
        if (this->velocity.x >= 0x28000)
            this->velocity.x = 0x28000;
    }
    else {
        this->velocity.x -= 0xC00;
        if (this->velocity.x <= -0x28000)
            this->velocity.x = -0x28000;
    }

    this->direction = this->velocity.x <= 0 ? FLIP_X : FLIP_NONE;
    this->position.x += this->velocity.x;

    int32 targetY = player->position.y - 0x400000;

    if (this->startPos.y > targetY)
        this->startPos.y -= 0x8000;
    else if (this->startPos.y < targetY)
        this->startPos.y += 0x8000;
    else
        this->startPos.y = this->startPos.y;

    this->position.y       = this->startPos.y + (Math::Sin512(this->angle) << 10);
    this->animator.frameID = this->timer >> 2;
    this->timer            = (this->timer + 1) & 7;
    this->angle            = (this->angle + 8) & 0x1FF;
}

void Animals::State_WaitBounce()
{
    SET_CURRENT_STATE();

    if (this->timer <= 0) {
        this->drawGroup = Zone::sVars->objectDrawGroup[0];
        this->state.Set(&Animals::State_Bounce);
    }
    else {
        this->timer--;
    }
}

void Animals::State_FlyAway()
{
    SET_CURRENT_STATE();

    if (this->position.x <= 0) {
        this->velocity.x += 0xC00;
        if (this->velocity.x >= 0x28000)
            this->velocity.x = 0x28000;
    }
    else {
        this->velocity.x -= 0xC00;
        if (this->velocity.x <= -0x28000)
            this->velocity.x = -0x28000;
    }

    this->direction = this->velocity.x <= 0 ? FLIP_X : FLIP_NONE;
    this->position.x += this->velocity.x;

    int32 targetY = this->position.y;

    if (this->startPos.y > targetY)
        this->startPos.y -= 0x10000;
    else if (this->startPos.y < targetY)
        this->startPos.y += 0x10000;
    else
        this->startPos.y = this->startPos.y;

    this->position.y       = this->startPos.y + (Math::Sin512(this->angle) << 10);
    this->animator.frameID = this->timer >> 2;
    this->timer            = (this->timer + 1) & 7;
    this->angle            = (this->angle + 8) & 0x1FF;
}

void Animals::State_MissionPlaced()
{
    SET_CURRENT_STATE();

    if (this->timer <= 0) {
        this->drawGroup  = Zone::sVars->objectDrawGroup[0];
        this->startPos.x = this->position.x;
        this->startPos.y = this->position.y;
        this->velocity.y = 0;
        this->angle      = 0xF8;

        this->animator.SetAnimation(sVars->aniFrames, 2 * this->type + 1, true, 0);
        this->state.Set(&Animals::State_FlyAway);
    }
    else {
        this->timer--;

        if (this->zdepth) {
            GameObject::Entity *parent = GameObject::Get<GameObject::Entity>(this->zdepth);

            this->position.x = this->startPos.x + parent->position.x;
            this->position.y = this->startPos.y + parent->position.y;
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void Animals::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 2 * this->type, true, 0);
    this->animator.DrawSprite(&this->position, false);
}

void Animals::EditorLoad()
{
    sVars->aniFrames.Load("Global/Animals.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Flicky");
    RSDK_ENUM_VAR("Ricky");
    RSDK_ENUM_VAR("Pocky");
    RSDK_ENUM_VAR("Pecky");
    RSDK_ENUM_VAR("Picky");
    RSDK_ENUM_VAR("Cucky");
    RSDK_ENUM_VAR("Rocky");
    RSDK_ENUM_VAR("Becky");
    RSDK_ENUM_VAR("Locky");
    RSDK_ENUM_VAR("Tocky");
    RSDK_ENUM_VAR("Wocky");
    RSDK_ENUM_VAR("Micky");

    RSDK_ACTIVE_VAR(sVars, behaviour);
    RSDK_ENUM_VAR("Free");
    RSDK_ENUM_VAR("Follow");
    RSDK_ENUM_VAR("Fixed");
    RSDK_ENUM_VAR("Intro");
}
#endif

#if RETRO_REV0U
void Animals::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Animals);

    sVars->aniFrames.Init();

    sVars->animalTypes[0] = Animals::Flicky;
    sVars->animalTypes[1] = Animals::Flicky;

    sVars->hitboxes[0]  = 0x70000;
    sVars->hitboxes[1]  = 0x70000;
    sVars->hitboxes[2]  = 0xC0000;
    sVars->hitboxes[3]  = 0xA0000;
    sVars->hitboxes[4]  = 0x80000;
    sVars->hitboxes[5]  = 0x80000;
    sVars->hitboxes[6]  = 0x80000;
    sVars->hitboxes[7]  = 0x80000;
    sVars->hitboxes[8]  = 0x70000;
    sVars->hitboxes[9]  = 0x50000;
    sVars->hitboxes[10] = 0x70000;
    sVars->hitboxes[11] = 0x60000;

    sVars->gravityStrength[0]  = 0x1800;
    sVars->gravityStrength[1]  = 0x3800;
    sVars->gravityStrength[2]  = 0x3800;
    sVars->gravityStrength[3]  = 0x3800;
    sVars->gravityStrength[4]  = 0x3800;
    sVars->gravityStrength[5]  = 0x3800;
    sVars->gravityStrength[6]  = 0x3800;
    sVars->gravityStrength[7]  = 0x3800;
    sVars->gravityStrength[8]  = 0x1800;
    sVars->gravityStrength[9]  = 0x3800;
    sVars->gravityStrength[10] = 0x3800;
    sVars->gravityStrength[11] = 0x3800;

    sVars->yVelocity[0]  = -0x40000;
    sVars->yVelocity[1]  = -0x38000;
    sVars->yVelocity[2]  = -0x40000;
    sVars->yVelocity[3]  = -0x30000;
    sVars->yVelocity[4]  = -0x30000;
    sVars->yVelocity[5]  = -0x30000;
    sVars->yVelocity[6]  = -0x18000;
    sVars->yVelocity[7]  = -0x30000;
    sVars->yVelocity[8]  = -0x30000;
    sVars->yVelocity[9]  = -0x20000;
    sVars->yVelocity[10] = -0x30000;
    sVars->yVelocity[11] = -0x38000;

    sVars->xVelocity[0]  = -0x30000;
    sVars->xVelocity[1]  = -0x28000;
    sVars->xVelocity[2]  = -0x20000;
    sVars->xVelocity[3]  = -0x18000;
    sVars->xVelocity[4]  = -0x1C000;
    sVars->xVelocity[5]  = -0x20000;
    sVars->xVelocity[6]  = -0x14000;
    sVars->xVelocity[7]  = -0x20000;
    sVars->xVelocity[8]  = -0x28000;
    sVars->xVelocity[9]  = -0x14000;
    sVars->xVelocity[10] = -0x2C000;
    sVars->xVelocity[11] = -0x20000;
}
#endif

void Animals::Serialize()
{
    RSDK_EDITABLE_VAR(Animals, VAR_INT32, type);
    RSDK_EDITABLE_VAR(Animals, VAR_INT32, behaviour);
}

} // namespace GameLogic