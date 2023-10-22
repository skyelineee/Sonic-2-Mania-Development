// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TransportTube Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "TransportTube.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TransportTube);

void TransportTube::Update() { this->state.Run(this); }

void TransportTube::LateUpdate() {}

void TransportTube::StaticUpdate() {}

void TransportTube::Draw() {}

void TransportTube::Create(void *data)
{
    this->drawFX = FX_FLIP;
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = TO_FIXED(192);
        this->updateRange.y = TO_FIXED(192);
        TransportTube::SetupDirections(this);

        switch (this->type) {
            case TRANSPORTTUBE_CHANGEDIR: this->state.Set(&TransportTube::State_ChangeDir); break;
            case TRANSPORTTUBE_ENTRY: this->state.Set(&TransportTube::State_Entry); break;

            case TRANSPORTTUBE_TOTARGET_NEXT:
            case TRANSPORTTUBE_TOTARGET_PREV: this->state.Set(&TransportTube::State_ToTargetEntity); break;

            case TRANSPORTTUBE_TOTARGET_NODE: this->state.Set(&TransportTube::State_TargetSeqNode); break;
            case TRANSPORTTUBE_JUNCTION: this->state.Set(&TransportTube::State_ChooseDir); break;
            case TRANSPORTTUBE_EXIT: this->state.Set(&TransportTube::State_Exit); break;
            default: break;
        }
    }
}

void TransportTube::StageLoad() { sVars->sfxTravel.Get("Tube/Travel.wav"); }

void TransportTube::SetupDirections(TransportTube *entity)
{
    int32 velocityX[] = { 0, 0, -16, 16, 12, -12, 12, -12 };
    int32 velocityY[] = { -16, 16, 0, 0, -12, -12, 12, 12 };

    entity->directionCount = 0;
    for (int32 i = 0; i < 8; ++i) {
        if (entity->dirMask & (1 << i)) {
            entity->dirVelocity[entity->directionCount].x = velocityX[i];
            entity->dirVelocity[entity->directionCount].y = velocityY[i];
            ++entity->directionCount;
        }
    }
}

void TransportTube::HandleVelocityChange(int32 velX, int32 velY)
{
    int32 id       = 0;
    int32 velStore = 0xFFFF;
    for (int32 v = 0; v < this->directionCount; ++v) {
        int32 x = abs(velX - this->dirVelocity[v].x);
        int32 y = abs(velY - this->dirVelocity[v].y);

        if (x + y < velStore) {
            id       = v;
            velStore = x + y;
        }
    }

    int32 pos = 0;
    for (int32 v = 0; v < this->directionCount; ++v) {
        if (v != id)
            this->directionIDs[pos++] = v;
    }

    int32 dir        = this->directionIDs[Math::Rand(0, this->directionCount - 1)];
    this->velocity.x = TO_FIXED(this->dirVelocity[dir].x);
    this->velocity.y = TO_FIXED(this->dirVelocity[dir].y);
}

void TransportTube::State_ChangeDir()
{
    for (int32 i = 0; i < Player::sVars->playerCount; ++i) {
        Player *player = GameObject::Get<Player>(i);
        int32 rx             = FROM_FIXED(player->position.x - this->position.x);
        int32 ry             = FROM_FIXED(player->position.y - this->position.y);

        if (this->playerTimers[i]) {
            if (rx * rx + ry * ry >= 0xC0)
                this->playerTimers[i]--;
        }
        else if (player->state.Matches(&Player::State_TransportTube) && !sVars->nextSlot[i] && rx * rx + ry * ry < 0xC0) {
            TransportTube::HandleVelocityChange(rx - FROM_FIXED(player->velocity.x), ry - FROM_FIXED(player->velocity.y));
            player->position.x    = this->position.x;
            player->position.y    = this->position.y;
            player->velocity      = this->velocity;
            this->playerTimers[i] = 2;
        }
    }
}

void TransportTube::State_Entry(void)
{
    for (int32 i = 0; i < Player::sVars->playerCount; ++i) {
        Player *player = GameObject::Get<Player>(i);

        if (player->CheckValidState()) {
            int32 rx = FROM_FIXED(player->position.x - this->position.x);
            int32 ry = FROM_FIXED(player->position.y - this->position.y);
            if (this->playerTimers[i]) {
                if (rx * rx + ry * ry >= 0xC0)
                    this->playerTimers[i]--;
            }
            else if (rx * rx + ry * ry < 0xC0) {
                player->position.x = this->position.x;
                player->position.y = this->position.y;
                if (player->state.Matches(&Player::State_TransportTube)) {
                    player->drawGroup      = Zone::sVars->playerDrawGroup[0];
                    player->tileCollisions = TILECOLLISION_DOWN;
                    player->interaction    = true;
                    player->state.Set(&Player::State_Air);
                    if (player->velocity.x)
                        player->direction = player->velocity.x <= 0;
                    this->playerTimers[i] = 2;
                }
                else {
                    TransportTube::HandleVelocityChange(rx - FROM_FIXED(player->velocity.x), ry - FROM_FIXED(player->velocity.y));
                    player->velocity = this->velocity;
                    player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP, false, 0);
                    player->drawGroup          = 1;
                    player->onGround           = false;
                    player->tileCollisions     = TILECOLLISION_NONE;
                    player->interaction        = false;
                    player->state.Set(&Player::State_TransportTube);
                    player->stateGravity.Set(&Player::Gravity_False);
                    player->nextAirState.Set(nullptr);
                    player->nextGroundState.Set(nullptr);
                    sVars->nextSlot[i] = 0;
                    sVars->sfxTravel.Play(false, 255);
                    this->playerTimers[i] = 2;
                }
            }
        }
    }
}

void TransportTube::State_ToTargetEntity()
{
    for (int32 i = 0; i < Player::sVars->playerCount; ++i) {
        Player *player       = GameObject::Get<Player>(i);
        int32 rx             = FROM_FIXED(player->position.x - this->position.x);
        int32 ry             = FROM_FIXED(player->position.y - this->position.y);

        if (this->playerTimers[i]) {
            if (rx * rx + ry * ry >= 0xC0)
                --this->playerTimers[i];
        }
        else if (player->state.Matches(&Player::State_TransportTube) && rx * rx + ry * ry < 0xC0) {
            if (this->players[i]) {
                sVars->nextSlot[i] = 0;
                this->players[i]           = nullptr;
                this->playerTimers[i]      = 2;
            }
            else {
                if (this->type == TRANSPORTTUBE_TOTARGET_NEXT)
                    sVars->nextSlot[i] = 1;
                else
                    sVars->nextSlot[i] = -1;

                TransportTube *node = GameObject::Get<TransportTube>(sceneInfo->entitySlot + sVars->nextSlot[i]);
                int32 angle        = Math::ATan2(FROM_FIXED(node->position.x - player->position.x), FROM_FIXED(node->position.y - player->position.y));
                player->velocity.x = 0xC00 * Math::Cos256(angle);
                player->velocity.y = 0xC00 * Math::Sin256(angle);
                node->players[i]   = player;
                this->playerTimers[i] = 2;
            }
        }
    }
}

void TransportTube::State_TargetSeqNode()
{
    for (int32 i = 0; i < Player::sVars->playerCount; ++i) {
        Player *player = this->players[i];
        if (player) {
            if (player->state.Matches(&Player::State_TransportTube)) {
                int32 rx = FROM_FIXED(player->position.x - this->position.x);
                int32 ry = FROM_FIXED(player->position.y - this->position.y);
                if (rx * rx + ry * ry < 0xC0) {
                    player->position.x = this->position.x;
                    player->position.y = this->position.y;

                    TransportTube *node = GameObject::Get<TransportTube>(sceneInfo->entitySlot + sVars->nextSlot[i]);
                    int32 angle = Math::ATan2(FROM_FIXED(node->position.x - player->position.x), FROM_FIXED(node->position.y - player->position.y));
                    player->velocity.x = 0xC00 * Math::Cos256(angle);
                    player->velocity.y = 0xC00 * Math::Sin256(angle);
                    this->players[i]   = nullptr;
                    node->players[i]   = player;
                }
            }
            else {
                this->players[i] = nullptr;
            }
        }
    }
}

void TransportTube::State_ChooseDir()
{
    for (int32 i = 0; i < Player::sVars->playerCount; ++i) {
        Player *player       = GameObject::Get<Player>(i);
        int32 rx             = FROM_FIXED(player->position.x - this->position.x);
        int32 ry             = FROM_FIXED(player->position.y - this->position.y);

        if (this->playerTimers[i]) {
            if (rx * rx + ry * ry >= 0xC0)
                --this->playerTimers[i];
        }
        else if (player->state.Matches(&Player::State_TransportTube) && rx * rx + ry * ry < 0xC0) {
            player->position.x = this->position.x;
            player->position.y = this->position.y;
            player->velocity.x = 0;
            player->velocity.y = 0;

            uint8 moveMask = this->dirMask & ((player->up << 0) | (player->down << 1) | (player->left << 2) | (player->right << 3));
            if (moveMask & 1)
                player->velocity.y = -0x100000;
            else if (moveMask & 2)
                player->velocity.y = 0x100000;
            else if (moveMask & 4)
                player->velocity.x = -0x100000;
            else if (moveMask & 8)
                player->velocity.x = 0x100000;

            if (player->velocity.x || player->velocity.y) {
                this->playerTimers[i] = 2;
                sVars->sfxTravel.Play(false, 255);
            }
        }
    }
}

void TransportTube::State_Exit()
{
    for (int32 i = 0; i < Player::sVars->playerCount; ++i) {
        Player *player = GameObject::Get<Player>(i);

        if (player->CheckValidState()) {
            int32 rx = FROM_FIXED(player->position.x - this->position.x);
            int32 ry = FROM_FIXED(player->position.y - this->position.y);

            if (this->playerTimers[i]) {
                if (rx * rx + ry * ry >= 0xC0)
                    this->playerTimers[i]--;
            }
            else if (rx * rx + ry * ry < 0xC0) {
                player->position.x     = this->position.x;
                player->position.y     = this->position.y;
                player->drawGroup      = Zone::sVars->playerDrawGroup[0];
                player->tileCollisions = TILECOLLISION_DOWN;
                player->interaction    = true;
                player->state.Set(&Player::State_Air);
                this->playerTimers[i]  = 2;
            }
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void TransportTube::EditorDraw()
{
    this->updateRange.x = 0xC00000;
    this->updateRange.y = 0xC00000;

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->animator.DrawSprite(nullptr, false);

    RSDK_DRAWING_OVERLAY(true);

    TransportTube::SetupDirections(this);
    for (int32 v = 0; v < this->directionCount; ++v) {
        DrawHelpers::DrawArrow(this->position.x, this->position.y, this->position.x + (this->dirVelocity[v].x << 18),
                              this->position.y + (this->dirVelocity[v].y << 18), 0xFF0000, INK_NONE, 0xFF);
    }

    RSDK_DRAWING_OVERLAY(false);

    this->animator.SetAnimation(sVars->aniFrames, this->type == TRANSPORTTUBE_JUNCTION ? 1 : 2, true, 0);
    this->animator.DrawSprite(nullptr, false);
}

void TransportTube::EditorLoad()
{
    sVars->aniFrames.Load("CPZ/TransportTube.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Change Direction", TRANSPORTTUBE_CHANGEDIR);
    RSDK_ENUM_VAR("Entrance", TRANSPORTTUBE_ENTRY);
    RSDK_ENUM_VAR("Start Sequence (Next Slot)", TRANSPORTTUBE_TOTARGET_NEXT);
    RSDK_ENUM_VAR("Start Sequence (Prev Slot)", TRANSPORTTUBE_TOTARGET_PREV);
    RSDK_ENUM_VAR("Sequence Node", TRANSPORTTUBE_TOTARGET_NODE);
    RSDK_ENUM_VAR("Junction", TRANSPORTTUBE_JUNCTION);
    RSDK_ENUM_VAR("Exit", TRANSPORTTUBE_EXIT);
}
#endif

void TransportTube::Serialize()
{
    RSDK_EDITABLE_VAR(TransportTube, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(TransportTube, VAR_UINT8, dirMask);
}
} // namespace GameLogic