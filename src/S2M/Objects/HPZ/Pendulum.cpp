// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Pendulum Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Pendulum.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Pendulum);

void Pendulum::Update()
{
    int32 ballPos = 0;
    if (this->velocity.y >= 0) {
        this->velocity.y += 0x1200;
    }
    else {
        this->velocity.y += 0xF00;
    }
    this->spikeBallPosFG += this->velocity.y;

	if (this->spikeBallPosFG < -0x340000) {
		this->spikeBallPosFG = -0x340000;
	}

	if (this->spikeBallPosFG >= 0x340000) {
		this->spikeBallPosFG = 0x340000;
		ballPos = this->velocity.y;
		ballPos >>= 4;
		this->velocity.y -= ballPos;
		this->velocity.y = -this->velocity.y;
		sVars->sfxImpact.Play(false, 255);
	}

	this->spikeBallPosBG = this->spikeBallPosFG;
	this->spikeBallPosBG = -this->spikeBallPosBG;
	this->spikeBallFrameFG = 3;
	this->spikeBallFrameBG = 6;

	ballPos = this->spikeBallPosFG;
	ballPos >>= 16;
	if (ballPos > -48)
		this->spikeBallFrameBG = 7;

	if (ballPos > -24)
		this->spikeBallFrameBG = 8;

	if (ballPos > -32)
		this->spikeBallFrameFG = 4;

	if (ballPos > 24)
		this->spikeBallFrameFG = 5;

	if (this->spikeBallFrameFG > 3) {
		int32 temp6 = this->position.y;
		this->position.y += this->spikeBallPosFG;

		for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
			if (currentPlayer->CheckCollisionTouch(this, &sVars->hitbox)) {
				currentPlayer->Hurt(this);
			}
		}

		this->position.y = temp6;
	}
}
void Pendulum::LateUpdate() {}
void Pendulum::StaticUpdate() {}
void Pendulum::Draw() 
{
	Vector2 drawPos;
	if (this->spikeBallPosBG > 0) {
		drawPos.y = this->spikeBallPosBG;
		drawPos.y *= 28;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 15);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosBG;
		drawPos.y *= 36;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 16);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosBG;
		drawPos.y *= 12;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 13);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosBG;
		drawPos.y *= 20;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 14);
		this->animator.DrawSprite(&drawPos, false);
	}
	else {
		drawPos.y = this->spikeBallPosBG;
		drawPos.y *= 32;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 16);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosBG;
		drawPos.y *= 40;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 15);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosBG;
		drawPos.y *= 16;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 14);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosBG;
		drawPos.y *= 24;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 13);
		this->animator.DrawSprite(&drawPos, false);
    }

	drawPos.y   = this->spikeBallPosBG;
    drawPos.x   = this->position.x;
    this->animator.SetAnimation(sVars->aniFrames, 0, true, this->spikeBallFrameBG);
    this->animator.DrawSprite(&drawPos, false);
    drawPos.y += this->position.y;

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->animator.DrawSprite(nullptr, false);

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 1);
    this->animator.DrawSprite(nullptr, false);

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 2);
    this->animator.DrawSprite(nullptr, false);

	if (this->spikeBallPosFG > 0) {
		drawPos.y = this->spikeBallPosFG;
		drawPos.y *= 12;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 9);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosFG;
		drawPos.y *= 20;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 10);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosFG;
		drawPos.y *= 28;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 11);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosFG;
		drawPos.y *= 36;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 12);
		this->animator.DrawSprite(&drawPos, false);
	}
	else {
		drawPos.y = this->spikeBallPosFG;
		drawPos.y *= 12;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 10);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosFG;
		drawPos.y *= 20;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 9);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosFG;
		drawPos.y *= 28;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 12);
		this->animator.DrawSprite(&drawPos, false);

		drawPos.y = this->spikeBallPosFG;
		drawPos.y *= 36;
		drawPos.y /= 52;
		drawPos.y += this->position.y;
		drawPos.x = this->position.x;
		this->animator.SetAnimation(sVars->aniFrames, 0, true, 11);
		this->animator.DrawSprite(&drawPos, false);
	}

	drawPos.y = this->spikeBallPosFG;
	drawPos.y += this->position.y;
	drawPos.x = this->position.x;
	this->animator.SetAnimation(sVars->aniFrames, 0, true, this->spikeBallFrameFG);
	this->animator.DrawSprite(&drawPos, false);
}

void Pendulum::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->spikeBallPosFG = -0x340000;
	}
}

void Pendulum::StageLoad() 
{
	sVars->aniFrames.Load("HPZ/Pendulum.bin", SCOPE_STAGE);
	sVars->sfxImpact.Get("Stage/Impact2.wav");

	sVars->hitbox.left   = -16;
	sVars->hitbox.top    = -16;
	sVars->hitbox.right  = 16;
	sVars->hitbox.bottom = 16;

    DebugMode::AddObject(sVars->classID, &Pendulum::DebugSpawn, &Pendulum::DebugDraw);
}

void Pendulum::DebugSpawn()
{
    GameObject::Create<Pendulum>(nullptr, this->position.x, this->position.y);
}

void Pendulum::DebugDraw()
{
	DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
	DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

#if RETRO_INCLUDE_EDITOR
void Pendulum::EditorDraw() {}

void Pendulum::EditorLoad() {}
#endif

void Pendulum::Serialize() {}

} // namespace GameLogic