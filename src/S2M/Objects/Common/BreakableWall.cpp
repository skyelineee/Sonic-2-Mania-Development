// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: BreakableWall Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "BreakableWall.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Common/ScreenWrap.hpp"
#include "Global/Shield.hpp"
#include "Global/ScoreBonus.hpp"
#include "ForceSpin.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(BreakableWall);

void BreakableWall::Update() { this->state.Run(this); }
void BreakableWall::LateUpdate() {}
void BreakableWall::StaticUpdate()
{
    if (!sVars->hasSetupConfig && Zone::sVars->timer > 1) {
        sVars->disableScoreBonus = false;
        sVars->field_138         = true;
        sVars->breakMode         = 0;
        sVars->hasSetupConfig    = true;
    }
}
void BreakableWall::Draw()
{
    this->stateDraw.Run(this);

    if (this->useLayerDrawGroup) {
        int32 startX = this->startPos.x;
        int32 startY = this->startPos.y;

        if (ScreenWrap::sVars) {
            if (ScreenWrap::sVars->activeVWrap)
                this->startPos.y += ScreenWrap::sVars->activeVWrap->buffer.x;
        }

        Vector2 drawPos;
        drawPos.y = this->startPos.y - (this->size.y << 19) + 0x80000;
        for (int32 y = 0; y < this->size.y; ++y) {
            drawPos.x = startX - (this->size.x << 19) + 0x80000;
            for (int32 x = 0; x < this->size.x; ++x) {
                int32 tileX = drawPos.x >> 20;
                int32 tileY = drawPos.y >> 20;

                if (ScreenWrap::sVars) {
                    if (ScreenWrap::sVars->activeVWrap)
                        this->startPos.y += (drawPos.y - ScreenWrap::sVars->activeVWrap->buffer.x) >> 20;
                }

                sVars->animator.DrawSprite(&drawPos, false);

                RSDK::Tile tile = this->targetLayer.GetTile(tileX, tileY);
                Graphics::DrawTile(&tile, 1, 1, &drawPos, nullptr, false);

                drawPos.x += 0x100000;
            }
            drawPos.y += 0x100000;
        }

        this->startPos.x = startX;
        this->startPos.y = startY;
    }
}

void BreakableWall::Create(void *data)
{
    this->startPos.x      = this->position.x;
    this->startPos.y      = this->position.y;
    this->gravityStrength = 0x3800;

    if (sceneInfo->inEditor) {
        this->visible = true;
        this->drawFX |= FLIP_X;
        this->drawGroup = Zone::sVars->objectDrawGroup[1];
    }
    else if (data) {
        int32 type          = VOID_TO_INT(data);
        this->visible       = true;
        this->updateRange.x = 0x100000;
        this->updateRange.y = 0x100000;
        if (this->velocity.x)
            this->drawFX = FX_ROTATE | FX_FLIP;
        else
            this->drawFX = FX_FLIP;
        this->active       = ACTIVE_NORMAL;
        this->tileRotation = Math::Rand(-8, 8);

        switch (type) {
            case BreakableWall::TileFixed:
                this->state.Set(&BreakableWall::State_Piece);
                this->stateDraw.Set(&BreakableWall::State_DrawPiece);
                break;

            default:
            case BreakableWall::TileDynamic:
                this->updateRange.x *= 4;
                this->updateRange.y *= 4;
                this->state.Set(&BreakableWall::State_PieceWait);
                this->stateDraw.Set(nullptr);
                break;
        }
    }
    else {
        this->drawFX |= FX_FLIP;

        if (!sceneInfo->inEditor) {
            this->visible         = false;
            this->targetDrawGroup = Zone::sVars->objectDrawGroup[1];
            this->active          = ACTIVE_BOUNDS;
            this->updateRange.x   = 0x800000;
            this->updateRange.y   = 0x800000;

            if (ScreenWrap::sVars) {
                ScreenWrap *wrap = ScreenWrap::sVars->activeVWrap;
                if (wrap) {
                    if (this->position.y <= wrap->buffer.y + (wrap->buffer.y >> 1))
                        this->useLayerDrawGroup = true;
                }
            }

            if (this->targetLayer.id) {
                this->targetLayer     = Zone::sVars->fgLayer[1];
                this->targetDrawGroup = Zone::sVars->objectDrawGroup[1];
            }
            else {
                this->targetLayer     = Zone::sVars->fgLayer[0];
                this->targetDrawGroup = Zone::sVars->objectDrawGroup[0];
            }

            if (this->useLayerDrawGroup) {
                this->drawGroup = this->targetLayer.GetTileLayer()->drawGroup[0];
            }
            else {
                this->drawGroup = this->targetDrawGroup;
            }

            this->size.x >>= 0x10;
            this->size.y >>= 0x10;

            switch (this->type) {
                case BreakableWall::Wall:
                    if (!this->size.x) {
                        this->size.x = 2;
                        this->size.y = 4;
                    }

                    this->state.Set(&BreakableWall::State_Wall);
                    this->stateDraw.Set(&BreakableWall::State_DrawWall);
                    break;

                case BreakableWall::Floor:
                    if (!this->size.x) {
                        this->size.x = 2;
                        this->size.y = 2;
                    }

                    this->state.Set(&BreakableWall::State_Floor);
                    this->stateDraw.Set(&BreakableWall::State_DrawFloor);
                    break;

                case BreakableWall::BurrowFloor:
                case BreakableWall::BurrowFloor2:
                    if (!this->size.x)
                        this->size.x = 2;

                    this->state.Set(&BreakableWall::State_BurrowFloor);
                    this->stateDraw.Set(&BreakableWall::State_DrawFloor);
                    break;

                case BreakableWall::BurrowFloorUp:
                    if (!this->size.x)
                        this->size.x = 2;

                    this->state.Set(&BreakableWall::State_BurrowFloorUp);
                    this->stateDraw.Set(&BreakableWall::State_DrawFloor);
                    break;

                case BreakableWall::Ceiling:
                    if (!this->size.x) {
                        this->size.x = 2;
                        this->size.y = 2;
                    }

                    this->state.Set(&BreakableWall::State_Ceiling);
                    this->stateDraw.Set(&BreakableWall::State_DrawFloor);
                    break;

                default: break;
            }

            this->hitbox.left   = -(8 * this->size.x);
            this->hitbox.top    = this->topOffset - (8 * this->size.y);
            this->hitbox.right  = 8 * this->size.x;
            this->hitbox.bottom = 8 * this->size.y;
        }
    }
}

void BreakableWall::StageLoad()
{
    sVars->aniFrames.Load("Global/TicMark.bin", SCOPE_STAGE);
    sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    sVars->sfxLedgeBreak.Get("Stage/LedgeBreak.wav");

    sVars->farPlaneLayer.Get("Far Plane");
    sVars->scratchLayer.Get("Scratch");

    sVars->hasSetupConfig = false;
}

// States
void BreakableWall::State_PieceWait()
{
    SET_CURRENT_STATE();

    if (--this->timer <= 0) {
        this->targetLayer.SetTile(this->tilePos.x, this->tilePos.y, -1);

        if (this->drawGroup < Zone::sVars->objectDrawGroup[0] && sVars->farPlaneLayer.Loaded()) {
            sVars->farPlaneLayer.SetTile(this->tilePos.x, this->tilePos.y, -1);
        }

        this->state.Set(&BreakableWall::State_Piece);
        this->stateDraw.Set(&BreakableWall::State_DrawPiece);
    }
}
void BreakableWall::State_Piece()
{
    SET_CURRENT_STATE();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += this->gravityStrength;

    if (this->velocity.x)
        this->rotation += this->tileRotation;

    if (this->drawGroup >= Zone::sVars->objectDrawGroup[0]) {
        if (!this->CheckOnScreen(&this->updateRange))
            this->Destroy();
    }
    else {
        if (++this->timer == 120)
            this->Destroy();
    }
}

void BreakableWall::State_Wall()
{
    SET_CURRENT_STATE();

    this->visible = DebugMode::sVars->debugActive | this->useLayerDrawGroup;

    CheckBreak_Wall();
}
void BreakableWall::State_Floor()
{
    SET_CURRENT_STATE();

    this->visible = DebugMode::sVars->debugActive;

    CheckBreak_Floor();
}
void BreakableWall::State_BurrowFloor()
{
    SET_CURRENT_STATE();

    this->visible = DebugMode::sVars->debugActive;

    CheckBreak_BurrowFloor();
}
void BreakableWall::State_BurrowFloorUp()
{
    SET_CURRENT_STATE();

    this->visible = DebugMode::sVars->debugActive;

    CheckBreak_BurrowFloorUp();
}
void BreakableWall::State_Ceiling()
{
    SET_CURRENT_STATE();

    this->visible = DebugMode::sVars->debugActive;

    CheckBreak_Ceiling();
}

// Draw States
void BreakableWall::State_DrawWall()
{
    SET_CURRENT_STATE();

    if (!this->useLayerDrawGroup || DebugMode::sVars->debugActive) {
        Vector2 drawPos;
        drawPos.x = this->position.x - (this->size.x << 19);
        drawPos.y = this->position.y - (this->size.y << 19);

        Graphics::DrawLine(drawPos.x - 0x10000, drawPos.y - 0x10000, drawPos.x + (this->size.x << 20), drawPos.y - 0x10000, 0xE0E0E0, 0x00, INK_NONE,
                           false);
        Graphics::DrawLine(drawPos.x - 0x10000, drawPos.y + (this->size.y << 20), drawPos.x + (this->size.x << 20), drawPos.y + (this->size.y << 20),
                           0xE0E0E0, 0x00, INK_NONE, false);
        Graphics::DrawLine(drawPos.x - 0x10000, drawPos.y - 0x10000, drawPos.x - 0x10000, drawPos.y + (this->size.y << 20), 0xE0E0E0, 0x00, INK_NONE,
                           false);
        Graphics::DrawLine(drawPos.x + (this->size.x << 20), drawPos.y - 0x10000, drawPos.x + (this->size.x << 20), drawPos.y + (this->size.y << 20),
                           0xE0E0E0, 0x00, INK_NONE, false);

        int32 storeDir  = this->direction;
        this->direction = FLIP_NONE;
        sVars->animator.DrawSprite(&drawPos, false);

        drawPos.x += this->size.x << 20;
        this->direction = FLIP_X;
        sVars->animator.DrawSprite(&drawPos, false);

        drawPos.y += this->size.y << 20;
        this->direction = FLIP_XY;
        sVars->animator.DrawSprite(&drawPos, false);

        drawPos.x -= this->size.x << 20;
        this->direction = FLIP_Y;
        sVars->animator.DrawSprite(&drawPos, false);

        this->direction = storeDir;
    }
}
// The... same function as above?
void BreakableWall::State_DrawFloor()
{
    SET_CURRENT_STATE();

    if (!this->useLayerDrawGroup || DebugMode::sVars->debugActive) {
        Vector2 drawPos;
        drawPos.x = this->position.x - (this->size.x << 19);
        drawPos.y = this->position.y - (this->size.y << 19);

        Graphics::DrawLine(drawPos.x - 0x10000, drawPos.y - 0x10000, drawPos.x + (this->size.x << 20), drawPos.y - 0x10000, 0xE0E0E0, 0x00, INK_NONE,
                           false);
        Graphics::DrawLine(drawPos.x - 0x10000, drawPos.y + (this->size.y << 20), drawPos.x + (this->size.x << 20), drawPos.y + (this->size.y << 20),
                           0xE0E0E0, 0x00, INK_NONE, false);
        Graphics::DrawLine(drawPos.x - 0x10000, drawPos.y - 0x10000, drawPos.x - 0x10000, drawPos.y + (this->size.y << 20), 0xE0E0E0, 0x00, INK_NONE,
                           false);
        Graphics::DrawLine(drawPos.x + (this->size.x << 20), drawPos.y - 0x10000, drawPos.x + (this->size.x << 20), drawPos.y + (this->size.y << 20),
                           0xE0E0E0, 0x00, INK_NONE, false);

        int32 storeDir  = this->direction;
        this->direction = FLIP_NONE;
        sVars->animator.DrawSprite(&drawPos, false);

        drawPos.x += this->size.x << 20;
        this->direction = FLIP_X;
        sVars->animator.DrawSprite(&drawPos, false);

        drawPos.y += this->size.y << 20;
        this->direction = FLIP_XY;
        sVars->animator.DrawSprite(&drawPos, false);

        drawPos.x -= this->size.x << 20;
        this->direction = FLIP_Y;
        sVars->animator.DrawSprite(&drawPos, false);

        this->direction = storeDir;
    }
}
void BreakableWall::State_DrawPiece()
{
    SET_CURRENT_STATE();

    this->angle = this->rotation;
    Graphics::DrawTile(&this->tileInfo, 1, 1, nullptr, nullptr, false);
}

// Breaking
void BreakableWall::CheckBreak_Wall()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (!this->onlyKnux || player->characterID == ID_KNUCKLES) {
                bool32 canBreak = abs(player->groundVel) >= 0x48000 && player->onGround && player->animator.animationID == Player::ANI_JUMP;

                if (player->shield == Shield::Fire) {
                    Shield *shield = GameObject::Get<Shield>(Player::sVars->maxPlayerCount + player->Slot());
                    canBreak |= shield->shieldAnimator.animationID == Shield::AniFireAttack;
                }

                switch (player->characterID) {
                    default: break;

                    case ID_SONIC:
                        canBreak |= player->animator.animationID == Player::ANI_DROPDASH;
                        canBreak |= player->superState == Player::SuperStateSuper;
                        break;

                    case ID_KNUCKLES: canBreak = true; break;
                }

                if (canBreak && !player->sidekick) {
                    if (player->CheckCollisionTouch(this, &this->hitbox)) {
                        Break(player->position.x > this->position.x);

                        if (player->characterID == ID_KNUCKLES) {
                            if (player->animator.animationID == Player::ANI_GLIDE) {
                                player->abilitySpeed -= player->abilitySpeed >> 2;
                                player->velocity.x -= player->velocity.x >> 2;
                                if (abs(player->velocity.x) <= 0x30000) {
                                    player->animator.SetAnimation(player->aniFrames, Player::ANI_GLIDE_DROP, false, 0);
                                    player->state.Set(&Player::State_KnuxGlideDrop);
                                }
                            }
                            else if (player->animator.animationID == Player::ANI_GLIDE_SLIDE) {
                                player->abilitySpeed -= player->abilitySpeed >> 2;
                                player->velocity.x -= player->velocity.x >> 2;
                            }
                        }

                        sVars->sfxLedgeBreak.Play();
                        this->Destroy();
                    }

                    continue; // skip to next loop, so we dont do the box collision
                }
                else {
                    switch (player->CheckCollisionBox(this, &this->hitbox)) {
                        case C_TOP: player->collisionFlagV |= 1; break;
                        case C_LEFT: player->collisionFlagH |= 1; break;
                        case C_RIGHT: player->collisionFlagH |= 2; break;
                        case C_BOTTOM: player->collisionFlagV |= 2; break;
                        default: break;
                    }
                }
            }

        player->CheckCollisionBox(this, &this->hitbox);
    }
}
void BreakableWall::CheckBreak_Floor()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 velY = player->velocity.y;

        bool32 tryBreak = true;
        uint8 side      = player->CheckCollisionBox(this, &this->hitbox);
        tryBreak &= side && side != C_BOTTOM;
        tryBreak &= sVars->field_138;
        tryBreak &= player->CheckCollisionBox(this, &this->hitbox) == C_TOP;

        if (tryBreak) {
                if (!this->onlyKnux || player->characterID == ID_KNUCKLES) {
                    bool32 canBreak = player->animator.animationID == Player::ANI_JUMP;
                    canBreak |= player->shield >= Shield::Unknown;

                    switch (player->characterID) {
                        default: break;

                        case ID_SONIC:
                            if (!canBreak)
                                canBreak = player->animator.animationID == Player::ANI_DROPDASH;
                            break;

                        case ID_KNUCKLES: canBreak = true; break;
                    }

                    if (sVars->disableScoreBonus)
                        canBreak &= velY > 0;

                    if (player->groundedStore && player->collisionMode != CMODE_LWALL && player->collisionMode != CMODE_RWALL)
                        canBreak = false;

                    if (canBreak && !player->sidekick) {
                        player->onGround = false;

                        Break(2);

                        sVars->sfxLedgeBreak.Play();

                        player->velocity.y = -0x30000;

                        if (!sVars->disableScoreBonus && !sVars->breakMode)
                            player->GiveScoreBonus(this->position);

                        this->Destroy();

                        continue; // skip to next loop, so we dont do the box collision
                    }
                }
        }

        player->CheckCollisionBox(this, &this->hitbox);
    }
}
void BreakableWall::CheckBreak_BurrowFloor()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 velY      = player->velocity.y;
        bool32 onGround = player->onGround;

        if (player->CheckCollisionBox(this, &this->hitbox) == C_TOP && !player->sidekick
            && ((player->collisionPlane == 1 && this->type == BreakableWall::BurrowFloor2) || this->type == BreakableWall::BurrowFloor)) {
                if (!this->onlyKnux || player->characterID == ID_KNUCKLES) {
                    bool32 canBreak = player->animator.animationID == Player::ANI_JUMP;

                    switch (player->characterID) {
                        default: break;

                        case ID_SONIC:
                            if (!canBreak)
                                canBreak = player->animator.animationID == Player::ANI_DROPDASH;
                            break;

                        case ID_KNUCKLES: canBreak = true; break;
                    }

                    if (onGround && player->collisionMode != CMODE_LWALL && player->collisionMode != CMODE_RWALL)
                        canBreak = false;

                    if (canBreak && !player->sidekick) {
                        player->onGround = false;

                        if (this->state.Matches(&BreakableWall::State_BurrowFloorUp)) {
                            if (this->size.y >= 3)
                                this->size.y = 2;
                        }
                        else {
                            this->size.y = 1;
                        }

                        Break(2);

                        player->GiveScoreBonus(this->position);

                        player->velocity.y = 0;

                        this->hitbox.top += 8;
                        this->hitbox.bottom -= 8;
                        --this->size.y;
                        this->position.y += 0x80000;

                        if (this->size.y <= 0)
                            this->Destroy();
                    }
                }
        }
    }
}
void BreakableWall::CheckBreak_BurrowFloorUp()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 velY = player->velocity.y;
        if (player->CheckCollisionBox(this, &this->hitbox) == C_BOTTOM) {
            if (!this->onlyKnux || player->characterID == ID_KNUCKLES) {
                if (!player->sidekick) {
                    Vector2 storePos = this->position;
                    if (this->state.Matches(&BreakableWall::State_BurrowFloorUp)) {
                        if (this->size.y >= 3)
                            this->size.y = 2;
                        }
                        else {
                            this->size.y = 1;
                        }

                    Break(2);
                    player->velocity.y = 0;
                    this->position     = storePos;
                    sVars->sfxLedgeBreak.Play();

                    if (this->size.y < 2) {
                        this->hitbox.top += 8;
                        this->size.y -= 1;
                        this->hitbox.bottom -= 8;
                        this->position.y -= 0x80000;
                    }
                    else {
                        this->hitbox.top += 16;
                        this->size.y -= 2;
                        this->hitbox.bottom -= 16;
                        this->position.y -= 0x100000;
                    }

                    player->velocity.y = velY;

                    if (this->size.y <= 0) {
                        this->Destroy();
                        break;
                    }
                }
            }
        }
    }
}
void BreakableWall::CheckBreak_Ceiling()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 velY = player->velocity.y;
        if (player->CheckCollisionBox(this, &this->hitbox) == C_BOTTOM) {
            if (!this->onlyKnux || player->characterID == ID_KNUCKLES) {
                player->onGround = false;

                Break(2);

                sVars->sfxLedgeBreak.Play();
                player->velocity.y = velY;

                this->Destroy();
                break;
            }
        }
    }
}
void BreakableWall::Break(uint8 direction)
{
    Vector2 storePos;

    int32 startX = this->startPos.x;
    int32 startY = this->startPos.y;
    int32 endX   = this->startPos.x - (this->size.x << 19) + 0x80000;
    int32 endY   = this->startPos.y - (this->size.y << 19) + 0x80000;

    switch (direction) {
        case FLIP_NONE: startX += this->size.x << 19; break;
        case FLIP_X: startX -= this->size.x << 19; break;
        case FLIP_Y: startY += this->size.y << 19; break;
        case FLIP_XY: startY -= this->size.y << 19; break;
        default: break;
    }

    int32 curY = endY - startY;
    for (int32 y = 0; y < this->size.y; ++y) {
        int32 curX   = endX - startX;
        int32 tileY  = (curY + startY) >> 20;
        int32 angleX = 2 * (endX - startX);

        for (int32 x = 0; x < this->size.x; ++x) {
            int32 tileX         = (curX + startX) >> 20;
            BreakableWall *tile = GameObject::Create<BreakableWall>(BreakableWall::TileFixed, curX + startX, curY + startY);
            tile->tileInfo      = this->targetLayer.GetTile(tileX, tileY);
            tile->drawGroup     = this->drawGroup;

            switch (direction) {
                case FLIP_NONE: {
                    int32 angle  = Math::ATan2(angleX, curY);
                    int32 angle2 = 0;
                    if (abs(curX) > 0x80000) {
                        if (curX + startX >= startX)
                            angle2 = Math::ATan2(0x80000, curY);
                        else
                            angle2 = Math::ATan2(-0x80000, curY);
                    }
                    else {
                        angle2 = Math::ATan2(curX, curY);
                    }

                    tile->velocity.x = -0x40000;
                    tile->velocity.x += 40 * Math::Cos256(angle) * ((3 * abs(curX) - abs(curY) + (this->size.y << 19)) >> 18);
                    tile->velocity.y = -0x20000;
                    tile->velocity.y += 40 * ((abs(curY) + abs(curX) + 2 * abs(curY)) >> 18) * Math::Sin256(angle2);
                    if (this->reverseX)
                        tile->velocity.x = -tile->velocity.x;
                    if (this->reverseY)
                        tile->velocity.y = -tile->velocity.y;
                    tile->gravityStrength = 0x7000;
                    break;
                }

                case FLIP_X: {
                    int32 angle  = Math::ATan2(angleX, curY);
                    int32 angle2 = 0;
                    if (abs(curX) > 0x80000) {
                        if (curX + startX >= startX)
                            angle2 = Math::ATan2(0x80000, curY);
                        else
                            angle2 = Math::ATan2(-0x80000, curY);
                    }
                    else {
                        angle2 = Math::ATan2(curX, curY);
                    }

                    tile->velocity.x = 0x10000;
                    tile->velocity.x += 40 * Math::Cos256(angle) * ((3 * abs(curX) - abs(curY) + (this->size.y << 19)) >> 18);
                    tile->velocity.y = 0x10000;
                    tile->velocity.y += 40 * ((abs(curY) + abs(curX) + 2 * abs(curY)) >> 18) * Math::Sin256(angle2);
                    if (this->reverseX)
                        tile->velocity.x = -tile->velocity.x;
                    if (this->reverseY)
                        tile->velocity.y = -tile->velocity.y;
                    tile->gravityStrength = 0x7000;
                    break;
                }

                case FLIP_Y: {
                    int32 angle = Math::ATan2(angleX, curY);

                    int32 powerX = 40;
                    int32 powerY = 40;
                    int32 multX  = 3;
                    int32 multY  = 3;
                    switch (sVars->breakMode) {
                        case 1:
                            powerX                = 4;
                            powerY                = 13;
                            multX                 = 16;
                            multY                 = 2;
                            gravityStrength       = 0x2000;
                            tile->gravityStrength = gravityStrength;
                            break;

                        case 2:
                            powerX                = 10;
                            powerY                = 10;
                            multX                 = 10;
                            multY                 = 1;
                            tile->gravityStrength = 0x2000;
                            tile->updateRange.x *= 4;
                            tile->updateRange.y *= 4;
                            tile->velocity.y = ((tile->velocity.y >> 16) - this->size.y + 1) << 16;
                            tile->velocity.y -= 0x8000;
                            break;

                        case 3:
                            powerX = 20;
                            powerY = 20;
                            multX  = 3;
                            multY  = 2;
                            tile->velocity.y -= (2 * this->size.y) << 16;
                            tile->gravityStrength = 0x2000;
                            break;

                        case 4:
                            multX                 = 16;
                            multY                 = 2;
                            tile->velocity.y      = ((tile->velocity.y >> 16) - this->size.y + 1) << 16;
                            tile->gravityStrength = 0x1C00;
                            powerX                = 4;
                            powerY                = 13;
                            break;

                        default: break;
                    }

                    tile->velocity.x += powerX * ((abs(curX) + multX * abs(curY)) >> 18) * Math::Cos256(angle);
                    tile->velocity.y += powerY * ((abs(curX) + multY * abs(curY)) >> 18) * Math::Sin256(angle);
                    break;
                }
            }

            this->targetLayer.SetTile(tileX, tileY, -1);
            if (this->drawGroup < Zone::sVars->objectDrawGroup[0]) {
                if (sVars->farPlaneLayer.Loaded())
                    sVars->farPlaneLayer.SetTile(tileX, tileY, -1);
            }

            curX += 0x10 << 16;
            angleX += 0x200000;
        }

        curY += 0x10 << 16;
    }

    if (this->useLayerDrawGroup)
        this->position = storePos;
}

#if RETRO_INCLUDE_EDITOR
void BreakableWall::EditorDraw()
{
    switch (this->type) {
        case BreakableWall::Wall:
            this->size.x >>= 16;
            this->size.y >>= 16;
            if (!this->size.x) {
                this->size.x = 2;
                this->size.y = 4;
            }

            State_DrawWall();

            this->size.x <<= 16;
            this->size.y <<= 16;
            break;

        case BreakableWall::Floor:
            this->size.x >>= 16;
            this->size.y >>= 16;
            if (!this->size.x) {
                this->size.x = 2;
                this->size.y = 2;
            }

            State_DrawFloor();

            this->size.x <<= 16;
            this->size.y <<= 16;
            break;

        case BreakableWall::BurrowFloor:
        case BreakableWall::BurrowFloor2:
        case BreakableWall::BurrowFloorUp:
        case BreakableWall::Ceiling:
            this->size.x >>= 16;
            this->size.y >>= 16;
            if (!this->size.x)
                this->size.x = 2;

            State_DrawFloor();

            this->size.x <<= 16;
            this->size.y <<= 16;
            break;

        default: break;
    }
}

void BreakableWall::EditorLoad()
{
    sVars->aniFrames.Load("Global/TicMark.bin", SCOPE_STAGE);
    sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Wall");
    RSDK_ENUM_VAR("Floor");
    RSDK_ENUM_VAR("Burrow Floor");
    RSDK_ENUM_VAR("Burrow Floor 2");
    RSDK_ENUM_VAR("Burrow Floor Up");
    RSDK_ENUM_VAR("Ceiling");
}
#endif

#if RETRO_REV0U
void BreakableWall::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(BreakableWall);

    sVars->aniFrames.Init();
    sVars->sfxLedgeBreak.Init();
}
#endif

void BreakableWall::Serialize()
{
    RSDK_EDITABLE_VAR(BreakableWall, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(BreakableWall, VAR_BOOL, onlyKnux);
    RSDK_EDITABLE_VAR(BreakableWall, VAR_INT32, targetLayer.id);
    RSDK_EDITABLE_VAR(BreakableWall, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(BreakableWall, VAR_BOOL, reverseX);
    RSDK_EDITABLE_VAR(BreakableWall, VAR_BOOL, reverseY);
    RSDK_EDITABLE_VAR(BreakableWall, VAR_INT8, topOffset);
}

} // namespace GameLogic