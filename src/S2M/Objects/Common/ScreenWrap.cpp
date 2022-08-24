// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ScreenWrap Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "ScreenWrap.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"

#include "Platform.hpp"
#include "Global/SuperSparkle.hpp"
#include "Global/ImageTrail.hpp"
#include "Global/BoundsMarker.hpp"
#include "Global/Camera.hpp"
#include "Global/Ring.hpp"

#include "Helpers/LogHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ScreenWrap);

void ScreenWrap::Update()
{
    if (this->active && this->type == ScreenWrap::Horizontal)
        this->state.Run(this);
}
void ScreenWrap::LateUpdate()
{
    if (this->active && this->type == ScreenWrap::Vertical)
        this->state.Run(this);
}
void ScreenWrap::StaticUpdate()
{
    if (sVars->timer)
        sVars->timer--;
}
void ScreenWrap::Draw()
{
    Vector2 drawPos = this->drawPos;
    sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 6);

    drawPos.x = this->buffer.y;
    sVars->animator.DrawSprite(&this->position, false);

    drawPos.x = this->buffer.x;
    sVars->animator.DrawSprite(&this->position, false);
}

void ScreenWrap::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active     = ACTIVE_NORMAL;
        this->visible    = false;
        this->vWrapMode  = 0;
        this->vWrapState = 0;
        this->setVBounds = false;
        if (this->deactivated)
            this->active = ACTIVE_NEVER;

        TileLayer *fgHigh = SceneLayer::GetTileLayer("FG High");

        int32 bufferY = this->buffer.y >> 16;
        switch (this->type) {
            default:
            case ScreenWrap::Vertical:
                this->vBoundary = (16 * fgHigh->height) << 16;
                this->buffer.x  = this->vBoundary - this->buffer.y;
                this->state.Set(&ScreenWrap::State_Vertical);
                if (!sVars->activeVWrap)
                    sVars->activeVWrap = this;
                break;

            case ScreenWrap::Horizontal:
                this->buffer.x = 16 * (fgHigh->width / bufferY);
                this->buffer.x <<= 16;
                this->state.Set(&ScreenWrap::State_Horizontal);
                if (!sVars->activeHWrap)
                    sVars->activeHWrap = this;
                break;
        }
    }
}

void ScreenWrap::StageLoad()
{
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);

    sVars->timer       = 0;
    sVars->activeVWrap = nullptr;
    sVars->activeHWrap = nullptr;
}

bool32 ScreenWrap::CheckCompetitionWrap() { return sVars != nullptr /*&& Competition::sVars != nullptr*/; }

void ScreenWrap::WrapTileLayer(uint8 layerID, bool32 right)
{
    TileLayer *layer = SceneLayer::GetTileLayer(layerID);

    if (layer) {
        ScreenWrap *hWrap = sVars->activeHWrap;

        if (hWrap) {
            if (right) {
                for (int32 i = 0; i < layer->scrollInfoCount; ++i) {
                    int32 factor = (layer->scrollInfo[i].parallaxFactor * (hWrap->hWrapDistance >> 16)) >> 8;
                    layer->scrollInfo[i].scrollPos += factor << 16;
                }
            }
            else {
                for (int32 i = 0; i < layer->scrollInfoCount; ++i) {
                    int32 factor = (layer->scrollInfo[i].parallaxFactor * (hWrap->hWrapDistance >> 16)) >> 8;
                    layer->scrollInfo[i].scrollPos -= factor << 16;
                }
            }
        }
    }
}
bool32 ScreenWrap::Unknown1(RSDK::GameObject::Entity *entity1, RSDK::GameObject::Entity *entity2)
{
    if (!CheckCompetitionWrap())
        return true;

    int32 x1 = (entity1->position.x >> 16);
    int32 x2 = (entity2->position.x >> 16);

    if ((!Platform::sVars || entity1->classID != Platform::sVars->classID)) {
        entity1 = nullptr;
    }

    if ((!Platform::sVars || entity2->classID != Platform::sVars->classID)) {
        entity2 = nullptr;
    }

    if (entity1)
        x1 = (((Platform *)entity1)->drawPos.x >> 16);
    int32 bufferX1 = x1 / (sVars->activeHWrap->buffer.x >> 16);

    if (entity2)
        x2 = (((Platform *)entity2)->drawPos.x >> 16);
    int32 bufferX2 = x2 / (sVars->activeHWrap->buffer.x >> 16);

    return bufferX1 == bufferX2;
}
void ScreenWrap::HandleVWrap(bool32 noPlayer, int32 direction)
{
    sVars->timer = 2;

    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

    int32 moveY = this->buffer.x;
    if (noPlayer)
        moveY = -this->buffer.x;

    int32 wrapPos = ((screenInfo->center.y + (screenInfo->center.y < 0)) << 15) & 0xFFFF0000;
    if (player1->state.Matches(&Player::State_Static) || !direction)
        wrapPos = 0;

    TileLayer *fgHigh = Zone::sVars->fgLayer[1].GetTileLayer();

    for (int32 s = 1; s < ENTITY_COUNT; ++s) {
        Entity *wrapEntity = GameObject::Get(s);

        bool32 canWrap = false;
        if (noPlayer) {
            if (wrapEntity->position.y >= this->buffer.x - wrapPos) {
                canWrap = wrapEntity->position.y <= wrapPos + (fgHigh->height << 20);
            }
            else {
                canWrap = false;
            }
        }
        else {
            canWrap = wrapEntity->position.y <= this->buffer.y + wrapPos && wrapEntity->position.y >= -wrapPos;
        }

        if (wrapEntity->classID == SuperSparkle::sVars->classID || wrapEntity->classID == ImageTrail::sVars->classID
            || wrapEntity->classID == BoundsMarker::sVars->classID || wrapEntity->classID == Camera::sVars->classID) {
            canWrap = false;
        }

        if (canWrap) {
            bool32 handledMove = false;
            bool32 asPlatform  = false;

            if (Platform::sVars)
                asPlatform = wrapEntity->classID == Platform::sVars->classID;

            if (asPlatform) {
                Platform *platform = (Platform *)wrapEntity;
                platform->drawPos.y += moveY;
                platform->centerPos.y += moveY;
            }

            if (!handledMove)
                wrapEntity->position.y += moveY;
        }
    }
}
void ScreenWrap::HandleHWrap(void *state, bool32 noPlayer)
{
    if (CheckCompetitionWrap() && sVars->activeHWrap && !sVars->handlingWrap) {
        Entity *entity     = (Entity *)sceneInfo->entity;
        Platform *platform = nullptr;

        if ((Platform::sVars && entity->classID == Platform::sVars->classID)) {
            platform = (Platform *)entity;
        }

        StateMachine<Entity> stateMachine;

        // converts from void* -> member func without the compiler interfering :]
        union {
            void *in;
            void (Entity::*out)();
        } u;
        u.in = state;
        stateMachine.Init();
        stateMachine.Set(u.out);

        sVars->handlingWrap = true;
        if (noPlayer) {
            ScreenWrap *hWrap = sVars->activeHWrap;
            ScreenWrap *vWrap = sVars->activeVWrap;

            Vector2 storePos = entity->position;
            Vector2 storeDrawPos;
            storeDrawPos.x = 0;
            storeDrawPos.y = 0;

            int32 wrapDistance = (entity->position.x >> 16) / (hWrap->buffer.x >> 16);
            if (platform)
                wrapDistance = (platform->drawPos.x >> 16) / (hWrap->buffer.x >> 16);

            if (platform) {
                storeDrawPos        = platform->drawPos;
                entity->position.x  = storePos.x - hWrap->buffer.x * wrapDistance;
                platform->drawPos.x = platform->drawPos.x - hWrap->buffer.x * wrapDistance;
            }
            else {
                entity->position.x = storePos.x - hWrap->buffer.x * wrapDistance;
            }

            for (int32 x = 0; x < (hWrap->buffer.y >> 16); ++x) {
                if (platform) {
                    if (platform->drawPos.x != storePos.x || platform->drawPos.y != storePos.y)
                        stateMachine.Run(entity);
                }
                else {
                    if (entity->position.x != storePos.x || entity->position.y != storePos.y)
                        stateMachine.Run(entity);
                }

                if (vWrap) {
                    if (platform) {
                        platform->drawPos.y += vWrap->buffer.x;
                        entity->position.y += vWrap->buffer.x;
                        stateMachine.Run(platform);

                        platform->drawPos.y -= vWrap->buffer.x;
                        platform->drawPos.y -= vWrap->buffer.x;

                        entity->position.y -= vWrap->buffer.x;
                        entity->position.y -= vWrap->buffer.x;
                        stateMachine.Run(platform);

                        platform->drawPos.y += vWrap->buffer.x;
                        entity->position.y += vWrap->buffer.x;

                        platform->drawPos.x += vWrap->buffer.x;
                    }
                    else {
                        entity->position.y += vWrap->buffer.x;
                        stateMachine.Run(entity);

                        entity->position.y -= vWrap->buffer.x;
                        entity->position.y -= vWrap->buffer.x;
                        stateMachine.Run(entity);

                        entity->position.x += vWrap->buffer.x;
                    }
                }
                else {
                    entity->position.x += hWrap->buffer.x;

                    if (platform)
                        platform->drawPos.x += hWrap->buffer.x;
                }
            }

            entity->position = storePos;
            if (platform)
                platform->drawPos = storeDrawPos;
        }
        else {
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                ScreenWrap *hWrap = sVars->activeHWrap;
                ScreenWrap *vWrap = sVars->activeVWrap;

                int32 storeX = player->position.x;
                int32 storeY = player->position.y;

                player->position.x = storeX - hWrap->buffer.x * ((player->position.x >> 16) / (hWrap->buffer.x >> 16));

                for (int32 x = 0; x < (hWrap->buffer.y >> 16); ++x) {
                    if (player->position.x != storeX || player->position.y != storeY)
                        stateMachine.Run(entity);

                    if (vWrap) {
                        player->position.y += vWrap->buffer.x;
                        stateMachine.Run(entity);

                        player->position.y -= vWrap->buffer.x;
                        player->position.y -= vWrap->buffer.x;
                        stateMachine.Run(entity);

                        player->position.y += vWrap->buffer.x;
                    }

                    player->position.x += hWrap->buffer.x;
                }

                player->position.x = storeX;
                player->position.y = storeY;
            }
        }

        sVars->handlingWrap = false;
    }
}

void ScreenWrap::State_Vertical()
{
    SET_CURRENT_STATE();

    if (!this->setVBounds) {
        int32 playerID = 0;
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            UNUSED(player);

            Zone::sVars->deathBoundary[playerID] = 2 * this->vBoundary;

            Zone::sVars->cameraBoundsB[playerID]      = 2 * (this->vBoundary >> 16);
            Zone::sVars->playerBoundsB[playerID]      = 2 * this->vBoundary;
            Zone::sVars->playerBoundActiveB[playerID] = false;

            Zone::sVars->cameraBoundsT[playerID]      = -2 * (this->vBoundary >> 16);
            Zone::sVars->playerBoundsT[playerID]      = -2 * this->vBoundary;
            Zone::sVars->playerBoundActiveT[playerID] = false;

            playerID++;
        }

        this->setVBounds = true;
    }

    if (sVars->activeHWrap) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (player->position.y >= (this->buffer.y / 2) + this->buffer.x) {
                player->position.y -= this->buffer.x;

                if (player->camera)
                    player->camera->position.y -= this->buffer.x;

                for (auto trail : GameObject::GetEntities<ImageTrail>(FOR_ALL_ENTITIES)) {
                    trail->position.y -= this->buffer.x;
                    trail->lastPos.y -= this->buffer.x;

                    for (int32 i = 0; i < IMAGETRAIL_TRACK_COUNT; ++i) trail->statePos[i].y -= this->buffer.x;
                }

                for (auto sparkle : GameObject::GetEntities<SuperSparkle>(FOR_ALL_ENTITIES)) {
                    sparkle->position.y -= this->buffer.x;
                }
            }
            else if (player->position.y <= (this->buffer.y / 2)) {
                player->position.y += this->buffer.x;

                if (player->camera)
                    player->camera->position.y += this->buffer.x;

                for (auto trail : GameObject::GetEntities<ImageTrail>(FOR_ALL_ENTITIES)) {
                    trail->position.y += this->buffer.x;
                    trail->lastPos.y += this->buffer.x;

                    for (int32 i = 0; i < IMAGETRAIL_TRACK_COUNT; ++i) trail->statePos[i].y += this->buffer.x;
                }

                for (auto sparkle : GameObject::GetEntities<SuperSparkle>(FOR_ALL_ENTITIES)) {
                    sparkle->position.y += this->buffer.x;
                }
            }
        }
    }
    else {
        Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

        if (player1->camera) {
            this->drawPos.x = player1->position.x;
            switch (this->vWrapMode) {
                case 0:
                    if (!this->vWrapState) {
                        if (player1->position.y >= this->buffer.x - (this->buffer.y >> 1)) {
                            HandleVWrap(false, 1);
                            this->vWrapMode  = 1;
                            this->vWrapState = 1;
                            LogHelpers::PrintText("FROM ABOVE: WRAP ENTITIES DOWN");
                        }
                        else if (player1->position.y <= this->buffer.y + (this->buffer.y >> 1)) {
                            HandleVWrap(true, 1);
                            this->vWrapMode  = 1;
                            this->vWrapState = 2;
                            LogHelpers::PrintText("FROM BELOW: WRAP ENTITIES UP");
                        }
                    }
                    break;

                case 1:
                    switch (this->vWrapState) {
                        case 0: break;

                        case 1:
                            if (player1->position.y >= this->buffer.x + (this->buffer.y >> 1)) {
                                player1->position.y -= this->buffer.x;

                                if (player1->camera)
                                    player1->camera->position.y -= this->buffer.x;

                                screenInfo[player1->camera->screenID].position.y -= (this->buffer.x >> 16);

                                for (auto trail : GameObject::GetEntities<ImageTrail>(FOR_ALL_ENTITIES)) {
                                    trail->position.y -= this->buffer.x;
                                    trail->lastPos.y -= this->buffer.x;

                                    for (int32 i = 0; i < IMAGETRAIL_TRACK_COUNT; ++i) trail->statePos[i].y -= this->buffer.x;
                                }

                                for (auto sparkle : GameObject::GetEntities<SuperSparkle>(FOR_ALL_ENTITIES)) {
                                    sparkle->position.y -= this->buffer.x;
                                }

                                HandleVWrap(true, 1);
                                sVars->stateWrapUp.Run(this);

                                this->vWrapMode = 2;
                                LogHelpers::PrintText("FROM ABOVE: WRAP PLAYER UP");
                            }
                            else if (player1->position.y <= this->buffer.x - (this->buffer.y >> 1)) {
                                HandleVWrap(true, 0);
                                this->vWrapMode  = 0;
                                this->vWrapState = 0;
                                LogHelpers::PrintText("FROM ABOVE: WRAP ENTITIES UP");
                            }
                            break;

                        case 2:
                            if (player1->position.y <= this->buffer.y - (this->buffer.y >> 1)) {
                                player1->position.y += this->buffer.x;

                                if (player1->camera)
                                    player1->camera->position.y += this->buffer.x;

                                screenInfo[player1->camera->screenID].position.y += (this->buffer.x >> 16);

                                for (auto trail : GameObject::GetEntities<ImageTrail>(FOR_ALL_ENTITIES)) {
                                    trail->position.y += this->buffer.x;
                                    trail->lastPos.y += this->buffer.x;

                                    for (int32 i = 0; i < IMAGETRAIL_TRACK_COUNT; ++i) trail->statePos[i].y += this->buffer.x;
                                }

                                for (auto sparkle : GameObject::GetEntities<SuperSparkle>(FOR_ALL_ENTITIES)) {
                                    sparkle->position.y += this->buffer.x;
                                }

                                HandleVWrap(false, 1);
                                sVars->stateWrapDown.Run(this);
                                this->vWrapMode = 2;
                                LogHelpers::PrintText("FROM BELOW: WRAP PLAYER DOWN");
                            }
                            else if (player1->position.y > this->buffer.y + (this->buffer.y >> 1)) {
                                HandleVWrap(false, 0);
                                this->vWrapMode  = 0;
                                this->vWrapState = 0;
                                LogHelpers::PrintText("FROM BELOW: WRAP ENTITIES DOWN");
                            }
                            break;
                    }
                    break;

                case 2:
                    switch (this->vWrapState) {
                        case 0: break;

                        case 1:
                            if (player1->position.y >= this->buffer.y + (this->buffer.y >> 1)) {
                                this->vWrapMode  = 0;
                                this->vWrapState = 0;
                                LogHelpers::PrintText("FROM ABOVE: COMPLETE");
                            }
                            else if (player1->position.y <= this->buffer.y - (this->buffer.y >> 1)) {
                                this->vWrapMode  = 1;
                                this->vWrapState = 1;
                                LogHelpers::PrintText("FROM ABOVE: RETURN");
                            }
                            break;

                        case 2:
                            if (player1->position.y <= this->buffer.x - (this->buffer.y / 2)) {
                                this->vWrapMode  = 0;
                                this->vWrapState = 0;
                                LogHelpers::PrintText("FROM BELOW: COMPLETE");
                            }
                            else if (player1->position.y >= this->buffer.x + (this->buffer.y / 2)) {
                                this->vWrapMode  = 1;
                                this->vWrapState = 2;
                                LogHelpers::PrintText("FROM BELOW: RETURN");
                            }
                            break;
                    }
                    break;
            }
        }
    }
}
void ScreenWrap::State_Horizontal()
{
    SET_CURRENT_STATE();

    if (Zone::sVars->timer > 1) {
        int32 bufferY = this->buffer.y >> 16;
        int32 moveX   = this->buffer.x;
        if (moveX < 0)
            moveX += 3;

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (player->position.x >= (moveX >> 2)) {
                if (player->position.x > moveX * bufferY - (moveX >> 2)) {
                    this->hWrapDistance = (bufferY - 1) * this->buffer.x;

                    player->position.x -= this->hWrapDistance;
                    if (player->camera)
                        player->camera->position.x -= this->hWrapDistance;

                    this->hWrapPlayerID = player->playerID;
                    sVars->stateWrapRight.Run(this);
                    this->hWrapPlayerID = -1;
                }
            }
            else {
                this->hWrapDistance = (bufferY - 1) * this->buffer.x;

                player->position.x += this->hWrapDistance;
                if (player->camera)
                    player->camera->position.x += this->hWrapDistance;

                this->hWrapPlayerID = player->playerID;
                sVars->stateWrapLeft.Run(this);
                this->hWrapPlayerID = -1;
            }
        }

        for (auto ring : GameObject::GetEntities<Ring>(FOR_ACTIVE_ENTITIES)) {
            if (ring->position.x >= (moveX >> 2)) {
                if (ring->position.x > this->buffer.x * bufferY - (moveX >> 2)) {
                    this->hWrapDistance = (bufferY - 1) * this->buffer.x;

                    ring->position.x -= this->hWrapDistance;
                }
            }
            else {
                this->hWrapDistance = (bufferY - 1) * moveX;

                ring->position.x += this->hWrapDistance;
            }
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void ScreenWrap::EditorDraw()
{
    sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 6);
    sVars->animator.DrawSprite(&this->position, false);
}

void ScreenWrap::EditorLoad()
{
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Vertical");
    RSDK_ENUM_VAR("Horizontal");
}
#endif

#if RETRO_REV0U
void ScreenWrap::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(ScreenWrap);

    sVars->aniFrames.Init();
}
#endif

void ScreenWrap::Serialize()
{
    RSDK_EDITABLE_VAR(ScreenWrap, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(ScreenWrap, VAR_VECTOR2, buffer);
    RSDK_EDITABLE_VAR(ScreenWrap, VAR_BOOL, deactivated);
}

} // namespace GameLogic
