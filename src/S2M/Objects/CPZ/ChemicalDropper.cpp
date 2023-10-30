// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ChemicalDropper Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "ChemicalDropper.hpp"
#include "CPZEggman.hpp"
#include "Global/Zone.hpp"
#include "Global/Player.hpp"
#include "Global/Explosion.hpp"
#include "Helpers/BadnikHelpers.hpp"

using namespace RSDK;

// REMEEEMBERRRRR THERE ARE NOW DIFFERENT STATE MACHINES TO HANDLE THE OBJECT ANIMATIONS ALSO EXISTING AS STATES IN V4, ADD THEM TO THE BASE STATES!!!!!!;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ChemicalDropper);

void ChemicalDropper::Update() 
{
    if (this->type == Drop) {
        this->position.y += this->velocity.y;
		this->velocity.y += 0x3800;
		
		if (!this->CheckOnScreen(nullptr)) {
			this->Destroy();
		}
		
		if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 11 << 16, 0)) {
			ChemicalDropper::Drop_CreateSplash();
		}

		for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
			if (currentPlayer->CheckBadnikTouch(this, &sVars->dropHitbox)) {
				ChemicalDropper::Drop_BubbleHit();
				ChemicalDropper::Drop_CreateSplash();
			}
		}
	}
    else if (this->type == Splash) {
    	this->position.x += this->velocity.x;
		this->position.y += this->velocity.y;
        this->velocity.y += 0x3800;
		
        if (!this->CheckOnScreen(nullptr)) {
			this->Destroy();
		}

		if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 4 << 16, 0)) {
			this->Destroy();
		}
	}

	this->state.Run(this);
    this->dropperAnim.Run(this);
	this->machineAnim.Run(this);
    this->dropAnimator.Process();
}

void ChemicalDropper::LateUpdate() {}
void ChemicalDropper::StaticUpdate() {}
void ChemicalDropper::Draw()
{
    if (this->state.Matches(&ChemicalDropper::State_MoveRight) || this->state.Matches(&ChemicalDropper::State_GetChemicals) // object.state < CHEMICALDROPPER_DESTROYED
	|| this->state.Matches(&ChemicalDropper::State_DropChemicals) || this->state.Matches(&ChemicalDropper::State_MoveLeft)) {
       int32 dropperPos = this->position.x;
        if (this->direction == FLIP_NONE) {
            dropperPos -= this->dropperOffset;
        }
        else {
            dropperPos += this->dropperOffset;
        }

        Vector2 drawPos;
        drawPos.x = dropperPos;
        drawPos.y = this->position.y - 0x380000;
        if (this->dropperAnim.Matches(&ChemicalDropper::Dani_Filling)) {
            this->dropletAnimator.SetAnimation(sVars->aniFrames, AniDroplet, true, this->dropletFrame);
            this->dropletAnimator.DrawSprite(nullptr, false);
        }

		if (this->dropperAnim.Matches(&ChemicalDropper::Dani_Wait) || this->dropperAnim.Matches(&ChemicalDropper::Dani_Extend) 
		|| this->dropperAnim.Matches(&ChemicalDropper::Dani_Filling) || this->dropperAnim.Matches(&ChemicalDropper::Dani_OpenHatch)) {
            if (this->dropletAnimTimer < 32) {
				this->chemicalAnimator.SetAnimation(sVars->aniFrames, AniChemicals, true, this->chemicalFrame);
				this->chemicalAnimator.DrawSprite(&drawPos, false);
			}
		}

		dropperPos = this->position.x;
		if (this->direction == FLIP_NONE) {
            dropperPos -= this->dropperOffset;
		}
		else {
            dropperPos += this->dropperOffset;
		}
        drawPos.x = dropperPos;
        drawPos.y = this->position.y - 0x380000;
        this->dropperAnimator.SetAnimation(sVars->aniFrames, AniDropper, true, this->dropperFrame);
		this->dropperAnimator.DrawSprite(&drawPos, false); // dropper animation

		switch (this->hatchFrame) {
		case 0:
            this->hatchAnimator.SetAnimation(sVars->aniFrames, AniHatch, true, 0); // hatch closed
			this->hatchAnimator.DrawSprite(&drawPos, false);
			break;

		case 1:
            this->hatchAnimator.SetAnimation(sVars->aniFrames, AniHatch, true, 3); // opening 3
			this->hatchAnimator.DrawSprite(&drawPos, false);
			this->hatchAnimator.SetAnimation(sVars->aniFrames, AniHatch, true, 1); // opening 1
			this->hatchAnimator.DrawSprite(&drawPos, false);
			break;

		case 2:
			this->hatchAnimator.SetAnimation(sVars->aniFrames, AniHatch, true, 4); // opened
			this->hatchAnimator.DrawSprite(&drawPos, false);
			this->hatchAnimator.SetAnimation(sVars->aniFrames, AniHatch, true, 2); // opening 2
			this->hatchAnimator.DrawSprite(&drawPos, false);
			break;
		}

		Vector2 tubePos;
		tubePos.x = this->position.x;
		tubePos.y = this->position.y;
		if (this->direction == FLIP_NONE) {
			tubePos.x += 0x40000;
		}
		else {
			tubePos.x -= 0x40000;
		}

		int32 tubeNum = 0; // this is used for creating the correct amount of tube sprites based on the dropper length (while tubenum is less than the objects current dropper length)
		while (tubeNum < this->dropperLength) {
			drawPos.x = tubePos.x; // need these here so they can be constantly set to the new values in the loop
			drawPos.y = tubePos.y + 0x190000;
			if (tubeNum == this->dropletNodeID) { // if the current tubenum is the active node
				this->tubeAnimator.SetAnimation(sVars->aniFrames, AniTube, true, this->tubeFrame); // tube animation frames for the active node
			}
			else {
				this->tubeAnimator.SetAnimation(sVars->aniFrames, AniTube, true, 0); // idle tube for every other node
			}
            this->tubeAnimator.DrawSprite(&drawPos, false);
			tubePos.y += 0x80000; // adds to the y position for the tube for every new one created
            tubeNum++;
		}
		this->machineAnimator.SetAnimation(sVars->aniFrames, AniMachine, true, 0); 
		this->machineAnimator.DrawSprite(nullptr, false); // base chemical machine
	}
	else {
		this->debrisAnimator.DrawSprite(nullptr, false);
		if (this->type == Drop) {
			this->dropAnimator.SetAnimation(sVars->aniFrames, AniDrop, false, 0); 
		}
		else if (this->type == Splash) {
			this->dropAnimator.SetAnimation(sVars->aniFrames, AniSplash, false, 0); 
		}
		this->dropAnimator.DrawSprite(nullptr, false);
	}
}

void ChemicalDropper::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        if (data) { // this object will literally do nothing until its created by another object through code
            this->drawFX    = FX_FLIP;
			this->active    = ACTIVE_NORMAL;
            this->visible   = true;
            this->drawGroup = 2;
            this->state.Set(&ChemicalDropper::State_MoveRight);
            this->dropperAnim.Set(&ChemicalDropper::Dani_Closed);
            this->machineAnim.Set(&ChemicalDropper::Mani_Idle);
        }
    }
}

void ChemicalDropper::StageLoad()
{
	sVars->aniFrames.Load("CPZ/Boss.bin", SCOPE_STAGE);
    sVars->splashSFX.Get("Stage/ChemSplash.wav");
    sVars->dropHitbox.left   = -6;
    sVars->dropHitbox.top    = -6;
    sVars->dropHitbox.right  = 6;
    sVars->dropHitbox.bottom = 6;
}

void ChemicalDropper::StaticLoad(Static *sVars)
{
     RSDK_INIT_STATIC_VARS(ChemicalDropper);
	 
     int32 tubeFrames[] = { 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1 };
     memcpy(sVars->tubeFrames, tubeFrames, sizeof(tubeFrames));

	 int32 dropletFrames[] = { 17, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 6, 6 };
     memcpy(sVars->dropletFrames, dropletFrames, sizeof(dropletFrames));

	 int32 dropperFrames[] = { 0, 1, 2, 2 };
     memcpy(sVars->dropperFrames, dropperFrames, sizeof(dropperFrames));
}

void ChemicalDropper::SpawnDebris()
{
    ChemicalDropper *debris = GameObject::Create<ChemicalDropper>(INT_TO_VOID(true), this->debrisPos.x, this->debrisPos.y); // temp0 is debrispos.x and temp1 is debrispos.y
    debris->debrisAnimator.SetAnimation(sVars->aniFrames, 0, true, this->debrisFrame);
	debris->state.Set(&ChemicalDropper::State_DebrisExplode);
	debris->dropperAnim.Set(nullptr);
	debris->machineAnim.Set(nullptr);
	debris->main = this; // "this" is the main dropper, as the main dropper is what called this function
    debris->direction = this->direction;

	debris->velocity.x = (Math::Rand(debris->velocity.x, 8) - 4) << 16;
	debris->velocity.y = -0x38000;

	debris->timer = (Math::Rand(debris->timer, 255) + 30) & 0x7F;
}

void ChemicalDropper::Oscillate()
{
	this->position.y = BadnikHelpers::Oscillate(this, this->originPos.y, 2, 10);

	CPZEggman *eggman = GameObject::Get<CPZEggman>(sceneInfo->entitySlot - 1);
	eggman->position.x = this->position.x;
	eggman->position.y = this->position.y;
	eggman->direction = this->direction;
}

void ChemicalDropper::State_MoveRight()
{
	this->direction = FLIP_NONE;
	if (this->position.x == this->boundsR) {
        if (this->dropperAnim.Matches(&ChemicalDropper::Dani_Closed)) {
			this->machineAnim.Set(&ChemicalDropper::Mani_Extend); // review
			this->tubeAnimTimer = 0;
			this->state.Set(&ChemicalDropper::State_GetChemicals);
		}
	}
	else {
		if (this->position.x > this->boundsR) {
			this->position.x -= 0x30000;
			if (this->position.x <= this->boundsR) {
				this->position.x = this->boundsR;
			}
		}
		else {
			this->position.x += 0x30000;
			if (this->position.x >= this->boundsR) {
				this->position.x = this->boundsR;
			}
		}
	}
	ChemicalDropper::Oscillate();
}

void ChemicalDropper::State_GetChemicals() { ChemicalDropper::Oscillate(); }

void ChemicalDropper::State_DropChemicals()
{
	int32 dropPosition = this->position.x;
	// gonna assume object[0] is player
	Player *player = GameObject::Get<Player>(SLOT_PLAYER1);
	if (player->position.x <= this->position.x) {
		dropPosition -= 0x480000;
		this->direction = FLIP_NONE;
	}
	else {
		dropPosition += 0x480000;
		this->direction = FLIP_X;
	}
	dropPosition &= 0xFFFF0000;

	if ((player->position.x &= 0xFFFF0000) == dropPosition) {
		if (this->dropperOffset == 0x580000) {
			this->dropperAnim.Set(&ChemicalDropper::Dani_OpenHatch); // drops the chemicals when the player is in the same position as the drop
		}
	}
	else {
		if (player->position.x < dropPosition) {
			dropPosition = (Zone::sVars->cameraBoundsL[0] - 16) << 16; //stage.newXBoundary1;
;			if (this->position.x > dropPosition) {
				this->position.x -= 0x10000;
			}
		}
		else {
			dropPosition = (Zone::sVars->cameraBoundsR[0] + 16) << 16; //stage.newXBoundary2;
			if (this->position.x < dropPosition) {
				this->position.x += 0x10000;
			}
		}
	}
	ChemicalDropper::Oscillate();
}

void ChemicalDropper::State_MoveLeft()
{
	this->direction = FLIP_X;
	if (this->position.x == this->boundsL) {
		if (this->dropperAnim.Matches(&ChemicalDropper::Dani_Closed)) {
			this->machineAnim.Set(&ChemicalDropper::Mani_Extend);
			this->tubeAnimTimer = 0;
			this->state.Set(&ChemicalDropper::State_GetChemicals);
		}
	}
	else {
		if (this->position.x > this->boundsL) {
			this->position.x -= 0x30000;
			if (this->position.x <= this->boundsL) {
				this->position.x = this->boundsL;
			}
		}
		else {
			this->position.x += 0x30000;
			if (this->position.x >= this->boundsL) {
				this->position.x = this->boundsL;
			}
		}
	}
	ChemicalDropper::Oscillate();
}

void ChemicalDropper::State_Destroyed()
{
	this->machineAnim.Set(&ChemicalDropper::Mani_Idle);
	this->dropperAnim.Set(&ChemicalDropper::Dani_Closed);
	this->debrisPos.x = this->position.x;
	if (this->direction == 0) {
		this->debrisPos.x -= this->dropperOffset;
	}
	else {
		this->debrisPos.x += this->dropperOffset;
	}
	this->debrisPos.y = this->position.y;
	this->debrisPos.y -= 0x380000;
	this->debrisFrame = 15;
	ChemicalDropper::SpawnDebris();
	this->debrisPos.y += 0x80000;
	int32 dropperLength = ((this->dropperOffset >> 16) - 16) / 24;
	while (dropperLength >= 0) {
		if (this->direction == FLIP_NONE) {
			this->debrisPos.x += 0x180000;
		}
		else {
			this->debrisPos.x -= 0x180000;
		}
        this->debrisFrame = 34;
		ChemicalDropper::SpawnDebris();
		dropperLength--;
	}
	this->debrisPos.x = this->position.x;
	this->debrisPos.y = this->position.y;

	this->debrisFrame = 35;
	ChemicalDropper::SpawnDebris();

	this->debrisFrame = 36;
	ChemicalDropper::SpawnDebris();

	this->debrisFrame = 37;
	ChemicalDropper::SpawnDebris();

	if (this->direction == 0) {
		this->debrisPos.x += 0x40000;
	}
	else {
		this->debrisPos.x -= 0x40000;
	}
	this->debrisPos.y += 0x190000;
	int32 tubeNum = 0;
	while (tubeNum < this->dropperLength) {
        this->debrisFrame = 1;
		ChemicalDropper::SpawnDebris();
		this->debrisPos.y += 0x80000;
		tubeNum++;
	}
	main->Destroy(); // need this as "main" as i only want the original entity destroyed, not the debris pieces which are also dropper entities
}

void ChemicalDropper::State_DebrisExplode()
{
	this->timer--;
	if (this->timer <= 0) {
		this->timer = 30;
        Explosion *explosion = GameObject::Create<Explosion>(INT_TO_VOID(Explosion::Type2), this->position.x, this->position.y);
        explosion->drawGroup = 5;
        explosion->sVars->sfxExplosion.Play(false, 255);
		this->state.Set(&ChemicalDropper::State_DebrisDelay);
	}
}

void ChemicalDropper::State_DebrisDelay()
{
	this->timer--;
	if (this->timer <= 0) {
		this->state.Set(&ChemicalDropper::State_Debris);
	}
}

void ChemicalDropper::State_Debris()
{
	this->position.x += this->velocity.x;
	this->position.y += this->velocity.y;
	this->velocity.y += 0x3800;
    if (!this->CheckOnScreen(nullptr)) {
		this->Destroy();
	}
}

void ChemicalDropper::Drop_BubbleHit()
{
	Player *currentPlayer = GameObject::Get<Player>(SLOT_PLAYER1);
	if (currentPlayer->shield != Player::Shield_Bubble) {
		if (!currentPlayer->state.Matches(&Player::State_Death)) {
			currentPlayer->Hurt(this);
		}
	}
}

void ChemicalDropper::Drop_CreateSplash()
{
	this->velocity.y = -this->velocity.y; // flipsign
	sVars->splashSFX.Play(false, 255);
	int32 splashNum = 0;
	while (splashNum < 5) {
		ChemicalDropper *splash = GameObject::Create<ChemicalDropper>(INT_TO_VOID(true), this->position.x, this->position.y);
		splash->drawGroup = 2;
		splash->type = Splash;
		splash->state.Set(nullptr);
        splash->dropperAnim.Set(nullptr);
        splash->machineAnim.Set(nullptr);

		splash->velocity.x = Math::Rand(splash->velocity.x, 0x200);
		splash->velocity.x -= 0x100;
		splash->velocity.x <<= 10;
		if (splash->velocity.x < 0) {
			splash->velocity.x += 0x20000;
		}
		splash->velocity.x -= 0x10000;

		if (splashNum > 0) {
			splash->velocity.y = Math::Rand(splash->velocity.y, 0x7FE);
			splash->velocity.y -= 0x400;
			splash->velocity.y <<= 8;
		}
		splash->velocity.y += this->velocity.y / 2;
		splashNum++;
	}
    drop->Destroy(); // only want the original drop destroyed rather than all of the splashes too
}

// ANIMATIONS

void ChemicalDropper::Mani_Idle() {}

void ChemicalDropper::Mani_Extend()
{
	this->dropperLength++;
	if (this->dropperLength == 8) {
		this->machineAnim.Set(&ChemicalDropper::Mani_Filling);
		this->dropletNodeID = 8;
		this->tubeAnimTimer = 0;
		this->dropperAnim.Set(&ChemicalDropper::Dani_Filling);
		this->chemicalFrame = -1; // 1 frame behind the chemical animation is empty
	}
}

void ChemicalDropper::Mani_Filling()
{
	this->tubeAnimTimer++;
	if (this->tubeAnimTimer == 18) {
		this->tubeAnimTimer = 0;
		this->dropletNodeID--;
		if (this->dropletNodeID < 0) {
			this->dropletNodeID = 9;
		}
	}

	this->tubeFrame = sVars->tubeFrames[this->tubeAnimTimer]; // I JUST IMPROVISED THIS AND IT WORKED?? my idea was that its setting the tubeFrame to the tubeFrames table index based on the tubeAnimTimer
	//GetTableValue(object.tubeFrame, object.tubeAnimTimer, ChemicalDropper_tubeFrames);
}

void ChemicalDropper::Mani_Retract()
{
    this->dropperLength--;
	if (this->dropperLength == 0) {
		this->machineAnim.Set(&ChemicalDropper::Mani_Idle);
		this->dropletNodeID = 8;
		this->tubeAnimTimer = 0;
		this->dropperAnim.Set(&ChemicalDropper::Dani_Extend);
		this->state.Set(&ChemicalDropper::State_DropChemicals);
	}
}

void ChemicalDropper::Dani_Closed()
{
	this->dropperOffset = 0x100000;
	this->dropperFrame = 0;
}

void ChemicalDropper::Dani_Filling()
{
	this->dropletAnimTimer++;
	if (this->dropletAnimTimer == 19) {
		this->dropletAnimTimer = 0;
		this->chemicalFrame++;
		if (this->chemicalFrame >= 11) {
			this->chemicalFrame = 11;
			this->machineAnim.Set(&ChemicalDropper::Mani_Retract);
			this->dropperAnim.Set(&ChemicalDropper::Dani_Wait);
		}
	}
	this->dropletFrame = sVars->dropletFrames[this->dropletAnimTimer]; 
}

void ChemicalDropper::Dani_Wait() {}

void ChemicalDropper::Dani_Extend()
{
	this->dropperOffset += 0x10000;
	if (this->dropperOffset >= 0x580000) {
		this->dropperOffset = 0x580000;
	}
	int32 dropperFrameIndex = ((this->dropperOffset >> 16) - 16) / 24;
	this->dropperFrame = sVars->dropperFrames[dropperFrameIndex]; 
}

void ChemicalDropper::Dani_OpenHatch()
{
	this->dropletAnimTimer++;
	Vector2 dropperPos;
	dropperPos.x = this->position.x;
	dropperPos.y = this->position.y;
    switch (this->dropletAnimTimer) {
    case 8:
		this->hatchFrame = 1;
		break;

	case 20:
		this->hatchFrame = 2;
		break;

	case 32:
		if (this->direction == FLIP_NONE) {
			dropperPos.x -= this->dropperOffset;
			dropperPos.x += 0xC0000;
		}
		else {
			dropperPos.x += this->dropperOffset;
			dropperPos.x -= 0xC0000;
		}
		dropperPos.y -= 0x260000;
		break;

	case 40:
		this->hatchFrame = 1;
		break;

	case 48:
		this->hatchFrame = 0;
        this->dropperAnim.Set(&ChemicalDropper::Dani_Retract);
		this->dropletAnimTimer = 0;
		if (this->nextDir == 0) {
			this->state.Set(&ChemicalDropper::State_MoveLeft);
		}
		else {
			this->state.Set(&ChemicalDropper::State_MoveRight);
		}
		this->nextDir ^= 1;
		break;
	}
    if (this->dropletAnimTimer == 32) { // need to do this here in its own if statement cause the compiler complains if i have it in the switch
        ChemicalDropper *drop = GameObject::Create<ChemicalDropper>(INT_TO_VOID(true), dropperPos.x, dropperPos.y);
        drop->state.Set(nullptr);
        drop->dropperAnim.Set(nullptr);
        drop->machineAnim.Set(nullptr);
        drop->type            = Drop;
        drop->drawGroup       = 2;
		drop->drop = drop;
	}
}

void ChemicalDropper::Dani_Retract()
{
	this->dropperOffset -= 0x10000;
	if (this->dropperOffset <= 0x100000) {
        this->dropperOffset = 0x100000;
        this->dropperAnim.Set(&ChemicalDropper::Dani_Closed);
	}
	int32 dropperFrameIndex = ((this->dropperOffset >> 16) - 16) / 24;
	this->dropperFrame = sVars->dropperFrames[dropperFrameIndex]; 
}

#if RETRO_INCLUDE_EDITOR
void ChemicalDropper::EditorDraw() {}

void ChemicalDropper::EditorLoad() {}
#endif

void ChemicalDropper::Serialize() {}

} // namespace GameLogic