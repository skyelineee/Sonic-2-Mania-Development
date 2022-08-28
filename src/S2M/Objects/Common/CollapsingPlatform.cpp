// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: CollapsingPlatform Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "CollapsingPlatform.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Common/ScreenWrap.hpp"
#include "BreakableWall.hpp"
#include "Button.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(CollapsingPlatform);

void CollapsingPlatform::Update()
{
    this->visible = false;
    if (DebugMode::sVars)
        this->visible = DebugMode::sVars->debugActive;

    if (!this->hasSetupTagLink) {
        SetupTagLink();
        this->hasSetupTagLink = true;
    }

    bool32 runState = false;

    if (this->active != ACTIVE_NORMAL) {
        if (this->collapseDelay) {
            if (Player::sVars) {
                for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                    if (player->CheckCollisionTouch(this, &this->hitboxTrigger) && player->characterID == ID_MIGHTY && player->jumpAbilityState > 1) {
                        runState = true;
                        break;
                    }
                }
            }

            if (!runState && --this->collapseDelay == 0)
                runState = true;
        }
        else {
            if (Player::sVars) {
                this->direction = FLIP_NONE;
                for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                    if (player->CheckCollisionTouch(this, &this->hitboxTrigger)
                        && (!this->mightyOnly || (player->characterID == ID_MIGHTY && player->state.Matches(&Player::State_MightyHammerDrop)))
                        && !player->sidekick && player->onGround && !player->collisionMode && !this->eventOnly && this->delay < 0xFFFF) {
                        this->stoodPos = player->position.x;
                        if (player->characterID == ID_MIGHTY && player->jumpAbilityState > 1) {
                            runState = true;
                            break;
                        }
                    }
                }
            }

            if (!runState && this->stoodPos) {
                this->collapseDelay = this->delay;
                if (!this->delay)
                    runState = true;
            }
        }

        if (runState) {
            this->state.Run(this);

            sVars->sfxLedgeBreak.Play();
            if (this->respawn) {
                this->collapseDelay = 0;
                this->stoodPos      = 0;
                this->active        = ACTIVE_NORMAL;
            }
            else {
                this->Destroy();
            }
        }

        if (this->eventOnly) {
            bool32 activated = false;

            for (int32 i = 0; i < (int32)this->taggedButtons.size(); ++i) {
                activated |= *this->taggedButtons[i];
            }

            if (activated) {
                this->collapseDelay = this->delay;
                this->eventOnly     = false;
            }
        }
    }
    else {
        if (!this->CheckOnScreen(&this->updateRange)) {
            int32 xOff = (this->originPos.x >> 20) - (this->size.x >> 21);
            int32 yOff = (this->originPos.y >> 20) - (this->size.y >> 21);

            if ((this->size.y & 0xFFF00000) && !(this->size.y & 0xFFF00000 & 0x80000000)) {
                int32 sx = this->size.x >> 20;
                int32 sy = this->size.y >> 20;
                for (int32 y = 0; y < sy; ++y) {
                    for (int32 x = 0; x < sx; ++x) {
                        this->targetLayer.SetTile(x + xOff, y + yOff, this->storedTiles[x + y * (this->size.x >> 20)]);
                    }
                }
            }

            if (this->useLayerDrawGroup) {
                int32 xOff = (this->originPos.x >> 20) - (this->size.x >> 21);
                int32 yOff = (this->originPos.y >> 20) - (this->size.y >> 21);

                if ((this->size.y & 0xFFF00000) && !(this->size.y & 0xFFF00000 & 0x80000000)) {
                    int32 sx = this->size.x >> 20;
                    int32 sy = this->size.y >> 20;
                    for (int32 y = 0; y < sy; ++y) {
                        for (int32 x = 0; x < sx; ++x) {
                            int32 bufferX = ScreenWrap::sVars->activeVWrap->buffer.x >> 16;
                            this->targetLayer.SetTile(x + xOff + ((bufferX + ((bufferX >> 27) & 0xF)) >> 4), y + yOff,
                                                      this->storedTiles[x + y * (this->size.x >> 20)]);
                        }
                    }
                }
            }

            this->active = ACTIVE_BOUNDS;
        }
    }
}
void CollapsingPlatform::LateUpdate() {}
void CollapsingPlatform::StaticUpdate() {}
void CollapsingPlatform::Draw()
{
    Vector2 drawPos;

    drawPos.x = this->position.x - (this->size.x >> 1);
    drawPos.y = this->position.y - (this->size.y >> 1);
    Graphics::DrawLine(drawPos.x - 0x10000, drawPos.y - 0x10000, drawPos.x + this->size.x, drawPos.y - 0x10000, 0xE0E0E0, 0, INK_NONE, false);
    Graphics::DrawLine(drawPos.x - 0x10000, this->size.y + drawPos.y, drawPos.x + this->size.x, this->size.y + drawPos.y, 0xE0E0E0, 0, INK_NONE,
                       false);
    Graphics::DrawLine(drawPos.x - 0x10000, drawPos.y - 0x10000, drawPos.x - 0x10000, drawPos.y + this->size.y, 0xE0E0E0, 0, INK_NONE, false);
    Graphics::DrawLine(drawPos.x + this->size.x, drawPos.y - 0x10000, drawPos.x + this->size.x, drawPos.y + this->size.y, 0xE0E0E0, 0, INK_NONE,
                       false);

    this->direction = FLIP_NONE;
    sVars->animator.DrawSprite(&drawPos, false);

    drawPos.x += this->size.x;
    this->direction = FLIP_X;
    sVars->animator.DrawSprite(&drawPos, false);

    drawPos.y += this->size.y;
    this->direction = FLIP_XY;
    sVars->animator.DrawSprite(&drawPos, false);

    drawPos.x -= this->size.x;
    this->direction = FLIP_Y;
    sVars->animator.DrawSprite(&drawPos, false);
}

void CollapsingPlatform::Create(void *data)
{
    this->visible   = true;
    this->originPos = this->position;
    this->position.x &= 0xFFF80000;
    this->position.y &= 0xFFF80000;
    this->drawFX |= FX_FLIP;
    this->drawGroup = Zone::sVars->objectDrawGroup[1];

    if (!sceneInfo->inEditor) {
        if (!globals->useManiaBehavior) {
            this->drawGroup = this->targetLayer.GetTileLayer()->drawGroup[0];
        }
        else {
            if (this->targetLayer.id != 0) {
                this->targetLayer = Zone::sVars->fgLayer[1];
                this->drawGroup   = Zone::sVars->objectDrawGroup[1];
            }
            else {
                this->targetLayer = Zone::sVars->fgLayer[0];
                this->drawGroup   = Zone::sVars->objectDrawGroup[0];
            }
        }

        if (data) {
            Vector2 *size = (Vector2 *)data;
            this->size    = *size;
        }

        this->useLayerDrawGroup = false;
        this->hasSetupTagLink   = false;
        this->timerMultipler    = 1.0;

        if (ScreenWrap::sVars) {
            ScreenWrap *wrap = ScreenWrap::sVars->activeVWrap;
            if (wrap) {
                if (this->position.y <= wrap->buffer.y + (abs(wrap->buffer.y) >> 1))
                    this->useLayerDrawGroup = true;
            }
        }

        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;

        SetupTiles();

        if (this->useLayerDrawGroup) {
            this->drawGroup = this->targetLayer.GetTileLayer()->drawGroup[0];
            SetupHWrapTiles();
        }

        switch (this->type) {
            default:
            case CollapsingPlatform::Left: this->state.Set(&CollapsingPlatform::CrumbleFromLeft); break;
            case CollapsingPlatform::Right: this->state.Set(&CollapsingPlatform::CrumbleFromRight); break;
            case CollapsingPlatform::Center: this->state.Set(&CollapsingPlatform::CrumbleFromCenter); break;
            case CollapsingPlatform::LR: this->state.Set(&CollapsingPlatform::CrumbleFromLeftRight); break;
            case CollapsingPlatform::LRC: this->state.Set(&CollapsingPlatform::CrumbleFromLeftRightCenter); break;
            case CollapsingPlatform::LR_Rv:
            case CollapsingPlatform::LRC_Rv: this->state.Set(&CollapsingPlatform::CrumbleFromLeftRightCenter_Reverse); break;
        }
    }
}

void CollapsingPlatform::StageLoad()
{
    sVars->aniFrames.Load("Global/TicMark.bin", SCOPE_STAGE);
    sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    sVars->sfxLedgeBreak.Get("Stage/LedgeBreak.wav");
}

void CollapsingPlatform::SetupTiles()
{
    int32 xOff = (this->originPos.x >> 20) - (this->size.x >> 21);
    int32 yOff = (this->originPos.y >> 20) - (this->size.y >> 21);

    if ((this->size.y & 0xFFF00000) && !(this->size.y & 0xFFF00000 & 0x80000000)) {
        int32 sx = this->size.x >> 20;
        int32 sy = this->size.y >> 20;
        for (int32 y = 0; y < sy; ++y) {
            for (int32 x = 0; x < sx; ++x) {
                this->storedTiles[x + y * (this->size.x >> 20)] = this->targetLayer.GetTile(x + xOff, y + yOff);
            }
        }
    }

    this->hitboxTrigger.left   = -(this->size.x >> 17);
    this->hitboxTrigger.top    = -16 - (this->size.y >> 17);
    this->hitboxTrigger.right  = this->size.x >> 17;
    this->hitboxTrigger.bottom = this->size.y >> 17;
}

void CollapsingPlatform::SetupHWrapTiles()
{
    int32 xOff = (this->originPos.x >> 20) - (this->size.x >> 21);
    int32 yOff = (this->originPos.y >> 20) - (this->size.y >> 21);

    if ((this->size.y & 0xFFF00000) && !(this->size.y & 0xFFF00000 & 0x80000000)) {
        int32 sx = this->size.x >> 20;
        int32 sy = this->size.y >> 20;
        for (int32 y = 0; y < sy; ++y) {
            for (int32 x = 0; x < sx; ++x) {
                int32 bufferX = ScreenWrap::sVars->activeVWrap->buffer.x >> 16;
                this->storedTiles[x + y * (this->size.x >> 20)] =
                    this->targetLayer.GetTile(x + xOff + ((bufferX + ((bufferX >> 27) & 0xF)) >> 4), y + yOff);
            }
        }
    }

    this->hitboxTrigger.left   = -(this->size.x >> 17);
    this->hitboxTrigger.top    = -16 - (this->size.y >> 17);
    this->hitboxTrigger.right  = this->size.x >> 17;
    this->hitboxTrigger.bottom = this->size.y >> 17;
}

void CollapsingPlatform::SetupTagLink()
{
    if (this->buttonTag > 0) {
        if (Button::sVars) {
            for (auto button : GameObject::GetEntities<Button>(FOR_ALL_ENTITIES)) {
                if (button->tag == this->buttonTag) {
                    this->taggedButtons.push_back(&button->activated);

                    int32 distX = abs(this->position.x - button->position.x) + 0x800000;
                    int32 distY = abs(this->position.y - button->position.y) + 0x800000;

                    if (this->updateRange.x < distX)
                        this->updateRange.x = distX;

                    if (this->updateRange.y < distY)
                        this->updateRange.y = distY;
                }
            }
        }
    }
}

void CollapsingPlatform::CrumbleFromLeft()
{
    SET_CURRENT_STATE();

    RSDK::Tile *tiles = this->storedTiles;
    int32 startTX     = (this->position.x >> 20) - (this->size.x >> 21);
    int32 startTY     = (this->position.y >> 20) - (this->size.y >> 21);
    int32 tx          = this->position.x - (this->size.x >> 1) + 0x80000;
    int32 ty          = this->position.y - (this->size.y >> 1) + 0x80000;

    int32 sx = this->size.x >> 20;
    int32 sy = this->size.y >> 20;

    for (int32 y = 0; y < sy; ++y) {
        for (int32 x = 0; x < sx; ++x) {
            BreakableWall *tile     = GameObject::Create<BreakableWall>(BreakableWall::TileDynamic, tx, ty);
            tile->targetLayer       = this->targetLayer;
            tile->tileInfo          = *tiles;
            tile->drawGroup         = this->drawGroup;
            tile->useLayerDrawGroup = this->useLayerDrawGroup;
            tile->tilePos.x         = x + startTX;
            tile->tilePos.y         = y + startTY;
            int32 timerX            = x >> this->shift;
            int32 timerY            = y >> this->shift;
            tile->timer             = 3 * (sy + 2 * timerX - timerY);
            tile->timer += 6 * timerX;
            tile->timer = (int32)(this->timerMultipler * tile->timer);

            ++tiles;
            tx += 0x100000;
        }

        tx -= this->size.x;
        ty += 0x100000;
    }
}
void CollapsingPlatform::CrumbleFromRight()
{
    SET_CURRENT_STATE();

    RSDK::Tile *tiles = this->storedTiles;
    int32 startTX     = (this->position.x >> 20) - (this->size.x >> 21);
    int32 startTY     = (this->position.y >> 20) - (this->size.y >> 21);
    int32 tx          = this->position.x - (this->size.x >> 1) + 0x80000;
    int32 ty          = this->position.y - (this->size.y >> 1) + 0x80000;

    int32 timerSX = this->size.x >> this->shift >> 20;

    int32 sx = this->size.x >> 20;
    int32 sy = this->size.y >> 20;

    for (int32 y = 0; y < sy; ++y) {
        for (int32 x = 0; x < sx; ++x) {
            BreakableWall *tile     = GameObject::Create<BreakableWall>(BreakableWall::TileDynamic, tx, ty);
            tile->targetLayer       = this->targetLayer;
            tile->tileInfo          = *tiles;
            tile->drawGroup         = this->drawGroup;
            tile->useLayerDrawGroup = this->useLayerDrawGroup;
            tile->tilePos.x         = x + startTX;
            tile->tilePos.y         = y + startTY;
            int32 timerX            = x >> this->shift;
            int32 timerY            = y >> this->shift;
            tile->timer             = 3 * (sy + 2 * (timerSX - timerX) - timerY);
            tile->timer += 6 * ((this->size.x >> 20) - timerX);
            tile->timer = (int32)(this->timerMultipler * tile->timer);

            ++tiles;
            tx += 0x100000;
        }

        tx -= this->size.x;
        ty += 0x100000;
    }
}
void CollapsingPlatform::CrumbleFromCenter()
{
    SET_CURRENT_STATE();

    RSDK::Tile *tiles = this->storedTiles;
    int32 startTX     = (this->position.x >> 20) - (this->size.x >> 21);
    int32 startTY     = (this->position.y >> 20) - (this->size.y >> 21);
    int32 tx          = this->position.x - (this->size.x >> 1) + 0x80000;
    int32 ty          = this->position.y - (this->size.y >> 1) + 0x80000;

    int32 timerSX = this->size.x >> this->shift >> 20;
    int32 timerSY = this->size.y >> this->shift >> 20;

    int32 sx = this->size.x >> 20;
    int32 sy = this->size.y >> 20;

    for (int32 y = 0; y < sy; ++y) {
        for (int32 x = 0; x < sx; ++x) {
            BreakableWall *tile     = GameObject::Create<BreakableWall>(BreakableWall::TileDynamic, tx, ty);
            tile->targetLayer       = this->targetLayer;
            tile->tileInfo          = *tiles;
            tile->drawGroup         = this->drawGroup;
            tile->useLayerDrawGroup = this->useLayerDrawGroup;
            tile->tilePos.x         = x + startTX;
            tile->tilePos.y         = y + startTY;
            int32 timerX            = abs((timerSX >> 1) - (x >> this->shift));
            int32 timerY            = y >> this->shift;
            tile->timer             = 3 * (timerSY + 2 * timerX - timerY);
            tile->timer += 6 * abs((timerSX >> 1) - (x >> this->shift));

            if (!(timerSX & 1) && x >> this->shift < (timerSX >> 1))
                tile->timer -= 6;

            tile->timer = (int32)(this->timerMultipler * tile->timer);

            ++tiles;
            tx += 0x100000;
        }

        tx -= this->size.x;
        ty += 0x100000;
    }
}
void CollapsingPlatform::CrumbleFromLeftRight()
{
    SET_CURRENT_STATE();

    int32 px = this->stoodPos;
    int32 x  = this->position.x;

    if (px < x)
        CrumbleFromLeft();
    else
        CrumbleFromRight();
}
void CollapsingPlatform::CrumbleFromLeftRightCenter()
{
    SET_CURRENT_STATE();

    int32 px = this->stoodPos;
    int32 x  = this->position.x;

    if (abs(px - x) < this->size.x / 6)
        CrumbleFromCenter();
    else if (px < x)
        CrumbleFromLeft();
    else
        CrumbleFromRight();
}
void CollapsingPlatform::CrumbleFromLeftRightCenter_Reverse()
{
    SET_CURRENT_STATE();

    int32 px = this->stoodPos;
    int32 x  = this->position.x;

    if (abs(px - x) < this->size.x / 6)
        CrumbleFromCenter();
    else if (px < x)
        CrumbleFromRight();
    else
        CrumbleFromLeft();
}
void CollapsingPlatform::CrumbleDestroy()
{
    SET_CURRENT_STATE();

    RSDK::Tile *tiles = this->storedTiles;
    int32 startTX     = (this->position.x >> 20) - (this->size.x >> 21);
    int32 startTY     = (this->position.y >> 20) - (this->size.y >> 21);
    int32 tx          = this->position.x - (this->size.x >> 1) + 0x80000;
    int32 ty          = this->position.y - (this->size.y >> 1) + 0x80000;

    int32 sx = this->size.x >> 20;
    int32 sy = this->size.y >> 20;

    for (int32 y = 0; y < sy; ++y) {
        for (int32 x = 0; x < sx; ++x) {
            BreakableWall *tile     = GameObject::Create<BreakableWall>(BreakableWall::TileDynamic, tx, ty);
            tile->targetLayer       = this->targetLayer;
            tile->tileInfo          = *tiles;
            tile->drawGroup         = this->drawGroup;
            tile->useLayerDrawGroup = this->useLayerDrawGroup;
            tile->tilePos.x         = x + startTX;
            tile->tilePos.y         = y + startTY;
            tile->timer             = 0;
            tile->Update();
            tile->Destroy();

            ++tiles;
            tx += 0x100000;
        }

        tx -= this->size.x;
        ty += 0x100000;
    }
}

#if RETRO_INCLUDE_EDITOR
void CollapsingPlatform::EditorDraw()
{
    int32 sizeX = this->size.x >> 20;
    int32 sizeY = this->size.y >> 20;

    if (sizeX * sizeY > 0x100) {
        if (sizeX <= sizeY)
            this->size.x = this->size.y;

        if (sizeX > sizeY)
            sizeX = sizeY;

        this->size.x = (0x100 / sizeX) << 20;
    }

    Draw();
}

void CollapsingPlatform::EditorLoad()
{
    sVars->aniFrames.Load("Global/TicMark.bin", SCOPE_STAGE);
    sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("From Left");
    RSDK_ENUM_VAR("From Right");
    RSDK_ENUM_VAR("From Center");
    RSDK_ENUM_VAR("From L/R");
    RSDK_ENUM_VAR("From L/R/C");
    RSDK_ENUM_VAR("From L/R Rv");
    RSDK_ENUM_VAR("From L/R/C Rv");
}
#endif

#if RETRO_REV0U
void CollapsingPlatform::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(CollapsingPlatform);

    sVars->aniFrames.Init();

    sVars->sfxLedgeBreak.Init();
}
#endif

void CollapsingPlatform::Serialize()
{
    RSDK_EDITABLE_VAR(CollapsingPlatform, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(CollapsingPlatform, VAR_BOOL, respawn);
    RSDK_EDITABLE_VAR(CollapsingPlatform, VAR_UINT16, targetLayer.id);
    RSDK_EDITABLE_VAR(CollapsingPlatform, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(CollapsingPlatform, VAR_ENUM, delay);
    RSDK_EDITABLE_VAR(CollapsingPlatform, VAR_BOOL, eventOnly);
    RSDK_EDITABLE_VAR(CollapsingPlatform, VAR_BOOL, mightyOnly);
    RSDK_EDITABLE_VAR(CollapsingPlatform, VAR_INT32, buttonTag);
    RSDK_EDITABLE_VAR(CollapsingPlatform, VAR_UINT8, shift);
}

} // namespace GameLogic
