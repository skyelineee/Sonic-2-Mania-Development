// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SinkTrash Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SinkTrash.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Debris.hpp"
#include "Global/SignPost.hpp"
#include "Global/Music.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SinkTrash);

void SinkTrash::Update()
{
    switch (this->type) {
        case SINKTRASH_SINK: {
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
            {
                if (player->CheckCollisionTouch(this, &this->hitboxTrash)) {
                    if (player->velocity.y >= 0) {
                        player->onGround      = true;
                        player->velocity.y    = 0;
                        player->collisionMode = CMODE_FLOOR;
                        player->angle         = 0;
                        player->position.y += 0x10000;
                        int32 speed = abs(player->velocity.y + player->velocity.x);
                        if (speed > 0x10000 && !(Zone::sVars->timer & 7)) {
                            int32 x              = player->position.x + Math::Rand(-0x40000, 0x40000);
                            int32 y              = player->position.y + 0x40000 + Math::Rand(-0x40000, 0x40000);
                            Debris *debris       = GameObject::Create<Debris>(Debris::Fall, x, y); // review

                            debris->animator1.SetAnimation(sVars->aniFrames, Math::Rand(0, 2) + 9, false, 0);
                            debris->velocity.x      = Math::Rand(-0x20000, 0x20000);
                            debris->velocity.y      = -0x20000;
                            debris->gravityStrength = 0x3800;
                            debris->updateRange.y   = 0x200000;
                            debris->updateRange.x   = 0x200000;
                            debris->drawGroup       = Zone::sVars->objectDrawGroup[0];
                        }
                    }
                }
                else if (player->CheckCollisionTouch(this, &this->hitboxDebris)) {
                    if (!(Zone::sVars->timer & 7)) {
                        Debris *debris = GameObject::Create<Debris>(Debris::Fall, player->position.x + Math::Rand(-0x40000, 0x40000),
                                                             player->position.y + Math::Rand(-0x100000, 0));
                        debris->animator1.SetAnimation(sVars->aniFrames, 0, false, Math::Rand(0, 20));
                        debris->velocity.x      = Math::Rand(-0x20000, 0x20000);
                        debris->velocity.y      = player->velocity.y;
                        debris->gravityStrength = 0x3800;
                        debris->updateRange.y   = 0x200000;
                        debris->updateRange.x   = 0x200000;
                        debris->drawGroup       = Zone::sVars->objectDrawGroup[0];
                    }
                }
            }
            break;
        }

        default:
        case SINKTRASH_SOLID: {
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) { player->CheckCollisionPlatform(this, &this->hitboxTrash); }

            for (auto signPost : GameObject::GetEntities<SignPost>(FOR_ACTIVE_ENTITIES))
            {
                if (signPost->state.Matches(&SignPost::State_Falling) && signPost->position.y + 0x180000 > this->position.y - (this->size.y >> 1)) {
                    SignPost::sVars->sfxSlide.Play(false, 255);
                    signPost->spinCount  = 4;
                    signPost->position.y = this->position.y - (this->size.y >> 1) - 0x180000;
                    signPost->velocity.y = 0;
                    Music::FadeOut(0.025);
                    signPost->state.Set(&SignPost::State_Spin);
                }
            }
            break;
        }

        case SINKTRASH_DECOR: break;
    }
}

void SinkTrash::LateUpdate() {}

void SinkTrash::StaticUpdate() {}

void SinkTrash::Draw()
{
    Vector2 drawPos;

    this->direction = FLIP_NONE;
    int32 drawX     = this->position.x - (this->size.x >> 1);

    SpriteFrame *frame = sVars->aniFrames.GetFrame(11, 0);
    drawPos.x          = this->position.x - (this->size.x >> 1);
    drawPos.y          = this->position.y - (this->size.y >> 1);
    frame->width       = 64;
    for (int32 i = 0; i < this->size.x; i += 0x400000) {
        if (this->size.x - i < 0x400000)
            frame->width = (this->size.x - i) >> 16;

        this->topAnimator.DrawSprite(&drawPos, false);

        drawPos.x += 0x400000;
    }

    frame         = sVars->aniFrames.GetFrame(11, 1);
    frame->height = 64;
    for (int32 i = 0; i < this->size.y; i += 0x400000) {
        drawPos.x = drawX;

        frame->width = 64;
        if (this->size.y - i < 0x400000)
            frame->height = (this->size.y - i) >> 16;

        for (int32 x = 0; x < this->size.x; x += 0x400000) {
            if (this->size.x - x < 0x400000)
                frame->width = (this->size.x - x) >> 16;

            this->mainAnimator.DrawSprite(&drawPos, false);

            drawPos.x += 0x400000;
        }
        drawPos.y += 0x400000;
    }

    frame           = sVars->aniFrames.GetFrame(11, 0);
    this->direction = FLIP_Y;
    drawPos.x       = drawX;
    frame->width    = 64;

    for (int32 i = 0; i < this->size.x; i += 0x400000) {
        if (this->size.x - i < 0x400000)
            frame->width = (this->size.x - i) >> 16;

        this->topAnimator.DrawSprite(&drawPos, false);

        drawPos.x += 0x400000;
    }

    if (this->type < SINKTRASH_SOLID) {
        for (int32 i = 0; i < 64; ++i) {
            drawPos.x                   = this->position.x + this->trashPos[i].x;
            drawPos.y                   = this->position.y + this->trashPos[i].y;
            this->trashAnimator.frameID = this->trashFrame[i];
            this->direction             = this->trashDir[i];
            this->trashAnimator.DrawSprite(&drawPos, false);
        }

        this->direction = FLIP_NONE;
    }
}

void SinkTrash::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->updateRange.x = this->size.x >> 1;
        this->updateRange.y = (this->size.y >> 1) + 0x400000;
        this->active        = ACTIVE_BOUNDS;
        this->visible       = true;
        this->drawFX        = FX_FLIP;
        this->drawGroup     = this->type == SINKTRASH_SINK ? (Zone::sVars->objectDrawGroup[1] - 2) : Zone::sVars->objectDrawGroup[0];

        this->hitboxTrash.left   = -(this->size.x >> 17);
        this->hitboxTrash.top    = -(this->size.y >> 17);
        this->hitboxTrash.right  = this->size.x >> 17;
        this->hitboxTrash.bottom = 24 - (this->size.y >> 17);

        this->hitboxDebris.left   = -(this->size.x >> 17);
        this->hitboxDebris.top    = (this->size.y >> 17) - 16;
        this->hitboxDebris.right  = this->size.x >> 17;
        this->hitboxDebris.bottom = this->size.y >> 17;

        this->trashAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->topAnimator.SetAnimation(sVars->aniFrames, 11, true, 0);
        this->mainAnimator.SetAnimation(sVars->aniFrames, 11, true, 1);

        for (int32 i = 0; i < 64; ++i) {
            this->trashPos[i].x = Math::Rand(-(this->size.x >> 1), this->size.x >> 1);
            this->trashPos[i].y = Math::Rand(-(this->size.y >> 1), this->size.y >> 1);
            this->trashFrame[i] = Math::Rand(0, 22);
            this->trashDir[i]   = Math::Rand(FLIP_NONE, FLIP_XY + 1);
        }
    }
}

void SinkTrash::StageLoad() { sVars->aniFrames.Load("SSZ/Trash.bin", SCOPE_STAGE); }

#if RETRO_INCLUDE_EDITOR
void SinkTrash::EditorDraw()
{
    this->updateRange.x = this->size.x >> 1;
    this->updateRange.y = (this->size.y >> 1) + 0x400000;
    this->drawFX        = FX_FLIP;
    this->direction     = FLIP_NONE;

    this->trashAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->topAnimator.SetAnimation(sVars->aniFrames, 11, true, 0);
    this->mainAnimator.SetAnimation(sVars->aniFrames, 11, true, 1);

    // this is pretty nasty but I didn't wanna make new vars and I needed smth to store it
    int32 *prevX = (int32 *)&this->hitboxTrash.left;
    int32 *prevY = (int32 *)&this->hitboxTrash.right;

    if (*prevX != this->size.x || *prevY != this->size.y) {
        for (int32 i = 0; i < 64; ++i) {
            this->trashPos[i].x = Math::Rand(-(this->size.x >> 1), this->size.x >> 1);
            this->trashPos[i].y = Math::Rand(-(this->size.y >> 1), this->size.y >> 1);
            this->trashFrame[i] = Math::Rand(0, 22);
            this->trashDir[i]   = Math::Rand(0, 4);
        }

        *prevX = this->size.x;
        *prevY = this->size.y;
    }

    // Draw everything except the trash, we'll do that seperately so it's part of the overlay, not the base sprite
    int32 type = this->type;
    this->type = SINKTRASH_SOLID;
    SinkTrash::Draw();
    this->type = type;

    if (this->type < SINKTRASH_SOLID) {
        RSDK_DRAWING_OVERLAY(true);
        Vector2 drawPos;
        for (int32 i = 0; i < 64; ++i) {
            drawPos.x                   = this->position.x + this->trashPos[i].x;
            drawPos.y                   = this->position.y + this->trashPos[i].y;
            this->trashAnimator.frameID = this->trashFrame[i];
            this->direction             = this->trashDir[i];
            this->trashAnimator.DrawSprite(&drawPos, false);
        }

        this->direction = FLIP_NONE;
        RSDK_DRAWING_OVERLAY(false);
    }

    DrawHelpers::DrawRectOutline(this->position.x, this->position.y, this->size.x, this->size.y, 0xFFFF00);
}

void SinkTrash::EditorLoad()
{
    sVars->aniFrames.Load("SSZ/Trash.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Sink Trash");
    RSDK_ENUM_VAR("Solid");
    RSDK_ENUM_VAR("Decoration");
}
#endif

void SinkTrash::Serialize()
{
    RSDK_EDITABLE_VAR(SinkTrash, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(SinkTrash, VAR_VECTOR2, size);
}

} // namespace GameLogic