// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TippingPlatform Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "TippingPlatform.hpp"
#include "Global/Zone.hpp"
#include "Common/Platform.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TippingPlatform);

void TippingPlatform::Update()
{
    if (this->bossID) {
        if (this->animator.frameID) {
            this->stateCollide.Set(&Platform::Collision_None);
            this->collision = Platform::C_None;
        }
        else {
            this->stateCollide.Set(&Platform::Collision_Platform);
            this->collision = Platform::C_Platform;
        }

        if (!this->stoodAngle && this->bossID > TIPPINGPLATFORM_EGGMAN) {
            this->stoodAngle = 1;
            this->timer      = this->intervalOffset + 1;
            this->animator.SetAnimation(Platform::sVars->aniFrames, 1, true, 0);
            this->state.Set(&TippingPlatform::State_Tipping_Delay);
        }
    }
    else {
        if (this->state.Matches(&Platform::State_Fixed)) {
            int32 timer = (Zone::sVars->timer + this->intervalOffset) % this->interval;
            if (timer >= this->duration) {
                if (timer >= this->duration + 22) {
                    this->animator.SetAnimation(Platform::sVars->aniFrames, 2, true, 6);
                }
                else {
                    this->active = ACTIVE_NORMAL;
                    this->animator.SetAnimation(Platform::sVars->aniFrames, 2, true, (timer - this->duration) >> 2);
                    this->state.Set(&TippingPlatform::State_Restore);
                }
            }
            else {
                this->active = ACTIVE_NORMAL;
                this->timer  = this->duration - timer;

                if (this->duration - timer < this->duration - 24) {
                    this->animator.SetAnimation(Platform::sVars->aniFrames, 1, true, 6);
                    this->state.Set(&TippingPlatform::State_Tipping);
                }
                else {
                    this->animator.SetAnimation(Platform::sVars->aniFrames, 1, true, timer >> 2);
                    this->state.Set(&TippingPlatform::State_Tipping);
                    this->animator.timer = (this->duration - this->timer) & 3;
                }
            }
        }

        bool32 isSolid = false;
        if (this->animator.animationID == 1)
            isSolid = this->animator.frameID == 0;
        else
            isSolid = this->animator.frameID == 6;

        if (isSolid) {
            this->stateCollide.Set(&Platform::Collision_Platform);
            this->collision = Platform::C_Platform;
        }
        else {
            this->stateCollide.Set(&Platform::Collision_None);
            this->collision = Platform::C_None;
        }
    }

    Platform *platform = (Platform *)this;
    platform->Update();
}

void TippingPlatform::LateUpdate() {}

void TippingPlatform::StaticUpdate() {}

void TippingPlatform::Draw() { this->animator.DrawSprite(&this->drawPos, false); }

void TippingPlatform::Create(void *data)
{
    if (!this->interval)
        this->interval = -16;

    if (!this->duration)
        this->duration = 120;

    this->collision    = Platform::C_Platform;
    Platform *platform = (Platform *)this;
    platform->Create(nullptr);

    if (this->bossID) {
        this->animator.SetAnimation(Platform::sVars->aniFrames, 1, true, 0);
        this->animator.speed = 0;
    }
    else {
        this->animator.SetAnimation(Platform::sVars->aniFrames, 2, true, 6);
    }

    this->state.Set(&Platform::State_Fixed);
    if (!sceneInfo->inEditor) {
        if (this->bossID > TIPPINGPLATFORM_PLAYER) {
            // foreach_all(Syringe, syringe)
            //{
            //     if (syringe->tag == this->bossID) {
            //         this->syringe = syringe;
            //         if (syringe) {
            //             this->updateRange.x = abs(this->position.x - syringe->position.x) + 0x400000;
            //             this->updateRange.y = abs(this->position.y - syringe->position.y) + 0x400000;
            //         }
            //         this->stateCollide = Platform_Collision_Solid;
            //         this->collision    = PLATFORM_C_SOLID;
            //         foreach_break;
            //     }
            // }
        }

        if (this->bossID == TIPPINGPLATFORM_PLAYER) {
            this->direction = FLIP_X;
            this->drawFX    = FX_FLIP;
        }
    }
}

void TippingPlatform::StageLoad() {}

void TippingPlatform::State_Tipping_Boss()
{
    if (this->animator.frameID < 6 && ++this->animator.timer == 4) {
        this->animator.timer = 0;
        this->animator.frameID++;
    }

    if (--this->timer <= 0) {
        this->active = ACTIVE_BOUNDS;
        if (this->bossID <= TIPPINGPLATFORM_EGGMAN) {
            this->state.Set(&TippingPlatform::State_RestorePlatform);
        }
        else
            this->state.Set(&Platform::State_Fixed);
    }
}

void TippingPlatform::State_RestorePlatform()
{
    if (this->animator.frameID <= 0) {
        this->active = ACTIVE_BOUNDS;
        this->state.Set(&Platform::State_Fixed);
    }
    else if (++this->animator.timer == 4) {
        this->animator.timer = 0;
        this->animator.frameID--;
    }
}

void TippingPlatform::State_Tipping()
{
    if (--this->timer <= 0) {
        this->animator.SetAnimation(Platform::sVars->aniFrames, 2, true, 0);
        this->state.Set(&TippingPlatform::State_RestorePlatform);
    }
}

void TippingPlatform::State_Restore()
{
    if (this->animator.frameID == 6) {
        this->active = ACTIVE_BOUNDS;
        this->state.Set(&Platform::State_Fixed);
    }
}

void TippingPlatform::State_Tipping_Delay()
{
    if (--this->timer <= 0)
        this->state.Set(&TippingPlatform::State_Tipping_Boss);
}

#if RETRO_INCLUDE_EDITOR
void TippingPlatform::EditorDraw()
{
    this->collision    = Platform::C_Platform;
    Platform *platform = (Platform *)this;
    platform->Create(nullptr);

    if (this->bossID) {
        this->animator.SetAnimation(Platform::sVars->aniFrames, 1, true, 0);
        this->animator.speed = 0;
    }
    else {
        this->animator.SetAnimation(Platform::sVars->aniFrames, 2, true, 6);
    }

    TippingPlatform::Draw();

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        if (this->bossID > TIPPINGPLATFORM_PLAYER) {
            // foreach_all(Syringe, syringe)
            //{
            //     if (syringe->tag == this->bossID) {
            //         DrawHelpers_DrawArrow(this->position.x, this->position.y, syringe->position.x, syringe->position.y, 0xFFFF00, INK_NONE, 0xFF);
            //         foreach_break;
            //     }
            // }
        }

        for (int32 s = sceneInfo->entitySlot + 1, i = 0; i < this->childCount; ++i) {
            // Entity *child = RSDK_GET_ENTITY_GEN(s + i);
            Entity *child = GameObject::Get(s + i);
            if (!child)
                continue;

            DrawHelpers::DrawArrow(this->position.x, this->position.y, child->position.x, child->position.y, 0xFFFF00, INK_NONE, 0xFF);
        }

        RSDK_DRAWING_OVERLAY(false);
    }
}

void TippingPlatform::EditorLoad(void)
{

    // bossID = 0: No tag
    // bossID = 1: Player Boss Platform
    // bossID = 2: Eggman Boss Platform
    // bossID 3 and above: match syringe tag
}
#endif

void TippingPlatform::Serialize(void)
{
    RSDK_EDITABLE_VAR(TippingPlatform, VAR_ENUM, childCount);
    RSDK_EDITABLE_VAR(TippingPlatform, VAR_UINT8, interval);
    RSDK_EDITABLE_VAR(TippingPlatform, VAR_UINT8, intervalOffset);
    RSDK_EDITABLE_VAR(TippingPlatform, VAR_UINT8, duration);
    RSDK_EDITABLE_VAR(TippingPlatform, VAR_UINT8, bossID);
}
} // namespace GameLogic