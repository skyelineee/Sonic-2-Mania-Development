// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UncurlPlant Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UncurlPlant.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UncurlPlant);

void UncurlPlant::Update()
{
    if (!this->stood) {
        if (this->uncurlPercent > 0)
            this->uncurlPercent -= 0x10;
    }
    else {

        if (this->uncurlMode <= 0) { // Very Slow uncurl
            for (int32 n = 0; n < this->stoodNodeID; ++n)
                this->targetNodeAngles[n] += (sVars->targetNodeAnglesStood[n] - this->targetNodeAngles[n]) >> 2;

            for (int32 n = this->stoodNodeID; n < UNCURLPLANT_NODE_COUNT; ++n)
                this->targetNodeAngles[n] += (sVars->targetNodeAnglesReleased[n] - this->targetNodeAngles[n]) >> 2;
        }
        else {
            for (int32 n = 0; n < this->stoodNodeID; ++n) this->targetNodeAngles[n] = sVars->targetNodeAnglesStood[n];

            for (int32 n = this->stoodNodeID; n < UNCURLPLANT_NODE_COUNT; ++n) this->targetNodeAngles[n] = sVars->targetNodeAnglesReleased[n];
        }

        if (this->uncurlMode > 1)
            this->uncurlPercent = 0x100;
        else if (this->uncurlPercent < 0x100)
            this->uncurlPercent += 0x20;
    }

    for (int32 n = 0; n < UNCURLPLANT_NODE_COUNT; ++n)
        this->nodeAngles[n] =
            sVars->startingNodeAngles[n] + ((this->uncurlPercent * (this->targetNodeAngles[n] - sVars->startingNodeAngles[n])) >> 8);

    int32 prevStoodNode = this->stoodNodeID;
    this->stood         = false;
    this->stoodNodeID   = -1;
    this->uncurlMode    = 0;
    UncurlPlant::CalculateDrawPositions();

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        int32 angle  = 0;
        int32 nodeID = 0;
        for (int32 n = 0; n < UNCURLPLANT_NODE_COUNT; ++n) {
            angle += this->nodeAngles[n];
            if (angle >= 0x90)
                break;

            this->position.x = this->nodePositions[n].x;
            this->position.y = this->nodePositions[n].y;
            if (player->CheckCollisionPlatform(this, &sVars->hitboxNode)) {
                player->position.y += 0x40000;
                this->stood = true;
                if (nodeID > this->stoodNodeID) {
                    if (abs(player->velocity.x) >= 0xC0000)
                        this->uncurlMode = 2; // Fast uncurl
                    else if (abs(player->velocity.x) >= 0x40000)
                        this->uncurlMode = 1; // Slow uncurl

                    this->stoodNodeID = nodeID;
                }
            }
            ++nodeID;
        }
    }

    this->stoodNodeID++;
    if (this->stood) {
        if (prevStoodNode > this->stoodNodeID)
            this->stoodNodeID = prevStoodNode - 1;
        else if (prevStoodNode < this->stoodNodeID)
            this->stoodNodeID = prevStoodNode + 1;
    }

    this->position.x = this->drawPositions[0].x;
    this->position.y = this->drawPositions[0].y;
}

void UncurlPlant::LateUpdate() {}

void UncurlPlant::StaticUpdate() {}

void UncurlPlant::Draw()
{
    for (int32 i = 0; i < UNCURLPLANT_NODE_COUNT; ++i) {
        this->nodeAnimator.DrawSprite(&this->drawPositions[i], false);
        this->decorAnimators[i].DrawSprite(&this->drawPositions[i], false);
    }
}

void UncurlPlant::Create(void *data)
{
    if (sceneInfo->inEditor) {
        this->nodeAngles[0] = 0x00;
        this->nodeAngles[1] = -0x10;
        this->nodeAngles[2] = 0x40;
        this->nodeAngles[3] = 0x60;
        this->nodeAngles[4] = 0x70;
        this->nodeAngles[5] = 0x90;
        this->nodeAngles[6] = 0xA0;
        this->nodeAngles[7] = 0xC0;
    }
    else {
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[1] - 2;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        UncurlPlant::CalculatePositions();

        this->nodeAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        for (int32 i = 0; i < UNCURLPLANT_NODE_COUNT; ++i) {
            this->decorAnimators[i].SetAnimation(sVars->aniFrames, 1, true, Math::Rand(1, 8));
            this->nodeAngles[i] = sVars->startingNodeAngles[i];
        }
    }
}

void UncurlPlant::StageLoad()
{
    sVars->aniFrames.Load("SWZ/Plants.bin", SCOPE_STAGE);

    sVars->hitboxNode.left   = -8;
    sVars->hitboxNode.top    = -12;
    sVars->hitboxNode.right  = 8;
    sVars->hitboxNode.bottom = 8;
}

void UncurlPlant::CalculateDrawPositions()
{
    int32 angle              = this->nodeAngles[0];
    this->drawPositions[0].x = this->position.x;
    this->drawPositions[0].y = this->position.y;

    if (this->direction == FLIP_NONE) {
        for (int32 i = 1; i < UNCURLPLANT_NODE_COUNT; ++i) {
            angle += this->nodeAngles[i];
            this->drawPositions[i].x = this->drawPositions[i - 1].x + (Math::Cos1024(angle) << 10);
            this->drawPositions[i].y = this->drawPositions[i - 1].y - (Math::Sin1024(angle) << 10);
        }
    }
    else {
        for (int32 i = 1; i < UNCURLPLANT_NODE_COUNT; ++i) {
            angle += this->nodeAngles[i];
            this->drawPositions[i].x = this->drawPositions[i - 1].x - (Math::Cos1024(angle) << 10);
            this->drawPositions[i].y = this->drawPositions[i - 1].y - (Math::Sin1024(angle) << 10);
        }
    }
}

void UncurlPlant::CalculatePositions()
{
    int32 angle              = sVars->targetNodeAnglesStood[0];
    this->nodePositions[0].x = this->position.x;
    this->nodePositions[0].y = this->position.y;

    if (this->direction == FLIP_NONE) {
        for (int32 i = 1; i < UNCURLPLANT_NODE_COUNT; ++i) {
            angle += sVars->targetNodeAnglesStood[i];
            this->nodePositions[i].x = this->nodePositions[i - 1].x + (Math::Cos1024(angle) << 10);
            this->nodePositions[i].y = this->nodePositions[i - 1].y - (Math::Sin1024(angle) << 10);
        }
    }
    else {
        for (int32 i = 1; i < UNCURLPLANT_NODE_COUNT; ++i) {
            angle += sVars->targetNodeAnglesStood[i];
            this->nodePositions[i].x = this->nodePositions[i - 1].x - (Math::Cos1024(angle) << 10);
            this->nodePositions[i].y = this->nodePositions[i - 1].y - (Math::Sin1024(angle) << 10);
        }
    }
}

#if RETRO_REV0U
void UncurlPlant::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(UncurlPlant);

    int32 startingNodeAngles[] = { 0x00, 0x10, 0x40, 0x60, 0x70, 0x90, 0xA0, 0xC0 };
    memcpy(sVars->startingNodeAngles, startingNodeAngles, sizeof(startingNodeAngles));

    int32 targetNodeAnglesReleased[] = { 0x00, -0x08, 0x20, 0x40, 0x40, 0x40, 0x40, 0x40 };
    memcpy(sVars->targetNodeAnglesReleased, targetNodeAnglesReleased, sizeof(targetNodeAnglesReleased));

    int32 targetNodeAnglesStood[] = { 0x00, -0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08 };
    memcpy(sVars->targetNodeAnglesStood, targetNodeAnglesStood, sizeof(targetNodeAnglesStood));
}
#endif

#if RETRO_INCLUDE_EDITOR
void UncurlPlant::EditorDraw()
{
    this->nodeAnimator.SetAnimation(sVars->aniFrames, 1, false, 0);
    UncurlPlant::CalculateDrawPositions();

    for (int32 i = 0; i < UNCURLPLANT_NODE_COUNT; ++i) this->nodeAnimator.DrawSprite(&this->drawPositions[i], false);
}

void UncurlPlant::EditorLoad()
{
    sVars->aniFrames.Load("SWZ/Plants.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Right", FLIP_NONE);
    RSDK_ENUM_VAR("Left", FLIP_X);
}
#endif

void UncurlPlant::Serialize() { RSDK_EDITABLE_VAR(UncurlPlant, VAR_UINT8, direction); }

} // namespace GameLogic