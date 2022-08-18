// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: InvisibleBlock Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "InvisibleBlock.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "DebugMode.hpp"
#include "Shield.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(InvisibleBlock);

void InvisibleBlock::Update()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if ((this->planeFilter <= 0 || player->collisionPlane == (((uint8)this->planeFilter - 1) & 1)) && (!this->noChibi || !player->isChibi)) {
            Player playerStore;
            memcpy(&playerStore, player, sizeof(playerStore));

            switch (player->CheckCollisionBox(this, &this->hitbox)) {
                case C_TOP:
                    if (!this->noCrush)
                        player->collisionFlagV |= 1;

                    if (!this->ignoreTop) {
                        switch (this->hurtType) {
                            default:
                            case InvisibleBlock::HurtNone: break;

                            case InvisibleBlock::HurtAll: player->Hurt(this); break;

                            case InvisibleBlock::HurtAqua:
                                if (player->shield != Shield::Bubble)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtFlame:
                                if (player->shield != Shield::Fire)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtThunder:
                                if (player->shield != Shield::Lightning)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtKill: player->deathType = Player::DeathDie_Sfx; break;
                        }
                    }
                    else if (!this->noPush) {
                        memcpy(player, &playerStore, sizeof(playerStore));
                    }
                    break;

                case C_LEFT:
                    if (!this->noCrush)
                        player->collisionFlagH |= 1;

                    if (this->noPush) {
                        int32 anim = player->animator.animationID;
                        if (anim == Player::ANI_PUSH || (anim >= Player::ANI_WALK && anim <= Player::ANI_DASH))
                            player->animator.SetAnimation(player->aniFrames, Player::ANI_IDLE, false, 0);
                    }

                    if (!this->ignoreLeft) {
                        switch (this->hurtType) {
                            default:
                            case InvisibleBlock::HurtNone: break;

                            case InvisibleBlock::HurtAll: player->Hurt(this); break;

                            case InvisibleBlock::HurtAqua:
                                if (player->shield != Shield::Bubble)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtFlame:
                                if (player->shield != Shield::Fire)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtThunder:
                                if (player->shield != Shield::Lightning)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtKill: player->deathType = Player::DeathDie_Sfx; break;
                        }
                    }
                    break;

                case C_RIGHT:
                    if (!this->noCrush)
                        player->collisionFlagH |= 2;

                    if (this->noPush) {
                        int32 anim = player->animator.animationID;
                        if (anim == Player::ANI_PUSH || (anim >= Player::ANI_WALK && anim <= Player::ANI_DASH))
                            player->animator.SetAnimation(player->aniFrames, Player::ANI_IDLE, false, 0);
                    }

                    if (!this->ignoreRight) {
                        switch (this->hurtType) {
                            default:
                            case InvisibleBlock::HurtNone: break;

                            case InvisibleBlock::HurtAll: player->Hurt(this); break;

                            case InvisibleBlock::HurtAqua:
                                if (player->shield != Shield::Bubble)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtFlame:
                                if (player->shield != Shield::Fire)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtThunder:
                                if (player->shield != Shield::Lightning)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtKill: player->deathType = Player::DeathDie_Sfx; break;
                        }
                    }
                    break;

                case C_BOTTOM:
                    if (!this->noCrush)
                        player->collisionFlagV |= 2;

                    if (!this->ignoreBottom) {
                        switch (this->hurtType) {
                            default:
                            case InvisibleBlock::HurtNone: break;

                            case InvisibleBlock::HurtAll: player->Hurt(this); break;

                            case InvisibleBlock::HurtAqua:
                                if (player->shield != Shield::Bubble)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtFlame:
                                if (player->shield != Shield::Fire)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtThunder:
                                if (player->shield != Shield::Lightning)
                                    player->Hurt(this);
                                break;

                            case InvisibleBlock::HurtKill: player->deathType = Player::DeathDie_Sfx; break;
                        }
                    }
                    break;

                default: break;
            }
        }
    }

    this->visible = DebugMode::sVars->debugActive;
}
void InvisibleBlock::LateUpdate() {}
void InvisibleBlock::StaticUpdate() {}
void InvisibleBlock::Draw() { DrawSprites(); }

void InvisibleBlock::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        if (this->timeAttackOnly && globals->gameMode < MODE_TIMEATTACK)
            this->Destroy();

        this->visible = false;
        this->active  = this->activeNormal ? ACTIVE_NORMAL : ACTIVE_BOUNDS;

        this->updateRange.x = (this->width + 5) << 19;
        this->updateRange.y = (this->height + 5) << 19;

        this->hitbox.right  = 8 * this->width + 8;
        this->hitbox.left   = -this->hitbox.right;
        this->hitbox.bottom = 8 * this->height + 8;
        this->hitbox.top    = -this->hitbox.bottom;
        this->drawGroup     = Zone::sVars->objectDrawGroup[1];
    }
    else {
        this->width  = 1;
        this->height = 1;
    }
}

void InvisibleBlock::StageLoad()
{
    sVars->aniFrames.Load("Global/ItemBox.bin", SCOPE_STAGE);

    sVars->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
    sVars->animator.frameID = 10;
}

void InvisibleBlock::DrawSprites()
{
    Vector2 drawPos;
    drawPos.x = this->position.x - (this->width << 19);
    drawPos.y = this->position.y - (this->height << 19);

    for (int32 y = 0; y <= this->height; ++y) {
        for (int32 x = 0; x <= this->width; ++x) {
            sVars->animator.DrawSprite(&drawPos, false);
            drawPos.x += 0x100000;
        }

        drawPos.x += -0x100000 - (this->width << 20);
        drawPos.y += 0x100000;
    }
}

#if RETRO_INCLUDE_EDITOR
void InvisibleBlock::EditorDraw() { DrawSprites(); }

void InvisibleBlock::EditorLoad()
{
    sVars->aniFrames.Load("Global/ItemBox.bin", SCOPE_STAGE);

    sVars->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
    sVars->animator.frameID = 10;

    RSDK_ACTIVE_VAR(sVars, planeFilter);
    RSDK_ENUM_VAR("All");
    RSDK_ENUM_VAR("AL");
    RSDK_ENUM_VAR("BL");
    RSDK_ENUM_VAR("AH");
    RSDK_ENUM_VAR("BH");

    RSDK_ACTIVE_VAR(sVars, hurtType);
    RSDK_ENUM_VAR("None");
    RSDK_ENUM_VAR("All");
    RSDK_ENUM_VAR("Aqua");
    RSDK_ENUM_VAR("Flame");
    RSDK_ENUM_VAR("Thunder");
    RSDK_ENUM_VAR("Kill");
}
#endif

#if RETRO_REV0U
void InvisibleBlock::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(InvisibleBlock);

    sVars->aniFrames.Init();
}
#endif

void InvisibleBlock::Serialize()
{
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_UINT8, width);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_UINT8, height);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_INT32, planeFilter);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_BOOL, noCrush);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_BOOL, noPush);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_BOOL, activeNormal);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_BOOL, timeAttackOnly);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_BOOL, noChibi);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_UINT8, hurtType);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_BOOL, ignoreTop);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_BOOL, ignoreBottom);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_BOOL, ignoreLeft);
    RSDK_EDITABLE_VAR(InvisibleBlock, VAR_BOOL, ignoreRight);
}

} // namespace GameLogic