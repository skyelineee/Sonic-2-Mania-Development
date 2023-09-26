// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Grounder Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Grounder.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Grounder);

void Grounder::Update()
{
    if (!this->isBrick) {
        if (!this->CheckOnScreen(nullptr)) {
            Vector2 storedPos;
            storedPos.x      = this->position.x;
            storedPos.y      = this->position.y;
            this->position.x = this->startPos.x;
            this->position.y = this->startPos.y;
            if (!this->CheckOnScreen(nullptr)) {
                this->state.Set(&Grounder::State_Hidden);
                this->timer = 0;
                this->active = ACTIVE_BOUNDS;
            }
            else {
                this->position.x = storedPos.x;
                this->position.y = storedPos.y;
            }
        }

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik)) {
                player->CheckBadnikBreak(this, true);
            }
        }
    }

	if (this->isBrick) {
	 	this->position.x += this->velocity.x;
		this->position.y += this->velocity.y;
		this->velocity.y += 0x3800;

		if (!this->CheckOnScreen(nullptr)) {
            this->Destroy();
		}
	}

	this->state.Run(this);
    this->animator.Process();
}

void Grounder::LateUpdate() {}
void Grounder::StaticUpdate() {}
void Grounder::Draw()
{
	if (this->state.Matches(&Grounder::State_Hidden)) {
        Vector2 drawPos;
		drawPos.x = this->position.x;
		drawPos.y = this->position.y;
		
		// draws the bricks when the grounder is still hidden
		drawPos.y -= 0x140000;
		this->brickAnimator.SetAnimation(sVars->aniFrames, Brick, true, 0);
		this->brickAnimator.DrawSprite(&drawPos, false);

		drawPos.y += 0x100000;
		drawPos.x -= 0x100000;
		this->brickAnimator.SetAnimation(sVars->aniFrames, Brick, true, 0);
		this->brickAnimator.DrawSprite(&drawPos, false);

		drawPos.x += 0x200000;
		this->brickAnimator.SetAnimation(sVars->aniFrames, Brick, true, 0);
		this->brickAnimator.DrawSprite(&drawPos, false);

		drawPos.y += 0x100000;
		drawPos.x -= 0x100000;
		this->brickAnimator.SetAnimation(sVars->aniFrames, Brick, true, 0);
		this->brickAnimator.DrawSprite(&drawPos, false);
	}
	else {
		this->animator.DrawSprite(nullptr, false);
	}

	if (this->isBrick) {
		this->animator.SetAnimation(sVars->aniFrames, Brick, true, this->brickFrame);
        this->animator.DrawSprite(nullptr, false);
	}
}

void Grounder::Create(void *data)
{
	if (!sceneInfo->inEditor) {
        if (data) {
            this->active    = ACTIVE_NORMAL;
            this->drawGroup = 3;
            this->visible   = true;
            this->isBrick   = true;
        }
        else {
            this->state.Set(&Grounder::State_Hidden);
            this->active    = ACTIVE_BOUNDS;
            this->drawFX    = FX_FLIP;
            this->drawGroup = 3;
            this->visible   = true;

            this->startPos = this->position;
        }
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
	}
}

void Grounder::StageLoad()
{
	sVars->aniFrames.Load("ARZ/Grounder.bin", SCOPE_STAGE);
	sVars->sfxLedgeBreak.Get("Stage/LedgeBreak.wav");

    sVars->hitboxBadnik.left   = -12;
    sVars->hitboxBadnik.top    = -20;
    sVars->hitboxBadnik.right  = 12;
    sVars->hitboxBadnik.bottom = 20;

	sVars->hiddenHitbox.left   = -96;
    sVars->hiddenHitbox.top    = -256;
    sVars->hiddenHitbox.right  = 96;
    sVars->hiddenHitbox.bottom = 256;

	DebugMode::AddObject(sVars->classID, &Grounder::DebugSpawn, &Grounder::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Grounder::DebugSpawn()
{
	Grounder *grounder = GameObject::Create<Grounder>(nullptr, this->position.x, this->position.y); 
	grounder->type     = 1;

}

void Grounder::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, Moving, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Grounder::State_Hidden()
{
    this->active = ACTIVE_NORMAL;
	if (this->type == 1) {
        this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 20 << 16, 8);
		this->state.Set(&Grounder::State_Appear);
		this->timer = 17;
	}

	for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
		if (currentPlayer->CheckCollisionTouch(this, &sVars->hiddenHitbox)) {
            Grounder *brick = GameObject::Create<Grounder>(INT_TO_VOID(true), this->position.x, this->position.y);
			brick->velocity.x = -0x10000;
			brick->velocity.y = -0x40000;
			brick->brickFrame = 1;
			
			Grounder *brick2 = GameObject::Create<Grounder>(INT_TO_VOID(true), this->position.x, this->position.y);
			brick2->velocity.x = 0x40000;
			brick2->velocity.y = -0x30000;
			brick2->brickFrame = 3;
			
			Grounder *brick3 = GameObject::Create<Grounder>(INT_TO_VOID(true), this->position.x, this->position.y);
			brick3->velocity.x = 0x20000;
			brick3->velocity.y = 0;
			brick3->brickFrame = 1;

			Grounder *brick4 = GameObject::Create<Grounder>(INT_TO_VOID(true), this->position.x, this->position.y);
			brick4->velocity.x = -0x30000;
			brick4->velocity.y = -0x10000;
			brick4->brickFrame = 2;

			Grounder *brick5 = GameObject::Create<Grounder>(INT_TO_VOID(true), this->position.x, this->position.y);
			brick5->velocity.x = -0x30000;
			brick5->velocity.y = -0x30000;
			brick5->brickFrame = 1;

			Grounder *brick6 = GameObject::Create<Grounder>(INT_TO_VOID(true), this->position.x, this->position.y);
			brick6->position.y -= 0x140000;
			brick6->velocity.x = 0x10000;
			brick6->velocity.y = -0x20000;
			brick6->brickFrame = 0;
			
			Grounder *brick7 = GameObject::Create<Grounder>(INT_TO_VOID(true), this->position.x, this->position.y);
			brick7->position.x += 0x100000;
			brick7->position.y -= 0x40000;
			brick7->velocity.x = 0x10000;
			brick7->velocity.y = -0x10000;
			brick7->brickFrame = 0;
			
			Grounder *brick8 = GameObject::Create<Grounder>(INT_TO_VOID(true), this->position.x, this->position.y);
			brick8->position.y += 0xC0000;
			brick8->velocity.x = -0x10000;
			brick8->velocity.y = -0x20000;
			brick8->brickFrame = 0;
			
			Grounder *brick9 = GameObject::Create<Grounder>(INT_TO_VOID(true), this->position.x, this->position.y);
			brick9->position.x -= 0x100000;
			brick9->position.y -= 0x40000;
			brick9->velocity.x = -0x10000;
			brick9->velocity.y = -0x10000;
			brick9->brickFrame = 0;

			this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 20 << 16, 8);
			this->animator.SetAnimation(sVars->aniFrames, Appear, false, 0);
			this->state.Set(&Grounder::State_Appear);
		}
	}
}

void Grounder::State_Appear() 
{
	this->timer++;
	if (this->timer == 8) {
		sVars->sfxLedgeBreak.Play();
	}

	if (this->timer == 18) {
		this->timer = 0;
		this->animator.SetAnimation(sVars->aniFrames, Moving, false, 0);
		this->state.Set(&Grounder::State_Moving);

		this->targetPlayerDistance = 0x7FFFFFFF;
		this->targetPlayer = 0;
		for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
			int32 distance = currentPlayer->position.x;
			distance -= this->position.x;
			int32 dir = FLIP_NONE;
			if (distance < 0) {
                distance = -distance;
			}
			else {
				dir = FLIP_X;
			}

			if (distance < this->targetPlayerDistance) {
				this->targetPlayerDistance = distance;
				this->targetPlayer 		   = currentPlayer;
				this->direction 		   = dir;
			}
		}

		if (this->direction == 0) {
			this->velocity.x = -0x10000;
		}
		else {
			this->velocity.x = 0x10000;
		}
	}
}

void Grounder::State_Moving()
{
	this->position.x += this->velocity.x;

	if (!this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 20 << 16, 8)) {
		this->timer = 60;
		this->state.Set(&Grounder::State_Turning);
	}
}

void Grounder::State_Turning()
{
	this->animator.SetAnimation(sVars->aniFrames, Turning, false, 0);
	this->timer--;
	if (this->timer == 0) {
		this->direction ^= FLIP_X;
		this->velocity.x = -this->velocity.x;
        this->animator.SetAnimation(sVars->aniFrames, Moving, false, 0);
		this->state.Set(&Grounder::State_Moving);
	}
}

#if RETRO_INCLUDE_EDITOR
void Grounder::EditorDraw() {}

void Grounder::EditorLoad() 
{
	sVars->aniFrames.Load("ARZ/Grounder.bin", SCOPE_STAGE);

	RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Start Hidden");
    RSDK_ENUM_VAR("Not Hidden");
}
#endif

void Grounder::Serialize() { RSDK_EDITABLE_VAR(Grounder, VAR_INT32, type); }

} // namespace GameLogic