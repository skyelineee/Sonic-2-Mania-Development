// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SpeedBooster Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SpeedBooster.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SpeedBooster);

void SpeedBooster::Update()
{
    this->state.Run(this);

    for (int32 i = 0; i < PLAYER_COUNT; i++) {
        if (this->playerTimer[i])
            this->playerTimer[i]--;
    }
}

void SpeedBooster::LateUpdate() {}

void SpeedBooster::StaticUpdate() {}

void SpeedBooster::Draw() { this->animator.DrawSprite(&this->drawPos, false); }

void SpeedBooster::Create(void *data)
{
    this->visible = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[1];

    this->drawPos       = this->position;
    this->updateRange.x = 0x400000;
    this->updateRange.y = 0x400000;

    this->active = ACTIVE_BOUNDS;
    if (!this->speed)
        this->speed = Stage::CheckSceneFolder("CPZ") ? 10 : 16;
    this->groundVel = this->speed << 16;

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->state.Set(&SpeedBooster::State_SpeedBooster);
}

void SpeedBooster::StageLoad()
{
    sVars->aniFrames.Load("CPZ/SpeedBooster.bin", SCOPE_STAGE);

    sVars->hitbox.left   = -16;
    sVars->hitbox.top    = -16;
    sVars->hitbox.right  = 16;
    sVars->hitbox.bottom = 16;

    sVars->sfxSpeedBooster.Get("Global/Spring.wav");

    sVars->defaultState.Set(&SpeedBooster::State_SpeedBooster);

    DebugMode::AddObject(sVars->classID, &SpeedBooster::DebugSpawn, &SpeedBooster::DebugDraw);
}

void SpeedBooster::DebugSpawn() { GameObject::Create<SpeedBooster>(nullptr, this->position.x, this->position.y); }

void SpeedBooster::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void SpeedBooster::State_SpeedBooster()
{
    this->animator.Process();

    SpeedBooster::HandleInteractions();
}

void SpeedBooster::HandleInteractions()
{
    bool32 isSSZ = false;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        int32 playerID = RSDKTable->GetEntitySlot(player);
        if (this->playerTimer[playerID] || player->animator.animationID == Player::ANI_HURT
            || !player->CheckCollisionTouch(this, &sVars->hitbox) || !player->onGround) {
            this->playerPos[playerID] = player->position.x;
        }
        else {
            this->velocity.x       = 2 * (player->position.x < this->position.x) - 1;
            this->state            = sVars->defaultState;
            this->animator.frameID = 0;
            sVars->sfxSpeedBooster.Play(false, 255);
            this->active = ACTIVE_NORMAL;

            bool32 flipFlag = isSSZ ? this->playerPos[playerID] <= this->position.x : !this->direction;
            if (flipFlag) {
                if (player->groundVel < this->groundVel)
                    player->groundVel = this->groundVel;
                player->direction = FLIP_NONE;
            }
            else {
                if (player->groundVel > -this->groundVel)
                    player->groundVel = -this->groundVel;
                player->direction = FLIP_X;
            }

            this->playerTimer[playerID] = 30;
            player->controlLock         = 16;
            player->pushing             = false;
            player->tileCollisions      = TILECOLLISION_DOWN;
            if (!player->state.Matches(&Player::State_Roll))
                player->state.Set(&Player::State_Ground);
            this->playerPos[playerID] = player->position.x;
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void SpeedBooster::EditorDraw()
{
    this->visible       = true;
    this->drawPos       = this->position;
    this->updateRange.x = 0x400000;
    this->updateRange.y = 0x400000;

    this->active = ACTIVE_BOUNDS;
    if (!this->speed)
        this->speed = Stage::CheckSceneFolder("CPZ") ? 10 : 16;
    this->groundVel = this->speed << 16;

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    SpeedBooster::Draw();
}

void SpeedBooster::EditorLoad()
{
    sVars->aniFrames.Load("CPZ/SpeedBooster.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip", FLIP_NONE);
    RSDK_ENUM_VAR("Flipped", FLIP_X);
}
#endif

void SpeedBooster::Serialize()
{
    RSDK_EDITABLE_VAR(SpeedBooster, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(SpeedBooster, VAR_UINT8, speed);
}
} // namespace GameLogic