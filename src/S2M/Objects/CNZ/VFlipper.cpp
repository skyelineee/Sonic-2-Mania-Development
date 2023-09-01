// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: VFlipper Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "VFlipper.hpp"
#include "Flipper.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(VFlipper);

void VFlipper::Update() 
{
    for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        bool32 checkResult = currentPlayer->CheckCollisionBox(this, &sVars->hitbox);
		if (currentPlayer->onGround) {
			switch (checkResult) {
			default:
			case C_NONE:
			case C_TOP:
			case C_BOTTOM:
				break;
				
			case C_LEFT:
				currentPlayer->groundVel = -0x100000;
				currentPlayer->velocity.x = -0x100000;

				if (!currentPlayer->state.Matches(&Player::State_TubeRoll)) {
					currentPlayer->state.Set(&Player::State_Roll);
					currentPlayer->animator.SetAnimation(currentPlayer->aniFrames, Player::ANI_JUMP, false, 0);
				}

				if (currentPlayer->animator.prevAnimationID != Player::ANI_JUMP) {
					currentPlayer->position.y -= currentPlayer->jumpOffset >> 16;
				}

				this->state.Set(&VFlipper::State_Activated_L);
				Flipper::sVars->sfxFlipper.Play();
				break;

			case C_RIGHT:
				currentPlayer->groundVel = 0x100000;
				currentPlayer->velocity.x = 0x100000;

				if (!currentPlayer->state.Matches(&Player::State_TubeRoll) && !currentPlayer->state.Matches(&Player::State_TubeAirRoll)) {
					currentPlayer->state.Set(&Player::State_Roll);
					currentPlayer->animator.SetAnimation(currentPlayer->aniFrames, Player::ANI_JUMP, false, 0);
				}

				if (currentPlayer->animator.prevAnimationID != Player::ANI_JUMP) {
					currentPlayer->position.y -= currentPlayer->jumpOffset >> 16;
				}

				this->state.Set(&VFlipper::State_Activated_R);
				Flipper::sVars->sfxFlipper.Play();
				break;
			}
		}
	}

	this->state.Run(this);
}

void VFlipper::LateUpdate() {}
void VFlipper::StaticUpdate() {}
void VFlipper::Draw() { this->animator.DrawSprite(nullptr, false); }

void VFlipper::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->visible       = true;
        this->drawGroup     = 3;
        this->state.Set(&VFlipper::State_Idle);
    }
}

void VFlipper::StageLoad()
{
	sVars->aniFrames.Load("CNZ/VFlipper.bin", SCOPE_STAGE);

	Flipper::sVars->sfxFlipper.Get("Stage/Flipper.wav");

	sVars->hitbox.left   = -8;
    sVars->hitbox.top    = -24;
    sVars->hitbox.right  = 8;
    sVars->hitbox.bottom = 24;

    DebugMode::AddObject(sVars->classID, &VFlipper::DebugSpawn, &VFlipper::DebugDraw);
}

void VFlipper::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void VFlipper::DebugSpawn() { GameObject::Create<VFlipper>(nullptr, this->position.x, this->position.y); }

void VFlipper::State_Idle() { this->animator.SetAnimation(sVars->aniFrames, 0, true, 0); }

void VFlipper::State_Activated_L()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 2);
    if (this->timer < 6) {
        this->timer++;
    }
	else {
        this->timer = 0;
        this->state.Set(&VFlipper::State_Idle);
	}
}

void VFlipper::State_Activated_R()
{
	this->animator.SetAnimation(sVars->aniFrames, 0, true, 1);
	if (this->timer < 6) {
		this->timer++;
	}
	else {
		this->timer = 0;
        this->state.Set(&VFlipper::State_Idle);
	}
}

#if RETRO_INCLUDE_EDITOR
void VFlipper::EditorDraw() {}

void VFlipper::EditorLoad() {}
#endif

void VFlipper::Serialize() {}

} // namespace GameLogic