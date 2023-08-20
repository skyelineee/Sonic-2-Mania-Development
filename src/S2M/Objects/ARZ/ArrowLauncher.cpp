// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ArrowLauncher Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "ArrowLauncher.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ArrowLauncher);

void ArrowLauncher::Update()
{
    if (this->isArrow) {
        if (this->timer > 0) {
            this->timer--;
            if (this->timer == 15) {
                // PlaySfx(SfxName[Small Fireball], false);
            }
        }
		
        if (this->timer < 16) {
            this->position.x += this->velocity.x;
        }
		
        if (this->timer < 12) {
            if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 16 << 16, 0, 2)) {
                this->Destroy();
            }
        }
		
        for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (currentPlayer->CheckCollisionTouch(this, &sVars->arrowHitbox)) {
                currentPlayer->Hurt(this);
            }
        }
		
        if (!this->CheckOnScreen(nullptr)) {
            this->Destroy();
        }
    }

	this->state.Run(this);
    this->animator.Process();
}

void ArrowLauncher::LateUpdate() {}
void ArrowLauncher::StaticUpdate() {}
void ArrowLauncher::Draw() 
{ 
	if (!this->isArrow) {
		this->animator.DrawSprite(nullptr, false); 
	}
    else {
		this->arrowAnimator.SetAnimation(sVars->aniFrames, Arrow, true, 0);
		this->arrowAnimator.DrawSprite(nullptr, false);
	}
}

void ArrowLauncher::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        if (data) {
            this->active    = ACTIVE_NORMAL;
            this->drawGroup = 3;
			this->drawFX    = FX_FLIP;
            this->visible   = true;
            this->isArrow   = true;
        }
        else {
            this->state.Set(&ArrowLauncher::State_AwaitPlayer);
            this->active    = ACTIVE_BOUNDS;
            this->drawFX    = FX_FLIP;
            this->drawGroup = 7;
            this->visible   = true;
        }
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
    }
}

void ArrowLauncher::StageLoad()
{
    sVars->aniFrames.Load("ARZ/ArrowLauncher.bin", SCOPE_STAGE);

	sVars->arrowHitbox.left   = -8;
    sVars->arrowHitbox.top    = -4;
    sVars->arrowHitbox.right  = 8;
    sVars->arrowHitbox.bottom = 4;

    sVars->idleHitbox.left   = -64;
    sVars->idleHitbox.top    = -256;
    sVars->idleHitbox.right  = 64;
    sVars->idleHitbox.bottom = 256;
}

void ArrowLauncher::State_AwaitPlayer()
{
    this->animator.SetAnimation(sVars->aniFrames, Idle, false, 0);
	for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
		if (currentPlayer->CheckCollisionTouch(this, &sVars->idleHitbox)) {
			this->state.Set(&ArrowLauncher::State_Activate);
		}
	}
}

void ArrowLauncher::State_Activate()
{
	this->animator.SetAnimation(sVars->aniFrames, Detected, false, 0);

	bool32 inHitbox = false;
	for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
		inHitbox += currentPlayer->CheckCollisionTouch(this, &sVars->idleHitbox);
	}

	if (inHitbox == false) {
		this->state.Set(&ArrowLauncher::State_FireArrow);
		this->timer = 0;
		this->animator.SetAnimation(sVars->aniFrames, Launch, false, 0);
		//PlaySfx(SfxName[Swish], false);
	}
}

void ArrowLauncher::State_FireArrow()
{
	this->timer++;
	switch (this->timer) {
		case 8:
			this->animator.SetAnimation(sVars->aniFrames, Open, false, 0);
			break;

		case 24:
			this->animator.SetAnimation(sVars->aniFrames, Shut, false, 0);
			break;

		case 32:
			this->timer = 0;
			this->animator.SetAnimation(sVars->aniFrames, Idle, false, 0);
			this->state.Set(&ArrowLauncher::State_AwaitPlayer);
			break;
	}

	if (this->timer == 8) { // need to create the arrow here because the compiler complains if i do it in the switch
		//CreateTempObject(TypeName[Arrow], 0, object.xpos, object.ypos);
		ArrowLauncher *arrow = GameObject::Create<ArrowLauncher>(INT_TO_VOID(true), this->position.x, this->position.y);
		if (this->direction == 1) {
			arrow->velocity.x = -0x40000;
		}
		else {
			arrow->velocity.x = 0x40000;
		}
		arrow->timer = 24;
		arrow->drawPosX = this->position.x;;
		arrow->direction = this->direction;
	}
}

#if RETRO_INCLUDE_EDITOR
void ArrowLauncher::EditorDraw() {}

void ArrowLauncher::EditorLoad()
{
	sVars->aniFrames.Load("ARZ/ArrowLauncher.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Right");
}
#endif

void ArrowLauncher::Serialize() { RSDK_EDITABLE_VAR(ArrowLauncher, VAR_UINT8, direction); }

} // namespace GameLogic