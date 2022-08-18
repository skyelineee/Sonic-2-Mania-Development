// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Camera Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Camera.hpp"
#include "Player.hpp"
#include "Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Camera);

void Camera::Update() {}
void Camera::LateUpdate()
{
    this->lastPos.x = this->position.x;
    this->lastPos.y = this->position.y;

    this->state.Run(this);

    this->velocity.x = this->position.x - this->lastPos.x;
    this->velocity.y = this->position.y - this->lastPos.y;

    SetCameraBoundsXY();

    if (this->shakePos.x) {
        if (this->shakePos.x > 0 && --this->shakeTimer.x <= 0) {
            this->shakeTimer.x = this->shakeDuration.x;
            this->shakePos.x   = 1 - this->shakePos.x;
        }
        else {
            this->shakePos.x = -this->shakePos.x;
        }
    }

    if (this->shakePos.y) {
        if (this->shakePos.y > 0 && --this->shakeTimer.y <= 0) {
            this->shakeTimer.y = this->shakeDuration.y;
            this->shakePos.y   = 1 - this->shakePos.y;
        }
        else {
            this->shakePos.y = -this->shakePos.y;
        }
    }

    if (!this->disableYOffset)
        this->offset.y = MAX(this->offset.y - (this->offset.y >> 3), 0);
}
void Camera::StaticUpdate()
{
    int32 max = (globals->secrets & SECRET_CAMERATRACKING) ? 0x180000 : 0x100000;
    if (sVars->centerBounds.x < max)
        sVars->centerBounds.x += 0x4000;

    if (sVars->centerBounds.y < 0x180000)
        sVars->centerBounds.y += 0x8000;
}
void Camera::Draw() {}

void Camera::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->offset.x = 0x80000;
        this->centerY  = 104;
        if (this->active != ACTIVE_NORMAL) {
            this->screenID = VOID_TO_INT(data);
            Graphics::AddCamera(&this->center, screenInfo[this->screenID].center.x << 16, screenInfo[this->screenID].center.y << 16, false);
        }
        this->unknown3.x     = 3;
        this->unknown3.y     = 2;
        this->boundsOffset.x = 3;
        this->boundsOffset.y = 2;
        this->field_10C      = 2;
        this->active         = ACTIVE_NORMAL;

        State_Init();
    }
}

void Camera::StageLoad()
{
    for (int32 s = 0; s < Graphics::GetVideoSetting(VIDEOSETTING_SCREENCOUNT); ++s) GameObject::Reset(SLOT_CAMERA1 + s, sVars->classID, s);

    sVars->centerBounds.x     = (globals->secrets & SECRET_CAMERATRACKING) ? 0x180000 : 0x100000;
    sVars->centerBounds.y     = 0x180000;
    sVars->lastCenterBounds.x = sVars->centerBounds.x;
    sVars->lastCenterBounds.y = sVars->centerBounds.y;
}

void Camera::State_Init()
{
    SET_CURRENT_STATE();

    this->boundsL = Zone::sVars->cameraBoundsL[this->screenID];
    this->boundsR = Zone::sVars->cameraBoundsR[this->screenID];
    this->boundsT = Zone::sVars->cameraBoundsT[this->screenID];
    this->boundsB = Zone::sVars->cameraBoundsB[this->screenID];

    if (this->target) {
        this->position.x = this->target->position.x;
        this->position.y = this->target->position.y;
        if (Player::sVars && this->target->classID == Player::sVars->classID)
            this->state.Set(&Camera::State_FollowXY);
        else
            this->state.Set(&Camera::State_Center);
    }
    else {
        this->state.Set(&Camera::State_MapView);
    }
}

void Camera::SetCameraBoundsXY()
{
    ScreenInfo *screen = &screenInfo[this->screenID];

    screen->position.x = (this->position.x >> 0x10) + this->lookPos.x - screen->center.x;
    screen->position.y = (this->position.y >> 0x10) + this->lookPos.y - this->centerY;

    if (this->lastPos.x - this->position.x <= 0) {
        int32 targetPos = this->boundsR - screen->size.x;
        if (screen->position.x > targetPos)
            screen->position.x = targetPos;

        if (this->boundsL > targetPos)
            this->boundsL = targetPos;

        if (screen->position.x < this->boundsL)
            screen->position.x = this->boundsL;
    }
    else {
        if (screen->position.x < this->boundsL)
            screen->position.x = this->boundsL;

        if (this->boundsR < this->boundsL + screen->size.x)
            this->boundsR = this->boundsL + screen->size.x;

        if (screen->position.x > this->boundsR - screen->size.x)
            screen->position.x = this->boundsR - screen->size.x;
    }

    if (this->lastPos.y - this->position.y <= 0) {
        int32 targetPos = this->boundsB - screen->size.y;
        if (screen->position.y > targetPos)
            screen->position.y = targetPos;

        if (this->boundsT > targetPos)
            this->boundsT = targetPos;

        if (screen->position.y < this->boundsT)
            screen->position.y = this->boundsT;
    }
    else {
        if (screen->position.y < this->boundsT)
            screen->position.y = this->boundsT;

        if (this->boundsB < this->boundsT + screen->size.y)
            this->boundsB = this->boundsT + screen->size.y;

        if (screen->position.y > this->boundsB - screen->size.y)
            screen->position.y = this->boundsB - screen->size.y;
    }

    screen->position.x += this->shakePos.x;
    screen->position.y += this->shakePos.y;

    this->center.x = screen->position.x + screen->center.x;
    this->center.y = screen->position.y + screen->center.y;
}

void Camera::SetCameraBoundsX()
{
    ScreenInfo *screen = &screenInfo[this->screenID];

    screen->position.x = (this->position.x >> 16) - screen->center.x + this->lookPos.x;

    if (screen->position.x < Zone::sVars->cameraBoundsL[this->screenID])
        screen->position.x = Zone::sVars->cameraBoundsL[this->screenID];

    if (screen->position.x + screen->size.x > Zone::sVars->cameraBoundsR[this->screenID])
        screen->position.x = Zone::sVars->cameraBoundsR[this->screenID] - screen->size.x;

    screen->position.x += this->shakePos.x;
    screen->position.y += this->shakePos.y;
    this->center.x = screen->center.x + screen->position.x;
    this->center.y = screen->center.y + screen->position.y;
}

Camera *Camera::SetTargetEntity(int32 screen, RSDK::GameObject::Entity *target)
{
    for (auto camera : GameObject::GetEntities<Camera>(FOR_ALL_ENTITIES)) {
        if (camera->screenID == screen) {
            camera->target     = target;
            camera->position.x = target->position.x;
            camera->position.y = target->position.y;
            return camera;
        }
    }

    return nullptr;
}

void Camera::ShakeScreen(int32 screen, int32 strengthX, int32 strengthY, int32 durationX, int32 durationY)
{
    for (auto camera : GameObject::GetEntities<Camera>(FOR_ALL_ENTITIES)) {
        if (camera->screenID == screen) {
            camera->shakePos.x      = -strengthX;
            camera->shakePos.y      = -strengthY;
            camera->shakeDuration.x = durationX;
            camera->shakeDuration.y = durationY;
            camera->shakeTimer.x    = durationX;
            camera->shakeTimer.y    = durationY;
            break;
        }
    }
}

void Camera::HandleHBounds()
{
    ScreenInfo *screen = &screenInfo[this->screenID];

    if (Zone::sVars->cameraBoundsL[this->screenID] > this->boundsL) {
        if (screen->position.x > Zone::sVars->cameraBoundsL[this->screenID])
            this->boundsL = Zone::sVars->cameraBoundsL[this->screenID];
        else
            this->boundsL = screen->position.x;
    }

    if (Zone::sVars->cameraBoundsL[this->screenID] < this->boundsL) {
        if (screen->position.x <= this->boundsL) {
            int32 off     = this->boundsL - this->boundsOffset.x;
            this->boundsL = off;
            if (this->velocity.x < 0) {
                this->boundsL += (this->velocity.x >> 0x10);
                if (this->boundsL < Zone::sVars->cameraBoundsL[this->screenID])
                    this->boundsL = Zone::sVars->cameraBoundsL[this->screenID];
            }
        }
        else {
            this->boundsL = Zone::sVars->cameraBoundsL[this->screenID];
        }
    }

    if (Zone::sVars->cameraBoundsR[this->screenID] < this->boundsR) {
        if (screen->size.x + screen->position.x < Zone::sVars->cameraBoundsR[this->screenID])
            this->boundsR = Zone::sVars->cameraBoundsR[this->screenID];
        else
            this->boundsR = screen->size.x + screen->position.x;
    }

    if (Zone::sVars->cameraBoundsR[this->screenID] > this->boundsR) {
        if (screen->size.x + screen->position.x >= this->boundsR) {
            this->boundsR += this->boundsOffset.x;
            if (this->velocity.x > 0) {
                this->boundsR = (this->velocity.x >> 0x10) + this->boundsR;
                if (this->boundsR > Zone::sVars->cameraBoundsR[this->screenID])
                    this->boundsR = Zone::sVars->cameraBoundsR[this->screenID];
            }
        }
        else {
            this->boundsR = Zone::sVars->cameraBoundsR[this->screenID];
        }
    }

    Zone::sVars->playerBoundsL[this->screenID] = this->boundsL << 16;
    Zone::sVars->playerBoundsR[this->screenID] = this->boundsR << 16;
}
void Camera::HandleVBounds()
{
    ScreenInfo *screen = &screenInfo[this->screenID];

    if (Zone::sVars->cameraBoundsT[this->screenID] > this->boundsT) {
        if (screen->position.y <= this->boundsT)
            this->boundsT = screen->position.y + this->boundsOffset.y;
        else
            this->boundsT = screen->position.y;
    }

    if (Zone::sVars->cameraBoundsT[this->screenID] < this->boundsT) {
        if (screen->position.y <= this->boundsT) {
            this->boundsT = this->boundsT - this->boundsOffset.y;

            if (this->velocity.y < 0) {
                this->boundsT += this->velocity.y >> 0x10;
                if (this->boundsT < Zone::sVars->cameraBoundsT[this->screenID])
                    this->boundsT = Zone::sVars->cameraBoundsT[this->screenID];
            }
        }
        else {
            this->boundsT = Zone::sVars->cameraBoundsT[this->screenID];
        }
    }

    if (Zone::sVars->cameraBoundsB[this->screenID] < this->boundsB) {
        if (screen->size.y + screen->position.y >= this->boundsB)
            this->boundsB -= 2;
        else
            this->boundsB = screen->size.y + screen->position.y;
    }

    if (Zone::sVars->cameraBoundsB[this->screenID] > this->boundsB) {
        if (screen->size.y + screen->position.y >= this->boundsB) {
            this->boundsB += this->boundsOffset.y;

            if (this->velocity.y > 0) {
                this->boundsB += this->velocity.y >> 0x10;
                if (this->boundsB > Zone::sVars->cameraBoundsB[this->screenID])
                    this->boundsB = Zone::sVars->cameraBoundsB[this->screenID];
            }
        }
        else {
            this->boundsB = Zone::sVars->cameraBoundsB[this->screenID];
        }
    }

    Zone::sVars->playerBoundsT[this->screenID] = this->boundsT << 16;
    Zone::sVars->playerBoundsB[this->screenID] = this->boundsB << 16;
}

void Camera::State_MapView()
{
    SET_CURRENT_STATE();

    ScreenInfo *screen = &screenInfo[this->screenID];

    int32 speed = 0x100000;
    if (!controllerInfo[Input::CONT_P1].keyA.down)
        speed = 0x40000;

    if (controllerInfo[Input::CONT_P1].keyUp.down)
        this->position.y -= speed;
    else if (controllerInfo[Input::CONT_P1].keyDown.down)
        this->position.y += speed;
    if (controllerInfo[Input::CONT_P1].keyLeft.down)
        this->position.x -= speed;
    else if (controllerInfo[Input::CONT_P1].keyRight.down)
        this->position.x += speed;

    this->position.x = this->position.x >> 0x10;
    this->position.y = this->position.y >> 0x10;

    if (this->position.x >= screen->center.x) {
        if (this->position.x > Zone::sVars->cameraBoundsR[this->screenID] - screen->center.x)
            this->position.x = Zone::sVars->cameraBoundsR[this->screenID] - screen->center.x;
    }
    else {
        this->position.x = screen->center.x;
    }

    if (this->position.y >= screen->center.y) {
        if (this->position.y > Zone::sVars->cameraBoundsB[this->screenID] - screen->center.y)
            this->position.y = Zone::sVars->cameraBoundsB[this->screenID] - screen->center.y;
    }
    else {
        this->position.y = screen->center.y;
    }

    this->position.x <<= 0x10;
    this->position.y <<= 0x10;
}
void Camera::State_Center()
{
    SET_CURRENT_STATE();

    if (this->target) {
        this->position  = this->target->position;
        this->centerY   = screenInfo[this->screenID].center.y;
        this->lookPos.x = 0;
        this->lookPos.y = 0;
        this->offset.x  = 0;
        this->offset.y  = 0;
    }
}
void Camera::State_FollowXY()
{
    SET_CURRENT_STATE();

    if (this->target) {
        HandleHBounds();
        HandleVBounds();

        Entity *target = this->target;

        Vector2 centerBounds = sVars->centerBounds;

        if (centerBounds.x == sVars->lastCenterBounds.x && centerBounds.y == sVars->lastCenterBounds.y) {
            ScreenInfo *screen = &screenInfo[this->screenID];

            int32 multX = MIN(abs(target->position.x - this->offset.x - this->position.x) / (80 * screen->center.x / 100), 0x10000);
            int32 multY = MIN(abs(target->position.y - this->adjustY - this->offset.y - this->position.y) / (80 * screen->center.y / 100), 0x10000);

            int32 x = (target->velocity.x - centerBounds.x) & ~((target->velocity.x - centerBounds.x) >> 31);
            int32 y = (target->velocity.y - centerBounds.y) & ~((target->velocity.y - centerBounds.y) >> 31);

            centerBounds.x += (multX * (x >> 16)) & 0xFFFF0000;
            centerBounds.y += (multY * (y >> 16)) & 0xFFFF0000;

            if (abs(target->velocity.x - centerBounds.x) < 0x10000)
                centerBounds.x = abs(target->velocity.x);

            if (abs(target->velocity.y - centerBounds.y) < 0x10000)
                centerBounds.x = abs(target->velocity.y);
        }

        target->position.x += this->targetMoveVel.x;

        if (target->position.x <= this->position.x + this->offset.x) {
            if (target->position.x < this->position.x - this->offset.x) {
                int32 pos = target->position.x + this->offset.x - this->position.x;
                if (pos < -centerBounds.x)
                    pos = -centerBounds.x;
                this->position.x += pos;
            }
            target->position.x -= this->targetMoveVel.x;
        }
        else {
            int32 pos = target->position.x - this->position.x - this->offset.x;
            if (pos > centerBounds.x)
                pos = centerBounds.x;
            this->position.x += pos;

            target->position.x -= this->targetMoveVel.x;
        }

        target->position.y += this->targetMoveVel.y;

        int32 adjust = target->position.y - this->adjustY;
        if (adjust <= this->position.y + this->offset.y) {
            if (adjust < this->position.y - this->offset.y) {
                int32 pos = target->position.y + this->offset.y - this->position.y - this->adjustY;
                if (pos < -centerBounds.y)
                    pos = -centerBounds.y;
                this->position.y += pos;
            }

            target->position.y -= this->targetMoveVel.y;
        }
        else {
            int32 pos = adjust - this->position.y - this->offset.y;
            if (pos > centerBounds.y)
                pos = centerBounds.y;
            this->position.y += pos;
            target->position.y -= this->targetMoveVel.y;
        }
    }
}
void Camera::State_FollowX()
{
    SET_CURRENT_STATE();

    if (this->target) {
        HandleHBounds();

        Entity *target = this->target;
        target->position.x += this->targetMoveVel.x;

        if (target->position.x <= this->position.x + this->offset.x) {
            if (target->position.x < this->position.x - this->offset.x) {
                int32 pos = target->position.x + this->offset.x - this->position.x;
                if (pos < -sVars->centerBounds.x)
                    pos = -sVars->centerBounds.x;
                this->position.x = this->position.x + pos;
            }
            target->position.x -= this->targetMoveVel.x;
        }
        else {
            int32 pos = target->position.x - this->position.x - this->offset.x;
            if (pos > sVars->centerBounds.x)
                pos = sVars->centerBounds.x;
            this->position.x = this->position.x + pos;
            target->position.x -= this->targetMoveVel.x;
        }
    }
}
void Camera::State_FollowY()
{
    SET_CURRENT_STATE();

    if (this->target) {
        HandleVBounds();

        Entity *target = this->target;
        target->position.y += this->targetMoveVel.y;

        int32 adjust = target->position.y - this->adjustY;
        if (adjust <= this->position.y + this->offset.y) {
            if (adjust < this->position.y - this->offset.y) {
                int32 pos = target->position.y + this->offset.y - this->position.y - this->adjustY;
                if (pos < -sVars->centerBounds.y)
                    pos = -sVars->centerBounds.y;
                this->position.y = this->position.y + pos;
            }
            target->position.y -= this->targetMoveVel.y;
        }
        else {
            int32 pos = adjust - this->position.y - this->offset.y;
            if (pos > sVars->centerBounds.y)
                pos = sVars->centerBounds.y;
            this->position.y = this->position.y + pos;
            target->position.y -= this->targetMoveVel.y;
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void Camera::EditorDraw() {}

void Camera::EditorLoad() {}
#endif

#if RETRO_REV0U
void Camera::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(Camera); }
#endif

void Camera::Serialize() {}

} // namespace GameLogic