// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: BreakoffPillar Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "BreakoffPillar.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(BreakoffPillar);

void BreakoffPillar::Update() 
{
    for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        currentPlayer->CheckCollisionBox(this, &sVars->hitbox); // base hitbox for the floating pillar

		int32 originPos = this->position.y; // sets originPos to the y position before it does anything with it
        this->position.y += this->offsetY; // adds to the y position by whatever the yoffset currently is
        currentPlayer->CheckCollisionBox(this, &sVars->fallenHitbox); // checks the collision for the fallen platform with the new y position
		this->position.y = originPos; // sets it back to the origin once the collision is checked
	}

	this->state.Run(this);
}

void BreakoffPillar::LateUpdate() {}
void BreakoffPillar::StaticUpdate() {}
void BreakoffPillar::Draw()
{
	this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
	this->animator.DrawSprite(nullptr, false);

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 1);
	this->animator.DrawSprite(nullptr, false);

	Vector2 drawPos;
	drawPos.x = this->position.x;
	drawPos.x += this->shakePosX;
	drawPos.y = this->position.y;
	drawPos.y += this->offsetY;
	this->animator.SetAnimation(sVars->aniFrames, 0, true, this->bottomFrame); //frame
	this->animator.DrawSprite(&drawPos, false);
}

void BreakoffPillar::Create(void *data)
{
	if (!sceneInfo->inEditor) {
		this->offsetY = 0;
		this->shakePosX = 0;
		this->velocity.y = 0;
		this->timer = 0;
		this->bottomFrame = 2;
		this->state.Set(&BreakoffPillar::State_AwaitPlayer);
		this->active = ACTIVE_BOUNDS;
		this->visible = true;
		this->drawGroup = 3;
		this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
	}
}

void BreakoffPillar::StageLoad()
{
	sVars->aniFrames.Load("ARZ/BreakoffPillar.bin", SCOPE_STAGE);

    sVars->hitbox.left   = -16;
    sVars->hitbox.top    = -32;
    sVars->hitbox.right  = 16;
    sVars->hitbox.bottom = 32;

	sVars->fallenHitbox.left   = -16;
    sVars->fallenHitbox.top    = 32;
    sVars->fallenHitbox.right  = 16;
    sVars->fallenHitbox.bottom = 65;

    sVars->idleHitbox.left   = -128;
    sVars->idleHitbox.top    = -256;
    sVars->idleHitbox.right  = 128;
    sVars->idleHitbox.bottom = 256;

	DebugMode::AddObject(sVars->classID, &BreakoffPillar::DebugSpawn, &BreakoffPillar::DebugDraw);
}

void BreakoffPillar::DebugSpawn() { GameObject::Create<BreakoffPillar>(nullptr, this->position.x, this->position.y); }

void BreakoffPillar::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 1);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 2);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void BreakoffPillar::State_AwaitPlayer()
{
    for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
		if (currentPlayer->CheckCollisionTouch(this, &sVars->idleHitbox)) {
            this->active = ACTIVE_NORMAL;
			this->timer = 8;
			this->state.Set(&BreakoffPillar::State_Shaking);
		}
	}
}

void BreakoffPillar::State_Shaking()
{
    this->timer--;
	if (this->timer >= 0) {
		this->shakePosX = sVars->shakePosTable[this->timer];
		//GetTableValue(object.shakePos.x, object.timer, BreakoffPillar_shakePosTable);
	}
	else {
		this->state.Set(&BreakoffPillar::State_Falling);
	}
}

void BreakoffPillar::State_Falling()
{
	this->offsetY += this->velocity.y;
	this->velocity.y += 0x3800;

	int32 yOffset1 = this->position.y;
	this->position.y += this->offsetY;
	int32 yOffset2 = this->position.y;
    if (this->TileGrip(Zone::sVars->collisionLayers, CMODE_FLOOR, this->collisionPlane, 0, 65 << 16, 2)) { 
		this->state.Set(&BreakoffPillar::State_Fallen);
		this->bottomFrame++;
	}
	yOffset2 -= this->position.y;
	this->position.y = yOffset1;

	this->offsetY -= yOffset2;
}

void BreakoffPillar::State_Fallen() {}

#if RETRO_INCLUDE_EDITOR
void BreakoffPillar::EditorDraw() {}

void BreakoffPillar::EditorLoad() {}
#endif

#if RETRO_REV0U
void BreakoffPillar::StaticLoad(Static *sVars)
{
	RSDK_INIT_STATIC_VARS(BreakoffPillar); 

	int32 shakePosTable[] = { 0, 0x10000, -0x10000, 0x10000, 0, -0x10000, 0, 0x10000 };

    memcpy(sVars->shakePosTable, shakePosTable, sizeof(shakePosTable));
}
#endif

void BreakoffPillar::Serialize() {}

} // namespace GameLogic