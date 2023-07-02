// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: BoundsMarker Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "BoundsMarker.hpp"
#include "Zone.hpp"
#include "DebugMode.hpp"

#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(BoundsMarker);

void BoundsMarker::Update()
{
    this->visible = DebugMode::sVars->debugActive;

    if (!this->deactivated) {
        if (!this->storedBounds) {
            BoundsMarker::StoreBounds();
            this->storedBounds = true;
        }

        if (!this->state.Matches(&BoundsMarker::State_Restore))
            this->state.Run(this);

        if (this->active == ACTIVE_NORMAL) {
            if (!BoundsMarker::CheckOnScreen(&this->position, &this->updateRange)) {
                this->active = ACTIVE_BOUNDS;

                if (this->onExit == BoundsMarker::ExitDestroyObj || this->onExit == BoundsMarker::ExitKeepBounds || !this->storedBounds) {
                    switch (this->onExit) {
                        case BoundsMarker::ExitRestoreBounds:
                        case BoundsMarker::ExitKeepBounds: this->state.Set(&BoundsMarker::State_Init); break;

                        case BoundsMarker::ExitDeactivateObj: this->state.Set(&BoundsMarker::State_Restore); break;

                        case BoundsMarker::ExitDestroyObj: this->Destroy(); break;

                        default: break;
                    }
                }
                else {
                    int32 playerID            = 0;
                    int32 restoredBoundsCount = 0;
                    bool32 disableOnExit      = false;
                    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                        if (this->field_180[playerID]) {
                            Zone::sVars->cameraBoundsL[playerID]      = this->boundsL[playerID];
                            Zone::sVars->cameraBoundsR[playerID]      = this->boundsR[playerID];
                            Zone::sVars->cameraBoundsT[playerID]      = this->boundsT[playerID];
                            Zone::sVars->cameraBoundsB[playerID]      = this->boundsB[playerID];
                            Zone::sVars->deathBoundary[playerID]      = this->deathBounds[playerID];
                            Zone::sVars->playerBoundActiveL[playerID] = this->playerBoundActiveL[playerID];
                            Zone::sVars->playerBoundActiveR[playerID] = this->playerBoundActiveR[playerID];
                            Zone::sVars->playerBoundActiveT[playerID] = this->playerBoundActiveT[playerID];
                            Zone::sVars->playerBoundActiveB[playerID] = this->playerBoundActiveB[playerID];

                            this->field_180[playerID] = false;
                            if (player->sidekick && this->field_180[0])
                                disableOnExit = true;

                            this->lockedL[playerID] = false;
                            this->lockedR[playerID] = false;

                            restoredBoundsCount++;
                        }

                        playerID++;
                    }

                    if (restoredBoundsCount == Player::sVars->playerCount)
                        this->storedBounds = false;

                    if (!disableOnExit) {
                        switch (this->onExit) {
                            case BoundsMarker::ExitRestoreBounds:
                            case BoundsMarker::ExitKeepBounds: this->state.Set(&BoundsMarker::State_Init); break;

                            case BoundsMarker::ExitDeactivateObj: this->state.Set(&BoundsMarker::State_Restore); break;

                            case BoundsMarker::ExitDestroyObj: this->Destroy(); break;

                            default: break;
                        }
                    }
                }
            }
        }
    }
}
void BoundsMarker::LateUpdate()
{
    if (this->state.Matches(&BoundsMarker::State_Restore))
        this->state.Run(this);
}
void BoundsMarker::StaticUpdate() {}
void BoundsMarker::Draw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 2);
    this->animator.DrawSprite(nullptr, false);

    if (this->width > 0) {
        Vector2 pos1, pos2;

        pos1.x = this->position.x - (this->width << 15);
        pos1.y = this->position.y;

        pos2.x = this->position.x + (this->width << 15);
        pos2.y = this->position.y;

        DrawHelpers::DrawLine(pos1, pos2, 0xFFFF00);
    }

    if (this->offset > 0) {
        Vector2 pos1, pos2;

        switch (this->type) {
            case BoundsMarker::AnyY:
            case BoundsMarker::BelowYAny:
                pos1.x = this->position.x;
                pos1.y = this->position.y;

                pos2.x = this->position.x;
                pos2.y = this->position.y;
                break;

            case BoundsMarker::AboveY:
                pos1.x = this->position.x;
                pos1.y = this->position.y - (this->offset << 16);

                pos2.x = this->position.x;
                pos2.y = this->position.y;
                break;

            case BoundsMarker::BelowY:
                pos1.x = this->position.x;
                pos1.y = this->position.y;

                pos2.x = this->position.x;
                pos2.y = this->position.y + (this->offset << 16);
                break;
        }

        DrawHelpers::DrawLine(pos1, pos2, 0xFFFF00);
    }
}

void BoundsMarker::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active = ACTIVE_XBOUNDS;
        if (this->lockLeft != BoundsMarker::LockNone || this->lockRight != BoundsMarker::LockNone)
            this->active = ACTIVE_BOUNDS;
        this->drawGroup = Zone::sVars->hudDrawGroup;

        if (this->width <= 0) {
            this->initialWidth  = this->width;
            this->width         = -0x8000 * this->width * screenInfo->size.x;
            this->updateRange.x = -0x8000 * this->width * screenInfo->size.x;
        }
        else {
            this->initialWidth  = this->width << 15;
            this->width         = this->width << 15;
        }
        this->updateRange.x = this->width;

        if (this->lockLeft != BoundsMarker::LockNone || this->lockRight != BoundsMarker::LockNone) {
            if (this->offset <= 0)
                this->updateRange.y = 48 << 16;
            else
                this->updateRange.y = 0x18000 * this->offset;
        }

        this->destroyOnDeactivate = false;
        this->lockedL[0]          = false;
        this->lockedL[1]          = false;
        this->lockedL[2]          = false;
        this->lockedL[3]          = false;
        this->storedBounds        = false;
        this->lockedR[0]          = false;
        this->lockedR[1]          = false;
        this->lockedR[2]          = false;
        this->lockedR[3]          = false;
        this->state.Set(&BoundsMarker::State_Init);
        this->restoreAccel.x >>= 16;
        this->restoreAccel.y >>= 16;

        for (auto player : GameObject::GetEntities<Player>(FOR_ALL_ENTITIES)) {
            UNUSED(player);
        }
    }
}

void BoundsMarker::StageLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }

void BoundsMarker::State_Init()
{
    SET_CURRENT_STATE();

    if (this->lockLeft != BoundsMarker::LockNone || this->lockRight != BoundsMarker::LockNone)
        this->active = ACTIVE_NORMAL;

    this->state.Set(&BoundsMarker::State_Apply);
}

void BoundsMarker::State_Apply()
{
    SET_CURRENT_STATE();

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        ApplyBounds(player, false);
    }
}

void BoundsMarker::State_Restore()
{
    SET_CURRENT_STATE();

    int32 playerID            = 0;
    int32 activePlayerCount   = 0;
    int32 disabledPlayerCount = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (this->field_180[playerID] && !player->sidekick) {
            if (player->camera)
                ++activePlayerCount;

            int32 screenX        = 0;
            int32 screenY        = 0;
            int32 screenWidth    = 0;
            int32 screenHeight   = 0;
            bool32 screenMissing = true;
            if (player->camera) {
                ScreenInfo *screen = &screenInfo[player->camera->screenID];

                screenX       = screen->position.x;
                screenY       = screen->position.y;
                screenWidth   = screen->size.x;
                screenHeight  = screen->size.y;
                screenMissing = false;
            }

            Zone::sVars->playerBoundActiveT[playerID] = this->playerBoundActiveT[playerID];
            Zone::sVars->playerBoundActiveB[playerID] = this->playerBoundActiveB[playerID];
            Zone::sVars->playerBoundActiveL[playerID] = this->playerBoundActiveL[playerID];
            Zone::sVars->playerBoundActiveR[playerID] = this->playerBoundActiveR[playerID];

            if (this->lockedL[playerID]) {
                if (Zone::sVars->cameraBoundsL[playerID] > this->boundsL[playerID]) {
                    Zone::sVars->cameraBoundsL[playerID] -= this->restoreSpd.x >> 16;

                    bool32 finished = Zone::sVars->cameraBoundsL[playerID] <= this->boundsL[playerID];
                    if (!screenMissing && Zone::sVars->cameraBoundsL[playerID] > this->boundsL[playerID])
                        finished = Zone::sVars->cameraBoundsL[playerID] <= screenX - (screenWidth >> 1);

                    if (finished) {
                        Zone::sVars->cameraBoundsL[playerID] = this->boundsL[playerID];
                        this->lockedL[playerID]              = 0;
                    }
                }
            }

            if (this->lockedR[playerID]) {
                if (Zone::sVars->cameraBoundsR[playerID] < this->boundsR[playerID]) {
                    Zone::sVars->cameraBoundsR[playerID] += this->restoreSpd.x >> 16;

                    bool32 finished = Zone::sVars->cameraBoundsR[playerID] < this->boundsR[playerID];
                    if (!screenMissing && Zone::sVars->cameraBoundsR[playerID] < this->boundsR[playerID])
                        finished = Zone::sVars->cameraBoundsR[playerID] < screenX + screenWidth + (screenWidth >> 1);

                    if (!finished) {
                        Zone::sVars->cameraBoundsR[playerID] = this->boundsR[playerID];
                        this->lockedR[playerID]              = 0;
                    }
                }
            }

            bool32 setBounds = false;
            switch (this->type) {
                case BoundsMarker::AnyY:
                    if (!this->field_14C[playerID]) {
                        if (Zone::sVars->cameraBoundsB[playerID] >= this->boundsB[playerID]) {
                            Zone::sVars->cameraBoundsB[playerID] -= this->restoreSpd.y >> 16;

                            bool32 finished = Zone::sVars->cameraBoundsB[playerID] <= this->boundsB[playerID];
                            if (!screenMissing && Zone::sVars->cameraBoundsB[playerID] > this->boundsB[playerID])
                                finished = Zone::sVars->cameraBoundsB[playerID] <= screenY + screenHeight + (screenHeight >> 1);

                            if (finished) {
                                Zone::sVars->cameraBoundsB[playerID] = this->boundsB[playerID];

                                if (!this->lockedL[playerID] && !this->lockedR[playerID])
                                    this->field_14C[playerID] = true;

                                setBounds = true;
                            }
                        }
                        else {
                            Zone::sVars->cameraBoundsB[playerID] += this->restoreSpd.y >> 16;

                            bool32 finished = Zone::sVars->cameraBoundsB[playerID] >= this->boundsB[playerID];
                            if (!screenMissing && Zone::sVars->cameraBoundsB[playerID] < this->boundsB[playerID])
                                finished = Zone::sVars->cameraBoundsB[playerID] >= screenY + screenHeight + (screenHeight >> 1);

                            if (finished) {
                                Zone::sVars->cameraBoundsB[playerID] = this->boundsB[playerID];

                                if (!this->lockedL[playerID] && !this->lockedR[playerID])
                                    this->field_14C[playerID] = true;

                                setBounds = true;
                            }
                        }
                    }
                    break;

                case BoundsMarker::AboveY:
                    if (!this->field_14C[playerID]) {
                        Zone::sVars->cameraBoundsB[playerID] += this->restoreSpd.y >> 16;

                        bool32 finished = Zone::sVars->cameraBoundsB[playerID] >= this->boundsB[playerID];
                        if (!screenMissing && Zone::sVars->cameraBoundsB[playerID] < this->boundsB[playerID])
                            finished = Zone::sVars->cameraBoundsB[playerID] >= screenY + screenHeight + (screenHeight >> 1);

                        if (finished) {
                            Zone::sVars->cameraBoundsB[playerID] = this->boundsB[playerID];

                            if (!this->lockedL[playerID] && !this->lockedR[playerID])
                                this->field_14C[playerID] = true;

                            setBounds = true;
                        }
                    }
                    break;

                case BoundsMarker::BelowY:
                    if (!this->field_14C[playerID]) {
                        Zone::sVars->cameraBoundsT[playerID] -= this->restoreSpd.y >> 16;

                        bool32 finished = Zone::sVars->cameraBoundsT[playerID] <= this->boundsT[playerID];
                        if (!screenMissing && Zone::sVars->cameraBoundsT[playerID] > this->boundsT[playerID])
                            finished = Zone::sVars->cameraBoundsT[playerID] <= screenY - (screenHeight >> 1);

                        if (finished) {
                            Zone::sVars->cameraBoundsT[playerID] = this->boundsT[playerID];

                            if (!this->lockedL[playerID] && !this->lockedR[playerID])
                                this->field_14C[playerID] = true;

                            setBounds = true;
                        }
                    }
                    break;

                case BoundsMarker::BelowYAny:
                    if (!this->field_14C[playerID]) {
                        if (Zone::sVars->cameraBoundsT[playerID] >= this->boundsT[playerID]) {
                            Zone::sVars->cameraBoundsT[playerID] -= this->restoreSpd.y >> 16;

                            bool32 finished = Zone::sVars->cameraBoundsT[playerID] <= this->boundsT[playerID];
                            if (!screenMissing && Zone::sVars->cameraBoundsT[playerID] > this->boundsT[playerID])
                                finished = Zone::sVars->cameraBoundsT[playerID] <= screenY - (screenHeight >> 1);

                            if (finished) {
                                Zone::sVars->cameraBoundsT[playerID] = this->boundsT[playerID];

                                if (!this->lockedL[playerID] && !this->lockedR[playerID])
                                    this->field_14C[playerID] = true;

                                setBounds = true;
                            }
                        }
                        else {
                            Zone::sVars->cameraBoundsT[playerID] += this->restoreSpd.y >> 16;

                            bool32 finished = Zone::sVars->cameraBoundsT[playerID] >= this->boundsT[playerID];
                            if (!screenMissing && Zone::sVars->cameraBoundsT[playerID] > this->boundsT[playerID])
                                finished = Zone::sVars->cameraBoundsT[playerID] >= screenY - (screenHeight >> 1);

                            if (finished) {
                                Zone::sVars->cameraBoundsT[playerID] = this->boundsT[playerID];

                                if (!this->lockedL[playerID] && !this->lockedR[playerID])
                                    this->field_14C[playerID] = true;

                                setBounds = true;
                            }
                        }
                    }
                    break;

                default: break;
            }

            if (setBounds && player->camera) {
                player->camera->boundsL = Zone::sVars->cameraBoundsL[playerID];
                player->camera->boundsR = Zone::sVars->cameraBoundsR[playerID];
                player->camera->boundsT = Zone::sVars->cameraBoundsT[playerID];
                player->camera->boundsB = Zone::sVars->cameraBoundsB[playerID];
            }

            if (this->field_14C[playerID]) {
                ++disabledPlayerCount;
                this->field_14C[playerID] = false;
            }
            ++playerID;

            this->restoreSpd.x += this->restoreAccel.x;
            this->restoreSpd.y += this->restoreAccel.y;
        }
    }

    if (disabledPlayerCount >= activePlayerCount) {
        RestoreBounds(false);

        this->deactivated = true;
        this->state.Set(nullptr);
        if (this->destroyOnDeactivate)
            this->Destroy();
    }
}

void BoundsMarker::StoreBounds()
{
    int32 playerID = 0;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        UNUSED(player);
        
        this->boundsL[playerID]            = Zone::sVars->cameraBoundsL[playerID];
        this->boundsR[playerID]            = Zone::sVars->cameraBoundsR[playerID];
        this->boundsT[playerID]            = Zone::sVars->cameraBoundsT[playerID];
        this->boundsB[playerID]            = Zone::sVars->cameraBoundsB[playerID];
        this->deathBounds[playerID]        = Zone::sVars->cameraBoundsB[playerID];
        this->playerBoundActiveL[playerID] = Zone::sVars->playerBoundActiveL[playerID];
        this->playerBoundActiveR[playerID] = Zone::sVars->playerBoundActiveR[playerID];
        this->playerBoundActiveT[playerID] = Zone::sVars->playerBoundActiveT[playerID];
        this->playerBoundActiveB[playerID] = Zone::sVars->playerBoundActiveB[playerID];

        playerID++;
    }
}

void BoundsMarker::RestoreBounds(bool32 useState)
{
    if (useState) {
        this->state.Set(&BoundsMarker::State_Restore);
    }
    else {
        int32 playerID = 0;
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            UNUSED(player);
            
            Zone::sVars->cameraBoundsL[playerID]      = this->boundsL[playerID];
            Zone::sVars->cameraBoundsR[playerID]      = this->boundsR[playerID];
            Zone::sVars->cameraBoundsT[playerID]      = this->boundsT[playerID];
            Zone::sVars->cameraBoundsB[playerID]      = this->boundsB[playerID];
            Zone::sVars->cameraBoundsB[playerID]      = this->deathBounds[playerID];
            Zone::sVars->playerBoundActiveL[playerID] = this->playerBoundActiveL[playerID];
            Zone::sVars->playerBoundActiveR[playerID] = this->playerBoundActiveR[playerID];
            Zone::sVars->playerBoundActiveT[playerID] = this->playerBoundActiveT[playerID];
            Zone::sVars->playerBoundActiveB[playerID] = this->playerBoundActiveB[playerID];

            playerID++;
        }
    }
}

void BoundsMarker::Unknown3(uint8 side)
{
    int32 playerID = 0;
    switch (side) {
        case 0:
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                UNUSED(player);
                
                this->lockedL[playerID]                   = 0;
                Zone::sVars->playerBoundActiveR[playerID] = this->playerBoundActiveL[playerID];
                Zone::sVars->cameraBoundsR[playerID]      = this->boundsL[playerID];

                playerID++;
            }
            break;

        case 1:
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                UNUSED(player);
                
                this->lockedR[playerID]                   = 0;
                Zone::sVars->playerBoundActiveR[playerID] = this->playerBoundActiveR[playerID];
                Zone::sVars->cameraBoundsR[playerID]      = this->boundsR[playerID];

                playerID++;
            }
            break;
    }
}

void BoundsMarker::Unknown4(uint8 side)
{
    int32 playerID = 0;
    switch (side) {
        case 0:
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                UNUSED(player);
                
                this->lockedL[playerID]                   = 0;
                this->lockLeft                            = BoundsMarker::LockNone;
                Zone::sVars->playerBoundActiveR[playerID] = true;
                Zone::sVars->cameraBoundsR[playerID]      = 0;

                playerID++;
            }
            break;

        case 1:
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                UNUSED(player);
                
                this->lockedR[playerID]                   = 0;
                this->lockRight                           = BoundsMarker::LockNone;
                Zone::sVars->playerBoundActiveR[playerID] = true;
                Zone::sVars->cameraBoundsR[playerID]      = TILE_SIZE * Zone::sVars->fgLayer[1].GetTileLayer()->width;

                playerID++;
            }
            break;
    }
}

void BoundsMarker::ApplyBounds(Player *player, bool32 setPos)
{
    if (!player->camera || !player->CheckValidState() || player->sidekick || player->state.Matches(&Player::State_FlyToPlayer))
        return;

    int32 playerID     = player->Slot();
    ScreenInfo *screen = &screenInfo[player->camera->screenID];

    bool32 canApply = false;
    switch (this->alignFrom) {
        case BoundsMarker::AlignPlayer:
            if (abs(this->position.x - player->position.x) < this->width)
                canApply = true;
            break;

        case BoundsMarker::AlignCameraL:
            if (this->field_180[playerID] || screen->position.x < (this->position.x >> 16) - (this->width >> 16))
                canApply = true;
            else if (this->field_180[playerID] || screen->position.x < (this->position.x >> 16) + (this->width >> 16))
                canApply = true;
            break;

        case BoundsMarker::AlignCameraR:
            if (this->field_180[playerID] || screen->position.x + screen->size.x > (this->position.x >> 16) + (this->width >> 16))
                canApply = true;
            else if (this->field_180[playerID] || screen->position.x + screen->size.x > (this->position.x >> 16) - (this->width >> 16))
                canApply = true;
            break;
    }

    if (canApply) {
        bool32 canAlign = true;
        switch (this->type) {
            case BoundsMarker::AnyY:
                Zone::sVars->cameraBoundsB[playerID] = this->position.y >> 16;
                Zone::sVars->playerBoundsB[playerID] = this->position.y;
                Zone::sVars->deathBoundary[playerID] = this->position.y;
                break;

            case BoundsMarker::AboveY:
                if (player->position.y < this->position.y - (this->offset << 16)) {
                    Zone::sVars->cameraBoundsB[playerID] = this->position.y >> 16;
                    Zone::sVars->playerBoundsB[playerID] = this->position.y;
                    Zone::sVars->deathBoundary[playerID] = this->position.y;
                }
                else {
                    this->field_180[playerID] = false;
                    canAlign                  = false;
                }
                break;

            case BoundsMarker::BelowY:
                if (player->position.y > this->position.y + (this->offset << 16)) {
                    Zone::sVars->playerBoundsT[playerID] = this->position.y;
                    Zone::sVars->cameraBoundsT[playerID] = this->position.y >> 16;
                }
                else {
                    this->field_180[playerID] = false;
                    canAlign                  = false;
                }
                break;

            case BoundsMarker::BelowYAny:
                Zone::sVars->playerBoundsT[playerID] = this->position.y;
                Zone::sVars->cameraBoundsT[playerID] = this->position.y >> 16;
                break;

            default: break;
        }

        if (canAlign) {
            switch (this->lockLeft) {
                default:
                case BoundsMarker::LockNone: break;

                case BoundsMarker::LockAtBounds:
                    if (!this->lockedL[playerID]) {
                        Zone::sVars->playerBoundsL[playerID]      = this->position.x - this->width;
                        Zone::sVars->cameraBoundsL[playerID]      = Zone::sVars->playerBoundsL[playerID] >> 16;
                        player->camera->boundsL                   = Zone::sVars->cameraBoundsL[playerID];
                        this->lockedL[playerID]                   = true;
                        Zone::sVars->playerBoundActiveL[playerID] = true;
                    }
                    break;

                case BoundsMarker::LockWithCam:
                    if (screen->position.x >= Zone::sVars->cameraBoundsL[playerID]) {
                        Zone::sVars->cameraBoundsL[playerID]      = screen->position.x;
                        Zone::sVars->playerBoundsL[playerID]      = Zone::sVars->cameraBoundsL[playerID] << 16;
                        this->lockedL[playerID]                   = true;
                        Zone::sVars->playerBoundActiveL[playerID] = true;
                    }
                    break;
            }

            switch (this->lockRight) {
                default:
                case BoundsMarker::LockNone: break;

                case BoundsMarker::LockAtBounds:
                    if (!this->lockedR[playerID]) {
                        Zone::sVars->playerBoundsR[playerID]      = this->position.x + this->width;
                        Zone::sVars->cameraBoundsR[playerID]      = Zone::sVars->playerBoundsR[playerID] >> 16;
                        this->lockedR[playerID]                   = true;
                        Zone::sVars->playerBoundActiveR[playerID] = true;
                    }
                    break;

                case BoundsMarker::LockWithCam:
                    if (screen->position.x + screen->size.x <= Zone::sVars->cameraBoundsR[playerID]) {
                        Zone::sVars->cameraBoundsR[playerID]      = screen->position.x + screen->size.x;
                        Zone::sVars->playerBoundsR[playerID]      = Zone::sVars->cameraBoundsR[playerID] << 16;
                        this->lockedR[playerID]                   = true;
                        Zone::sVars->playerBoundActiveR[playerID] = true;
                    }
                    break;
            }

            this->field_180[playerID] = true;
            this->field_1A0           = true;
        }
    }

    if (setPos) {
        if (player->camera) {
            player->camera->boundsL = Zone::sVars->cameraBoundsL[playerID];
            player->camera->boundsR = Zone::sVars->cameraBoundsR[playerID];
            player->camera->boundsT = Zone::sVars->cameraBoundsT[playerID];
            player->camera->boundsB = Zone::sVars->cameraBoundsB[playerID];
        }
    }
}

void BoundsMarker::ApplyAllBounds(Player *player, bool32 setPos)
{
    if ((player->CheckValidState() || player->classID == DebugMode::sVars->classID) && !player->sidekick) {
        if (!player->state.Matches(&Player::State_FlyToPlayer)) {
            for (auto marker : GameObject::GetEntities<BoundsMarker>(FOR_ALL_ENTITIES)) {
                marker->ApplyBounds(player, setPos);
            }
        }
    }
}

bool32 BoundsMarker::CheckOnScreen(RSDK::Vector2 *position, RSDK::Vector2 *range)
{
    position->CheckOnScreen(range);

    if (position && range) {

        for (int32 p = 0; p < Player::sVars->playerCount; ++p) {
            Player *player = GameObject::Get<Player>(p);
            if (!player->sidekick) {
                if (!player->state.Matches(&Player::State_FlyToPlayer)) {
                    ScreenInfo *screen = &screenInfo[player->playerID];

                    if (abs(position->x - ((screen->center.x + screen->position.x) << 16)) <= range->x + (screen->center.x << 16)) {
                        if (abs(position->y - ((screen->center.y + screen->position.y) << 16)) <= range->y + (screen->center.y << 16)) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

BoundsMarker *BoundsMarker::GetNearestX()
{
    GameObject::Entity *self = (GameObject::Entity *)sceneInfo->entity;

    BoundsMarker *targetMarker = nullptr;
    int32 targetDistance       = 0x7FFFFFFF;

    for (auto marker : GameObject::GetEntities<BoundsMarker>(FOR_ACTIVE_ENTITIES)) {
        if (!marker->deactivated) {
            int32 distX = abs(marker->position.x - self->position.x);
            if (distX < targetDistance) {
                targetDistance = distX;
                targetMarker   = marker;
            }
        }
    }

    return targetMarker;
}

#if RETRO_INCLUDE_EDITOR
void BoundsMarker::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 2);
    this->animator.DrawSprite(nullptr, false);

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);
        if (this->width > 0) {
            Vector2 pos1, pos2;

            pos1.x = this->position.x - (this->width << 15);
            pos1.y = this->position.y;

            pos2.x = this->position.x + (this->width << 15);
            pos2.y = this->position.y;

            DrawHelpers::DrawLine(pos1, pos2, 0xFFFF00);
        }

        if (this->offset > 0) {
            Vector2 pos1, pos2;

            switch (this->type) {
                case BoundsMarker::AnyY:
                case BoundsMarker::BelowYAny:
                    pos1.x = this->position.x;
                    pos1.y = this->position.y;

                    pos2.x = this->position.x;
                    pos2.y = this->position.y;
                    break;

                case BoundsMarker::AboveY:
                    pos1.x = this->position.x;
                    pos1.y = this->position.y - (this->offset << 16);

                    pos2.x = this->position.x;
                    pos2.y = this->position.y;
                    break;

                case BoundsMarker::BelowY:
                    pos1.x = this->position.x;
                    pos1.y = this->position.y;

                    pos2.x = this->position.x;
                    pos2.y = this->position.y + (this->offset << 16);
                    break;
            }

            DrawHelpers::DrawLine(pos1, pos2, 0xFFFF00);
        }

        RSDK_DRAWING_OVERLAY(false);
    }
}

void BoundsMarker::EditorLoad()
{
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Any Y");
    RSDK_ENUM_VAR("Above Y");
    RSDK_ENUM_VAR("Below Y");
    RSDK_ENUM_VAR("Below Y Any");

    RSDK_ACTIVE_VAR(sVars, alignFrom);
    RSDK_ENUM_VAR("Player");
    RSDK_ENUM_VAR("Camera Left");
    RSDK_ENUM_VAR("Camera Right");

    RSDK_ACTIVE_VAR(sVars, lockLeft);
    RSDK_ENUM_VAR("No Lock");
    RSDK_ENUM_VAR("Lock at Boundary");
    RSDK_ENUM_VAR("Lock with Camera");

    RSDK_ACTIVE_VAR(sVars, lockRight);
    RSDK_ENUM_VAR("No Lock");
    RSDK_ENUM_VAR("Lock at Boundary");
    RSDK_ENUM_VAR("Lock with Camera");

    RSDK_ACTIVE_VAR(sVars, onExit);
    RSDK_ENUM_VAR("Restore Boundaries");
    RSDK_ENUM_VAR("Deactivate Object");
    RSDK_ENUM_VAR("Destroy Object");
    RSDK_ENUM_VAR("Keep Boundaries");
}
#endif

#if RETRO_REV0U
void BoundsMarker::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(BoundsMarker);

    sVars->aniFrames.Init();
}
#endif

void BoundsMarker::Serialize()
{
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_UINT8, alignFrom);
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_INT32, width);
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_INT32, offset);
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_VECTOR2, restoreSpd);
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_VECTOR2, restoreAccel);
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_UINT32, lockLeft);
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_UINT32, lockRight);
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_BOOL, vsDisable);
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_BOOL, deactivated);
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_UINT8, onExit);
    RSDK_EDITABLE_VAR(BoundsMarker, VAR_INT32, parameter);
}

} // namespace GameLogic
