// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: FlingRamp Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "FlingRamp.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(FlingRamp);

void FlingRamp::Update()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->onGround) {
            bool32 left  = this->direction == FLIP_NONE || this->direction == FLIP_X;
            bool32 right = this->direction == FLIP_NONE || this->direction == FLIP_Y;

            if (left && !(player->direction & FLIP_X) && player->velocity.x >= 0x40000) {
                if (player->CheckCollisionTouch(this, &sVars->hitboxRamp)) {
                    player->velocity.x += 0x40000;
                    player->velocity.y = -0x70000;
                    player->onGround   = false;
                    player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, true, 0);
                }
            }
            else if (right && (player->direction & FLIP_X) && player->velocity.x <= -0x40000) {
                if (player->CheckCollisionTouch(this, &sVars->hitboxRamp)) {
                    player->velocity.x -= 0x40000;
                    player->velocity.y = -0x70000;
                    player->onGround   = false;
                    player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, true, 0);
                }
            }
        }
    }

    this->visible = DebugMode::sVars->debugActive;
}
void FlingRamp::LateUpdate() {}
void FlingRamp::StaticUpdate() {}
void FlingRamp::Draw()
{
    int32 startDir = this->direction;
    switch (this->direction) {
        default:
        case 0: this->animator.SetAnimation(sVars->aniFrames, 0, true, 6); break;

        case 1:
            this->direction = FLIP_NONE;
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 12);
            break;

        case 2:
            this->direction = FLIP_X;
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 12);
            break;
    }

    this->drawFX = FX_FLIP;
    this->animator.DrawSprite(&this->position, false);

    this->direction = startDir;
}

void FlingRamp::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active  = ACTIVE_BOUNDS;
        this->visible = false;
        this->visible = Zone::sVars->hudDrawGroup;
    }
}

void FlingRamp::StageLoad()
{
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);

    sVars->hitboxRamp.left   = -16;
    sVars->hitboxRamp.top    = -16;
    sVars->hitboxRamp.right  = 16;
    sVars->hitboxRamp.bottom = 16;
}

#if RETRO_INCLUDE_EDITOR
void FlingRamp::EditorDraw()
{
    int32 startDir = this->direction;
    switch (this->direction) {
        default:
        case 0: this->animator.SetAnimation(sVars->aniFrames, 0, true, 6); break;

        case 1:
            this->direction = FLIP_NONE;
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 12);
            break;

        case 2:
            this->direction = FLIP_X;
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 12);
            break;
    }

    this->drawFX = FX_FLIP;
    this->animator.DrawSprite(&this->position, false);

    this->direction = startDir;
}

void FlingRamp::EditorLoad()
{
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Omni");
    RSDK_ENUM_VAR("Right");
    RSDK_ENUM_VAR("Left");
}
#endif

#if RETRO_REV0U
void FlingRamp::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(FlingRamp);

    sVars->aniFrames.Init();
}
#endif

void FlingRamp::Serialize() { RSDK_EDITABLE_VAR(FlingRamp, VAR_UINT8, direction); }

} // namespace GameLogic