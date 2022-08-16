// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Shield Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Shield.hpp"
#include "Zone.hpp"
// #include "Common/ScreenWrap.hpp"
#include "Debris.hpp"
// #include "Common/Water.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Shield);

void Shield::Update()
{
    this->state.Run(this);

    Player *player = this->player;
    if (player) {
        // bit 0 = flipX
        // bit 1 = flipY
        // if bit 2 is set, draw on player draw order (draw above player), else draw behind player
        // bits 3-7 aren't used, the values are stored as 0-9 in ASCII because thats how RSDK anim editor treats it
        if (this->frameFlags < 0 || !(this->frameFlags & ~3))
            this->drawGroup = player->drawGroup - 1;
        else
            this->drawGroup = player->drawGroup;
        this->visible      = this->forceVisible & player->visible;
        this->forceVisible = true;

        if (this->zdepth)
            this->visible = false;
    }
}
void Shield::LateUpdate() {}
void Shield::StaticUpdate() {}
void Shield::Draw()
{
    if (this->type != Shield::Unknown) {
        Player *player = this->player;
        if (!player || player->classID != Player::sVars->classID || (player->superState == 0 || player->superState == 3)) {
            int32 dirStore = this->direction;
            if (globals->gravityDir == CMODE_ROOF)
                this->direction = dirStore ^ 3;

            if (player) {
                if (player->isChibi) {
                    this->drawFX |= FX_SCALE;
                    this->scale.x = 0x100;
                    this->scale.y = 0x100;
                }
                else {
                    if (player->drawFX & FX_SCALE)
                        this->drawFX |= FX_SCALE;
                    else
                        this->drawFX &= ~FX_SCALE;
                    this->scale.x = player->scale.x;
                    this->scale.y = player->scale.y;
                }

                this->position = player->position;

                Hitbox *playerHitbox = player->animator.GetHitbox(0);
                if (playerHitbox) {
                    if (player->direction & FLIP_X)
                        this->position.x += (playerHitbox->left << 15) - (playerHitbox->right << 15) - (playerHitbox->left << 16);
                    else
                        this->position.x += ((playerHitbox->right + 2 * playerHitbox->left) << 15) - (playerHitbox->left << 15);

                    if ((player->direction & FLIP_Y) || player->invertGravity)
                        this->position.y += (playerHitbox->top << 15) - (playerHitbox->bottom << 15) - (playerHitbox->top << 16);
                    else
                        this->position.y += ((playerHitbox->bottom + 2 * playerHitbox->top) << 15) - (playerHitbox->top << 15);
                }
            }

            if (globals->useManiaBehavior && this->type == Shield::Bubble) {
                this->inkEffect = INK_ADD;
                this->fxAnimator.DrawSprite(nullptr, false);
                this->inkEffect = INK_BLEND;
            }

            this->shieldAnimator.DrawSprite(nullptr, false);
            this->direction = dirStore;

            // ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Shield::Draw), false);
        }
    }
}

void Shield::Create(void *data)
{
    this->active  = ACTIVE_NORMAL;
    this->visible = true;

    if (data) {
        Player *player = (Player *)data;
        if (player->classID == Player::sVars->classID)
            this->type = player->shield;
        this->player = player;
    }

    this->drawFX = FX_FLIP;
    this->state.Set(&Shield::State_Default);
    switch (this->type) {
        case Shield::Blue:
            this->shieldAnimator.SetAnimation(sVars->aniFrames, Shield::AniBlue, true, 0);
            this->inkEffect = INK_ADD;
            this->alpha     = 0x80;
            break;

        case Shield::Bubble:
            this->fxAnimator.SetAnimation(sVars->aniFrames, Shield::AniBubbleAdd, true, 0);
            this->shieldAnimator.SetAnimation(sVars->aniFrames, Shield::AniBubble, true, 0);
            this->alpha = 0x100;
            break;

        case Shield::Fire: this->shieldAnimator.SetAnimation(sVars->aniFrames, Shield::AniFire, true, 0); break;

        case Shield::Lightning: this->shieldAnimator.SetAnimation(sVars->aniFrames, Shield::AniLightning, true, 0); break;

        default: break;
    }
}

void Shield::StageLoad()
{
    sVars->aniFrames.Load("Global/Shields.bin", SCOPE_STAGE);

    sVars->sfxBlueShield.Get("Global/BlueShield.wav");
    sVars->sfxBubbleShield.Get("Global/BubbleShield.wav");
    sVars->sfxFireShield.Get("Global/FireShield.wav");
    sVars->sfxLightningShield.Get("Global/LightningShield.wav");
    sVars->sfxInstaShield.Get("Global/InstaShield.wav");
    sVars->sfxBubbleBounce.Get("Global/BubbleBounce.wav");
    sVars->sfxFireDash.Get("Global/FireDash.wav");
    sVars->sfxLightningJump.Get("Global/LightningJump.wav");
}

void Shield::SpawnLightningSparks(RSDK::GameObject::Entity *parent, RSDK::SpriteAnimation &aniFrames, uint32 listID)
{
    Debris *debris     = GameObject::Create<Debris>(Debris::Move, parent->position.x, parent->position.y);
    debris->duration   = 22;
    debris->velocity.x = -0x20000;
    debris->velocity.y = -0x20000;
    debris->drawGroup  = Zone::sVars->playerDrawGroup[1];
    debris->animator1.SetAnimation(aniFrames, listID, true, 0);
    if (parent->drawFX & FX_SCALE) {
        debris->drawFX |= FX_SCALE;
        debris->scale.x = parent->scale.x;
        debris->scale.y = parent->scale.y;
    }

    debris             = GameObject::Create<Debris>(Debris::Move, parent->position.x, parent->position.y);
    debris->duration   = 22;
    debris->velocity.x = 0x20000;
    debris->velocity.y = -0x20000;
    debris->drawGroup  = Zone::sVars->playerDrawGroup[1];
    debris->animator1.SetAnimation(aniFrames, listID, true, 0);
    if (parent->drawFX & FX_SCALE) {
        debris->drawFX |= FX_SCALE;
        debris->scale.x = parent->scale.x;
        debris->scale.y = parent->scale.y;
    }

    debris             = GameObject::Create<Debris>(Debris::Move, parent->position.x, parent->position.y);
    debris->duration   = 22;
    debris->velocity.x = -0x20000;
    debris->velocity.y = 0x20000;
    debris->drawGroup  = Zone::sVars->playerDrawGroup[1];
    debris->animator1.SetAnimation(aniFrames, listID, true, 0);
    if (parent->drawFX & FX_SCALE) {
        debris->drawFX |= FX_SCALE;
        debris->scale.x = parent->scale.x;
        debris->scale.y = parent->scale.y;
    }

    debris             = GameObject::Create<Debris>(Debris::Move, parent->position.x, parent->position.y);
    debris->duration   = 22;
    debris->velocity.x = 0x20000;
    debris->velocity.y = 0x20000;
    debris->drawGroup  = Zone::sVars->playerDrawGroup[1];
    debris->animator1.SetAnimation(aniFrames, listID, true, 0);
    if (parent->drawFX & FX_SCALE) {
        debris->drawFX |= FX_SCALE;
        debris->scale.x = parent->scale.x;
        debris->scale.y = parent->scale.y;
    }
}

void Shield::State_Default()
{
    SET_CURRENT_STATE();

    this->shieldAnimator.Process();
    this->fxAnimator.Process();

    this->frameFlags = this->shieldAnimator.GetFrameID() & 7;
    this->direction  = this->frameFlags & 3;
}
void Shield::State_Insta()
{
    SET_CURRENT_STATE();

    this->shieldAnimator.Process();
    this->frameFlags = this->shieldAnimator.GetFrameID() & 7;

    if (this->player)
        this->player->invincibleTimer = 1;

    if (this->shieldAnimator.frameID == this->shieldAnimator.frameCount - 1)
        this->Destroy();
}
void Shield::State_BubbleDrop()
{
    SET_CURRENT_STATE();

    State_Default();

    if (globals->useManiaBehavior) {
        if (this->fxAnimator.frameID == this->fxAnimator.frameCount - 1) {
            this->fxAnimator.SetAnimation(sVars->aniFrames, Shield::AniBubbleAttackUpAdd, true, 0);
            this->shieldAnimator.SetAnimation(nullptr, 0, true, 0);

            this->state.Set(&Shield::State_BubbleBounce);
        }
    }
    else {
        if (this->shieldAnimator.frameID == this->shieldAnimator.frameCount - 1) {
            this->shieldAnimator.SetAnimation(sVars->aniFrames, Shield::AniBubble, true, 0);

            this->state.Set(&Shield::State_Default);
        }
    }
}
void Shield::State_BubbleBounce()
{
    SET_CURRENT_STATE();

    State_Default();

    if (globals->useManiaBehavior) {
        if (this->fxAnimator.frameID == this->fxAnimator.frameCount - 1) {
            this->fxAnimator.SetAnimation(sVars->aniFrames, Shield::AniBubbleAdd, true, 0);
            this->shieldAnimator.SetAnimation(sVars->aniFrames, Shield::AniBubble, true, 0);

            this->state.Set(&Shield::State_Default);
        }
    }
    else {
        if (this->shieldAnimator.frameID == this->shieldAnimator.frameCount - 1) {
            this->shieldAnimator.SetAnimation(sVars->aniFrames, Shield::AniBubble, true, 0);

            this->state.Set(&Shield::State_Default);
        }
    }
}
void Shield::State_FireDash()
{
    SET_CURRENT_STATE();

    this->shieldAnimator.Process();
    this->frameFlags = this->shieldAnimator.GetFrameID() & 7;

    if (++this->timer > 24) {
        this->timer = 0;
        this->shieldAnimator.SetAnimation(sVars->aniFrames, Shield::AniFire, true, 0);

        this->state.Set(&Shield::State_Default);
    }
}

void Shield::State_LightningSparks()
{
    SET_CURRENT_STATE();

    this->shieldAnimator.Process();
    this->frameFlags = this->shieldAnimator.GetFrameID() & 7;
    this->direction  = this->frameFlags & 3;

    if (this->player) {
        this->position.x = this->player->position.x;
        this->position.y = this->player->position.y;
    }

    SpawnLightningSparks(this, sVars->aniFrames, Shield::AniLightningSpark);

    this->state.Set(&Shield::State_Default);
}

void Shield::State_LightningFlash()
{
    SET_CURRENT_STATE();

    this->timer++;
    // if (this->timer < 3) {
    //     color flashColor = paletteBank[0].GetEntry(16);
    //     for (int32 c = 0; c < 256; ++c) paletteBank[Water::sVars->waterPalette].SetEntry(c, flashColor);
    // }
    // else {
    //     for (int32 c = 0; c < 256; ++c) paletteBank[Water::sVars->waterPalette].SetEntry(c, Water::sVars->flashColorStorage[c]);
    //     Water::sVars->isLightningFlashing = false;
    //     this->Destroy();
    // }
}

#if RETRO_INCLUDE_EDITOR
void Shield::EditorDraw() { this->shieldAnimator.DrawSprite(&this->position, false); }

void Shield::EditorLoad() { sVars->aniFrames.Load("Global/Shields.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void Shield::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Shield);
    sVars->aniFrames.Init();

    sVars->sfxBlueShield.Init();
    sVars->sfxBubbleShield.Init();
    sVars->sfxFireShield.Init();
    sVars->sfxLightningShield.Init();
    sVars->sfxInstaShield.Init();
    sVars->sfxBubbleBounce.Init();
    sVars->sfxFireDash.Init();
    sVars->sfxLightningJump.Init();
}
#endif

void Shield::Serialize() {}

} // namespace GameLogic