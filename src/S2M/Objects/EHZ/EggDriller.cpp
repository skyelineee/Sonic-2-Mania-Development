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

    if (car->state.Matches(&EggDriller::Car_SelfDriving) || car->state.Matches(&EggDriller::Car_EggmanEnter) || car->state.Matches(&EggDriller::Car_Driving)) {
        car->drawPos.y = car->carPos.y - 0xC0000;
        car->carPos.y  = 0;
    }

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
}

void EggDriller::LateUpdate() {}
void EggDriller::StaticUpdate() {}
void EggDriller::Draw()
{ 
	this->backWheelAnimator.DrawSprite(nullptr, false); 
	Vector2 carPosition;
	carPosition.x = car->position.x;
	carPosition.y = car->drawPos.y;
	this->carAnimator.DrawSprite(&carPosition, false); 
	//this->carAnimator.DrawSprite(nullptr, false); 
	this->drillAnimator.DrawSprite(nullptr, false); 
	this->frontWheelAnimator.DrawSprite(nullptr, false);
}

void EggDriller::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        if (data) {
			this->drawFX  = FX_FLIP;
            this->visible = true;
			this->drawGroup = 4;
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
	if (Zone::sVars->timer &= 15) {
		if (this->direction == FLIP_NONE) {
			exhaustPos.x += 0x2E0000;
		}
		else {
			exhaustPos.x -= 0x2E0000;
		}

		exhaustPos.y = this->drawPos.y -= 0x40000;
		//CreateTempObject(TypeName[Exhaust Puff], 0, temp0, temp1);
		//object[tempObjectPos].yvel = -0x10000;
	}
}

void EggDriller::Car_EggmanEnter() {}

void EggDriller::Car_Driving()
{
	this->position.x += this->velocity.x;

		// changes direction when the this gets out of bounds
		if (this->position.x <= this->boundsL) {
			this->velocity.x = -this->velocity.x;
			this->direction         ^= FLIP_X; // whats the difference between ^= and =??? doing this->direction ^= FLIP_X works but not when its just this->direction = FLIP_X
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
			this->direction         ^= FLIP_X;
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
		eggman->position.y = this->drawPos.y;
		eggman->position.y -= 0x80000;

		// review
		Vector2 exhaustPos;
		exhaustPos.x = this->position.x;
		if (Zone::sVars->timer &= 15) {
			if (this->direction == FLIP_NONE) {
				exhaustPos.x += 0x2E0000;
			}
			else {
				exhaustPos.x -= 0x2E0000;
			}

			exhaustPos.y = this->drawPos.y - 0x40000;
			//CreateTempObject(TypeName[Exhaust Puff], 0, temp0, temp1)
			//object[tempObjectPos].yvel = -0x10000
		}
}

void EggDriller::Car_Explode()
{
}

void EggDriller::Car_Destroyed() {}

void EggDriller::Drill_Attached()
{
	this->position.x = car->position.x;
	if (this->direction == FLIP_NONE) {
		this->position.x -= 0x360000;
	}
	else {
		this->position.x += 0x360000;
	}

	this->position.y = car->drawPos.y; // car->drawPos.y;
	this->position.y += 0x80000;

	/*if (car->state > EggDriller::Car_EggmanEnter) { // this is for the drill animation i think but i dont htink its needed
		object.timer++
		if (object.timer == 18) {
			object.timer = 0
		}

		object.frame = object.timer
		object.frame /= 6
	}*/
}

void EggDriller::Drill_Idle() {}

void EggDriller::Drill_Fired()
{
	this->position.x += this->velocity.x;

	this->timer++;
	if (this->timer == 18) {
		this->timer = 0;
	}

	ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];
    Vector2 range;
    range.x = screen->size.x << 16;
    range.y = screen->size.y << 16;

	if (this->CheckOnScreen(&range)) {
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
		this->position.x += this->xOffset;
	}
	else {
		this->position.x -= this->xOffset;
	}


	this->position.y = car->position.y;
	this->position.y += 0xC0000;
	this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 17 << 16, 32);

	if (this->xOffset != -0x2C0000) {
		car->carPos.y += this->position.y; // CHECK THIS
	}
}

void EggDriller::Wheel_Idle() {}

void EggDriller::Wheel_Bounce()
{
}

#if RETRO_INCLUDE_EDITOR
void EggDriller::EditorDraw() {}

void EggDriller::EditorLoad() {}
#endif

void EggDriller::Serialize() {}

} // namespace GameLogic