// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Caterkiller Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Caterkiller.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Caterkiller);

void Caterkiller::Update() { this->state.Run(this); }

void Caterkiller::LateUpdate() {}

void Caterkiller::StaticUpdate() {}

void Caterkiller::Draw() { this->stateDraw.Run(this); }

void Caterkiller::Create(void *data)
{
    this->visible = true;
    if (this->planeFilter > 0 && ((uint8)(this->planeFilter - 1) & 2))
        this->drawGroup = Zone::sVars->objectDrawGroup[1];
    else
        this->drawGroup = Zone::sVars->objectDrawGroup[0];

    this->drawFX |= FX_FLIP;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    if (data) {
        // wtf does this mean
        // self->state = (Type_StateMachine)data

        if (this->state.Matches(&Caterkiller::StateSplit_Head))
            this->headAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        else
            this->headAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

        this->stateDraw.Set(&Caterkiller::Draw_Segment);
    }
    else {
        this->startPos = this->position;
        this->startDir = this->direction;

        this->headOffset = 0;
        int32 offset     = this->startDir ? -0xC0000 : 0xC0000;

        int32 posX = this->position.x;
        for (int32 i = 0; i < CATERKILLER_BODY_COUNT; ++i) {
            posX += offset;
            this->bodyPosition[i].x = posX;
            this->bodyPosition[i].y = this->position.y;
            this->bodyDirection[i]  = this->direction;
        }

        this->timer = 0;
        this->headAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->state.Set(&Caterkiller::State_Init);
        this->stateDraw.Set(&Caterkiller::Draw_Body);
    }
}

void Caterkiller::StageLoad()
{
    sVars->aniFrames.Load("SSZ/Caterkiller.bin", SCOPE_STAGE);

    sVars->hitbox.left   = -8;
    sVars->hitbox.top    = -8;
    sVars->hitbox.right  = 8;
    sVars->hitbox.bottom = 8;

    DebugMode::AddObject(sVars->classID, &Caterkiller::DebugSpawn, &Caterkiller::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Caterkiller::DebugSpawn()
{
    GameObject::Create<Caterkiller>(nullptr, this->position.x, this->position.y);
}

void Caterkiller::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Caterkiller::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
        this->position  = this->startPos;
        this->direction = this->startDir;
        Caterkiller::Create(nullptr);
    }
}

void Caterkiller::CheckTileCollisions()
{
    int32 storeX = 0;
    int32 storeY = 0;

    if (this->state.Matches(&Caterkiller::State_LiftHead)) {
        storeX = this->position.x;
        storeY = this->position.y;
    }
    else {
        if (!this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x80000, 8))
            this->direction ^= FLIP_X;

        storeX = this->position.x;
        storeY = this->position.y;

        if (Caterkiller::CheckTileAngle(this->position.x, this->position.y, this->direction))
            this->direction ^= FLIP_X;
    }

    for (int32 i = 0; i < CATERKILLER_BODY_COUNT; ++i) {
        if (!this->state.Matches(&Caterkiller::State_LowerHead) || i != (CATERKILLER_BODY_COUNT - 1)) {
            this->position.x = this->bodyPosition[i].x;
            this->position.y = this->bodyPosition[i].y;

            if (!this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x80000, 8))
                this->bodyDirection[i] = this->direction;

            this->bodyPosition[i].y = this->position.y;
            if (Caterkiller::CheckTileAngle(this->bodyPosition[i].x, this->bodyPosition[i].y, this->bodyDirection[i]))
                this->bodyDirection[i] = this->direction;
        }
    }

    this->position.x = storeX;
    this->position.y = storeY;
}

void Caterkiller::Draw_Body()
{
    int32 storeDir = this->direction;
    for (int32 i = CATERKILLER_BODY_COUNT - 1; i >= 0; --i) {
        Vector2 drawPos = this->bodyPosition[i];
        drawPos.y -= this->bodyOffset[i] << 15;
        this->direction = this->bodyDirection[i];
        this->bodyAnimator.DrawSprite(&drawPos, false);
    }

    Vector2 drawPos = this->position;
    drawPos.y -= this->headOffset << 15;
    this->direction = storeDir;
    this->headAnimator.DrawSprite(&drawPos, false);
}

void Caterkiller::Draw_Segment() { this->headAnimator.DrawSprite(nullptr, false); }

void Caterkiller::HandlePlayerInteractions()
{
    int32 storeX = this->position.x;
    int32 storeY = this->position.y;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (this->planeFilter <= 0 || player->collisionPlane == ((this->planeFilter - 1) & 1)) {
            if (player->CheckBadnikTouch(this, &sVars->hitbox)) {
                player->CheckBadnikBreak(this, true);
            }
            else {
                for (int32 i = 0; i < CATERKILLER_BODY_COUNT; ++i) {
                    this->position.x = this->bodyPosition[i].x;
                    this->position.y = this->bodyPosition[i].y;

                    if (player->CheckCollisionTouch(this, &sVars->hitbox)) {
                        player->Hurt(this);

                        for (int32 d = 0; d < CATERKILLER_BODY_COUNT + 1; ++d) {
                            int32 spawnX             = storeX;
                            int32 spawnY             = storeY;
                            int32 spawnDir           = this->direction;
                            StateMachine<Caterkiller> spawnState; //= Caterkiller::StateSplit_Head;
                            spawnState.Set(&Caterkiller::StateSplit_Head);

                            if (d) {
                                spawnX     = this->bodyPosition[d - 1].x;
                                spawnY     = this->bodyPosition[d - 1].y;
                                spawnDir   = this->bodyDirection[d - 1];
                                spawnState.Set(&Caterkiller::StateSplit_Body);
                            }

                            Caterkiller *segment = GameObject::Create<Caterkiller>(&spawnState, spawnX, spawnY);
                            segment->direction         = spawnDir;
                            if (!segment->direction)
                                segment->velocity.x = (d & 1) ? -0x18000 : -0x20000;
                            else
                                segment->velocity.x = (d & 1) ? 0x18000 : 0x20000;

                            if ((d & 3) >= 2)
                                segment->velocity.x = -segment->velocity.x;
                            segment->velocity.y = -0x40000;

                            if (!d)
                                segment->headAnimator.frameID = this->headAnimator.frameID;

                            segment->planeFilter = this->planeFilter;
                            segment->drawGroup   = this->drawGroup;
                        }

                        this->Destroy();
                        this->active = ACTIVE_DISABLED;
                        break;
                    }
                }
            }

            this->position.x = storeX;
            this->position.y = storeY;
        }
    }
}

bool32 Caterkiller::CheckTileAngle(int32 x, int32 y, int32 dir)
{
    int32 tx = x >> 16;
    int32 ty = (y >> 16) + 8;

    Tile tile = Zone::sVars->fgLayer[1].GetTile(tx, ty);
    if (tile.id == (uint16)-1)
        tile = Zone::sVars->fgLayer[0].GetTile(tx, ty);

    uint8 angle = tile.GetAngle(0, CMODE_FLOOR);

    if (dir) {
        if (angle > 0x80 && angle < 0xE8)
            return true;
    }
    else {
        if (angle > 0x18 && angle < 0x80)
            return true;
    }

    return false;
}

void Caterkiller::State_Init()
{
    this->active = ACTIVE_NORMAL;

    this->state.Set(&Caterkiller::State_Contract);
    Caterkiller::State_Contract();
}

void Caterkiller::State_Contract()
{
    if (this->timer) {
        this->timer--;
        Caterkiller::HandlePlayerInteractions();
        Caterkiller::CheckOffScreen();
    }
    else {
        this->timer                = 15;
        this->headAnimator.frameID = 1;

        this->state.Set(&Caterkiller::State_LiftHead);
        Caterkiller::State_LiftHead();
    }
}

void Caterkiller::State_LiftHead()
{
    if (this->timer) {
        this->timer--;
        for (int32 b = 0; b < CATERKILLER_BODY_COUNT; ++b) {
            if (this->bodyDirection[b])
                this->bodyPosition[b].x += 0x4000 * (b + 1);
            else
                this->bodyPosition[b].x -= 0x4000 * (b + 1);
        }

        ++this->headOffset;
        ++this->bodyOffset[CATERKILLER_BODY_COUNT / 2];
    }
    else {
        this->timer = 7;
        this->state.Set(&Caterkiller::State_Uncontract);
    }

    Caterkiller::CheckTileCollisions();
    Caterkiller::HandlePlayerInteractions();
    Caterkiller::CheckOffScreen();
}

void Caterkiller::State_Uncontract()
{
    if (this->timer) {
        this->timer--;

        Caterkiller::HandlePlayerInteractions();
        Caterkiller::CheckOffScreen();
    }
    else {
        this->timer                = 15;
        this->headAnimator.frameID = 0;

        this->state.Set(&Caterkiller::State_LowerHead);
        Caterkiller::State_LowerHead();
    }
}

void Caterkiller::State_LowerHead()
{
    if (this->timer) {
        this->timer--;

        int32 mult = 1;
        for (int32 b = CATERKILLER_BODY_COUNT - 2; b >= 0; --b) {
            this->bodyPosition[b].x += this->bodyDirection[b] ? (0x4000 * mult) : (-0x4000 * mult);
            ++mult;
        }

        this->position.x += this->direction ? (0x4000 * mult) : (-0x4000 * mult);

        --this->headOffset;
        --this->bodyOffset[CATERKILLER_BODY_COUNT / 2];
    }
    else {
        this->timer = 7;
        this->state.Set(&Caterkiller::State_Contract);
    }

    Caterkiller::CheckTileCollisions();
    Caterkiller::HandlePlayerInteractions();
    Caterkiller::CheckOffScreen();
}

void Caterkiller::StateSplit_Head()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x3800;

    if (this->CheckOnScreen(&this->updateRange)) {
        if (this->velocity.y > 0 && this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x80000, 4)) {
            this->velocity.y = -0x40000;
        }

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if ((this->planeFilter <= 0 || player->collisionPlane == ((uint8)(this->planeFilter - 1) & 1))
                && player->CheckBadnikTouch(this, &sVars->hitbox)) {
                player->CheckBadnikBreak(this, true);
            }
        }
    }
    else {
        this->Destroy();
    }
}

void Caterkiller::StateSplit_Body()
{
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += 0x3800;

    if (this->CheckOnScreen(&this->updateRange)) {
        if (this->velocity.y > 0 && this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x80000, 4)) {
            this->velocity.y = -0x40000;
        }

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (this->planeFilter <= 0 || player->collisionPlane == ((this->planeFilter - 1) & 1)) {
                if (player->CheckCollisionTouch(this, &sVars->hitbox))
                    player->Hurt(this);
            }
        }
    }
    else {
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void Caterkiller::EditorDraw()
{
    this->startPos = this->position;

    this->headOffset = 0;
    int32 offset     = this->startDir ? -0xC0000 : 0xC0000;

    int32 posX = this->position.x;
    for (int32 i = 0; i < CATERKILLER_BODY_COUNT; ++i) {
        posX += offset;
        this->bodyPosition[i].x = posX;
        this->bodyPosition[i].y = this->position.y;
        this->bodyDirection[i]  = this->direction;
    }

    this->headAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->bodyAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

    Caterkiller::Draw_Body();
}

void Caterkiller::EditorLoad()
{
    sVars->aniFrames.Load("MMZ/Caterkiller.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, planeFilter);
    RSDK_ENUM_VAR("None");
    RSDK_ENUM_VAR("AL");
    RSDK_ENUM_VAR("BL");
    RSDK_ENUM_VAR("AH");
    RSDK_ENUM_VAR("BH");
}
#endif

void Caterkiller::Serialize() { RSDK_EDITABLE_VAR(Caterkiller, VAR_ENUM, planeFilter); }
} // namespace GameLogic