// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Spiker Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Spiker.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Spiker);

void Spiker::Update()
{
    if (!this->CheckOnScreen(nullptr)) {
        int32 lastPos = this->position.x;
		this->position.x = this->startPosX;

		if (!this->CheckOnScreen(nullptr)) {
			Spiker::Setup();
		}
		else {
			this->position.x = lastPos;
		}
	}

	for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
		if (!this->state.Matches(&Spiker::State_Cone)) {
			if (currentPlayer->CheckBadnikTouch(this, &sVars->hitboxBadnik)) {
				currentPlayer->CheckBadnikBreak(this, true);
			}

			if (this->coneState.Matches(&Spiker::Cone_Attached)) {
				if (currentPlayer->CheckCollisionTouch(this, &sVars->activateHitbox)) {
					this->coneState.Set(&Spiker::Cone_Launching);
					this->launchTimer 	= 8;
					this->backupState 	= this->state;
					this->state.Set(&Spiker::State_Launching);
				}
			}
		}

		if (this->coneState.Matches(&Spiker::Cone_Attached) || this->coneState.Matches(&Spiker::Cone_Launching)) {
			if (currentPlayer->CheckCollisionTouch(this, &sVars->coneHitbox)) {
				currentPlayer->Hurt(this);
			}
		}
	}

	this->state.Run(this);
}
void Spiker::LateUpdate() {}
void Spiker::StaticUpdate() {}
void Spiker::Draw()
{	
	if (!this->state.Matches(&Spiker::State_Cone)) {
		// Actual Spiker object, draw the body frame
		this->animator.DrawSprite(nullptr, false);
	}

	if (this->coneState.Matches(&Spiker::Cone_Attached) || this->coneState.Matches(&Spiker::Cone_Launching)) {
		// Spiker cone projectile
		this->coneAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
		this->coneAnimator.DrawSprite(nullptr, false);
	}
}

void Spiker::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->drawFX        = FX_FLIP;

        this->startPosX     = this->position.x;
        int32 directionPos = GET_BIT(this->direction, 1);
        if (directionPos == 0) {
			this->position.y += 0x80000;
		}	
		else {
			this->position.y -= 0x80000;
		}
		Spiker::Setup();
    }
}

void Spiker::StageLoad() 
{
    sVars->aniFrames.Load("HTZ/Spiker.bin", SCOPE_STAGE);

	sVars->hitboxBadnik.left   = -8;
	sVars->hitboxBadnik.top    = -10;
	sVars->hitboxBadnik.right  = 8;
	sVars->hitboxBadnik.bottom = 10;
	
	sVars->activateHitbox.left   = -32;
	sVars->activateHitbox.top    = -128;
	sVars->activateHitbox.right  = 32;
	sVars->activateHitbox.bottom = 128;
	
	sVars->coneHitbox.left   = -8;
	sVars->coneHitbox.top    = -32;
	sVars->coneHitbox.right  = 8;
	sVars->coneHitbox.bottom = 0;

    DebugMode::AddObject(sVars->classID, &Spiker::DebugSpawn, &Spiker::DebugDraw);
}

void Spiker::DebugSpawn() { GameObject::Create<Spiker>(nullptr, this->position.x, this->position.y); }

void Spiker::DebugDraw()
{
	DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
	DebugMode::sVars->animator.DrawSprite(nullptr, false);
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 1, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Spiker::Setup()
{
	this->direction = this->direction;
	int32 directionVelocity = GET_BIT(this->direction, 0);
	if (directionVelocity == 0) {
		this->velocity.x = -0x8000;
	}
	else {
		this->velocity.x = 0x8000;
	}
	this->velocity.y 		= 0;
	this->timer 			= 64;
	this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
	this->state.Set(&Spiker::State_Pause);
	this->coneState.Set(&Spiker::Cone_Attached);
	this->active 			= ACTIVE_BOUNDS;
}

void Spiker::State_Pause()
{
    this->active = ACTIVE_NORMAL;
    this->position.x += this->velocity.x;

	this->timer--;
	if (this->timer == 0) {
		this->timer = 16;
		this->state.Set(&Spiker::State_Wandering);
	}

	this->animator.Process();
}

void Spiker::State_Wandering()
{
	this->timer--;
	if (this->timer < 0) {
		this->timer = 64;
		this->direction ^= 1;
		this->velocity.x = -this->velocity.x;
		this->state.Set(&Spiker::State_Pause);
	}
}

void Spiker::State_Launching()
{
	this->launchTimer--;
	if (this->launchTimer < 0) {
		this->coneState.Set(&Spiker::Cone_None);
		Spiker *spiker = GameObject::Create<Spiker>(nullptr, this->position.x, this->position.y);
		spiker->coneState.Set(&Spiker::Cone_Attached);
		spiker->state.Set(&Spiker::State_Cone);
		spiker->direction = this->direction;
		int32 directionVelocity = GET_BIT(this->direction, 1);
		if (directionVelocity == 0)
			spiker->velocity.y = -0x20000;
		else
			spiker->velocity.y = 0x20000;
		
		this->state = this->backupState;
	}
}

void Spiker::State_Cone()
{
	this->position.y += this->velocity.y;
	this->direction ^= FLIP_X;
}

void Spiker::Cone_None() {}

void Spiker::Cone_Attached() {}

void Spiker::Cone_Launching() {}

#if RETRO_INCLUDE_EDITOR
void Spiker::EditorDraw() {}

void Spiker::EditorLoad()
{
    sVars->aniFrames.Load("HTZ/Spiker.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Right");
    RSDK_ENUM_VAR("Left (Upside Down)");
    RSDK_ENUM_VAR("Right (Upside Down)");
}
#endif

void Spiker::Serialize() { RSDK_EDITABLE_VAR(Spiker, VAR_UINT8, direction); }

} // namespace GameLogic