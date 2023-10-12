// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TornadoPath Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "TornadoPath.hpp"
#include "Tornado.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Global/StarPost.hpp"
#include "Helpers/CutsceneRules.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TornadoPath);

void TornadoPath::Update() { this->state.Run(this); }

void TornadoPath::LateUpdate() {}

void TornadoPath::StaticUpdate() {}

void TornadoPath::Draw() {}

void TornadoPath::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        switch (this->type) {
            case TORNADOPATH_START:
                if (!StarPost::sVars->postIDs[0]
                    /*&& CutsceneRules::CheckPlayerPos(this->position.x - (this->size.x >> 1), this->position.y - (this->size.y >> 1),
                                                    this->position.x + (this->size.x >> 1), this->position.y + (this->size.y >> 1))*/) {
                    TornadoPath::SetupHitbox();
                    this->active = ACTIVE_NORMAL;

                    Camera *camera = GameObject::Get<Camera>(SLOT_CAMERA1);
                    if (camera) {
                        camera->state.Set(nullptr);
                        camera->position.x  = this->position.x;
                        camera->position.y  = this->position.y;
                        sVars->camera = camera;
                    }

                    for (auto player : GameObject::GetEntities<Player>(FOR_ALL_ENTITIES)) { player->camera = nullptr; }

                    screenInfo->position.y = (this->position.y >> 16) - screenInfo->center.y;
                    this->state.Set(&TornadoPath::State_SetTornadoSpeed);
                    this->currentSpeed     = this->targetSpeed;
                }
                break;

            case TORNADOPATH_DUD: this->active = ACTIVE_NEVER; break;

            case TORNADOPATH_SETSPEED:
            case TORNADOPATH_SETSPEED_HIGH:
                this->active = ACTIVE_NEVER;
                this->timer  = 1;
                this->state.Set(&TornadoPath::State_SetTornadoSpeed);
                break;

            case TORNADOPATH_SETCAMERA:
                this->active = ACTIVE_NEVER;
                this->state.Set(&TornadoPath::State_ReturnCamera);
                break;

            case TORNADOPATH_DISABLEINTERACTIONS:
                this->active = ACTIVE_NEVER;
                this->state.Set(&TornadoPath::State_DisablePlayerInteractions);
                break;

            case TORNADOPATH_EXITTORNADO_STOPAUTOSCROLL:
            case TORNADOPATH_EXITTORNADO_TRAINSEQ:
                this->active = ACTIVE_NEVER;
                this->state.Set(&TornadoPath::State_ExitTornadoSequence);
                break;

            case TORNADOPATH_ENTERTORNADO:
            case TORNADOPATH_ENTERTORNADO_FLYTOBOSS:
                if (!StarPost::sVars->postIDs[0]
                    /*&& CutsceneRules::CheckPlayerPos(this->position.x - (this->size.x >> 1), this->position.y - (this->size.y >> 1),
                                                    this->position.x + (this->size.x >> 1), this->position.y + (this->size.y >> 1))*/) {
                    this->active       = ACTIVE_NEVER;
                    this->state.Set(&TornadoPath::State_PrepareCatchPlayer);
                    this->currentSpeed = this->targetSpeed;
                }
                else {
                    this->active       = ACTIVE_XBOUNDS;
                    this->state.Set(&TornadoPath::State_PrepareCatchPlayer);
                    this->currentSpeed = this->targetSpeed;
                }
                break;

            case TORNADOPATH_HANDLEBOSS_MSZ1ST:
                this->active       = ACTIVE_NEVER;
                this->timer        = 1;
                this->state.Set(&TornadoPath::State_UberCaterkillerBossNode);
                this->currentSpeed = this->targetSpeed;
                break;

            case TORNADOPATH_GOTOSTOP:
                this->active = ACTIVE_NEVER;
                this->state.Set(&TornadoPath::State_GoToStopNode);
                break;

            case TORNADOPATH_SETUPMSZ1CUTSCENEST:
                this->active = ACTIVE_NEVER;
                this->state.Set(&TornadoPath::State_SetupMSZ1CutsceneST);
                break;

            case TORNADOPATH_CRASHFINISH:
                this->active = ACTIVE_NEVER;
                this->state.Set(&TornadoPath::State_FinishCrash);
                break;

            default: break;
        }
    }
}

void TornadoPath::StageLoad()
{
    sVars->moveVel.x = 0;
    sVars->moveVel.y = 0;
    sVars->hitboxID  = 0;
}

void TornadoPath::SetupHitbox()
{
    this->hitbox.left   = -(this->size.x >> 17);
    this->hitbox.top    = -(this->size.y >> 17);
    this->hitbox.right  = this->size.x >> 17;
    this->hitbox.bottom = this->size.y >> 17;
}

void TornadoPath::HandleMoveSpeed()
{
    Camera *camera = sVars->camera;
    if (camera) {
        TornadoPath *node       = GameObject::Get<TornadoPath>(sceneInfo->entitySlot + 1);
        int32 y                 = (camera->position.y - node->position.y) >> 16;
        int32 x                 = (camera->position.x - node->position.x) >> 16;
        this->angle             = Math::ATan2(x, y);

        int32 cx = camera->position.x & 0xFFFF0000;
        int32 cy = camera->position.y & 0xFFFF0000;
        camera->position.x -= this->currentSpeed * Math::Cos256(this->angle);
        camera->position.y -= this->currentSpeed * Math::Sin256(this->angle);
        sVars->hitboxID = this->fastMode;

        int32 radius = this->currentSpeed >> 3;
        if (x * x + y * y < radius) {
            this->active       = ACTIVE_NEVER;
            node->active       = ACTIVE_NORMAL;
            node->currentSpeed = node->easeToSpeed ? this->currentSpeed : this->targetSpeed;
        }

        sVars->moveVel.x = (camera->position.x & 0xFFFF0000) - cx;
        sVars->moveVel.y = (camera->position.y & 0xFFFF0000) - cy;
    }
}

void TornadoPath::State_SetTornadoSpeed()
{
    if (this->type == TORNADOPATH_SETSPEED_HIGH) {
        for (auto tornado : GameObject::GetEntities<Tornado>(FOR_ACTIVE_ENTITIES)) { tornado->drawGroup = Zone::sVars->playerDrawGroup[1]; }
    }

    if (this->currentSpeed >= this->targetSpeed) {
        if (this->currentSpeed > this->targetSpeed) {
            this->currentSpeed -= 16;

            if (this->currentSpeed < this->targetSpeed)
                this->currentSpeed = this->targetSpeed;
        }
    }
    else {
        this->currentSpeed += 16;
        if (this->currentSpeed > this->targetSpeed)
            this->currentSpeed = this->targetSpeed;
    }

    if (this->forceSpeed > 0)
        this->currentSpeed = this->forceSpeed;

    if (this->timer <= 0) {
        TornadoPath::HandleMoveSpeed();
    }
    else {
        this->timer--;
    }
}

void TornadoPath::State_ReturnCamera()
{
    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

    sVars->camera->state.Set(&Camera::State_FollowXY);
    player1->camera            = sVars->camera;
}

void TornadoPath::State_DisablePlayerInteractions()
{
    Player *player1   = GameObject::Get<Player>(SLOT_PLAYER1);
    player1->collisionPlane = 1;
    player1->interaction    = false;

    for (auto tornado : GameObject::GetEntities<Tornado>(FOR_ACTIVE_ENTITIES)) { tornado->drawGroup = Zone::sVars->playerDrawGroup[1]; }

    TornadoPath::HandleMoveSpeed();
    this->state.Set(&TornadoPath::State_SetTornadoSpeed);
}

void TornadoPath::State_ExitTornadoSequence()
{
    Player *player1      = GameObject::Get<Player>(SLOT_PLAYER1);
    sVars->camera->state.Set(&Camera::State_FollowXY);

    player1->camera         = sVars->camera;
    player1->collisionPlane = 0;
    player1->interaction    = true;
    player1->applyJumpCap   = false;
    player1->drawGroup      = Zone::sVars->playerDrawGroup[0];

    if (player1->groundedStore)
        player1->Action_Jump();

    for (auto tornado : GameObject::GetEntities<Tornado>(FOR_ACTIVE_ENTITIES))
    {
        if (this->type == TORNADOPATH_EXITTORNADO_STOPAUTOSCROLL) {
            tornado->drawGroup = Zone::sVars->objectDrawGroup[1];
            //MSZSetup_ReloadBGParallax_Multiply(0x000);
            tornado->state.Set(&Tornado::State_FlyAway_Right);
        }
        else {
            tornado->drawGroup = Zone::sVars->objectDrawGroup[0];
            tornado->state.Set(&Tornado::State_FlyAway_Left);
        }
    }

    for (auto node : GameObject::GetEntities<TornadoPath>(FOR_ALL_ENTITIES))
    {
        bool32 isNextNode = false;
        if (this->type == TORNADOPATH_EXITTORNADO_STOPAUTOSCROLL)
            isNextNode = node->type == TORNADOPATH_ENTERTORNADO;
        else
            isNextNode = node->type == TORNADOPATH_ENTERTORNADO_FLYTOBOSS;

        if (isNextNode)
            node->active = ACTIVE_XBOUNDS;
    }

    sVars->camera = nullptr;
    this->active        = ACTIVE_NEVER;
}

void TornadoPath::State_PrepareCatchPlayer()
{
    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

    bool32 isPlayerControlled = false;
    for (auto tornado : GameObject::GetEntities<Tornado>(FOR_ACTIVE_ENTITIES))
    {
        if (tornado->state.Matches(&Tornado::State_PlayerControlled))
            isPlayerControlled = true;
    }

    if (!isPlayerControlled) {
        if (player1->position.x > this->position.x && player1->position.y < this->position.y) {
            this->active        = ACTIVE_NORMAL;
            player1->stateInput.Set(nullptr);
            player1->left       = false;
            player1->right      = true;
            if (player1->pushing > 0) {
                if (player1->onGround)
                    player1->jumpPress = true;

                player1->jumpHold = true;
            }

            if (!player1->onGround) {
                this->state.Set(&TornadoPath::State_CatchPlayer);
                sVars->moveVel.x = 0;
                sVars->moveVel.y = 0;
            }
        }
    }
}

void TornadoPath::State_CatchPlayer()
{
    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

    ++this->timer;
    if (this->timer == 60) {
        for (auto tornado : GameObject::GetEntities<Tornado>(FOR_ALL_ENTITIES))
        {
            tornado->position.x = player1->position.x - 0x1400000;
            tornado->position.y = player1->position.y + 0x400000;
            tornado->offsetX    = 0xA0000;
            tornado->showFlame  = true;
            tornado->state.Set(&Tornado::State_PlayerControlled);
            tornado->active     = ACTIVE_NORMAL;
        }
    }

    if (this->timer == 90) {
        player1->right      = false;
        player1->velocity.x = 0;
        player1->groundVel  = 0;
    }

    if (!player1->velocity.y && !player1->state.Matches(&Player::State_Static)) {
        player1->stateInput.Set(&Player::Input_Gamepad);
        this->timer         = 0;

        int32 velX = 0;
        int32 velY = 0;
        if (this->type == TORNADOPATH_ENTERTORNADO_FLYTOBOSS) {
            for (auto node : GameObject::GetEntities<TornadoPath>(FOR_ALL_ENTITIES))
            {
                if (node->type == TORNADOPATH_HANDLEBOSS_MSZ1ST) {
                    velX         = node->position.x - player1->position.x;
                    velY         = node->position.y - player1->position.y;
                    node->active = ACTIVE_NORMAL;
                }
            }
        }

        player1->position.x += velX;
        player1->position.y += velY;
        Camera *camera = player1->camera;
        if (camera) {
            camera->position.x += velX;
            camera->position.y += velY;
            camera->state.Set(nullptr);
            sVars->camera = camera;
            player1->camera     = nullptr;
        }

        for (auto tornado : GameObject::GetEntities<Tornado>(FOR_ACTIVE_ENTITIES))
        {
            tornado->position.x += velX;
            tornado->position.y += velY;
            tornado->offsetX   = 0x80000;
            tornado->showFlame = false;
        }

        this->state.Set(&TornadoPath::State_SetTornadoSpeed);
    }
}

void TornadoPath::State_UberCaterkillerBossNode()
{
    this->state.Set(&TornadoPath::State_HandleUberCaterkillerBoss);
    TornadoPath::State_HandleUberCaterkillerBoss();
}

void TornadoPath::State_HandleUberCaterkillerBoss()
{
    this->currentSpeed = 0;

    sVars->moveVel.x = 0;
    sVars->moveVel.y = 0;

    //if (UberCaterkiller->defeated)
    //    this->state.Set(&TornadoPath::State_SetTornadoSpeed);
}

void TornadoPath::State_GoToStopNode()
{
    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

    this->currentSpeed     = 0;
    sVars->moveVel.x = 0;
    sVars->moveVel.y = 0;

    if (player1->onGround) {
        int32 velocityX = 0;
        int32 velocityY = 0;

        for (auto node : GameObject::GetEntities<TornadoPath>(FOR_ALL_ENTITIES))
        {
            if (node->type == TORNADOPATH_HANDLEBOSS_MSZ1ST) {
                velocityX    = node->position.x - player1->position.x;
                velocityY    = node->position.y - player1->position.y;
                node->active = ACTIVE_NORMAL;
            }
        }

        player1->position.x += velocityX;
        player1->position.y += velocityY;

        Camera *camera = sVars->camera;
        camera->position.x += velocityX;
        camera->position.y += velocityY;
        for (auto tornado : GameObject::GetEntities<Tornado>(FOR_ACTIVE_ENTITIES))
        {
            tornado->position.x += velocityX;
            tornado->position.y += velocityY;
            tornado->offsetX   = 0x80000;
            tornado->showFlame = false;
        }
    }
}

void TornadoPath::State_SetupMSZ1CutsceneST()
{
    if (!this->timer) {
        //foreach_all(MSZCutsceneST, cutscene)
        //{
        //    cutscene->active   = ACTIVE_NORMAL;
        //    cutscene->pathNode = this;
        //}
    }
}

void TornadoPath::State_FinishCrash()
{
    sVars->moveVel.x     = 0;
    sVars->moveVel.y     = 0;
    sVars->crashFinished = true;
}

#if RETRO_INCLUDE_EDITOR
void TornadoPath::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 7);

    this->animator.DrawSprite(nullptr, false);
}

void TornadoPath::EditorLoad()
{
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);
    sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 7);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Start Node", TORNADOPATH_START);
    RSDK_ENUM_VAR("Blank Node", TORNADOPATH_DUD);
    RSDK_ENUM_VAR("Set Tornado Speed", TORNADOPATH_SETSPEED);
    RSDK_ENUM_VAR("Return Player Camera", TORNADOPATH_SETCAMERA);
    RSDK_ENUM_VAR("Disable Player Interactions", TORNADOPATH_DISABLEINTERACTIONS);
    RSDK_ENUM_VAR("Exit Tornado Sequence (Stop AutoScroll)", TORNADOPATH_EXITTORNADO_STOPAUTOSCROLL);
    RSDK_ENUM_VAR("Exit Tornado Sequence (Start Train Sequence)", TORNADOPATH_EXITTORNADO_TRAINSEQ);
    RSDK_ENUM_VAR("Enter Tornado Sequence", TORNADOPATH_ENTERTORNADO);
    RSDK_ENUM_VAR("Enter Tornado Sequence (Fly to Boss Node)", TORNADOPATH_ENTERTORNADO_FLYTOBOSS);
    RSDK_ENUM_VAR("Handle Uber Caterkiller Boss", TORNADOPATH_HANDLEBOSS_MSZ1ST);
    RSDK_ENUM_VAR("Set Tornado Speed (Set High Group)", TORNADOPATH_SETSPEED_HIGH);
    RSDK_ENUM_VAR("Goto \"Stop Tornado\" Node", TORNADOPATH_GOTOSTOP);
    RSDK_ENUM_VAR("Setup MSZ1CutsceneST", TORNADOPATH_SETUPMSZ1CUTSCENEST);
    RSDK_ENUM_VAR("Finish Crash", TORNADOPATH_CRASHFINISH);
}
#endif

void TornadoPath::Serialize()
{
    RSDK_EDITABLE_VAR(TornadoPath, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(TornadoPath, VAR_ENUM, targetSpeed);
    RSDK_EDITABLE_VAR(TornadoPath, VAR_ENUM, timer);
    RSDK_EDITABLE_VAR(TornadoPath, VAR_BOOL, easeToSpeed);
    RSDK_EDITABLE_VAR(TornadoPath, VAR_BOOL, fastMode);
    RSDK_EDITABLE_VAR(TornadoPath, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(TornadoPath, VAR_ENUM, forceSpeed);
}
} // namespace GameLogic