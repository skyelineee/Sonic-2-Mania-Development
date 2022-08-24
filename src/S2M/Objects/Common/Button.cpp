// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Button Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Button.hpp"

#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "ScreenWrap.hpp"

#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Button);

void Button::Update()
{
    this->down            = false;
    this->currentlyActive = false;

    if (this->interaction) {
        switch (this->type) {
            case Button::Floor: HandleFloor(); break;
            case Button::Roof: HandleRoof(); break;
            case Button::RWall: HandleRWall(); break;
            case Button::LWall: HandleLWall(); break;

            default: break;
        }
    }

    if (!this->down)
        this->wasActivated = false;

    this->baseAnimator.frameID = this->down + 1;
}
void Button::LateUpdate() {}
void Button::StaticUpdate() {}
void Button::Draw()
{
    Vector2 drawPos = this->position;

    if (this->type > 1)
        drawPos.x -= this->pressPos;
    else
        drawPos.y += this->pressPos;

    this->buttonAnimator.DrawSprite(&drawPos, false);
    this->baseAnimator.DrawSprite(nullptr, false);

    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Button::Draw), true);
}

void Button::Create(void *data)
{
    this->drawFX = FX_FLIP;
    if (!sceneInfo->inEditor) {
        switch (this->type) {
            case Button::Floor:
                this->direction = FLIP_NONE;
                this->buttonAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
                this->baseAnimator.SetAnimation(sVars->aniFrames, 0, true, 1);

                this->hitboxButton.left   = sVars->hitboxV.left;
                this->hitboxButton.top    = sVars->hitboxV.top;
                this->hitboxButton.right  = sVars->hitboxV.right;
                this->hitboxButton.bottom = sVars->hitboxV.bottom;
                break;

            case Button::Roof:
                this->direction = FLIP_Y;
                this->buttonAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
                this->baseAnimator.SetAnimation(sVars->aniFrames, 0, true, 1);

                this->hitboxButton.left   = sVars->hitboxV.left;
                this->hitboxButton.top    = sVars->hitboxV.top;
                this->hitboxButton.right  = sVars->hitboxV.right;
                this->hitboxButton.bottom = sVars->hitboxV.bottom;
                break;

            case Button::RWall:
                this->direction = FLIP_NONE;
                this->buttonAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
                this->baseAnimator.SetAnimation(sVars->aniFrames, 1, true, 1);

                this->hitboxButton.left   = sVars->hitboxH.left;
                this->hitboxButton.top    = sVars->hitboxH.top;
                this->hitboxButton.right  = sVars->hitboxH.right;
                this->hitboxButton.bottom = sVars->hitboxH.bottom;
                break;

            case Button::LWall:
                this->direction = FLIP_X;
                this->buttonAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
                this->baseAnimator.SetAnimation(sVars->aniFrames, 1, true, 1);

                this->hitboxButton.left   = sVars->hitboxH.left;
                this->hitboxButton.top    = sVars->hitboxH.top;
                this->hitboxButton.right  = sVars->hitboxH.right;
                this->hitboxButton.bottom = sVars->hitboxH.bottom;
                break;

            default: break;
        }

        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x200000;
        this->updateRange.y = 0x200000;
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];

        if (ScreenWrap::CheckCompetitionWrap())
            this->active = ACTIVE_NORMAL;
    }
}

void Button::StageLoad()
{
    sVars->activatePos  = 0x80000;
    sVars->buttonOffset = 0x50000;
    sVars->hitboxOffset = 13;
    sVars->unused1      = 5;

    RSDK_DYNAMIC_PATH_ACTID("Button");
    sVars->aniFrames.Load(dynamicPath, SCOPE_STAGE);

    sVars->hitboxH.left   = -3;
    sVars->hitboxH.top    = -16;
    sVars->hitboxH.right  = 8;
    sVars->hitboxH.bottom = 16;

    sVars->hitboxV.left   = -16;
    sVars->hitboxV.top    = -8;
    sVars->hitboxV.right  = 16;
    sVars->hitboxV.bottom = 3;

    sVars->sfxButton.Get("Stage/Button.wav");
}

void Button::HandleFloor()
{
    int32 startPressPos = this->pressPos;

    if (!ScreenWrap::CheckCompetitionWrap() || !ScreenWrap::sVars->handlingWrap) {
        this->pressPos = (this->pressPos - sVars->buttonOffset) & ~((this->pressPos - sVars->buttonOffset) >> 31);
    }
    else {
        this->pressPos = 0;
    }

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        this->hitboxButton.top    = (startPressPos >> 16) - (sVars->hitboxOffset & 0xFFFF);
        this->hitboxButton.bottom = this->hitboxButton.top + 32;

        int32 playerX                   = player->position.x;
        int32 playerY                   = player->position.y;
        int32 xVel                      = player->velocity.x;
        int32 yVel                      = player->velocity.y;
        int32 vel                       = player->groundVel;
        bool32 grounded                 = player->onGround;
        bool32 groundedStore            = player->groundedStore;
        StateMachine<Player> nextGState = player->nextGroundState;
        StateMachine<Player> nextAState = player->nextAirState;
        StateMachine<Player> state      = player->state;

        if (player->CheckCollisionBox(this, &this->hitboxButton) == C_TOP || this->walkOnto) {
            player->position.x      = playerX;
            player->position.y      = playerY;
            player->velocity.x      = xVel;
            player->velocity.y      = yVel;
            player->groundVel       = vel;
            player->onGround        = grounded;
            player->groundedStore   = groundedStore;
            player->nextGroundState = nextGState;
            player->nextAirState    = nextAState;
            player->state           = state;

            this->hitboxButton.top -= (startPressPos >> 16);
            this->hitboxButton.top += (sVars->activatePos >> 16);
            int32 newPressPos = this->pressPos;

            if (player->CheckCollisionPlatform(this, &this->hitboxButton) == C_TOP) {
                this->pressPos = sVars->activatePos;
            }
            else {
                this->position.y -= sVars->activatePos;
                if (player->CheckCollisionTouch(this, &this->hitboxButton)) {
                    Hitbox *playerHitbox = player->GetHitbox();
                    this->pressPos       = sVars->buttonOffset + (player->position.y & 0xFFFF0000) + (playerHitbox->bottom << 16) - this->position.y;
                    this->pressPos       = CLAMP(this->pressPos, 0, sVars->activatePos) & 0xFFFF0000;
                }

                this->position.y += sVars->activatePos;
            }

            if (this->pressPos == sVars->activatePos) {
                player->CheckCollisionBox(this, &this->hitboxButton);
                player->angle = 0;
                if (!this->wasActivated) {
                    if (!this->silent)
                        sVars->sfxButton.Play();
                    this->currentlyActive = true;
                    this->toggled ^= true;
                }
                this->wasActivated = true;
                this->down         = true;
                this->activated    = true;
            }

            if (newPressPos > this->pressPos)
                this->pressPos = newPressPos;
        }

        if (this->pressPos)
            startPressPos = this->pressPos;
    }

    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Button::HandleFloor), true);
}
void Button::HandleRoof()
{
    int32 startPressPos = this->pressPos;
    this->pressPos      = 0;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        this->hitboxButton.top    = -1 - (sVars->hitboxOffset & 0xFFFF) - (startPressPos >> 16);
        this->hitboxButton.bottom = this->hitboxButton.top + 32;

        int32 playerX                   = player->position.x;
        int32 playerY                   = player->position.y;
        int32 xVel                      = player->velocity.x;
        int32 yVel                      = player->velocity.y;
        int32 vel                       = player->groundVel;
        bool32 grounded                 = player->onGround;
        bool32 groundedStore            = player->groundedStore;
        StateMachine<Player> nextGState = player->nextGroundState;
        StateMachine<Player> nextAState = player->nextAirState;
        StateMachine<Player> state      = player->state;

        if (player->CheckCollisionBox(this, &this->hitboxButton) == C_BOTTOM || this->walkOnto) {
            player->position.x      = playerX;
            player->position.y      = playerY;
            player->velocity.x      = xVel;
            player->velocity.y      = yVel;
            player->groundVel       = vel;
            player->onGround        = grounded;
            player->groundedStore   = groundedStore;
            player->nextGroundState = nextGState;
            player->nextAirState    = nextAState;
            player->state           = state;

            this->hitboxButton.top += (startPressPos >> 16);
            this->hitboxButton.top += (sVars->activatePos >> 16);
            int32 newPressPos = this->pressPos;

            if (player->CheckCollisionBox(this, &this->hitboxButton) == C_BOTTOM) {
                this->pressPos = -sVars->activatePos;
            }
            else {
                this->position.y += sVars->activatePos;
                if (player->CheckCollisionTouch(this, &this->hitboxButton)) {
                    Hitbox *playerHitbox = player->GetHitbox();
                    this->pressPos       = (player->position.y & 0xFFFF0000) + (playerHitbox->top << 16) - sVars->buttonOffset - this->position.y;
                    this->pressPos       = CLAMP(this->pressPos, -sVars->activatePos, 0) & 0xFFFF0000;
                }

                this->position.y -= sVars->activatePos;
            }

            if (this->pressPos == -sVars->activatePos) {
                if (!this->wasActivated) {
                    if (!this->silent)
                        sVars->sfxButton.Play();
                    this->currentlyActive = true;
                    this->toggled ^= true;
                }

                this->wasActivated = true;
                this->down         = true;
                this->activated    = true;
            }

            if (newPressPos < this->pressPos)
                this->pressPos = newPressPos;
        }

        if (this->pressPos)
            startPressPos = this->pressPos;
    }

    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Button::HandleRoof), true);
}
void Button::HandleRWall()
{
    int32 startPressPos = this->pressPos;
    this->pressPos      = 0;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        this->hitboxButton.right = (sVars->hitboxOffset & 0xFFFF) - (startPressPos >> 16) + 1;
        this->hitboxButton.left  = this->hitboxButton.right - 16;

        int32 playerX                   = player->position.x;
        int32 playerY                   = player->position.y;
        int32 xVel                      = player->velocity.x;
        int32 yVel                      = player->velocity.y;
        int32 vel                       = player->groundVel;
        bool32 grounded                 = player->onGround;
        bool32 groundedStore            = player->groundedStore;
        StateMachine<Player> nextGState = player->nextGroundState;
        StateMachine<Player> nextAState = player->nextAirState;
        StateMachine<Player> state      = player->state;

        if (player->CheckCollisionBox(this, &this->hitboxButton) == C_RIGHT || this->walkOnto) {
            player->position.x      = playerX;
            player->position.y      = playerY;
            player->velocity.x      = xVel;
            player->velocity.y      = yVel;
            player->groundVel       = vel;
            player->onGround        = grounded;
            player->groundedStore   = groundedStore;
            player->nextGroundState = nextGState;
            player->nextAirState    = nextAState;
            player->state           = state;

            this->hitboxButton.right += (startPressPos >> 16);
            this->hitboxButton.right = this->hitboxButton.right - (sVars->activatePos >> 16) - 1;
            int32 newPressPos        = this->pressPos;

            if (player->CheckCollisionBox(this, &this->hitboxButton) == C_RIGHT) {
                this->pressPos = sVars->activatePos;
            }
            else {
                this->position.x += sVars->activatePos;
                if (player->CheckCollisionTouch(this, &this->hitboxButton)) {
                    Hitbox *playerHitbox = player->GetHitbox();
                    this->pressPos       = sVars->buttonOffset - (playerHitbox->left << 16) - (player->position.x & 0xFFFF0000) + this->position.x;
                    this->pressPos       = CLAMP(this->pressPos, 0, sVars->activatePos) & 0xFFFF0000;
                }
                this->position.x -= sVars->activatePos;
            }

            if (this->pressPos == sVars->activatePos) {
                if (!this->wasActivated) {
                    if (!this->silent)
                        sVars->sfxButton.Play();
                    this->currentlyActive = true;
                    this->toggled ^= true;
                }

                this->wasActivated = true;
                this->down         = true;
                this->activated    = true;
            }

            if (newPressPos > this->pressPos)
                this->pressPos = newPressPos;
        }

        if (this->pressPos)
            startPressPos = this->pressPos;
    }

    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Button::HandleRWall), true);
}
void Button::HandleLWall()
{
    int32 startPressPos = this->pressPos;
    this->pressPos      = 0;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        this->hitboxButton.right = (startPressPos >> 16) + (sVars->hitboxOffset & 0xFFFF);
        this->hitboxButton.left  = this->hitboxButton.right - 16;

        int32 playerX                   = player->position.x;
        int32 playerY                   = player->position.y;
        int32 xVel                      = player->velocity.x;
        int32 yVel                      = player->velocity.y;
        int32 vel                       = player->groundVel;
        bool32 grounded                 = player->onGround;
        bool32 groundedStore            = player->groundedStore;
        StateMachine<Player> nextGState = player->nextGroundState;
        StateMachine<Player> nextAState = player->nextAirState;
        StateMachine<Player> state      = player->state;

        if (player->CheckCollisionBox(this, &this->hitboxButton) == C_LEFT || this->walkOnto) {
            player->position.x      = playerX;
            player->position.y      = playerY;
            player->velocity.x      = xVel;
            player->velocity.y      = yVel;
            player->groundVel       = vel;
            player->onGround        = grounded;
            player->groundedStore   = groundedStore;
            player->nextGroundState = nextGState;
            player->nextAirState    = nextAState;
            player->state           = state;

            this->hitboxButton.right -= (startPressPos >> 16);
            this->hitboxButton.right -= (sVars->activatePos >> 16);
            int32 newPressPos = this->pressPos;

            if (player->CheckCollisionBox(this, &this->hitboxButton) == C_LEFT) {
                this->pressPos = -sVars->activatePos;
            }
            else {
                this->position.x -= sVars->activatePos;
                if (player->CheckCollisionTouch(this, &this->hitboxButton)) {
                    Hitbox *playerHitbox = player->GetHitbox();
                    this->pressPos       = this->position.x - (playerHitbox->right << 16) - (player->position.x & 0xFFFF0000) - sVars->buttonOffset;
                    this->pressPos       = CLAMP(this->pressPos, -sVars->activatePos, 0) & 0xFFFF0000;
                }

                this->position.x += sVars->activatePos;
            }

            if (this->pressPos == -sVars->activatePos) {
                if (!this->wasActivated) {
                    if (!this->silent)
                        sVars->sfxButton.Play();
                    this->currentlyActive = true;
                    this->toggled ^= true;
                }
                this->wasActivated = true;
                this->down         = true;
                this->activated    = true;
            }

            if (newPressPos < this->pressPos)
                this->pressPos = newPressPos;
        }

        if (this->pressPos)
            startPressPos = this->pressPos;
    }

    this->hitboxButton.right = (sVars->hitboxOffset & 0xFFFF) + (startPressPos >> 16);
    this->hitboxButton.left  = this->hitboxButton.left - 16;

    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Button::HandleLWall), true);
}

#if RETRO_INCLUDE_EDITOR
void Button::EditorDraw()
{
    switch (this->type) {
        case Button::Floor:
            this->direction = FLIP_NONE;
            this->buttonAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->baseAnimator.SetAnimation(sVars->aniFrames, 0, true, 1);
            break;

        case Button::Roof:
            this->direction = FLIP_Y;
            this->buttonAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->baseAnimator.SetAnimation(sVars->aniFrames, 0, true, 1);
            break;

        case Button::RWall:
            this->direction = FLIP_NONE;
            this->buttonAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
            this->baseAnimator.SetAnimation(sVars->aniFrames, 1, true, 1);
            break;

        case Button::LWall:
            this->direction = FLIP_X;
            this->buttonAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
            this->baseAnimator.SetAnimation(sVars->aniFrames, 1, true, 1);
            break;

        default: break;
    }

    if (this->tag) {
        int32 storeDir = this->direction;

        this->direction = FLIP_NONE;
        DrawHelpers::Load();
        this->direction = storeDir;
    }

    this->buttonAnimator.DrawSprite(nullptr, false);
    this->baseAnimator.DrawSprite(nullptr, false);
}

void Button::EditorLoad()
{
    RSDK_DYNAMIC_PATH_ACTID("Button");
    sVars->aniFrames.Load(dynamicPath, SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Normal");
    RSDK_ENUM_VAR("Upside-Down");
    RSDK_ENUM_VAR("Rotated Right");
    RSDK_ENUM_VAR("Rotated Left");
}
#endif

#if RETRO_REV0U
void Button::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Button);

    sVars->aniFrames.Init();

    sVars->sfxButton.Init();
}
#endif

void Button::Serialize()
{
    RSDK_EDITABLE_VAR(Button, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(Button, VAR_BOOL, walkOnto);
    RSDK_EDITABLE_VAR(Button, VAR_UINT8, tag);
    RSDK_EDITABLE_VAR(Button, VAR_BOOL, silent);
}

} // namespace GameLogic