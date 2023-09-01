// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ChopChop Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "ChopChop.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Common/Water.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ChopChop);

void ChopChop::Update()
{
    if (!this->CheckOnScreen(nullptr)) {
        Vector2 storedPos;
        storedPos.x      = this->position.x;
        storedPos.y      = this->position.y;
        this->position.x = this->startPos.x;
		this->position.y = this->startPos.y;
        if (!this->CheckOnScreen(nullptr)) {
			ChopChop::Setup();
		}
		else {
			this->position.x = storedPos.x;
			this->position.y = storedPos.y;
		}
	}

	if (this->bubbleTimer == 0) {
		//CreateTempObject(TypeName[Air Bubble], 2, object.xpos, object.ypos);
		Water *bubble = GameObject::Create<Water>(Water::Bubble, this->position.x, this->position.y);
		if (this->direction) {
		    bubble->position.x -= TO_FIXED(6);
		    bubble->angle = 0x100;
		}
		else {
		    bubble->position.x += TO_FIXED(6);
		}

		bubble->childPtr   = this;
		bubble->bubbleX    = bubble->position.x;
		bubble->velocity.y = -0x8800;
		bubble->drawGroup  = this->drawGroup + 1;
	}

	for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->CheckBadnikTouch(this, &sVars->hitboxBadnik)) {
            player->CheckBadnikBreak(this, true);
        }
    }

    this->state.Run(this);
    this->animator.Process();
}

void ChopChop::LateUpdate() {}
void ChopChop::StaticUpdate() {}
void ChopChop::Draw() { this->animator.DrawSprite(nullptr, false); }

void ChopChop::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        ChopChop::Setup();
        this->state.Set(&ChopChop::State_Swimming);
        this->active    = ACTIVE_BOUNDS;
        this->drawFX    = FX_FLIP;
        this->drawGroup = 3;
        this->visible   = true;

        this->startPos = this->position;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
    }
}

void ChopChop::StageLoad()
{
    sVars->aniFrames.Load("ARZ/ChopChop.bin", SCOPE_STAGE);

	sVars->hitboxBadnik.left   = -12;
    sVars->hitboxBadnik.top    = -16;
    sVars->hitboxBadnik.right  = 12;
    sVars->hitboxBadnik.bottom = 16;

	sVars->hitboxRange.left   = -160;
    sVars->hitboxRange.top    = -32;
    sVars->hitboxRange.right  = 16;
    sVars->hitboxRange.bottom = 32;

	DebugMode::AddObject(sVars->classID, &ChopChop::DebugSpawn, &ChopChop::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void ChopChop::DebugSpawn() { GameObject::Create<ChopChop>(nullptr, this->position.x, this->position.y); }

void ChopChop::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, Swim, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void ChopChop::Setup()
{
	if (this->direction == FLIP_NONE) {
		this->velocity.x = -0x4000;
	}
	else {
		this->velocity.x = 0x4000;
	}

	this->velocity.y = 0;
	this->timer = 512;
	this->bubbleTimer = 0;
	this->animator.SetAnimation(sVars->aniFrames, Swim, false, 0);
}

void ChopChop::State_Swimming()
{
    this->active = ACTIVE_NORMAL;
	this->position.x += this->velocity.x;

	this->timer--;
	if (this->timer == 0) {
		this->direction ^= FLIP_X;
		this->velocity.x = -this->velocity.x;
		this->timer = 512;
	}

	for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
		if (this->state.Matches(&ChopChop::State_Swimming)) {
			if (currentPlayer->CheckCollisionTouch(this, &sVars->hitboxRange)) {
				this->state.Set(&ChopChop::State_AttackDelay);
                this->animator.SetAnimation(sVars->aniFrames, Attack, false, 0);
				this->timer = 16;
				if (currentPlayer->position.x < this->position.x) {
					this->velocity.x = -0x20000;
				}
				else {
					this->velocity.x = 0x20000;
				}

				int32 inRange = this->position.y;
				inRange -= currentPlayer->position.y;
				abs(inRange);
				if (inRange > 0x100000) {
					if (currentPlayer->position.y < this->position.y) {
						this->velocity.y = -0x8000;
					}
					else {
						this->velocity.y = 0x8000;
					}
				}
			}
		}
	}

	this->bubbleTimer++;
	if (this->bubbleTimer >= 240) {
		this->bubbleTimer = 0;
	}
}

void ChopChop::State_AttackDelay() 
{
	this->timer--;
	if (this->timer < 0) {
		this->state.Set(&ChopChop::State_Attack);
	}
}

void ChopChop::State_Attack() 
{
	this->position.x += this->velocity.x;
	this->position.y += this->velocity.y;

	this->bubbleTimer++;
    if (this->bubbleTimer >= 20) {
        this->bubbleTimer = 0;
    }
}

#if RETRO_INCLUDE_EDITOR
void ChopChop::EditorDraw() {}

void ChopChop::EditorLoad()
{
    sVars->aniFrames.Load("ARZ/ChopChop.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Left");
    RSDK_ENUM_VAR("Right");
}
#endif

void ChopChop::Serialize() { RSDK_EDITABLE_VAR(ChopChop, VAR_UINT8, direction); }

} // namespace GameLogic