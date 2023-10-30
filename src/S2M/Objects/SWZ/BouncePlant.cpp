// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: BouncePlant Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "BouncePlant.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(BouncePlant);

void BouncePlant::Update()
{
    if (this->stood) {
        if (this->instantRecoil) {
            this->depression = 0x100;
        }
        else {
            if (this->depression < 0x100)
                this->depression += 0x20;
        }
    }
    else {
        if (this->depression > 0)
            this->depression -= 0x10;
    }

    this->stood         = false;
    this->instantRecoil = false;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (abs(player->position.x - this->position.x) <= 0x320000 && (player->velocity.y >= 0 || player->onGround)) {
            Hitbox *playerHitbox = player->GetHitbox();

            if (this->stood) {
                int32 posY = BouncePlant::GetNodeY(player->position.x) + this->stoodPos.y - (playerHitbox->bottom << 16) - 0x40000;
                if (player->position.y > posY - 0x80000) {
                    player->velocity.x += Math::Sin256(this->angle) << 13 >> 8;
                    player->position.y    = posY;
                    player->velocity.y    = 0;
                    player->onGround      = true;
                    player->groundedStore = true;
                    player->angle         = 0;
                    player->collisionMode = CMODE_FLOOR;
                    player->groundVel     = player->velocity.x;
                    if (player->state.Matches(&Player::State_KnuxGlideSlide) || player->state.Matches(&Player::State_KnuxGlideDrop))
                        player->state.Set(&Player::State_Ground);
                }
            }
            else {
                int32 posY = BouncePlant::GetNodeStandY(player->position.x) + this->position.y - (playerHitbox->bottom << 16) - 0x40000;
                if (player->position.y > posY - 0x80000 && player->position.y < this->position.y + 0x400000) {
                    player->position.y = posY;
                    if (abs(player->velocity.x) > 0xC0000)
                        this->instantRecoil = true;

                    if (abs(player->position.x - this->centerX) >= abs(player->velocity.x)) {
                        player->velocity.x += Math::Sin256(this->angle) << 13 >> 8;
                        player->velocity.y    = 0;
                        player->onGround      = true;
                        player->groundedStore = true;
                        player->angle         = 0;
                        player->collisionMode = CMODE_FLOOR;
                        player->groundVel     = player->velocity.x;

                        if (player->state.Matches(&Player::State_KnuxGlideSlide) || player->state.Matches(&Player::State_KnuxGlideDrop)) {
                            player->state.Set(&Player::State_Ground);
                        }
                    }
                    else if (this->depression > 0xA0) {
                        if (this->direction == FLIP_NONE)
                            player->velocity.x = -0xB4000;
                        else
                            player->velocity.x = 0xB4000;
                        player->velocity.y = -0xB4000;
                        player->onGround   = false;
                        player->state.Set(&Player::State_Air);
                        player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, false, 0);
                        sVars->sfxBouncePlant.Play(false, 255);
                    }
                    else if (abs(player->groundVel) <= 0xC00000) {
                        player->velocity.x += Math::Sin256(this->angle) << 13 >> 8;
                        player->velocity.y    = 0;
                        player->onGround      = true;
                        player->groundedStore = true;
                        player->angle         = 0;
                        player->collisionMode = CMODE_FLOOR;
                        player->groundVel     = player->velocity.x;

                        if (player->state.Matches(&Player::State_KnuxGlideSlide) || player->state.Matches(&Player::State_KnuxGlideDrop)) {
                            player->state.Set(&Player::State_Ground);
                        }
                    }
                    else {
                        if (this->direction == FLIP_NONE)
                            player->velocity.x = -0xB4000;
                        else
                            player->velocity.x = 0xB4000;
                        player->velocity.y = -0xB4000;
                        player->onGround   = false;
                        player->state.Set(&Player::State_Air);
                        player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, false, 0);
                        sVars->sfxBouncePlant.Play(false, 255);
                    }
                    this->stood          = true;
                    this->recoilDuration = 60;
                    this->stoodPos.x     = player->position.x;
                    this->stoodPos.y     = this->position.y + BouncePlant::GetNodeStandY(player->position.x);
                }
            }
        }
    }

    if (this->stood) {
        for (int32 i = 0; i < BOUNCEPLANT_NODE_COUNT; ++i) this->drawPos[i].y = this->stoodPos.y + BouncePlant::GetNodeY(this->drawPos[i].x);
    }
    else {
        if (this->recoilDuration <= 0) {
            for (int32 i = 0; i < BOUNCEPLANT_NODE_COUNT; ++i) this->drawPos[i] = this->nodeStartPos[i];
        }
        else {
            this->recoilDuration--;

            for (int32 i = 0; i < BOUNCEPLANT_NODE_COUNT; ++i) {
                this->recoilVelocity[i] += ((this->nodeStartPos[i].y - this->drawPos[i].y) >> 3) - (this->recoilVelocity[i] >> 3);
                this->drawPos[i].y += this->recoilVelocity[i];
            }
        }
    }
}

void BouncePlant::LateUpdate() {}

void BouncePlant::StaticUpdate() {}

void BouncePlant::Draw()
{
    for (int32 i = 0; i < BOUNCEPLANT_NODE_COUNT; ++i) {
        this->nodeAnimator.DrawSprite(&this->drawPos[i], false);
        this->decorAnimators[i].DrawSprite(&this->drawPos[i], false);
    }
}

void BouncePlant::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        BouncePlant::SetupNodePositions();

        if (this->direction) {
            this->centerX = this->position.x - 0x180000;
            this->angle   = 0x40;
        }
        else {
            this->centerX = this->position.x + 0x180000;
            this->angle   = 0xC0;
        }

        this->nodeAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        for (int32 i = 0; i < BOUNCEPLANT_NODE_COUNT; ++i) {
            this->decorAnimators[i].SetAnimation(sVars->aniFrames, 1, true, Math::Rand(1, 8));
            this->drawPos[i].x = this->nodeStartPos[i].x;
            this->drawPos[i].y = this->nodeStartPos[i].y;
        }
    }
}

void BouncePlant::StageLoad()
{
    sVars->aniFrames.Load("SWZ/Plants.bin", SCOPE_STAGE);

    sVars->hitbox.left  = -50;
    sVars->hitbox.left  = -12;
    sVars->hitbox.right = 50;
    sVars->hitbox.right = 8;

    sVars->sfxBouncePlant.Get("SWZ/BouncePlant.wav");
}

void BouncePlant::SetupNodePositions()
{
    if (this->direction == FLIP_NONE) {
        int32 x = this->position.x - 0x2A0000;
        for (int32 i = 0; i < BOUNCEPLANT_NODE_COUNT; ++i) {
            this->nodeStartPos[i].x = x;
            x += 0xC0000;
        }
    }
    else {
        int32 x = this->position.x + 0x2A0000;
        for (int32 i = 0; i < BOUNCEPLANT_NODE_COUNT; ++i) {
            this->nodeStartPos[i].x = x;
            x -= 0xC0000;
        }
    }

    int32 y = this->position.y + 0x2A0000;
    for (int32 i = 0; i < BOUNCEPLANT_NODE_COUNT; ++i) {
        this->nodeStartPos[i].y = y;
        y -= 0xC0000;
    }
}

int32 BouncePlant::GetNodeStandY(int32 x)
{
    int32 dist = 0;
    int32 pos  = 0;
    if (this->direction == FLIP_NONE) {
        int32 right = this->position.x + 0x180000;
        dist        = this->position.x - x;
        if (x > right)
            pos = 3 * (right - x);
        else
            pos = (right - x) / 3;
    }
    else {
        int32 left = this->position.x - 0x180000;
        dist       = x - this->position.x;
        if (x < left)
            pos = 3 * (x - left);
        else
            pos = (x - left) / 3;
    }

    dist = CLAMP(dist, -0x320000, 0x320000);
    pos  = CLAMP(pos + 0x200000, -0x320000, 0x320000);
    return dist + ((this->depression * (pos - dist)) >> 8);
}

int32 BouncePlant::GetNodeY(int32 x)
{
    int32 y         = 0;
    int32 distanceX = 0;
    int32 distanceY = 0;
    if (this->direction) {
        y = (x - this->stoodPos.x) >> 15;
        if (x < this->stoodPos.x) {
            distanceX = (this->stoodPos.x - this->nodeStartPos[BOUNCEPLANT_NODE_COUNT - 1].x) >> 16;
            distanceY = (this->stoodPos.y - this->nodeStartPos[BOUNCEPLANT_NODE_COUNT - 1].y) >> 16;
        }
        else {
            distanceX = (this->nodeStartPos[0].x - this->stoodPos.x) >> 16;
            distanceY = (this->nodeStartPos[0].y - this->stoodPos.y) >> 16;
        }
    }
    else {
        y = (this->stoodPos.x - x) >> 15;
        if (x > this->stoodPos.x) {
            distanceX = (this->nodeStartPos[BOUNCEPLANT_NODE_COUNT - 1].x - this->stoodPos.x) >> 16;
            distanceY = (this->stoodPos.y - this->nodeStartPos[BOUNCEPLANT_NODE_COUNT - 1].y) >> 16;
        }
        else {
            distanceX = (this->stoodPos.x - this->nodeStartPos[0].x) >> 16;
            distanceY = (this->nodeStartPos[0].y - this->stoodPos.y) >> 16;
        }
    }

    if (distanceX > 0)
        y = y * distanceY / distanceX;
    return y << 15;
}

#if RETRO_INCLUDE_EDITOR
void BouncePlant::EditorDraw()
{
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    BouncePlant::SetupNodePositions();

    if (this->direction) {
        this->centerX = this->position.x - 0x180000;
        this->angle   = 0x40;
    }
    else {
        this->centerX = this->position.x + 0x180000;
        this->angle   = 0xC0;
    }

    this->nodeAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    for (int32 i = 0; i < BOUNCEPLANT_NODE_COUNT; ++i) {
        this->decorAnimators[i].SetAnimation(sVars->aniFrames, 1, true, 1);
        this->drawPos[i].x = this->nodeStartPos[i].x;
        this->drawPos[i].y = this->nodeStartPos[i].y;
    }

    BouncePlant::Draw();
}

void BouncePlant::EditorLoad()
{
    sVars->aniFrames.Load("SWZ/Plants.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip", FLIP_NONE);
    RSDK_ENUM_VAR("Flip X", FLIP_X);
}
#endif

void BouncePlant::Serialize() { RSDK_EDITABLE_VAR(BouncePlant, VAR_UINT8, direction); }

} // namespace GameLogic