// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SuperFlicky Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "SuperFlicky.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "Animals.hpp"
#include "Common/Water.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SuperFlicky);

void SuperFlicky::Update()
{
    if (this->targetSlot < 0)
        return;

    Entity *target = GameObject::Get(this->targetSlot);

    if (target->drawGroup > Zone::sVars->objectDrawGroup[1])
        this->drawGroup = target->drawGroup + 1;
    else
        this->drawGroup = Zone::sVars->objectDrawGroup[1];

    this->state.Run(this);

    if (!this->state.Matches(&SuperFlicky::State_Drop)) {
        if (this->targetPosition.x >= (this->position.x >> 16)) {
            if (this->velocity.x <= 0)
                this->velocity.x += 0x8000;
            else
                this->velocity.x += 0x2000;
        }
        else {
            if (this->velocity.x >= 0)
                this->velocity.x += -0x8000;
            else
                this->velocity.x += -0x2000;
        }

        bool32 reachedTargetVel = this->targetPosition.y == (this->position.y >> 16);
        this->targetPosition.y -= this->position.y >> 16;
        if (this->targetPosition.y < 0 || reachedTargetVel) {
            if (this->velocity.y > -0x100000) {
                if (this->velocity.y < 0)
                    this->targetVelocity.y = -0x20;
                else
                    this->targetVelocity.y = -0x80;
            }
            else {
                this->targetVelocity.y = 0x80;
            }
        }
        else {
            if (this->velocity.y < 0x100000) {
                if (this->velocity.y > 0)
                    this->targetVelocity.y = 0x20;
                else
                    this->targetVelocity.y = 0x80;
            }
            else {
                this->targetVelocity.y = -0x80;
            }
        }

        this->velocity.y += this->targetVelocity.y << 8;
        this->position.x += this->velocity.x;
        this->position.y += this->velocity.y;
    }

    this->angle += 2;

    this->animator.Process();

    if (this->velocity.x > 0x9000) {
        if (globals->tileCollisionMode == TILECOLLISION_DOWN)
            this->direction = FLIP_NONE;
        else
            this->direction = FLIP_Y;
    }
    else if (this->velocity.x < -0x9000) {
        if (globals->tileCollisionMode == TILECOLLISION_DOWN)
            this->direction = FLIP_X;
        else
            this->direction = FLIP_XY;
    }

    HandleSuperColors(false);
}
void SuperFlicky::LateUpdate() {}
void SuperFlicky::StaticUpdate()
{
    switch (sVars->state) {
        case 0:
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                if (player->characterID == ID_TAILS && player->superState == Player::SuperStateSuper) {
                    if (player->hyperAbilityState) {
                        sVars->targetPlayerID = player->playerID;
                        sVars->state          = 1;
                    }
                }
            }
            break;

        case 1: {
            for (int32 i = 0; i < 0x80; ++i) {
                Zone::sVars->flickyAttackList[i].slotID  = -1;
                Zone::sVars->flickyAttackList[i].classID = TYPE_NONE;
            }

            Entity *target           = GameObject::Get(sVars->targetPlayerID);
            sVars->activeFlickyCount = 4;
            for (int32 f = 0; f < sVars->activeFlickyCount; ++f) {
                SuperFlicky *flicky    = GameObject::Create<SuperFlicky>(0, target->position.x, target->position.y);
                flicky->isPermanent    = true;
                flicky->attackListPos  = -1;
                flicky->targetSlot     = sVars->targetPlayerID;
                flicky->flickyID       = f;
                flicky->attackDelay    = 120;
                flicky->superBlendMode = 2;
                flicky->angle          = 0x100 / sVars->activeFlickyCount * f;
                flicky->position.x -= (screenInfo->size.x / 2) << 16;
                flicky->position.y -= (screenInfo->size.x / 2) << 16;
            }
            sVars->state = 2;
            break;
        }

        case 2:
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                if (player->characterID == ID_TAILS && player->superState != Player::SuperStateSuper)
                    sVars->state = 3;
            }

            for (int32 i = 0; i < 0x80; ++i) {
                if (Zone::sVars->flickyAttackList[i].slotID != -1) {
                    if (Zone::sVars->flickyAttackList[i].isTargeted) {
                        Zone::sVars->flickyAttackList[i].timer--;
                        if (Zone::sVars->flickyAttackList[i].timer <= 0) {
                            if (!Zone::sVars->flickyAttackList[i].timer) {
                                for (auto flicky : GameObject::GetEntities<SuperFlicky>(FOR_ACTIVE_ENTITIES)) {
                                    if (i == flicky->attackListPos)
                                        flicky->attackListPos = -1;
                                }
                                Zone::sVars->flickyAttackList[i].slotID     = -1;
                                Zone::sVars->flickyAttackList[i].classID    = TYPE_NONE;
                                Zone::sVars->flickyAttackList[i].isTargeted = false;
                            }
                        }
                    }
                }
            }

            break;

        case 3:
            for (int32 i = 0; i < 0x80; ++i) {
                if (!Zone::sVars->hyperList[i].classID) {
                    Zone::sVars->flickyAttackList[i].slotID        = -1;
                    Zone::sVars->flickyAttackList[i].classID       = TYPE_NONE;
                    Zone::sVars->flickyAttackList[i].isTargeted    = false;
                    Zone::sVars->flickyAttackList[i].hitbox.left   = 0;
                    Zone::sVars->flickyAttackList[i].hitbox.top    = 0;
                    Zone::sVars->flickyAttackList[i].hitbox.right  = 0;
                    Zone::sVars->flickyAttackList[i].hitbox.bottom = 0;
                    Zone::sVars->flickyAttackList[i].position.x    = 0;
                    Zone::sVars->flickyAttackList[i].position.y    = 0;
                    Zone::sVars->flickyAttackList[i].timer         = 0;
                }
            }

            for (auto flicky : GameObject::GetEntities<SuperFlicky>(FOR_ACTIVE_ENTITIES)) {
                flicky->targetPosition.x = screenInfo->position.x;
                flicky->targetPosition.y = screenInfo->position.y;
                flicky->attackDelay      = 4 * flicky->flickyID;
                if (sceneInfo->timeEnabled) {
                    flicky->velocity.x = 0;
                    flicky->velocity.y = -(flicky->flickyID << 16);
                    if (globals->tileCollisionMode != 1)
                        flicky->velocity.y = -flicky->velocity.y;
                    flicky->animator.SetAnimation(Animals::sVars->aniFrames, 24, true, 0);
                    flicky->state.Set(&SuperFlicky::State_Drop);
                }
                else {
                    flicky->state.Set(&SuperFlicky::State_FlyAway);
                }
                flicky->superBlendMode  = 4;
                flicky->superBlendTimer = 0;
            }
            sVars->state = 0;
            break;

        default: break;
    }
}
void SuperFlicky::Draw()
{
    color colorStorage[2][32];
    for (int32 c = 0; c < 32; ++c) {
        colorStorage[1][c] = paletteBank[0].GetEntry(c);
        colorStorage[0][c] = paletteBank[1].GetEntry(c);
    }
    HandleSuperColors(true);

    this->animator.DrawSprite(&this->position, false);

    for (int32 c = 0; c < 32; ++c) {
        paletteBank[0].SetEntry(c, colorStorage[1][c]);
        paletteBank[1].SetEntry(c, colorStorage[0][c]);
    }
}

void SuperFlicky::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_NORMAL;
        this->visible       = true;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->drawFX        = FX_FLIP;
        this->startPos.x    = this->position.x;
        this->startPos.y    = this->position.y;
        this->targetSlot    = -1;
        this->state.Set(&SuperFlicky::State_Init);
        this->animator.SetAnimation(Animals::sVars->aniFrames, Animals::SuperFlicky, true, 0);
    }
}

void SuperFlicky::StageLoad()
{
    Animator animator;
    animator.SetAnimation(Animals::sVars->aniFrames, Animals::SuperFlicky, true, 0);

    sVars->hitbox            = *animator.GetHitbox(0);
    sVars->targetPlayerID    = 0;
    sVars->state             = 0;
    sVars->activeFlickyCount = 0;
}

void SuperFlicky::HandleSuperColors(bool32 updatePalette)
{
    if (updatePalette) {
        paletteBank[0].SetEntry(2, Player::sVars->superSonicPalette[this->superPaletteIndex + 0]);
        paletteBank[0].SetEntry(3, Player::sVars->superSonicPalette[this->superPaletteIndex + 1]);
        paletteBank[0].SetEntry(4, Player::sVars->superSonicPalette[this->superPaletteIndex + 2]);
        if (Water::sVars) {
            paletteBank[1].SetEntry(2, Player::sVars->superSonicPalette[this->superPaletteIndex + 0]);
            paletteBank[1].SetEntry(3, Player::sVars->superSonicPalette[this->superPaletteIndex + 1]);
            paletteBank[1].SetEntry(4, Player::sVars->superSonicPalette[this->superPaletteIndex + 2]);
        }
    }
    else {
        this->superBlendTimer++;
        switch (this->superBlendMode) {
            default:
                if (this->superBlendTimer >= 3) {
                    this->superBlendTimer = 0;
                    this->superPaletteIndex -= 3;
                    if (this->superPaletteIndex <= 0) {
                        this->superPaletteIndex = 0;
                        this->superBlendMode    = 0;
                    }
                }
                break;

            case Player::SuperStateSuper:
                if (this->superBlendTimer >= (this->superPaletteIndex >= 18 ? 7 : 2)) {
                    this->superBlendTimer = 0;
                    this->superPaletteIndex += 3;
                    if (this->superPaletteIndex >= 30)
                        this->superPaletteIndex = 18;
                }
                break;
        }
    }
}

void SuperFlicky::HandleAttack()
{
    Vector2 range;
    range.x = 0x200000;
    range.y = 0x200000;

    if (this->attackListPos >= 0) {
        bool32 clear = true;

        if (Zone::sVars->flickyAttackList[this->attackListPos].slotID >= 0) {
            Entity *target = GameObject::Get(Zone::sVars->flickyAttackList[this->attackListPos].slotID);

            if (target->classID == Zone::sVars->flickyAttackList[this->attackListPos].classID) {
                if (target->active) {
                    this->targetPosition.x = Zone::sVars->flickyAttackList[this->attackListPos].position.x >> 16;
                    this->targetPosition.y = Zone::sVars->flickyAttackList[this->attackListPos].position.y >> 16;

                    int32 left = 0, right = 0;
                    if (this->direction & FLIP_X) {
                        left  = abs(Zone::sVars->flickyAttackList[this->attackListPos].hitbox.right);
                        right = Zone::sVars->flickyAttackList[this->attackListPos].hitbox.left;
                    }
                    else {
                        left  = abs(Zone::sVars->flickyAttackList[this->attackListPos].hitbox.left);
                        right = Zone::sVars->flickyAttackList[this->attackListPos].hitbox.right;
                    }

                    this->targetPosition.x += (left - right) / 2;

                    int32 bottom = Zone::sVars->flickyAttackList[this->attackListPos].hitbox.bottom;
                    int32 top    = abs(Zone::sVars->flickyAttackList[this->attackListPos].hitbox.top);
                    this->targetPosition.y += (top - bottom) / 2;

                    if (Zone::sVars->autoScrollSpeed || this->CheckOnScreen(&range)) {
                        this->targetVelocity.x = (this->position.x >> 16) - this->targetPosition.x + 12;
                        if (this->targetVelocity.x < 12)
                            this->targetVelocity.y = (this->position.y >> 16) - this->targetPosition.y + 12;

                        clear = false;
                    }
                }
            }
        }

        if (clear) {
            Zone::sVars->flickyAttackList[this->attackListPos].slotID        = -1;
            Zone::sVars->flickyAttackList[this->attackListPos].classID       = TYPE_NONE;
            Zone::sVars->flickyAttackList[this->attackListPos].isTargeted    = false;
            Zone::sVars->flickyAttackList[this->attackListPos].hitbox.left   = 0;
            Zone::sVars->flickyAttackList[this->attackListPos].hitbox.top    = 0;
            Zone::sVars->flickyAttackList[this->attackListPos].hitbox.right  = 0;
            Zone::sVars->flickyAttackList[this->attackListPos].hitbox.bottom = 0;
            Zone::sVars->flickyAttackList[this->attackListPos].position.x    = 0;
            Zone::sVars->flickyAttackList[this->attackListPos].position.y    = 0;
            Zone::sVars->flickyAttackList[this->attackListPos].timer         = -1;
            this->attackDelay                                                = 120;
            this->attackListPos                                              = -1;
        }
    }
}

void SuperFlicky::State_Init()
{
    SET_CURRENT_STATE();

    this->visible = true;
    this->active  = ACTIVE_NORMAL;
    this->animator.SetAnimation(Animals::sVars->aniFrames, 25, true,
                                this->flickyID - this->flickyID / this->animator.frameCount * this->animator.frameCount);
    this->state.Set(&SuperFlicky::State_Active);
}

void SuperFlicky::State_Active()
{
    SET_CURRENT_STATE();

    Entity *target = GameObject::Get(this->targetSlot);
    if (this->attackListPos == -1) {
        if (!this->attackDelay) {
            for (int32 i = 0; i < 0x80; ++i) {
                if (Zone::sVars->flickyAttackList[i].slotID != -1 && !Zone::sVars->flickyAttackList[i].isTargeted) {
                    this->attackListPos                         = i;
                    Zone::sVars->flickyAttackList[i].isTargeted = true;
                    HandleAttack();
                    return;
                }
            }
        }
        else {
            this->attackDelay--;
        }

        this->targetVelocity.x = Math::Cos256(this->angle) >> 3;
        this->targetVelocity.y = Math::Sin256(this->angle) >> 4;

        this->targetPosition.x = target->position.x >> 16;
        this->targetPosition.y = target->position.y >> 16;

        if (globals->tileCollisionMode == 2)
            this->targetPosition.y += 32;
        else
            this->targetPosition.y -= 32;
        this->targetPosition.x += this->targetVelocity.x;
        this->targetPosition.y += this->targetVelocity.y;
    }
    else {
        HandleAttack();
    }
}

void SuperFlicky::State_Restore()
{
    SET_CURRENT_STATE();

    sVars->state = this->zdepth;

    this->animator.SetAnimation(Animals::sVars->aniFrames, 25, true, 0);
    this->animator.SetAnimation(Animals::sVars->aniFrames, 25, true,
                                this->flickyID - this->flickyID / this->animator.frameCount * this->animator.frameCount);

    this->state = this->stateStored;
    this->state.Run(this);
}

void SuperFlicky::State_Drop()
{
    SET_CURRENT_STATE();

    bool32 collided = false;
    if (globals->tileCollisionMode == 1) {
        this->velocity.y += 0x3800;
        this->position.x += this->velocity.x;
        this->position.y += this->velocity.y;

        int32 height = this->animator.GetFrame(Animals::sVars->aniFrames)->height;
        collided = this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, (((height + (height >> 15)) << 16) >> 1) & 0xFFFF0000, true);
    }
    else {
        this->velocity.y -= 0x3800;
        this->position.x += this->velocity.x;
        this->position.y += this->velocity.y;

        int32 height = this->animator.GetFrame(Animals::sVars->aniFrames)->height;
        collided     = this->TileCollision(Zone::sVars->collisionLayers, CMODE_ROOF, 0, 0, -((height + (height >> 15)) << 16) >> 1, true);
    }

    if (collided) {
        this->velocity.x = 0;
        this->velocity.y = 0;
        this->animator.SetAnimation(Animals::sVars->aniFrames, 25, true, 0);
        this->state.Set(&SuperFlicky::State_FlyAway);
    }

    if (!this->CheckOnScreen(&this->updateRange)) {
        this->isPermanent = false;
        this->Destroy();
    }
}

void SuperFlicky::State_FlyAway()
{
    SET_CURRENT_STATE();

    if (!this->CheckOnScreen(&this->updateRange)) {
        this->isPermanent = false;
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void SuperFlicky::EditorDraw()
{
    this->animator.SetAnimation(Animals::sVars->aniFrames, 25, true, 0);
    this->animator.DrawSprite(&this->position, false);
}

void SuperFlicky::EditorLoad() {}
#endif

#if RETRO_REV0U
void SuperFlicky::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(SuperFlicky); }
#endif

void SuperFlicky::Serialize() {}

} // namespace GameLogic