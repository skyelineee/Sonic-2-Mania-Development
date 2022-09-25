// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Message Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Message.hpp"
#include "HP_Setup.hpp"
#include "HP_Player.hpp"
#include "Global/Zone.hpp"
#include "Global/Music.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Message);

void HP_Message::Update() { this->state.Run(this); }
void HP_Message::LateUpdate() {}
void HP_Message::StaticUpdate() {}
void HP_Message::Draw() { this->stateDraw.Run(this); }

void HP_Message::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawGroup = Zone::sVars->hudDrawGroup;

        this->charAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);

        if (data) {
            this->charAnimator.SetAnimation(sVars->aniFrames, 0, true, (int16)VOID_TO_INT(data) - 1);
            this->state.Set(&HP_Message::State_AniChar);
            this->stateDraw.Set(&HP_Message::Draw_AniChar);
            this->duration = 120;
            this->rotation = 0;
            this->scale.x  = 0x200;
            this->scale.y  = 0x200;
            this->drawFX   = FX_FLIP | FX_ROTATE | FX_SCALE;
        }
        else {
            switch (this->type) {
                default:
                case HP_Message::Generic: break;
                case HP_Message::StartMessage: this->state.Set(&HP_Message::State_InitStartMessage); break;
                case HP_Message::RingReminder: this->state.Set(&HP_Message::State_RingReminderTrigger); break;
            }
        }
    }
}

void HP_Message::StageLoad()
{
    sVars->aniFrames.Load("Special/TextMessage.bin", SCOPE_STAGE);

    RSDK::Animator animator;
    animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    for (int32 f = 0; f < animator.frameCount; ++f) {
        animator.frameID = f;
        uint16 unicodeChar = animator.GetFrameID() - '0';

        if (unicodeChar <= 9) 
            sVars->numFrames[unicodeChar] = f;
    }

    if (globals->gameMode == MODE_COMPETITION) {
        for (auto message : GameObject::GetEntities<HP_Message>(FOR_ALL_ENTITIES)) {
            if (message->type == HP_Message::StartMessage) {
                Stage::SetEngineState(ENGINESTATE_PAUSED);
                break;
            }
        }
    }
}

void HP_Message::SetMessage(void (HP_Message::*state)(), int32 number, int32 duration, bool32 resetRings, ...)
{
    this->state.Set(state);
    this->number     = number;
    this->timer      = 0;
    this->duration   = duration;
    this->resetRings = resetRings;

    int32 id = 0;

    va_list args;
    va_start(args, resetRings);
    while (true) {
        const char *message = va_arg(args, const char *);
        if (!message)
            break;

        this->message[id] = message;
        this->message[id].SetSpriteString(sVars->aniFrames, 0);

        id++;
    }
    va_end(args);
}

void HP_Message::SetupAniChars(uint8 id)
{
    this->charPos     = TO_FIXED(4 - (this->message[id].length << 2));
    this->angleOffset = 224 / this->message[id].length;

    for (int32 c = 0; c < this->message[id].length; ++c) {
        this->charAnimator.frameID = this->message[id].chars[c];

        if (this->charAnimator.frameID >= 0) {
            switch (this->charAnimator.GetFrameID()) {
                case '#': this->charAnimator.frameID = sVars->numFrames[this->number / 100]; break;
                case '$': this->charAnimator.frameID = sVars->numFrames[(this->number % 100) / 10]; break;
                case '%': this->charAnimator.frameID = sVars->numFrames[this->number % 10]; break;
                default: break;
            }

            HP_Message *aniChar = GameObject::Create<HP_Message>(1 + this->charAnimator.frameID, this->charPos, this->position.y);

            if (this->spinDirection == FLIP_NONE)
                aniChar->angle = this->angle;
            else
                aniChar->angle = (-this->angle + 0x100) & 0x1FF;

            aniChar->groundVel   = -0x10000;
            aniChar->scaleSpeed  = 16;
            aniChar->rotateSpeed = Math::Rand(-16, 16);
        }

        this->angle = (this->angle - this->angleOffset) & 0x1FF;
        this->charPos += TO_FIXED(8);
    }
}
void HP_Message::DrawMessage(uint8 id)
{
    this->charPos = screenInfo->center.x + 4 - (this->message[id].length << 2);

    for (int32 c = 0; c < this->message[id].length; ++c) {
        this->charAnimator.frameID = this->message[id].chars[c];

        if (this->charAnimator.frameID >= 0) {
            // special characters
            switch (this->charAnimator.GetFrameID()) {
                case '#': this->charAnimator.frameID = sVars->numFrames[this->number / 100]; break;
                case '$': this->charAnimator.frameID = sVars->numFrames[(this->number % 100) / 10]; break;
                case '%': this->charAnimator.frameID = sVars->numFrames[this->number % 10]; break;
                default: break;
            }

            Vector2 drawPos;
            drawPos.x = TO_FIXED(this->charPos);
            drawPos.y = this->position.y;

            if (this->charAnimator.frameID >= 0)
                this->charAnimator.DrawSprite(&drawPos, true);
        }

        this->charPos += 8;
    }
}

// States
void HP_Message::State_SingleMessage()
{
    SET_CURRENT_STATE();

    if (this->stateDraw.Matches(nullptr))
        this->stateDraw.Set(&HP_Message::Draw_Message);

    if (this->timer++ >= this->duration) {
        this->angle = 96;

        if (this->stateDraw.Matches(&HP_Message::Draw_GetRings)) {
            if (this->number > 99)
                this->SetupAniChars(2);
            else if (this->number > 9)
                this->SetupAniChars(1);
            else
                this->SetupAniChars(0);
        }
        else {
            this->SetupAniChars(0);
        }

        this->Destroy();

        if (this->resetRings && HP_Setup::sVars->checkpointID < 3) {
            HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
            HP_Player *player2 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER2);

            player1->rings = 0;
            player2->rings = 0;
        }
    }
}
void HP_Message::State_NotEnough()
{
    SET_CURRENT_STATE();

    if (this->stateDraw.Matches(nullptr))
        this->stateDraw.Set(&HP_Message::Draw_NotEnough);

    if (this->timer++ >= this->duration) {
        this->spinDirection = FLIP_NONE;
        this->position.y    = TO_FIXED(116);
        this->angle         = 96;
        this->SetupAniChars(0);

        this->spinDirection = FLIP_X;
        this->position.y += TO_FIXED(22);
        this->angle = 96;
        this->SetupAniChars(1);

        this->Destroy();
    }
}
void HP_Message::State_RingReminder()
{
    SET_CURRENT_STATE();

    HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
    HP_Player *player2 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER2);

    if (this->stateDraw.Matches(nullptr))
        this->stateDraw.Set(&HP_Message::Draw_RingReminder);

    int32 totalRings = player1->rings + (GET_CHARACTER_ID(2) ? player2->rings : 0);

    if (totalRings < HP_Setup::sVars->ringCounts[HP_Setup::sVars->checkpointID]) {
        this->charAnimator.timer = 0;
    }
    else {
        if (this->charAnimator.timer < 30)
            this->charAnimator.timer++;
    }

    this->visible = false;

    if (this->charAnimator.timer < 30) {
        this->visible = this->timer & 4;
        this->timer   = (this->timer + 1) & 7;

        this->number = HP_Setup::sVars->ringCounts[HP_Setup::sVars->checkpointID] - totalRings;
        if (this->number < 0)
            this->number = 0;
    }
}
void HP_Message::State_AniChar()
{
    SET_CURRENT_STATE();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->scale.x += this->scaleSpeed;
    this->scale.y += this->scaleSpeed;

    this->rotation = (this->rotation + this->rotateSpeed) & 0x1FF;

    this->groundVel -= 0x8000;
    this->scaleSpeed++;

    this->velocity.x = (Math::Sin512(this->angle) * this->groundVel) >> 9;
    this->velocity.y = (Math::Cos512(this->angle) * this->groundVel) >> 9;

    if (this->timer++ >= this->duration)
        this->Destroy();
}

void HP_Message::State_InitStartMessage()
{
    SET_CURRENT_STATE();

    this->fadeColor = 0x000000;
    this->stateDraw.Set(&HP_Message::Draw_Fade);
    this->charAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);

    this->timer = 256;
    if (globals->gameMode == MODE_COMPETITION) {
        // this->vsReady       = false;
        // this->sentReadyFlag = true;
        this->state.Set(&HP_Message::State_StartMessageDelay);
    }
    else {
        // this->vsReady       = true;
        // this->sentReadyFlag = false;

        this->state.Set(&HP_Message::State_StartMessageStartFadeIn);
    }
}
void HP_Message::State_StartMessageDelay()
{
    SET_CURRENT_STATE();

    if (this->timer++ >= 256 + 15) {
        // SendEntity(this, false);

        if (globals->gameMode == MODE_COMPETITION) {
            // if (!ReceiveValue) {
            //     this->vsReady = true;
            // }
            // else {
            //     HP_Message *buffer = GameObject::Get<HP_Message>(0x400);
            //     ReceiveEntity(buffer, true);
            //     this->vsReady = buffer->sentReadyFlag;
            //     buffer->Destroy();
            // }
            //
            // if (this->vsReady) {
            //     this->timer = 256;
            //     this->state.Set(&HP_Message::State_StartMessageSendDelay);
            // }
        }
    }
}
void HP_Message::State_StartMessageSendDelay()
{
    SET_CURRENT_STATE();

    // SendEntity(this, false);

    if (this->timer++ >= 256 + 15) {
        this->state.Set(&HP_Message::State_StartMessageStartFadeIn);
        this->timer = 256;
    }
}
void HP_Message::State_StartMessageStartFadeIn()
{
    SET_CURRENT_STATE();

    Stage::SetEngineState(ENGINESTATE_REGULAR);
    this->state.Set(&HP_Message::State_StartMessageFadeIn);

    if (globals->gameMode == MODE_COMPETITION) {
        HP_Message *message = GameObject::Get<HP_Message>(SLOT_HP_MESSAGE);
        message->Reset(HP_Message::sVars->classID, 0);

        message->position.y = TO_FIXED(116);
        message->SetMessage(&HP_Message::State_RingReminder, 0, 120, false, "PLAYER & PLAYER", nullptr);
    }

    Music::PlayTrack(Music::TRACK_STAGE);
}
void HP_Message::State_StartMessageFadeIn()
{
    SET_CURRENT_STATE();

    if (this->timer > 0) {
        this->timer -= 8;
    }
    else {
        if (globals->gameMode == MODE_COMPETITION)
            this->position.y = -TO_FIXED(75);
        else
            this->position.y = -TO_FIXED(15);

        this->charAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->state.Set(&HP_Message::State_StartMessageEnterMessage);
        this->stateDraw.Set(&HP_Message::Draw_StartMessage);
    }
}
void HP_Message::State_StartMessageEnterMessage()
{
    SET_CURRENT_STATE();

    if (this->position.y < TO_FIXED(108))
        this->position.y += TO_FIXED(1);
    else
        this->state.Set(&HP_Message::State_StartMessageShowMessage);
}
void HP_Message::State_StartMessageShowMessage()
{
    SET_CURRENT_STATE();

    if (this->timer++ >= 30) {
        this->state.Set(&HP_Message::State_StartMessageSetupNextMsg);
        this->stateDraw.Set(nullptr);
        this->timer = 0;

        int32 x = -0x380000;
        int32 y = this->position.y;

        // Flag L
        HP_Message *aniChar = GameObject::Create<HP_Message>(1 + 1, x, y);
        aniChar->charAnimator.SetAnimation(sVars->aniFrames, 1, true, aniChar->charAnimator.frameID);
        aniChar->angle       = 72 & 0x1FF;
        aniChar->scale.x     = 0x200;
        aniChar->scale.y     = 0x200;
        aniChar->groundVel   = -0x10000;
        aniChar->scaleSpeed  = 16;
        aniChar->rotateSpeed = Math::Rand(-16, 16);
        x += 0x180000;

        // "S"
        aniChar = GameObject::Create<HP_Message>(1 + 2, x, y);
        aniChar->charAnimator.SetAnimation(sVars->aniFrames, 1, true, aniChar->charAnimator.frameID);
        aniChar->angle       = 48 & 0x1FF;
        aniChar->scale.x     = 0x200;
        aniChar->scale.y     = 0x200;
        aniChar->groundVel   = -0x10000;
        aniChar->scaleSpeed  = 16;
        aniChar->rotateSpeed = Math::Rand(-16, 16);
        x += 0x100000;

        // "T"
        aniChar = GameObject::Create<HP_Message>(1 + 3, x, y);
        aniChar->charAnimator.SetAnimation(sVars->aniFrames, 1, true, aniChar->charAnimator.frameID);
        aniChar->angle       = 24 & 0x1FF;
        aniChar->scale.x     = 0x200;
        aniChar->scale.y     = 0x200;
        aniChar->groundVel   = -0x10000;
        aniChar->scaleSpeed  = 16;
        aniChar->rotateSpeed = Math::Rand(-16, 16);
        x += 0x100000;

        // "A"
        aniChar = GameObject::Create<HP_Message>(1 + 4, x, y);
        aniChar->charAnimator.SetAnimation(sVars->aniFrames, 1, true, aniChar->charAnimator.frameID);
        aniChar->angle       = 0 & 0x1FF;
        aniChar->scale.x     = 0x200;
        aniChar->scale.y     = 0x200;
        aniChar->groundVel   = -0x10000;
        aniChar->scaleSpeed  = 16;
        aniChar->rotateSpeed = Math::Rand(-16, 16);
        x += 0x100000;

        // "R"
        aniChar = GameObject::Create<HP_Message>(1 + 5, x, y);
        aniChar->charAnimator.SetAnimation(sVars->aniFrames, 1, true, aniChar->charAnimator.frameID);
        aniChar->angle       = -24 & 0x1FF;
        aniChar->scale.x     = 0x200;
        aniChar->scale.y     = 0x200;
        aniChar->groundVel   = -0x10000;
        aniChar->scaleSpeed  = 16;
        aniChar->rotateSpeed = Math::Rand(-16, 16);
        x += 0x100000;

        // "T"
        aniChar = GameObject::Create<HP_Message>(1 + 6, x, y);
        aniChar->charAnimator.SetAnimation(sVars->aniFrames, 1, true, aniChar->charAnimator.frameID);
        aniChar->angle       = -48 & 0x1FF;
        aniChar->scale.x     = 0x200;
        aniChar->scale.y     = 0x200;
        aniChar->groundVel   = -0x10000;
        aniChar->scaleSpeed  = 16;
        aniChar->rotateSpeed = Math::Rand(-16, 16);
        x += 0x180000;

        // Flag R
        aniChar = GameObject::Create<HP_Message>(1 + 7, x, y);
        aniChar->charAnimator.SetAnimation(sVars->aniFrames, 1, true, aniChar->charAnimator.frameID);
        aniChar->angle       = -72 & 0x1FF;
        aniChar->scale.x     = 0x200;
        aniChar->scale.y     = 0x200;
        aniChar->groundVel   = -0x10000;
        aniChar->scaleSpeed  = 16;
        aniChar->rotateSpeed = Math::Rand(-16, 16);
    }
}
void HP_Message::State_StartMessageSetupNextMsg()
{
    SET_CURRENT_STATE();

    if (this->timer++ >= 40) {
        HP_Message *message = GameObject::Get<HP_Message>(SLOT_HP_MESSAGE);
        message->Reset(HP_Message::sVars->classID, 0);

        if (globals->gameMode == MODE_COMPETITION) {
            message->position.y = TO_FIXED(116);
            message->SetMessage(&HP_Message::State_SingleMessage, 0, 90, false, "MOST RINGS WINS!", nullptr);
        }
        else {
            message->position.y = TO_FIXED(116);
            message->SetMessage(&HP_Message::State_SingleMessage, HP_Setup::sVars->ringCounts[HP_Setup::sVars->checkpointID], 90, false,
                                "GET % RINGS!", "GET $% RINGS!", "GET #$% RINGS!", nullptr);
            message->stateDraw.Set(&HP_Message::Draw_GetRings);
        }

        this->Destroy();
    }
}

void HP_Message::State_RingReminderTrigger()
{
    SET_CURRENT_STATE();

    for (auto player : GameObject::GetEntities<HP_Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->localPos.z > this->localPos.z - 0x20000 && player->localPos.z < this->localPos.z + 0x20000) {
            HP_Message *message = GameObject::Get<HP_Message>(SLOT_HP_MESSAGE);
            message->Reset(HP_Message::sVars->classID, 0);

            message->position.y = TO_FIXED(64);
            message->SetMessage(&HP_Message::State_RingReminder, 0, 0, false, "% RING TO GO!", "% RINGS TO GO!", "$% RINGS TO GO!",
                                "#$% RINGS TO GO!", nullptr);
        }
    }
}

// Draw States
void HP_Message::Draw_Message()
{
    SET_CURRENT_STATE();
    this->DrawMessage(0);
}
void HP_Message::Draw_NotEnough()
{
    SET_CURRENT_STATE();

    this->DrawMessage(0);

    this->position.y += TO_FIXED(22);
    this->DrawMessage(1);
    this->position.y -= TO_FIXED(22);
}
void HP_Message::Draw_GetRings()
{
    SET_CURRENT_STATE();

    if (this->number > 99)
        this->DrawMessage(2);
    else if (this->number > 9)
        this->DrawMessage(1);
    else
        this->DrawMessage(0);
}
void HP_Message::Draw_RingReminder()
{
    SET_CURRENT_STATE();

    if (this->number > 99)
        this->DrawMessage(3);
    else if (this->number > 9)
        this->DrawMessage(2);
    else if (this->number == 1)
        this->DrawMessage(1);
    else
        this->DrawMessage(0);
}
void HP_Message::Draw_AniChar()
{
    SET_CURRENT_STATE();

    Vector2 drawPos = this->position;
    drawPos.x += TO_FIXED(screenInfo[sceneInfo->currentScreenID].center.x);
    this->charAnimator.DrawSprite(&drawPos, true);
}
void HP_Message::Draw_StartMessage()
{
    SET_CURRENT_STATE();

    Vector2 drawPos = this->position;
    drawPos.x       = TO_FIXED(screenInfo[sceneInfo->currentScreenID].center.x);
    this->charAnimator.DrawSprite(&drawPos, true);
}
void HP_Message::Draw_Fade()
{
    SET_CURRENT_STATE();

    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    Graphics::FillScreen(this->fadeColor, this->timer, this->timer, this->timer);

    if (globals->gameMode == MODE_COMPETITION) {
        Vector2 drawPos = screen->size;
        drawPos.x       = TO_FIXED(drawPos.x);
        drawPos.y       = TO_FIXED(drawPos.y);

        this->charAnimator.DrawSprite(&drawPos, true);
    }
}

#if RETRO_INCLUDE_EDITOR
void HP_Message::EditorDraw()
{
    this->charAnimator.SetAnimation(sVars->aniFrames, 3, true, this->type);
    this->charAnimator.DrawSprite(nullptr, false);
}

void HP_Message::EditorLoad()
{
    sVars->aniFrames.Load("Special/TextMessage.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Generic");
    RSDK_ENUM_VAR("Start Message");
    RSDK_ENUM_VAR("Ring Reminder");
}
#endif

#if RETRO_REV0U
void HP_Message::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(HP_Message);

    sVars->aniFrames.Init();
}
#endif

void HP_Message::Serialize() { RSDK_EDITABLE_VAR(HP_Message, VAR_UINT8, type); }

} // namespace GameLogic