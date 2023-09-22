// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: PopOut Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "PopOut.hpp"
#include "Global/Spring.hpp"
#include "Global/Spikes.hpp"
#include "Global/Zone.hpp"
#include "Common/Button.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(PopOut);

void PopOut::Update()
{
    int32 storeDir = this->direction;
    int32 storeX   = this->position.x;
    int32 storeY   = this->position.y;

    Spring *child = GameObject::Get<Spring>(sceneInfo->entitySlot + 1);
    if (child->classID != Spring::sVars->classID && child->classID != Spikes::sVars->classID)
        child = nullptr;

    this->direction = FLIP_NONE;
    if (!this->manualTrigger) {
        this->shouldAppear = false;
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (player->CheckCollisionTouch(this, &this->hitboxRange)) {
                this->shouldAppear = true;
                break;
            }
        }
    }
    else if (sVars->hasButton) {
        for (auto button : GameObject::GetEntities<Button>(FOR_ACTIVE_ENTITIES))
        {
            if (button->tag == this->tag && button->activated)
                this->shouldAppear = true;
        }
    }

    if (this->shouldAppear) {
        if (this->timer < this->delay) {
            this->timer++;
        }
        else {
            if (this->appearTimer < 8) {
                this->appearTimer++;
                if (this->childType <= POPOUT_CHILD_SPRING_YELLOW && child && this->appearTimer == 8) {
                    child->sfxTimer         = 0;
                    child->animator.speed   = 0;
                    child->animator.frameID = 0;
                }
            }
        }
    }
    else {
        this->timer = 0;
        if (this->appearTimer > 0) {
            this->appearTimer--;
            if (this->childType <= POPOUT_CHILD_SPRING_YELLOW && child && this->appearTimer == 7) {
                child->sfxTimer         = 0;
                child->animator.speed   = 0;
                child->animator.frameID = 0;
            }
        }
    }

    if (child) {
        if (this->appearTimer == 8) {
            child->active  = ACTIVE_BOUNDS;
            child->visible = true;
        }
        else {
            child->active  = ACTIVE_NEVER;
            child->visible = false;
        }
    }

    this->active = (this->appearTimer <= 0) ? ACTIVE_BOUNDS : ACTIVE_NORMAL;

    this->position.x += -32 * this->appearVelocity.x + 4 * this->appearTimer * this->appearVelocity.x;
    this->position.y += -32 * this->appearVelocity.y + 4 * this->appearTimer * this->appearVelocity.y;
    for (auto playerLoop : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) { playerLoop->CheckCollisionBox(this, &this->hitboxSolid); }
    this->direction  = storeDir;
    this->position.x = storeX;
    this->position.y = storeY;
}

void PopOut::LateUpdate() {}

void PopOut::StaticUpdate() {}

void PopOut::Draw()
{
    int32 dir        = this->direction;
    int32 storeX     = this->position.x;
    int32 storeY     = this->position.y;
    this->position.x = -16 * this->appearVelocity.x + this->position.x + 2 * this->appearTimer * this->appearVelocity.x;
    this->position.y = -16 * this->appearVelocity.y + this->position.y + 2 * this->appearTimer * this->appearVelocity.y;
    this->rotation   = this->orientation << 7;

    if (!this->appearTimer) {
        int32 off = 128;
        if (!dir)
            off = -128;
        this->rotation += off;
    }
    else if (this->appearTimer < 8) {
        int32 off = 16;
        if (!dir)
            off = -16;
        this->rotation += (7 - this->appearTimer) * off;
    }

    this->mountAnimator.DrawSprite(nullptr, false);

    if (this->appearTimer < 8)
        this->childAnimator.DrawSprite(nullptr, false);

    this->position.x = storeX;
    this->position.y = storeY;
    this->direction  = dir;
    this->rotation   = 0;
}

void PopOut::Create(void *data)
{
    this->drawFX = FX_ROTATE | FX_FLIP;

    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->startPos.x    = this->position.x;
        this->startPos.y    = this->position.y;
        this->visible       = true;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->mountAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);

        Spring *child = GameObject::Get<Spring>(sceneInfo->entitySlot + 1);
        if (child->classID != Spring::sVars->classID && child->classID != Spikes::sVars->classID)
            child = nullptr;

        if (child->classID == Spring::sVars->classID) {
            this->childType = (child->type & 1) ? POPOUT_CHILD_SPRING_RED : POPOUT_CHILD_SPRING_YELLOW;
            this->childAnimator.SetAnimation(sVars->aniFrames, 1, true, this->childType);
        }
        else {
            // Assume its spikes
            this->childType = POPOUT_CHILD_SPIKES;
            this->childAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
        }
        PopOut::SetupHitboxes();

        switch (this->orientation + 4 * this->direction) {
            case 0:
            case 4:
                this->appearVelocity.x = 0;
                this->appearVelocity.y = -0x10000;
                break;

            case 1:
            case 5:
                this->appearVelocity.y = 0;
                this->appearVelocity.x = 0x10000;
                break;

            case 2:
            case 6:
                this->appearVelocity.x = 0;
                this->appearVelocity.y = 0x10000;
                break;

            default:
            case 3:
            case 7:
                this->appearVelocity.y = 0;
                this->appearVelocity.x = -0x10000;
                break;
        }
    }
}

void PopOut::StageLoad()
{
    sVars->aniFrames.Load("DEZ/PopOut.bin", SCOPE_STAGE);

    if (RSDKTable->FindObject("Button"))
        sVars->hasButton = true;
}

void PopOut::SetupHitboxes()
{
    this->hitboxRange.left   = 0;
    this->hitboxRange.top    = 0;
    this->hitboxRange.right  = 0;
    this->hitboxRange.bottom = 0;

    this->hitboxSolid.left   = 0;
    this->hitboxSolid.top    = 0;
    this->hitboxSolid.right  = 0;
    this->hitboxSolid.bottom = 0;

    switch (this->orientation + 4 * this->direction) {
        case 0:
            this->hitboxRange.right  = 80;
            this->hitboxRange.top    = -64;
            this->hitboxRange.bottom = 32;

            this->hitboxSolid.left = -32;
            this->hitboxSolid.top  = -32;
            break;

        case 1:
            this->hitboxRange.right  = 80;
            this->hitboxRange.bottom = 64;
            this->hitboxRange.left   = -32;

            this->hitboxSolid.top   = -32;
            this->hitboxSolid.right = 32;
            break;

        case 2:
            this->hitboxRange.left   = -80;
            this->hitboxRange.top    = -32;
            this->hitboxRange.bottom = 64;

            this->hitboxSolid.right  = 32;
            this->hitboxSolid.bottom = 32;
            break;

        case 3:
            this->hitboxRange.left  = -64;
            this->hitboxRange.top   = -80;
            this->hitboxRange.right = 32;

            this->hitboxSolid.left   = -32;
            this->hitboxSolid.bottom = 32;
            break;

        case 4:
            this->hitboxRange.left   = -80;
            this->hitboxRange.top    = -63;
            this->hitboxRange.bottom = 32;

            this->hitboxSolid.top   = -32;
            this->hitboxSolid.right = 32;
            break;

        case 5:
            this->hitboxRange.top   = -80;
            this->hitboxRange.right = 64;
            this->hitboxRange.left  = -32;

            this->hitboxSolid.right  = 32;
            this->hitboxSolid.bottom = 32;
            break;

        case 6:
            this->hitboxRange.right  = 80;
            this->hitboxRange.bottom = 64;
            this->hitboxRange.top    = -32;

            this->hitboxSolid.left   = -32;
            this->hitboxSolid.bottom = 32;
            break;

        default:
        case 7:
            this->hitboxRange.right  = 80;
            this->hitboxRange.bottom = 32;
            this->hitboxRange.left   = -64;

            this->hitboxSolid.left = -32;
            this->hitboxSolid.top  = -32;
            break;
    }
}

#if RETRO_INCLUDE_EDITOR
void PopOut::EditorDraw()
{
    sVars->hasButton = false;
    if (RSDKTable->FindObject("Button"))
        sVars->hasButton = true;

    this->mountAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);

    Spring *child = GameObject::Get<Spring>(sceneInfo->entitySlot + 1);
    if (child->classID != Spring::sVars->classID && child->classID != Spikes::sVars->classID)
        child = nullptr;

    if (child && child->classID == Spring::sVars->classID) {
        this->childType = (child->type & 1) ? POPOUT_CHILD_SPRING_RED : POPOUT_CHILD_SPRING_YELLOW;
        this->childAnimator.SetAnimation(sVars->aniFrames, 1, true, this->childType);
    }
    else {
        this->childType = POPOUT_CHILD_SPIKES;
        this->childAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    }

    this->appearTimer      = 0;
    this->appearVelocity.x = 0;
    this->appearVelocity.y = 0;

    int32 storeX = this->position.x;
    int32 storeY = this->position.y;

    PopOut::Draw();

    if (showGizmos()) {
        PopOut::SetupHitboxes();

        switch (this->orientation + 4 * this->direction) {
            case 0:
            case 4:
                this->appearVelocity.x = 0;
                this->appearVelocity.y = -0x10000;
                break;

            case 1:
            case 5:
                this->appearVelocity.y = 0;
                this->appearVelocity.x = 0x10000;
                break;

            case 2:
            case 6:
                this->appearVelocity.x = 0;
                this->appearVelocity.y = 0x10000;
                break;

            default:
            case 3:
            case 7:
                this->appearVelocity.y = 0;
                this->appearVelocity.x = -0x10000;
                break;
        }

        DrawHelpers::DrawHitboxOutline(this->position.x, this->position.y, &this->hitboxRange, FLIP_NONE, 0xFF0000);

        this->appearTimer = 8;
        this->position.x  = -16 * this->appearVelocity.x + this->position.x + 2 * this->appearTimer * this->appearVelocity.x;
        this->position.y  = -16 * this->appearVelocity.y + this->position.y + 2 * this->appearTimer * this->appearVelocity.y;
        this->inkEffect   = INK_BLEND;

        PopOut::Draw();

        this->inkEffect  = INK_NONE;
        this->position.x = storeX;
        this->position.y = storeY;

        if (this->manualTrigger && sVars->hasButton) {
            Button *taggedButton = nullptr;
            for (auto button : GameObject::GetEntities<Button>(FOR_ALL_ENTITIES))
            {
                if (button->tag == this->tag) {
                    taggedButton = button;
                    break;
                }
            }

            if (taggedButton) {
                RSDK_DRAWING_OVERLAY(true);
                if (taggedButton) {
                    DrawHelpers::DrawArrow(taggedButton->position.x, taggedButton->position.y, this->position.x, this->position.y, 0xFFFF00, INK_NONE,
                                          0xFF);
                }
                RSDK_DRAWING_OVERLAY(false);
            }
        }
    }
}

void PopOut::EditorLoad() { sVars->aniFrames.Load("DEZ/PopOut.bin", SCOPE_STAGE); }
#endif

void PopOut::Serialize()
{
    RSDK_EDITABLE_VAR(PopOut, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(PopOut, VAR_UINT8, orientation);
    RSDK_EDITABLE_VAR(PopOut, VAR_UINT8, tag);
    RSDK_EDITABLE_VAR(PopOut, VAR_BOOL, manualTrigger);
    RSDK_EDITABLE_VAR(PopOut, VAR_ENUM, delay);
}

} // namespace GameLogic