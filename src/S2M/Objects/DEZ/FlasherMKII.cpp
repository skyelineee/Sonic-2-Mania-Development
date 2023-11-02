// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: FlasherMKII Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "FlasherMKII.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(FlasherMKII);

void FlasherMKII::Update()
{
    this->animator.Process();

    this->state.Run(this);
}

void FlasherMKII::LateUpdate() {}

void FlasherMKII::StaticUpdate() {}

void FlasherMKII::Draw()
{
    this->animator.DrawSprite(nullptr, false);
}

void FlasherMKII::Create(void *data)
{
    this->drawFX |= FX_FLIP;
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->startPos      = this->position;

        switch (this->orientation) {
            case FLASHERMKII_ORIENTATION_UP:
                this->type      = 0;
                this->direction = FLIP_NONE;
                break;

            case FLASHERMKII_ORIENTATION_DOWN:
                this->type      = 0;
                this->direction = FLIP_Y;
                break;

            case FLASHERMKII_ORIENTATION_RIGHT:
                this->direction = FLIP_NONE;
                this->type      = 4;
                break;

            case FLASHERMKII_ORIENTATION_LEFT:
                this->direction = FLIP_X;
                this->type      = 4;
                break;

            default: break;
        }

        this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);
        this->state.Set(&FlasherMKII::State_Idle);
    }
}

void FlasherMKII::StageLoad()
{
    sVars->aniFrames.Load("DEZ/FlasherMKII.bin", SCOPE_STAGE);

    DebugMode::AddObject(sVars->classID, &FlasherMKII::DebugSpawn, &FlasherMKII::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);

    sVars->sfxFlop.Get("DEZ/FlasherFlop.wav");
    sVars->sfxZap.Get("DEZ/FlasherZap.wav");
}

void FlasherMKII::DebugSpawn()
{
    GameObject::Create<FlasherMKII>(nullptr, this->position.x, this->position.y);
}

void FlasherMKII::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void FlasherMKII::CheckOffScreen()
{
    if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange))
        FlasherMKII::Create(nullptr);
}

void FlasherMKII::HandlePlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        Hitbox *hitbox = this->animator.GetHitbox(0);

        if (player->CheckBadnikTouch(this, hitbox)) {
            if ((this->animator.animationID & 3) == 3) {
                if (player->shield == Player::Shield_Lightning || player->invincibleTimer || player->blinkTimer) {
                    if (player->CheckBadnikBreak(this, true))
                        sVars->sfxZap.Stop();
                }
                else {
                    player->Hurt(this);
                }
            }
            else {
                player->CheckBadnikBreak(this, true);
            }
        }
    }
}

void FlasherMKII::HandleHarmPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->shield == Player::Shield_Lightning || player->invincibleTimer || player->blinkTimer) {
            Hitbox *hitbox = this->animator.GetHitbox(0);
            if (player->CheckBadnikTouch(this, hitbox) && player->CheckBadnikBreak(this, true))
                sVars->sfxZap.Stop();
        }
        else {
            Hitbox *hitbox = this->animator.GetHitbox(1);
            if (player->CheckCollisionTouch(this, hitbox))
                player->Hurt(this);
        }
    }
}

void FlasherMKII::State_Idle()
{
    this->active = ACTIVE_NORMAL;
    if (this->timer > 0)
        this->timer--;

    Player *player = Player::GetNearestPlayerXY();
    if (player) {
        int32 rx = (this->position.x - player->position.x) >> 16;
        int32 ry = (this->position.y - player->position.y) >> 16;

        if (!this->timer && rx * rx + ry * ry < 0x1000) {
            this->animator.SetAnimation(sVars->aniFrames, this->type + 2, false, 0);
            sVars->sfxZap.Play(false, 255);
            this->state.Set(&FlasherMKII::State_WeakFlash);
        }
        else {
            switch (this->orientation) {
                case FLASHERMKII_ORIENTATION_UP:
                    if (player->position.x >= this->position.x) {
                        if ((uint32)(rx + 0xFF) <= 0xBE && this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0x600000, 0xC0000, 2)) {
                            this->animator.SetAnimation(sVars->aniFrames, this->type + 1,false, 0);
                            this->direction  = FLIP_X;
                            this->velocity.x = 0x220000;
                            this->state.Set(&FlasherMKII::State_Moving);
                            break;
                        }
                    }
                    else if ((uint32)(rx - 65) <= 0xBE && this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, -0x600000, 0xC0000, 2)) {
                        this->animator.SetAnimation(sVars->aniFrames, this->type + 1, false, 0);
                        this->direction  = FLIP_NONE;
                        this->velocity.x = -0x220000;
                        this->state.Set(&FlasherMKII::State_Moving);
                        break;
                    }
                    break;

                case FLASHERMKII_ORIENTATION_DOWN:
                    if (player->position.x >= this->position.x) {
                        if ((uint32)(rx + 0xFF) <= 0xBE && this->TileGrip(Zone::sVars->collisionLayers, CMODE_ROOF, 0, 0x600000, -0xC0000, 2)) {
                            this->animator.SetAnimation(sVars->aniFrames, this->type + 1, false, 0);
                            this->velocity.x = 0x220000;
                            this->direction  = FLIP_XY;
                            this->state.Set(&FlasherMKII::State_Moving);
                            break;
                        }
                    }
                    else if ((uint32)(rx - 65) <= 0xBE && this->TileGrip(Zone::sVars->collisionLayers, CMODE_ROOF, 0, -0x600000, -0xC0000, 2)) {
                        this->animator.SetAnimation(sVars->aniFrames, this->type + 1, false, 0);
                        this->direction  = FLIP_Y;
                        this->velocity.x = -0x220000;
                        this->state.Set(&FlasherMKII::State_Moving);
                        break;
                    }
                    break;

                case FLASHERMKII_ORIENTATION_RIGHT:
                    if (player->position.y >= this->position.y) {
                        if ((uint32)(ry + 0xFF) <= 0xBE && this->TileGrip(Zone::sVars->collisionLayers, CMODE_RWALL, 0, -0xC0000, 0x600000, 2)) {
                            this->animator.SetAnimation(sVars->aniFrames, this->type + 1, false, 0);
                            this->direction  = FLIP_Y;
                            this->velocity.y = 0x220000;
                            this->state.Set(&FlasherMKII::State_Moving);
                            break;
                        }
                    }
                    else if ((uint32)(ry - 65) <= 0xBE && this->TileGrip(Zone::sVars->collisionLayers, CMODE_RWALL, 0, -0xC0000, -0x600000, 2)) {
                        this->animator.SetAnimation(sVars->aniFrames, this->type + 1, false, 0);
                        this->direction  = FLIP_NONE;
                        this->velocity.y = -0x220000;
                        this->state.Set(&FlasherMKII::State_Moving);
                        break;
                    }
                    break;

                case FLASHERMKII_ORIENTATION_LEFT:
                    if (player->position.y >= this->position.y) {
                        if ((uint32)(ry + 0xFF) <= 0xBE && this->TileGrip(Zone::sVars->collisionLayers, CMODE_LWALL, 0, 0xC0000, 0x600000, 2)) {
                            this->animator.SetAnimation(sVars->aniFrames, this->type + 1, false, 0);
                            this->velocity.y = 0x220000;
                            this->direction  = FLIP_XY;
                            this->state.Set(&FlasherMKII::State_Moving);
                            break;
                        }
                    }
                    else if ((uint32)(ry - 65) <= 0xBE && this->TileGrip(Zone::sVars->collisionLayers, CMODE_LWALL, 0, 0xC0000, -0x600000, 2)) {
                        this->animator.SetAnimation(sVars->aniFrames, this->type + 1, false, 0);
                        this->direction  = FLIP_X;
                        this->velocity.y = -0x220000;
                        this->state.Set(&FlasherMKII::State_Moving);
                        break;
                    }
                    break;

                default: break;
            }
        }
    }

    FlasherMKII::HandlePlayerCollisions();
    FlasherMKII::CheckOffScreen();
}

void FlasherMKII::State_Moving()
{
    if ((this->animator.frameID == 5 || this->animator.frameID == 12) && this->animator.timer == 1) {
        this->position.x += this->velocity.x;
        this->position.y += this->velocity.y;

        if (this->onScreen == 1)
            sVars->sfxFlop.Play(false, 255);
    }

    if (this->animator.frameID == this->animator.frameCount - 1) {
        this->animator.SetAnimation(sVars->aniFrames, this->type, false, 0);
        this->state.Set(&FlasherMKII::State_Idle);
    }

    FlasherMKII::HandlePlayerCollisions();
    FlasherMKII::CheckOffScreen();
}

void FlasherMKII::State_WeakFlash()
{
    if (++this->timer == 60) {
        this->timer = 0;
        this->animator.SetAnimation(sVars->aniFrames, this->type + 3, false, 0);
        this->state.Set(&FlasherMKII::State_StrongFlash);
    }

    FlasherMKII::HandlePlayerCollisions();
    FlasherMKII::CheckOffScreen();
}

void FlasherMKII::State_StrongFlash()
{
    if (++this->timer == 120) {
        this->timer = 0;
        this->animator.SetAnimation(sVars->aniFrames, this->type + 2, false, 0);
        this->state.Set(&FlasherMKII::State_FinishedFlashing);
    }

    FlasherMKII::HandleHarmPlayerCollisions();
    FlasherMKII::CheckOffScreen();
}

void FlasherMKII::State_FinishedFlashing()
{
    if (++this->timer == 30) {
        this->animator.SetAnimation(sVars->aniFrames, this->type, false, 0);
        this->state.Set(&FlasherMKII::State_Idle);
    }

    FlasherMKII::HandlePlayerCollisions();
    FlasherMKII::CheckOffScreen();
}

#if RETRO_INCLUDE_EDITOR
void FlasherMKII::EditorDraw()
{
    this->startPos = this->position;

    switch (this->orientation) {
        case FLASHERMKII_ORIENTATION_UP:
            this->type      = 0;
            this->direction = FLIP_NONE;
            break;

        case FLASHERMKII_ORIENTATION_DOWN:
            this->type      = 0;
            this->direction = FLIP_Y;
            break;

        case FLASHERMKII_ORIENTATION_RIGHT:
            this->direction = FLIP_NONE;
            this->type      = 4;
            break;

        case FLASHERMKII_ORIENTATION_LEFT:
            this->direction = FLIP_X;
            this->type      = 4;
            break;

        default: break;
    }

    this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);

    FlasherMKII::Draw();
}

void FlasherMKII::EditorLoad()
{
    sVars->aniFrames.Load("DEZ/FlasherMKII.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, orientation);
    RSDK_ENUM_VAR("Up", FLASHERMKII_ORIENTATION_UP);
    RSDK_ENUM_VAR("Down", FLASHERMKII_ORIENTATION_DOWN);
    RSDK_ENUM_VAR("Right", FLASHERMKII_ORIENTATION_RIGHT);
    RSDK_ENUM_VAR("Left", FLASHERMKII_ORIENTATION_LEFT);
}
#endif

void FlasherMKII::Serialize() { RSDK_EDITABLE_VAR(FlasherMKII, VAR_UINT8, orientation); }


} // namespace GameLogic