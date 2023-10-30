// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: PetalPile Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "PetalPile.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/Explosion.hpp"
#include "Global/DebugMode.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(PetalPile);

void PetalPile::Update()
{
    this->state.Run(this);

    if (!this->animator.animationID)
        this->animator.Process();
}

void PetalPile::LateUpdate() {}

void PetalPile::StaticUpdate() {}

void PetalPile::Draw() { this->stateDraw.Run(this); }

void PetalPile::Create(void *data)
{
    this->active        = ACTIVE_BOUNDS;
    this->visible       = true;
    this->drawGroup     = Zone::sVars->objectDrawGroup[1];
    this->drawFX        = FX_FLIP;
    this->updateRange.x = 0x100000;
    this->updateRange.y = 0x100000;

    if (sceneInfo->inEditor) {
        if (!this->maxSpeed.x)
            this->maxSpeed.x = 0x18000;

        if (!this->pileSize.x && !this->pileSize.y) {
            this->pileSize.x = 0x300000;
            this->pileSize.y = 0x80000;
        }
    }
    else {
        if (this->tileLayer)
            this->layerID = RSDKTable->GetTileLayerID("FG Low");
        else
            this->layerID = RSDKTable->GetTileLayerID("FG High");
    }

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->state.Set(&PetalPile::State_Init);
}

void PetalPile::StageLoad()
{
    sVars->aniFrames.Load("SWZ/Petal.bin", SCOPE_STAGE);

    sVars->sfxPetals.Get("SWZ/Petals.wav");
}

int32 PetalPile::GetLeafPattern(Vector2 *patternPtr)
{
    int32 count    = sVars->patternSize[this->leafPattern];
    int32 *pattern = NULL;
    switch (this->leafPattern) {
        case PETALPILE_PATTERN_0: pattern = sVars->pattern1; break;
        case PETALPILE_PATTERN_1: pattern = sVars->pattern2; break;
        case PETALPILE_PATTERN_2: pattern = sVars->pattern3; break;
        case PETALPILE_PATTERN_3: pattern = sVars->pattern4; break;
        case PETALPILE_PATTERN_4: pattern = sVars->pattern5; break;
        default: return 0;
    }

    int32 sizeX = MAX(this->pileSize.x, 0x20000);
    int32 sizeY = MAX(this->pileSize.y, 0x20000);
    for (int32 i = 0; i < count; ++i) {
        patternPtr[i].x = pattern[(i * 2) + 0] * (sizeX >> 17);
        patternPtr[i].y = pattern[(i * 2) + 1] * (sizeY >> 17);
    }

    return count;
}

void PetalPile::State_Init()
{
    this->hitbox.left   = -(this->pileSize.x >> 17);
    this->hitbox.top    = -(this->pileSize.y >> 17);
    this->hitbox.right  = this->pileSize.x >> 17;
    this->hitbox.bottom = this->pileSize.y >> 17;

    if (this->noRemoveTiles) {
        this->state.Set(&PetalPile::State_SetupEmitter);
        PetalPile::State_SetupEmitter();
    }
    else {
        this->state.Set(&PetalPile::State_HandleInteractions);
        PetalPile::State_HandleInteractions();
    }
}

void PetalPile::State_HandleInteractions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckCollisionTouch(this, &this->hitbox)) {
            if (!player->sidekick) {
                if (abs(player->groundVel) >= 0x60000 || player->velocity.y > 0x60000 || player->velocity.y < -0x70000
                    || player->state.Matches(&Player::State_Spindash) || player->state.Matches(&Player::State_DropDash)) {
                    bool32 isFast = false;
                    if (player->state.Matches(&Player::State_Spindash) || player->state.Matches(&Player::State_DropDash) || abs(player->groundVel) >= 0x60000)
                        isFast = true;

                    this->petalDir = isFast * (2 * (player->direction != FLIP_NONE) - 1);

                    if (player->state.Matches(&Player::State_Spindash) || player->state.Matches(&Player::State_DropDash))
                        this->petalRadius = 0xF5555;
                    else
                        this->petalRadius = 0xB5555;

                    this->petalVel   = player->groundVel >> 1;
                    this->distance.x = player->position.x - this->position.x;
                    this->distance.y = 0;
                    sVars->sfxPetals.Play(false, 255);
                    this->state.Set(&PetalPile::State_SetupEmitter);

                    break;
                }
            }

            if (!this->emitterMode && abs(player->groundVel) < 0x60000 && abs(player->groundVel) > abs(this->maxSpeed.x))
                player->groundVel =
                    player->groundVel - ((abs(player->groundVel) - abs(this->maxSpeed.x)) >> 1) * (((player->groundVel >> 31) & -2) + 1);
        }
    }

    if (!this->state.Matches(&PetalPile::State_SetupEmitter)) {
        Hitbox hitbox;
        hitbox.left   = -8;
        hitbox.top    = -8;
        hitbox.right  = 8;
        hitbox.bottom = 8;

        for (auto explosion : GameObject::GetEntities<Explosion>(FOR_ACTIVE_ENTITIES))
        {
            if (this->CheckCollisionTouchBox(&this->hitbox, explosion, &hitbox)) {
                this->petalDir    = 0;
                this->petalRadius = 0xF5555;
                this->distance.x  = explosion->position.x - this->position.x;
                this->distance.y  = 0;
                sVars->sfxPetals.Play(false, 255);
                this->state.Set(&PetalPile::State_SetupEmitter);

                break;
            }
        }
    }
}

void PetalPile::State_SetupEmitter()
{
    Vector2 pattern[0x100];
    memset(pattern, 0, sizeof(pattern));
    int32 count = PetalPile::GetLeafPattern(pattern);

    int32 offsetX = 0, offsetY = 0;
    switch (this->petalDir) {
        case -1:
            offsetX = this->position.x + ((this->hitbox.right + 16) << 16);
            offsetY = this->position.y + ((this->hitbox.bottom + 32) << 16);
            break;

        case 0:
            offsetX = this->position.x + this->distance.x;
            offsetY = this->position.y + ((this->hitbox.bottom + 32) << 16);
            break;

        case 1:
            offsetX = this->position.x + ((this->hitbox.left - 16) << 16);
            offsetY = this->position.y + ((this->hitbox.bottom + 32) << 16);
            break;
    }

    int32 pos = 0;
    for (int32 i = 0; i < count; ++i) pos = MAX(pos, abs((this->position.x - offsetX) + pattern[i].x));

    for (int32 i = 0; i < count; ++i) {
        int32 spawnX = pattern[i].x + this->position.x;
        int32 spawnY = pattern[i].y + this->position.y;

        int32 angle            = Math::ATan2(spawnX - offsetX, spawnY - offsetY);
        PetalPile *petal       = GameObject::Create<PetalPile>(this, spawnX, spawnY);
        petal->state.Set(&PetalPile::StateLeaf_Setup);
        petal->stateDraw.Set(&PetalPile::Draw_Leaf);

        int32 radius = this->petalRadius >> 1;
        if (this->petalDir) {
            petal->direction = this->petalDir <= 0;
            petal->petalVel  = this->petalVel;
            radius           = (this->petalRadius >> 8) * ((16 * (abs(spawnX - offsetX) / (pos >> 16))) >> 12);
            petal->timer     = (pos - abs(spawnX - offsetX)) >> 18;
        }
        else {
            petal->direction = ZONE_RAND(FLIP_NONE, FLIP_X);
        }

        petal->velStore.x = (radius >> 8) * Math::Cos256(angle);
        petal->velStore.y = (radius >> 9) * Math::Sin256(angle) - 0x20000;
        petal->drawGroup  = this->tileLayer ? Zone::sVars->objectDrawGroup[0] : Zone::sVars->objectDrawGroup[1];
    }

    if (this->emitterMode) {
        this->state.Set(&PetalPile::State_Emitter);
    }
    else {
        if (!this->noRemoveTiles) {
            int32 left   = (this->position.x >> 16) + this->hitbox.left;
            int32 right  = (this->position.x >> 16) + this->hitbox.right;
            int32 top    = (this->position.y >> 16) + this->hitbox.top;
            int32 bottom = (this->position.y >> 16) + this->hitbox.bottom;

            int32 sizeX = (right >> 4) - (left >> 4);
            int32 sizeY = (bottom >> 4) - (top >> 4);
            for (int32 x = 0; x <= sizeX; ++x) {
                for (int32 y = 0; y <= sizeY; ++y) {
                    RSDKTable->SetTile(this->layerID, x + (left >> 4), y + (top >> 4), -1);
                }
            }
        }

        this->Destroy();
    }
}

void PetalPile::State_Emitter()
{
    bool32 collided = false;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckCollisionTouch(this, &this->hitbox))
            collided = true;
    }

    if (!collided)
        this->state.Set(&PetalPile::State_Init);
}

void PetalPile::StateLeaf_Setup()
{
    this->hitbox.left   = -1;
    this->hitbox.top    = -1;
    this->hitbox.right  = 1;
    this->hitbox.bottom = 1;

    this->active        = ACTIVE_NORMAL;
    this->updateRange.x = 0x10000;
    this->updateRange.y = 0x10000;
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    this->state.Set(&PetalPile::StateLeaf_Delay);
    PetalPile::StateLeaf_Delay();
}

void PetalPile::StateLeaf_Delay()
{
    if (this->timer <= 0) {
        this->state.Set(&PetalPile::StateLeaf_HandleVelocity);
        this->timer    = 0;
        this->velocity = this->velStore;
    }
    else {
        this->timer--;
    }

    if (!this->CheckOnScreen(&this->updateRange))
        this->Destroy();
}

void PetalPile::StateLeaf_HandleVelocity()
{
    if (this->petalVel > 0) {
        this->petalVel -= 4096;
        if (this->petalVel < 0)
            this->petalVel = 0;
    }
    else if (this->petalVel < 0) {
        this->petalVel += 0x1000;
        if (this->petalVel > 0)
            this->petalVel = 0;
    }

    this->position.x += this->petalVel;

    if (this->velocity.x <= 0)
        this->velocity.x += MIN(abs(this->velocity.x), 0x8000);
    else
        this->velocity.x -= MIN(abs(this->velocity.x), 0x8000);
    this->velocity.y += 0x4000;

    if (this->velocity.y > 0)
        this->velocity.y = 0;

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    if (++this->timer > 2) {
        if (ZONE_RAND(0, 10) > 6)
            this->direction = this->direction == FLIP_NONE;

        this->timer = 0;
    }

    if (this->velocity.y >= 0 && !this->velocity.x) {
        this->petalOffset = ZONE_RAND(0, 255);
        this->state.Set(&PetalPile::StateLeaf_Fall);
    }

    if (!this->CheckOnScreen(&this->updateRange))
        this->Destroy();
}

void PetalPile::StateLeaf_Fall()
{
    if (this->petalVel > 0) {
        this->petalVel -= 4096;
        if (this->petalVel < 0)
            this->petalVel = 0;
    }
    else if (this->petalVel < 0) {
        this->petalVel += 0x1000;
        if (this->petalVel > 0)
            this->petalVel = 0;
    }

    this->position.x += this->petalVel;

    this->velocity.y += 0x4000;
    if (this->velocity.y > 0x10000)
        this->velocity.y = 0x10000;

    this->velocity.x = Math::Sin256(4 * this->petalOffset) << 8;

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    ++this->timer;
    if (this->timer > 3) {
        if (ZONE_RAND(0, 10) > 6)
            this->direction = this->direction == FLIP_NONE;

        this->timer = 0;
    }

    if (!this->CheckOnScreen(&this->updateRange))
        this->Destroy();

    this->petalOffset++;
}

void PetalPile::Draw_Leaf() { this->animator.DrawSprite(nullptr, false); }

#if RETRO_REV0U
void PetalPile::StaticLoad(Static *sVars)
{ 
    RSDK_INIT_STATIC_VARS(PetalPile); 

    int32 patternSize[] = { 7, 6, 6, 10, 3 };
    memcpy(sVars->patternSize, patternSize, sizeof(patternSize));

    int32 pattern1[] = { -0x10000, 0x10000, -0x8000, -0x10000, -0x5555, 0x10000, 0, -0x10000, 0x5555, 0x10000, 0x8000, -0x10000, 0x10000, 0x10000 };
    memcpy(sVars->pattern1, pattern1, sizeof(pattern1));

    int32 pattern2[] = { -0x10000, 0x10000, -0x5555, -0x5555, 0x10000, -0x10000, 0, 0xAAAA, 0xAAAA, 0, 0x10000, 0x10000 };
    memcpy(sVars->pattern2, pattern2, sizeof(pattern2));

    int32 pattern3[] = { 0x10000, 0x10000, 0x5555, -0x5555, -0x10000, -0x10000, 0, 0xAAAA, -0xAAAA, 0, -0x10000, 0x10000 };
    memcpy(sVars->pattern3, pattern3, sizeof(pattern3));

    int32 pattern4[] = { -0x10000, 0x10000,  -0x8000, -0x10000, -0x5555, 0x10000, 0, -0x10000, 0x5555, 0x10000, 0x8000, -0x10000, 0x10000, 0x10000, -0xAAAA, 0, 0, 0, 0xAAAA, 0 };
    memcpy(sVars->pattern4, pattern4, sizeof(pattern4));

    int32 pattern5[] = { 0, -0x10000, -0x10000, 0x10000, 0x10000, 0x10000 };
    memcpy(sVars->pattern5, pattern5, sizeof(pattern5));
}
#endif

#if RETRO_INCLUDE_EDITOR
void PetalPile::EditorDraw()
{
    DrawHelpers::DrawRectOutline(this->position.x, this->position.y, this->pileSize.x, this->pileSize.y, 0xFFFF00);
}

void PetalPile::EditorLoad()
{
    sVars->aniFrames.Load("SWZ/Petal.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, leafPattern);
    RSDK_ENUM_VAR("Pattern 0", PETALPILE_PATTERN_0);
    RSDK_ENUM_VAR("Pattern 1", PETALPILE_PATTERN_1);
    RSDK_ENUM_VAR("Pattern 2", PETALPILE_PATTERN_2);
    RSDK_ENUM_VAR("Pattern 3", PETALPILE_PATTERN_3);
    RSDK_ENUM_VAR("Pattern 4", PETALPILE_PATTERN_4);

    RSDK_ACTIVE_VAR(sVars, tileLayer);
    RSDK_ENUM_VAR("FG High", PETALPILE_FGHIGH);
    RSDK_ENUM_VAR("FG Low", PETALPILE_FGLOW);
}
#endif

void PetalPile::Serialize()
{
    RSDK_EDITABLE_VAR(PetalPile, VAR_INT8, leafPattern);
    RSDK_EDITABLE_VAR(PetalPile, VAR_INT8, tileLayer);
    RSDK_EDITABLE_VAR(PetalPile, VAR_VECTOR2, pileSize);
    RSDK_EDITABLE_VAR(PetalPile, VAR_VECTOR2, maxSpeed);
    RSDK_EDITABLE_VAR(PetalPile, VAR_BOOL, emitterMode);
}

} // namespace GameLogic