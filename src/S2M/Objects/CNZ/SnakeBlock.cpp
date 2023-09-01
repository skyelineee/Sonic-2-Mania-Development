// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SnakeBlock Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SnakeBlock.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SnakeBlock);

void SnakeBlock::Update()
{
    int32 snakeFrame = this->frame;
    this->frame = this->animationTimer;
    this->frame >>= 4;
    this->animationTimer++;
	this->animationTimer &= 0xFF;

	this->collisionOffsetY = 0;
	if (this->frame > snakeFrame) {
		switch (snakeFrame) {
		case 1:
		case 2:
		case 3:
			this->collisionOffsetY = -0x100000;
			break;
		case 12:
		case 13:
		case 14:
			this->collisionOffsetY = 0x100000;
			break;
		}
	}

    for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
		bool32 collisionCheck = false;
        switch (snakeFrame) {
		case 1:
			sVars->snakeHitbox.left   = -40;
			sVars->snakeHitbox.top    = 16;
			sVars->snakeHitbox.right  = -24;
			sVars->snakeHitbox.bottom = 32;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -40, 16, -24, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 2:
			sVars->snakeHitbox.left   = -40;
			sVars->snakeHitbox.top    = 0;
			sVars->snakeHitbox.right  = -24;
			sVars->snakeHitbox.bottom = 32;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -40, 0, -24, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 3:
			sVars->snakeHitbox.left   = -40;
			sVars->snakeHitbox.top    = -16;
			sVars->snakeHitbox.right  = -24;
			sVars->snakeHitbox.bottom = 32;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -40, -16, -24, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 4:
			sVars->snakeHitbox.left   = -40;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = -24;
			sVars->snakeHitbox.bottom = 32;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -40, -32, -24, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 5:
			sVars->snakeHitbox.left   = -40;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = -24;
			sVars->snakeHitbox.bottom = 32;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -40, -32, -24, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			sVars->snakeHitbox.left   = -24;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = -8;
			sVars->snakeHitbox.bottom = -16;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -24, -32, -8, -16, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 6:
			sVars->snakeHitbox.left   = -40;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = -24;
			sVars->snakeHitbox.bottom = 16;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -40, -32, -24, 16, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			sVars->snakeHitbox.left   = -24;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = 8;
			sVars->snakeHitbox.bottom = -16;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -24, -32, 8, -16, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 7:
			sVars->snakeHitbox.left   = -40;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = -24;
			sVars->snakeHitbox.bottom = 0;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -40, -32, -24, 0, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			sVars->snakeHitbox.left   = -24;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = 24;
			sVars->snakeHitbox.bottom = -16;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -24, -32, 24, -16, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 8:
			sVars->snakeHitbox.left   = -40;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = 40;
			sVars->snakeHitbox.bottom = -16;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -40, -32, 40, -16, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 9:
			sVars->snakeHitbox.left   = 24;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = 40;
			sVars->snakeHitbox.bottom = 0;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, 24, -32, 40, 0, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			sVars->snakeHitbox.left   = -24;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = 24;
			sVars->snakeHitbox.bottom = -16;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -24, -32, 24, -16, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 10:
			sVars->snakeHitbox.left   = 24;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = 40;
			sVars->snakeHitbox.bottom = 16;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, 24, -32, 40, 16, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			sVars->snakeHitbox.left   = -8;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = 24;
			sVars->snakeHitbox.bottom = -16;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, -8, -32, 24, -16, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 11:
			sVars->snakeHitbox.left   = 24;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = 40;
			sVars->snakeHitbox.bottom = 32;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, 24, -32, 40, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			sVars->snakeHitbox.left   = 8;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = 24;
			sVars->snakeHitbox.bottom = -16;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, 8, -32, 24, -16, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 12:
			sVars->snakeHitbox.left   = 24;
			sVars->snakeHitbox.top    = -32;
			sVars->snakeHitbox.right  = 40;
			sVars->snakeHitbox.bottom = 32;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, 24, -32, 40, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 13:
			sVars->snakeHitbox.left   = 24;
			sVars->snakeHitbox.top    = -16;
			sVars->snakeHitbox.right  = 40;
			sVars->snakeHitbox.bottom = 32;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, 24, -16, 40, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 14:
			sVars->snakeHitbox.left   = 24;
			sVars->snakeHitbox.top    = 0;
			sVars->snakeHitbox.right  = 40;
			sVars->snakeHitbox.bottom = 32;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, 24, 0, 40, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;

		case 15:
			sVars->snakeHitbox.left   = 24;
			sVars->snakeHitbox.top    = 16;
			sVars->snakeHitbox.right  = 40;
			sVars->snakeHitbox.bottom = 32;
			collisionCheck = currentPlayer->CheckCollisionBox(this, &sVars->snakeHitbox);
			//BoxCollisionTest(C_SOLID, this->entityPos, 24, 16, 40, 32, currentPlayer, C_BOX, C_BOX, C_BOX, C_BOX);
			break;
		}

		if (collisionCheck == C_TOP) {
			currentPlayer->position.y += this->collisionOffsetY;
		}
	}
}

void SnakeBlock::LateUpdate() {}
void SnakeBlock::StaticUpdate() {}
void SnakeBlock::Draw()
{
	this->animator.SetAnimation(sVars->aniFrames, 0, true, this->frame);
	this->animator.DrawSprite(nullptr, false);
}

void SnakeBlock::Create(void *data)
{
	if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->visible       = true;
        this->drawGroup     = 2;
    }
}

void SnakeBlock::StageLoad() 
{ 
	sVars->aniFrames.Load("CNZ/SnakeBlock.bin", SCOPE_STAGE); 

    DebugMode::AddObject(sVars->classID, &SnakeBlock::DebugSpawn, &SnakeBlock::DebugDraw);
}

void SnakeBlock::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 8);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void SnakeBlock::DebugSpawn() { GameObject::Create<SnakeBlock>(nullptr, this->position.x, this->position.y); }

#if RETRO_INCLUDE_EDITOR
void SnakeBlock::EditorDraw() {}

void SnakeBlock::EditorLoad() {}
#endif

void SnakeBlock::Serialize() {}

} // namespace GameLogic