// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: DebugMode Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "DebugMode.hpp"
#include "Player.hpp"
#include "Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(DebugMode);

void DebugMode::Update()
{
    bool32 moved = false;

    bool32 up    = controllerInfo[Input::CONT_P1].keyUp.down || analogStickInfoL[Input::CONT_P1].keyUp.down;
    bool32 down  = controllerInfo[Input::CONT_P1].keyDown.down || analogStickInfoL[Input::CONT_P1].keyDown.down;
    bool32 left  = controllerInfo[Input::CONT_P1].keyLeft.down || analogStickInfoL[Input::CONT_P1].keyLeft.down;
    bool32 right = controllerInfo[Input::CONT_P1].keyRight.down || analogStickInfoL[Input::CONT_P1].keyRight.down;

    if (up && down) {
        moved = true;
        up   = false;
        down = false;
    }

    if (left && right) {
        moved = true;
        left  = false;
        right = false;
    }

    if (up) {
        this->position.y -= this->velocity.y;
        moved = true;
    }
    else if (down) {
        this->position.y += this->velocity.y;
        moved = true;
    }

    if (left) {
        this->position.x -= this->velocity.y;
        moved = true;
    }
    else if (right) {
        this->position.x += this->velocity.y;
        moved = true;
    }

    if (!moved) {
        this->velocity.y = 0;
    }
    else {
        this->velocity.y += 0xC00;
        if (this->velocity.y > 0x100000)
            this->velocity.y = 0x100000;
    }

    bool32 keySpawn = controllerInfo[Input::CONT_P1].keyY.press;
    bool32 keyBack  = controllerInfo[Input::CONT_P1].keyX.press;

    if (controllerInfo[Input::CONT_P1].keyA.press) {
        sVars->itemID   = sVars->itemID + 1 - (sVars->itemID + 1) / sVars->itemCount * sVars->itemCount;
        sVars->itemType = 0;
    }
    else if (keyBack) {

        {
            // Do this so we can access the player variables again
            Player *player     = (Player *)this;
            player->classID    = Player::sVars->classID;
            player->groundVel  = 0;
            player->velocity.x = 0;
            player->velocity.y = 0;
            player->state.Set(&Player::State_Air);
            player->collisionPlane     = 0;
            player->tileCollisions     = globals->tileCollisionMode;
            player->interaction        = true;
            player->visible            = true;
            player->disableGroundAnims = 0;
            if (Zone::sVars)
                player->drawGroup = Zone::sVars->playerDrawGroup[0];
            else
                player->drawGroup = 1;

            for (int32 p = 0; p < Player::sVars->activePlayerCount; ++p) {
                Player *playerPtr = GameObject::Get<Player>(p);
                playerPtr->drawGroup = player->drawGroup;
                if (playerPtr->sidekick)
                    playerPtr->stateInput.Set(&Player::Input_AI_Follow);
                else
                    playerPtr->stateInput.Set(&Player::Input_Gamepad);
            }

            if (player->camera)
                player->camera->state.Set(&Camera::State_FollowXY);
        }

        sceneInfo->timeEnabled = true;
        if (sceneInfo->minutes == 9 && sceneInfo->seconds == 59 && sceneInfo->milliseconds == 99) {
            sceneInfo->minutes      = 0;
            sceneInfo->seconds      = 0;
            sceneInfo->milliseconds = 0;
        }

        sVars->debugActive = false;
    }
    else if (controllerInfo[Input::CONT_P1].keyA.down) {
        if (controllerInfo[Input::CONT_P1].keyC.press || keySpawn) {
            if (--sVars->itemID < 0)
                sVars->itemID = sVars->itemCount - 1;

            sVars->itemType = 0;
        }

        if (controllerInfo[Input::CONT_P1].keyB.press) {
            sVars->itemType--;

            if (sVars->itemType >= sVars->itemTypeCount)
                sVars->itemType = sVars->itemTypeCount - 1;
        }
    }
    else if (controllerInfo[Input::CONT_P1].keyC.press || keySpawn) {
        sVars->spawn[sVars->itemID].Run(this);
    }
    else if (controllerInfo[Input::CONT_P1].keyB.press) {
        if (sVars->itemType >= sVars->itemTypeCount - 1)
            sVars->itemType = 0;
        else
            sVars->itemType++;
    }
}
void DebugMode::LateUpdate() {}
void DebugMode::StaticUpdate() {}
void DebugMode::Draw() { sVars->draw[sVars->itemID].Run(this); }

void DebugMode::Create(void *data)
{
    this->active  = ACTIVE_NORMAL;
    this->visible = true;
}

void DebugMode::StageLoad()
{
    sVars->itemID      = 0;
    sVars->itemCount   = 0;
    sVars->debugActive = false;

    for (int32 i = 0; i < 0x100; ++i) {
        sVars->classIDs[i] = 0;
        sVars->draw[i].Set(&DebugMode::State_None);
        sVars->spawn[i].Set(nullptr);
    }
}

void DebugMode::State_None() {}

#if RETRO_INCLUDE_EDITOR
void DebugMode::EditorDraw() { this->animator.DrawSprite(&this->position, false); }

void DebugMode::EditorLoad() {}
#endif

#if RETRO_REV0U
void DebugMode::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(DebugMode); }
#endif

void DebugMode::Serialize() {}

} // namespace GameLogic