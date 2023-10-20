// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Fan Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Fan.hpp"
#include "Global/Zone.hpp"
#include "Common/Platform.hpp"
#include "Common/Water.hpp"
#include "Common/Button.hpp"
#include "Helpers/MathHelpers.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Fan);

void Fan::Update()
{
    this->stateActivate.Run(this);
    this->stateDeactivate.Run(this);

    this->animator.Process();

    this->state.Run(this);
}

void Fan::LateUpdate() {}

void Fan::StaticUpdate()
{
    sVars->activePlayers = 0;

    if (Water::sVars) {
        int32 offsetH = 0;
        int32 offsetV = 0;

        for (auto water : GameObject::GetEntities<Water>(FOR_ACTIVE_ENTITIES))
        {
            if (water->state.Matches(&Water::BubbleFinishPopBehavior) && water->activePlayers) {
                for (auto fan : GameObject::GetEntities<Fan>(FOR_ACTIVE_ENTITIES))
                {
                    sVars->hitboxTop.top = (Math::Sin256(2 * Zone::sVars->timer) >> 5) - fan->size;
                    if (fan->state.Matches(&Fan::HandlePlayerInteractions_Top)) {
                        if (MathHelpers::PointInHitbox(fan->position.x, fan->position.y, water->position.x, water->position.y, fan->direction,
                                                      &sVars->hitboxTop)) {
                            offsetV -= 0x20000;
                        }
                    }

                    sVars->hitboxBottom.top = (Math::Sin256(2 * Zone::sVars->timer) >> 5) - fan->size;
                    if (fan->state.Matches(&Fan::HandlePlayerInteractions_Bottom)) {
                        if (MathHelpers::PointInHitbox(fan->position.x, fan->position.y, water->position.x, water->position.y, fan->direction,
                                                      &sVars->hitboxBottom)) {
                            offsetV += 0x20000;
                        }
                    }

                    sVars->hitboxSides.left = (Math::Sin256(2 * Zone::sVars->timer) >> 5) - fan->size;
                    if (fan->state.Matches(&Fan::HandlePlayerInteractions_Left)) {
                        if (MathHelpers::PointInHitbox(fan->position.x, fan->position.y, water->position.x, water->position.y, fan->direction,
                                                      &sVars->hitboxSides)) {
                            offsetH -= 0x20000;
                        }
                    }
                    if (fan->state.Matches(&Fan::HandlePlayerInteractions_Right)) {
                        if (MathHelpers::PointInHitbox(fan->position.x, fan->position.y, water->position.x, water->position.y, fan->direction,
                                                      &sVars->hitboxSides)) {
                            offsetH += 0x20000;
                        }
                    }
                }

                offsetH = CLAMP(offsetH, -0x20000, 0x20000);
                offsetV = CLAMP(offsetV, -0x20000, 0x20000);

                if (water->bubbleOffset.x < offsetH) {
                    water->bubbleOffset.x += 0x800;

                    if (water->bubbleOffset.x > offsetH)
                        water->bubbleOffset.x = offsetH;
                }

                if (water->bubbleOffset.x > offsetH) {
                    water->bubbleOffset.x -= 0x800;

                    if (water->bubbleOffset.x < offsetH)
                        water->bubbleOffset.x = offsetH;
                }

                if (water->bubbleOffset.y < offsetV) {
                    water->bubbleOffset.y += 0x800;

                    if (water->bubbleOffset.y > offsetV)
                        water->bubbleOffset.y = offsetV;
                }

                if (water->bubbleOffset.y > offsetV) {
                    water->bubbleOffset.y -= 0x800;

                    if (water->bubbleOffset.y < offsetV)
                        water->bubbleOffset.y = offsetV;
                }
            }
        }
    }
}

void Fan::Draw() { this->animator.DrawSprite(nullptr, false); }

void Fan::Create(void *data)
{
    this->drawFX = FX_FLIP;

    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0] + 1;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;

        if (this->type == FAN_V)
            this->direction *= FLIP_Y;

        if (!this->size)
            this->size = 5;

        this->size *= 16;
        this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);
        this->state.Set(&Fan::State_Stopped);
        this->animator.speed = 0;

        switch (this->activation) {
            case FAN_ACTIVATE_NONE:
                Fan::Activate();
                this->animator.speed = 128;
                break;

            case FAN_ACTIVATE_INTERVAL: this->stateActivate.Set(&Fan::Activate_Interval); break;
                                                   
            case FAN_ACTIVATE_PLATFORM: this->stateActivate.Set(&Fan::Activate_Platform); break;

            case FAN_ACTIVATE_BUTTON:
                Fan::SetupTagLink();
                this->stateActivate.Set(&Fan::Activate_Button);
                break;

            default: break;
        }

        if (this->deactivation == FAN_DEACTIVATE_BUTTON) {
            Fan::SetupTagLink();
            this->stateDeactivate.Set(&Fan::Deactivate_Button);
        }
    }
}

void Fan::StageLoad()
{
    sVars->aniFrames.Load("OOZ/Fan.bin", SCOPE_STAGE);

    sVars->hitboxTop.left  = -64;
    sVars->hitboxTop.right = 64;

    sVars->unused      = 5;
    sVars->minVelocity = -0x50000;

    sVars->hitboxBottom.left   = -16;
    sVars->hitboxBottom.right  = 16;
    sVars->hitboxBottom.top    = -160;
    sVars->hitboxBottom.bottom = 32;

    sVars->hitboxSides.top    = -160;
    sVars->hitboxSides.bottom = 112;
    sVars->hitboxSides.left   = -112;
    sVars->hitboxSides.right  = 32;

    sVars->playerHitbox.left   = -1;
    sVars->playerHitbox.top    = -1;
    sVars->playerHitbox.right  = 1;
    sVars->playerHitbox.bottom = 1;
}

void Fan::SetupTagLink()
{
    this->taggedButton         = nullptr;
    Button *taggedButton       = GameObject::Get<Button>(RSDKTable->GetEntitySlot(this) - 1);

    if (this->buttonTag > 0) {
        bool32 matchedTag = false;

        if (Button::sVars) {
            for (auto button : GameObject::GetEntities<Button>(FOR_ALL_ENTITIES))
            {
                if (button->tag == this->buttonTag) {
                    taggedButton = button;
                    matchedTag   = true;
                }
            }
        }
    }

    if (taggedButton) {
        if (Button::sVars && taggedButton->classID == Button::sVars->classID) {
            int32 distX = abs(this->position.x - taggedButton->position.x) + 0x800000;
            int32 distY = abs(this->position.y - taggedButton->position.y) + 0x800000;

            if (this->updateRange.x < distX)
                this->updateRange.x = distX;

            if (this->updateRange.y < distY)
                this->updateRange.y = distY;

            this->taggedButton = taggedButton;
        }
    }
}

void Fan::HandlePlayerInteractions_Top()
{
    sVars->hitboxTop.top    = (Math::Sin256(2 * Zone::sVars->timer) >> 5) - this->size;
    sVars->hitboxTop.bottom = 48;

    int32 playerID = 1;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (!player->state.Matches(&Player::State_Static)) {
            int32 anim = player->animator.animationID;
            if (anim != Player::ANI_HURT && anim != Player::ANI_DIE && anim != Player::ANI_DROWN
                && this->CheckCollisionTouchBox(&sVars->hitboxTop, player, &sVars->playerHitbox)) {
                if (Water::sVars && player->position.y > Water::sVars->waterLevel)
                    player->animator.SetAnimation(player->aniFrames, Player::ANI_FAN, false, 1);
                else
                    player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, false, 1);

                player->state.Set(&Player::State_Air);
                player->tileCollisions = TILECOLLISION_DOWN;
                player->onGround       = false;

                int32 velocity = MAX((this->position.y + (sVars->hitboxTop.top << 16) - player->position.y) >> 4, sVars->minVelocity);
                if (player->velocity.y <= velocity) {
                    player->velocity.y = velocity;
                }
                else {
                    player->velocity.y = player->velocity.y + (velocity >> 2) + (velocity >> 1);
                    if (player->velocity.y < velocity)
                        player->velocity.y = velocity;
                }

                if (!(playerID & sVars->activePlayers)) {
                    if (player->velocity.y > -0x40000 && player->velocity.y < 0)
                        player->velocity.x += (32 * player->velocity.x / 31) >> 5;

                    sVars->activePlayers |= playerID;
                }
            }
        }

        playerID <<= 1;
    }

    if (Water::sVars) {
        if (this->position.y > Water::sVars->waterLevel && !(Zone::sVars->timer & 3)) {
            Water *water = GameObject::Create<Water>(INT_TO_VOID(Water::Bubble), this->position.x, this->position.y - 0x100000);

            water->isPermanent = false;
            water->position.x += Math::Rand(-6, 7) << 16;
            water->bubbleX    = water->position.x;
            water->velocity.y = -0x40000;
            water->childPtr   = nullptr;
        }
    }

    Fan::State_Started();
}

void Fan::HandlePlayerInteractions_Bottom()
{
    sVars->hitboxBottom.bottom = this->size - (Math::Sin256(2 * Zone::sVars->timer) >> 5);

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (!player->state.Matches(&Player::State_Static)) {
            int32 anim = player->animator.animationID;

            if (anim != Player::ANI_HURT && anim != Player::ANI_DIE && anim != Player::ANI_DROWN
                && this->CheckCollisionTouchBox(&sVars->hitboxBottom, player, &sVars->playerHitbox)) {
                int32 max = (this->position.y - player->position.y + 0xA00000) >> 4;
                if (player->velocity.y < max)
                    player->velocity.y += ((this->position.y - player->position.y + 0xA00000) >> 9);
            }
        }
    }

    Fan::State_Started();
}

void Fan::HandlePlayerInteractions_Left()
{
    sVars->hitboxSides.left = (Math::Sin256(2 * Zone::sVars->timer) >> 5) - this->size;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (!player->state.Matches(&Player::State_Static)) {
            int32 anim = player->animator.animationID;

            if (anim != Player::ANI_HURT && anim != Player::ANI_DIE && anim != Player::ANI_DROWN && player->collisionMode != CMODE_LWALL
                && this->CheckCollisionTouchBox(&sVars->hitboxSides, player, &sVars->playerHitbox)) {
                player->position.x += (this->position.x - player->position.x - 0xA00000) >> 4;
            }
        }
    }

    Fan::State_Started();
}

void Fan::HandlePlayerInteractions_Right()
{
    sVars->hitboxSides.left = (Math::Sin256(2 * Zone::sVars->timer) >> 5) - this->size;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (!player->state.Matches(&Player::State_Static)) {
            int32 anim = player->animator.animationID;

            if (anim != Player::ANI_HURT && anim != Player::ANI_DIE && anim != Player::ANI_DROWN) {
                if (player->collisionMode != CMODE_LWALL && player->collisionMode != CMODE_RWALL
                    && this->CheckCollisionTouchBox(&sVars->hitboxSides, player, &sVars->playerHitbox)) {
                    player->position.x += (this->position.x - player->position.x + 0xA00000) >> 4;
                }
            }
        }
    }

    Fan::State_Started();
}

void Fan::State_Started()
{
    if (this->animator.speed < 0x80)
        this->animator.speed += 4;
}

void Fan::State_Stopped()
{
    if (this->animator.speed)
        this->animator.speed -= 2;
}

void Fan::Activate_Interval()
{
    if (this->delay) {
        if (!--this->delay)
            this->state.Set(&Fan::State_Stopped);
    }
    else {
        if (!((Zone::sVars->timer + this->intervalOffset) % this->interval) && this->state.Matches(&Fan::State_Stopped)) {
            this->active = ACTIVE_NORMAL;
            Fan::Activate();
        }
    }
}

void Fan::Activate_Button()
{
    Button *button = this->taggedButton;
    if ((!button || button->activated) && this->state.Matches(&Fan::State_Stopped)) {
        this->active = ACTIVE_NORMAL;
        Fan::Activate();
    }
}

void Fan::Deactivate_Button()
{
    Button *button = this->taggedButton;
    if ((!button || button->activated) && !this->state.Matches(&Fan::State_Stopped)) {
        this->active = ACTIVE_BOUNDS;
        this->state.Set(&Fan::State_Stopped);
    }
}

void Fan::Activate_Platform()
{
    int32 slot               = sceneInfo->entitySlot - 1;
    Platform *platform = GameObject::Get<Platform>(slot);
    while (platform->classID == sVars->classID) {
        --slot;
        platform = GameObject::Get<Platform>(slot);
    }

    if (platform->classID == Platform::sVars->classID) {
        if (platform->amplitude.y == platform->amplitude.x)
            this->state.Set(&Fan::State_Stopped);
        else if (this->state.Matches(&Fan::State_Stopped))
            Fan::Activate();
    }
}

void Fan::Activate()
{
    if (this->type != FAN_V) {
        if (this->direction == FLIP_NONE)
            this->state.Set(&Fan::HandlePlayerInteractions_Left);
        else
            this->state.Set(&Fan::HandlePlayerInteractions_Right);

        this->delay = this->duration;
    }
    else {
        if (this->direction == FLIP_NONE)
            this->state.Set(&Fan::HandlePlayerInteractions_Top);
        else
            this->state.Set(&Fan::HandlePlayerInteractions_Bottom);

        this->delay = this->duration;
    }
}

#if RETRO_INCLUDE_EDITOR
void Fan::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);

    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;

    int32 dir = this->direction;
    if (this->type == FAN_V)
        this->direction *= FLIP_Y;

    Fan::Draw();

    this->direction = dir;

    if (showGizmos()) {
        if (this->activation == FAN_ACTIVATE_BUTTON || this->deactivation == FAN_DEACTIVATE_BUTTON) {
            Fan::SetupTagLink();

            RSDK_DRAWING_OVERLAY(true);
            if (this->taggedButton) {
                DrawHelpers::DrawArrow(this->taggedButton->position.x, this->taggedButton->position.y, this->position.x, this->position.y, 0xFFFF00,
                                      INK_NONE, 0xFF);
            }
            RSDK_DRAWING_OVERLAY(false);
        }
    }
}

void Fan::EditorLoad()
{
    sVars->aniFrames .Load("OOZ/Fan.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Vertical", FAN_V);
    RSDK_ENUM_VAR("Horiozontal", FAN_H);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip", FLIP_NONE);
    RSDK_ENUM_VAR("Flipped", FLIP_X);

    RSDK_ACTIVE_VAR(sVars, activation);
    RSDK_ENUM_VAR("None", FAN_ACTIVATE_NONE);
    RSDK_ENUM_VAR("On Interval", FAN_ACTIVATE_INTERVAL);
    RSDK_ENUM_VAR("On Platform Moved", FAN_ACTIVATE_PLATFORM);
    RSDK_ENUM_VAR("On Button Press", FAN_ACTIVATE_BUTTON);

    RSDK_ACTIVE_VAR(sVars, deactivation);
    RSDK_ENUM_VAR("None", FAN_DEACTIVATE_NONE);
    RSDK_ENUM_VAR("On Button Press", FAN_DEACTIVATE_BUTTON);
}
#endif

void Fan::Serialize()
{
    RSDK_EDITABLE_VAR(Fan, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(Fan, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(Fan, VAR_UINT8, activation);
    RSDK_EDITABLE_VAR(Fan, VAR_UINT8, deactivation);
    RSDK_EDITABLE_VAR(Fan, VAR_ENUM, size);
    RSDK_EDITABLE_VAR(Fan, VAR_UINT16, interval);
    RSDK_EDITABLE_VAR(Fan, VAR_UINT16, intervalOffset);
    RSDK_EDITABLE_VAR(Fan, VAR_UINT16, duration);
    RSDK_EDITABLE_VAR(Fan, VAR_ENUM, buttonTag);
}
} // namespace GameLogic