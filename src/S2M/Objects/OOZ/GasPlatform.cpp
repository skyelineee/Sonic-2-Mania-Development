// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: GasPlatform Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "GasPlatform.hpp"
#include "Global/Zone.hpp"
#include "Common/Platform.hpp"

// will forever be incomplete bc idk how to deal with the shared platform stuff the original object does in mania so no collision ig
// nvm its done now i figured it out
using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(GasPlatform);

void GasPlatform::Update()
{
    if (this->state.Matches(&Platform::State_Fixed)) {
        switch (this->type) {
            case GASPLATFORM_INTERVAL:
                if (!((Zone::sVars->timer + this->intervalOffset) % this->interval))
                    GasPlatform::PopPlatform();
                break;

            case GASPLATFORM_CONTACT: {
                int32 playerID = 0;
                for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
                {
                    if ((1 << playerID) & this->stoodPlayers && !player->sidekick) {
                        if (abs(player->position.x - this->position.x) < 0x40000) {
                            player->position.x = this->position.x;
                            player->state.Set(&Player::State_Static);
                            player->velocity.x = 0;
                            player->groundVel  = 0;

                            GasPlatform::PopPlatform();
                        }
                    }

                    ++playerID;
                }
                break;
            }
        }
    }

    Platform *platform = (Platform *)this;
    platform->Update();
}

void GasPlatform::LateUpdate() {}

void GasPlatform::StaticUpdate() {}

void GasPlatform::Draw()
{
    if (this->centerPos.y - this->drawPos.y > 0x180000)
        this->gasAnimator.DrawSprite(nullptr, false);

    this->animator.DrawSprite(&this->drawPos, false);
}

void GasPlatform::Create(void *data)
{
    if (!this->interval)
        this->interval = 1;

    this->frameID   = 2;
    this->collision = Platform::C_Platform;
    Platform *platform = (Platform *)this;
    platform->Create(nullptr);
    this->gasAnimator.SetAnimation(Platform::sVars->aniFrames, 2, true, 0);

    this->stateCollide.Set(&Platform::Collision_Solid);
    this->state.Set(&Platform::State_Fixed);
}

void GasPlatform::StageLoad(void)
{
    sVars->hitboxGas.top    = -16;
    sVars->hitboxGas.left   = -16;
    sVars->hitboxGas.right  = 16;
    sVars->hitboxGas.bottom = 0;

    sVars->range.x = 0x800000;
    sVars->range.y = 0x180000;

    sVars->sfxGasPop.Get("OOZ/GasPop.wav");
    sVars->sfxSpring.Get("Global/Spring.wav");
}

void GasPlatform::PopPlatform()
{
    if (this->CheckOnScreen(&sVars->range))
        sVars->sfxGasPop.Play(false, 255);

    this->active     = ACTIVE_NORMAL;
    this->velocity.y = -0x96800;
    this->state.Set(&GasPlatform::State_Popped);
}

void GasPlatform::State_Popped()
{
    this->drawPos.y += this->velocity.y;

    if (this->drawPos.y >= this->centerPos.y) {
        this->drawPos.y = this->centerPos.y;
        if (this->velocity.y <= 0x10000) {
            this->active = ACTIVE_BOUNDS;
            this->state.Set(&Platform::State_Fixed);
            return;
        }

        this->velocity.y = -(this->velocity.y >> 2);
    }
    else {
        this->velocity.y += 0x3800;
    }

    if (this->centerPos.y - this->drawPos.y > 0x180000) {
        this->gasAnimator.Process();
        int32 storeX     = this->position.x;
        int32 storeY     = this->position.y;
        this->position.x = this->centerPos.x;
        this->position.y = this->centerPos.y;

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (player->CheckCollisionTouch(this, &sVars->hitboxGas)) {
                // player->ElementHurt(this, Player::Shield_Fire);
                if (player->shield != Player::Shield_Fire) {
                    player->Hurt(this);
                }
            }
        }

        this->position.x = storeX;
        this->position.y = storeY;
    }

    if (this->type == GASPLATFORM_CONTACT) {
        int32 playerID = 0;
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            int32 launchY = this->centerPos.y - 0x780000;

            if (this->position.y <= launchY) {
                this->position.y = launchY;
                if ((1 << playerID) & this->stoodPlayers) {
                    player->velocity.y = -0x100000;
                    player->state.Set(&Player::State_Air);
                    player->onGround   = false;
                    player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                    sVars->sfxSpring.Play(false, 255);
                    this->timer = 240;
                    this->state.Set(&GasPlatform::State_SpringCooldown);
                }
            }
            else if ((1 << playerID) & this->stoodPlayers) {
                if (abs(player->position.x - this->position.x) < 0x40000) {
                    player->position.x = this->position.x;
                    player->state.Set(&Player::State_Static);
                    player->velocity.x = 0;
                    player->groundVel  = 0;
                }
            }

            ++playerID;
        }
    }
}

void GasPlatform::State_SpringCooldown()
{
    this->gasAnimator.Process();

    int32 storeX = this->position.x;
    int32 storeY = this->position.y;

    this->position.x = this->centerPos.x;
    this->position.y = this->centerPos.y;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckCollisionTouch(this, &sVars->hitboxGas)) {
            // player->ElementHurt(this, Player::Shield_Fire);
            if (player->shield != Player::Shield_Fire) {
                player->Hurt(this);
            }
        }
    }

    this->position.x = storeX;
    this->position.y = storeY;

    if (!--this->timer) {
        this->velocity.y = 0;
        this->state.Set(&GasPlatform::State_Popped);
    }
}

void GasPlatform::State_Shaking()
{
    this->drawPos.x = (Math::Rand(-1, 1) << 16) + this->centerPos.x;
    this->drawPos.y = (Math::Rand(-2, 2) << 16) + this->centerPos.y;

    if (this->timer <= 0) {
        sVars->sfxGasPop.Play(false, 255);
        this->active     = ACTIVE_NORMAL;
        this->velocity.y = -0x8C000;
        this->state.Set(&GasPlatform::State_Popped);
    }
    else {
        this->timer--;
    }
}

#if RETRO_INCLUDE_EDITOR
void GasPlatform::EditorDraw()
{
    this->drawPos = this->position;

    GasPlatform::Draw();
}

void GasPlatform::EditorLoad()
{
    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Pop On Interval", GASPLATFORM_INTERVAL);
    RSDK_ENUM_VAR("Pop On Contact", GASPLATFORM_CONTACT); // never used in mania afaik, it's used in S2 though
}
#endif

void GasPlatform::Serialize()
{
    RSDK_EDITABLE_VAR(GasPlatform, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(GasPlatform, VAR_ENUM, childCount);
    RSDK_EDITABLE_VAR(GasPlatform, VAR_UINT16, interval);
    RSDK_EDITABLE_VAR(GasPlatform, VAR_UINT16, intervalOffset);
}
} // namespace GameLogic