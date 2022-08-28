// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ForceUnstick Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "ForceUnstick.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Global/ItemBox.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ForceUnstick);

void ForceUnstick::Update()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->CheckCollisionTouch(this, &this->hitbox)) {
            player->collisionMode = CMODE_FLOOR;

            if (this->breakClimb && player->state.Matches(&Player::State_KnuxWallClimb)) {
                player->animator.SetAnimation(player->aniFrames, Player::ANI_GLIDE_DROP, false, 2);
                player->state.Set(&Player::State_KnuxGlideDrop);
            }
        }
    }

    this->visible = DebugMode::sVars->debugActive;
}
void ForceUnstick::LateUpdate() {}
void ForceUnstick::StaticUpdate() {}
void ForceUnstick::Draw() { DrawSprites(); }

void ForceUnstick::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->updateRange.x = this->width << 19;
        this->updateRange.y = this->height << 19;

        this->hitbox.right  = 8 * this->width + 8;
        this->hitbox.left   = -this->hitbox.right;
        this->hitbox.bottom = 8 * this->height + 8;
        this->hitbox.top    = -this->hitbox.bottom;

        this->visible   = false;
        this->active    = ACTIVE_BOUNDS;
        this->drawGroup = Zone::sVars->objectDrawGroup[1];
    }
}

void ForceUnstick::StageLoad() { sVars->animator.SetAnimation(ItemBox::sVars->aniFrames, 2, true, 6); }

void ForceUnstick::DrawSprites()
{
    Vector2 drawPos;
    drawPos.x = this->position.x - (this->width << 19);
    drawPos.y = this->position.y - (this->height << 19);

    sVars->animator.frameID = this->breakClimb ? 9 : 6;
    for (int32 y = 0; y < this->height + 1; ++y) {
        for (int32 x = 0; x < this->width + 1; ++x) {
            sVars->animator.DrawSprite(&drawPos, false);
            drawPos.x += 0x100000;
        }
        drawPos.x += -0x100000 - (this->width << 20);
        drawPos.y += 0x100000;
    }
}

#if RETRO_INCLUDE_EDITOR
void ForceUnstick::EditorDraw() { DrawSprites(); }

void ForceUnstick::EditorLoad() { sVars->animator.SetAnimation(ItemBox::sVars->aniFrames, 2, true, 6); }
#endif

#if RETRO_REV0U
void ForceUnstick::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(ForceUnstick);

    sVars->aniFrames.Init();
}
#endif

void ForceUnstick::Serialize()
{
    RSDK_EDITABLE_VAR(ForceUnstick, VAR_UINT8, width);
    RSDK_EDITABLE_VAR(ForceUnstick, VAR_UINT8, height);
    RSDK_EDITABLE_VAR(ForceUnstick, VAR_BOOL, breakClimb);
}

} // namespace GameLogic