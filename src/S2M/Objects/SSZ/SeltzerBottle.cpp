// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SeltzerBottle Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SeltzerBottle.hpp"
#include "SeltzerWater.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Common/PlatformNode.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SeltzerBottle);

void SeltzerBottle::Update()
{
    this->waterAnimator.Process();

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        player->CheckCollisionBox(this, &sVars->hitboxBottle);

        if (player->CheckCollisionPlatform(this, &sVars->hitboxButton) && !this->buttonAnimator.frameID) {
            this->buttonAnimator.frameID = 1;
            this->active                 = ACTIVE_NORMAL;
            this->state.Set(&SeltzerBottle::State_Spraying);
            this->sprayAnimator.SetAnimation(sVars->aniFrames, 6, false, 0);
            player->groundVel = CLAMP(player->groundVel, -0xC0000, 0xC0000);

            for (int32 p = 0; p < Player::sVars->playerCount; ++p) GameObject::Get<Player>(p)->collisionLayers |= sVars->seltzerPathLayerMask;

            sVars->sfxSpray.Play(false, 0xFF);
        }
    }

    this->state.Run(this);
}

void SeltzerBottle::LateUpdate() {}

void SeltzerBottle::StaticUpdate() {}

void SeltzerBottle::Draw()
{
    Graphics::DrawRect(this->position.x - 0x2E0000, this->position.y - this->waterLevel + 0x2C0000, 0x5C0000, this->waterLevel, 0x00F0F0, 0x40, INK_SUB,
                  false);

    Vector2 drawPos = this->position;
    drawPos.y += 0x2C0000 - this->waterLevel;
    this->inkEffect = INK_ADD;
    this->waterAnimator.DrawSprite(&drawPos, false);

    this->bottleAnimator.frameID = 0;
    this->inkEffect              = INK_SUB;
    this->bottleAnimator.DrawSprite(nullptr, false);

    this->bottleAnimator.frameID = 1;
    this->inkEffect              = INK_ADD;
    this->bottleAnimator.DrawSprite(nullptr, false);

    this->bottleAnimator.frameID = 2;
    this->inkEffect              = INK_NONE;
    this->bottleAnimator.DrawSprite(nullptr, false);

    this->bottleAnimator.frameID = 3;
    this->bottleAnimator.DrawSprite(nullptr, false);

    this->bottleAnimator.frameID = 4;
    this->bottleAnimator.DrawSprite(nullptr, false);

    this->bottleAnimator.frameID = 5;
    this->drawFX                 = FX_FLIP;
    this->sprayAnimator.DrawSprite(nullptr, false);

    this->bottleAnimator.DrawSprite(nullptr, false);

    this->drawFX = FX_NONE;
    this->buttonAnimator.DrawSprite(nullptr, false);
}

void SeltzerBottle::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->bottleAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->buttonAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->waterAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);

        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[1];
        this->alpha         = 0xFF;
        this->timer         = this->sprayTime;
        this->waterLevel    = 0x400000;
    }
}

void SeltzerBottle::StageLoad()
{
    sVars->aniFrames.Load("SSZ/Seltzer.bin", SCOPE_STAGE);
    sVars->nullFrames.Load("", SCOPE_STAGE);

    sVars->hitboxBottle.left   = -48;
    sVars->hitboxBottle.top    = -80;
    sVars->hitboxBottle.right  = 48;
    sVars->hitboxBottle.bottom = 64;

    sVars->hitboxButton.left   = -16;
    sVars->hitboxButton.top    = -90;
    sVars->hitboxButton.right  = 16;
    sVars->hitboxButton.bottom = sVars->hitboxButton.top + 16;

    sVars->seltzerPathLayer.Get("Seltzer Path");
    if (sVars->seltzerPathLayer.id)
        sVars->seltzerPathLayerMask = 1 << sVars->seltzerPathLayer.id;

    sVars->sfxSpray.Get("Stage/Spray.wav");
}

void SeltzerBottle::State_Spraying()
{
    this->waterLevel = MAX(((this->timer << 14) / this->sprayTime) << 8, 0x40000);

    SeltzerWater *spray = GameObject::Create<SeltzerWater>(nullptr, this->position.x, this->position.y - 0x4C0000);
    spray->position.x += this->direction == FLIP_X ? 0x300000 : -0x300000;
    spray->oscillateRadius = Math::Rand(0xA00, 0xC00);
    spray->offsetAngle     = Math::Rand(0, 0x100);
    spray->drawGroup       = Zone::sVars->playerDrawGroup[0];
    spray->nodeSlot        = sceneInfo->entitySlot + 1;

    int32 storeX = this->position.x;
    int32 storeY = this->position.y;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->onGround) {
            this->position.x = player->position.x + (Math::Sin256(player->angle) << 13);
            this->position.y = player->position.y + (Math::Cos256(player->angle) << 13);

            if (this->TileCollision(sVars->seltzerPathLayerMask, player->collisionMode, 0, 0, 0, false)) {
                if (this->direction == FLIP_X) {
                    if (player->groundVel < 0x50000)
                        player->groundVel = 0x50000;
                }
                else {
                    if (player->groundVel > -0x50000)
                        player->groundVel = -0x50000;
                }
            }
        }
    }

    this->position.x = storeX;
    this->position.y = storeY;

    this->sprayAnimator.Process();

    if (--this->timer <= 0) {
        for (int32 p = 0; p < Player::sVars->playerCount; ++p) GameObject::Get<Player>(p)->collisionLayers &= ~sVars->seltzerPathLayerMask;

        this->sprayAnimator.SetAnimation(sVars->nullFrames, 0, false, 0);
        this->state.Set(&SeltzerBottle::State_TryReset);
        for (auto water : GameObject::GetEntities<SeltzerWater>(FOR_ACTIVE_ENTITIES))
        {
            water->gravityStrength = Math::Rand(0x3800, 0x4000);
            water->state.Set(&SeltzerWater::State_Falling);
        }
    }
}

void SeltzerBottle::State_TryReset()
{
    if (!this->CheckOnScreen(nullptr)) {
        this->state.Set(nullptr);
        this->timer                  = this->sprayTime;
        this->waterLevel             = 0x400000;
        this->buttonAnimator.frameID = 0;
        this->active                 = ACTIVE_BOUNDS;
    }
}

#if RETRO_INCLUDE_EDITOR
void SeltzerBottle::EditorDraw()
{
    this->bottleAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->buttonAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    this->waterAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);

    this->waterLevel = 0x400000;

    SeltzerBottle::Draw();

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        this->inkEffect = INK_BLEND;

        int32 slotID   = sceneInfo->entitySlot;
        int32 nextSlot = sceneInfo->entitySlot + 1;

        if (nextSlot != -1) {
            PlatformNode *lastNode = GameObject::Get<PlatformNode>(slotID);
            while (lastNode) {
                PlatformNode *nextNode = GameObject::Get<PlatformNode>(nextSlot);
                if (!nextNode || nextNode->classID != PlatformNode::sVars->classID)
                    break;

                DrawHelpers::DrawArrow(lastNode->position.x, lastNode->position.y, nextNode->position.x, nextNode->position.y, 0xFFFF00, INK_NONE,
                                      0xFF);

                if (slotID >= nextSlot) {
                    nextSlot--;
                    slotID--;
                }
                else {
                    nextSlot++;
                    slotID++;
                }

                lastNode = nextNode;
            }
        }

        this->inkEffect = INK_NONE;

        RSDK_DRAWING_OVERLAY(false);
    }
}

void SeltzerBottle::EditorLoad()
{
    sVars->aniFrames.Load("SSZ/Seltzer.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Right");
    RSDK_ENUM_VAR("Left");
}
#endif

void SeltzerBottle::Serialize()
{
    RSDK_EDITABLE_VAR(SeltzerBottle, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(SeltzerBottle, VAR_ENUM, sprayTime);
}

} // namespace GameLogic