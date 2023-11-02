// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Plunger Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Plunger.hpp"
#include "Global/Player.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Plunger);

void Plunger::Update()
{
    if (this->type == Diagonal) {
        if (this->plungerActive == true) {
            if (this->compression < 0x100000) {
                this->compression += 0x4000;
            }
        }
        else {
            if (this->compression > 0) {
                this->compression -= 0x20000;
                if (this->compression < 0) {
                    this->compression = 0;
                }
            }
            else {
                this->active = ACTIVE_BOUNDS;
            }
        }

        bool32 isPlungerActive = this->plungerActive;
        this->plungerActive    = false;
        int32 fadingPosX       = this->position.x;
        int32 fadingPosY       = this->position.y;
        this->position.x -= this->compression;
        this->position.y += this->compression;
        for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (!currentPlayer->state.Matches(&Player::State_Death) && !currentPlayer->state.Matches(&Player::State_Hurt)) {
                if (currentPlayer->CheckCollisionTouch(this, &sVars->diagonalHitbox)) {
                    currentPlayer->state.Set(&Player::State_Static);
                    currentPlayer->animator.SetAnimation(currentPlayer->aniFrames, Player::ANI_JUMP, false, 0);
                    currentPlayer->position.x = this->position.x + 0x1B0000;
                    currentPlayer->position.y = this->position.y - 0x1B0000;
                    currentPlayer->velocity.x = -1;
                    currentPlayer->velocity.y = 1;
                    if (currentPlayer->jumpHold) {
                        this->plungerActive = true;
                    }
                }
            }
        }

        if (this->compression > 0x60000) {
            if (isPlungerActive) {
                if (this->plungerActive == false) {
                    this->active = ACTIVE_NORMAL;
                    for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                        if (!currentPlayer->state.Matches(&Player::State_Death) && !currentPlayer->state.Matches(&Player::State_Hurt)) {
                            if (currentPlayer->state.Matches(&Player::State_Static)) {
                                if (currentPlayer->CheckCollisionTouch(this, &sVars->diagonalHitbox)) {
                                    if (this->releaseType == Lock) {
                                        currentPlayer->state.Set(&Player::State_TubeRoll);
                                    }
                                    else {
                                        currentPlayer->state.Set(&Player::State_Air);
                                    }

                                    currentPlayer->timer      = 0;
                                    currentPlayer->direction  = FLIP_NONE;
                                    currentPlayer->velocity.x = this->compression;
                                    currentPlayer->groundVel  = this->compression;
                                    currentPlayer->velocity.y = this->compression;
                                    currentPlayer->velocity.y = -currentPlayer->velocity.y;
                                    sVars->sfxPlunger.Play(false, 255);
                                }
                            }
                        }
                    }
                }
            }
        }
        this->position.x = fadingPosX;
        this->position.y = fadingPosY;

        // Fading for yellow plunger
        fadingPosX = this->compression >> 14;
        this->angle += fadingPosX;
        fadingPosY = Math::Sin256(this->angle) >> 4;

        fadingPosX  = ((this->compression >> 12) - 64) + fadingPosY;
        this->alpha = fadingPosX;
    }
    else {
        // BUG: (only sometimes) something here is causing the player to stick to the vertical plunger even after letting go of the jump button, this
        // does not happen for the diagonal type the code is pretty much exactly like it was in v4, so i dont really know whats going wrong

        // FIX DETAILS:
        // the first loop checks if the player is on the plunger via *box*, setting onGround
        // this isn't erased anywhere else so the player keeps staying on ground in an air state so all momentum gets cancelled in a frame
        // easiest fix is to set onground to false manually
        if (this->plungerActive == true) {
            if (this->compression < 0x210000) {
                this->compression += 0x4000;
            }
        }
        else {
            if (this->compression > 0) {
                this->compression -= 0x20000;
                if (this->compression < 0) {
                    this->compression = 0;
                }
            }
            else {
                this->active = ACTIVE_BOUNDS;
            }
        }

        int32 isPlungerActive = this->plungerActive;
        this->plungerActive   = false;
        for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (!currentPlayer->state.Matches(&Player::State_Death) && !currentPlayer->state.Matches(&Player::State_Hurt)) {
                int32 hitboxTop                 = (this->compression >> 16) - 28;
                sVars->dynamicVertHitbox.top    = hitboxTop;
                sVars->dynamicVertHitbox.bottom = 28;
                bool32 collisionCheck           = currentPlayer->CheckCollisionBox(this, &sVars->dynamicVertHitbox);

                int32 hitboxBottom = hitboxTop;
                hitboxTop -= 2;
                sVars->dynamicVertHitbox.top    = hitboxTop;
                sVars->dynamicVertHitbox.bottom = hitboxBottom;
                collisionCheck                  = currentPlayer->CheckCollisionTouch(this, &sVars->dynamicVertHitbox);
                if (collisionCheck) {
                    currentPlayer->state.Set(&Player::State_Static);
                    currentPlayer->stateGravity.Set(&Player::Gravity_False);
                    currentPlayer->animator.SetAnimation(currentPlayer->aniFrames, Player::ANI_JUMP, false, 0);
                    currentPlayer->position.x = this->position.x;
                    currentPlayer->position.y = currentPlayer->GetHitbox()->bottom;
                    currentPlayer->position.y = (-currentPlayer->position.y << 16) - 0x1C0000;
                    currentPlayer->position.y += (this->compression & 0xFFFF0000) + this->position.y;
                    currentPlayer->velocity.x = 0;
                    currentPlayer->velocity.y = 1;
                    if (currentPlayer->jumpHold) {
                        this->plungerActive = true;
                    }
                }
            }
        }

        if (this->compression > 0x60000) {
            if (isPlungerActive) {
                if (this->plungerActive == false) {
                    this->active = ACTIVE_NORMAL;
                    for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                        if (!currentPlayer->state.Matches(&Player::State_Death) && !currentPlayer->state.Matches(&Player::State_Hurt)) {
                            if (currentPlayer->state.Matches(&Player::State_Static)) {
                                if (currentPlayer->CheckCollisionTouch(this, &sVars->verticalHitbox)) {
                                    currentPlayer->state.Set(&Player::State_Air);
                                    currentPlayer->stateGravity.Set(&Player::Gravity_True);
                                    currentPlayer->timer      = 0;
                                    currentPlayer->groundVel  = 0;
                                    currentPlayer->velocity.x = 0;
                                    currentPlayer->onGround   = false;
                                    currentPlayer->velocity.y = -((this->compression * 3) >> 2);
                                    sVars->sfxPlunger.Play(false, 255);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Fading for yellow plunger
        int32 yellowAlpha = this->compression >> 15;
        this->angle += yellowAlpha;
        int32 fadingPos = Math::Sin256(this->angle) >> 4;

        yellowAlpha = (this->compression >> 13) - 64;
        yellowAlpha += fadingPos;
        this->alpha = yellowAlpha;
    }
}

void Plunger::LateUpdate() {}
void Plunger::StaticUpdate() {}
void Plunger::Draw()
{
    if (this->type == Diagonal) {
        // Spring & Base
        this->inkEffect   = INK_NONE;
        int32 springFrame = this->compression >> 20;
        this->animator.SetAnimation(sVars->aniFrames, 0, true, springFrame);
        this->animator.DrawSprite(nullptr, false);

        // Plunger (Red)
        int32 springPosX = this->position.x - this->compression;
        int32 springPosY = this->position.y + this->compression;
        Vector2 drawPos;
        drawPos.x = springPosX;
        drawPos.y = springPosY;
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 3);
        this->animator.DrawSprite(&drawPos, false);

        // Plunger (Yellow)
        this->inkEffect = INK_ALPHA;
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 4);
        this->animator.DrawSprite(&drawPos, false);
    }
    else {
        // Spring & Base
        this->inkEffect   = INK_NONE;
        int32 springFrame = this->compression >> 20;
        this->animator.SetAnimation(sVars->aniFrames, 1, true, springFrame);
        this->animator.DrawSprite(nullptr, false);

        // Plunger (Red)
        int32 springPosY = (-0x1C0000 + this->compression) + this->position.y;
        Vector2 drawPos;
        drawPos.x = this->position.x;
        drawPos.y = springPosY;
        this->animator.SetAnimation(sVars->aniFrames, 1, true, 3);
        this->animator.DrawSprite(&drawPos, false);

        // Plunger (Yellow)
        this->inkEffect = INK_ALPHA;
        this->animator.SetAnimation(sVars->aniFrames, 1, true, 4);
        this->animator.DrawSprite(&drawPos, false);
    }
}

void Plunger::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->visible       = true;
        this->drawGroup     = 3;
        this->inkEffect     = INK_NONE;
    }
}

void Plunger::StageLoad()
{
    sVars->aniFrames.Load("CNZ/Plunger.bin", SCOPE_STAGE);
    sVars->sfxPlunger.Get("Stage/Plunger.wav");

    sVars->diagonalHitbox.left   = 0;
    sVars->diagonalHitbox.top    = -19;
    sVars->diagonalHitbox.right  = 19;
    sVars->diagonalHitbox.bottom = 0;

    sVars->verticalHitbox.left   = -11;
    sVars->verticalHitbox.top    = -28;
    sVars->verticalHitbox.right  = 11;
    sVars->verticalHitbox.bottom = 28;

    sVars->dynamicVertHitbox.left  = -11;
    sVars->dynamicVertHitbox.right = 11;
}

#if RETRO_INCLUDE_EDITOR
void Plunger::EditorDraw() {}

void Plunger::EditorLoad()
{
    sVars->aniFrames.Load("CNZ/Plunger.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, releaseType);
    RSDK_ENUM_VAR("Lock Controls");
    RSDK_ENUM_VAR("Release Controls");

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Diagonal");
    RSDK_ENUM_VAR("Vertical");
}
#endif

void Plunger::Serialize()
{
    RSDK_EDITABLE_VAR(Plunger, VAR_INT32, releaseType);
    RSDK_EDITABLE_VAR(Plunger, VAR_INT32, type);
}

} // namespace GameLogic