// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: EggDriller Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "EggDriller.hpp"
#include "EHZEggman.hpp"
#include "Helpers/BadnikHelpers.hpp"
#include "Global/Zone.hpp"
#include "Global/Player.hpp"
#include "Global/Dust.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(EggDriller);

void EggDriller::Update() 
{ 
	this->state.Run(this);

    this->backWheelAnimator.Process();
    this->carAnimator.Process();
    this->drillAnimator.Process();
    this->frontWheelAnimator.Process();

	if (this->type == Drill) {
        if (car->state.Matches(&EggDriller::Car_Driving) || car->state.Matches(&EggDriller::Car_Explode) || car->state.Matches(&EggDriller::Car_Destroyed)) { // this makes the drill hitbox only active after the boss sets up
            for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                if (currentPlayer->CheckBadnikTouch(this, &sVars->hitbox)) {
                    currentPlayer->Hurt(this);
                }
            }
        }
    }

	// this is a very weird way of solving(?) an issue where drawPos.y would be 0 no matter what for some reason??? ig its bc some entities dont have
    // the wheel running which is where the drawpos stuff happens
    if (!car->state.Matches(&EggDriller::Car_Explode) && !car->state.Matches(&EggDriller::Car_Destroyed)) {
		if (this->xOffset == -0xC0000 || this->xOffset == 0x1C0000) { // this gets the two front wheels, as these are their matching xoffsets, everything else shouldnt have these same values
			car->position.y = this->position.y; // sets the y position of the car back to the wheel position (this-> is wheel here due to the previous check)
		}
	}
}

void EggDriller::LateUpdate() {}
void EggDriller::StaticUpdate() {}
void EggDriller::Draw()
{ 
	this->backWheelAnimator.DrawSprite(nullptr, false); 
	Vector2 carPosition;
	carPosition.x = car->position.x;
	carPosition.y = car->position.y - TO_FIXED(11);
	this->carAnimator.DrawSprite(&carPosition, false); 
	this->drillAnimator.DrawSprite(nullptr, false); 
	this->frontWheelAnimator.DrawSprite(nullptr, false);
}

void EggDriller::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        if (data) {
			this->drawFX  = FX_FLIP;
            this->visible = true;
			this->drawGroup = 3;
			this->active = ACTIVE_NORMAL;
            this->type = VOID_TO_INT(data);
            switch (this->type) {
                case Car: 
					this->carAnimator.SetAnimation(sVars->aniFrames, Car - 1, true, 0);
                    this->state.Set(&EggDriller::Car_SelfDriving);
					break;
                case Drill: 
					this->drillAnimator.SetAnimation(sVars->aniFrames, Drill - 1, true, 0); 
					this->state.Set(&EggDriller::Drill_Attached);
					break;
                case BackWheel: 
					this->backWheelAnimator.SetAnimation(sVars->aniFrames, BackWheel - 1, true, 0); 
					this->state.Set(&EggDriller::Wheel_Attached);
					break;
                case FrontWheel: 
					this->frontWheelAnimator.SetAnimation(sVars->aniFrames, FrontWheel - 1, true, 0);
					this->state.Set(&EggDriller::Wheel_Attached);
					break;

				default: break;
			}
        }
    }
}

void EggDriller::StageLoad()
{ 
	sVars->aniFrames.Load("EHZ/Boss.bin", SCOPE_STAGE);
    sVars->hitbox.left   = -8;
    sVars->hitbox.top    = -8;
    sVars->hitbox.right  = 8;
    sVars->hitbox.bottom = 8;
}

void EggDriller::Car_SelfDriving()
{
    this->position.x -= 0x10000;

	this->timer++;
	// if the timer hits 321, sets the state to eggman coming in
	if (this->timer == 321) {
		this->timer = 0;
		this->state.Set(&EggDriller::Car_EggmanEnter);
	}

	// review
	Vector2 exhaustPos;
	exhaustPos.x = this->position.x;
	if ((Zone::sVars->timer &= 15) == 0) {
		if (this->direction == FLIP_NONE) {
			exhaustPos.x += 0x2E0000;
		}
		else {
			exhaustPos.x -= 0x2E0000;
		}

		exhaustPos.y = this->position.y - 0x40000;
		Dust *dust = GameObject::Create<Dust>(nullptr, exhaustPos.x, exhaustPos.y);
		dust->velocity.y = -0x10000;
		dust->state.Set(&Dust::State_DustPuff);
	}
}

void EggDriller::Car_EggmanEnter() {}

void EggDriller::Car_Driving()
{
	this->position.x += this->velocity.x;

		// changes direction when the this gets out of bounds
		if (this->position.x <= this->boundsL) {
			this->velocity.x = -this->velocity.x;
			this->direction        ^= FLIP_X; // whats the difference between ^= and =??? doing this->direction ^= FLIP_X works but not when its just this->direction = FLIP_X
			wheel[0]->direction    ^= FLIP_X; // back wheel
			eggman->direction      ^= FLIP_X;
			drill->direction       ^= FLIP_X;
			wheel[1]->direction    ^= FLIP_X; // front wheels
			wheel[2]->direction    ^= FLIP_X;

			// if we can fire drill, do fire drill
			if (eggman->health == 1) {
				if (drill->state.Matches(&EggDriller::Drill_Attached)) {
					drill->state.Set(&EggDriller::Drill_Fired);
					drill->position.x = this->position.x;
					if (this->direction == FLIP_NONE) {
						drill->position.x -= 0x360000;
						drill->velocity.x = -0x30000;
					}
					else {
						drill->position.x += 0x360000;
						drill->velocity.x = 0x30000;
					}
				}
			}
		}

		// changes direction when the this gets out of bounds
		if (this->position.x >= this->boundsR) {
			this->velocity.x = -this->velocity.x;
			this->direction        ^= FLIP_X;
			wheel[0]->direction    ^= FLIP_X; // back wheel
			eggman->direction      ^= FLIP_X;
			drill->direction       ^= FLIP_X;
			wheel[1]->direction    ^= FLIP_X; // front wheels
			wheel[2]->direction    ^= FLIP_X;

			// if we can fire drill, do fire drill
			if (eggman->health == 1) {
				if (drill->state.Matches(&EggDriller::Drill_Attached)) {
					drill->state.Set(&EggDriller::Drill_Fired);
					drill->position.x = this->position.x;
					if (this->direction == FLIP_NONE) {
						drill->position.x -= 0x360000;
						drill->velocity.x = -0x30000;
					}
					else {
						drill->position.x += 0x360000;
						drill->velocity.x = 0x30000;
					}
				}
			}
		}

		// Handle eggman pos
		eggman->position.x = this->position.x;
		eggman->position.y = this->position.y - TO_FIXED(18);

		// review
		Vector2 exhaustPos;
		exhaustPos.x = this->position.x;
		if ((Zone::sVars->timer &= 15) == 0) {
			if (this->direction == FLIP_NONE) {
				exhaustPos.x += 0x2E0000;
			}
			else {
				exhaustPos.x -= 0x2E0000;
			}

			exhaustPos.y = this->position.y - 0x40000;
			Dust *dust = GameObject::Create<Dust>(nullptr, exhaustPos.x, exhaustPos.y);
			dust->velocity.y = -0x10000;
			dust->state.Set(&Dust::State_DustPuff);
		}
}

void EggDriller::Car_Explode()
{
	this->position.y += this->velocity.y;
	this->velocity.y += 0x3800;

	int32 wheelXVelocity; // pretty sure temp0 in the original was for setting the horizontal velocity of each wheel
	if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 17 << 16, 0)) {
		if (this->direction == FLIP_NONE) {
			wheelXVelocity = -0x20000;
		}
		else {
			wheelXVelocity = 0x20000;
		}

		// Back Wheel
		wheel[0]->state.Set(&EggDriller::Wheel_Bounce);
		wheel[0]->velocity.x = wheelXVelocity;
		wheel[0]->velocity.y = 0x30000;
		wheel[0]->velocity.x = -wheel[0]->velocity.x; // flip sign

		// Front Wheel 1
		wheel[1]->state.Set(&EggDriller::Wheel_Bounce);
		wheel[1]->velocity.x = wheelXVelocity;
		wheel[1]->velocity.y = 0x30000;

		// Front Wheel 2
		wheel[2]->state.Set(&EggDriller::Wheel_Bounce);
		wheel[2]->velocity.x = wheelXVelocity;
		wheel[2]->velocity.y = 0x30000;

		this->state.Set(&EggDriller::Car_Destroyed);
	}

	// Eggman
    eggman->position.x = this->position.x;
	eggman->position.y = this->position.y - TO_FIXED(18);
}

void EggDriller::Car_Destroyed()
{
	if (this->velocity.y > 0) { // this checks if its going down (positive value)
		eggman->position.y = this->position.y - TO_FIXED(18);
		if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 4 << 16, 0)) {
			this->velocity.y = 0; // sets the y velocity to 0 once it makes contact with the ground
		}
	}

	this->position.y += this->velocity.y;
	if (!this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 4 << 16, 0)) { // need this check here, as if theres none the velocity keeps getting added anyway
		this->velocity.y += 0x2000; // this adds the velocity every frame so the car slowly goes more and more down until it hits the ground and its set to 0
	}
}

void EggDriller::Drill_Attached()
{
	this->position.x = car->position.x;
	if (this->direction == FLIP_NONE) {
		this->position.x -= 0x360000;
	}
	else {
		this->position.x += 0x360000;
	}

	this->position.y = car->position.y - TO_FIXED(9);
}

void EggDriller::Drill_Idle() {}

void EggDriller::Drill_Fired()
{
	this->position.x += this->velocity.x;

	this->timer++;
	if (this->timer == 18) {
		this->timer = 0;
	}

	if (this->CheckOnScreen(nullptr)) {
		if (this->isDrillOOB == true) {
			this->Destroy();
		}
	}
	else {
		this->isDrillOOB = true;
	}
}

void EggDriller::Wheel_Attached() 
{
	this->position.x = car->position.x;
	if (this->direction == FLIP_NONE) {
		this->position.x += this->xOffset; // this changes the wheel position by adding whatever the xoffset for that wheel is
	}
	else {
		this->position.x -= this->xOffset;
	}


	this->position.y = car->position.y;
	this->position.y += 0xC0000;
	this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 17 << 16, 32);
}

void EggDriller::Wheel_Idle() {}

void EggDriller::Wheel_Bounce()
{
	if (this->velocity.y >= 0) { // this checks if its going down
		if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 17 << 16, 0)) {
			this->velocity.y = -this->velocity.y; // flip sign and reverses the velocity when it hits the ground
		}
	}

	this->position.x += this->velocity.x;
	this->position.y += this->velocity.y;
	this->velocity.y += 0x3800; // this adds the velocity every frame so the wheel slowly goes more and more down until it hits the ground and the velocity gets reversed

	if (!this->CheckOnScreen(nullptr)) {
		this->Destroy();
	}
}

#if RETRO_INCLUDE_EDITOR
void EggDriller::EditorDraw() {}

void EggDriller::EditorLoad() {}
#endif

void EggDriller::Serialize() {}

} // namespace GameLogic