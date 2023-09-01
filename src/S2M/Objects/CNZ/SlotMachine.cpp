// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SlotMachine Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SlotMachine.hpp"
#include "SlotDisplay.hpp"
#include "SlotPrize.hpp"
#include "Global/ScoreBonus.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SlotMachine);

void SlotMachine::Update() 
{
    if (this->state.Matches(&SlotMachine::State_Wait)) {
        for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            currentPlayer->CheckCollisionBox(this, &sVars->hitboxLeaveL);
            currentPlayer->CheckCollisionBox(this, &sVars->hitboxLeaveR);
        }
	}
	// i assume player state static in v4 also disables collisions, but here it doesnt, so i have to disable the hitbox box for the slot entirely when the player is supposed to be in it
    if (!this->state.Matches(&SlotMachine::State_Ready) && !this->state.Matches(&SlotMachine::State_Hold)
     && !this->state.Matches(&SlotMachine::State_Reward) && !this->state.Matches(&SlotMachine::State_Wait)) {
        for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            switch (currentPlayer->CheckCollisionBox(this, &sVars->hitboxEntry)) {
                case C_TOP:
                case C_BOTTOM:
                    if (this->state.Matches(&SlotMachine::State_Idle)) {
                        this->active = ACTIVE_NORMAL;
                        if (this->type == ComboMachine) {
                            this->state.Set(&SlotMachine::State_Ready);
                        }
                        else {
                            this->state.Set(&SlotMachine::State_Hold);
                            this->targetSpinSpeedL = Math::Rand(this->targetSpinSpeedL, 4);
                            this->targetSpinSpeedL += 2;
                            this->targetSpinSpeedM = Math::Rand(this->targetSpinSpeedM, 4);
                            this->targetSpinSpeedM += 2;
                            this->targetSpinSpeedR = Math::Rand(this->targetSpinSpeedR, 4);
                            this->targetSpinSpeedR += 2;
                            this->timer = Math::Rand(this->timer, 16);
                            this->timer += 14;
                            this->timer *= 3;

                            int32 randomizationIndex = 0;
                            int32 slotTimer = 0;
                            int32 prizeTimer = Zone::sVars->timer;
                            prizeTimer &= 0xFF;
                            while (slotTimer != 0xFF) {
                                slotTimer = sVars->randomizeTable[randomizationIndex];
                                prizeTimer -= slotTimer;
                                if (prizeTimer >= 0) {
                                    randomizationIndex += 4;
                                }
                                else {
                                    slotTimer = 0xFF;
                                }
                            }

                            slotTimer = sVars->randomizeTable[randomizationIndex];
                            if (slotTimer != 0xFF) {
                                randomizationIndex++;
                                this->prizeTablePosL = sVars->randomizeTable[randomizationIndex];

                                randomizationIndex++;
                                this->prizeTablePosM = sVars->randomizeTable[randomizationIndex];

                                randomizationIndex++;
                                this->prizeTablePosR = sVars->randomizeTable[randomizationIndex];
                            }
                            else {
                                this->prizeTablePosL = Math::Rand(this->prizeTablePosL, 8);
                                this->prizeTablePosM = Math::Rand(this->prizeTablePosM, 8);
                                this->prizeTablePosR = Math::Rand(this->prizeTablePosR, 8);
                            }
                        }
                    }

                    currentPlayer->nextGroundState.Set(nullptr);
                    currentPlayer->nextAirState.Set(nullptr);
                    currentPlayer->onGround          = false;
                    currentPlayer->velocity.x = 0;
                    currentPlayer->velocity.y = 1;
                    currentPlayer->position.x = this->position.x;
                    currentPlayer->position.y = this->position.y;
                    currentPlayer->state.Set(&Player::State_Static);
                    currentPlayer->stateGravity.Set(&Player::Gravity_NULL);
                    currentPlayer->animator.SetAnimation(currentPlayer->aniFrames, Player::ANI_JUMP, false, 0);
                    break;
            }
		}
	}

	this->state.Run(this);
    this->animator.Process();
}

void SlotMachine::LateUpdate() {}
void SlotMachine::StaticUpdate() {}
void SlotMachine::Draw() { this->animator.DrawSprite(nullptr, false); }

void SlotMachine::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->visible       = true;
        this->active        = ACTIVE_BOUNDS;
        this->drawGroup     = 5;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
        this->state.Set(&SlotMachine::State_Idle);

		// Randomize the initial slot prize displays
        sVars->slotPrizeL = Math::Rand(sVars->slotPrizeL, 8);
        sVars->slotPrizeL <<= 5;
        sVars->slotPrizeM = Math::Rand(sVars->slotPrizeM, 8);
        sVars->slotPrizeM <<= 5;
        sVars->slotPrizeR = Math::Rand(sVars->slotPrizeR, 8);
		sVars->slotPrizeR <<= 5;
    }
}

void SlotMachine::StageLoad()
{
    sVars->aniFrames.Load("CNZ/SlotMachine.bin", SCOPE_STAGE);

	sVars->sfxGrab.Get("Global/Grab.wav");
	sVars->sfxSlot.Get("Stage/SlotMachine.wav");

    sVars->hitboxEntry.left   = -24;
    sVars->hitboxEntry.top    = -20;
    sVars->hitboxEntry.right  = 24;
    sVars->hitboxEntry.bottom = 20;
	
	sVars->hitboxLeaveL.left   = -24;
    sVars->hitboxLeaveL.top    = -20;
    sVars->hitboxLeaveL.right  = -17;
    sVars->hitboxLeaveL.bottom = 20;

	sVars->hitboxLeaveR.left   = 17;
    sVars->hitboxLeaveR.top    = -20;
    sVars->hitboxLeaveR.right  = 24;
    sVars->hitboxLeaveR.bottom = 20;

    DebugMode::AddObject(sVars->classID, &SlotMachine::DebugSpawn, &SlotMachine::DebugDraw);
}

void SlotMachine::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void SlotMachine::DebugSpawn() 
{ 
	SlotMachine *slot = GameObject::Create<SlotMachine>(nullptr, this->position.x, this->position.y);
	slot->type = ComboMachine;
}

void SlotMachine::State_Idle() { this->animator.SetAnimation(sVars->aniFrames, Inactive, true, 0); }

void SlotMachine::State_Ready() 
{
    this->animator.SetAnimation(sVars->aniFrames, Active, false, 0);

	int32 scoreTimer = this->timer;
    scoreTimer &= 0xF;
	if (scoreTimer == 0xF) {
		ScoreBonus *bonus = GameObject::Create<ScoreBonus>(nullptr, this->position.x, this->position.y);
		bonus->drawGroup = 4;
		sVars->sfxSlot.Play(false, 255);
		for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
			if (currentPlayer->state.Matches(&Player::State_Static)) {
				if (currentPlayer->CheckCollisionTouch(this, &sVars->hitboxEntry)) {
					currentPlayer->score += 100;
				}
			}
		}
	}

	this->timer++;
	if (this->timer == 128) {
		this->timer = 0;
		this->state.Set(&SlotMachine::State_Wait);
		this->active = ACTIVE_BOUNDS;
		for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
			if (currentPlayer->state.Matches(&Player::State_Static)) {
				if (currentPlayer->CheckCollisionTouch(this, &sVars->hitboxEntry)) {
					currentPlayer->state.Set(&Player::State_Air);
				}
			}
		}
	}
}

void SlotMachine::State_Hold()
{
	this->animator.SetAnimation(sVars->aniFrames, Active, false, 0);

	SlotDisplay::sVars->needsSetup = false; // placeholder
	int32 scoreTimer = Zone::sVars->timer;
	scoreTimer &= 0xF;
	if (scoreTimer == 0xF) {
		sVars->sfxSlot.Play(false, 255);
	}

	scoreTimer &= 7;
	if (scoreTimer == 0) {
		if (this->spinSpeedL < this->targetSpinSpeedL) {
			this->spinSpeedL++;
		}

		if (this->spinSpeedM < this->targetSpinSpeedM) {
			this->spinSpeedM++;
		}

		if (this->spinSpeedR < this->targetSpinSpeedR) {
			this->spinSpeedR++;
		}
	}

	sVars->slotPrizeL += this->spinSpeedL;
	sVars->slotPrizeL &= 0xFF;
	sVars->slotPrizeM += this->spinSpeedM;
	sVars->slotPrizeM &= 0xFF;
	sVars->slotPrizeR += this->spinSpeedR;
	sVars->slotPrizeR &= 0xFF;

	if (this->timer != 0) {
		this->timer--;
    }
	else {
		if (this->slotSpinTimer != 0) {
			this->slotSpinTimer++;
		}

		int32 prizeCard = 0;
		switch (this->activeSlot) {
            case Slot_L:
				switch (this->slotSpinTimer)  {
					case SlotTimer_Spin:
						prizeCard = sVars->slotPrizeL;
						prizeCard >>= 5;
						if (prizeCard == this->prizeTablePosL) {
							prizeCard = sVars->slotPrizeL;
							prizeCard &= 0x1F;
							if (prizeCard < this->spinSpeedL) {
								this->spinSpeedL 		= 2;
								this->targetSpinSpeedL = 2;
								sVars->sfxGrab.Play(false, 255);
								this->slotSpinTimer++;
							}
						}
						break;

					case SlotTimer_Slow:
						this->spinSpeedL 	   = -2;
						this->targetSpinSpeedL = -2;
						break;

					case SlotTimer_Stop:
						this->spinSpeedL 		= 0;
						this->targetSpinSpeedL  = 0;
						this->slotSpinTimer 	= 0;
						this->activeSlot++;
						break;
				}

			break;
			case Slot_M:
				switch (this->slotSpinTimer) {
					case SlotTimer_Spin:
						GET_BIT(prizeCard, sVars->slotPrizeL, 4);
						if (prizeCard == true) {
							sVars->slotPrizeL += 0x20;
							sVars->slotPrizeL &= 0xFF;
						}
						sVars->slotPrizeL &= 0xE0;

						prizeCard = sVars->slotPrizeM;
						prizeCard >>= 5;
						if (prizeCard == this->prizeTablePosM) {
							prizeCard = sVars->slotPrizeM;
							prizeCard &= 0x1F;
							if (prizeCard < this->spinSpeedM) {
								this->spinSpeedM 		= 2;
								this->targetSpinSpeedM = 2;
								sVars->sfxGrab.Play(false, 255);
								this->slotSpinTimer++;
							}
						}
						break;

					case SlotTimer_Slow:
						this->spinSpeedM 		= -2;
						this->targetSpinSpeedM = -2;
						break;

					case SlotTimer_Stop:
						this->spinSpeedM 		= 0;
						this->targetSpinSpeedM  = 0;
						this->slotSpinTimer 	= 0;
						this->activeSlot++;
						break;
				}

			break;
			case Slot_R:
				switch (this->slotSpinTimer) {
                    case SlotTimer_Spin:
						GET_BIT(prizeCard, sVars->slotPrizeM, 4);
						if (prizeCard == true) {
							sVars->slotPrizeM += 0x20;
							sVars->slotPrizeM &= 0xFF;
						}
						sVars->slotPrizeM &= 0xE0;

						prizeCard = sVars->slotPrizeR;
						prizeCard >>= 5;
						if (prizeCard == this->prizeTablePosR) {
							prizeCard = sVars->slotPrizeR;
							prizeCard &= 0x1F;
							if (prizeCard < this->spinSpeedR) {
								this->spinSpeedR 		= 2;
								this->targetSpinSpeedR  = 2;
								sVars->sfxGrab.Play(false, 255);
								this->slotSpinTimer++;
							}
						}
						break;

					case SlotTimer_Slow:
						this->spinSpeedR 		= -2;
						this->targetSpinSpeedR = -2;
						break;

					case SlotTimer_Stop:
						this->spinSpeedR 		= 0;
						this->targetSpinSpeedR  = 0;
						break;

					case SlotTimer_Calc:
						GET_BIT(prizeCard, sVars->slotPrizeR, 4);
						if (prizeCard == true) {
							sVars->slotPrizeR += 0x20;
							sVars->slotPrizeR &= 0xFF;
						}
						sVars->slotPrizeR &= 0xE0;

						// All Slots Stopped, its prize time!
						this->slotSpinTimer	= 0;
						this->activeSlot	= Slot_L;
						this->state.Set(&SlotMachine::State_Reward);

						int32 rewardL = sVars->prizeLTable[this->prizeTablePosL];
						int32 rewardM = sVars->prizeMTable[this->prizeTablePosM];
						int32 rewardR = sVars->prizeRTable[this->prizeTablePosR];

						this->rewardRingCount = 0;
						if (rewardL == rewardM) { // PrizeL == PrizeM
							if (rewardM == rewardR) { // PrizeM == PrizeR
								this->rewardRingCount = sVars->slotRewardTable[rewardL];
							}
							else {
								if (rewardL == Jackpot) {	 // PrizeM != PrizeR && PrizeL == Jackpot 
									this->rewardRingCount = sVars->slotRewardTable[rewardR];
									this->rewardRingCount <<= 2;
								}
								else {
									if (rewardR == Jackpot) {	 // PrizeM != PrizeR && PrizeR == Jackpot 
										this->rewardRingCount = sVars->slotRewardTable[rewardL];
										this->rewardRingCount <<= 1;
									}
								}
							}
						}
						else {
							if (rewardL == rewardR) { // PrizeL == PrizeR
								if (rewardL == Jackpot) {	 // PrizeL == PrizeR && PrizeL != PrizeM && PrizeL == Jackpot 
									this->rewardRingCount = sVars->slotRewardTable[rewardM];
									this->rewardRingCount <<= 2;
								}
								else {
									if (rewardM == Jackpot) {	 // PrizeL == PrizeR && PrizeL != PrizeM && PrizeR == Jackpot 
										this->rewardRingCount = sVars->slotRewardTable[rewardL];
										this->rewardRingCount <<= 1;
									}
								}
							}
							else {
								if (rewardM == rewardR) { // PrizeM == PrizeR
									if (rewardM == Jackpot) {	 // PrizeM == PrizeR && PrizeL != PrizeM && PrizeL != PrizeR && PrizeM == Jackpot 
										this->rewardRingCount = sVars->slotRewardTable[rewardL];
										this->rewardRingCount <<= 2;
									}
									else {
										if (rewardL == Jackpot) {	  // PrizeM == PrizeR && PrizeL != PrizeM && PrizeL != PrizeR && PrizeL == Jackpot 
											this->rewardRingCount = sVars->slotRewardTable[rewardM];
											this->rewardRingCount <<= 1;
										}
									}
								}
							}
						}

						// if no jackpot at all, but we have a BAR...
						if (this->rewardRingCount == 0) {
							if (rewardL == Bar) {
								this->rewardRingCount += 2;
							}

							if (rewardM == Bar) {
								this->rewardRingCount += 2;
							}

							if (rewardR == Bar) {
								this->rewardRingCount += 2;
							}
						}

						// Eggman...
						if (this->rewardRingCount < 0) {
							this->rewardRingCount = -100;
						}


						// No Rings...?
						if (this->rewardRingCount == 0) {
							this->timer = 1;
						}
						else {
							if (this->rewardRingCount > 0) { // Winner!!!
								this->timer = 26;
							}
							else {	// Loser...
								this->timer = 30;
							}
						}
					break;
				}
			break;
		}
	}
}

void SlotMachine::State_Unused() {}

void SlotMachine::State_Reward()
{
	this->animator.SetAnimation(sVars->aniFrames, Active, false, 0);

	if (this->rewardRingCount == 0) {
		this->timer--;
		if (this->timer <= 0) {
			this->angle = 0;
			this->active = ACTIVE_BOUNDS;
			this->state.Set(&SlotMachine::State_Wait);
            for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
				if (currentPlayer->state.Matches(&Player::State_Static)) {
					if (currentPlayer->CheckCollisionTouch(this, &sVars->hitboxEntry)) {
						currentPlayer->state.Set(&Player::State_Air);
					}
				}
			}
		}
	}
	else {
		int32 prizePosX = 0;
		int32 prizePosY = 0;
		GET_BIT(prizePosX, Zone::sVars->timer, 0);
		if (prizePosX == false) {
			prizePosX = Math::Cos256(this->angle);
			prizePosX <<= 15;
			prizePosX += this->position.x;

			prizePosY = Math::Sin256(this->angle);
			prizePosY <<= 15;
			prizePosY += this->position.y;

			SlotPrize *prize = GameObject::Create<SlotPrize>(INT_TO_VOID(true), prizePosX, prizePosY);
			prize->originPos.x 	= this->position.x;
			prize->originPos.y 	= this->position.y;
			prize->inkEffect 		= INK_ALPHA; // 2
			if (this->rewardRingCount > 0) {
				prize->timer = 26;
				prize->state.Set(&SlotPrize::State_Winner);
                prize->listID = SlotPrize::Rings;
				this->angle += 0x89;
				this->rewardRingCount--;
			}
			else {
				prize->timer = 30;
				prize->state.Set(&SlotPrize::State_Loser);
				prize->listID = SlotPrize::Spike;
				this->angle += 0x90;
				this->rewardRingCount++;
			}
			this->angle &= 0xFF;
		}
	}
}

void SlotMachine::State_Wait()
{
	this->animator.SetAnimation(sVars->aniFrames, Inactive, false, 0);
	this->timer++;
	if (this->timer == 30) {
		this->timer = 0;
		this->state.Set(&SlotMachine::State_Idle);
	}
}

#if RETRO_REV0U
void SlotMachine::StaticLoad(Static *sVars)
{
	RSDK_INIT_STATIC_VARS(SlotMachine);

	int32 prizeLTable[] = {
	    3, // SLOTPRIZE_BAR
	    5, // SLOTPRIZE_EGGMAN
	    2, // SLOTPRIZE_RING
	    1, // SLOTPRIZE_TAILS
	    0, // SLOTPRIZE_SONIC
	    4, // SLOTPRIZE_JACKPOT
	    1, // SLOTPRIZE_TAILS
	    2, // SLOTPRIZE_RING
	};
	memcpy(sVars->prizeLTable, prizeLTable, sizeof(prizeLTable));

	int32 prizeMTable[] = {
	    3, // SLOTPRIZE_BAR
	    5, // SLOTPRIZE_EGGMAN
	    2, // SLOTPRIZE_RING
	    1, // SLOTPRIZE_TAILS
	    0, // SLOTPRIZE_SONIC
	    4, // SLOTPRIZE_JACKPOT
	    5, // SLOTPRIZE_EGGMAN
	    0, // SLOTPRIZE_SONIC
	};
	memcpy(sVars->prizeMTable, prizeMTable, sizeof(prizeMTable));

	int32 prizeRTable[] = {
	    3, // SLOTPRIZE_BAR
	    5, // SLOTPRIZE_EGGMAN
	    2, // SLOTPRIZE_RING
	    1, // SLOTPRIZE_TAILS
	    0, // SLOTPRIZE_SONIC
	    4, // SLOTPRIZE_JACKPOT
	    1, // SLOTPRIZE_TAILS
	    2, // SLOTPRIZE_RING
	};
	memcpy(sVars->prizeRTable, prizeRTable, sizeof(prizeRTable));

	int32 slotRewardTable[] = { 30, 25, 10, 20, 150, -1 };
	memcpy(sVars->slotRewardTable, slotRewardTable, sizeof(slotRewardTable));

	int32 randomizeTable[] = { 8, 0, 0, 0, 18, 4, 4, 4, 18, 3, 3, 3, 36, 2, 2, 2, 30, 5, 5, 5, 30, 1, 1, 1, 255, 15, 15, 15 };
	memcpy(sVars->randomizeTable, randomizeTable, sizeof(randomizeTable));
}
#endif

#if RETRO_INCLUDE_EDITOR
void SlotMachine::EditorDraw() {}

void SlotMachine::EditorLoad()
{
	sVars->aniFrames.Load("CNZ/SlotMachine.bin", SCOPE_STAGE);
	
	RSDK_ACTIVE_VAR(sVars, type);
	RSDK_ENUM_VAR("Slot Combo Machine");
	RSDK_ENUM_VAR("Slot Prize Machine");
}
#endif

void SlotMachine::Serialize() { RSDK_EDITABLE_VAR(SlotMachine, VAR_INT32, type); }

} // namespace GameLogic