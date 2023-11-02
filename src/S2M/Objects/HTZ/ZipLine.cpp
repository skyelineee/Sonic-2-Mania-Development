// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ZipLine Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "ZipLine.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ZipLine);

void ZipLine::Update()
{
    this->state.Run(this);

    int32 storeX = this->position.x;
    int32 storeY = this->position.y;

    this->position = this->handlePos;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 playerID = RSDKTable->GetEntitySlot(player);
        if (this->grabDelay[playerID])
            this->grabDelay[playerID]--;

        if ((1 << playerID) & this->activePlayers) {
            if (player->CheckValidState()) {
                Hitbox *playerHitbox = player->GetHitbox();

                if (!player->state.Matches(&Player::State_Hurt)) {
                    if (player->velocity.x) {
                        this->groundVel = player->groundVel;
                        if (this->angle >= 0x40 && this->angle <= 0xC0)
                            this->groundVel = -player->groundVel;

                        this->groundVel = CLAMP(this->groundVel, -0xA0000, 0xA0000);

                        this->velocity.x   = this->groundVel * Math::Cos256(this->angle) >> 8;
                        this->velocity.y   = this->groundVel * Math::Sin256(this->angle) >> 8;
                        player->velocity.x = 0;
                        player->velocity.y = 0;
                        player->groundVel  = 0;
                        player->angle      = 0;
                        player->rotation   = 0;
                    }

                    int32 lastX        = player->position.x;
                    player->position.x = this->position.x;
                    player->position.y = this->position.y + (((sVars->hitboxHandle.bottom - sVars->hitboxHandle.top) << 15) & 0xFFFF0000)
                                         + ((sVars->hitboxHandle.top - playerHitbox->top) << 16);

                    if (abs(lastX - this->position.x) <= 0x100000) {
                        if (!this->grabDelay[playerID] && player->jumpPress) {
                            player->velocity.y       = -0x40000;
                            player->jumpAbilityState = 1;
                            player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP, false, 0);
                            player->animator.speed = 48;
                            player->state.Set(&Player::State_Air);
                            this->grabDelay[playerID] = 60;
                            this->activePlayers &= ~(1 << playerID);
                            player->onGround       = false;
                            player->groundedStore  = false;
                            player->tileCollisions = TILECOLLISION_DOWN;
                        }
                    }
                    else {
                        player->state.Set(&Player::State_Air);
                        this->grabDelay[playerID] = 60;
                        this->activePlayers &= ~(1 << playerID);
                        player->onGround       = false;
                        player->groundedStore  = false;
                        player->tileCollisions = TILECOLLISION_DOWN;
                    }
                }
                else {
                    this->grabDelay[playerID] = 60;
                    this->activePlayers &= ~(1 << playerID);
                    player->onGround       = false;
                    player->groundedStore  = false;
                    player->tileCollisions = TILECOLLISION_DOWN;
                }
            }
            else {
                this->activePlayers &= ~(1 << playerID);
                if (!player->state.Matches(&Player::State_Death)) {
                    player->tileCollisions = TILECOLLISION_DOWN;
                }
            }
        }
        else if (!this->grabDelay[playerID] && !player->state.Matches(&Player::State_Static) && !player->down) {
            Hitbox *playerHitbox = player->GetHitbox();
            Hitbox otherHitbox;
            otherHitbox.top    = playerHitbox->top - 4;
            otherHitbox.left   = playerHitbox->left;
            otherHitbox.right  = playerHitbox->right;
            otherHitbox.bottom = otherHitbox.top + 8;

            if (this->CheckCollisionTouchBox(&sVars->hitboxHandle, player, &otherHitbox)) {
                if (player->sidekick || this->state.Matches(&ZipLine::State_Moving)) {
                    ZipLine::GrabHandle(player, playerID, playerHitbox);
                }
                else if (this->state.Matches(nullptr)) {
                    this->groundVel = player->groundVel;
                    if (this->angle >= 0x40 && this->angle <= 0xC0)
                        this->groundVel = -player->groundVel;

                    this->groundVel = CLAMP(this->groundVel, -0xA0000, 0xA0000);

                    this->velocity.x = this->groundVel * Math::Cos256(this->angle) >> 8;
                    this->velocity.y = this->groundVel * Math::Sin256(this->angle) >> 8;

                    if (this->angle & 0x7F) {
                        if ((uint8)this->angle < 0x80) {
                            if (this->handlePos.x == this->startPos.x) {
                                if (this->velocity.y < 0) {
                                    this->velocity.x = 0;
                                    this->velocity.y = 0;
                                    this->groundVel  = 0;
                                }
                                this->state.Set(&ZipLine::State_Moving);
                            }

                            if (this->handlePos.x == this->endPos.x && this->velocity.y < 0)
                                ZipLine::GrabHandle(player, playerID, playerHitbox);
                        }
                        else {
                            if (this->handlePos.x == this->endPos.x) {
                                if (this->velocity.y < 0) {
                                    this->velocity.x = 0;
                                    this->velocity.y = 0;
                                    this->groundVel  = 0;
                                }

                                this->state.Set(&ZipLine::State_Moving);
                            }

                            if (this->handlePos.x == this->startPos.x && this->velocity.y < 0)
                                ZipLine::GrabHandle(player, playerID, playerHitbox);
                        }
                    }
                    else if (this->groundVel) {
                        if ((this->groundVel < 0 && this->handlePos.x != this->startPos.x)
                            || (this->groundVel > 0 && this->handlePos.x != this->endPos.x)) {
                            ZipLine::GrabHandle(player, playerID, playerHitbox);
                        }
                    }
                }
            }
        }
    }
    this->position.x = storeX;
    this->position.y = storeY;
}

void ZipLine::LateUpdate() {}

void ZipLine::StaticUpdate() {}

void ZipLine::Draw()
{
    Graphics::DrawLine(this->startPos.x, this->startPos.y, this->endPos.x, this->endPos.y, 0x6060A0, 0x00, INK_NONE, false);
    Graphics::DrawLine(this->startPos.x, this->startPos.y + 0x10000, this->endPos.x, this->endPos.y + 0x10000, 0x303070, 0x00, INK_NONE, false);

    this->animator.frameID = 0;
    this->animator.DrawSprite(&this->handlePos, false);
    this->animator.frameID = 1;

    this->animator.DrawSprite(&this->startPos, false);
    this->animator.DrawSprite(&this->endPos, false);
}

void ZipLine::Create(void *data)
{
    this->active  = ACTIVE_BOUNDS;
    this->visible = true;
    this->drawFX  = FX_FLIP;
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->drawGroup = Zone::sVars->playerDrawGroup[0];

    this->handlePos = this->position;
    this->startPos  = this->position;
    this->endPos.x  = this->position.x + (this->length << 8) * Math::Cos256(this->angle);
    this->endPos.y  = this->position.y + (this->length << 8) * Math::Sin256(this->angle);

    if (!sceneInfo->inEditor) {
        this->position.x += (this->endPos.x - this->startPos.x) >> 1;
        this->position.y += (this->endPos.y - this->startPos.y) >> 1;
        this->updateRange.x = (abs(this->endPos.x - this->startPos.x) >> 1) + 0x400000;
        this->updateRange.y = (abs(this->endPos.y - this->startPos.y) >> 1) + 0x400000;
        this->joinPos       = ZipLine::GetJoinPos();
    }
}

void ZipLine::StageLoad()
{
    sVars->aniFrames.Load("HTZ/ZipLine.bin", SCOPE_STAGE);

    sVars->hitboxHandle.top    = 0;
    sVars->hitboxHandle.left   = -8;
    sVars->hitboxHandle.bottom = 24;
    sVars->hitboxHandle.right  = 8;

    // Zone_AddVSSwapCallback(ZipLine::VSSwap_CheckBusy);
}

void ZipLine::VSSwap_CheckBusy()
{
    for (auto zipline : GameObject::GetEntities<ZipLine>(FOR_ACTIVE_ENTITIES)) {
        if ((1 << Zone::sVars->swapPlayerID) & zipline->activePlayers)
            Zone::sVars->playerSwapEnabled[Zone::sVars->swapPlayerID] = false;
    }
}

void ZipLine::GrabHandle(Player *player, int32 playerID, Hitbox *playerHitbox)
{
    this->state.Set(&ZipLine::State_Moving);
    this->activePlayers |= 1 << playerID;
    player->onGround      = false;
    player->groundedStore = false;
    player->velocity.x    = 0;
    player->velocity.y    = 0;
    player->groundVel     = 0;
    player->angle         = 0;
    player->rotation      = 0;
    player->position.x    = this->position.x;
    player->position.y    = this->position.y;
    player->position.y +=
        ((sVars->hitboxHandle.top - playerHitbox->top) << 16) + (((sVars->hitboxHandle.bottom - sVars->hitboxHandle.top) << 15) & 0xFFFF0000);
    player->tileCollisions = TILECOLLISION_NONE;
    player->animator.SetAnimation(player->aniFrames, Player::ANI_HANG, true, 0);
    player->state.Set(&Player::State_Static);
    player->nextAirState.Set(nullptr);
    player->nextGroundState.Set(nullptr);
    Player::sVars->sfxGrab.Play(false, 0xFF);
    this->grabDelay[playerID] = 15;
    this->active              = ACTIVE_NORMAL;
}

void ZipLine::ForceReleasePlayers()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 playerID = RSDKTable->GetEntitySlot(player);
        if ((1 << playerID) & this->activePlayers) {
            this->grabDelay[playerID] = 60;
            player->velocity.y        = this->velocity.y;
            player->velocity.x        = this->velocity.x;
            player->groundVel         = player->velocity.x;
            this->activePlayers &= ~(1 << playerID);
            player->onGround       = false;
            player->groundedStore  = false;
            player->tileCollisions = TILECOLLISION_DOWN;
            player->state.Set(&Player::State_Air);
        }
    }
}

// this func actually rules, you can join any number of ZipLines together using this
Vector2 ZipLine::GetJoinPos()
{
    ZipLine *endMarker = GameObject::Get<ZipLine>(sceneInfo->entitySlot - 1);
    Vector2 result;
    result.x = 0;
    result.y = 0;

    if (endMarker->classID == sVars->classID) {
        Hitbox thisHitbox, otherHitbox;

        if (this->startPos.y >= this->endPos.y) {
            otherHitbox.top    = (this->endPos.y - this->position.y) >> 16;
            otherHitbox.bottom = (this->startPos.y - this->position.y) >> 16;
        }
        else {
            otherHitbox.top    = (this->startPos.y - this->position.y) >> 16;
            otherHitbox.bottom = (this->endPos.y - this->position.y) >> 16;
        }

        if (this->startPos.x >= this->endPos.x) {
            otherHitbox.left  = (this->endPos.x - this->position.y) >> 16;
            otherHitbox.right = (this->startPos.x - this->position.y) >> 16;
        }
        else {
            otherHitbox.left  = (this->startPos.x - this->position.x) >> 16;
            otherHitbox.right = (this->endPos.x - this->position.x) >> 16;
        }

        if (endMarker->startPos.y >= endMarker->endPos.y) {
            thisHitbox.top    = (endMarker->endPos.y - endMarker->position.y) >> 16;
            thisHitbox.bottom = (endMarker->startPos.y - endMarker->position.y) >> 16;
        }
        else {
            thisHitbox.top    = (endMarker->startPos.y - endMarker->position.y) >> 16;
            thisHitbox.bottom = (endMarker->endPos.y - endMarker->position.y) >> 16;
        }

        if (endMarker->startPos.x >= endMarker->endPos.x) {
            thisHitbox.left  = (endMarker->endPos.x - endMarker->position.y) >> 16;
            thisHitbox.right = (endMarker->startPos.x - endMarker->position.y) >> 16;
        }
        else {
            thisHitbox.left  = (endMarker->startPos.x - endMarker->position.x) >> 16;
            thisHitbox.right = (endMarker->endPos.x - endMarker->position.x) >> 16;
        }

        otherHitbox.left -= 8;
        otherHitbox.top -= 8;
        otherHitbox.right += 8;
        otherHitbox.bottom += 8;
        thisHitbox.left -= 8;
        thisHitbox.top -= 8;
        thisHitbox.right += 8;
        thisHitbox.bottom += 8;
        if (endMarker->CheckCollisionTouchBox(&thisHitbox, this, &otherHitbox)) {
            int32 distX1 = (this->startPos.x >> 17) - (this->endPos.x >> 17);
            int32 distY1 = (this->endPos.y >> 17) - (this->startPos.y >> 17);
            int32 distX2 = (endMarker->startPos.x >> 17) - (endMarker->endPos.x >> 17);
            int32 distY2 = (endMarker->endPos.y >> 17) - (endMarker->startPos.y >> 17);

            int32 val1    = distX1 * (this->startPos.y >> 17) + distY1 * (this->startPos.x >> 17);
            int32 val2    = distX2 * (endMarker->startPos.y >> 17) + distY2 * (endMarker->startPos.x >> 17);
            float divisor = (float)(distY1 * distX2 - distX1 * distY2);
            if (divisor != 0.0f) {
                endMarker->handlePos.x = -0x100000;
                result.x               = (int32)((float)(distX2 * val1 - distX1 * val2) / divisor) << 17;
                result.y               = (int32)((float)(distY1 * val2 - distY2 * val1) / divisor) << 17;
            }
        }
    }
    return result;
}

void ZipLine::State_Moving()
{
    this->groundVel += (Math::Sin256(this->angle) << 14 >> 8);
    if (this->groundVel >= 0xA0000)
        this->groundVel = 0xA0000;

    this->velocity.x = this->groundVel * Math::Cos256(this->angle) >> 8;
    this->velocity.y = this->groundVel * Math::Sin256(this->angle) >> 8;

    if (this->joinPos.x) {
        int32 storeX     = this->position.x;
        int32 storeY     = this->position.y;
        this->position.x = this->handlePos.x;
        this->position.y = this->handlePos.y;

        Hitbox otherHitbox;
        otherHitbox.top    = ((this->joinPos.y - this->position.y - (this->velocity.y >> 1)) >> 16) + 8;
        otherHitbox.bottom = (((this->velocity.y >> 1) + (this->joinPos.y - this->position.y)) >> 16) + 16;

        if (this->velocity.x >= 0) {
            otherHitbox.left  = (this->joinPos.x - this->position.x) >> 16;
            otherHitbox.right = ((this->joinPos.x + this->velocity.x - this->position.x) >> 16) + 5;
        }
        else {
            otherHitbox.left  = (((this->joinPos.x - this->position.x) - this->velocity.x) >> 16) - 5;
            otherHitbox.right = (this->joinPos.x - this->position.x) >> 16;
        }

        if (this->CheckCollisionTouchBox(&sVars->hitboxHandle, this, &otherHitbox)) {
            ZipLine *endMarker       = GameObject::Get<ZipLine>(sceneInfo->entitySlot - 1);
            endMarker->handlePos.x   = this->joinPos.x;
            endMarker->handlePos.y   = this->joinPos.y;
            endMarker->onGround      = true;
            endMarker->activePlayers = this->activePlayers;
            endMarker->groundVel     = this->groundVel;
            endMarker->state.Set(&ZipLine::State_Moving);
            this->position.x    = storeX;
            this->position.y    = storeY;
            this->activePlayers = 0;
            this->groundVel     = 0;
            this->handlePos.x   = -0x100000;
            this->state.Set(nullptr);
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                if ((1 << RSDKTable->GetEntitySlot(player)) & endMarker->activePlayers) {
                    Hitbox *playerHitbox = player->GetHitbox();
                    player->velocity.x   = 0;
                    player->velocity.y   = 0;
                    player->groundVel    = 0;
                    player->angle        = 0;
                    player->rotation     = 0;
                    player->position.x   = endMarker->handlePos.x;
                    player->position.y   = endMarker->handlePos.y + (((sVars->hitboxHandle.bottom - sVars->hitboxHandle.top) << 15) & 0xFFFF0000)
                                         + ((sVars->hitboxHandle.top - playerHitbox->top) << 16);
                }
            }
            return;
        }
        this->position.x = storeX;
        this->position.y = storeY;
    }

    this->handlePos.x += this->velocity.x;
    this->handlePos.y += this->velocity.y;
    if (this->groundVel < 0) {
        if (this->velocity.x < 0) {
            if (this->handlePos.x < this->startPos.x)
                this->handlePos.x = this->startPos.x;

            if (this->velocity.y < 0) {
                if (this->handlePos.y < this->startPos.y)
                    this->handlePos.y = this->startPos.y;
            }
            if (this->velocity.y > 0) {
                if (this->handlePos.y > this->startPos.y)
                    this->handlePos.y = this->startPos.y;
            }

            if (this->handlePos.x == this->startPos.x && this->handlePos.y == this->startPos.y) {
                this->groundVel = 0;
                if (this->startPos.y >= this->endPos.y || !this->onGround) {
                    this->onGround = false;
                    this->active   = ACTIVE_BOUNDS;
                    this->state.Set(nullptr);
                    ZipLine::ForceReleasePlayers();
                }
            }
        }
        else if (this->velocity.x > 0) {
            if (this->handlePos.x > this->startPos.x)
                this->handlePos.x = this->startPos.x;

            if (this->velocity.y < 0) {
                if (this->handlePos.y < this->startPos.y)
                    this->handlePos.y = this->startPos.y;
            }
            if (this->velocity.y > 0) {
                if (this->handlePos.y > this->startPos.y)
                    this->handlePos.y = this->startPos.y;
            }

            if (this->handlePos.x == this->startPos.x && this->handlePos.y == this->startPos.y) {
                this->groundVel = 0;
                if (this->startPos.y >= this->endPos.y || !this->onGround) {
                    this->onGround = false;
                    this->active   = ACTIVE_BOUNDS;
                    this->state.Set(nullptr);
                    ZipLine::ForceReleasePlayers();
                }
            }
        }
    }
    else if (this->groundVel > 0) {
        if (this->velocity.x > 0) {
            if (this->handlePos.x > this->endPos.x) {
                this->handlePos.x = this->endPos.x;
            }
        }
        else if (this->velocity.x < 0) {
            this->endPos.x = this->endPos.x;
            if (this->handlePos.x < this->endPos.x) {
                this->handlePos.x = this->endPos.x;
            }
        }

        if (this->velocity.y < 0) {
            if (this->handlePos.y < this->endPos.y)
                this->handlePos.y = this->endPos.y;
        }
        else if (this->velocity.y > 0) {
            if (this->handlePos.y > this->endPos.y)
                this->handlePos.y = this->endPos.y;
        }

        if (this->handlePos.x == this->endPos.x && this->handlePos.y == this->endPos.y) {
            this->groundVel = 0;
            if (this->endPos.y >= this->startPos.y || !this->onGround) {
                this->onGround = false;
                this->active   = ACTIVE_BOUNDS;
                this->state.Set(nullptr);
                ZipLine::ForceReleasePlayers();
            }
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void ZipLine::EditorDraw()
{
    this->handlePos = this->position;
    this->startPos  = this->position;
    this->endPos.x  = this->position.x + (this->length << 8) * Math::Cos256(this->angle);
    this->endPos.y  = this->position.y + (this->length << 8) * Math::Sin256(this->angle);

    ZipLine::Draw();
}

void ZipLine::EditorLoad() { sVars->aniFrames.Load("HTZ/ZipLine.bin", SCOPE_STAGE); }
#endif

void ZipLine::Serialize()
{
    RSDK_EDITABLE_VAR(ZipLine, VAR_INT32, angle);
    RSDK_EDITABLE_VAR(ZipLine, VAR_ENUM, length);
}

} // namespace GameLogic