// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Dust Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Dust.hpp"
#include "Player.hpp"
#include "Zone.hpp"
// #include "Common/ScreenWrap.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Dust);

void Dust::Update() {  }
void Dust::LateUpdate() { this->state.Run(this); }
void Dust::StaticUpdate() {}
void Dust::Draw()
{
    this->animator.DrawSprite(&this->position, false);
    // ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Dust::Draw), true);
}

void Dust::Create(void *data)
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawFX    = FX_ROTATE | FX_FLIP;
        this->drawGroup = Zone::sVars->objectDrawGroup[1];
        this->parent    = (GameObject::Entity *)data;
    }
}

void Dust::StageLoad() { sVars->aniFrames.Load("Global/Dust.bin", SCOPE_STAGE); }

void Dust::State_SpinDash()
{
    SET_CURRENT_STATE();

    Player *player = (Player *)this->parent;
    if (!player) {
        this->Destroy();
    }
    else {
        Hitbox *playerHitbox = player->GetHitbox();

        this->position.x = player->position.x;
        this->position.y = player->position.y;

        int32 bottom = playerHitbox->bottom << 16;
        if (player->invertGravity) {
            this->position.y -= bottom;
        }
        else {
            switch (player->collisionMode) {
                case CMODE_FLOOR: this->position.y += bottom; break;
                case CMODE_LWALL: this->position.x -= bottom; break;
                case CMODE_ROOF: this->position.y -= bottom; break;
                case CMODE_RWALL: this->position.x += bottom; break;
                default: break;
            }
        }

        this->direction = player->direction;
        this->drawGroup = player->drawGroup;
        this->rotation  = player->rotation;

        this->animator.Process();

        if (!player->state.Matches(&Player::State_Spindash))
            this->Destroy();
    }
}
void Dust::State_DustTrail()
{
    SET_CURRENT_STATE();

    Player *player = (Player *)this->parent;
    if (!player) {
        this->Destroy();
    }
    else {
        this->visible = false;
        if (!this->timer) {
            if (player->onGround) {
                Hitbox *playerHitbox = player->GetHitbox();

                Dust *dust = GameObject::Create<Dust>(this, player->position.x, player->position.y);
                dust->state.Set(&Dust::State_DustPuff);

                switch (player->collisionMode) {
                    case CMODE_FLOOR: dust->position.y += playerHitbox->bottom << 16; break;
                    case CMODE_LWALL: dust->position.x += playerHitbox->bottom << 16; break;
                    case CMODE_ROOF: dust->position.y -= playerHitbox->bottom << 16; break;
                    case CMODE_RWALL: dust->position.x -= playerHitbox->bottom << 16; break;
                    default: break;
                }

                dust->drawGroup = player->drawGroup;
            }
        }
        else {
            this->timer = 0;
        }

        this->timer = (this->timer + 1) & 3;
        if (player->animator.animationID != Player::ANI_SKID)
            this->Destroy();
    }
}
void Dust::State_DustPuff()
{
    SET_CURRENT_STATE();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->animator.Process();
    if (this->animator.frameID == this->animator.frameCount - 1)
        this->Destroy();
}
void Dust::State_DustPuff_Collide()
{
    SET_CURRENT_STATE();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->TileGrip(this->collisionLayers, this->collisionMode, this->collisionPlane, 0, 0, 8);

    this->animator.Process();
    if (this->animator.frameID == this->animator.frameCount - 1)
        this->Destroy();
}
void Dust::State_GlideTrail()
{
    SET_CURRENT_STATE();

    Player *player = (Player *)this->parent;
    if (!player) {
        this->Destroy();
    }
    else {
        this->visible = false;
        if (!this->timer) {
            if (player->onGround) {
                Hitbox *playerHitbox = player->GetHitbox();

                Dust *dust = GameObject::Create<Dust>(this, player->position.x, player->position.y);
                dust->state.Set(&Dust::State_DustPuff);

                switch (player->collisionMode) {
                    case CMODE_FLOOR: dust->position.y = dust->position.y + 0x40000 + (playerHitbox->bottom << 16); break;
                    case CMODE_LWALL: dust->position.x = dust->position.x + 0x40000 - (playerHitbox->bottom << 16); break;
                    case CMODE_ROOF: dust->position.y = dust->position.y + 0x40000 - (playerHitbox->bottom << 16); break;
                    case CMODE_RWALL: dust->position.x = dust->position.x - 0x40000 + (playerHitbox->bottom << 16); break;
                    default: break;
                }

                dust->drawGroup = player->drawGroup;
            }
        }
        else {
            this->timer = 0;
        }

        this->timer = (this->timer + 1) & 7;
        if (player->animator.animationID != Player::ANI_GLIDE_SLIDE || !player->groundVel)
            this->Destroy();
    }
}
void Dust::State_Static()
{
    SET_CURRENT_STATE();

    if (this->parent) {
        this->position.x = this->parent->position.x;
        this->direction  = this->parent->direction;

        this->animator.Process();
    }
    else {
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void Dust::EditorDraw() { this->animator.DrawSprite(&this->position, false); }

void Dust::EditorLoad()
{
    sVars->aniFrames.Load("Global/Dust.bin", SCOPE_STAGE);
}
#endif

#if RETRO_REV0U
void Dust::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Dust);

    sVars->aniFrames.Init();
}
#endif

void Dust::Serialize() {}

} // namespace GameLogic