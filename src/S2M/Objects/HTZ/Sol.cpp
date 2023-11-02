// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Sol Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Sol.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"
#include "OOZ/OOZSetup.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Sol);

void Sol::Update()
{
    this->state.Run(this);
}

void Sol::LateUpdate() {}

void Sol::StaticUpdate() {}

void Sol::Draw()
{
    for (int32 i = 0; i < SOL_FLAMEORB_COUNT; ++i) {
        if ((1 << i) & this->activeOrbs)
            this->ballAnimator.DrawSprite(&this->positions[i], false);
    }

    this->mainAnimator.DrawSprite(nullptr, false);
}

void Sol::Create(void *data)
{
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];
    this->drawFX    = FX_FLIP;

    if (data) {
        this->mainAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->active = ACTIVE_NORMAL;
        this->drawFX |= FX_ROTATE;
        this->updateRange.x = 0x1000000;
        this->updateRange.y = 0x1000000;
        this->state.Set(&Sol::State_SmallFireball);
    }
    else {
        this->startPos      = this->position;
        this->startDir      = this->direction;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->activeOrbs    = 2 | 8;

        this->mainAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->ballAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->state.Set(&Sol::State_Init);
        this->velocity.x = this->direction == FLIP_NONE ? -0x4000 : 0x4000;
    }
}

void Sol::StageLoad()
{
    sVars->aniFrames.Load("HTZ/Sol.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left   = -8;
    sVars->hitboxBadnik.top    = -8;
    sVars->hitboxBadnik.right  = 8;
    sVars->hitboxBadnik.bottom = 8;

    sVars->hitboxOrb.left   = -4;
    sVars->hitboxOrb.top    = -4;
    sVars->hitboxOrb.right  = 4;
    sVars->hitboxOrb.bottom = 4;

    DebugMode::AddObject(sVars->classID, &Sol::DebugSpawn, &Sol::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Sol::DebugSpawn()
{
    GameObject::Create<Sol>(nullptr, this->position.x, this->position.y);
}

void Sol::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Sol::HandlePlayerInteractions()
{
    int32 storeX = this->position.x;
    int32 storeY = this->position.y;

    for (int32 i = 0; i < SOL_FLAMEORB_COUNT; ++i) {
        if ((1 << i) & this->activeOrbs) {
            this->position.x = this->positions[i].x;
            this->position.y = this->positions[i].y;

            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
            {
                Sol::HandlePlayerHurt();
            }
        }
    }

    this->position.x = storeX;
    this->position.y = storeY;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik) && player->CheckBadnikBreak(this, false)) {
            int32 angle = this->angle;
            for (int32 i = 0; i < SOL_FLAMEORB_COUNT; ++i) {
                if ((1 << i) & this->activeOrbs) {
                    this->position.x = this->positions[i].x;
                    this->position.y = this->positions[i].y;

                    Sol *sol = GameObject::Create<Sol>(INT_TO_VOID(true), this->positions[i].x, this->positions[i].y);

                    sol->state.Set(&Sol::State_ActiveFireball);
                    sol->velocity.x = 0x380 * Math::Cos256(angle);
                    sol->velocity.y = 0x380 * Math::Sin256(angle);
                }

                angle += (0x100 / SOL_FLAMEORB_COUNT);
            }

            this->Destroy();
        }
    }
}

void Sol::HandlePlayerHurt()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->CheckCollisionTouch(this, &sVars->hitboxOrb)) {
            if (player->shield != Player::Shield_Fire) {
                player->Hurt(this);
            }
        }
    }
}

void Sol::HandleRotation()
{
    int32 angle = this->angle;
    if (this->direction)
        this->angle = (angle - 1) & 0xFF;
    else
        this->angle = (angle + 1) & 0xFF;

    for (int32 i = 0; i < SOL_FLAMEORB_COUNT; ++i) {
        if ((1 << i) & this->activeOrbs) {
            this->positions[i].x = (Math::Cos256(angle) << 12) + this->position.x;
            this->positions[i].y = (Math::Sin256(angle) << 12) + this->position.y;
        }

        angle += (0x100 / SOL_FLAMEORB_COUNT);
    }
}

void Sol::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position  = this->startPos;
        this->direction = this->startDir;
        Sol::Create(nullptr);
    }
}

void Sol::State_Init()
{
    this->active = ACTIVE_NORMAL;

    this->state.Set(&Sol::State_Moving);
    Sol::State_Moving();
}

void Sol::State_Moving()
{
    this->ballAnimator.Process();

    this->position.x += this->velocity.x;
    this->position.y = (Math::Sin256(this->oscillateAngle) << 10) + this->startPos.y;
    this->oscillateAngle += 4;

    Sol::HandleRotation();
    Sol::HandlePlayerInteractions();

    if (this->fireOrbs) {
        Player *playerPtr = nullptr;
        int32 distanceX         = 0x7FFFFFFF;
        int32 distanceY         = 0x7FFFFFFF;

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
        {
            if (abs(player->position.y - this->position.y) < distanceY)
                distanceY = abs(player->position.y - this->position.y);

            if (abs(player->position.y - this->position.y) < 0x400000) {
                if (!playerPtr) {
                    if (abs(player->position.x - this->position.x) < distanceX) {
                        distanceX = abs(player->position.x - this->position.x);
                        playerPtr = player;
                    }
                }
                else {
                    if (abs(player->position.x - this->position.x) < distanceX) {
                        distanceX = abs(player->position.x - this->position.x);
                        playerPtr = player;
                    }
                }
            }
        }

        if (!playerPtr)
            playerPtr = GameObject::Get<Player>(SLOT_PLAYER1);

        if (distanceX <= 0x800000) {
            this->state.Set(&Sol::State_ShootingOrbs);
            this->mainAnimator.frameID = 1;
        }

        this->direction = playerPtr->position.x >= this->position.x;
    }

    Sol::CheckOffScreen();
}

void Sol::State_ShootingOrbs()
{
    this->ballAnimator.Process();

    this->position.x += this->velocity.x;
    this->position.y = (Math::Sin256(this->oscillateAngle) << 10) + this->startPos.y;
    this->oscillateAngle += 4;

    uint8 angle = this->angle;
    Sol::HandleRotation();

    for (int32 i = 0; i < SOL_FLAMEORB_COUNT; ++i) {
        if (angle == 0x40) {
            if ((1 << i) & this->activeOrbs) {
                this->activeOrbs &= ~(1 << i);
                Sol *sol  = GameObject::Create<Sol>(INT_TO_VOID(true), this->positions[i].x, this->positions[i].y);
                sol->velocity.x = this->direction == FLIP_NONE ? -0x20000 : 0x20000;
            }
        }

        angle += (0x100 / SOL_FLAMEORB_COUNT);
    }

    Sol::HandlePlayerInteractions();

    if (!this->activeOrbs) {
        this->state.Set(&Sol::State_NoOrbs);
        this->velocity.x = this->direction == FLIP_NONE ? -0x4000 : 0x4000;
    }

    if (this->mainAnimator.timer >= 0x10)
        this->mainAnimator.frameID = 2;
    else
        this->mainAnimator.timer++;

    Sol::CheckOffScreen();
}

void Sol::State_NoOrbs()
{
    this->ballAnimator.Process();

    this->position.x += this->velocity.x;
    this->position.y = (Math::Sin256(this->oscillateAngle) << 10) + this->startPos.y;
    this->oscillateAngle += 4;

    Sol::HandleRotation();
    Sol::HandlePlayerInteractions();
    Sol::CheckOffScreen();
}

void Sol::State_SmallFireball()
{
    this->mainAnimator.Process();

    this->position.x += this->velocity.x;

    if (this->CheckOnScreen(&this->updateRange))
        Sol::HandlePlayerHurt();
    else
        this->Destroy();
}

void Sol::State_ActiveFireball()
{
    if (this->CheckOnScreen(&this->updateRange)) {
        this->position.x += this->velocity.x;
        this->position.y += this->velocity.y;
        this->velocity.y += 0x3800;

        this->rotation = 2 * Math::ATan2(this->velocity.x >> 16, this->velocity.y >> 16) + 384;

        int32 offsetX = Math::Sin512(512 - this->rotation) << 10;
        int32 offsetY = Math::Cos512(512 - this->rotation) << 10;
        int32 cmode   = 3 - (((this->rotation - 0x40) >> 7) & 3);

        bool32 collided = this->TileCollision(Zone::sVars->collisionLayers, cmode, 1, offsetX, offsetY, true);
        if (!collided)
            collided = this->TileCollision(Zone::sVars->collisionLayers, cmode, 0, offsetX, offsetY, true);

        if (collided) {
            this->mainAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
            this->state.Set(&Sol::State_FlameDissipate);
            this->rotation = (this->rotation + 64) & 0x180;
        }

        int32 spawnX = this->position.x + offsetX;
        int32 spawnY = this->position.y + offsetY;
        Tile tile  = Zone::sVars->fgLayer[1].GetTile(spawnX >> 20, (spawnY - 0x10000) >> 20);
        if (tile.id == (uint16)-1)
            tile = Zone::sVars->fgLayer[0].GetTile(spawnX >> 20, (spawnY - 0x10000) >> 20);

        int32 tileFlags = tile.GetFlags(0);
        if (((tileFlags == OOZSetup::OOZ_TFLAGS_OILSTRIP || tileFlags == OOZSetup::OOZ_TFLAGS_OILSLIDE) && collided) || tileFlags == OOZSetup::OOZ_TFLAGS_OILPOOL) {
            this->position.x = spawnX - 0x40000;
            this->position.y = spawnY - 0x80000;
            this->rotation   = 0;
            this->velocity.x = -0x40000;
            this->velocity.y = 0;
            this->mainAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
            this->state.Set(&Sol::State_FireballOilFlame);
        
            Sol *sol  = GameObject::Create<Sol>(INT_TO_VOID(true), spawnX, spawnY - 0x80000);
            sol->velocity.x = 0x40000;
            sol->velocity.y = 0;
            sol->mainAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
            sol->state.Set(&Sol::State_FireballOilFlame);
            sol->oscillateAngle = sol->position.x & 0xF00000;
        
            if (tileFlags == OOZSetup::OOZ_TFLAGS_OILPOOL) {
                this->position.y = (this->position.y & 0xFFF00000) + 0x20000;
                sol->position.y  = (sol->position.y & 0xFFF00000) + 0x20000;
                sol->state.Set(&Sol::State_OilFlame);
                this->state.Set(&Sol::State_OilFlame);
            }
            else {
                this->position.y -= 0x80000;
            }
        }

        if (this->interaction)
            Sol::HandlePlayerHurt();
    }
    else {
        this->Destroy();
    }
}

void Sol::State_FlameDissipate()
{
    this->mainAnimator.Process();

    if (!this->CheckOnScreen(&this->updateRange) || this->mainAnimator.frameID == this->mainAnimator.frameCount - 1) {
        this->Destroy();
    }
}

void Sol::State_FireballOilFlame()
{
    OOZSetup *setup = GameObject::Get<OOZSetup>(sceneInfo->entitySlot);

    if (this->CheckOnScreen(&this->updateRange)) {
        bool32 collided = this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 1, 0, 0x80000, 16);
        if (!collided)
            collided = this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x80000, 16);

        if (collided) {
            Tile tile = Zone::sVars->fgLayer[1].GetTile(this->position.x >> 20, (this->position.y + 0x90000) >> 20);
            if (tile.id == (uint16)-1)
                tile = Zone::sVars->fgLayer[0].GetTile(this->position.x >> 20, (this->position.y + 0x90000) >> 20);

            this->rotation = 2 * tile.GetAngle(0, 0);
        }

        Tile tile = Zone::sVars->fgLayer[1].GetTile(this->position.x >> 20, (this->position.y + 0x70000) >> 20);
        if (tile.id == (uint16)-1)
            tile = Zone::sVars->fgLayer[0].GetTile(this->position.x >> 20, (this->position.y + 0x70000) >> 20);

        int32 tileFlags = tile.GetFlags(0);
        if (tileFlags == OOZSetup::OOZ_TFLAGS_NORMAL || tileFlags == OOZSetup::OOZ_TFLAGS_OILFALL) {
            if (collided) {
                this->mainAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
                this->state.Set(&Sol::State_FlameDissipate);
            }
            else {
                this->state.Set(&Sol::State_ActiveFireball);
            }
        }
        else {
            this->position.y -= 0x80000;
            if ((this->position.x & 0xF00000) != this->oscillateAngle)
                setup->StartFire((this->position.x & 0xFFF00000) + 0x70000, this->position.y & 0xFFFF0000, this->rotation >> 1);
        
            this->oscillateAngle = this->position.x & 0xF00000;
        }

        this->position.x += this->velocity.x;
        this->position.y += 0x80000;

        this->mainAnimator.Process();

        Sol::HandlePlayerHurt();
    }
    else {
        this->Destroy();
    }
}

void Sol::State_OilFlame()
{
    OOZSetup *setup = GameObject::Get<OOZSetup>(sceneInfo->entitySlot);

    if (this->CheckOnScreen(&this->updateRange)) {
        Tile tile = Zone::sVars->fgLayer[1].GetTile(this->position.x >> 20, (this->position.y + 0xF0000) >> 20);
        if (tile.id == (uint16)-1)
            tile = Zone::sVars->fgLayer[0].GetTile(this->position.x >> 20, (this->position.y + 0xF0000) >> 20);

        if (tile.GetFlags(0) == OOZSetup::OOZ_TFLAGS_OILPOOL) {
            if ((this->position.x & 0xF00000) != this->oscillateAngle)
                setup->StartFire((this->position.x & 0xFFF00000) + 0x70000, this->position.y & 0xFFFF0000, this->rotation >> 1);
        
            this->oscillateAngle = this->position.x & 0xF00000;
        }
        else {
            this->mainAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
            this->state.Set(&Sol::State_FlameDissipate);
        }

        this->position.x += this->velocity.x;

        this->mainAnimator.Process();

        Sol::HandlePlayerHurt();
    }
    else {
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void Sol::EditorDraw()
{
    int32 angle = this->angle;
    Sol::HandleRotation();
    this->angle = angle;

    Sol::Draw();
}

void Sol::EditorLoad()
{
    sVars->aniFrames.Load("HTZ/Sol.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Right");
}
#endif

void Sol::Serialize()
{
    RSDK_EDITABLE_VAR(Sol, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(Sol, VAR_BOOL, fireOrbs);
}
} // namespace GameLogic