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
		
		ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];
		Vector2 range;
		range.x = screen->size.x << 16;
		range.y = screen->size.y << 16;
		if (!this->CheckOnScreen(&range)) {
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
    if (this->type == Splash) {
    	this->position.x += this->velocity.x;
		this->position.y += this->velocity.y;
        this->velocity.y += 0x3800;
		
		ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];
        Vector2 range;
        range.x = screen->size.x << 16;
        range.y = screen->size.y << 16;
        if (!this->CheckOnScreen(&range)) {
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
        int32 temp0 = this->position.x;
        if (this->direction == FLIP_NONE) {
            temp0 -= this->dropperOffset;
        }
        else {
            temp0 += this->dropperOffset;
        }

        int32 temp1 = this->position.y;
        temp1 -= 0x380000;
        Vector2 drawPos;
        drawPos.x = temp0;
        drawPos.y = temp1;
        if (this->dropperAnim.Matches(&ChemicalDropper::Dani_Filling)) {
            this->dropletAnimator.SetAnimation(sVars->aniFrames, 0, true, this->dropletFrame);
            this->dropletAnimator.DrawSprite(nullptr, false);
            this->chemicalAnimator.SetAnimation(sVars->aniFrames, 0, true, this->chemicalFrame);
            this->chemicalAnimator.DrawSprite(&drawPos, false);
        }

		if (this->dropperAnim.Matches(&ChemicalDropper::Dani_Wait)) {
            this->chemicalAnimator.SetAnimation(sVars->aniFrames, 0, true, this->chemicalFrame);
            this->chemicalAnimator.DrawSprite(&drawPos, false);
		}

		if (this->dropperAnim.Matches(&ChemicalDropper::Dani_Extend)) {
            this->chemicalAnimator.SetAnimation(sVars->aniFrames, 0, true, this->chemicalFrame);
            this->chemicalAnimator.DrawSprite(&drawPos, false);
		}

		if (this->dropperAnim.Matches(&ChemicalDropper::Dani_OpenHatch)) {
            if (this->dropletAnimTimer < 32) {
                this->chemicalAnimator.SetAnimation(sVars->aniFrames, 0, true, this->chemicalFrame);
                this->chemicalAnimator.DrawSprite(&drawPos, false);
            }
		}
		temp0 = this->position.x;
		if (this->direction == FLIP_NONE) {
            temp0 -= this->dropperOffset;
		}
		else {
            temp0 += this->dropperOffset;
		}
		temp1 = this->position.y;
		temp1 -= 0x380000;
        drawPos.x = temp0;
        drawPos.y = temp1;
        this->dropperAnimator.SetAnimation(sVars->aniFrames, 0, true, this->dropperFrame);
		this->dropperAnimator.DrawSprite(&drawPos, false);
		//DrawSpriteFX(object.dropperFrame, FX_FLIP, temp0, temp1);

		switch (this->hatchFrame) {
		case 0:
			//DrawSpriteFX(18, FX_FLIP, temp0, temp1); empty frame ?
            this->hatchAnimator.SetAnimation(sVars->aniFrames, 0, true, 13); // fix animationID // hatch closed
			this->hatchAnimator.DrawSprite(&drawPos, false);
			//DrawSpriteFX(13, FX_FLIP, temp0, temp1); // hatch closed frame
			break;

		case 1:
            this->hatchAnimator.SetAnimation(sVars->aniFrames, 0, true, 16); // fix animationID // opening 3
			this->hatchAnimator.DrawSprite(&drawPos, false);
			this->hatchAnimator.SetAnimation(sVars->aniFrames, 0, true, 14); // fix animationID // opening 1
			this->hatchAnimator.DrawSprite(&drawPos, false);
			//DrawSpriteFX(16, FX_FLIP, temp0, temp1);
			//DrawSpriteFX(14, FX_FLIP, temp0, temp1);
			break;

		case 2:
			this->hatchAnimator.SetAnimation(sVars->aniFrames, 0, true, 17); // fix animationID // opened
			this->hatchAnimator.DrawSprite(&drawPos, false);
			this->hatchAnimator.SetAnimation(sVars->aniFrames, 0, true, 15); // fix animationID // opening 2
			this->hatchAnimator.DrawSprite(&drawPos, false);
			//DrawSpriteFX(17, FX_FLIP, temp0, temp1);
			//DrawSpriteFX(15, FX_FLIP, temp0, temp1);
			break;
		}

		temp0 = this->position.x;
		if (this->direction == FLIP_NONE) {
			temp0 += 0x40000;
		}
		else {
			temp0 -= 0x40000;
		}

		temp1 = this->position.y;
		temp1 += 0x190000;
		int32 temp2 = 0;
		while (temp2 < this->dropperLength) {
			drawPos.x = temp0; // need these here so they can be constantly set to the new values in the loop
			drawPos.y = temp1;
			if (temp2 == this->dropletNodeID) {
				this->tubeAnimator.SetAnimation(sVars->aniFrames, 0, true, this->tubeFrame); // fix animationID
				this->tubeAnimator.DrawSprite(&drawPos, false);
				//DrawSpriteFX(object.tubeFrame, FX_FLIP, temp0, temp1);
			}
			else {
				this->tubeAnimator.SetAnimation(sVars->aniFrames, 0, true, 1); // fix animationID // tube normal
				this->tubeAnimator.DrawSprite(&drawPos, false);
				//DrawSpriteFX(1, FX_FLIP, temp0, temp1);
			}
			temp1 += 0x80000;
			temp2++;
		}
		this->machineAnimator.SetAnimation(sVars->aniFrames, 0, true, 0); 
		this->machineAnimator.DrawSprite(nullptr, false); // base chemical machine
		//DrawSpriteFX(0, FX_FLIP, object.xpos, object.ypos);
	}
	else {
		this->debrisAnimator.DrawSprite(nullptr, false);
		if (this->type == Drop) {
			this->dropAnimator.SetAnimation(sVars->aniFrames, 13, false, 0); 
			this->dropAnimator.DrawSprite(nullptr, false);
		}

		if (this->type == Splash) {
			this->dropAnimator.SetAnimation(sVars->aniFrames, 14, true, 0); 
			this->dropAnimator.DrawSprite(nullptr, false);
		}
		//DrawSpriteFX(object.frame, FX_FLIP, object.xpos, object.ypos); ???
	}
}

void ChemicalDropper::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        if (data) {
            this->drawFX    = FX_FLIP;
            this->visible   = true;
            this->drawGroup = 4;
            this->state.Set(&ChemicalDropper::State_MoveRight);
            this->dropperAnim.Set(&ChemicalDropper::Dani_Closed);
            this->machineAnim.Set(&ChemicalDropper::Mani_Idle);
        }
    }
}

void ChemicalDropper::StageLoad()
{
	sVars->aniFrames.Load("CPZ/Boss.bin", SCOPE_STAGE);
    sVars->splashSFX.Get(""); // fix
    sVars->dropHitbox.left   = -6;
    sVars->dropHitbox.top    = -6;
    sVars->dropHitbox.right  = 6;
    sVars->dropHitbox.bottom = 6;
}

void ChemicalDropper::StaticLoad(Static *sVars)
{
     RSDK_INIT_STATIC_VARS(ChemicalDropper);
	 
     int32 tubeFrames[] = { 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2 };
     memcpy(sVars->tubeFrames, tubeFrames, sizeof(tubeFrames));

	 int32 dropletFrames[] = { 18, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 9, 10, 10, 10 };
     memcpy(sVars->dropletFrames, dropletFrames, sizeof(dropletFrames));

	 int32 dropperFrames[] = { 12, 31, 33 };
     memcpy(sVars->dropperFrames, dropperFrames, sizeof(dropperFrames));

	 // unused 
	 int32 hatchFrames[] = { 13, 18, 14, 16, 15, 17 };
     memcpy(sVars->hatchFrames, hatchFrames, sizeof(hatchFrames));
}

void ChemicalDropper::SpawnDebris()
{
	//CreateTempObject(TypeName[Chemical Dropper], 0, temp0, temp1)
    ChemicalDropper *debris = GameObject::Create<ChemicalDropper>(INT_TO_VOID(true), this->debrisPos.x, this->debrisPos.y); // temp0 is debrispos.x and temp1 is debrispos.y
	debris->active = ACTIVE_NORMAL;
    debris->debrisAnimator.SetAnimation(sVars->aniFrames, 0, true, this->debrisFrame);
	debris->state.Set(&ChemicalDropper::State_DebrisExplode);
	debris->dropperAnim.Set(nullptr);
	debris->machineAnim.Set(nullptr);
	debris->main = this; // "this" is the main dropper, as the main dropper is what called this function
    debris->direction = this->direction;

	//Rand(object[tempObjectPos].xvel, 8)
	debris->velocity.x = Math::Rand(debris->velocity.x, 8);
	debris->velocity.x -= 4;
	debris->velocity.x <<= 16;
	debris->velocity.y = -0x38000;

	//Rand(object[tempObjectPos].timer, 255)
	debris->timer = Math::Rand(debris->timer, 255);
	debris->timer += 30;
	debris->timer &= 0x7F;
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
	int32 temp0 = this->position.x;
	// gonna assume object[0] is player
	Player *player = GameObject::Get<Player>(SLOT_PLAYER1);
	if (player->position.x <= this->position.x) {
		temp0 -= 0x480000;
		this->direction = FLIP_NONE;
	}
	else {
		temp0 += 0x480000;
		this->direction = FLIP_X;
	}
	temp0 &= 0xFFFF0000;

	int32 temp1 = player->position.x;
	temp1 &= 0xFFFF0000;

	if (temp1 == temp0) {
		if (this->dropperOffset == 0x580000) {
			this->dropperAnim.Set(&ChemicalDropper::Dani_OpenHatch); // review
		}
	}
	else {
		if (player->position.x < temp0) {
			temp0 = Zone::sVars->cameraBoundsL[0]; //stage.newXBoundary1;
			temp0 -= 16;
			temp0 <<= 16;
;			if (this->position.x > temp0) {
				this->position.x -= 0x10000;
			}
		}
		else {
			temp0 = Zone::sVars->cameraBoundsR[0]; //stage.newXBoundary2;
			temp0 += 16;
			temp0 <<= 16;
			if (this->position.x < temp0) {
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
	this->debrisFrame = 35; // review if this debrisframe method works
	ChemicalDropper::SpawnDebris();
	this->debrisPos.y += 0x80000;
	int32 temp2 = this->dropperOffset;
	temp2 >>= 16;
	temp2 -= 16;
	temp2 /= 24;
	while (temp2 >= 0) {
		if (this->direction == FLIP_NONE) {
			this->debrisPos.x += 0x180000;
		}
		else {
			this->debrisPos.x -= 0x180000;
		}
        this->debrisFrame = 36;
		ChemicalDropper::SpawnDebris();
		temp2--;
	}
	this->debrisPos.x = this->position.x;
	this->debrisPos.y = this->position.y;

	this->debrisFrame = 37;
	ChemicalDropper::SpawnDebris();

	this->debrisFrame = 38;
	ChemicalDropper::SpawnDebris();

	this->debrisFrame = 39;
	ChemicalDropper::SpawnDebris();

	if (this->direction == 0) {
		this->debrisPos.x += 0x40000;
	}
	else {
		this->debrisPos.x -= 0x40000;
	}
	this->debrisPos.y += 0x190000;
	temp2 = 0;
	while (temp2 < this->dropperLength) {
        this->debrisFrame = 1;
		ChemicalDropper::SpawnDebris();
		this->debrisPos.y += 0x80000;
		temp2++;
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
    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];
    Vector2 range;
    range.x = screen->size.x << 16;
    range.y = screen->size.y << 16;
    if (!this->CheckOnScreen(&range)) {
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
	int32 temp1 = this->velocity.y;
	temp1 >>= 1;
	temp1 = -temp1; // flipsign
	sVars->splashSFX.Play(false, 255);
	int32 temp0 = 0;
	while (temp0 < 5) {
		ChemicalDropper *splash = GameObject::Create<ChemicalDropper>(INT_TO_VOID(true), this->position.x, this->position.y);
		//CreateTempObject(TypeName[Chemical Splash], 0, object.xpos, object.ypos)
		splash->drawGroup = 2;
		splash->active = ACTIVE_NORMAL;
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

		if (temp0 > 0) {
			splash->velocity.y = Math::Rand(splash->velocity.y, 0x7FE);
			splash->velocity.y -= 0x400;
			splash->velocity.y <<= 8;
		}
		splash->velocity.y += temp1;

		temp0++;
	}
    drop->Destroy(); // only want the original drop destroyed rather than all of the splashes too
}

// ANIMATIONS

void ChemicalDropper::Mani_Idle() {}

void ChemicalDropper::Mani_Extend()
{
	this->tubeFrame = 1;
	this->dropperLength++;
	if (this->dropperLength == 8) {
		this->machineAnim.Set(&ChemicalDropper::Mani_Filling);
		this->dropletNodeID = 8;
		this->tubeAnimTimer = 0;
		this->dropperAnim.Set(&ChemicalDropper::Dani_Filling);
		this->chemicalFrame = 18;
		this->dropletFrame = 4;
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
	this->tubeFrame = 1;
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
	this->dropperFrame = 12;
}

void ChemicalDropper::Dani_Filling()
{
	this->dropletAnimTimer++;
	if (this->dropletAnimTimer == 19) {
		this->dropletAnimTimer = 0;
		this->chemicalFrame++;
		if (this->chemicalFrame >= 30) {
			this->chemicalFrame = 30;
			this->machineAnim.Set(&ChemicalDropper::Mani_Retract);
			this->dropperAnim.Set(&ChemicalDropper::Dani_Wait);
		}
	}
	this->dropletFrame = sVars->dropletFrames[this->dropletAnimTimer]; 
	//GetTableValue(object.dropletFrame, object.dropletAnimTimer, ChemicalDropper_dropletFrames);
}

void ChemicalDropper::Dani_Wait() {}

void ChemicalDropper::Dani_Extend()
{
	this->dropperOffset += 0x10000;
	if (this->dropperOffset >= 0x580000) {
		this->dropperOffset = 0x580000;
	}
	int32 temp0 = this->dropperOffset;
	temp0 >>= 16;
	temp0 -= 16;
	temp0 /= 24;
	this->dropperFrame = sVars->dropperFrames[temp0]; 
	// weird workaround way of dealing with an issue where the above line is reading invalid data for some reason even tho this is exactly how it was in v4
	// so whenever it hits frame 13 (this is the frame that gets set wrong due to the invalid data) it just sets it to the correct frame instead (that being 33)
	if (this->dropperFrame == 13) { 
		this->dropperFrame = 33;    
	}
	//GetTableValue(object.dropperFrame, temp0, ChemicalDropper_dropperFrames);
}

void ChemicalDropper::Dani_OpenHatch()
{
	this->dropletAnimTimer++;
	int32 temp0 = this->position.x;
	int32 temp1 = this->position.y;
    switch (this->dropletAnimTimer) {
    case 8:
		this->hatchFrame = 1;
		break;

	case 20:
		this->hatchFrame = 2;
		break;

	case 32:
		if (this->direction == FLIP_NONE) {
			temp0 -= this->dropperOffset;
			temp0 += 0xC0000;
		}
		else {
			temp0 += this->dropperOffset;
			temp0 -= 0xC0000;
		}
		temp1 -= 0x260000;
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
        // CreateTempObject(TypeName[Chemical Drop], 0, temp0, temp1);
        ChemicalDropper *drop = GameObject::Create<ChemicalDropper>(INT_TO_VOID(true), temp0, temp1);
        drop->active          = ACTIVE_NORMAL;
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
	
	int32 temp0 = this->dropperOffset;
	temp0 >>= 16;
	temp0 -= 16;
	temp0 /= 24;
	this->dropperFrame = sVars->dropperFrames[temp0]; 
}

#if RETRO_INCLUDE_EDITOR
void ChemicalDropper::EditorDraw() {}

void ChemicalDropper::EditorLoad() {}
#endif

void ChemicalDropper::Serialize() {}

} // namespace GameLogic