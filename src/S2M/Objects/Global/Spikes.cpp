// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Spikes Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Spikes.hpp"
#include "Zone.hpp"
#include "Shield.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Spikes);

void Spikes::Update()
{
    switch (this->stateMove) {
        default:
        case Spikes::MoveStatic: break;

        case Spikes::MoveHidden:
            if (this->stagger << 6 == (Zone::sVars->timer & 0x40)) {
                if ((Zone::sVars->timer & 0x3F) == this->timer) {
                    this->stateMove++;
                    if (this->onScreen == 1)
                        sVars->sfxMove.Play();
                }
            }
            break;

        case Spikes::MoveAppear:
            if (this->moveOffset >= 0x200000) {
                this->stateMove++;
            }
            else {
                this->moveOffset += 0x80000;
                this->position.x += this->velocity.x;
                this->position.y += this->velocity.y;
            }
            break;

        case Spikes::MoveShown:
            if ((Zone::sVars->timer & 0x3F) == this->timer) {
                this->stateMove++;
                if (this->onScreen == 1)
                    sVars->sfxMove.Play();
            }
            break;

        case Spikes::MoveDisappear:
            if (this->moveOffset <= 0) {
                this->stateMove = Spikes::MoveHidden;
            }
            else {
                this->moveOffset -= 0x80000;
                this->position.x -= this->velocity.x;
                this->position.y -= this->velocity.y;
            }
            break;
    }

    this->position.x -= this->collisionOffset.x;
    this->position.y -= this->collisionOffset.y;

    if (this->stateMove != Spikes::MoveDisappear) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (this->planeFilter <= 0 || player->collisionPlane == (((uint8)this->planeFilter - 1) & 1)) {
                Vector2 velocity = player->velocity;

                switch (player->CheckCollisionBox(this, &this->hitbox)) {
                    default:
                    case C_NONE: break;

                    case C_TOP:
                        player->collisionFlagV |= 1;
                        if (this->moveOffset == 0x80000)
                            player->onGround = false;

                        if (player->field_3D0.y != player->velocity.y >= 0 || this->stateMove == Spikes::MoveAppear) {
                            player->position.x += this->collisionOffset.x;
                            player->position.y += this->collisionOffset.y;
                            if (this->type == C_TOP)
                                CheckHit(player, velocity.x, velocity.y);
                        }
                        break;

                    case C_LEFT:
                        player->collisionFlagH |= 1;
                        if ((player->field_3D0.x != player->velocity.x >= 0 || this->stateMove == Spikes::MoveAppear) && this->type == C_LEFT)
                            CheckHit(player, velocity.x, velocity.y);
                        break;

                    case C_RIGHT:
                        player->collisionFlagH |= 2;
                        if ((player->field_3D0.x != player->velocity.x <= 0 || this->stateMove == Spikes::MoveAppear) && this->type == C_RIGHT)
                            CheckHit(player, velocity.x, velocity.y);
                        break;
                    case C_BOTTOM:
                        player->collisionFlagV |= 2;
                        if ((player->field_3D0.y != player->velocity.y <= 0 || this->stateMove == Spikes::MoveAppear) && this->type == C_BOTTOM)
                            CheckHit(player, velocity.x, velocity.y);
                        break;
                }

            }
        }
    }

    this->position.x += this->collisionOffset.x;
    this->position.y += this->collisionOffset.y;
}
void Spikes::LateUpdate() {}
void Spikes::StaticUpdate()
{
    sVars->verticalAnimator.Process();
    sVars->horizontalAnimator.Process();
}
void Spikes::Draw() { sVars->stateDraw.Run(this); }

void Spikes::Create(void *data)
{

    this->drawFX = FX_FLIP;
    if (this->count < 2)
        this->count = 2;

    if (!sceneInfo->inEditor) {
        if (data)
            this->type = VOID_TO_INT(data);

        this->active  = ACTIVE_BOUNDS;
        int32 dir     = this->type & 1;
        this->visible = true;
        this->type    = (this->type >> 1) & 1;
        if (this->planeFilter > 0 && ((uint8)this->planeFilter - 1) & 2)
            this->drawGroup = Zone::sVars->objectDrawGroup[1];
        else
            this->drawGroup = Zone::sVars->objectDrawGroup[0];
        this->alpha = 0x80;

        switch (this->type) {
            case 0: // vertical
                this->updateRange.x = (this->count + 6) << 20;
                this->updateRange.y = 0x600000;
                this->direction     = FLIP_Y * dir;
                if (this->direction) {
                    this->velocity.y = 0x80000;
                    this->type       = C_BOTTOM;
                }
                else {
                    this->velocity.y = -0x80000;
                    this->type       = C_TOP;
                }
                this->hitbox.left   = -8 * this->count;
                this->hitbox.top    = -16;
                this->hitbox.right  = 8 * this->count;
                this->hitbox.bottom = 16;
                break;

            case 1: // horizontal
                this->updateRange.x = 0x600000;
                this->updateRange.y = (this->count + 6) << 20;
                this->direction     = dir;
                if (this->direction) {
                    this->velocity.x = -0x80000;
                    this->type       = C_LEFT;
                }
                else {
                    this->velocity.x = 0x80000;
                    this->type       = C_RIGHT;
                }
                this->hitbox.left   = -16;
                this->hitbox.top    = -8 * this->count;
                this->hitbox.right  = 16;
                this->hitbox.bottom = 8 * this->count;
                break;
        }

        if (this->moving) {
            this->position.x -= 4 * this->velocity.x;
            this->position.y -= 4 * this->velocity.y;
            this->stateMove = Spikes::MoveHidden;
        }
    }
}

void Spikes::StageLoad()
{
    sVars->stateDraw.Set(&Spikes::Draw_Normal);

    sVars->aniFrames.Load("Global/Spikes.bin", SCOPE_STAGE);
    sVars->sfxMove.Get("Global/SpikesMove.wav");

    sVars->verticalAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    sVars->horizontalAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

    sVars->unused1 = 0x100000;
    sVars->unused2 = 0x100000;

    sVars->sfxSpike.Get("Global/Spike.wav");
}

void Spikes::Draw_Normal()
{
    Vector2 drawPos;

    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    int32 cnt = this->count >> 1;
    switch (this->type) {
        case C_TOP:
        case C_BOTTOM:
            drawPos.x = 0x100000 - (this->count << 19) + this->position.x;
            for (; cnt; --cnt) {
                sVars->verticalAnimator.DrawSprite(&drawPos, false);
                drawPos.x += 0x200000;
            }

            if (this->count & 1) {
                drawPos.x -= 0x100000;
                sVars->verticalAnimator.DrawSprite(&drawPos, false);
            }
            break;

        case C_LEFT:
        case C_RIGHT:
            drawPos.y = 0x100000 - (this->count << 19) + this->position.y;
            for (; cnt; --cnt) {
                sVars->horizontalAnimator.DrawSprite(&drawPos, false);
                drawPos.y += 0x200000;
            }

            if (this->count & 1) {
                drawPos.y -= 0x100000;
                sVars->horizontalAnimator.DrawSprite(&drawPos, false);
            }
            break;

        default: break;
    }
}

void Spikes::CheckHit(Player *player, int32 playerVelX, int32 playerVelY)
{
    if (player->state.Matches(&Player::State_Hurt))
        return;
    if (!player->CheckValidState() || player->invincibleTimer || player->blinkTimer > 0)
        return;

    if (player->characterID == ID_MIGHTY
        && (player->animator.animationID == Player::ANI_JUMP || player->animator.animationID == Player::ANI_SPINDASH)) {

        if (abs(playerVelX) < 0x20000) {
            switch (this->type) {
                default: break;
                case C_TOP:
                        player->velocity.y = -0x48000;
                        if (!(player->direction & FLIP_X))
                            player->velocity.x = -0x28000;
                        else
                            player->velocity.x = 0x28000;
                        player->blinkTimer = 60;
                        player->state.Set(&Player::State_Hurt);

                    player->velocity.x -= player->velocity.x >> 2;
                    break;

                case C_LEFT:
                    player->velocity.y = -0x40000;
                    player->velocity.x = -0x28000;
                    player->state.Set(&Player::State_Air);
                    break;

                case C_RIGHT:
                    player->velocity.y = -0x40000;
                    player->velocity.x = 0x28000;
                    player->state.Set(&Player::State_Air);
                    break;

                case C_BOTTOM:
                    player->velocity.y = 0x20000;
                    player->state.Set(&Player::State_Air);
                    break;
            }

            player->onGround         = false;
            player->applyJumpCap     = false;
            player->jumpAbilityState = 0;
            if (player->state.Matches(&Player::State_Hurt)) {
                player->animator.SetAnimation(player->aniFrames, Player::ANI_HURT, false, 0);
                sVars->sfxSpike.Play();
            }

            if (player->underwater) {
                player->velocity.x >>= 1;
                player->velocity.y >>= 1;
            }
        }
        else if (this->type == C_TOP) {
                if (playerVelY > 0x28000) {
                    player->velocity.y       = -0x20000;
                    player->state.Set(&Player::State_Air);
                    player->onGround         = false;
                    player->applyJumpCap     = false;
                    player->jumpAbilityState = 0;
                }

                player->groundedStore = true;
                player->state.Set(&Player::State_Roll);
                player->nextGroundState.Set(&Player::State_Roll);
                player->nextAirState.Set(&Player::State_Air);
        }
        return; // dont do the code below
    }

    if (player->position.x > this->position.x)
        player->velocity.x = 0x20000;
    else
        player->velocity.x = -0x20000;

    player->Hit();

    if (player->deathType == Player::DeathDie_Sfx) {
        player->deathType = Player::DeathDie_NoSfx;
        sVars->sfxSpike.Play();
    }
    else if (player->state.Matches(&Player::State_Hurt) && (player->shield || player->sidekick)) {
        Player::sVars->sfxHurt.Stop();
        sVars->sfxSpike.Play();
    }
}

#if RETRO_INCLUDE_EDITOR
void Spikes::EditorDraw()
{
    if (this->count < 2)
        this->count = 2;

    int32 dir  = this->type & 1;
    int32 type = 0;
    switch ((this->type >> 1) & 1) {
        case 0:
            this->direction = FLIP_Y * dir;
            if (this->direction)
                type = C_BOTTOM;
            else
                type = C_TOP;
            break;

        case 1:
            this->direction = dir;
            if (this->direction)
                type = C_LEFT;
            else
                type = C_RIGHT;
            break;
    }

    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y;
    int32 cnt = this->count >> 1;
    switch (type) {
        case C_TOP:
        case C_BOTTOM:
            drawPos.x = 0x100000 - (this->count << 19) + this->position.x;
            for (; cnt; --cnt) {
                sVars->verticalAnimator.DrawSprite(&drawPos, false);
                drawPos.x += 0x200000;
            }

            if (this->count & 1) {
                drawPos.x -= 0x100000;
                sVars->verticalAnimator.DrawSprite(&drawPos, false);
            }
            break;

        case C_LEFT:
        case C_RIGHT:
            drawPos.y = 0x100000 - (this->count << 19) + this->position.y;
            for (; cnt; --cnt) {
                sVars->horizontalAnimator.DrawSprite(&drawPos, false);
                drawPos.y += 0x200000;
            }

            if (this->count & 1) {
                drawPos.y -= 0x100000;
                sVars->horizontalAnimator.DrawSprite(&drawPos, false);
            }
            break;

        default: break;
    }
}

void Spikes::EditorLoad()
{
    sVars->aniFrames.Load("Global/Spikes.bin", SCOPE_STAGE);

    sVars->verticalAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    sVars->horizontalAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

    RSDK_ACTIVE_VAR(sVars, planeFilter);
    RSDK_ENUM_VAR("All");
    RSDK_ENUM_VAR("AL");
    RSDK_ENUM_VAR("BL");
    RSDK_ENUM_VAR("AH");
    RSDK_ENUM_VAR("BH");
}
#endif

#if RETRO_REV0U
void Spikes::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Spikes);

    sVars->aniFrames.Init();

    sVars->sfxMove.Init();
    sVars->sfxSpike.Init();
}
#endif

void Spikes::Serialize()
{
    RSDK_EDITABLE_VAR(Spikes, VAR_INT32, type);
    RSDK_EDITABLE_VAR(Spikes, VAR_BOOL, moving);
    RSDK_EDITABLE_VAR(Spikes, VAR_UINT8, count);
    RSDK_EDITABLE_VAR(Spikes, VAR_UINT8, stagger);
    RSDK_EDITABLE_VAR(Spikes, VAR_UINT8, timer);
    RSDK_EDITABLE_VAR(Spikes, VAR_INT32, planeFilter);
}

} // namespace GameLogic