// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Batbot Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Batbot.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Batbot);

void Batbot::Update()
{
    this->bodyAnimator.Process();
    if (this->bodyAnimator.animationID == 1)
        this->direction = this->swoopDir ^ sVars->directionTable[this->bodyAnimator.frameID];

    this->state.Run(this);

    Batbot::CheckPlayerCollisions();
    if (!this->state.Matches(&Batbot::State_Init)) {
        if (!this->CheckOnScreen(nullptr) && !RSDKTable->CheckPosOnScreen(&this->startPos, &this->updateRange)) {
            this->direction = this->startDir;
            this->position  = this->startPos;
            Batbot::Create(nullptr);
        }
    }
}

void Batbot::LateUpdate() {}

void Batbot::StaticUpdate() {}

void Batbot::Draw()
{
    this->bodyAnimator.DrawSprite(nullptr, false);
    if (this->showJet)
        this->jetAnimator.DrawSprite(nullptr, false);
}

void Batbot::Create(void *data)
{
    this->visible       = true;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->startPos      = this->position;
    this->startDir      = this->direction;
    this->drawFX        = FX_FLIP;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->state.Set(&Batbot::State_Init);
}

void Batbot::StageLoad()
{
    sVars->aniFrames.Load("HPZ/Batbot.bin", SCOPE_STAGE);

    sVars->hitboxBadnik.left   = -16;
    sVars->hitboxBadnik.top    = -12;
    sVars->hitboxBadnik.right  = 16;
    sVars->hitboxBadnik.bottom = 12;

    sVars->hitboxSpinCheck.left   = -96;
    sVars->hitboxSpinCheck.top    = -64;
    sVars->hitboxSpinCheck.right  = 96;
    sVars->hitboxSpinCheck.bottom = 128;

    sVars->hitboxAttack.left   = -80;
    sVars->hitboxAttack.top    = -64;
    sVars->hitboxAttack.right  = 80;
    sVars->hitboxAttack.bottom = 96;

    sVars->hitboxPlayer.left   = 0;
    sVars->hitboxPlayer.top    = 0;
    sVars->hitboxPlayer.right  = 0;
    sVars->hitboxPlayer.bottom = 0;

    sVars->directionTable[0] = FLIP_NONE;
    sVars->directionTable[1] = FLIP_NONE;
    sVars->directionTable[2] = FLIP_NONE;
    sVars->directionTable[3] = FLIP_X;
    sVars->directionTable[4] = FLIP_X;

    DebugMode::AddObject(sVars->classID, &Batbot::DebugSpawn, &Batbot::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Batbot::DebugSpawn()
{
    Batbot *batBot = GameObject::Create<Batbot>(nullptr, this->position.x, this->position.y);
    batBot->direction    = this->direction;
    batBot->startDir     = this->direction;
}

void Batbot::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 3);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Batbot::CheckPlayerCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik))
            player->CheckBadnikBreak(this, true);
    }
}

void Batbot::State_Init()
{
    this->timer   = 0;
    this->unused3 = 0;
    this->originY = this->position.y;
    this->active  = ACTIVE_NORMAL;
    this->showJet = true;
    this->jetAnimator.SetAnimation(sVars->aniFrames, 3,true, 0);
    this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->state.Set(&Batbot::State_Idle);
    Batbot::State_Idle();
}

void Batbot::State_Idle()
{
    this->arcAngle   = (this->arcAngle + 8) & 0x1FF;
    this->position.y = (Math::Sin512(this->arcAngle) << 9) + this->originY;
    bool32 spin      = false;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (player->CheckCollisionTouchBox(&sVars->hitboxPlayer, this, &sVars->hitboxSpinCheck)) {
            this->bodyAnimator.SetAnimation(sVars->aniFrames, 1, false, 0);
            spin = true;
        }

        if (player->CheckCollisionTouchBox(&sVars->hitboxPlayer, this, &sVars->hitboxAttack)) {
            this->playerPtr = player;
            this->state.Set(&Batbot::State_Attack);
        }
    }

    if (!spin)
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, false, 0);
}

void Batbot::State_Attack()
{
    if (this->arcAngle) {
        this->arcAngle   = (this->arcAngle + 8) & 0x1FF;
        this->position.y = (Math::Sin512(this->arcAngle) << 9) + this->originY;
    }

    if (++this->timer == 20) {
        this->timer   = 0;
        this->landPos = this->position;
        if (this->playerPtr->position.x > this->position.x) {
            this->landPos.x += 0x500000;
            this->swoopAngle = 0x100;
            this->swoopDir   = FLIP_X;
            this->state.Set(&Batbot::State_SwoopRight);
        }
        else {
            this->landPos.x -= 0x500000;
            this->swoopAngle = 0x000;
            this->swoopDir   = FLIP_NONE;
            this->state.Set(&Batbot::State_SwoopLeft);
        }
        this->showJet = false;
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    }
}

void Batbot::State_SwoopLeft()
{
    this->swoopAngle += 4;
    this->position.x = this->landPos.x + 0x2800 * Math::Cos512(this->swoopAngle);
    this->position.y = this->landPos.y + 0x2800 * Math::Sin512(this->swoopAngle);
    if (this->swoopAngle == 0x100) {
        this->showJet = true;
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->state.Set(&Batbot::State_Idle);
    }
}

void Batbot::State_SwoopRight()
{
    this->swoopAngle -= 4;
    this->position.x = this->landPos.x + 0x2800 * Math::Cos512(this->swoopAngle);
    this->position.y = this->landPos.y + 0x2800 * Math::Sin512(this->swoopAngle);
    if (this->swoopAngle == 0x000) {
        this->showJet = true;
        this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->state.Set(&Batbot::State_Idle);
    }
}

#if RETRO_INCLUDE_EDITOR
void Batbot::EditorDraw()
{
    this->jetAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
    this->bodyAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);

    Batbot::Draw();
}

void Batbot::EditorLoad()
{
    sVars->aniFrames.Load("HPZ/Batbot.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip");
    RSDK_ENUM_VAR("Flip X");
}
#endif

void Batbot::Serialize() { RSDK_EDITABLE_VAR(Batbot, VAR_UINT8, direction); }

} // namespace GameLogic