// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Crawl Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Crawl.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Crawl);

void Crawl::Update()
{
    if (!this->CheckOnScreen(nullptr)) {
		int32 temp0 = this->position.x;
		int32 temp1 = this->position.y;
		this->position.x = this->startPos.x;
		this->position.y = this->startPos.y;
		if (!this->CheckOnScreen(nullptr)) {
			if (this->active != 4) {
				this->active = ACTIVE_BOUNDS;
			}
			this->state.Set(&Crawl::State_Setup);
		}
		else {
			this->position.x = temp0;
			this->position.y = temp1;
		}
	}

	for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
		if (currentPlayer->CheckCollisionTouch(this, &sVars->hitboxBadnik)) {
			currentPlayer->CheckBadnikBreak(this, true);
		}
	}
	this->state.Run(this);
	this->animator.Process();
}
void Crawl::LateUpdate() {}
void Crawl::StaticUpdate() {}
void Crawl::Draw() { this->animator.DrawSprite(nullptr, false); }

void Crawl::Create(void *data)
{
	if (!sceneInfo->inEditor) {
		this->active = ACTIVE_BOUNDS;
		this->visible = true;
		this->drawGroup = 2;
		this->drawFX    = FX_FLIP;
		this->state.Set(&Crawl::State_Setup);

        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->startPos.x    = this->position.x;
        this->startPos.y    = this->position.y;
	}   
}

void Crawl::StageLoad()
{
	sVars->aniFrames.Load("CNZ/Crawl.bin", SCOPE_STAGE);

	sVars->hitboxBadnik.left   = -15;
    sVars->hitboxBadnik.top    = -16;
    sVars->hitboxBadnik.right  = 15;
    sVars->hitboxBadnik.bottom = 16;

    DebugMode::AddObject(sVars->classID, &Crawl::DebugSpawn, &Crawl::DebugDraw);
    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void Crawl::DebugSpawn() { GameObject::Create<Crawl>(nullptr, this->position.x, this->position.y); }

void Crawl::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, Idle, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}


void Crawl::State_Setup()
{
    if (this->active != 4) {
        this->active = ACTIVE_NORMAL;
    }

	if (this->direction == 0) {
        this->velocity.x = -0x2000;
    }
	else {
        this->velocity.x = 0x2000;
	}

	this->state.Set(&Crawl::State_Moving);
}

void Crawl::State_Moving()
{
    this->position.x += this->velocity.x;
	this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 16 << 16, 8);

	this->timer++;
	if (this->timer == 512) {
        this->timer = 0;
		this->state.Set(&Crawl::State_Idle);
	}

	if (this->foundTarget == true) {
		this->prevState = this->state;
		this->state.Set(&Crawl::State_Defend);
		this->animator.SetAnimation(sVars->aniFrames, Idle, false, 0);
	}

	this->animator.SetAnimation(sVars->aniFrames, Move, false, 0);
}

void Crawl::State_Idle()
{
	this->timer++;
	if (this->timer == 60) {
		this->timer = 0;
		this->direction ^= FLIP_X;
		this->velocity.x = -this->velocity.x;
		this->state.Set(&Crawl::State_Moving);
	}

	if (this->foundTarget == true) {
		this->prevState = this->state;
		this->state.Set(&Crawl::State_Defend);
	}

	this->animator.SetAnimation(sVars->aniFrames, Idle, false, 0);
}

void Crawl::State_Defend()
{
	this->animator.SetAnimation(sVars->aniFrames, Defend, false, 0);

	if (this->foundTarget == false) {
		this->state = this->prevState;
	}
}

#if RETRO_INCLUDE_EDITOR
void Crawl::EditorDraw() {}

void Crawl::EditorLoad() {}
#endif

void Crawl::Serialize() {}

} // namespace GameLogic