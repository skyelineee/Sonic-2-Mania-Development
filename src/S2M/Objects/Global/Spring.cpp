// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Spring Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Spring.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "DebugMode.hpp"
#include "Common/ScreenWrap.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Spring);

void Spring::Update()
{
    if (this->sfxTimer > 0)
        this->sfxTimer--;

    this->state.Run(this);

    this->animator.Process();

    if (this->animator.frameID == this->animator.frameCount - 1)
        this->animator.speed = 0;
}
void Spring::LateUpdate() {}
void Spring::StaticUpdate() {}
void Spring::Draw()
{
    this->animator.DrawSprite(&this->position, false);
    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Spring::Draw), true);
}

void Spring::Create(void *data)
{
    this->drawFX = FX_FLIP;
    if (!sceneInfo->inEditor) {
        this->type %= 6;
        if (data) {
            int32 propertyVal = VOID_TO_INT(data);
            this->type        = (propertyVal >> 0) & 0xFF;
            this->flipFlag    = (propertyVal >> 8) & 0xFF;
        }

        this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);
        this->active         = ACTIVE_BOUNDS;
        this->animator.speed = 0;
        this->updateRange.x  = 0x600000;
        this->updateRange.y  = 0x600000;
        this->visible        = true;

        if (this->planeFilter && ((uint8)this->planeFilter - 1) & 2)
            this->drawGroup = Zone::sVars->objectDrawGroup[1];
        else
            this->drawGroup = Zone::sVars->objectDrawGroup[0];

        switch (this->type >> 1) {
            case 0: // vertical
                this->direction = this->flipFlag;

                if (this->velOverride >= 1) {
                    this->velocity.y = this->velOverride << 16;
                }
                else {
                    if (this->type & 1)
                        this->velocity.y = 0x100000;
                    else
                        this->velocity.y = 0xA0000;
                }

                if (!this->flipFlag)
                    this->velocity.y = -this->velocity.y;

                this->hitbox.left   = -16;
                this->hitbox.top    = -8;
                this->hitbox.right  = 16;
                this->hitbox.bottom = 8;

                this->state.Set(&Spring::State_Vertical);
                break;

            case 1: // horizontal
                this->direction = this->flipFlag;

                if (this->velOverride >= 1) {
                    this->velocity.x = this->velOverride << 16;
                }
                else {
                    if (this->type & 1)
                        this->velocity.x = 0x100000;
                    else
                        this->velocity.x = 0xA0000;
                }

                if (this->flipFlag)
                    this->velocity.x = -this->velocity.x;

                this->hitbox.left   = -8;
                this->hitbox.top    = -16;
                this->hitbox.right  = 8;
                this->hitbox.bottom = 16;

                this->state.Set(&Spring::State_Horizontal);
                break;

            case 2: // diagonal
                this->direction = this->flipFlag;

                if (this->velOverride >= 1) {
                    this->velocity.x = this->velOverride << 16;
                    this->velocity.y = this->velOverride << 16;
                }
                else {
                    if (this->type & 1) {
                        this->velocity.x = 0xB4000;
                        this->velocity.y = 0xB4000;
                    }
                    else {
                        this->velocity.x = 0x74000;
                        this->velocity.y = 0x74000;
                    }
                }

                if (this->flipFlag < FLIP_Y)
                    this->velocity.y = -this->velocity.y;
                if (this->flipFlag & FLIP_X)
                    this->velocity.x = -this->velocity.x;

                this->hitbox.left   = -12;
                this->hitbox.top    = -12;
                this->hitbox.right  = 12;
                this->hitbox.bottom = 12;

                this->state.Set(&Spring::State_Diagonal);
                break;
        }

        if (ScreenWrap::CheckCompetitionWrap())
            this->active = ACTIVE_NORMAL;
    }
}

void Spring::StageLoad()
{
    sVars->aniFrames.Load("Global/Springs.bin", SCOPE_STAGE);
    sVars->sfxSpring.Get("Global/Spring.wav");

    DebugMode::AddObject(sVars->classID, &Spring::DebugSpawn, &Spring::DebugDraw);
}

void Spring::DebugSpawn()
{
    Spring *spring = GameObject::Create<Spring>(0, this->position.x, this->position.y);

    int32 type = DebugMode::sVars->itemType;
    if (type <= 0xF) {
        if ((1 << type) & 0xFF00) {
            if (((type & 0xFB) - 9) <= 2)
                spring->flipFlag = ((type & 0xFB) - 9) + 1;

            if (type < 12)
                spring->type = Spring::YellowDiagonal;
            else
                spring->type = Spring::RedDiagonal;
        }
        else if ((1 << type) & 0xF) {
            spring->type = type >> 1;
            if ((type & 0xFFFFFFFD) == 1)
                spring->flipFlag = FLIP_Y;
        }
        else {
            spring->type = type >> 1;
            if ((type & 0xFFFFFFFD) == 5)
                spring->flipFlag = FLIP_X;
        }

        spring->Create(nullptr);
    }
}
void Spring::DebugDraw()
{
    DebugMode::sVars->itemTypeCount = 16;
    this->direction                 = FLIP_NONE;

    if (DebugMode::sVars->itemType <= 0xF) {
        if ((1 << DebugMode::sVars->itemType) & 0xFF00) {
            if ((1 << DebugMode::sVars->itemType) & 0xEE00) {
                int32 dirType = DebugMode::sVars->itemType & 0xFB;
                if (dirType == 10)
                    this->direction = FLIP_Y;
                else
                    this->direction = FLIP_X;

                if (dirType == 11)
                    this->direction = FLIP_XY;
            }

            if (DebugMode::sVars->itemType < 12)
                DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 4, true, 0);
            else
                DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 5, true, 0);
        }
        else {
            if ((1 << DebugMode::sVars->itemType) & 0xF) {
                if ((DebugMode::sVars->itemType & 0xFFFFFFFD) == 1)
                    this->direction = FLIP_Y;
            }

            if ((DebugMode::sVars->itemType & 0xFFFFFFFD) == Spring::RedDiagonal)
                this->direction = FLIP_X;

            DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, DebugMode::sVars->itemType >> 1, true, 0);
        }
    }

    DebugMode::sVars->animator.DrawSprite(nullptr, false);

    this->direction = FLIP_NONE;
}

void Spring::State_Vertical()
{
    SET_CURRENT_STATE();

    if (this->direction == FLIP_NONE) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if ((!this->planeFilter || player->collisionPlane == ((uint8)(this->planeFilter - 1) & 1)) && this->interaction) {
                int32 side = ((this->type == Spring::TubeSpring && player->velocity.y < -0x50000) || this->passThrough)
                                 ? player->CheckCollisionPlatform(this, &this->hitbox)
                                 : player->CheckCollisionBox(this, &this->hitbox);

                bool32 collided = side == C_TOP;

                if (!collided && player->onGround) {
                    if (player->velocity.y >= 0) {
                        Hitbox hitbox;
                        hitbox.left   = this->hitbox.left;
                        hitbox.top    = this->hitbox.top - 8;
                        hitbox.right  = this->hitbox.right;
                        hitbox.bottom = this->hitbox.top;
                        if (player->CheckCollisionTouch(this, &hitbox)) {
                            collided = true;
                        }
                    }
                }

                if (collided) {
                    int32 anim = player->animator.animationID;

                    player->animationReserve = Player::ANI_WALK;
                    if (anim > Player::ANI_WALK && anim <= Player::ANI_DASH)
                        player->animationReserve = player->animator.animationID;

                    if (player->state.Matches(&Player::State_TubeAirRoll) || player->state.Matches(&Player::State_TubeRoll)) {
                        player->state.Set(&Player::State_TubeAirRoll);
                    }
                    else {
                        if (anim == Player::ANI_JUMP)
                            player->position.y -= player->jumpOffset;

                        if (!globals->gravityDir) {
                            if (this->twirl) {
                                if (player->spriteType) {
                                    player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                                }
                                else {
                                    player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, true, 0);
                                }
                            }
                            else {
                                player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                            }
                        }

                        player->state.Set(&Player::State_Air);
                    }

                    player->onGround       = false;
                    player->velocity.y     = this->velocity.y;
                    player->tileCollisions = globals->tileCollisionMode;
                    player->rotation       = (globals->tileCollisionMode != 1) << 8;

                    this->animator.speed   = 0x80;
                    this->animator.timer   = 0;
                    this->animator.frameID = 1;

                    if (!this->sfxTimer) {
                        sVars->sfxSpring.Play();
                        this->sfxTimer = 8;
                    }

                    if (this->swap)
                        player->collisionPlane = this->swap - 1;
                }
            }
        }
    }
    else {
        if (globals->gravityDir == CMODE_ROOF) {

            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                if ((!this->planeFilter || player->collisionPlane == ((uint8)(this->planeFilter - 1) & 1)) && this->interaction) {
                    int32 side = ((this->type == Spring::TubeSpring && player->velocity.y > 0x50000) || this->passThrough)
                                     ? (player->CheckCollisionPlatform(this, &this->hitbox) * C_BOTTOM)
                                     : player->CheckCollisionBox(this, &this->hitbox);

                    bool32 collided = side == C_BOTTOM;

                    if (!collided && player->onGround) {
                        if (player->velocity.y <= 0) {
                            Hitbox hitbox;
                            hitbox.left   = this->hitbox.left;
                            hitbox.top    = this->hitbox.bottom;
                            hitbox.right  = this->hitbox.right;
                            hitbox.bottom = this->hitbox.bottom + 8;
                            if (player->CheckCollisionTouch(this, &hitbox)) {
                                collided = true;
                            }
                        }
                    }

                    if (collided) {
                        int32 anim = player->animator.animationID;

                        player->animationReserve = Player::ANI_WALK;
                        if (anim > Player::ANI_WALK && anim <= Player::ANI_DASH)
                            player->animationReserve = player->animator.animationID;

                        if (player->state.Matches(&Player::State_TubeAirRoll) || player->state.Matches(&Player::State_TubeRoll)) {
                            player->state.Set(&Player::State_TubeAirRoll);
                        }
                        else {
                            if (anim == Player::ANI_JUMP)
                                player->position.y -= player->jumpOffset;

                            if (!globals->gravityDir) {
                                if (this->twirl) {
                                    if (player->spriteType) {
                                        player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                                    }
                                    else {
                                        player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, true, 0);
                                    }
                                }
                                else {
                                    player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                                }
                            }

                            player->state.Set(&Player::State_Air);
                        }

                        player->applyJumpCap   = 0;
                        player->onGround       = false;
                        player->velocity.y     = this->velocity.y;
                        player->tileCollisions = globals->tileCollisionMode;
                        player->rotation       = (globals->tileCollisionMode != 1) << 8;

                        this->animator.speed   = 0x80;
                        this->animator.timer   = 0;
                        this->animator.frameID = 1;

                        if (!this->sfxTimer) {
                            sVars->sfxSpring.Play();
                            this->sfxTimer = 8;
                        }

                        if (this->swap)
                            player->collisionPlane = this->swap - 1;
                    }
                }
            }
        }
        else {
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                if ((!this->planeFilter || player->collisionPlane == ((uint8)(this->planeFilter - 1) & 1)) && this->interaction) {
                    if (player->CheckCollisionBox(this, &this->hitbox) == C_BOTTOM) {

                        if (player->animator.animationID == Player::ANI_JUMP && globals->gravityDir == 2)
                            player->position.y += player->jumpOffset;

                        if (player->state.Matches(&Player::State_TubeAirRoll) || player->state.Matches(&Player::State_TubeRoll)) {
                            player->state.Set(&Player::State_TubeAirRoll);
                        }
                        else {
                            player->state.Set(&Player::State_Air);
                        }

                        player->applyJumpCap   = 0;
                        player->onGround       = false;
                        player->velocity.y     = this->velocity.y;
                        player->tileCollisions = globals->tileCollisionMode;
                        player->rotation       = (globals->tileCollisionMode != 1) << 8;

                        this->animator.speed   = 0x80;
                        this->animator.timer   = 0;
                        this->animator.frameID = 1;

                        if (!this->sfxTimer) {
                            sVars->sfxSpring.Play();
                            this->sfxTimer = 8;
                        }

                        if (this->swap)
                            player->collisionPlane = this->swap - 1;
                    }
                }
            }
        }
    }

    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Spring::State_Vertical), true);
}
void Spring::State_Horizontal()
{
    SET_CURRENT_STATE();

    bool32 forceActivate = false;
    if (this->direction == FLIP_NONE) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if ((!this->planeFilter || player->collisionPlane == ((uint8)(this->planeFilter - 1) & 1)) && this->interaction) {
                bool32 collided = false;

                if (player->onGround && !player->velocity.x) {
                    Hitbox hitbox;
                    hitbox.left   = this->hitbox.right;
                    hitbox.top    = this->hitbox.top;
                    hitbox.right  = this->hitbox.right + 8;
                    hitbox.bottom = this->hitbox.bottom;

                    if (player->CheckCollisionTouch(this, &hitbox)) {
                        collided      = true;
                        forceActivate = true;
                    }
                }
                else {
                    forceActivate = false;

                    if (player->CheckCollisionBox(this, &this->hitbox) == C_RIGHT && (!this->onGround || player->onGround)) {
                        collided = true;
                    }
                }

                if (forceActivate || collided) {
                    if (player->collisionMode == CMODE_ROOF) {
                        player->velocity.x = -this->velocity.x;
                        player->groundVel  = -this->velocity.x;
                    }
                    else {
                        player->velocity.x = this->velocity.x;
                        player->groundVel  = player->velocity.x;
                    }

                    if (!player->state.Matches(&Player::State_Roll) && player->state.Matches(&Player::State_TubeAirRoll)
                        && !player->state.Matches(&Player::State_TubeRoll)) {
                        player->state.Set(player->onGround ? &Player::State_Ground : &Player::State_Air);
                    }

                    int32 anim = player->animator.animationID;
                    if (anim != Player::ANI_JUMP && (anim < Player::ANI_WALK || anim > Player::ANI_DASH))
                        player->animator.animationID = Player::ANI_WALK;

                    player->controlLock = 16;
                    player->skidding    = false;
                    player->pushing     = false;
                    player->direction   = FLIP_NONE;
                    player->rotation    = (globals->tileCollisionMode != 1) << 8;

                    this->animator.speed   = 0x80;
                    this->animator.timer   = 0;
                    this->animator.frameID = 1;

                    if (this->sfxTimer == 0) {
                        sVars->sfxSpring.Play();
                        this->sfxTimer = 8;
                    }

                    if (this->swap)
                        player->collisionPlane = this->swap - 1;
                }
            }
        }
    }
    else {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if ((!this->planeFilter || player->collisionPlane == ((uint8)(this->planeFilter - 1) & 1)) && this->interaction) {
                bool32 collided = false;

                if (player->onGround && !player->velocity.x) {
                    Hitbox hitbox;
                    hitbox.left   = this->hitbox.left - 8;
                    hitbox.top    = this->hitbox.top;
                    hitbox.right  = this->hitbox.left;
                    hitbox.bottom = this->hitbox.bottom;

                    if (player->CheckCollisionTouch(this, &hitbox)) {
                        collided      = true;
                        forceActivate = true;
                    }
                }
                else {
                    forceActivate = false;

                    if (player->CheckCollisionBox(this, &this->hitbox) == C_LEFT && (!this->onGround || player->onGround)) {
                        collided = true;
                    }
                }

                if (forceActivate || collided) {
                    if (player->collisionMode == CMODE_ROOF) {
                        player->velocity.x = -this->velocity.x;
                        player->groundVel  = -this->velocity.x;
                    }
                    else {
                        player->velocity.x = this->velocity.x;
                        player->groundVel  = player->velocity.x;
                    }

                    if (!player->state.Matches(&Player::State_Roll) && player->state.Matches(&Player::State_TubeAirRoll)
                        && !player->state.Matches(&Player::State_TubeRoll)) {
                        player->state.Set(player->onGround ? &Player::State_Ground : &Player::State_Air);
                    }

                    int32 anim = player->animator.animationID;
                    if (anim != Player::ANI_JUMP && (anim < Player::ANI_WALK || anim > Player::ANI_DASH))
                        player->animator.animationID = Player::ANI_WALK;

                    player->controlLock    = 16;
                    player->skidding       = 0;
                    player->pushing        = false;
                    player->direction      = FLIP_X;
                    player->tileCollisions = TILECOLLISION_DOWN;

                    this->animator.speed   = 0x80;
                    this->animator.timer   = 0;
                    this->animator.frameID = 1;

                    if (this->sfxTimer == 0) {
                        sVars->sfxSpring.Play();
                        this->sfxTimer = 8;
                    }

                    if (this->swap)
                        player->collisionPlane = this->swap - 1;
                }
            }
        }
    }

    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Spring::State_Horizontal), true);
}
void Spring::State_Diagonal()
{
    SET_CURRENT_STATE();

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if ((!this->planeFilter || player->collisionPlane == ((uint8)(this->planeFilter - 1) & 1)) && this->interaction) {
            if (player->CheckCollisionTouch(this, &this->hitbox)) {

                if (player->animator.animationID == Player::ANI_JUMP) {
                    if (globals->gravityDir == CMODE_ROOF && this->velocity.y >= 0)
                        player->position.y += player->jumpOffset;
                    else if (globals->gravityDir == CMODE_FLOOR && this->velocity.y >= 0)
                        player->position.y -= player->jumpOffset;
                }

                bool32 collided = false;
                if (player->onGround || player->velocity.y >= 0)
                    collided = true;
                else
                    collided = abs(player->velocity.x) > -player->velocity.y;

                if (collided) {
                    if (player->state.Matches(&Player::State_TubeAirRoll) || player->state.Matches(&Player::State_TubeRoll)) {
                        player->state.Set(&Player::State_TubeAirRoll);
                    }
                    else {
                        player->state.Set(&Player::State_Air);

                        int32 anim = player->animator.animationID;
                        if (anim != Player::ANI_JUMP && (anim < Player::ANI_WALK || anim > Player::ANI_DASH))
                            player->animator.animationID = Player::ANI_WALK;
                    }

                    if (this->direction < FLIP_Y) {
                        if (!player->state.Matches(&Player::State_TubeAirRoll) && !player->state.Matches(&Player::State_TubeRoll)) {
                            int32 anim               = player->animator.animationID;
                            player->animationReserve = Player::ANI_WALK;
                            if (anim > Player::ANI_WALK && anim <= Player::ANI_DASH)
                                player->animationReserve = player->animator.animationID;
                            player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                        }
                    }

                    int32 velocityX = this->velocity.x;

                    player->direction      = this->direction & 1;
                    player->onGround       = false;
                    player->velocity.x     = this->velocity.x;
                    player->velocity.y     = this->velocity.y;
                    player->tileCollisions = globals->tileCollisionMode;
                    player->rotation       = (globals->tileCollisionMode != 1) << 8;

                    if (player->shield >= 5)
                        player->velocity.x = velocityX + velocityX / 8;

                    this->animator.speed   = 0x80;
                    this->animator.timer   = 0;
                    this->animator.frameID = 1;

                    if (this->sfxTimer == 0) {
                        sVars->sfxSpring.Play();
                        this->sfxTimer = 8;
                    }

                    if (this->swap)
                        player->collisionPlane = this->swap - 1;
                }
            }
        }
    }

    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Spring::State_Diagonal), true);
}

void Spring::State_Falling()
{
    SET_CURRENT_STATE();

    this->velocity.y += 0x3800;
    this->position.y += this->velocity.y;

    if (!this->CheckOnScreen(&this->updateRange))
        this->Destroy();
}

#if RETRO_INCLUDE_EDITOR
void Spring::EditorDraw()
{
    this->type      = this->type % 6;
    this->direction = this->flipFlag & 3;
    this->flipFlag &= 3;

    this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);
    this->animator.DrawSprite(&this->position, false);
}

void Spring::EditorLoad()
{
    sVars->aniFrames.Load("Global/Springs.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, planeFilter);
    RSDK_ENUM_VAR("All");
    RSDK_ENUM_VAR("AL");
    RSDK_ENUM_VAR("BL");
    RSDK_ENUM_VAR("AH");
    RSDK_ENUM_VAR("BH");

    RSDK_ACTIVE_VAR(sVars, swap);
    RSDK_ENUM_VAR("None");
    RSDK_ENUM_VAR("To A Plane");
    RSDK_ENUM_VAR("To B Plane");
}
#endif

#if RETRO_REV0U
void Spring::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Spring);

    sVars->aniFrames.Init();

    sVars->sfxSpring.Init();
}
#endif

void Spring::Serialize()
{
    RSDK_EDITABLE_VAR(Spring, VAR_INT32, type);
    RSDK_EDITABLE_VAR(Spring, VAR_INT32, flipFlag);
    RSDK_EDITABLE_VAR(Spring, VAR_BOOL, onGround);
    RSDK_EDITABLE_VAR(Spring, VAR_UINT8, planeFilter);
    RSDK_EDITABLE_VAR(Spring, VAR_BOOL, twirl);
    RSDK_EDITABLE_VAR(Spring, VAR_UINT8, swap);
    RSDK_EDITABLE_VAR(Spring, VAR_BOOL, passThrough);
    RSDK_EDITABLE_VAR(Spring, VAR_INT16, velOverride);
}

} // namespace GameLogic