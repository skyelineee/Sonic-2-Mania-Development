// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Player Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Player.hpp"
#include "Zone.hpp"
#include "SaveGame.hpp"
#include "DebugMode.hpp"
#include "Common/ScreenWrap.hpp"
#include "Camera.hpp"
#include "PauseMenu.hpp"
#include "Music.hpp"
#include "Shield.hpp"
#include "HUD.hpp"
#include "Dust.hpp"
#include "Ring.hpp"
#include "ItemBox.hpp"
#include "Animals.hpp"
#include "Animals.hpp"
#include "ScoreBonus.hpp"
#include "SuperSparkle.hpp"
#include "SuperFlicky.hpp"
#include "ImageTrail.hpp"
#include "BoundsMarker.hpp"
#include "GameOver.hpp"
#include "Common/Water.hpp"

#include "Helpers/FXFade.hpp"
// #include "Competition/CompetitionSession.hpp"
#include "Helpers/BadnikHelpers.hpp"
#include "Helpers/DrawHelpers.hpp"
#include "Helpers/LogHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Player);

RSDK::Hitbox Player::fallbackHitbox = { -10, -20, 10, 20 };

bool32 inDeathHold = false;

void Player::Update()
{
    if (!this->state.Matches(&Player::State_Static))
        this->direction = (this->direction & ~FLIP_Y) | (FLIP_Y * (globals->gravityDir == CMODE_ROOF));

    if (!this->playerID)
        sVars->finishedInput = false;

    this->stateInputReplay.Run(this);
    this->stateInput.Run(this);

    if (this->sidekick)
        this->HandleSidekickRespawn();

    if (this->classID == sVars->classID) {
        if (this->camera) {
            if (this->scrollDelay > 0) {
                this->scrollDelay--;

                if (!this->scrollDelay)
                    this->camera->state.Set(&Camera::State_FollowXY);
                else if (this->velocity.x > 0 && (this->position.x >> 16) >= screenInfo->position.x + screenInfo->size.x - 32)
                    this->camera->state.Set(&Camera::State_FollowXY);
                else if (this->velocity.x < 0 && (this->position.x >> 16) < screenInfo->position.x + 32)
                    this->camera->state.Set(&Camera::State_FollowXY);
            }
            else if (!this->state.Matches(&Player::State_LookUp) && !this->state.Matches(&Player::State_Crouch)) {
                if (this->camera->lookPos.y > 0)
                    this->camera->lookPos.y -= 2;
                else if (this->camera->lookPos.y < 0)
                    this->camera->lookPos.y += 2;
            }
        }

        if (this->invincibleTimer) {
            if (this->invincibleTimer < 0) {
                this->invincibleTimer++;
            }
            else {
                this->invincibleTimer--;
                if (!this->invincibleTimer) {
                    ApplyShield();

                    if (globals->gameMode != MODE_ENCORE || !this->sidekick) {
                        bool32 stopPlaying = true;
                        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                            if (player->invincibleTimer > 0)
                                stopPlaying = false;
                        }

                        if (stopPlaying)
                            Music::JingleFadeOut(Music::TRACK_INVINCIBLE, true);
                    }
                }
            }
        }

        if (this->speedShoesTimer > 0) {
            this->speedShoesTimer--;
            if (!this->speedShoesTimer) {
                UpdatePhysicsState();

                bool32 stopPlaying = true;
                for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                    if (player->speedShoesTimer > 0)
                        stopPlaying = false;
                }

                if (stopPlaying) {
                    if (globals->ostStyle == GAME_SM)
                        Music::JingleFadeOut(Music::TRACK_SNEAKERS, true);
                    else
                        Music::sVars->musicSlowDown.Run(nullptr);
                }
            }
        }

        if (!this->state.Matches(&Player::State_Hurt)) {
            if (this->blinkTimer > 0) {
                this->blinkTimer--;
                this->visible = !(this->blinkTimer & 4);
            }
        }

        if (this->uncurlTimer > 0)
            this->uncurlTimer--;

        this->HandleSuperForm();

        if (this->characterID == ID_TAILS && !this->state.Matches(&Player::State_TailsFlight)
            && this->abilitySpeed)
            this->abilitySpeed = 0;

        // Hurt Player if we're touching T/B or L/R sides at same time
        if (this->collisionFlagH == (1 | 2) || this->collisionFlagV == (1 | 2))
            this->deathType = Player::DeathDie_Sfx;

        this->collisionFlagH = 0;
        this->collisionFlagV = 0;
        if (this->collisionLayers & Zone::sVars->moveLayerMask) {
            TileLayer *move  = Zone::sVars->moveLayer.GetTileLayer();
            move->position.x = -this->moveLayerPosition.x >> 16;
            move->position.y = -this->moveLayerPosition.y >> 16;
        }

        if (this->forceHoldRespawn) {
            this->state.Set(&Player::State_HoldRespawn);
            this->stateGravity.Set(&Player::Gravity_NULL);
        }

        if (this->isTransforming) {
            if (!this->state.Matches(&Player::State_Transform)) {
                if (!this->state.Matches(&Player::State_Air) && !this->state.Matches(&Player::State_Ground)) {
                    GameObject::Get<Music>(SLOT_MUSIC)->Destroy();

                    this->isTransforming = false;
                    sVars->sfxTransform2.Stop();
                    this->superState = Player::SuperStateFadeOut;
                    UpdatePhysicsState();
                }
                else {
                    this->state.Set(&Player::State_Transform);
                }
            }
        }

        if (!this->state.Matches(&Player::State_FlyToPlayer))
            this->state.Run(this);

        if (this->disableGravity)
            return;

        if (this->classID == sVars->classID) {
            this->flailing      = false;
            this->invertGravity = false;

            if (!this->disableTileCollisions) {
                if (this->outerBox) {
                    this->groundedStore = this->onGround;
                    this->ProcessMovement(this->outerBox, this->innerBox);
                }
                else {
                    this->outerBox      = this->animator.GetHitbox(0);
                    this->innerBox      = this->animator.GetHitbox(1);
                    this->groundedStore = this->onGround;
                    this->ProcessMovement(this->outerBox, this->innerBox);
                    this->outerBox = nullptr;
                }
            }

            this->collisionLayers &= ~Zone::sVars->moveLayerMask;
            if (this->onGround && !this->collisionMode)
                this->collisionFlagV |= 1;
        }
    }
}
void Player::LateUpdate()
{
    if (this->state.Matches(&Player::State_FlyToPlayer))
        this->state.Run(this);

    if (this->state.Matches(&Player::State_Hurt)) {
        int32 bounds = Zone::sVars->playerBoundsT[this->playerID] + 0x100000;
        if (this->position.y < bounds) {
            this->position.y = bounds;
            if (this->velocity.y < 0)
                this->velocity.y = 0;
        }
    }

    if (this->superState == Player::SuperStateFadeIn && !this->state.Matches(&Player::State_Transform))
        TryTransform(false, Player::TransformEmeralds);

    if (this->state.Matches(&Player::State_FlyCarried)) {
        this->flyCarryLeaderPos.x = this->position.x >> 0x10 << 0x10;
        this->flyCarryLeaderPos.y = this->position.y >> 0x10 << 0x10;
    }

    if (this->deathType) {
        this->abilityValues[0] = 0;

        if (this->sidekick || globals->gameMode == MODE_COMPETITION || globals->gameMode == MODE_ENCORE) {
            if (this->invincibleTimer > 1)
                this->invincibleTimer = 1;
            if (this->speedShoesTimer > 1)
                this->speedShoesTimer = 1;
        }
        else {
            this->invincibleTimer = 0;
            this->speedShoesTimer = 0;
        }

        if (this->gravityStrength <= 1)
            this->direction |= FLIP_Y;
        else
            this->direction &= ~FLIP_Y;

        this->visible    = true;
        this->onGround   = false;
        this->groundVel  = 0;
        this->velocity.x = 0;
        this->nextGroundState.Set(nullptr);
        this->nextAirState.Set(nullptr);
        this->interaction    = false;
        this->tileCollisions = TILECOLLISION_NONE;

        if (globals->gameMode != MODE_COMPETITION)
            this->active = ACTIVE_ALWAYS;

        this->shield         = Shield::None;
        this->collisionFlagH = 0;
        this->collisionFlagV = 0;
        this->underwater     = false;
        UpdatePhysicsState();

        GameObject::Get<Shield>(sVars->maxPlayerCount + this->Slot())->Destroy();

        switch (this->deathType) {
            default: break;

            case Player::DeathDie_Sfx:
                sVars->sfxHurt.Play();
                // [Fallthrough]
            case Player::DeathDie_NoSfx:
                this->deathType  = Player::DeathNone;
                this->velocity.y = -0x68000;
                this->state.Set(&Player::State_Death);
                this->stateGravity.Set(&Player::Gravity_NULL);

                if (!(this->drawFX & FX_SCALE) || this->scale.x == 0x200)
                    this->drawGroup = Zone::sVars->playerDrawGroup[1];

                if (this->sidekick || globals->gameMode == MODE_COMPETITION) {
                    if (this->camera) {
                        this->scrollDelay = 2;
                        this->camera->state.Set(nullptr);
                    }
                }
                else if (globals->gameMode == MODE_ENCORE) {
                    Player *sidekick = GameObject::Get<Player>(SLOT_PLAYER2);
                    if (!globals->stock && !sidekick->classID) {
                        Stage::SetEngineState(ENGINESTATE_FROZEN);
                        sceneInfo->timeEnabled = false;
                    }

                    if (this->camera) {
                        this->scrollDelay = 2;
                        this->camera->state.Set(nullptr);
                    }
                }
                else {
                    Stage::SetEngineState(ENGINESTATE_FROZEN);
                    sceneInfo->timeEnabled = false;
                    if (this->camera) {
                        this->scrollDelay = 2;
                        this->camera->state.Set(nullptr);
                    }
                }
                break;

            case Player::DeathDrown:
                this->deathType = Player::DeathNone;
                // Bug Details:
                // setting this actually causes a slight bug, as when you die the underwater flag is cleared but your gravity strength isn't updated
                // so if you debug out, you'll have regular speed with a moon jump
                // (yes I know this is super minor but its neat to know anyways)
                this->gravityStrength = 0x1000;
                this->velocity.y      = 0;
                Water::sVars->sfxDrown.Play();
                this->state.Set(&Player::State_Drown);
                this->stateGravity.Set(&Player::Gravity_NULL);

                if (!this->sidekick) {
                    if (globals->gameMode == MODE_COMPETITION) {
                        Music::JingleFadeOut(Music::TRACK_DROWNING, false);
                    }
                    else if (globals->gameMode == MODE_ENCORE) {
                        Player *sidekick = GameObject::Get<Player>(SLOT_PLAYER2);
                        if (globals->stock == 0 && !sidekick->classID) {
                            sceneInfo->timeEnabled = false;
                        }
                    }
                    else {
                        sceneInfo->timeEnabled = false;
                    }
                }

                if (this->camera) {
                    this->scrollDelay = 2;
                    this->camera->state.Set(nullptr);
                }
                break;
        }
    }

    if (this->onGround) {
        if (!this->nextGroundState.Matches(nullptr)) {
            Shield *shield = GameObject::Get<Shield>(sVars->maxPlayerCount + this->playerID);
            if (shield->classID == Shield::sVars->classID && shield->state.Matches(&Shield::State_Insta)) {
                shield->shieldAnimator.SetAnimation(Shield::sVars->aniFrames, Shield::AniInsta, true, shield->shieldAnimator.frameCount - 1);
                this->invincibleTimer = 0;
            }

            this->state = this->nextGroundState;
            this->nextGroundState.Set(nullptr);
            this->stateGravity.Set(&Player::Gravity_False);
            if (this->sidekick)
                GameObject::Get<Player>(SLOT_PLAYER1)->scoreBonus = 0;
            else
                this->scoreBonus = 0;

            if (this->animator.animationID == ANI_JUMP) {
                switch (this->collisionMode) {
                    case CMODE_FLOOR: this->position.y -= this->jumpOffset; break;
                    case CMODE_LWALL: this->position.x -= this->jumpOffset; break;
                    case CMODE_ROOF: this->position.y += this->jumpOffset; break;
                    case CMODE_RWALL: this->position.x += this->jumpOffset; break;
                    default: break;
                }
            }
        }

        if (this->camera && this->animator.animationID != ANI_JUMP) {
            this->camera->adjustY = 0;
        }
    }
    else if (!this->nextAirState.Matches(nullptr)) {
        this->state = this->nextAirState;
        this->nextAirState.Set(nullptr);
        this->stateGravity.Set(&Player::Gravity_True);
    }

    if (this->disableGravity)
        this->disableGravity = false;
    else
        this->stateGravity.Run(this);

    int32 state = this->hyperAbilityState - 2;
    switch (state) {
        case Player::HyperStateNone:
        case Player::HyperStateActive: this->hyperAbilityState = Player::HyperStateActive; break;

        case Player::HyperStateHyperDash:
        case Player::HyperStateHyperSlam: this->hyperAbilityState = Player::HyperStateHyperDash; break;

        default: break;
    }

    if (!this->tailFrames.Matches(nullptr)) {
        if (this->velocity.x && this->state.Matches(&Player::State_TransportTube)) {
            this->tailAnimator.SetAnimation(nullptr, 0, false, 0);
        }
        else {
            switch (this->animator.animationID) {
                case ANI_IDLE:
                case ANI_BORED_1:
                case ANI_BORED_2:
                case ANI_LOOK_UP:
                case ANI_CROUCH:
                    this->tailAnimator.SetAnimation(this->tailFrames, 0, false, 0);
                    this->tailDirection = this->direction;
                    this->tailRotation  = this->rotation;
                    break;

                case ANI_JUMP:
                    this->tailAnimator.SetAnimation(this->tailFrames, 1, false, 0);
                    if (this->zdepth == 1 || (this->zdepth != 2 && this->onGround)) {
                        this->tailRotation = this->rotation;
                        if (this->groundVel <= 0)
                            this->tailDirection |= FLIP_X;
                        else
                            this->tailDirection &= ~FLIP_X;
                    }
                    else {
                        this->tailRotation = Math::ATan2(this->velocity.x, this->velocity.y) << 1;
                        if (this->direction & FLIP_X)
                            this->tailRotation = (this->tailRotation - 256) & 0x1FF;
                        this->tailDirection = this->direction;
                    }
                    break;

                case ANI_SKID:
                    this->tailAnimator.SetAnimation(this->tailFrames, 5, false, 0);
                    this->tailDirection = this->direction;
                    break;

                case ANI_PUSH:
                    this->tailAnimator.SetAnimation(this->tailFrames, 4, false, 0);
                    this->tailDirection = this->direction;
                    break;

                case ANI_SPINDASH:
                    this->tailAnimator.SetAnimation(this->tailFrames, 2, false, 0);
                    this->tailDirection = this->direction;
                    break;

                case ANI_HANG_MOVE:
                    if (this->spriteType == ManiaSprites) {
                        this->tailAnimator.SetAnimation(this->tailFrames, 6, false, 0);
                        this->tailDirection = this->direction;
                    }
                    else {
                        this->tailAnimator.SetAnimation(nullptr, 0, false, 0);
                    }
                    break;

                case ANI_FLY:
                case ANI_FLY_TIRED:
                case ANI_FLY_LIFT:
                case ANI_FLY_LIFT_DOWN:
                case ANI_FLY_LIFT_TIRED:
                    this->tailAnimator.SetAnimation(this->tailFrames, 8, false, 0);
                    this->tailDirection = this->direction;
                    if (this->velocity.y < 0)
                        this->tailAnimator.speed = 2;
                    else
                        this->tailAnimator.speed = 1;
                    break;

                default: this->tailAnimator.SetAnimation(nullptr, 0, false, 0); break;
            }
        }

        this->tailAnimator.Process();
    }

    this->animator.Process();

    if (sceneInfo->debugMode) {
        if (controllerInfo[this->controllerID].keyY.press) {
            if (!this->state.Matches(&Player::State_Transform)
                && (this->superState == Player::SuperStateNone || this->superState == Player::SuperStateSuper)) {
                this->InvertGravity();
            }
        }
    }
}
void Player::StaticUpdate()
{
    if (sVars->superDashCooldown > 0) {
        GameObject::Get<Player>(SLOT_PLAYER1);
        HUD::EnableRingFlash();
        --sVars->superDashCooldown;
    }

    bool32 flying = false;
    bool32 tired  = false;
    if (sceneInfo->state != ENGINESTATE_REGULAR) {
        if (!sVars->playingFlySFX) {
            if (sVars->sfxFlying.IsPlaying())
                sVars->sfxFlying.Stop();
        }
        sVars->playingFlySFX = false;
    }

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->characterID == ID_TAILS) {
            int32 anim = player->animator.animationID;
            if (anim == ANI_FLY || anim == ANI_FLY_LIFT)
                flying = true;
            if (anim == ANI_FLY_TIRED || anim == ANI_FLY_LIFT_TIRED)
                tired = true;
        }

        player->lastMoveLayerPosition.x = player->moveLayerPosition.x;
        player->lastMoveLayerPosition.y = player->moveLayerPosition.y;
    }

    if (flying) {
        if (!sVars->playingFlySFX) {
            sVars->sfxFlying.Play();
            sVars->playingFlySFX = true;
        }
    }

    if (tired) {
        if (!sVars->playingTiredSFX) {
            sVars->sfxTired.Play();
            sVars->playingTiredSFX = true;
        }
    }

    if (!flying && sVars->playingFlySFX) {
        sVars->sfxFlying.Stop();
        sVars->playingFlySFX = false;
    }

    if (!tired && sVars->playingTiredSFX) {
        sVars->sfxTired.Stop();
        sVars->playingTiredSFX = false;
    }

}
void Player::Draw()
{
    color paletteStorage[2][32];
    for (int32 i = 0LL; i < 32; ++i) {
        paletteStorage[1][i] = paletteBank[0].GetEntry(i);
        paletteStorage[0][i] = paletteBank[1].GetEntry(i);
    }

    switch (this->characterID) {
        default: break;
        case ID_SONIC: this->HandleSuperColors_Sonic(true); break;
        case ID_TAILS: this->HandleSuperColors_Tails(true); break;
        case ID_KNUCKLES: this->HandleSuperColors_Knux(true); break;
    }

    bool32 allowDraw = true;

    if (ScreenWrap::CheckCompetitionWrap()) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ALL_ENTITIES)) {
            if (player != this && player->characterID == this->characterID) {
                if (sceneInfo->currentScreenID == player->playerID) {
                    int32 timer = Zone::sVars->timer >= 0 ? Zone::sVars->timer : (Zone::sVars->timer + 1);
                    if (Zone::sVars->timer - (timer & -2) != (player->playerID & 1)) {
                        allowDraw = false;
                        break;
                    }
                }
            }
        }
    }

    if (allowDraw) {
        int32 dirStore = this->direction;

        if (globals->gravityDir == CMODE_ROOF && this->animator.rotationStyle == Animator::RotateNone)
            this->direction = dirStore ^ FLIP_X;

        if (this->isGhost) {
            this->inkEffect = INK_BLEND;
            this->alpha     = 0x7F;
        }

        Entity *parent   = (Entity *)this->abilityPtrs[0];
        Vector2 posStore = this->position;
        if (this->state.Matches(&Player::State_FlyToPlayer) && parent) {
            this->position.x = parent->position.x;
            this->position.y = parent->position.y;
        }

        if (!this->tailFrames.Matches(nullptr)) {
            int32 rotStore       = this->rotation;
            int32 dirStore       = this->direction;
            bool32 groundedStore = this->onGround;

            this->rotation  = this->tailRotation;
            this->direction = this->tailDirection;
            this->onGround  = false;

            if (globals->gravityDir == CMODE_ROOF && this->tailAnimator.rotationStyle == Animator::RotateNone)
                this->direction ^= FLIP_X;

            Player::DrawSprite(this, &this->tailAnimator);

            this->rotation  = rotStore;
            this->direction = dirStore;
            this->onGround  = groundedStore;
        }

        Player::DrawSprite(this, &this->animator);

        if (this->state.Matches(&Player::State_FlyToPlayer) && parent) {
            this->position.x = posStore.x;
            this->position.y = posStore.y;
        }
        this->direction = dirStore;

        if (sVars->showHitbox) {
            Hitbox hitbox;

            Hitbox *innerBox = this->innerBox;
            if (!innerBox)
                innerBox = this->animator.GetHitbox(1);

            hitbox.left   = innerBox->left - 1;
            hitbox.top    = innerBox->top - 1;
            hitbox.right  = innerBox->right;
            hitbox.bottom = innerBox->bottom;
            DrawHelpers::DrawPath(this, &hitbox, this->onGround ? 0x00FF00 : 0xFFFF00);

            Hitbox *outerBox = this->outerBox;
            if (!outerBox)
                outerBox = this->animator.GetHitbox(0);

            hitbox.left   = outerBox->left - 1;
            hitbox.top    = outerBox->top - 1;
            hitbox.right  = outerBox->right;
            hitbox.bottom = outerBox->bottom;
            DrawHelpers::DrawPath(this, &hitbox, this->onGround ? 0xFF0000 : 0xFF00FF);
        }

        for (int32 i = 0; i < 32; ++i) {
            paletteBank[0].SetEntry(i, paletteStorage[1][i]);
            paletteBank[1].SetEntry(i, paletteStorage[0][i]);
        }

        ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Player::Draw), false);
    }
}

void Player::Create(void *data)
{
    if (sceneInfo->inEditor) {
        this->animator.SetAnimation(sVars->sonicFrames, ANI_IDLE, true, 0);
        this->characterID = ID_SONIC;
    }
    else {
        this->playerID = this->Slot();

        // Handle character specific stuff
        switch (this->characterID) {
            default:
            case ID_SONIC:
                this->aniFrames = sVars->sonicFrames;
                this->tailFrames.Init();
                this->jumpOffset = 0x50000;
                this->stateAbility.Set(&Player::Action_DblJumpSonic);

                this->sensorY = 0x140000;

                if (globals->medalMods & MEDAL_PEELOUT) {
                    this->statePeelout.Set(&Player::Action_Peelout);
                    for (int32 f = 0; f < 4; ++f) {
                        SpriteFrame *dst = this->aniFrames.GetFrame(ANI_DASH, f + 1);
                        SpriteFrame *src = this->aniFrames.GetFrame(ANI_PEELOUT, f);
                        *dst             = *src;
                    }
                }
                break;

            case ID_TAILS:
                this->aniFrames  = sVars->tailsFrames;
                this->tailFrames = sVars->tailsTailsFrames;
                this->jumpOffset = 0;
                this->stateAbility.Set(&Player::Action_DblJumpTails);
                this->sensorY = 0x100000;
                break;

            case ID_KNUCKLES:
                this->aniFrames = sVars->knuxFrames;
                this->tailFrames.Init();
                this->jumpOffset = 0x50000;
                this->stateAbility.Set(&Player::Action_DblJumpKnux);
                this->sensorY = 0x140000;
                break;
        }

        // Handle Sensor setup
        this->sensorX[0] = 0xA0000;
        this->sensorX[1] = 0x50000;
        this->sensorX[2] = 0;
        this->sensorX[3] = -0x50000;
        this->sensorX[4] = -0xA0000;

        this->active         = ACTIVE_NORMAL;
        this->tileCollisions = globals->tileCollisionMode;
        this->visible        = true;
        this->drawGroup      = Zone::sVars->playerDrawGroup[0];
        this->scale.x        = 0x200;
        this->scale.y        = 0x200;
        this->controllerID   = this->playerID + 1;
        this->state.Set(&Player::State_Ground);

        // Handle Input Setup
        if (!sceneInfo->entitySlot || globals->gameMode == MODE_COMPETITION) {
            this->stateInput.Set(&Player::Input_Gamepad);
        }
        else {
            Input::AssignInputSlotToDevice(this->controllerID, Input::INPUT_AUTOASSIGN);
            this->stateInput.Set(&Player::Input_AI_WaitForP1);
            this->sidekick = true;
        }

        analogStickInfoL[this->controllerID].deadzone = 0.3f;

        // Handle Powerups
        this->rings          = sVars->rings;
        this->ringExtraLife  = sVars->ringExtraLife;
        sVars->rings         = 0;
        sVars->ringExtraLife = 100;
        this->hyperRing      = (sVars->powerups >> 6) & 1;
        sVars->powerups &= ~0x40;
        if (sVars->powerups >= 0x80) {
            this->state.Set(&Player::State_StartSuper);
            sVars->powerups &= ~0x80;
        }

        if (sVars->powerups) {
            this->shield = sVars->powerups;
            ApplyShield();
        }
        sVars->powerups = 0;

        // Handle Lives/Score setup
        if (globals->gameMode == MODE_TIMEATTACK) {
            this->lives    = 1;
            this->score    = 0;
            this->score1UP = 50000;
        }
        else {
            this->lives    = sVars->savedLives;
            this->score    = sVars->savedScore;
            this->score1UP = sVars->savedScore1UP;
        }

        // Setup the target score
        int32 target = this->score1UP;
        while (this->score1UP <= target) this->score1UP += 50000;

        this->collisionLayers = Zone::sVars->collisionLayers;
        this->drawFX          = FX_ROTATE | FX_FLIP;
        this->animator.SetAnimation(this->aniFrames, ANI_IDLE, true, 0);

        UpdatePhysicsState();
        this->storedStateInput = this->stateInput;

        this->minJogVelocity  = this->spriteType == ManiaSprites ? 0x40000 : 0x60000;
        this->minRunVelocity  = this->spriteType == ManiaSprites ? 0x60000 : 0xA0000;
        this->minDashVelocity = 0xC0000;
    }
}

void Player::StageLoad()
{
    if (!globals->playerID)
        globals->playerID = ID_DEFAULT_PLAYER;

    sceneInfo->debugMode = globals->medalMods & MEDAL_DEBUGMODE;
    Dev::AddViewableVariable("Debug Mode", &sceneInfo->debugMode, Dev::VIEWVAR_BOOL, false, true);

    // TEMP
    sceneInfo->debugMode = true;

    if (globals->medalMods & MEDAL_ANDKNUCKLES) {
        globals->playerID &= 0xFF;
        globals->playerID |= ID_KNUCKLES_ASSIST;
    }

    sVars->playerCount       = 0;
    sVars->maxPlayerCount = 4;
    sVars->active            = ACTIVE_ALWAYS;

    // Sprite loading & characterID management
    if (globals->gameMode == MODE_COMPETITION)
        LoadSpritesVS();
    else
        LoadSprites();

    if (globals->gameMode == MODE_ENCORE) {
        sVars->playerCount = 2;
        Player *sidekick   = GameObject::Get<Player>(SLOT_PLAYER2);
        sidekick->playerID = 1;
    }
    else {
        sVars->playerCount = GameObject::Count<Player>(false);
    }

    // Handle Sidekick stuff setup
    sVars->nextLeaderPosID   = 1;
    sVars->lastLeaderPosID   = 0;
    sVars->disableP2KeyCheck = false;

    sVars->upState        = false;
    sVars->downState      = false;
    sVars->leftState      = false;
    sVars->rightState     = false;
    sVars->jumpPressState = false;
    sVars->jumpHoldState  = false;

    sVars->sfxJump.Get("Global/Jump.wav");
    sVars->sfxLoseRings.Get("Global/LoseRings.wav");
    sVars->sfxHurt.Get("Global/Hurt.wav");
    sVars->sfxRoll.Get("Global/Roll.wav");
    sVars->sfxCharge.Get("Global/Charge.wav");
    sVars->sfxRelease.Get("Global/Release.wav");
    sVars->sfxPeelCharge.Get("Global/PeelCharge.wav");
    sVars->sfxPeelRelease.Get("Global/PeelRelease.wav");
    sVars->sfxDropdash.Get("Global/DropDash.wav");
    sVars->sfxSkidding.Get("Global/Skidding.wav");
    sVars->sfxGrab.Get("Global/Grab.wav");
    sVars->sfxFlying.Get("Global/Flying.wav");
    sVars->sfxTired.Get("Global/Tired.wav");
    sVars->sfxLand.Get("Global/Land.wav");
    sVars->sfxSlide.Get("Global/Slide.wav");
    sVars->sfxTransform2.Get("Global/Transform2.wav");
    sVars->sfxEarthquake.Get("Global/Earthquake.wav"); // TODO: add this to the gameconfig for it to load

    sVars->lookUpDelay    = 60;
    sVars->lookUpDistance = 96;

    sVars->activeSuperSonicPalette        = sVars->superSonicPalette;
    sVars->activeSuperSonicPalette_Water  = sVars->superSonicPalette;
    sVars->activeHyperSonicPalette        = sVars->hyperSonicPalette;
    sVars->activeHyperSonicPalette_Water  = sVars->hyperSonicPalette;
    sVars->activeSuperTailsPalette        = sVars->superTailsPalette;
    sVars->activeSuperTailsPalette_Water  = sVars->superTailsPalette;
    sVars->activeSuperKnuxPalette         = sVars->superKnuxPalette;
    sVars->activeSuperKnuxPalette_Water   = sVars->superKnuxPalette;

    sVars->canSuperCB = nullptr;

    for (int32 p = 0; p < PLAYER_COUNT; ++p) sVars->gotHit[p] = false;
}

// Extra Entity Functions
void Player::LoadSprites()
{
    for (auto spawn : GameObject::GetEntities<Player>(FOR_ALL_ENTITIES)) {
        int32 playerID = GET_CHARACTER_ID(1);

        if (spawn->characterID & playerID) {
            spawn->characterID = GET_CHARACTER_ID(1);
            spawn->LoadPlayerSprites();

            Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);
            spawn->Copy(player1, true);
            player1->camera = Camera::SetTargetEntity(0, player1);
            Graphics::AddCamera(&player1->position, screenInfo->center.x << 16, screenInfo->center.y << 16, true);
        }
        else {
            spawn->Destroy();
        }
    }

    if (GET_CHARACTER_ID(2) > 0) {
        Player *leader   = GameObject::Get<Player>(SLOT_PLAYER1);
        Player *sidekick = GameObject::Get<Player>(SLOT_PLAYER2);

        for (int32 i = 0; i < 0x10; ++i) sVars->leaderPositionBuffer[i] = leader->position;

        sidekick->classID    = sVars->classID;
        sidekick->position.x = leader->position.x;
        sidekick->position.y = leader->position.y;

        if (globals->gameMode != MODE_TIMEATTACK) {
            Graphics::AddCamera(&sidekick->position, screenInfo->center.x << 16, screenInfo->center.y << 16, true);
            sidekick->position.x -= 0x100000;
        }

        sidekick->characterID = globals->playerID >> 8;
        sidekick->LoadPlayerSprites();
    }
}
void Player::LoadSpritesVS()
{
    // CompetitionSession *session = CompetitionSession::GetSession();
    //
    // for (auto spawn : GameObject::GetEntities<Player>(FOR_ALL_ENTITIES)) {
    //     if (spawn->characterID & ID_SONIC) {
    //         int32 slotID = 0;
    //         for (int32 i = 0; i < session->playerCount; ++i, ++slotID) {
    //             Player *player = GameObject::Get<Player>(slotID);
    //             spawn->Copy(player, true);
    //
    //             player->characterID = GET_CHARACTER_ID(1 + i);
    //             spawn->LoadPlayerSprites();
    //
    //             player->controllerID = i + 1;
    //             player->camera       = Camera::SetTargetEntity(i, player);
    //         }
    //     }
    //
    //     spawn->Destroy();
    // }
}

void Player::LoadPlayerSprites()
{
    switch (this->characterID) {
        default:
        case ID_SONIC:
            sVars->sonicFrames.Load("Players/Sonic.bin", SCOPE_STAGE);
            sVars->superFrames.Load("Players/SuperSonic.bin", SCOPE_STAGE);
            this->superColorIndex = 64;
            this->superColorCount = 6;
            break;

        case ID_TAILS:
            sVars->tailsFrames.Load("Players/Tails.bin", SCOPE_STAGE);
            sVars->tailsTailsFrames.Load("Players/TailSprite.bin", SCOPE_STAGE);
            this->superColorIndex = 70;
            this->superColorCount = 6;
            break;

        case ID_KNUCKLES:
            sVars->knuxFrames.Load("Players/Knux.bin", SCOPE_STAGE);
            this->superColorIndex = 80;
            this->superColorCount = 6;
            break;
    }

    this->spriteType = ManiaSprites;
}

void Player::DrawSprite(Player *self, RSDK::Animator *animator)
{
    Animator drawAnimator;
    memcpy(&drawAnimator, animator, sizeof(drawAnimator));

    Vector2 drawPos = self->position;
    int32 fxStore   = self->drawFX;
    int32 rotStore  = self->rotation;
    int32 dirStore  = self->direction;

    if (drawAnimator.rotationStyle == Animator::RotateStaticFrames) {
        drawAnimator.rotationStyle = Animator::RotateFull;

        if (rotStore >= 0x100)
            self->rotation = 8 - ((0x214 - rotStore) >> 6);
        else
            self->rotation = (rotStore + 20) >> 6;

        self->drawFX = fxStore | FX_ROTATE;

        switch (self->rotation) {
            default:
            case 0:
                self->rotation = 0;

                if (globals->tileCollisionMode != 1) {
                    if (dirStore & FLIP_X)
                        self->direction |= FLIP_X;
                    else
                        self->direction &= ~FLIP_X;
                }
                break;

            case 1:
                // increment "frameCount" frames
                // this should put the frameID on the equivelent frame in the "angled" ver of the anim
                drawAnimator.frameID += animator->frameCount;

                if (!(dirStore & FLIP_X)) {
                    self->rotation = 0x80;
                    if (globals->tileCollisionMode != 1)
                        self->direction = FLIP_NONE;
                }
                else {
                    self->rotation = 0;
                    if (globals->tileCollisionMode != 1)
                        self->direction = FLIP_X;
                }
                break;

            case 2:
                self->rotation = 0x80;
                if (self->onGround)
                    drawPos.x = self->position.x - 0x10000;
                break;

            case 3:
                // increment "frameCount" frames
                // this should put the frameID on the equivelent frame in the "angled" ver of the anim
                drawAnimator.frameID += animator->frameCount;

                if (dirStore & FLIP_X)
                    self->rotation = 0x280;
                else
                    self->rotation = 0x100;
                break;

            case 4:
                self->rotation = 0x100;
                if (self->onGround)
                    drawPos.y = self->position.y - 0x10000;
                break;

            case 5:
                // increment "frameCount" frames
                // this should put the frameID on the equivelent frame in the "angled" ver of the anim
                drawAnimator.frameID += animator->frameCount;

                if (!(dirStore & FLIP_X))
                    self->rotation = 0x180;
                else
                    self->rotation = 0x100;
                break;

            case 6: self->rotation = 0x180; break;

            case 7:
                // increment "frameCount" frames
                // this should put the frameID on the equivelent frame in the "angled" ver of the anim
                drawAnimator.frameID += animator->frameCount;

                if (dirStore & FLIP_X) {
                    self->rotation = 0x180;

                    if (globals->tileCollisionMode != 1)
                        self->direction |= FLIP_X;
                }
                else {
                    self->rotation = 0;

                    if (globals->tileCollisionMode != 1)
                        self->direction &= ~FLIP_X;
                }
                break;
        }
    }

    drawAnimator.DrawSprite(&drawPos, false);

    self->drawFX    = fxStore;
    self->rotation  = rotStore;
    self->direction = dirStore;
}

bool32 Player::CheckValidState()
{
    if (this->classID == Player::sVars->classID && !this->deathType) {
        if (!this->state.Matches(&Player::State_DeathHold) && !this->state.Matches(&Player::State_Death) && !this->state.Matches(&Player::State_Drown)
            && !this->state.Matches(&Player::State_ReturnToPlayer) && !this->state.Matches(&Player::State_HoldRespawn)
            && !this->state.Matches(&Player::State_FlyToPlayer) && !this->state.Matches(&Player::State_Transform)) {
            return true;
        }
    }

    return false;
}
void Player::SaveValues()
{
    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

    globals->restartLives[0] = sVars->savedLives = player1->lives;
    globals->restartScore = sVars->savedScore = player1->score;
    globals->restartScore1UP = sVars->savedScore1UP = player1->score1UP;

    NotifyCallback(NOTIFY_PLAYER_SAVED_VALUES, player1->score, 0, 0);
}
void Player::GiveScoreBonus(RSDK::Vector2 position)
{
    int32 scoreBonuses[16] = { 100, 200, 500, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 10000 };

    ScoreBonus *scoreBonus       = GameObject::Create<ScoreBonus>(0, position.x, position.y);
    scoreBonus->drawGroup        = Zone::sVars->objectDrawGroup[1];
    scoreBonus->animator.frameID = this->scoreBonus;

    if (this->scoreBonus < 15) {
        this->GiveScore(scoreBonuses[this->scoreBonus]);
        this->scoreBonus++;
    }
}
void Player::GiveScore(int32 score)
{
    Player *player = this;
    if (player->sidekick)
        player = GameObject::Get<Player>(SLOT_PLAYER1);

    player->score += score;
    if (player->score > 9999999)
        player->score = 9999999;

    if (player->score >= player->score1UP) {
        player->GiveLife();
        while (player->score1UP <= player->score) player->score1UP += 50000;
    }
}
void Player::GiveRings(int32 amount, bool32 playSfx)
{
    Player *player = this;
    if (player->sidekick)
        player = GameObject::Get<Player>(SLOT_PLAYER1);

    player->rings = CLAMP(player->rings + amount, 0, 999);

    if (player->rings >= player->ringExtraLife) {
        player->GiveLife();
        player->ringExtraLife += 100;
        // literally 1984
        if (player->ringExtraLife > 300)
            player->ringExtraLife = 1000;
    }

    if (playSfx) {
        if (Ring::sVars->pan) {
            int32 channel = Ring::sVars->sfxRing.Play();
            channels[channel].SetAttributes(1.0, -1.0, 1.0);
            Ring::sVars->pan = 0;
        }
        else {
            int32 channel = Ring::sVars->sfxRing.Play();
            channels[channel].SetAttributes(1.0, 1.0, 1.0);
            Ring::sVars->pan = 1;
        }
    }
}
void Player::GiveLife()
{
    if (globals->gameMode != MODE_TIMEATTACK && globals->gameMode != MODE_ENCORE) {
        Player *player = this;
        if (player->sidekick)
            player = GameObject::Get<Player>(SLOT_PLAYER1);

        if (player->lives < 99)
            player->lives++;

        Music::PlayJingle(Music::TRACK_1UP);
    }
}
void Player::UpdatePhysicsState()
{
    int32 *tablePtr  = nullptr;
    int32 tablePos   = 0;
    int32 decelShift = 0;

    switch (this->characterID) {
        default:
        case ID_SONIC: tablePtr = sVars->sonicPhysicsTable; break;
        case ID_TAILS: tablePtr = sVars->tailsPhysicsTable; break;
        case ID_KNUCKLES: tablePtr = sVars->knuxPhysicsTable; break;
    }

    switch (globals->gravityDir) {
        default: break;
        case CMODE_FLOOR:
        case CMODE_RWALL:
            if (this->underwater) {
                this->gravityStrength = 0x2000;
                tablePos              = 1;
                if (this->speedShoesTimer >= 0)
                    this->gravityStrength = 0x1000;
            }
            else {
                this->gravityStrength = 0x5800;
                if (this->speedShoesTimer >= 0)
                    this->gravityStrength = 0x3800;
            }
            break;

        case CMODE_LWALL:
        case CMODE_ROOF:
            if (this->underwater) {
                this->gravityStrength = -0x2000;
                tablePos              = 1;
                if (this->speedShoesTimer >= 0)
                    this->gravityStrength = -0x1000;
            }
            else {
                this->gravityStrength = -0x5800;
                if (this->speedShoesTimer >= 0)
                    this->gravityStrength = -0x3800;
            }
            break;
    }

    if (this->superState == Player::SuperStateSuper) {
        tablePos |= 2;
        decelShift = 2;
    }

    if (this->speedShoesTimer > 0) {
        tablePos |= 4;
        decelShift = 1;
    }

    int32 tableID             = 8 * tablePos;
    this->topSpeed            = tablePtr[tableID];
    this->acceleration        = tablePtr[tableID + 1];
    this->deceleration        = tablePtr[tableID + 1] >> decelShift;
    this->airAcceleration     = tablePtr[tableID + 2];
    this->airDeceleration     = tablePtr[tableID + 3];
    this->skidSpeed           = tablePtr[tableID + 4];
    this->rollingFriction     = tablePtr[tableID + 5];
    this->jumpStrength        = tablePtr[tableID + 6];
    this->jumpCap             = tablePtr[tableID + 7];
    this->rollingDeceleration = 0x2000;

    if (this->speedShoesTimer < 0) {
        this->topSpeed >>= 1;
        this->acceleration >>= 1;
        this->airAcceleration >>= 1;
        this->skidSpeed >>= 1;
        this->rollingFriction >>= 1;
        this->airDeceleration >>= 1;
    }

    if (this->isChibi) {
        this->topSpeed -= this->topSpeed >> 3;
        this->acceleration -= this->acceleration >> 4;
        this->airAcceleration -= this->airAcceleration >> 4;
        this->jumpStrength -= this->jumpStrength >> 3;
        this->jumpCap -= this->jumpCap >> 3;
    }
}
void Player::ApplyShield()
{
    Player *player = this;
    if (player->sidekick)
        player = GameObject::Get<Player>(SLOT_PLAYER1);

    if (player->shield) {
        if (player->superState != Player::SuperStateSuper && player->invincibleTimer <= 0) {
            Shield *shield = GameObject::Get<Shield>(sVars->maxPlayerCount + player->Slot());
            int32 frameID  = shield->shieldAnimator.frameID;
            int32 animID   = shield->shieldAnimator.animationID;
            int32 type     = shield->type;
            shield->Reset(Shield::sVars->classID, player);
            if (type == player->shield)
                shield->shieldAnimator.SetAnimation(Shield::sVars->aniFrames, animID, true, frameID);
        }
    }
    else {
        Shield *shield = GameObject::Get<Shield>(sVars->maxPlayerCount + player->Slot());
        if (shield->classID == Shield::sVars->classID) {
            if (!shield->state.Matches(&Shield::State_Insta))
                shield->Destroy();
        }
    }
}

void Player::ChangeCharacter(int32 character)
{
    uint16 playerID   = this->playerID;
    this->characterID = character;
    globals->playerID &= ~(255 << 8 * playerID);
    globals->playerID |= character << 8 * this->playerID;

    LoadPlayerSprites();

    switch (this->characterID) {
        default:
        case ID_SONIC:
            this->aniFrames = sVars->sonicFrames;
            this->tailFrames.Init();
            this->jumpOffset = 0x50000;
            this->stateAbility.Set(&Player::Action_DblJumpSonic);
            this->sensorY = 0x140000;

            if (globals->medalMods & MEDAL_PEELOUT) {
                this->statePeelout.Set(&Player::Action_Peelout);
                for (int32 f = 0; f < 4; ++f) {
                    SpriteFrame *dst = this->aniFrames.GetFrame(ANI_DASH, f + 1);
                    SpriteFrame *src = this->aniFrames.GetFrame(ANI_PEELOUT, f);
                    *dst             = *src;
                }
            }
            break;

        case ID_TAILS:
            this->aniFrames  = sVars->tailsFrames;
            this->tailFrames = sVars->tailsTailsFrames;
            this->jumpOffset = 0;
            this->stateAbility.Set(&Player::Action_DblJumpTails);
            this->sensorY = 0x100000;
            break;

        case ID_KNUCKLES:
            this->aniFrames = sVars->knuxFrames;
            this->tailFrames.Init();
            this->jumpOffset = 0x50000;
            this->stateAbility.Set(&Player::Action_DblJumpKnux);
            this->sensorY = 0x140000;
            break;
    }

    this->sensorX[0] = 0xA0000;
    this->sensorX[1] = 0x50000;
    this->sensorX[2] = 0;
    this->sensorX[3] = -0x50000;
    this->sensorX[4] = -0xA0000;

    if (this->state.Matches(&Player::State_KnuxWallClimb) || this->state.Matches(&Player::State_DropDash) || this->state.Matches(&Player::State_TailsFlight)
        || this->state.Matches(&Player::State_KnuxGlideDrop) || this->state.Matches(&Player::State_KnuxGlideLeft)
        || this->state.Matches(&Player::State_KnuxGlideRight) || this->state.Matches(&Player::State_KnuxGlideSlide)
        || this->state.Matches(&Player::State_KnuxLedgePullUp)) {
        this->state.Set(&Player::State_Air);
        this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
    }
    else {
        this->animator.SetAnimation(this->aniFrames, this->animator.animationID, false, 0);
    }

    if (this->superState != Player::SuperStateNone) {
        if (this->superState == Player::SuperStateSuper)
            TryTransform(true, this->isHyper ? Player::TransformHyper : Player::TransformSuper);
    }

    UpdatePhysicsState();
}

void Player::InvertGravity()
{
    if (globals->gravityDir)
        globals->tileCollisionMode = 1;
    else
        globals->tileCollisionMode = 2;
    globals->gravityDir = CMODE_ROOF * (globals->gravityDir == CMODE_FLOOR);

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->tileCollisions) {
            player->onGround = false;
            if (globals->gravityDir)
                player->tileCollisions = TILECOLLISION_UP;
            else
                player->tileCollisions = TILECOLLISION_DOWN;

            if (player->state.Matches(&Player::State_Ground)) {
                player->onGround      = false;
                player->groundedStore = false;
                player->state.Set(&Player::State_Air);
            }

            player->UpdatePhysicsState();
        }
    }

    for (auto ring : GameObject::GetEntities<Ring>(FOR_ACTIVE_ENTITIES)) {
        if (ring->tileCollisions) {
            if (globals->gravityDir)
                ring->tileCollisions = TILECOLLISION_UP;
            else
                ring->tileCollisions = TILECOLLISION_DOWN;
        }
    }

    for (auto itemBox : GameObject::GetEntities<ItemBox>(FOR_ACTIVE_ENTITIES)) {
        if (itemBox->tileCollisions) {
            if (globals->gravityDir)
                itemBox->tileCollisions = TILECOLLISION_UP;
            else
                itemBox->tileCollisions = TILECOLLISION_DOWN;
        }
    }

    for (auto animal : GameObject::GetEntities<Animals>(FOR_ACTIVE_ENTITIES)) {
        if (animal->tileCollisions) {
            if (globals->gravityDir)
                animal->tileCollisions = TILECOLLISION_UP;
            else
                animal->tileCollisions = TILECOLLISION_DOWN;
        }
    }
}

bool32 Player::TryTransform(bool32 fastTransform, TransformModes transformMode)
{
    if (transformMode == TransformAuto) {
        if (SaveGame::GetEmeralds(SaveGame::EmeraldBoth))
            transformMode = TransformHyper;
        else
            transformMode = TransformSuper;
    }

    if (!sceneInfo->timeEnabled)
        return false;

    if (sVars->canSuperCB) {
        if (!sVars->canSuperCB(false))
            return false;
    }

    if (this->superState <= Player::SuperStateFadeIn
        && (SaveGame::GetEmeralds(SaveGame::EmeraldChaosOnly) || SaveGame::GetEmeralds(SaveGame::EmeraldBoth))) {
        if (transformMode == TransformEmeralds && (!fastTransform || this->rings < 50))
            return false;
    }
    else if (transformMode == TransformEmeralds) {
        return false;
    }

    sVars->sfxSwapFail.Stop();

    if (this->characterID == ID_SONIC && !this->isChibi)
        this->aniFrames = sVars->superFrames;

    if (fastTransform) {
        this->superState        = Player::SuperStateSuper;
        this->hyperAbilityState = SaveGame::GetEmeralds(SaveGame::EmeraldBoth);
        if (this->hyperAbilityState != Player::HyperStateNone)
            this->isHyper = true;

        if (transformMode == TransformHyper) {
            this->hyperAbilityState = Player::HyperStateActive;
            this->isHyper           = true;
        }
        else if (transformMode == TransformSuper) {
            this->hyperAbilityState = Player::HyperStateNone;
            this->isHyper           = false;
        }

        UpdatePhysicsState();

        if (this->hyperAbilityState == Player::HyperStateNone) {
            SuperSparkle *sparkle = GameObject::Get<SuperSparkle>(this->playerID + sVars->maxPlayerCount);
            sparkle->Reset(SuperSparkle::sVars->classID, this);

            if (globals->useManiaBehavior) {
                ImageTrail *trail = GameObject::Get<ImageTrail>(this->playerID + sVars->maxPlayerCount * 2);
                trail->Reset(ImageTrail::sVars->classID, this);
            }
        }
        else {
            SuperSparkle *sparkle = GameObject::Get<SuperSparkle>(this->playerID + sVars->maxPlayerCount);
            sparkle->Reset(SuperSparkle::sVars->classID, this);

            ImageTrail *trail = GameObject::Get<ImageTrail>(this->playerID + sVars->maxPlayerCount * 2);
            trail->Reset(ImageTrail::sVars->classID, this);
        }
    }
    else {
        if (this->isChibi)
            this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
        else
            this->animator.SetAnimation(this->aniFrames, ANI_TRANSFORM, false, 0);

        this->invincibleTimer  = 60;
        this->abilityValues[6] = this->velocity.x;
        this->abilityValues[7] = this->velocity.y;
        this->velocity.x       = 0;
        this->velocity.y       = 0;
        this->groundVel        = 0;
        this->onGround         = false;
        this->interaction      = false;
        this->nextGroundState.Set(nullptr);
        this->nextAirState.Set(nullptr);
        this->state.Set(&Player::State_Transform);
        this->isTransforming = true;
        if (globals->superMusicEnabled && Music::sVars->restartTrackID < 0) {
            this->drownTimer = 0;
            Music::ClearMusicStack();
            Music::PlayOnFade(Music::TRACK_SUPER, 0.035f);
        }
        this->jumpAbilityState = 0;

        this->hyperAbilityState = SaveGame::GetEmeralds(SaveGame::EmeraldBoth);
        if (this->hyperAbilityState != Player::HyperStateNone)
            this->isHyper = true;

        if (transformMode == TransformHyper) {
            this->hyperAbilityState = Player::HyperStateActive;
            this->isHyper           = true;
        }
        else if (transformMode == TransformSuper) {
            this->hyperAbilityState = Player::HyperStateNone;
            this->isHyper           = false;
        }

        this->superState = Player::SuperStateFadeIn;
    }

    this->superRingLossTimer = 60;
    this->superBlendAmount   = 0;
    this->superBlendState    = 0;
    this->timer              = 0;
    this->outtaHereTimer     = 0;
    this->superRingLossTimer = 60;
    this->superBlendAmount   = 0;
    this->superBlendState    = 0;
    this->timer              = 0;
    this->outtaHereTimer     = 0;

    if (globals->secrets & SECRET_SUPERDASH)
        this->stateAbility.Set(&Player::Action_SuperDash);

    return true;
}

Player *Player::GetNearestPlayerX()
{
    GameObject::Entity *self = (GameObject::Entity *)sceneInfo->entity;

    Player *targetPlayer = GameObject::Get<Player>(SLOT_PLAYER1);
    int32 targetDistance = 0x7FFFFFFF;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 distX = abs(player->position.x - self->position.x);
        if (distX < targetDistance) {
            targetDistance = distX;
            targetPlayer   = player;
        }
    }

    return targetPlayer;
}
Player *Player::GetNearestPlayerXY()
{
    GameObject::Entity *self = (GameObject::Entity *)sceneInfo->entity;

    Player *targetPlayer = GameObject::Get<Player>(SLOT_PLAYER1);
    int32 targetDistance = 0x7FFFFFFF;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 distX = abs(player->position.x - self->position.x);
        int32 distY = abs(player->position.y - self->position.y);
        if (distX + distY < targetDistance) {
            targetDistance = distX + distY;
            targetPlayer   = player;
        }
    }

    return targetPlayer;
}

void Player::HandleIdleAnimation_Classic()
{
    switch (this->characterID) {
        case ID_SONIC:
            if (this->spriteType == ClassicSprites) {
                if (this->outtaHereTimer >= 240 && this->superState == Player::SuperStateNone) {
                    this->animator.SetAnimation(this->aniFrames, ANI_BORED_1, false, 0);
                }
                else {
                    this->outtaHereTimer++;
                    this->animator.SetAnimation(this->aniFrames, ANI_IDLE, false, 0);
                }
            }
            else {
                if (this->superState != Player::SuperStateSuper) {
                    if (this->outtaHereTimer != 720 || this->isChibi || this->superState == Player::SuperStateSuper) {
                        if (this->outtaHereTimer < 240) {
                            this->outtaHereTimer++;
                            this->animator.SetAnimation(this->aniFrames, ANI_IDLE, false, 0);
                        }
                        else {
                            this->outtaHereTimer++;
                            if (this->animator.animationID == ANI_BORED_1) {
                                if (this->animator.frameID == 41)
                                    this->outtaHereTimer = 0;
                            }
                            else
                                this->animator.SetAnimation(this->aniFrames, ANI_BORED_1, false, 0);
                        }
                    }
                    else {
                        if (this->animator.animationID == ANI_BORED_2) {
                            if (this->animator.frameID == 67)
                                this->outtaHereTimer = 0;
                        }
                        else
                            this->animator.SetAnimation(this->aniFrames, ANI_BORED_2, false, 0);
                    }
                }
                else {
                    this->animator.SetAnimation(this->aniFrames, ANI_IDLE, false, 0);
                }
            }
            break;

        case ID_TAILS:
            if (this->outtaHereTimer < 240) {
                this->outtaHereTimer++;
                this->animator.SetAnimation(this->aniFrames, ANI_IDLE, false, 0);
            }
            else if (this->animator.animationID == ANI_BORED_1) {
                if (this->animator.frameID == 45)
                    this->outtaHereTimer = 0;
            }
            else {
                this->animator.SetAnimation(this->aniFrames, ANI_BORED_1, false, 0);
            }
            break;

        case ID_KNUCKLES:
            if (this->outtaHereTimer < 240) {
                this->outtaHereTimer++;
                this->animator.SetAnimation(this->aniFrames, ANI_IDLE, false, 0);
            }
            else if (this->animator.animationID == ANI_BORED_1) {
                if (this->animator.frameID == 69)
                    this->outtaHereTimer = 0;
            }
            else {
                this->animator.SetAnimation(this->aniFrames, ANI_BORED_1, false, 0);
            }
            break;

        default: break;
    }
}

void Player::HandleGroundAnimation_Classic()
{
    if (this->skidding > 0) {
        int32 animID = this->animator.animationID & 0xFFFE;

        if (animID != ANI_SKID) {
            if (this->state.Matches(&Player::State_Ground)) {
                this->animator.SetAnimation(this->aniFrames, ANI_SKID, false, 0);
                if (abs(this->groundVel) >= 0x60000) {
                    if (abs(this->groundVel) >= 0xA0000)
                        this->animator.speed = 64;
                    else
                        this->animator.speed = 144;
                }
                else {
                    this->skidding -= 8;
                }

                sVars->sfxSkidding.Play();
                Dust *dust = GameObject::Create<Dust>(this, this->position.x, this->position.y);
                dust->state.Set(&Dust::State_DustTrail);
            }
        }
        else {
            if (animID == ANI_SKID) {
                int32 spd = this->animator.speed;
                if (this->direction & FLIP_X) {
                    if (this->groundVel >= 0) {
                        this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 0);
                        this->skidding = 0;
                    }
                }
                else if (this->groundVel <= 0) {
                    this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 0);
                    this->skidding = 0;
                }

                this->animator.speed = spd;
            }
        }

        if (this->animator.frameID == this->animator.frameCount - 1) {
            this->skidding = 1;
            this->direction ^= FLIP_X;
            this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 0);
        }

        --this->skidding;
    }
    else {
        if (this->pushing > -3 && this->pushing < 3) {
            int32 angle = (this->angle & 0xFF) + (globals->gravityDir << 6);
            if (this->groundVel || (angle >= 0x20 && angle <= 0xE0 && !this->invertGravity)) {
                this->timer          = 0;
                this->outtaHereTimer = 0;

                int32 extraVel = this->superState == Player::SuperStateSuper ? 0x20000 : 0x00000;
                int32 velocity = abs(this->groundVel);

                bool32 allowDash =
                    ((globals->playerID & 0xFF) == ID_TAILS) || ((globals->playerID & 0xFF) == ID_SONIC && (globals->medalMods & MEDAL_PEELOUT));

                if (velocity < this->minJogVelocity + extraVel) {
                    if (this->animator.animationID == ANI_JOG) {
                        if (this->animator.frameID == 9)
                            this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 9);
                    }
                    else if (this->animator.animationID == ANI_FALL) {
                        this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, this->animator.frameID);
                    }
                    else {
                        this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 0);
                    }
                    this->animator.speed = 60 * abs(this->groundVel) / 0x60000 + 20;
                    this->minJogVelocity = 0x60000;
                    this->minRunVelocity = this->spriteType == ManiaSprites ? 0xA0000 : 0x70000;
                }
                else if (velocity < this->minRunVelocity + extraVel || !allowDash) {
                    this->animator.SetAnimation(this->aniFrames, ANI_RUN, false, 0);

                    if (this->spriteType == ManiaSprites) {
                        this->animator.speed = MIN(80 * abs(this->groundVel) / 0x60000, 0x200);
                        this->minJogVelocity = 0x58000;
                        this->minRunVelocity = 0xA0000;
                    }
                    else {
                        if (abs(this->groundVel) <= 0x60000 + 2 * this->acceleration)
                            this->animator.speed = 80;
                        else if (abs(this->groundVel) >= 0x70000)
                            this->animator.speed = 240;
                        else
                            this->animator.speed = 160;

                        this->minJogVelocity = 0x58000;
                        if ((globals->medalMods & MEDAL_PEELOUT) && (globals->playerID & 0xFF) == ID_SONIC)
                            this->minRunVelocity = 0x98000;
                        else
                            this->minRunVelocity = 0x68000;
                    }
                }
                else if (allowDash) {
                    this->animator.SetAnimation(this->aniFrames, ANI_DASH, false, 0);

                    this->minJogVelocity = 0x58000;
                    if ((this->spriteType == ManiaSprites || (globals->medalMods & MEDAL_PEELOUT)) && (globals->playerID & 0xFF) == ID_SONIC)
                        this->minRunVelocity = 0x98000;
                    else
                        this->minRunVelocity = 0x68000;
                }
            }
            else {
                Vector2 posStore = this->position;

                if (this->flailing >= 0) {
                    switch (globals->gravityDir) {
                        case CMODE_FLOOR:
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_FLOOR, this->collisionPlane, this->sensorX[0], this->sensorY, 10) << 0;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_FLOOR, this->collisionPlane, this->sensorX[1], this->sensorY, 10) << 1;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_FLOOR, this->collisionPlane, this->sensorX[2], this->sensorY, 10) << 2;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_FLOOR, this->collisionPlane, this->sensorX[3], this->sensorY, 10) << 3;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_FLOOR, this->collisionPlane, this->sensorX[4], this->sensorY, 10) << 4;
                            break;

                        case CMODE_LWALL:
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_LWALL, this->collisionPlane, this->sensorY, this->sensorX[0], 10) << 0;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_LWALL, this->collisionPlane, this->sensorY, this->sensorX[1], 10) << 1;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_LWALL, this->collisionPlane, this->sensorY, this->sensorX[2], 10) << 2;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_LWALL, this->collisionPlane, this->sensorY, this->sensorX[3], 10) << 3;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_LWALL, this->collisionPlane, this->sensorY, this->sensorX[4], 10) << 4;
                            break;

                        case CMODE_ROOF:
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_ROOF, this->collisionPlane, this->sensorX[0], -this->sensorY, 10) << 0;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_ROOF, this->collisionPlane, this->sensorX[1], -this->sensorY, 10) << 1;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_ROOF, this->collisionPlane, this->sensorX[2], -this->sensorY, 10) << 2;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_ROOF, this->collisionPlane, this->sensorX[3], -this->sensorY, 10) << 3;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_ROOF, this->collisionPlane, this->sensorX[4], -this->sensorY, 10) << 4;
                            break;

                        case CMODE_RWALL:
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_RWALL, this->collisionPlane, -this->sensorY, this->sensorX[0], 10) << 0;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_RWALL, this->collisionPlane, -this->sensorY, this->sensorX[1], 10) << 1;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_RWALL, this->collisionPlane, -this->sensorY, this->sensorX[2], 10) << 2;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_RWALL, this->collisionPlane, -this->sensorY, this->sensorX[3], 10) << 3;
                            this->flailing |=
                                this->TileGrip(this->collisionLayers, CMODE_RWALL, this->collisionPlane, -this->sensorY, this->sensorX[4], 10) << 4;
                            break;
                        default: break;
                    }
                }

                this->position = posStore;
                switch (this->flailing) {
                    default:
                    case 0b00000000: HandleIdleAnimation(); break;

                    case 0b00000001:
                        this->direction = (this->direction & ~FLIP_X) | (globals->gravityDir == CMODE_FLOOR);

                        if (this->characterID == ID_SONIC) {
                            if (this->superState == Player::SuperStateSuper)
                                this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_1, false, 0);
                            else
                                this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_2, false, 0);
                        }
                        else {
                            this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_1, false, 0);
                        }
                        break;

                    case 0b00000010:
                    case 0b00000011:
                        this->direction = (this->direction & ~FLIP_X) | (globals->gravityDir == CMODE_FLOOR);

                        this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_1, false, 0);
                        break;

                    case 0b00001000:
                    case 0b00011000:
                        this->direction = (this->direction & ~FLIP_X) | (globals->gravityDir != CMODE_FLOOR);

                        this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_1, false, 0);
                        break;

                    case 0b00010000:
                        this->direction = (this->direction & ~FLIP_X) | (globals->gravityDir != CMODE_FLOOR);

                        if (this->characterID == ID_SONIC) {
                            if (this->superState == Player::SuperStateSuper)
                                this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_1, false, 0);
                            else
                                this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_2, false, 0);
                        }
                        else {
                            this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_1, false, 0);
                        }
                        break;
                }
            }
        }
        else {
            this->pushing = CLAMP(this->pushing, -3, 3);
            this->animator.SetAnimation(this->aniFrames, ANI_PUSH, false, 0);
        }
    }

    this->flailing = 0;
}

void Player::HandleIdleAnimation()
{
    switch (this->characterID) {
        case ID_SONIC:
            if (this->timer != 720 || this->isChibi || this->superState == Player::SuperStateSuper) {
                if (this->timer < 240) {
                    this->timer++;
                    this->animator.SetAnimation(this->aniFrames, ANI_IDLE, false, 0);
                }
                else {
                    this->timer++;
                    if (this->animator.animationID == ANI_BORED_1) {
                        if (this->animator.frameID == 41)
                            this->timer = 0;
                    }
                    else
                        this->animator.SetAnimation(this->aniFrames, ANI_BORED_1, false, 0);
                }
            }
            else {
                if (this->animator.animationID == ANI_BORED_2) {
                    if (this->animator.frameID == 67)
                        this->timer = 0;
                }
                else
                    this->animator.SetAnimation(this->aniFrames, ANI_BORED_2, false, 0);
            }
            break;

        case ID_TAILS:
            if (this->timer < 240) {
                this->timer++;
                this->animator.SetAnimation(this->aniFrames, ANI_IDLE, false, 0);
            }
            else if (this->animator.animationID == ANI_BORED_1) {
                if (this->animator.frameID == 45)
                    this->timer = 0;
            }
            else {
                this->animator.SetAnimation(this->aniFrames, ANI_BORED_1, false, 0);
            }
            break;

        case ID_KNUCKLES:
            if (this->timer < 240) {
                this->timer++;
                this->animator.SetAnimation(this->aniFrames, ANI_IDLE, false, 0);
            }
            else if (this->animator.animationID == ANI_BORED_1) {
                if (this->animator.frameID == 69)
                    this->timer = 0;
            }
            else {
                this->animator.SetAnimation(this->aniFrames, ANI_BORED_1, false, 0);
            }
            break;

        default: break;
    }
}
void Player::HandleGroundAnimation()
{
    if (this->skidding > 0) {
        if (this->animator.animationID != ANI_SKID) {
            if (this->animator.animationID == ANI_SKID_TURN) {
                if (this->animator.frameID == this->animator.frameCount - 1) {
                    this->direction ^= FLIP_X;
                    this->skidding = 1;
                    this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 0);
                }
            }
            else {
                this->animator.SetAnimation(this->aniFrames, ANI_SKID, false, 0);
                if (abs(this->groundVel) >= 0x60000) {
                    if (abs(this->groundVel) >= 0xA0000)
                        this->animator.speed = 64;
                    else
                        this->animator.speed = 144;
                }
                else {
                    this->skidding -= 8;
                }

                sVars->sfxSkidding.Play();
                Dust *dust = GameObject::Create<Dust>(this, this->position.x, this->position.y);
                dust->state.Set(&Dust::State_DustTrail);
            }
        }
        else {
            int32 spd = this->animator.speed;
            if (this->direction) {
                if (this->groundVel >= 0) {
                    this->animator.SetAnimation(this->aniFrames, ANI_SKID_TURN, false, 0);
                }
            }
            else if (this->groundVel <= 0) {
                this->animator.SetAnimation(this->aniFrames, ANI_SKID_TURN, false, 0);
            }

            this->animator.speed = spd;
        }

        --this->skidding;
    }
    else {
        if (this->pushing > -3 && this->pushing < 3) {
            if (this->groundVel || (this->angle >= 0x20 && this->angle <= 0xE0 && !this->invertGravity)) {
                this->timer          = 0;
                this->outtaHereTimer = 0;

                int32 velocity = abs(this->groundVel);
                if (velocity < this->minJogVelocity) {
                    if (this->animator.animationID == ANI_JOG) {
                        if (this->animator.frameID == 9)
                            this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 9);
                    }
                    else if (this->animator.animationID == ANI_FALL) {
                        this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, this->animator.frameID);
                    }
                    else {
                        this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 0);
                    }
                    this->animator.speed = (velocity >> 12) + 48;
                    this->minJogVelocity = 0x40000;
                }
                else if (velocity < this->minRunVelocity) {
                    if (this->animator.animationID != ANI_WALK || this->animator.frameID == 3)
                        this->animator.SetAnimation(this->aniFrames, ANI_JOG, false, 0);
                    this->animator.speed = (velocity >> 12) + 0x40;
                    this->minJogVelocity = 0x38000;
                    this->minRunVelocity = 0x60000;
                }
                else if (velocity < this->minDashVelocity) {
                    if (this->animator.animationID == ANI_DASH || this->animator.animationID == ANI_RUN)
                        this->animator.SetAnimation(this->aniFrames, ANI_RUN, false, 0);
                    else
                        this->animator.SetAnimation(this->aniFrames, ANI_RUN, false, 1);

                    this->animator.speed  = MIN((velocity >> 12) + 0x60, 0x200);
                    this->minRunVelocity  = 0x58000;
                    this->minDashVelocity = 0xC0000;
                }
                else {
                    if (this->animator.animationID == ANI_DASH || this->animator.animationID == ANI_RUN)
                        this->animator.SetAnimation(this->aniFrames, ANI_DASH, false, 0);
                    else
                        this->animator.SetAnimation(this->aniFrames, ANI_DASH, false, 1);
                    this->minDashVelocity = 0xB8000;
                }
            }
            else {
                this->minJogVelocity  = 0x40000;
                this->minRunVelocity  = 0x60000;
                this->minDashVelocity = 0xC0000;

                Vector2 posStore = this->position;

                this->flailing |= this->TileGrip(this->collisionLayers, CMODE_FLOOR, this->collisionPlane, this->sensorX[0], this->sensorY, 10) << 0;
                this->flailing |= this->TileGrip(this->collisionLayers, CMODE_FLOOR, this->collisionPlane, this->sensorX[1], this->sensorY, 10) << 1;
                this->flailing |= this->TileGrip(this->collisionLayers, CMODE_FLOOR, this->collisionPlane, this->sensorX[2], this->sensorY, 10) << 2;
                this->flailing |= this->TileGrip(this->collisionLayers, CMODE_FLOOR, this->collisionPlane, this->sensorX[3], this->sensorY, 10) << 3;
                this->flailing |= this->TileGrip(this->collisionLayers, CMODE_FLOOR, this->collisionPlane, this->sensorX[4], this->sensorY, 10) << 4;

                this->position = posStore;
                switch (this->flailing) {
                    case 0b00000001:
                    case 0b00000011:
                        if (this->direction == FLIP_X || (this->characterID == ID_SONIC && this->superState == Player::SuperStateSuper)
                            || this->isChibi) {
                            this->direction = FLIP_X;
                            this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_1, false, 0);
                        }
                        else {
                            this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_2, false, 0);
                        }
                        break;

                    case 0b00010000:
                    case 0b00011000:
                        if (this->direction && (this->characterID != ID_SONIC || this->superState != Player::SuperStateSuper) && !this->isChibi) {
                            this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_2, false, 0);
                        }
                        else {
                            this->direction = FLIP_NONE;
                            this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_1, false, 0);
                        }
                        break;

                    // bit 5 & 6 are never set, this code cannot be reached
                    // in theory, bit 5 & 6 would be set using sensorLC & sensorLR equivalents from v4/S2/S3
                    case 0b01101111:
                    case 0b01110100: this->animator.SetAnimation(this->aniFrames, ANI_BALANCE_1, false, 0); break;

                    // Not balancing
                    case 0b00000000:
                    default: HandleIdleAnimation(); break;
                }
            }
        }
        else {
            this->pushing = CLAMP(this->pushing, -3, 3);
            this->animator.SetAnimation(this->aniFrames, ANI_PUSH, false, 0);
        }
    }
}

void Player::HandleGroundMovement()
{
    uint8 angle = this->angle + (globals->gravityDir << 6);
    if (this->controlLock > 0) {
        this->controlLock--;

        if (!this->invertGravity)
            this->groundVel += Math::Sin256(angle) << 13 >> 8;
    }
    else {
        bool32 left  = false;
        bool32 right = false;

        if (globals->gravityDir == CMODE_ROOF || this->invertGravity) {
            right = this->left;
            left  = this->right;
        }
        else {
            left  = this->left;
            right = this->right;
        }

        if (left) {
            if (this->groundVel > -this->topSpeed) {
                if (this->groundVel <= 0) {
                    this->groundVel -= this->acceleration;
                }
                else {
                    if (this->collisionMode == globals->gravityDir && this->groundVel > 0x40000 && !Zone::sVars->autoScrollSpeed) {
                        this->direction &= ~FLIP_X;
                        this->skidding = 24;
                    }

                    if (this->groundVel < this->skidSpeed)
                        this->groundVel = -abs(this->skidSpeed);
                    else
                        this->groundVel -= this->skidSpeed;
                }
            }

            if (this->groundVel <= 0 && this->skidding < 1)
                this->direction |= FLIP_X;
        }

        if (right) {
            if (this->groundVel < this->topSpeed) {
                if (this->groundVel >= 0) {
                    this->groundVel += this->acceleration;
                }
                else {
                    if (this->collisionMode == globals->gravityDir && this->groundVel < -0x40000 && !Zone::sVars->autoScrollSpeed) {
                        this->direction |= FLIP_X;
                        this->skidding = 24;
                    }

                    if (this->groundVel > -this->skidSpeed)
                        this->groundVel = abs(this->skidSpeed);
                    else
                        this->groundVel += this->skidSpeed;
                }
            }

            if (this->groundVel >= 0 && this->skidding < 1)
                this->direction &= ~FLIP_X;
        }

        if (this->left || this->right) {
            if (!this->invertGravity) {
                this->groundVel += Math::Sin256(angle) << 13 >> 8;

                if (this->right) {
                    if (!this->left) {
                        if (angle > 0xC0 && angle < 0xE4 && this->groundVel > -0x20000 && this->groundVel < 0x28000) {
                            this->controlLock = 30;
                        }
                    }
                }
                else if (this->left) {
                    if (angle > 0x1C && angle < 0x40 && this->groundVel > -0x28000 && this->groundVel < 0x20000) {
                        this->controlLock = 30;
                    }
                }
            }
        }
        else {
            if (this->groundVel <= 0) {
                this->groundVel += this->deceleration;

                if (this->groundVel > 0)
                    this->groundVel = 0;
            }
            else {
                this->groundVel -= this->deceleration;

                if (this->groundVel < 0)
                    this->groundVel = 0;
            }

            if (!this->invertGravity) {
                if (this->groundVel > 0x2000 || this->groundVel < -0x2000)
                    this->groundVel += Math::Sin256(this->angle + (globals->gravityDir << 6)) << 13 >> 8;

                if (angle > 0xC0 && angle < 0xE4) {
                    if (abs(this->groundVel) < 0x10000)
                        this->controlLock = 30;
                }

                if (angle > 0x1C && angle < 0x40) {
                    if (abs(this->groundVel) < 0x10000)
                        this->controlLock = 30;
                }
            }
        }
    }

    if (!this->invertGravity && angle >= 0x40 && angle <= 0xC0 && abs(this->groundVel) < 0x20000) {
        this->velocity.x    = this->groundVel * Math::Cos256(this->angle) >> 8;
        this->velocity.y    = this->groundVel * Math::Sin256(this->angle) >> 8;
        this->onGround      = false;
        this->angle         = globals->gravityDir << 6;
        this->collisionMode = globals->gravityDir;
    }
}
void Player::HandleGroundRotation()
{
    if (this->animator.rotationStyle == Animator::RotateStaticFrames) {
        this->rotation = this->angle << 1;
    }
    else if (this->angle <= 0x04 || this->angle >= 0xFC) {
        this->rotation = 0;
    }
    else {
        int32 targetRotation = 0;
        if (this->angle > 0x10 && this->angle < 0xE8)
            targetRotation = this->angle << 1;

        int32 rotate = targetRotation - this->rotation;
        int32 shift  = (abs(this->groundVel) <= 0x60000) + 1;

        if (abs(rotate) >= abs(rotate - 0x200)) {
            if (abs(rotate - 0x200) < abs(rotate + 0x200))
                this->rotation += (rotate - 0x200) >> shift;
            else
                this->rotation += (rotate + 0x200) >> shift;
        }
        else {
            if (abs(rotate) < abs(rotate + 0x200))
                this->rotation += rotate >> shift;
            else
                this->rotation += (rotate + 0x200) >> shift;
        }

        this->rotation &= 0x1FF;
    }
}
void Player::HandleAirFriction()
{
    if (this->velocity.y > -0x40000 && this->velocity.y < 0)
        this->velocity.x -= this->velocity.x >> 5;

    if (this->left) {
        if (this->velocity.x > -this->topSpeed)
            this->velocity.x -= this->airAcceleration;

        if (globals->gravityDir == CMODE_ROOF)
            this->direction &= ~FLIP_X;
        else
            this->direction |= FLIP_X;
    }

    if (this->right) {
        if (this->velocity.x < this->topSpeed)
            this->velocity.x += this->airAcceleration;

        if (globals->gravityDir == CMODE_ROOF)
            this->direction |= FLIP_X;
        else
            this->direction &= ~FLIP_X;
    }
}
void Player::HandleRollDeceleration()
{
    bool32 left  = false;
    bool32 right = false;

    if (globals->gravityDir == CMODE_ROOF || this->invertGravity) {
        right = this->left;
        left  = this->right;
    }
    else {
        left  = this->left;
        right = this->right;
    }

    int32 initialVel = this->groundVel;
    if (right && this->groundVel < 0)
        this->groundVel += this->rollingDeceleration;

    if (left && this->groundVel > 0)
        this->groundVel -= this->rollingDeceleration;

    int32 angle = this->angle + (globals->gravityDir << 6);
    if (this->groundVel) {
        if (this->groundVel < 0) {
            this->groundVel += this->rollingFriction;

            if (Math::Sin256(angle) >= 0)
                this->groundVel += 0x1400 * Math::Sin256(angle) >> 8;
            else
                this->groundVel += 0x5000 * Math::Sin256(angle) >> 8;

            if (this->groundVel < -0x120000)
                this->groundVel = -0x120000;
        }
        else {
            this->groundVel -= this->rollingFriction;

            if (Math::Sin256(angle) <= 0)
                this->groundVel += 0x1400 * Math::Sin256(angle) >> 8;
            else
                this->groundVel += 0x5000 * Math::Sin256(angle) >> 8;

            if (this->groundVel > 0x120000)
                this->groundVel = 0x120000;
        }
    }
    else {
        this->groundVel += 0x5000 * Math::Sin256(angle) >> 8;
    }

    switch (this->collisionMode) {
        case CMODE_FLOOR:
            if (this->state.Matches(&Player::State_TubeRoll)) {
                if (abs(this->groundVel) < 0x10000) {
                    if (this->direction & FLIP_Y)
                        this->groundVel = -0x40000;
                    else
                        this->groundVel = 0x40000;
                }
            }
            else {
                if ((this->groundVel >= 0 && initialVel <= 0) || (this->groundVel <= 0 && initialVel >= 0)) {
                    this->groundVel = 0;
                    this->state.Set(&Player::State_Ground);
                    // this->animator.SetAnimation(this->aniFrames, ANI_IDLE, true, 0);
                    // this->position.y -= this->jumpOffset;
                }
            }
            break;

        case CMODE_ROOF:
            if (this->invertGravity) {
                if ((this->groundVel >= 0 && initialVel <= 0) || (this->groundVel <= 0 && initialVel >= 0)) {
                    this->groundVel = 0;
                    this->state.Set(&Player::State_Ground);
                    // this->animator.SetAnimation(this->aniFrames, ANI_IDLE, true, 0);
                    // this->position.y += this->jumpOffset;
                }
            }
            else {
                if (this->state.Matches(&Player::State_TubeRoll)) {
                    if (abs(this->groundVel) < 0x10000) {
                        if (this->direction & FLIP_Y)
                            this->groundVel = -0x40000;
                        else
                            this->groundVel = 0x40000;
                    }
                }
                else {
                    if ((this->groundVel >= 0 && initialVel <= 0) || (this->groundVel <= 0 && initialVel >= 0)) {
                        this->groundVel = 0;
                        this->state.Set(&Player::State_Ground);
                        this->animator.SetAnimation(this->aniFrames, ANI_IDLE, true, 0);
                        this->position.y += this->jumpOffset;
                    }
                }
            }

        case CMODE_LWALL:
        case CMODE_RWALL: break;
    }

    if (angle >= 0x40 && angle <= 0xC0 && abs(this->groundVel) < 0x20000) {
        this->velocity.x    = this->groundVel * Math::Cos256(this->angle) >> 8;
        this->velocity.y    = this->groundVel * Math::Sin256(this->angle) >> 8;
        this->onGround      = false;
        this->angle         = globals->gravityDir << 6;
        this->collisionMode = globals->gravityDir;
    }
}

void Player::CheckStartFlyCarry(Player *leader)
{
    if (this->flyCarryTimer > 0)
        this->flyCarryTimer--;

    int32 off = this->position.y;

    switch (globals->gravityDir) {
        default: break;

        case CMODE_FLOOR:
            off += 0x1C0000;
            if (leader->animator.animationID == ANI_JUMP)
                off += this->jumpOffset;
            break;

        case CMODE_ROOF:
            off -= 0x1C0000;
            if (leader->animator.animationID == ANI_JUMP)
                off -= this->jumpOffset;
            break;
    }

    if (!leader->state.Matches(&Player::State_FlyCarried) && (!leader->onGround || this->velocity.y < 0)) {
        bool32 canFlyCarry = (leader->state.Matches(&Player::State_Roll) || leader->state.Matches(&Player::State_LookUp)
                              || leader->state.Matches(&Player::State_Crouch) || leader->state.Matches(&Player::State_Air)
                              || leader->state.Matches(&Player::State_Ground));

        if (canFlyCarry && (leader->animator.animationID != ANI_FAN)) {
            if (abs(this->position.x - leader->position.x) < 0xC0000 && abs(off - leader->position.y) < 0xC0000 && !this->flyCarryTimer
                && !leader->down && !leader->onGround) {
                leader->animator.SetAnimation(leader->aniFrames, ANI_HANG, false, 0);
                leader->state.Set(&Player::State_FlyCarried);
                leader->nextAirState.Set(nullptr);
                leader->nextGroundState.Set(nullptr);
                sVars->sfxGrab.Play();
            }
        }
    }

    if (leader->state.Matches(&Player::State_FlyCarried)) {
        int32 entityXPos = this->position.x;
        int32 entityYPos = this->position.y;
        int32 entityXVel = this->velocity.x;
        int32 entityYVel = this->velocity.y;

        Hitbox *sidekickOuterBox = this->animator.GetHitbox(0);
        Hitbox *sidekickInnerBox = this->animator.GetHitbox(1);
        if (this->collisionLayers & Zone::sVars->moveLayerMask) {
            TileLayer *move  = Zone::sVars->moveLayer.GetTileLayer();
            move->position.x = -this->moveLayerPosition.x >> 16;
            move->position.y = -this->moveLayerPosition.y >> 16;
        }

        this->ProcessMovement(sidekickOuterBox, sidekickInnerBox);

        if (this->onGround) {
            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR: this->collisionFlagV |= 1; break;

                case CMODE_ROOF: this->collisionFlagV |= 2; break;
            }
        }

        leader->flyCarrySidekickPos.x = this->position.x & 0xFFFF0000;
        leader->flyCarrySidekickPos.y = this->position.y & 0xFFFF0000;

        this->position.x = entityXPos;
        this->position.y = entityYPos;
        this->velocity.x = entityXVel;
        this->velocity.y = entityYVel;

        leader->position.x = entityXPos;
        leader->position.y = entityYPos;
        switch (globals->gravityDir) {
            default: break;
            case CMODE_FLOOR: leader->position.y += 0x1C0000; break;
            case CMODE_ROOF: leader->position.y -= 0x1C0000; break;
        }
        leader->velocity.x = entityXVel;
        leader->velocity.y = entityYVel;

        Hitbox *leaderOuterBox = leader->GetHitbox();
        Hitbox *leaderInnerBox = leader->GetAltHitbox();
        if (leader->collisionLayers & Zone::sVars->moveLayerMask) {
            TileLayer *move  = Zone::sVars->moveLayer.GetTileLayer();
            move->position.x = -leader->moveLayerPosition.x >> 16;
            move->position.y = -leader->moveLayerPosition.y >> 16;
        }

        leader->ProcessMovement(leaderOuterBox, leaderInnerBox);

        if (leader->onGround) {
            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR: leader->collisionFlagV |= 1; break;

                case CMODE_ROOF: leader->collisionFlagV |= 2; break;
            }
        }

        leader->collisionPlane = this->collisionPlane;
        leader->direction      = this->direction;
        leader->velocity.x     = 0;
        leader->velocity.y     = 0;
    }
}

void Player::HandleSuperColors_Sonic(bool32 updatePalette)
{
    if (updatePalette) {
        if (this->isHyper) {
            // TODO: make this not a clone of super maybe ?

            if (this->superBlendState & 2) {
                paletteBank[0].SetLimitedFade(&sVars->activeHyperSonicPalette[6], &sVars->activeHyperSonicPalette[12], this->superBlendAmount,
                                              this->superColorIndex, this->superColorCount);

                if (Water::sVars && !Water::sVars->isLightningFlashing) {
                    paletteBank[Water::sVars->waterPalette].SetLimitedFade(&sVars->activeHyperSonicPalette_Water[6],
                                                                           &sVars->activeHyperSonicPalette_Water[12], this->superBlendAmount,
                                                                           this->superColorIndex, this->superColorCount);
                }
            }
            else {
                paletteBank[0].SetLimitedFade(&sVars->activeHyperSonicPalette[0], &sVars->activeHyperSonicPalette[12], this->superBlendAmount,
                                              this->superColorIndex, this->superColorCount);

                if (Water::sVars && !Water::sVars->isLightningFlashing) {
                    paletteBank[Water::sVars->waterPalette].SetLimitedFade(&sVars->activeHyperSonicPalette_Water[0],
                                                                           &sVars->activeHyperSonicPalette_Water[12], this->superBlendAmount,
                                                                           this->superColorIndex, this->superColorCount);
                }
            }
        }
        else {
            if (this->superBlendState & 2) {
                paletteBank[0].SetLimitedFade(&sVars->activeSuperSonicPalette[6], &sVars->activeSuperSonicPalette[12], this->superBlendAmount,
                                              this->superColorIndex, this->superColorCount);

                if (Water::sVars && !Water::sVars->isLightningFlashing) {
                    paletteBank[Water::sVars->waterPalette].SetLimitedFade(&sVars->activeSuperSonicPalette_Water[6],
                                                                           &sVars->activeSuperSonicPalette_Water[12], this->superBlendAmount,
                                                                           this->superColorIndex, this->superColorCount);
                }
            }
            else {
                paletteBank[0].SetLimitedFade(&sVars->activeSuperSonicPalette[0], &sVars->activeSuperSonicPalette[12], this->superBlendAmount,
                                              this->superColorIndex, this->superColorCount);

                if (Water::sVars && !Water::sVars->isLightningFlashing) {
                    paletteBank[Water::sVars->waterPalette].SetLimitedFade(&sVars->activeSuperSonicPalette_Water[0],
                                                                           &sVars->activeSuperSonicPalette_Water[12], this->superBlendAmount,
                                                                           this->superColorIndex, this->superColorCount);
                }
            }
        }
    }
    else {
        if (this->superState == Player::SuperStateSuper) {
            if (this->superBlendState & 1) {
                if (this->superBlendAmount <= 0)
                    this->superBlendState &= ~1;
                else
                    this->superBlendAmount -= 4;
            }
            else if (this->superBlendAmount >= 256) {
                this->superBlendState |= 1;
                if (this->superBlendState == 1) {
                    for (int32 i = 0; i < 6; ++i) {
                        paletteBank[6].SetEntry(i + 0x40, sVars->activeSuperSonicPalette[i + 6]);
                        paletteBank[7].SetEntry(i + 0x40, sVars->activeSuperSonicPalette[i + 12]);
                    }
                    this->superBlendState  = 2;
                    this->superBlendAmount = 0;
                }
            }
            else {
                this->superBlendAmount += 4;
            }
        }
        else {
            if (this->superBlendAmount <= 0) {
                this->superBlendState &= ~2;
                this->superState = Player::SuperStateDone;
            }
            else
                this->superBlendAmount -= 4;
        }
    }
}
void Player::HandleSuperColors_Tails(bool32 updatePalette)
{
    if (updatePalette) {
        paletteBank[0].SetLimitedFade(&sVars->activeSuperTailsPalette[0], &sVars->activeSuperTailsPalette[12], this->superBlendAmount,
                                      this->superColorIndex, this->superColorCount);

        if (Water::sVars && !Water::sVars->isLightningFlashing) {
            paletteBank[Water::sVars->waterPalette].SetLimitedFade(&sVars->activeSuperTailsPalette_Water[0],
                                                                   &sVars->activeSuperTailsPalette_Water[12], this->superBlendAmount,
                                                                   this->superColorIndex, this->superColorCount);
        }
    }
    else {
        if (this->superState != Player::SuperStateSuper) {
            if (this->superBlendAmount <= 0)
                this->superState = Player::SuperStateNone;
            else
                this->superBlendAmount -= 4;
        }
        else if (this->superBlendState) {
            if (this->superBlendAmount <= 0)
                this->superBlendState = 0;
            else
                this->superBlendAmount -= 4;
        }
        else {
            if (this->superBlendAmount >= 256)
                this->superBlendState = 1;
            else
                this->superBlendAmount += 4;
        }
    }
}
void Player::HandleSuperColors_Knux(bool32 updatePalette)
{
    if (updatePalette) {
        paletteBank[0].SetLimitedFade(&sVars->activeSuperKnuxPalette[0], &sVars->activeSuperKnuxPalette[12], this->superBlendAmount,
                                      this->superColorIndex, this->superColorCount);

        if (Water::sVars && !Water::sVars->isLightningFlashing) {
            paletteBank[Water::sVars->waterPalette].SetLimitedFade(&sVars->activeSuperKnuxPalette_Water[0], &sVars->activeSuperKnuxPalette_Water[12],
                                                                   this->superBlendAmount, this->superColorIndex, this->superColorCount);
        }
    }
    else {
        if (this->superState != Player::SuperStateSuper) {
            if (this->superBlendAmount <= 0)
                this->superState = Player::SuperStateNone;
            else
                this->superBlendAmount -= 4;
        }
        else if (this->superBlendState) {
            if (this->superBlendAmount <= 0)
                this->superBlendState = 0;
            else
                this->superBlendAmount -= 4;
        }
        else {
            if (this->superBlendAmount >= 256)
                this->superBlendState = 1;
            else
                this->superBlendAmount += 4;
        }
    }
}

void Player::HandleSuperForm()
{
    if (this->superState != Player::SuperStateNone) {
        switch (this->characterID) {
            case ID_SONIC: HandleSuperColors_Sonic(false); break;
            case ID_TAILS: HandleSuperColors_Tails(false); break;
            case ID_KNUCKLES: HandleSuperColors_Knux(false); break;
        }
    }

    if (this->superState == Player::SuperStateSuper) {
        bool32 canStopSuper = false;
        if (!sceneInfo->timeEnabled) {
            this->superState = Player::SuperStateFadeOut;
            canStopSuper     = true;
        }

        if (!canStopSuper) {
            this->invincibleTimer = 60;
            if (--this->superRingLossTimer <= 0) {
                this->superRingLossTimer = 60;
                if (--this->rings <= 0) {
                    this->rings      = 0;
                    this->superState        = Player::SuperStateFadeOut;
                    this->hyperAbilityState = Player::HyperStateNone;
                }
            }
        }
    }

    if (this->superState == Player::SuperStateFadeOut) {
        switch (this->characterID) {
            case ID_SONIC: this->stateAbility.Set(&Player::Action_DblJumpSonic); break;
            case ID_TAILS: this->stateAbility.Set(&Player::Action_DblJumpTails); break;
            case ID_KNUCKLES: this->stateAbility.Set(&Player::Action_DblJumpKnux); break;
            default: break;
        }

        bool32 stopPlaying = true;
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (player->superState == Player::SuperStateSuper)
                stopPlaying = false;
        }

        if (stopPlaying && sceneInfo->timeEnabled && Music::sVars->restartTrackID < 0) {
            Music::ClearMusicStack();
            Music::JingleFadeOut(Music::TRACK_SUPER, true);
        }

        if (this->characterID == ID_SONIC && !this->isChibi) {
            this->aniFrames = sVars->sonicFrames;
            if (!this->animator.animationID)
                this->animator.frameID = 0;

            this->animator.SetAnimation(this->aniFrames, this->animator.animationID, true, this->animator.frameID);
        }

        this->invincibleTimer = 0;

        if (this->shield)
            ApplyShield();

        this->superState = Player::SuperStateDone;
        UpdatePhysicsState();
    }
}

void Player::DoSuperDash(Player *player)
{
    SET_CURRENT_STATE();

    uint8 dashDir = 0;
    if (player->up)
        dashDir |= 1;

    if (player->down)
        dashDir |= 2;

    if (player->left)
        dashDir |= 4;

    if (player->right)
        dashDir |= 8;

    int32 dashAngle    = 0;
    uint8 dashAngles[] = { 0x00, 0xC0, 0x40, 0x00, 0x80, 0xA0, 0x60, 0x00, 0x00, 0xE0, 0x20 };

    if (dashDir == 0 && player->direction)
        dashAngle = 0x80;
    else
        dashAngle = dashAngles[dashDir];

    player->rings -= 5;
    if (player->rings < 0)
        player->rings = 0;

    player->velocity.x = 0x1600 * Math::Cos256(dashAngle);
    player->velocity.y = 0x1600 * Math::Sin256(dashAngle);

    ItemBox::sVars->sfxHyperRing.Play();
    sVars->sfxPeelRelease.Play();

    switch (player->characterID) {
        default:
        case ID_SONIC: player->animator.SetAnimation(this->aniFrames, ANI_RUN, false, 0); break;
        case ID_TAILS: player->animator.SetAnimation(this->aniFrames, ANI_DASH, false, 0); break;
        case ID_KNUCKLES:
            player->animator.SetAnimation(this->aniFrames, ANI_GLIDE, false, 6);
            player->animator.rotationStyle = Animator::RotateFull;
            break;
    }

    player->state.Set(&Player::State_SuperFlying);
    player->abilityValues[0] = 60;

    if (FXFade::sVars) {
        FXFade *fxFade   = GameObject::Create<FXFade>(0xF0F0F0, this->position.x, this->position.y);
        fxFade->speedIn  = 256;
        fxFade->speedOut = 64;
    }

    sVars->superDashCooldown = 30;
}

void Player::FinishedReturnToPlayer(Player *leader)
{
    Entity *parent       = (Entity *)this->abilityPtrs[0];
    this->abilityPtrs[0] = nullptr;

    this->state.Set(&Player::State_Air);
    this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
    this->onGround       = false;
    this->tileCollisions = globals->tileCollisionMode;
    this->interaction    = true;
    this->controlLock    = 0;
    this->angle          = globals->gravityDir << 6;

    if (!this->sidekick) {
        this->stateInput.Set(&Player::Input_Gamepad);
    }
    else {
        this->stateInput.Set(&Player::Input_AI_Follow);

        Player *player1  = GameObject::Get<Player>(SLOT_PLAYER1);
        this->velocity.x = player1->velocity.x;
        this->groundVel  = player1->groundVel;
        this->velocity.y = 0;

        sVars->upState        = 0;
        sVars->downState      = 0;
        sVars->leftState      = 0;
        sVars->rightState     = 0;
        sVars->jumpHoldState  = 0;
        sVars->jumpPressState = 0;
    }

    this->collisionPlane     = leader->collisionPlane;
    this->disableGroundAnims = false;
    this->drawGroup          = leader->drawGroup;
    this->active             = ACTIVE_NORMAL;
    this->position.x         = parent->position.x;
    this->position.y         = parent->position.y;
    if (this->playerID == 0)
        this->blinkTimer = 120;

    if (this->camera) {
        Camera::SetTargetEntity(this->camera->screenID, this);
        this->camera->state.Set(&Camera::State_FollowXY);
    }
    parent->Destroy();

    sVars->upState        = 0;
    sVars->downState      = 0;
    sVars->leftState      = 0;
    sVars->rightState     = 0;
    sVars->jumpPressState = 0;
    sVars->jumpHoldState  = 0;

    this->UpdatePhysicsState();

    if (BoundsMarker::sVars)
        BoundsMarker::ApplyAllBounds(this, false);

    if (Water::sVars)
        Water::ApplyHeightTriggers();
}

// Actions
void Player::Action_Roll()
{
    this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
    switch (this->collisionMode) {
        case CMODE_FLOOR:
            this->position.y += this->jumpOffset;
            if (this->camera)
                this->camera->adjustY = this->jumpOffset;
            break;

        case CMODE_LWALL: this->position.x += this->jumpOffset; break;

        case CMODE_ROOF:
            this->position.y -= this->jumpOffset;
            if (this->camera)
                this->camera->adjustY = -this->jumpOffset;
            break;

        case CMODE_RWALL: this->position.x -= this->jumpOffset; break;
        default: break;
    }
    this->pushing = 0;
    this->state.Set(&Player::State_Roll);
}

void Player::ResetBoundaries()
{
    Vector2 size;

    int32 screen = this->Slot();
    Zone::sVars->fgLayer[0].Size(&size, true);

    Zone::sVars->cameraBoundsL[screen]      = 0;
    Zone::sVars->cameraBoundsR[screen]      = size.x;
    Zone::sVars->cameraBoundsT[screen]      = 0;
    Zone::sVars->cameraBoundsB[screen]      = size.y;
    Zone::sVars->playerBoundsL[screen]      = Zone::sVars->cameraBoundsL[screen] << 16;
    Zone::sVars->playerBoundsR[screen]      = Zone::sVars->cameraBoundsR[screen] << 16;
    Zone::sVars->playerBoundsT[screen]      = Zone::sVars->cameraBoundsT[screen] << 16;
    Zone::sVars->playerBoundsB[screen]      = Zone::sVars->cameraBoundsB[screen] << 16;
    Zone::sVars->deathBoundary[screen]      = Zone::sVars->cameraBoundsB[screen] << 16;
    Zone::sVars->playerBoundActiveT[screen] = true;
    Zone::sVars->playerBoundActiveT[screen] = false;

    if (this->camera) {
        this->camera->boundsL = Zone::sVars->cameraBoundsL[screen];
        this->camera->boundsR = Zone::sVars->cameraBoundsR[screen];
        this->camera->boundsT = Zone::sVars->cameraBoundsT[screen];
        this->camera->boundsB = Zone::sVars->cameraBoundsB[screen];
    }
}
void Player::HandleDeath()
{
    if (this->sidekick) {
        sVars->respawnTimer = 0;

        Dust *dust        = GameObject::Create<Dust>(true, this->position.x, this->position.y);
        dust->visible     = false;
        dust->active      = ACTIVE_NEVER;
        dust->isPermanent = true;

        switch (globals->gravityDir) {
            default: break;
            case CMODE_FLOOR: dust->position.y = (screenInfo->position.y - 128) << 16; break;
            case CMODE_ROOF: dust->position.y = (screenInfo->position.y + screenInfo->size.y + 128) << 16; break;
        }

        // Sidekicks just respawn, no biggie
        Player *leader = GameObject::Get<Player>(SLOT_PLAYER1);
        if (globals->gameMode != MODE_ENCORE || (!leader->state.Matches(&Player::State_Death) && !leader->state.Matches(&Player::State_Drown))) {
            this->angle = 0x80;
            this->state.Set(&Player::State_HoldRespawn);
            this->abilityPtrs[0]   = dust;
            this->abilityValues[0] = 0;
            this->nextAirState.Set(nullptr);
            this->nextGroundState.Set(nullptr);
            this->stateInput.Set(&Player::GetDelayedInput);
            this->position.x     = -0x400000;
            this->position.y     = -0x400000;
            this->velocity.x     = 0;
            this->velocity.y     = 0;
            this->groundVel      = 0;
            this->tileCollisions = TILECOLLISION_NONE;
            this->interaction    = false;
            this->drawGroup      = Zone::sVars->playerDrawGroup[1];
            this->drownTimer     = 0;
            this->active         = ACTIVE_NORMAL;
        }
        else {
            this->state.Set(&Player::State_DeathHold);
            this->velocity.x = 0;
            this->velocity.y = 0;
            this->position.x = -0x200000;
            this->position.y = -0x200000;
        }
    }
    else {
        // regular death event

        if (this->lives > 0 && globals->gameMode != MODE_TIMEATTACK && !(globals->medalMods & MEDAL_NOLIVES))
            this->lives--;
        sVars->savedLives                     = this->lives;
        sVars->savedScore                     = this->score;
        sVars->savedScore1UP                  = this->score1UP;
        globals->restartLives[this->playerID] = this->lives;

        if (globals->gameMode != MODE_ENCORE) {
            this->rings           = 0;
            this->ringExtraLife   = 0;
            globals->restartRings = 0;
            globals->restart1UP   = 100;
        }
        globals->coolBonus[this->playerID] = 0;

        if (globals->gameMode != MODE_ENCORE) {
            if (this->lives || (globals->medalMods & MEDAL_NOLIVES)) {
                if (Zone::sVars->gotTimeOver && !(globals->medalMods & MEDAL_NOLIVES)) {
                    // Time Over!!
                    this->classID = TYPE_NONE;

                    SaveGame *saveRAM = SaveGame::GetSaveRAM();
                    if (globals->gameMode == MODE_COMPETITION) {
                    }
                    else if (saveRAM) {
                        saveRAM->lives     = this->lives;
                        saveRAM->score     = this->score;
                        saveRAM->score1UP  = this->score1UP;
                        saveRAM->continues = globals->continues;
                        if (globals->gameMode == MODE_ENCORE) {
                            globals->playerID &= 0xFF;
                            saveRAM->playerID = globals->playerID;

                            int32 id = -1;
                            for (int32 i = this->characterID; i > 0; ++id) i >>= 1;
                            globals->characterFlags &= ~(1 << id);

                            saveRAM->characterFlags = globals->characterFlags;
                            saveRAM->stock          = globals->stock;
                        }

                        SaveGame::SaveFile(nullptr);
                    }

                    GameOver *gameOver = GameObject::Get<GameOver>(SLOT_GAMEOVER);
                    gameOver->Reset(GameOver::sVars->classID, true);
                    gameOver->playerID = this->Slot();
                    GameOver::sVars->activeScreens |= 1 << this->playerID;

                    Stage::SetEngineState(ENGINESTATE_FROZEN);
                    sceneInfo->timeEnabled = false;
                }
                else if (globals->gameMode != MODE_COMPETITION) {
                    // Regular Death, fade out and respawn
                    SaveGame *saveRAM = SaveGame::GetSaveRAM();
                    if (saveRAM) {
                        saveRAM->lives     = this->lives;
                        saveRAM->score     = this->score;
                        saveRAM->score1UP  = this->score1UP;
                        saveRAM->continues = globals->continues;
                        if (globals->gameMode == MODE_ENCORE) {
                            globals->playerID &= 0xFF;
                            saveRAM->playerID = globals->playerID;

                            int32 id = -1;
                            for (int32 i = this->characterID; i > 0; ++id) i >>= 1;
                            globals->characterFlags &= ~(1 << id);

                            saveRAM->characterFlags = globals->characterFlags;
                            saveRAM->stock          = globals->stock;
                        }
                        SaveGame::SaveFile(nullptr);
                    }

                    Music::FadeOut(0.025f);
                    Zone::StartFadeOut_MusicFade(10, 0x000000);
                    this->classID = TYPE_NONE;
                }
            }
            else {
                // GGGGGGGame Over...!
                int32 screenID = 0;
                this->classID  = TYPE_NONE;
                if (this->camera) {
                    screenID             = this->camera->screenID;
                    this->camera->target = (Entity *)this->camera;
                }

                bool32 showGameOver = true;
                if (globals->gameMode == MODE_COMPETITION)
                    showGameOver = false;

                if (showGameOver) {
                    GameOver *gameOver = GameObject::Get<GameOver>(SLOT_GAMEOVER);
                    gameOver->Reset(GameOver::sVars->classID, false);
                    gameOver->playerID = this->Slot();
                    GameOver::sVars->activeScreens |= 1 << screenID;

                    Stage::SetEngineState(ENGINESTATE_FROZEN);
                    sceneInfo->timeEnabled = false;
                }
            }

            if (this->classID == sVars->classID) {
                this->abilityValues[0] = 0;

                if (globals->useManiaBehavior) {
                    this->ResetBoundaries();
                }

                if (BoundsMarker::sVars)
                    BoundsMarker::ApplyAllBounds(this, true);

                if (Water::sVars)
                    Water::ApplyHeightTriggers();
            }
        }
        else {
            // Encore death, lets switch to the next character and be on our way if possible

            int32 id = -1;
            for (int32 i = this->characterID; i > 0; ++id) i >>= 1;
            globals->characterFlags &= ~(1 << id);

            if (!globals->characterFlags) {
                int32 screenID = 0;
                this->classID  = TYPE_NONE;
                if (this->camera) {
                    screenID             = this->camera->screenID;
                    this->camera->target = (Entity *)this->camera;
                }

                GameOver *gameOver = GameObject::Get<GameOver>(SLOT_GAMEOVER);
                gameOver->Reset(GameOver::sVars->classID, false);
                gameOver->playerID = this->Slot();
                GameOver::sVars->activeScreens |= 1 << screenID;

                Stage::SetEngineState(ENGINESTATE_FROZEN);
                sceneInfo->timeEnabled = false;

                if (this->classID == sVars->classID) {
                    this->abilityValues[0] = 0;

                    if (globals->useManiaBehavior) {
                        ResetBoundaries();
                    }

                    if (BoundsMarker::sVars)
                        BoundsMarker::ApplyAllBounds(this, true);

                    if (Water::sVars)
                        Water::ApplyHeightTriggers();
                }
            }
            else {
                Player *player2     = GameObject::Get<Player>(SLOT_PLAYER2);
                sVars->respawnTimer = 0;

                if (globals->stock) {
                    globals->stock >>= 8;
                }
                else {
                    globals->playerID &= 0xFF;
                    player2->classID = TYPE_NONE;
                }
            }
        }
    }
}

void Player::Action_Jump()
{
    this->controlLock = 0;
    this->onGround    = false;

    if (this->collisionMode == CMODE_FLOOR && !this->state.Matches(&Player::State_Roll))
        this->position.y += this->jumpOffset;

    int32 jumpForce  = this->gravityStrength + this->jumpStrength;
    this->velocity.x = (this->groundVel * Math::Cos256(this->angle) + jumpForce * Math::Sin256(this->angle)) >> 8;
    this->velocity.y = (this->groundVel * Math::Sin256(this->angle) - jumpForce * Math::Cos256(this->angle)) >> 8;

    if (this->camera) {
        this->camera->offset.y       = 0x200000;
        this->camera->disableYOffset = true;
    }

    this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
    if (this->characterID == ID_TAILS)
        this->animator.speed = 120;
    else
        this->animator.speed = ((abs(this->groundVel) * 0xF0) / 0x60000) + 0x30;

    if (this->animator.speed > 0xF0)
        this->animator.speed = 0xF0;

    this->angle            = globals->gravityDir << 6;
    this->collisionMode    = globals->gravityDir;
    this->skidding         = 0;
    this->applyJumpCap     = true;
    this->jumpAbilityState = 1;

    if (!this->sidekick || this->onScreen || this->blinkTimer)
        sVars->sfxJump.Play();

    this->state.Set(&Player::State_Air);

    this->disableGravity = true;
}
void Player::Action_Spindash()
{
    Dust *dust = GameObject::Create<Dust>(this, this->position.x, this->position.y);
    dust->animator.SetAnimation(Dust::sVars->aniFrames, 1, true, 0);
    dust->state.Set(&Dust::State_SpinDash);
    dust->drawGroup = this->drawGroup;

    this->animator.SetAnimation(this->aniFrames, ANI_SPINDASH, true, 0);
    this->state.Set(&Player::State_Spindash);

    this->abilityTimer   = 0;
    this->spindashCharge = 0;
    sVars->sfxCharge.Play();
}

void Player::Action_Peelout()
{
    this->animator.SetAnimation(this->aniFrames, ANI_WALK, true, 0);

    this->state.Set(&Player::State_Peelout);
    this->abilityTimer   = 0;
    this->spindashCharge = 0;
    sVars->sfxPeelCharge.Play();
}

void Player::Action_DblJumpSonic()
{
    SET_CURRENT_STATE();

    if (this->shield != Shield::None && !this->sidekick && this->up && this->superState != Player::SuperStateSuper)
        return;

    bool32 dropdashDisabled = this->jumpAbilityState <= 1;

    if (this->jumpAbilityState == 1) {
        if (!this->stateInput.Matches(&Player::Input_AI_Follow) || (this->up && globals->gameMode != MODE_ENCORE)) {
            if (this->jumpPress) {
                Shield *shield = GameObject::Get<Shield>(sVars->maxPlayerCount + this->Slot());
                if (this->invincibleTimer) {
                    if (shield->classID != Shield::sVars->classID || shield->shieldAnimator.animationID != Shield::AniInsta) {
                        if (!(globals->medalMods & MEDAL_NODROPDASH))
                            ++this->jumpAbilityState;

                        if (this->hyperAbilityState != Player::HyperStateNone && this->jumpHold) {
                            this->jumpAbilityState = 0;

                            if (this->up || this->down || this->left || this->right) {
                                this->velocity.x = 0;
                                this->velocity.y = 0;
                            }
                            else {
                                if (globals->tileCollisionMode == TILECOLLISION_DOWN) {
                                    if (this->direction & FLIP_X)
                                        this->velocity.x = -TO_FIXED(8);
                                    else
                                        this->velocity.x = TO_FIXED(8);
                                }
                                else {
                                    if (this->direction & FLIP_X)
                                        this->velocity.x = TO_FIXED(8);
                                    else
                                        this->velocity.x = -TO_FIXED(8);
                                }
                            }

                            if (this->up) {
                                if (globals->tileCollisionMode == TILECOLLISION_DOWN)
                                    this->velocity.y = -TO_FIXED(8);
                                else
                                    this->velocity.y = TO_FIXED(8);
                            }

                            if (this->down) {
                                if (globals->tileCollisionMode == TILECOLLISION_DOWN)
                                    this->velocity.y = TO_FIXED(8);
                                else
                                    this->velocity.y = -TO_FIXED(8);
                            }

                            if (this->left)
                                this->velocity.x = -TO_FIXED(8);

                            if (this->right)
                                this->velocity.x = TO_FIXED(8);

                            if (this->camera && !Zone::sVars->autoScrollSpeed) {
                                this->scrollDelay = 15;
                                this->camera->state.Set(&Camera::State_FollowY);
                            }
                            this->hyperAbilityState = Player::HyperStateStartHyperDash;

                            if (FXFade::sVars) {
                                FXFade *fade   = GameObject::Create<FXFade>(0xFFFFFF, this->position.x, this->position.y);
                                fade->speedIn  = 0x30;
                                fade->speedOut = 0x200;
                                fade->state.Set(&FXFade::State_FadeOut);
                            }
                            sVars->sfxRelease.Play();
                            Shield::SpawnLightningSparks(this, SuperSparkle::sVars->aniFrames, 3);
                        }
                    }
                }
                else {
                    switch (this->shield) {
                        case Shield::None:
                            if (globals->medalMods & MEDAL_INSTASHIELD) {
                                this->invincibleTimer  = -8;
                                this->jumpAbilityState = 0;
                                Shield::sVars->sfxInstaShield.Play();
                                shield->Reset(Shield::sVars->classID, this);
                                if (globals->useManiaBehavior) {
                                    shield->inkEffect = INK_ADD;
                                    shield->alpha     = 0x100;
                                }
                                shield->shieldAnimator.SetAnimation(Shield::sVars->aniFrames, Shield::AniInsta, true, 0);
                                shield->state.Set(&Shield::State_Insta);
                            }
                            // [Fallthrough]
                        case Shield::Blue:
                            // returns 0 if dropdash (bit 4) is disabled
                            // returns 1 if dropdash is enabled and instashield (bit 3) is disabled
                            // returns 2 if dropdash AND instashield are enabled
                            if (!(globals->medalMods & MEDAL_NODROPDASH))
                                this->jumpAbilityState = ((~(globals->medalMods & 0xFF) >> 3) & 2);
                            break;

                        case Shield::Bubble:
                            if (globals->useManiaBehavior)
                                this->velocity.x >>= 1;
                            else
                                this->velocity.x = 0;

                            if (globals->gravityDir == CMODE_ROOF)
                                this->velocity.y = -0x80000;
                            else
                                this->velocity.y = 0x80000;

                            this->state.Set(&Player::State_BubbleBounce);
                            this->nextGroundState.Set(nullptr);
                            this->nextAirState.Set(nullptr);
                            shield->fxAnimator.SetAnimation(Shield::sVars->aniFrames, Shield::AniBubbleAttackDAdd, true, 0);
                            shield->shieldAnimator.SetAnimation(Shield::sVars->aniFrames, Shield::AniBubbleAttackD, true, 0);
                            shield->state.Set(&Shield::State_BubbleDrop);
                            Shield::sVars->sfxBubbleBounce.Play();
                            break;

                        case Shield::Fire:
                            this->jumpAbilityState = 0;
                            if (globals->gravityDir == CMODE_ROOF)
                                this->velocity.x = (this->direction & FLIP_X) ? 0x80000 : -0x80000;
                            else
                                this->velocity.x = (this->direction & FLIP_X) ? -0x80000 : 0x80000;
                            this->velocity.y = 0;
                            shield->shieldAnimator.SetAnimation(Shield::sVars->aniFrames, Shield::AniFireAttack, true, 0);
                            shield->state.Set(&Shield::State_FireDash);
                            shield->direction = this->direction;
                            if (this->camera && !Zone::sVars->autoScrollSpeed) {
                                this->scrollDelay = 15;
                                this->camera->state.Set(&Camera::State_FollowY);
                            }
                            Shield::sVars->sfxFireDash.Play();
                            break;

                        case Shield::Lightning:
                            this->jumpAbilityState = 0;
                            if (globals->gravityDir == CMODE_ROOF || this->invertGravity)
                                this->velocity.y = 0x58000;
                            else
                                this->velocity.y = -0x58000;
                            shield->state.Set(&Shield::State_LightningSparks);
                            Shield::sVars->sfxLightningJump.Play();
                            break;

                        default: break;
                    }
                }
            }
            else {
                if (controllerInfo[this->controllerID].keyY.press) {
                    SaveGame::GetSaveRAM();
                    TryTransform(false, Player::TransformEmeralds);
                }
            }
            return;
        }

        dropdashDisabled = true;
    }

    if (!dropdashDisabled && this->jumpHold
        && (this->hyperAbilityState == Player::HyperStateNone || (!this->up && !this->down && !this->left && !this->right))) {
        if (++this->jumpAbilityState >= 22) {
            this->state.Set(&Player::State_DropDash);
            this->nextGroundState.Set(nullptr);
            this->nextAirState.Set(nullptr);
            this->animator.SetAnimation(this->aniFrames, ANI_DROPDASH, false, 0);
            sVars->sfxDropdash.Play();
        }
    }
}
void Player::Action_DblJumpTails()
{
    SET_CURRENT_STATE();

    if (this->jumpPress && this->jumpAbilityState == 1
        && (!this->stateInput.Matches(&Player::Input_AI_Follow) || (this->up && globals->gameMode != MODE_ENCORE))) {
        if (!this->invertGravity) {
            this->jumpAbilityState = 0;
            this->outtaHereTimer   = 0;
            this->timer            = 0;
            this->abilitySpeed     = 0x800;
            if (!this->underwater)
                this->animator.SetAnimation(this->aniFrames, ANI_FLY, false, 0);
            else
                this->animator.SetAnimation(this->aniFrames, ANI_SWIM, false, 0);
            this->state.Set(&Player::State_TailsFlight);
            this->stateGravity.Set(&Player::Gravity_NULL);
            this->nextGroundState.Set(nullptr);
            this->nextAirState.Set(nullptr);
        }
    }
    else if (controllerInfo[this->controllerID].keyY.press) {
        SaveGame::GetSaveRAM();
        TryTransform(false, Player::TransformEmeralds);
    }
}
void Player::Action_DblJumpKnux()
{
    SET_CURRENT_STATE();

    if (this->jumpPress && this->jumpAbilityState == 1
        && (!this->stateInput.Matches(&Player::Input_AI_Follow) || (this->up && globals->gameMode != MODE_ENCORE))) {
        if (!this->invertGravity) {
            if (GameObject::Get<Player>(SLOT_PLAYER1)->characterID == ID_KNUCKLES) {
                NotifyCallback(NOTIFY_STATS_CHARA_ACTION, 0, 0, 1);
            }

            this->jumpAbilityState = 0;
            this->abilitySpeed     = 0x40000;

            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR:
                    this->velocity.y += 0x20000;
                    if (this->velocity.y < 0)
                        this->velocity.y = 0;

                    if (!(this->direction & FLIP_X)) {
                        this->state.Set(&Player::State_KnuxGlideRight);
                        this->stateGravity.Set(&Player::Gravity_NULL);
                        this->velocity.x     = 0x40000;
                        this->timer          = 0;
                        this->outtaHereTimer = 0;
                    }
                    else {
                        this->state.Set(&Player::State_KnuxGlideLeft);
                        this->stateGravity.Set(&Player::Gravity_NULL);
                        this->velocity.x = -0x40000;
                        this->timer      = 256;
                    }
                    break;

                case CMODE_ROOF:
                    this->velocity.y -= 0x20000;
                    if (this->velocity.y > 0)
                        this->velocity.y = 0;

                    if (this->direction & FLIP_X) {
                        this->state.Set(&Player::State_KnuxGlideRight);
                        this->stateGravity.Set(&Player::Gravity_NULL);
                        this->velocity.x     = 0x40000;
                        this->timer          = 0;
                        this->outtaHereTimer = 0;
                    }
                    else {
                        this->state.Set(&Player::State_KnuxGlideLeft);
                        this->stateGravity.Set(&Player::Gravity_NULL);
                        this->velocity.x = -0x40000;
                        this->timer      = 256;
                    }
                    break;
            }

            this->nextGroundState.Set(nullptr);
            this->nextAirState.Set(nullptr);

            if (this->spriteType == ClassicSprites)
                this->animator.SetAnimation(this->aniFrames, ANI_GLIDE, false, 6);
            else
                this->animator.SetAnimation(this->aniFrames, ANI_GLIDE, false, 2);
        }
    }
    else if (controllerInfo[this->controllerID].keyY.press) {
        SaveGame::GetSaveRAM();
        TryTransform(false, Player::TransformEmeralds);
    }
}

void Player::Action_SuperDash()
{
    SET_CURRENT_STATE();

    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);
    if (player1->jumpAbilityState == 1 && player1->jumpPress) {
        switch (player1->characterID) {
            default:
            case ID_SONIC: player1->animator.SetAnimation(this->aniFrames, ANI_RUN, false, 0); break;
            case ID_TAILS: player1->animator.SetAnimation(this->aniFrames, ANI_DASH, false, 0); break;
            case ID_KNUCKLES:
                player1->animator.SetAnimation(this->aniFrames, ANI_GLIDE, false, 6);
                player1->animator.rotationStyle = Animator::RotateFull;
                break;
        }

        this->state.Set(&Player::State_SuperFlying);
    }
}

// States
void Player::State_Static() { SET_CURRENT_STATE(); }
void Player::State_Ground()
{
    SET_CURRENT_STATE();

    if (!this->onGround) {
        this->state.Set(&Player::State_Air);
        this->stateGravity.Set(&Player::Gravity_True);
    }
    else {
        if (!this->groundVel) {
            if (this->left)
                --this->pushing;
            else if (this->right)
                ++this->pushing;
            else
                this->pushing = 0;
        }
        else {
            if (!this->left && !this->right) {
                this->pushing = 0;
            }
            else {
                if (abs(this->groundVel) > 0x10000)
                    this->pushing = 0;
            }
        }

        if (this->collisionMode != globals->gravityDir)
            this->pushing = 0;

        HandleGroundRotation();
        HandleGroundMovement();

        this->stateGravity.Set(&Player::Gravity_False);

        if (!this->disableGroundAnims) {
            switch (this->spriteType) {
                case ClassicSprites: HandleGroundAnimation_Classic(); break;
                case ManiaSprites: HandleGroundAnimation(); break;
            }
        }

        if (this->jumpPress) {
            if (this->state.Matches(&Player::State_Ground)) {
                this->Action_Jump();
                if (!this->disableGravity) {
                    this->timer          = 0;
                    this->outtaHereTimer = 0;
                }
            }
        }
        else {
            if (this->groundVel) {
                int32 minRollVel = this->state.Matches(&Player::State_Crouch) ? 0x11000 : 0x8800;
                if (abs(this->groundVel) >= minRollVel && !this->left && !this->right && this->state.Matches(&Player::State_Ground) && this->down) {
                    Action_Roll();
                    sVars->sfxRoll.Play();
                }
            }
            else {
                bool32 canStandStill = false;
                if (this->invertGravity && this->angle == 128) {
                    canStandStill = true;
                }
                else if (this->collisionMode == globals->gravityDir) {
                    switch (this->collisionMode) {
                        case CMODE_FLOOR: canStandStill = this->angle < 0x20 || this->angle > 0xE0; break;
                        case CMODE_LWALL: canStandStill = this->angle > 0x20 && this->angle < 0x60; break;
                        case CMODE_ROOF: canStandStill = this->angle > 0x60 && this->angle < 0xA0; break;
                        case CMODE_RWALL: canStandStill = this->angle > 0xA0 && this->angle < 0xE0; break;
                        default: break;
                    }
                }

                if (canStandStill) {
                    if (this->state.Matches(&Player::State_Ground)) {
                        if (this->up) {
                            this->animator.SetAnimation(this->aniFrames, ANI_LOOK_UP, true, 1);
                            this->state.Set(&Player::State_LookUp);
                            this->timer          = 0;
                            this->outtaHereTimer = 0;
                        }
                        else if (this->down) {
                            this->animator.SetAnimation(this->aniFrames, ANI_CROUCH, true, 1);
                            this->state.Set(&Player::State_Crouch);
                            this->timer          = 0;
                            this->outtaHereTimer = 0;
                        }
                    }
                }
            }
        }
    }
}
void Player::State_Air()
{
    SET_CURRENT_STATE();
    this->tileCollisions = globals->tileCollisionMode;

    HandleAirFriction();

    if (this->onGround) {
        this->state.Set(&Player::State_Ground);
        this->stateGravity.Set(&Player::Gravity_False);
    }
    else {
        this->stateGravity.Set(&Player::Gravity_True);
        if (this->animator.animationID != ANI_SPINDASH)
            this->nextGroundState.Set(&Player::State_Ground);

        if ((this->velocity.y > 0 && globals->gravityDir == CMODE_FLOOR) || (this->velocity.y < 0 && globals->gravityDir == CMODE_ROOF)) {
            if (this->animator.animationID >= ANI_SPRING) {
                if (this->animator.animationID <= ANI_SPRING) {
                    this->animator.SetAnimation(this->aniFrames, this->animationReserve, false, 0);
                }
                else if ((this->animator.animationID == ANI_SPRING_CS) && !this->animator.frameID) {
                    this->animator.SetAnimation(this->aniFrames, ANI_FALL, false, 0);
                }
            }
        }

        // Air Curl!
        // TODO: may wanna put this in a specific animation case, but for rn it is what it is
        if (this->animator.animationID != ANI_JUMP) {
            if (globals->gravityDir == CMODE_ROOF) {
                if (this->velocity.y > 0 && this->jumpPress) {
                    this->animator.SetAnimation(this->aniFrames, ANI_AIRCURL, false, 0);
                    if (this->animator.animationID = ANI_AIRCURL) {
                        this->animator.frameID = this->animator.frameCount - 5;
                        this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
                    }
                    this->velocity.y >>= 1;
                }
            }
            else if (globals->gravityDir == CMODE_FLOOR) {
                if (this->velocity.y < 0 && this->jumpPress) {
                    this->animator.SetAnimation(this->aniFrames, ANI_AIRCURL, false, 0);
                    if (this->animator.frameID == this->animator.frameCount - 5) {
                        this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
                    }
                    this->velocity.y >>= 1;
                }
            }
        }

        switch (this->animator.animationID) {
            case ANI_IDLE:
            case ANI_WALK:
                if (this->animator.speed < 64)
                    this->animator.speed = 64;
                this->animator.SetAnimation(this->aniFrames, ANI_FALL, false, this->animator.frameID);
                break;

            case ANI_LOOK_UP:
            case ANI_CROUCH:
            case ANI_SKID_TURN: this->animator.SetAnimation(this->aniFrames, ANI_FALL, false, this->animator.frameID); break;

            case ANI_JOG: {
                int32 speed = this->animator.speed;
                this->animator.SetAnimation(this->aniFrames, ANI_FALL, false, 0);
                this->animator.speed = speed;
                break;
            }
            
            case ANI_RUN: {           
                this->animator.SetAnimation(this->aniFrames, ANI_FALL, false, 0);
                break;
            }

            case ANI_DASH: {
                this->animator.SetAnimation(this->aniFrames, ANI_FALL, false, 0);
                break;
            }

            case ANI_PEELOUT: {
                this->animator.SetAnimation(this->aniFrames, ANI_FALL, false, 0);
                break;
            }

            case ANI_JUMP:
                if (globals->gravityDir == CMODE_ROOF) {
                    if (this->velocity.y <= -this->jumpCap)
                        this->stateAbility.Run(this);
                }
                else if (globals->gravityDir == CMODE_FLOOR) {
                    if (this->velocity.y >= this->jumpCap)
                        this->stateAbility.Run(this);
                }
                break;

            case ANI_SKID:
                if (this->skidding <= 0)
                    return this->animator.SetAnimation(this->aniFrames, ANI_FALL, false, this->animator.frameID);
                else
                    this->skidding--;
                break;

            case ANI_SPINDASH:
                if (!this->nextGroundState.Set(&Player::State_Ground))
                    this->nextGroundState.Set(&Player::State_Spindash);
                break;

            default: break;
        }
    }
}

void Player::State_LookUp()
{
    SET_CURRENT_STATE();

    if (this->invertGravity)
        this->rotation = 128;

    this->stateGravity.Set(&Player::Gravity_True);
    this->nextAirState.Set(&Player::State_Air);

    if (this->up) {
        this->animator.SetAnimation(this->aniFrames, ANI_LOOK_UP, false, 1);

        this->left  = false;
        this->right = false;

        HandleGroundMovement();

        if (this->animator.frameID == 5)
            this->animator.speed = 0;

        if (this->timer < sVars->lookUpDelay) {
            this->timer++;
        }
        else if (this->camera) {
            if (this->invertGravity || globals->gravityDir == CMODE_ROOF) {
                if (this->camera->lookPos.y < sVars->lookUpDistance)
                    this->camera->lookPos.y += 2;
            }
            else {
                if (this->camera->lookPos.y > -sVars->lookUpDistance)
                    this->camera->lookPos.y -= 2;
            }
        }

        if (this->jumpPress) {
            if (!this->statePeelout.Matches(nullptr)) {
                this->statePeelout.Run(this);
            }
            else {
                Action_Jump();
            }
        }
    }
    else {
        if (this->spriteType == ManiaSprites) {
            HandleGroundMovement();

            this->animator.speed = 64;

            if (this->animator.frameID == 0 || this->left || this->right)
                this->state.Set(&Player::State_Ground);

            if (this->jumpPress)
                Action_Jump();
        }
        else {
            this->state.Set(&Player::State_Ground);
            this->animator.SetAnimation(this->aniFrames, ANI_IDLE, true, 0);
            State_Ground();
        }
    }
}

void Player::State_Crouch()
{
    SET_CURRENT_STATE();

    if (this->invertGravity)
        this->rotation = 0x80;

    this->left  = false;
    this->right = false;

    HandleGroundMovement();

    this->stateGravity.Set(&Player::Gravity_True);
    this->nextAirState.Set(&Player::State_Air);

    if (this->down) {
        this->animator.SetAnimation(this->aniFrames, ANI_CROUCH, false, 1);

        if (this->animator.frameID == 4)
            this->animator.speed = 0;

        if (this->timer < 60) {
            this->timer++;
        }
        else if (this->camera) {
            if (this->invertGravity || globals->gravityDir == CMODE_ROOF) {
                if (this->camera->lookPos.y > -96)
                    this->camera->lookPos.y -= 2;
            }
            else {
                if (this->camera->lookPos.y < 96)
                    this->camera->lookPos.y += 2;
            }
        }

        if (this->jumpPress) {
            Action_Spindash();
        }
    }
    else {
        if (this->spriteType == ManiaSprites) {
            this->animator.speed = 128;
            if (!this->animator.frameID || this->left || this->right)
                this->state.Set(&Player::State_Ground);

            if (this->jumpPress)
                Action_Jump();
        }
        else {
            this->state.Set(&Player::State_Ground);
            this->animator.SetAnimation(this->aniFrames, ANI_IDLE, true, 0);
            State_Ground();
        }
    }
}

void Player::State_Roll()
{
    HandleGroundRotation();
    HandleRollDeceleration();

    this->applyJumpCap = false;
    if (!this->onGround) {
        this->state.Set(&Player::State_Air);
        this->stateGravity.Set(&Player::Gravity_True);
    }
    else {
        if (this->characterID == ID_TAILS)
            this->animator.speed = 120;
        else
            this->animator.speed = ((abs(this->groundVel) * 0xF0) / 0x60000) + 0x30;

        if (this->animator.speed > 0xF0)
            this->animator.speed = 0xF0;

        this->stateGravity.Set(&Player::Gravity_False);

        if (this->jumpPress)
            Action_Jump();
    }
}

void Player::State_Spindash()
{
    if (this->onGround && !this->groundedStore) {
        this->nextAirState.Set(&Player::State_Air);

        if (this->angle >= 0x20) {
            this->state.Set(&Player::State_Ground);
        }
        else {

            bool32 hasDust = false;
            for (auto dust : GameObject::GetEntities<Dust>(FOR_ACTIVE_ENTITIES)) {
                if (dust->state.Matches(&Dust::State_SpinDash) && dust->parent == this)
                    hasDust = true;
            }

            if (!hasDust) {
                Dust *dust = GameObject::Create<Dust>(this, this->position.x, this->position.y);
                dust->animator.SetAnimation(Dust::sVars->aniFrames, 1, true, 0);
                dust->state.Set(&Dust::State_SpinDash);
                dust->drawGroup = this->drawGroup;

                this->groundVel = 0;
            }
        }
    }
    else {
        if (this->jumpPress) {
            this->abilityTimer += 0x20000;

            if (this->abilityTimer > 0x90000)
                this->abilityTimer = 0x90000;

            if (this->spindashCharge < 12)
                this->spindashCharge++;

            if (this->spindashCharge < 0)
                this->spindashCharge = 0;

            this->animator.SetAnimation(this->aniFrames, ANI_SPINDASH, true, 0);
            int32 channel = sVars->sfxCharge.Play();
            channels[channel].SetAttributes(1.0, 0.0, sVars->spindashChargeSpeeds[this->spindashCharge]);
        }
        else {
            this->abilityTimer -= this->abilityTimer >> 5;
        }
    }

    if (!this->down) {
        this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
        this->animator.Process();

        if (this->camera && !Zone::sVars->autoScrollSpeed) {
            this->scrollDelay = 15;
            this->camera->state.Set(&Camera::State_FollowY);
        }

        int32 vel = 0;
        if (this->superState == Player::SuperStateSuper)
            vel = (((uint32)this->abilityTimer >> 1) & 0x7FFF8000) + 0xB0000;
        else
            vel = (((uint32)this->abilityTimer >> 1) & 0x7FFF8000) + 0x80000;

        if (this->direction & FLIP_X)
            this->groundVel = -vel;
        else
            this->groundVel = vel;

        sVars->sfxCharge.Stop();
        sVars->sfxRelease.Play();

        this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);

        switch (this->collisionMode) {
            default: break;

            case CMODE_FLOOR:
                this->position.y += this->jumpOffset;

                if (this->camera)
                    this->camera->adjustY = this->jumpOffset;
                break;

            case CMODE_LWALL: this->position.x += this->jumpOffset; break;

            case CMODE_ROOF:
                this->position.y -= this->jumpOffset;

                if (this->camera)
                    this->camera->adjustY = -this->jumpOffset;
                break;

            case CMODE_RWALL: this->position.x -= this->jumpOffset; break;
        }

        this->pushing = 0;
        this->state.Set(&Player::State_Roll);
    }
}

void Player::State_Peelout()
{
    if (this->gravityStrength == 0x1000) {
        if (this->superState == Player::SuperStateSuper) {
            this->abilityTimer += 0xA000;
            if (this->abilityTimer > 0x70000)
                this->abilityTimer = 0x70000;
        }
        else {
            this->abilityTimer += 0x6000;
            if (this->abilityTimer > 0x60000)
                this->abilityTimer = 0x60000;
        }
    }
    else if (this->superState == Player::SuperStateSuper) {
        this->abilityTimer += 0xC000;
        if (this->abilityTimer > 0xD0000)
            this->abilityTimer = 0xD0000;
    }
    else {
        this->abilityTimer += 0x8000;
        if (this->abilityTimer > 0xC0000)
            this->abilityTimer = 0xC0000;
    }

    if (this->abilityTimer < this->minJogVelocity) {
        if (this->spriteType == ManiaSprites) {
            if (this->animator.animationID == ANI_JOG) {
                if (this->animator.frameID == 9)
                    this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 9);
            }
            else if (this->animator.animationID == ANI_FALL) {
                this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, this->animator.frameID);
            }
            else {
                this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 0);
            }
        }
        else {
            if (this->animator.animationID == ANI_JOG) {
                if (this->animator.frameID == 9)
                    this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 9);
            }
            else if (this->animator.animationID == ANI_FALL) {
                this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, this->animator.frameID);
            }
            else {
                this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 0);
            }
        }

        this->animator.speed = (this->abilityTimer >> 12) + 48;
        this->minJogVelocity = 0x40000;
    }
    else if (this->abilityTimer < this->minRunVelocity) {
        if (this->animator.animationID != ANI_WALK || this->animator.frameID == 3)
            this->animator.SetAnimation(this->aniFrames, ANI_JOG, false, 0);
        this->animator.speed = 12 * (this->abilityTimer >> 16) + 64;
        this->minJogVelocity = 0x38000;
        this->minRunVelocity = 0x60000;
    }
    else if (this->abilityTimer < this->minDashVelocity) {
        if (this->animator.animationID == ANI_DASH || this->animator.animationID == ANI_RUN)
            this->animator.SetAnimation(this->aniFrames, ANI_RUN, false, 0);
        else
            this->animator.SetAnimation(this->aniFrames, ANI_RUN, false, 1);

        this->animator.speed = (this->abilityTimer >> 12) + 96;
        if (this->animator.speed > 0x200)
            this->animator.speed = 512;
        this->minDashVelocity = 0xC0000;
        this->minRunVelocity  = 0x58000;
    }
    else {
        if (this->animator.animationID == ANI_DASH || this->animator.animationID == ANI_RUN)
            this->animator.SetAnimation(this->aniFrames, ANI_DASH, false, 0);
        else
            this->animator.SetAnimation(this->aniFrames, ANI_DASH, false, 1);

        this->minDashVelocity = 0xB8000;
    }

    if (!this->up) {
        sVars->sfxPeelCharge.Stop();

        if (this->abilityTimer >= 0x60000) {
            if (this->camera && !Zone::sVars->autoScrollSpeed) {
                this->scrollDelay = 15;
                this->camera->state.Set(&Camera::State_FollowY);
            }

            if (this->direction & FLIP_X)
                this->groundVel = -this->abilityTimer;
            else
                this->groundVel = this->abilityTimer;

            sVars->sfxPeelRelease.Play();
        }

        this->state.Set(&Player::State_Ground);
    }
}

void Player::State_Hurt()
{
    SET_CURRENT_STATE();

    if (this->onGround) {
        this->state.Set(&Player::State_Ground);

        if (this->velocity.x >= -0x20000) {
            if (this->velocity.x <= 0x20000)
                this->groundVel = 0;
            else
                this->groundVel -= 0x20000;
        }
        else {
            this->groundVel += 0x20000;
        }

        this->pushing     = false;
        this->controlLock = false;

        this->stateGravity.Set(&Player::Gravity_False);

        this->skidding = false;
    }
    else {
        if (this->camera) {
            this->camera->offset.y       = 0x200000;
            this->camera->disableYOffset = true;
        }

        ++this->timer;
        this->skidding = 0;

        switch (globals->gravityDir) {
            default: break;

            case CMODE_FLOOR:
                if (!this->underwater)
                    this->velocity.y += 0x3000;
                else
                    this->velocity.y += 0xF00;
                break;

            case CMODE_ROOF:
                if (!this->underwater)
                    this->velocity.y -= 0x3000;
                else
                    this->velocity.y -= 0xF00;
                break;
        }
    }
}
void Player::State_Death()
{
    SET_CURRENT_STATE();

    if (this->superState == Player::SuperStateSuper)
        this->superState = Player::SuperStateFadeOut;

    if (this->blinkTimer) {
        this->blinkTimer = 0;
        this->visible    = true;
    }

    this->velocity.x = 0;
    this->velocity.y += this->gravityStrength;

    this->animator.SetAnimation(this->aniFrames, ANI_DIE, false, 0);

    if (!this->sidekick) {
        if (this->camera) {

            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR:
                    if (this->position.y > this->camera->position.y - this->camera->targetMoveVel.y + 0x1800000)
                        this->position.y = this->camera->position.y - this->camera->targetMoveVel.y + 0x1800000;
                    break;

                case CMODE_ROOF:
                    if (this->position.y > this->camera->position.y - this->camera->targetMoveVel.y - 0x1800000)
                        this->position.y = this->camera->position.y - this->camera->targetMoveVel.y - 0x1800000;
                    break;
            }

            this->scrollDelay = 2;
        }
    }

    switch (globals->gravityDir) {
        default: break;

        case CMODE_FLOOR:
            if (this->velocity.y > 0x100000)
                HandleDeath();
            break;

        case CMODE_ROOF:
            if (this->velocity.y < -0x100000)
                HandleDeath();
            break;
    }
}
void Player::State_Drown()
{
    SET_CURRENT_STATE();

    if (this->superState == Player::SuperStateSuper)
        this->superState = Player::SuperStateFadeOut;

    if (this->blinkTimer) {
        this->blinkTimer = 0;
        this->visible    = true;
    }

    this->velocity.x = 0;
    this->velocity.y += this->gravityStrength;

    this->animator.SetAnimation(this->aniFrames, ANI_DROWN, false, 0);

    if (!this->sidekick) {
        if (this->camera) {

            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR:
                    if (this->position.y > this->camera->position.y + 0x1000000)
                        this->position.y = this->camera->position.y + 0x1000000;
                    break;

                case CMODE_ROOF:
                    if (this->position.y > this->camera->position.y - 0x1000000)
                        this->position.y = this->camera->position.y - 0x1000000;
                    break;
            }
            this->scrollDelay = 2;
        }
    }

    switch (globals->gravityDir) {
        default: break;

        case CMODE_FLOOR:
            if (this->velocity.y > 0x100000)
                HandleDeath();
            break;

        case CMODE_ROOF:
            if (this->velocity.y < -0x100000)
                HandleDeath();
            break;
    }
}
void Player::State_DeathHold()
{
    SET_CURRENT_STATE();

    inDeathHold = true;
}
void Player::State_DropDash()
{
    SET_CURRENT_STATE();

    if (this->onGround) {
        this->stateGravity.Set(&Player::Gravity_False);

        bool32 left  = false;
        bool32 right = false;

        if (globals->gravityDir == CMODE_ROOF || this->invertGravity) {
            right = this->left;
            left  = this->right;
        }
        else {
            left  = this->left;
            right = this->right;
        }

        if (right)
            this->direction &= ~FLIP_X;
        if (left)
            this->direction |= FLIP_X;

        int32 vel = 0x80000;
        int32 cap = 0xC0000;
        if (this->superState == Player::SuperStateSuper) {
            vel = 0xC0000;
            cap = 0xD0000;

            if (this->camera)
                Camera::ShakeScreen(this->camera->screenID, 6, 6, 1, 1);
        }

        if (this->direction & FLIP_X) {
            if (this->velocity.x <= 0) {
                this->groundVel = (this->groundVel >> 2) - vel;

                if (this->groundVel < -cap)
                    this->groundVel = -cap;
            }
            else if (this->angle) {
                this->groundVel = (this->groundVel >> 1) - vel;
            }
            else {
                this->groundVel = -vel;
            }
        }
        else {
            if (this->velocity.x >= 0) {
                this->groundVel = vel + (this->groundVel >> 2);

                if (this->groundVel > cap)
                    this->groundVel = cap;
            }
            else if (this->angle) {
                this->groundVel = vel + (this->groundVel >> 1);
            }
            else {
                this->groundVel = vel;
            }
        }

        if (this->camera && !Zone::sVars->autoScrollSpeed) {
            this->scrollDelay = 8;
            this->camera->state.Set(&Camera::State_FollowY);
        }

        Dust *dust = GameObject::Create<Dust>(this, this->position.x, this->position.y);
        dust->animator.SetAnimation(Dust::sVars->aniFrames, 2, true, 0);
        dust->state.Set(&Dust::State_DustPuff);
        if (globals->gravityDir == CMODE_ROOF) {
            dust->position.y -= this->animator.GetHitbox(0)->bottom << 16;
            dust->direction = this->direction ^ FLIP_X;
        }
        else {
            dust->position.y += this->animator.GetHitbox(0)->bottom << 16;
            dust->direction = this->direction;
        }
        dust->drawGroup = this->drawGroup;

        sVars->sfxDropdash.Stop();
        if (this->superState == Player::SuperStateSuper)
            sVars->sfxPeelRelease.Play();
        else
            sVars->sfxRelease.Play();

        this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);

        switch (this->collisionMode) {
            case CMODE_FLOOR:
                this->position.y += this->jumpOffset;
                if (this->camera)
                    this->camera->adjustY = this->jumpOffset;
                break;

            case CMODE_LWALL: this->position.x += this->jumpOffset; break;

            case CMODE_ROOF:
                this->position.y -= this->jumpOffset;
                if (this->camera)
                    this->camera->adjustY = -this->jumpOffset;
                break;

            case CMODE_RWALL: this->position.x -= this->jumpOffset; break;

            default: break;
        }

        this->pushing = 0;
        this->state.Set(&Player::State_Roll);
    }
    else if (this->jumpHold) {
        HandleAirFriction();
        this->stateGravity.Set(&Player::Gravity_True);

        this->animator.speed += 8;
        if (this->animator.speed > 0x100)
            this->animator.speed = 0x100;
    }
    else {
        this->jumpAbilityState = 0;
        this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
        this->state.Set(&Player::State_Air);
    }
}
void Player::State_BubbleBounce()
{
    SET_CURRENT_STATE();

    if (this->shield == Shield::Bubble && !this->invincibleTimer && this->superState != Player::SuperStateSuper) {
        if (!this->onGround) {
            HandleAirFriction();

            if (this->velocity.y < 0)
                this->controlLock = 0;
        }
        else {
            this->controlLock = 0;

            int32 vel = 0;
            if (this->underwater != 1) // only apply lower velocity if in water level types, pools use regular vel
                vel = this->gravityStrength + 0x78000;
            else
                vel = this->gravityStrength + 0x40000;
            this->onGround = false;

            this->velocity.x = (this->groundVel * Math::Cos256(this->angle) + vel * Math::Sin256(this->angle)) >> 8;
            this->velocity.y = (this->groundVel * Math::Sin256(this->angle) - vel * Math::Cos256(this->angle)) >> 8;

            if (this->camera) {
                this->camera->offset.y       = 0x200000;
                this->camera->disableYOffset = true;
            }

            this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
            this->animator.speed = (abs(this->groundVel) >> 12) + 0x30;

            Shield *shield = GameObject::Get<Shield>(sVars->maxPlayerCount + this->playerID);
            if (globals->useManiaBehavior) {
                shield->fxAnimator.SetAnimation(Shield::sVars->aniFrames, Shield::AniBubbleAttackUp, true, 0);
                shield->shieldAnimator.SetAnimation(nullptr, 0, true, 0);
            }
            else {
                shield->shieldAnimator.SetAnimation(Shield::sVars->aniFrames, Shield::AniBubbleAttackUp, true, 0);
                shield->fxAnimator.SetAnimation(nullptr, 0, true, 0);
            }

            shield->state.Set(&Shield::State_BubbleBounce);
            if (this->animator.speed > 0xF0)
                this->animator.speed = 0xF0;

            this->angle            = globals->gravityDir << 6;
            this->collisionMode    = globals->gravityDir;
            this->applyJumpCap     = false;
            this->jumpAbilityState = 1;
            Shield::sVars->sfxBubbleBounce.Play();

            this->state.Set(&Player::State_Air);
        }
    }
    else {
        this->state.Set(&Player::State_Air);
    }
}
void Player::State_TailsFlight()
{
    SET_CURRENT_STATE();

    HandleAirFriction();

    if (this->onGround) {
        this->state.Set(&Player::State_Ground);
        this->stateGravity.Set(&Player::Gravity_True);
    }
    else {

        uint16 slot = 0;
        if (!this->sidekick)
            slot = sceneInfo->entitySlot;

        switch (globals->gravityDir) {
            default: break;

            case CMODE_FLOOR:
                if (this->velocity.y < -0x10000)
                    this->abilitySpeed = 0x800;

                if (this->velocity.y <= 0) {
                    if (this->abilityValue >= 60)
                        this->abilitySpeed = 0x800;
                    else
                        this->abilityValue++;
                }

                this->velocity.y += this->abilitySpeed;

                if (this->position.y < Zone::sVars->playerBoundsT[slot] + 0x100000 && this->velocity.y < 0)
                    this->velocity.y = 0;
                break;

            case CMODE_ROOF:
                if (this->velocity.y > 0x10000)
                    this->abilitySpeed = 0x800;

                if (this->velocity.y <= 0) {
                    if (this->abilityValue >= 60)
                        this->abilitySpeed = 0x800;
                    else
                        this->abilityValue++;
                }

                this->velocity.y -= this->abilitySpeed;

                if (this->position.y > Zone::sVars->playerBoundsB[slot] - 0x100000 && this->velocity.y > 0)
                    this->velocity.y = 0;
                break;
        }

        Player *leader = GameObject::Get<Player>(SLOT_PLAYER1);

        if (leader->abilityValues[1] == 3)
            this->velocity.y = 0;

        if (globals->gameMode != MODE_COMPETITION && !this->isChibi && !leader->isChibi)
            this->CheckStartFlyCarry(leader);

        if (this->timer >= 480) {
            if (!this->underwater) {
                if (leader->state.Matches(&Player::State_FlyCarried))
                    this->animator.SetAnimation(this->aniFrames, ANI_FLY_LIFT_TIRED, false, 0);
                else
                    this->animator.SetAnimation(this->aniFrames, ANI_FLY_TIRED, false, 0);
            }
            else {
                if (leader->state.Matches(&Player::State_FlyCarried))
                    this->animator.SetAnimation(this->aniFrames, ANI_SWIM_LIFT, false, 0);
                else
                    this->animator.SetAnimation(this->aniFrames, ANI_SWIM_TIRED, false, 0);
            }
        }
        else {
            if (this->underwater) {
                if (leader->state.Matches(&Player::State_FlyCarried)) {
                    this->animator.SetAnimation(this->aniFrames, ANI_SWIM_LIFT, false, 0);
                }
                else {
                    this->animator.SetAnimation(this->aniFrames, ANI_SWIM, false, 0);

                    this->animator.speed = this->spriteType == ManiaSprites ? 128 : 64;

                    if ((this->velocity.y >= 0 && globals->gravityDir == CMODE_FLOOR) || (this->velocity.y <= 0 && globals->gravityDir == CMODE_ROOF))
                        this->animator.speed = this->spriteType == ManiaSprites ? 64 : 32;
                }
            }
            else {
                if (!leader->state.Matches(&Player::State_FlyCarried)) {
                    this->animator.SetAnimation(this->aniFrames, ANI_FLY, false, 0);
                }
                else {
                    if ((this->velocity.y < 0 && globals->gravityDir == CMODE_FLOOR) || (this->velocity.y > 0 && globals->gravityDir == CMODE_ROOF))
                        this->animator.SetAnimation(this->aniFrames, ANI_FLY_LIFT, false, 0);
                    else
                        this->animator.SetAnimation(this->aniFrames, ANI_FLY_LIFT_DOWN, false, 0);
                }

                if ((this->velocity.y >= 0 && globals->gravityDir == CMODE_FLOOR) || (this->velocity.y <= 0 && globals->gravityDir == CMODE_ROOF))
                    this->animator.speed = this->spriteType == ManiaSprites ? 128 : 64;
                else
                    this->animator.speed = this->spriteType == ManiaSprites ? 256 : 128;
            }

            if (++this->timer == 480) {
                if (!this->underwater) {
                    if (leader->state.Matches(&Player::State_FlyCarried))
                        this->animator.SetAnimation(this->aniFrames, ANI_FLY_LIFT_TIRED, false, 0);
                    else
                        this->animator.SetAnimation(this->aniFrames, ANI_FLY_TIRED, false, 0);
                }
                else {
                    if (leader->state.Matches(&Player::State_FlyCarried))
                        this->animator.SetAnimation(this->aniFrames, ANI_SWIM_LIFT, false, 0);
                    else
                        this->animator.SetAnimation(this->aniFrames, ANI_SWIM_TIRED, false, 0);
                }
            }
            else if (this->jumpPress && (!this->underwater || !leader->state.Matches(&Player::State_FlyCarried))) {
                this->abilitySpeed = -0x2000;
                this->abilityValue = 0;
            }
        }

        // Flight Cancel!

        if (!this->sidekick) {
            if (this->timer < 480 && this->down && !leader->state.Matches(&Player::State_FlyCarried)) {
                this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
                this->state.Set(&Player::State_Air);
                this->jumpAbilityState = 0;
            }
        }
        else {
            // TODO: Do something else for P2 to activate it?
        }
    }
}
void Player::State_FlyCarried()
{
    SET_CURRENT_STATE();

    Player *sidekick = GameObject::Get<Player>(SLOT_PLAYER2);

    if (!sidekick->state.Matches(&Player::State_TailsFlight))
        this->state.Set(&Player::State_Air);

    if (this->flyCarrySidekickPos.x != this->flyCarryLeaderPos.x)
        this->state.Set(&Player::State_Air);

    if (this->onGround) {
        if ((this->velocity.y >= 0 && globals->gravityDir == CMODE_FLOOR) || (this->velocity.y <= 0 && globals->gravityDir == CMODE_ROOF))
            this->state.Set(&Player::State_Ground);
    }

    if (this->state.Matches(&Player::State_FlyCarried)) {
        if (this->jumpPress && this->down) {
            this->state.Set(&Player::State_Air);

            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR:
                    if (!this->underwater)
                        this->velocity.y = -0x40000;
                    else
                        this->velocity.y = -0x20000;
                    break;

                case CMODE_ROOF:
                    if (!this->underwater)
                        this->velocity.y = 0x40000;
                    else
                        this->velocity.y = 0x20000;
                    break;
            }

            this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
        }
    }
    else {
        sidekick->flyCarryTimer = 30;
    }
}

void Player::State_KnuxGlideLeft()
{
    SET_CURRENT_STATE();

    if (!this->onGround) {
        if (!this->jumpHold) {
            this->outtaHereTimer = 0;
            this->timer          = 0;

            this->velocity.x >>= 2;
            if (Zone::sVars->autoScrollSpeed < -0x10000) {
                int32 newVel = 0x10000 - Zone::sVars->autoScrollSpeed;
                if (this->velocity.x > newVel)
                    this->velocity.x = newVel;
            }
            this->abilitySpeed = 0;
            this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_DROP, false, 0);
            this->state.Set(&Player::State_KnuxGlideDrop);
        }
        else {
            int32 velocity = this->abilitySpeed;

            if (this->timer == 256) {
                this->animator.speed = 1;

                if (this->abilitySpeed < 0x180000) {
                    if (this->superState == Player::SuperStateSuper)
                        this->abilitySpeed += 0x800;
                    else
                        this->abilitySpeed += 0x400;
                }
            }
            else {
                this->animator.speed = 0;

                if (this->abilitySpeed < 0x40000)
                    this->abilitySpeed += 0x1000;
            }

            if (this->timer < 256)
                this->timer += 4;

            int32 storeX = this->position.x;
            int32 highY = 0, lowY = 0;
            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR:
                    if (this->velocity.y <= 0x8000)
                        this->velocity.y += 0x2000;
                    else
                        this->velocity.y -= 0x2000;

                    if (this->isChibi) {
                        highY = -0x10000;
                        lowY  = 0x30000;
                    }
                    else {
                        highY = -0x20000;
                        lowY  = 0xB0000;
                    }
                    break;

                case CMODE_ROOF:
                    if (this->velocity.y <= -0x8000)
                        this->velocity.y += 0x2000;
                    else
                        this->velocity.y -= 0x2000;

                    if (this->isChibi) {
                        highY = 0x10000;
                        lowY  = -0x30000;
                    }
                    else {
                        highY = 0x20000;
                        lowY  = -0xB0000;
                    }
                    break;
            }

            bool32 collidedHigh = false, collidedLow = false;
            int32 highPos = 0, lowPos = 0;
            if (this->timer >= 128) {
                if (this->spriteType == ClassicSprites) {
                    switch (globals->gravityDir) {
                        default: break;

                        case CMODE_FLOOR: this->direction |= FLIP_X; break;

                        case CMODE_ROOF: this->direction &= ~FLIP_X; break;
                    }
                }

                this->position.x += this->velocity.x;
                int32 startPos = this->position.x;

                collidedHigh = this->TileCollision(this->collisionLayers, CMODE_RWALL, this->collisionPlane, -0xC0000, highY, true);
                highPos      = this->position.x;

                this->position.x = startPos;
                collidedLow      = this->TileCollision(this->collisionLayers, CMODE_RWALL, this->collisionPlane, -0xC0000, lowY, true);
                lowPos           = this->position.x;
            }

            int32 prevVelX   = this->velocity.x;
            bool32 isMoving  = this->velocity.x;
            this->velocity.x = this->abilitySpeed * Math::Cos512(this->timer) >> 9;
            if (!prevVelX && this->velocity.x) {
                isMoving           = false;
                this->abilitySpeed = 0x40000;
                this->velocity.x   = this->abilitySpeed * Math::Cos512(this->timer) >> 9;
            }

            this->position.x = storeX;

            if (this->spriteType == ClassicSprites) {
                if (this->timer < 44)
                    this->animator.frameID = 2;
                else if (this->timer < 86)
                    this->animator.frameID = 1;
                else if (this->timer < 170)
                    this->animator.frameID = 0;
                else if (this->timer < 212)
                    this->animator.frameID = 1;
                else
                    this->animator.frameID = 2;
            }

            if (this->right) {
                this->state.Set(&Player::State_KnuxGlideRight);

                if (this->spriteType == ManiaSprites) {
                    switch (globals->gravityDir) {
                        default: break;
                        case CMODE_FLOOR: this->direction &= ~FLIP_X; break;
                        case CMODE_ROOF: this->direction |= FLIP_X; break;
                    }

                    this->animator.frameID = 0;
                    this->animator.timer   = 0;
                }
            }
            else if (!isMoving && this->timer == 256) {
                if (collidedHigh || collidedLow) {
                    this->abilitySpeed   = 0;
                    this->outtaHereTimer = 0;
                    this->timer          = 0;

                    if (highPos == lowPos) {
                        if (velocity >= 0x45555 && this->hyperAbilityState != Player::HyperStateNone) {
                            this->hyperAbilityState = Player::HyperStateStartHyperSlam;
                            Camera::ShakeScreen(0, 8, 0, 1, 1);
                            sVars->sfxEarthquake.Play();
                        }

                        this->state.Set(&Player::State_KnuxWallClimb);
                        this->velocity.x     = 0;
                        this->velocity.y     = 0;
                        this->outtaHereTimer = 0;
                        this->timer          = 0;
                        this->position.x += 1 << 16;
                        sVars->sfxGrab.Play();
                    }
                    else {
                        this->velocity.x >>= 2;
                        if (Zone::sVars->autoScrollSpeed < -0x10000) {
                            int32 newVel = 0x10000 - Zone::sVars->autoScrollSpeed;
                            if (this->velocity.x > newVel)
                                this->velocity.x = newVel;
                        }
                        this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_DROP, false, 0);
                        this->state.Set(&Player::State_KnuxGlideDrop);
                    }
                }
                else if (collidedHigh) {
                    this->outtaHereTimer = 0;
                    this->timer          = 0;

                    this->velocity.x >>= 2;
                    if (Zone::sVars->autoScrollSpeed < -0x10000) {
                        int32 newVel = 0x10000 - Zone::sVars->autoScrollSpeed;
                        if (this->velocity.x > newVel)
                            this->velocity.x = newVel;
                    }
                    this->abilitySpeed = 0;
                    this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_DROP, false, 0);
                    this->state.Set(&Player::State_KnuxGlideDrop);
                }
            }
        }
    }
    else if (this->collisionMode == globals->gravityDir) {
        this->timer = 0;
        this->state.Set(&Player::State_KnuxGlideSlide);

        this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_SLIDE, false, 0);
        switch (globals->gravityDir) {
            default: break;

            case CMODE_FLOOR: this->groundVel = this->velocity.x; break;

            case CMODE_ROOF: this->groundVel = -this->velocity.x; break;
        }

        Dust *dust = GameObject::Create<Dust>(this, this->position.x, this->position.y);
        dust->state.Set(&Dust::State_GlideTrail);
    }
    else {
        this->state.Set(&Player::State_Ground);

        Gravity_False();

        this->skidding = 0;
    }

    switch (globals->gravityDir) {
        default: break;

        case CMODE_FLOOR:
            if (this->position.y < Zone::sVars->playerBoundsT[sceneInfo->entitySlot] + 0x100000) {
                this->velocity.x   = 0;
                this->abilitySpeed = 0;
            }
            break;

        case CMODE_ROOF:
            if (this->position.y > Zone::sVars->playerBoundsB[sceneInfo->entitySlot] - 0x100000) {
                this->velocity.x   = 0;
                this->abilitySpeed = 0;
            }
            break;
    }
}
void Player::State_KnuxGlideRight()
{
    SET_CURRENT_STATE();

    if (!this->onGround) {
        if (!this->jumpHold) {
            this->outtaHereTimer = 0;
            this->timer          = 0;

            this->velocity.x >>= 2;
            if (Zone::sVars->autoScrollSpeed < -0x10000) {
                int32 newVel = 0x10000 - Zone::sVars->autoScrollSpeed;
                if (this->velocity.x < newVel)
                    this->velocity.x = newVel;
            }

            this->abilitySpeed = 0;
            this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_DROP, false, 0);
            this->state.Set(&Player::State_KnuxGlideDrop);
        }
        else {
            int32 velocity = this->abilitySpeed;

            if (this->timer == 0) {
                this->animator.speed = 1;

                if (this->abilitySpeed < 0x180000) {
                    if (this->superState == Player::SuperStateSuper)
                        this->abilitySpeed += 0x800;
                    else
                        this->abilitySpeed += 0x400;
                }
            }
            else {
                this->animator.speed = 0;

                if (this->abilitySpeed < 0x40000)
                    this->abilitySpeed += 0x1000;
            }

            if (this->timer > 0)
                this->timer -= 4;

            int32 storeX = this->position.x;
            int32 highY = 0, lowY = 0;
            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR:
                    if (this->velocity.y <= 0x8000)
                        this->velocity.y += 0x2000;
                    else
                        this->velocity.y -= 0x2000;

                    if (this->isChibi) {
                        highY = -0x10000;
                        lowY  = 0x30000;
                    }
                    else {
                        highY = -0x20000;
                        lowY  = 0xB0000;
                    }
                    break;

                case CMODE_ROOF:
                    if (this->velocity.y <= -0x8000)
                        this->velocity.y += 0x2000;
                    else
                        this->velocity.y -= 0x2000;

                    if (this->isChibi) {
                        highY = 0x10000;
                        lowY  = -0x30000;
                    }
                    else {
                        highY = 0x20000;
                        lowY  = -0xB0000;
                    }
                    break;
            }

            bool32 collidedHigh = false, collidedLow = false;
            int32 highPos = 0, lowPos = 0;
            if (this->timer < 128) {
                if (this->spriteType == ClassicSprites) {
                    switch (globals->gravityDir) {
                        default: break;
                        case CMODE_FLOOR: this->direction &= ~FLIP_X; break;
                        case CMODE_ROOF: this->direction |= FLIP_X; break;
                    }
                }

                this->position.x += this->velocity.x;
                int32 startPos = this->position.x;

                collidedHigh = this->TileCollision(this->collisionLayers, CMODE_LWALL, this->collisionPlane, 0xC0000, highY, true);
                highPos      = this->position.x;

                this->position.x = startPos;
                collidedLow      = this->TileCollision(this->collisionLayers, CMODE_LWALL, this->collisionPlane, 0xC0000, lowY, true);
                lowPos           = this->position.x;
            }

            int32 prevVelX   = this->velocity.x;
            bool32 isMoving  = this->velocity.x;
            this->velocity.x = this->abilitySpeed * Math::Cos512(this->timer) >> 9;
            if (!prevVelX && this->velocity.x) {
                isMoving           = false;
                this->abilitySpeed = 0x40000;
                this->velocity.x   = this->abilitySpeed * Math::Cos512(this->timer) >> 9;
            }

            if (this->spriteType == ClassicSprites) {
                if (this->timer < 44)
                    this->animator.frameID = 2;
                else if (this->timer < 86)
                    this->animator.frameID = 1;
                else if (this->timer < 170)
                    this->animator.frameID = 0;
                else if (this->timer < 212)
                    this->animator.frameID = 1;
                else
                    this->animator.frameID = 2;
            }

            this->position.x = storeX;
            if (this->left) {
                this->state.Set(&Player::State_KnuxGlideLeft);

                if (this->spriteType == ManiaSprites) {
                    switch (globals->gravityDir) {
                        default: break;
                        case CMODE_FLOOR: this->direction |= FLIP_X; break;
                        case CMODE_ROOF: this->direction &= ~FLIP_X; break;
                    }

                    this->animator.frameID = 0;
                    this->animator.timer   = 0;
                }
            }
            else if (!isMoving && !this->timer) {
                if (collidedHigh || collidedLow) {
                    this->abilitySpeed   = 0;
                    this->outtaHereTimer = 0;
                    this->timer          = 0;

                    if (highPos == lowPos) {
                        if (velocity >= 0x45555 && this->hyperAbilityState != Player::HyperStateNone) {
                            this->hyperAbilityState = Player::HyperStateStartHyperSlam;
                            Camera::ShakeScreen(0, 8, 0, 1, 1);
                            sVars->sfxEarthquake.Play();
                        }

                        this->state.Set(&Player::State_KnuxWallClimb);
                        this->velocity.x     = 0;
                        this->velocity.y     = 0;
                        this->outtaHereTimer = 0;
                        this->timer          = 0;
                        this->position.x -= 1 << 16;
                        sVars->sfxGrab.Play();
                    }
                    else {
                        this->velocity.x >>= 2;
                        if (Zone::sVars->autoScrollSpeed < -0x10000) {
                            int32 newVel = 0x10000 - Zone::sVars->autoScrollSpeed;
                            if (this->velocity.x < newVel)
                                this->velocity.x = newVel;
                        }

                        this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_DROP, false, 0);
                        this->state.Set(&Player::State_KnuxGlideDrop);
                    }
                }
                else if (collidedHigh) {
                    this->outtaHereTimer = 0;
                    this->timer          = 0;

                    this->velocity.x >>= 2;
                    if (Zone::sVars->autoScrollSpeed < -0x10000) {
                        int32 newVel = 0x10000 - Zone::sVars->autoScrollSpeed;
                        if (this->velocity.x < newVel)
                            this->velocity.x = newVel;
                    }

                    this->abilitySpeed = 0;
                    this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_DROP, false, 0);
                    this->state.Set(&Player::State_KnuxGlideDrop);
                }
            }
        }
    }
    else if (this->collisionMode == globals->gravityDir) {
        this->timer = 0;
        this->state.Set(&Player::State_KnuxGlideSlide);

        this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_SLIDE, false, 0);
        switch (globals->gravityDir) {
            default: break;

            case CMODE_FLOOR: this->groundVel = this->velocity.x; break;

            case CMODE_ROOF: this->groundVel = -this->velocity.x; break;
        }

        Dust *dust = GameObject::Create<Dust>(this, this->position.x, this->position.y);
        dust->state.Set(&Dust::State_GlideTrail);
    }
    else {
        this->state.Set(&Player::State_Ground);

        Gravity_False();

        this->skidding = 0;
    }

    switch (globals->gravityDir) {
        default: break;

        case CMODE_FLOOR:
            if (this->position.y < Zone::sVars->playerBoundsT[sceneInfo->entitySlot] + 0x100000) {
                this->velocity.x   = 0;
                this->abilitySpeed = 0;
            }
            break;

        case CMODE_ROOF:
            if (this->position.y > Zone::sVars->playerBoundsB[sceneInfo->entitySlot] - 0x100000) {
                this->velocity.x   = 0;
                this->abilitySpeed = 0;
            }
            break;
    }
}
void Player::State_KnuxGlideDrop()
{
    SET_CURRENT_STATE();

    if (this->onGround) {
        if (!this->timer)
            sVars->sfxLand.Play();

        Gravity_False();

        if (abs(Zone::sVars->autoScrollSpeed) > 0x20000) {
            if (Zone::sVars->autoScrollSpeed <= 0) {
                this->groundVel  = Zone::sVars->autoScrollSpeed + 0x20000;
                this->velocity.x = Zone::sVars->autoScrollSpeed + 0x20000;
            }
            else {
                this->groundVel  = Zone::sVars->autoScrollSpeed - 0x20000;
                this->velocity.x = Zone::sVars->autoScrollSpeed - 0x20000;
            }
            this->timer = 16;
        }
        else {
            this->groundVel  = 0;
            this->velocity.x = 0;
            this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_LAND, false, 0);
        }

        if (this->timer >= 16) {
            this->state.Set(&Player::State_Ground);
            this->skidding       = 0;
            this->timer          = 0;
            this->outtaHereTimer = 0;
        }
        else {
            if (this->down && this->jumpPress) {
                Action_Spindash();
            }
            else if (this->jumpPress) {
                Action_Jump();
                this->timer          = 0;
                this->outtaHereTimer = 0;
            }
            else {
                this->timer++;
            }
        }
    }
    else {
        HandleAirFriction();
        Gravity_True();
    }
}
void Player::State_KnuxGlideSlide()
{
    SET_CURRENT_STATE();

    if (!this->onGround) {
        this->timer          = 0;
        this->outtaHereTimer = 0;
        this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_DROP, false, 0);
        this->state.Set(&Player::State_KnuxGlideDrop);
    }
    else {
        if (this->groundVel && !Zone::sVars->autoScrollSpeed) {
            if (!this->timer)
                sVars->sfxSlide.Play();

            this->timer = (this->timer + 1) & 7;
            if (this->animator.frameID == 2) {
                this->animator.speed = 0;
                this->animator.timer = 0;
            }

            bool32 canGetUp = true;
            if (this->groundVel <= 0) {
                this->groundVel += 0x2000;
                if (this->groundVel < 0)
                    canGetUp = false;
            }
            else {
                this->groundVel -= 0x2000;
                if (this->groundVel > 0)
                    canGetUp = false;
            }

            if (canGetUp || !this->jumpHold) {
                this->animator.frameID = this->spriteType == ManiaSprites ? 3 : 2;
                this->timer            = 0;
                this->groundVel        = 0;
            }
        }
        else {
            Gravity_False();

            this->animator.speed = 1;

            if (this->timer >= 16) {
                this->state.Set(&Player::State_Ground);
                this->skidding = 0;
            }
            else {
                this->timer++;
            }
        }

        if (this->animator.frameID && this->down)
            this->state.Set(&Player::State_Crouch);
    }
}
void Player::State_KnuxWallClimb()
{
    SET_CURRENT_STATE();

    if (this->onGround) {
        this->state.Set(&Player::State_Ground);

        Gravity_False();
    }
    else {
        bool32 up   = false;
        bool32 down = false;

        switch (globals->gravityDir) {
            default: break;

            case CMODE_FLOOR:
                up   = this->up;
                down = this->down;
                break;

            case CMODE_ROOF:
                up   = this->down;
                down = this->up;
                break;
        }

        if (up) {
            if (this->superState == Player::SuperStateSuper)
                this->velocity.y = -0x20000;
            else
                this->velocity.y = -0x10000;

            int32 boundary = Zone::sVars->playerBoundsT[sceneInfo->entitySlot] + 0x140000;
            if (this->position.y < boundary)
                this->position.y = boundary;
        }
        else if (down) {
            if (this->superState == Player::SuperStateSuper)
                this->velocity.y = 0x20000;
            else
                this->velocity.y = 0x10000;
        }
        else {
            this->velocity.y = 0;
        }

        if (this->jumpPress) {
            this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
            this->animator.speed = 120;
            this->state.Set(&Player::State_Air);
            this->applyJumpCap     = false;
            this->jumpAbilityState = 1;

            sVars->sfxJump.Play();

            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR:
                    if (this->direction & FLIP_X) {
                        this->velocity.x = 0x40000;
                        this->groundVel  = 0x40000;
                        this->direction  = FLIP_NONE;
                    }
                    else {
                        this->velocity.x = -0x40000;
                        this->groundVel  = -0x40000;
                        this->direction  = FLIP_X;
                    }

                    this->velocity.y = -0x38000;
                    break;

                case CMODE_ROOF:
                    if (!(this->direction & FLIP_X)) {
                        this->velocity.x = 0x40000;
                        this->groundVel  = 0x40000;
                        this->direction  = FLIP_NONE;
                    }
                    else {
                        this->velocity.x = -0x40000;
                        this->groundVel  = -0x40000;
                        this->direction  = FLIP_X;
                    }

                    this->velocity.y = 0x38000;
                    break;
            }

            if (this->underwater) {
                this->velocity.x >>= 1;
                this->velocity.y >>= 1;
                this->groundVel >>= 1;
            }
        }
        else {
            Hitbox *hitbox   = this->GetHitbox();
            Hitbox *innerBox = this->GetAltHitbox();
            int32 storeX     = this->position.x;

            int32 highY = 0;
            int32 lowY  = 0;

            int32 roofX = 0;
            int32 roofY = 0;

            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR:
                    if (this->isChibi) {
                        highY = -0x40000;
                        lowY  = 0x40000;
                        roofY = -0xC0000;
                    }
                    else {
                        highY = -0xA0000;
                        lowY  = 0xB0000;
                        roofY = -0x140000;
                    }
                    break;

                case CMODE_ROOF:
                    if (this->isChibi) {
                        highY = 0x40000;
                        lowY  = -0x40000;
                        roofY = 0xC0000;
                    }
                    else {
                        highY = 0xA0000;
                        lowY  = -0xB0000;
                        roofY = 0x140000;
                    }
                    break;
            }

            bool32 collidedHigh = false, collidedLow = false;
            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR:
                    if (this->direction & FLIP_X) {
                        collidedHigh = this->TileGrip(this->collisionLayers, CMODE_RWALL, this->collisionPlane, hitbox->left << 16, highY, 8);
                        int32 highX  = this->position.x;

                        this->position.x = storeX;
                        collidedLow      = this->TileGrip(this->collisionLayers, CMODE_RWALL, this->collisionPlane, hitbox->left << 16, lowY, 8);
                        if (this->velocity.y > 0 && this->position.x < highX)
                            this->velocity.y = 0;
                        roofX = -0x40000;
                    }
                    else {
                        collidedHigh = this->TileGrip(this->collisionLayers, CMODE_LWALL, this->collisionPlane, hitbox->right << 16, highY, 8);
                        int32 highY  = this->position.x;

                        this->position.x = storeX;
                        collidedLow      = this->TileGrip(this->collisionLayers, CMODE_LWALL, this->collisionPlane, hitbox->right << 16, lowY, 8);
                        if (this->velocity.y < 0 && this->position.x > highY)
                            this->velocity.y = 0;
                        roofX = 0x40000;
                    }

                    this->position.y += this->velocity.y;

                    if (this->TileCollision(this->collisionLayers, CMODE_FLOOR, this->collisionPlane, roofX, roofY, true))
                        this->velocity.y = 0;
                    break;

                case CMODE_ROOF:
                    if (!(this->direction & FLIP_X)) {
                        collidedHigh = this->TileGrip(this->collisionLayers, CMODE_RWALL, this->collisionPlane, hitbox->left << 16, highY, 8);
                        int32 highX  = this->position.x;

                        this->position.x = storeX;
                        collidedLow      = this->TileGrip(this->collisionLayers, CMODE_RWALL, this->collisionPlane, hitbox->left << 16, lowY, 8);
                        if (this->velocity.y < 0 && this->position.x < highX)
                            this->velocity.y = 0;
                        roofX = -0x40000;
                    }
                    else {
                        collidedHigh = this->TileGrip(this->collisionLayers, CMODE_LWALL, this->collisionPlane, hitbox->right << 16, highY, 8);
                        int32 highY  = this->position.x;

                        this->position.x = storeX;
                        collidedLow      = this->TileGrip(this->collisionLayers, CMODE_LWALL, this->collisionPlane, hitbox->right << 16, lowY, 8);
                        if (this->velocity.y < 0 && this->position.x > highY)
                            this->velocity.y = 0;
                        roofX = 0x40000;
                    }

                    this->position.y += this->velocity.y;

                    if (this->TileCollision(this->collisionLayers, CMODE_ROOF, this->collisionPlane, roofX, roofY, true))
                        this->velocity.y = 0;
                    break;
            }

            if (collidedHigh && collidedLow) {
                if (!this->velocity.y)
                    this->animator.SetAnimation(this->aniFrames, ANI_CLIMB_IDLE, false, 0);
                else if (this->velocity.y > 0)
                    this->animator.SetAnimation(this->aniFrames, ANI_CLIMB_DOWN, false, 0);
                else if (this->velocity.y < 0)
                    this->animator.SetAnimation(this->aniFrames, ANI_CLIMB_UP, false, 0);

                this->velocity.y = 0;
            }
            else if (collidedHigh) {
                this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_DROP, false, 2);
                this->velocity.y = 0;
                this->state.Set(&Player::State_KnuxGlideDrop);
            }
            else if (collidedLow) {
                this->position.y &= 0xFFF00000;
                if (this->isChibi)
                    this->position.y -= 0x10000;
                else
                    this->position.y += 0xA0000;

                this->velocity.y = 0;
                this->position.x = storeX;
                this->state.Set(&Player::State_KnuxLedgePullUp);
                this->timer          = 1;
                this->tileCollisions = TILECOLLISION_NONE;
                this->velocity.y     = 0;
            }
        }
    }
}
void Player::State_KnuxLedgePullUp()
{
    SET_CURRENT_STATE();

    this->left  = false;
    this->right = false;

    Animator backup;
    memcpy(&backup, &this->animator, sizeof(backup));

    Hitbox *outerBox = this->GetHitbox();
    Hitbox *innerBox = this->GetAltHitbox();

    Vector2 velocity;
    velocity.x = 0;
    velocity.y = 0;

    switch (this->spriteType) {
        case ClassicSprites:
            this->animator.SetAnimation(this->aniFrames, ANI_LEDGE_PULL_UP, false, 0);

            switch (this->animator.frameID) {
                case 0:
                    if (this->timer < 5) {
                        this->timer++;
                    }
                    else {
                        this->timer            = 0;
                        this->animator.frameID = 1;

                        switch (globals->gravityDir) {
                            default: break;

                            case CMODE_FLOOR:
                                if (this->direction & FLIP_X)
                                    this->position.x += -0x90000;
                                else
                                    this->position.x += 0x90000;

                                this->position.y += -0xA0000;
                                break;

                            case CMODE_ROOF:
                                if (this->direction & FLIP_X)
                                    this->position.x += 0x90000;
                                else
                                    this->position.x += -0x90000;

                                this->position.y += 0xA0000;
                                break;
                        }
                    }
                    break;

                case 1:
                    if (this->timer < 5) {
                        this->timer++;
                    }
                    else {
                        this->timer            = 0;
                        this->outtaHereTimer   = 0;
                        this->animator.frameID = 2;

                        switch (globals->gravityDir) {
                            default: break;

                            case CMODE_FLOOR:
                                if (this->direction & FLIP_X)
                                    this->position.x += -0x50000;
                                else
                                    this->position.x += 0x50000;

                                this->position.y += -0x20000;
                                break;

                            case CMODE_ROOF:
                                if (this->direction & FLIP_X)
                                    this->position.x += 0x50000;
                                else
                                    this->position.x += -0x50000;

                                this->position.y += 0x20000;
                                break;
                        }
                    }
                    break;

                case 2:
                    if (this->timer < 5) {
                        this->timer++;
                    }
                    else {
                        this->animator.SetAnimation(this->aniFrames, ANI_IDLE, false, 0);
                        this->timer          = 0;
                        this->outtaHereTimer = 0;
                        this->onGround       = true;
                        this->state.Set(&Player::State_Air);
                        this->tileCollisions = globals->tileCollisionMode;

                        switch (globals->gravityDir) {
                            default: break;

                            case CMODE_FLOOR: this->position.y += -0xA0000; break;

                            case CMODE_ROOF: this->position.y += 0xA0000; break;
                        }
                    }
                    break;
            }
            break;

        case ManiaSprites:
            this->animator.SetAnimation(this->aniFrames, ANI_LEDGE_PULL_UP, false, 1);

            this->animator.Process();

            if (this->timer != this->animator.frameID && this->animator.frameID < 6) {
                this->timer = this->animator.frameID;

                switch (globals->gravityDir) {
                    default: break;

                    case CMODE_FLOOR:
                        if (!this->isChibi) {
                            if (this->direction & FLIP_X)
                                this->position.x += -0x50000;
                            else
                                this->position.x += 0x50000;

                            this->position.y += -0x80000;
                        }
                        else {
                            if (this->direction & FLIP_X)
                                this->position.x += -0x40000;
                            else
                                this->position.x += 0x40000;

                            this->position.y += -0x40000;
                        }
                        break;

                    case CMODE_ROOF:
                        if (!this->isChibi) {
                            if (this->direction & FLIP_X)
                                this->position.x += 0x50000;
                            else
                                this->position.x += -0x50000;

                            this->position.y += 0x80000;
                        }
                        else {
                            if (this->direction & FLIP_X)
                                this->position.x += 0x40000;
                            else
                                this->position.x += -0x40000;

                            this->position.y += 0x40000;
                        }
                        break;
                }
            }

            if (this->animator.frameID == 6) {
                this->onGround       = true;
                this->tileCollisions = globals->tileCollisionMode;
            }

            if (this->animator.frameID == this->animator.frameCount - 1) {
                this->timer          = 0;
                this->outtaHereTimer = 0;
                this->state.Set(&Player::State_Ground);
            }

            memcpy(&this->animator, &backup, sizeof(backup));
            break;
    }
}

void Player::State_FlyToPlayer()
{
    SET_CURRENT_STATE();

    Player *leader = nullptr;
    if (this->playerID)
        leader = GameObject::Get<Player>(SLOT_PLAYER1);
    // else
    //     leader = (EntityPlayer *)Zone->entityStorage[1];

    sVars->respawnTimer  = 0;
    this->tileCollisions = TILECOLLISION_NONE;
    this->interaction    = false;

    switch (this->characterID) {
        default:
        case ID_SONIC:

        case ID_TAILS:
            if (Water::sVars && this->position.y > Water::sVars->waterLevel)
                this->animator.SetAnimation(this->aniFrames, ANI_SWIM, false, 0);
            else
                this->animator.SetAnimation(this->aniFrames, ANI_FLY, false, 0);

            this->scale.x = 0x200;
            this->scale.y = 0x200;
            this->drawFX &= ~FX_SCALE;
            break;

        case ID_KNUCKLES:
            this->animator.SetAnimation(this->aniFrames, ANI_GLIDE, false, 0);
            this->scale.x = 0x200;
            this->scale.y = 0x200;
            this->drawFX &= ~FX_SCALE;
            break;
    }

    if (leader->underwater && leader->position.y < Water::sVars->waterLevel)
        this->drawGroup = leader->drawGroup;
    else
        this->drawGroup = Zone::sVars->playerDrawGroup[1];

    Entity *parent = (Entity *)this->abilityPtrs[0];
    int32 screenX  = (screenInfo->size.x + screenInfo->center.x) << 16;
    int32 screenY  = (screenInfo->size.y + screenInfo->center.y) << 16;
    if (parent->position.x < leader->position.x - screenX || parent->position.x > screenX + leader->position.x
        || parent->position.y < leader->position.y - screenY || parent->position.y > leader->position.y + screenY) {
        parent->position = leader->position;

        switch (globals->gravityDir) {
            default: break;
            case CMODE_FLOOR: parent->position.y = (screenInfo->position.y - 128) << 16; break;
            case CMODE_ROOF: parent->position.y = (screenInfo->position.y + screenInfo->size.y + 128) << 16; break;
        }
    }

    if (this->camera && this->camera->target != parent) {
        if (globals->gameMode != MODE_ENCORE) {
            this->camera->position.x = parent->position.x;
            this->camera->position.y = parent->position.y;
            Camera::SetTargetEntity(this->camera->screenID, parent);
        }
    }

    int32 maxDistance = 0;
    if (this->abilityValues[0] || this->characterID == ID_TAILS || this->characterID == ID_KNUCKLES) {
        maxDistance    = sVars->targetLeaderPosition.x - parent->position.x;
        int32 distance = maxDistance >> 4;

        if (this->characterID != ID_TAILS && this->characterID != ID_KNUCKLES) {
            distance = ((sVars->targetLeaderPosition.x - parent->position.x) >> 5) + ((sVars->targetLeaderPosition.x - parent->position.x) >> 4);
        }

        if (parent->position.x >= sVars->targetLeaderPosition.x + 0x40000) {
            this->direction = (this->direction & ~FLIP_X) | (globals->tileCollisionMode == 1 ? FLIP_X : FLIP_NONE);

            if (distance < -0xC0000)
                distance = -0xC0000;

            if (leader->velocity.x < 0)
                distance += leader->velocity.x;

            distance -= 0x10000;
            if (distance < maxDistance) {
                distance    = maxDistance;
                maxDistance = 0;
            }
        }
        else if (parent->position.x <= sVars->targetLeaderPosition.x - 0x40000) {
            this->direction = (this->direction & ~FLIP_X) | (globals->tileCollisionMode != 1 ? FLIP_X : FLIP_NONE);

            if (distance > 0xC0000)
                distance = 0xC0000;

            if (leader->velocity.x > 0)
                distance += leader->velocity.x;

            distance += 0x10000;
            if (distance > maxDistance) {
                distance    = maxDistance;
                maxDistance = 0;
            }
        }
        parent->position.x += distance;

        int32 yVel = 0;
        if (this->characterID == ID_TAILS || this->characterID == ID_KNUCKLES) {
            yVel = 0x10000;
            if (globals->gameMode == MODE_ENCORE)
                yVel = 0x20000;
        }
        else {
            yVel = 0x30000;
        }

        if (parent->position.y > sVars->targetLeaderPosition.y) {
            parent->position.y -= yVel;

            switch (this->animator.animationID) {
                default: break;

                case ANI_FLY: this->animator.speed = 2; break;
                case ANI_SWIM: this->animator.speed = 40; break;
            }
        }
        else if (parent->position.y < sVars->targetLeaderPosition.y) {
            parent->position.y += yVel;

            switch (this->animator.animationID) {
                default: break;

                case ANI_FLY: this->animator.speed = 1; break;
                case ANI_SWIM: this->animator.speed = 60; break;
            }
        }

        parent->position.x &= 0xFFFF0000;
        parent->position.y &= 0xFFFF0000;
    }
    else {
        this->drawGroup    = Zone::sVars->playerDrawGroup[1];
        parent->position.x = sVars->targetLeaderPosition.x;

        switch (globals->gravityDir) {
            default: break;

            case CMODE_FLOOR:
                parent->position.y = (screenInfo->position.y + screenInfo->center.y + 32) << 16;
                parent->position.y += (screenInfo->center.y - 32) * Math::Sin512(this->angle) << 8;
                break;

            case CMODE_ROOF:
                parent->position.y = (screenInfo->position.y + screenInfo->center.y - 32) << 16;
                parent->position.y += (screenInfo->center.y + 32) * Math::Sin512(this->angle) << 8;
                break;
        }

        this->drawFX |= FX_SCALE;
        this->scale.x = 0x2000 - 16 * this->angle - 8 * this->angle;
        this->scale.y = 0x2000 - 16 * this->angle - 8 * this->angle;
        if (this->scale.x < 0x200) {
            this->scale.x = 0x200;
            this->scale.y = 0x200;
        }

        bool32 inPlace = false;
        switch (globals->gravityDir) {
            default: break;
            case CMODE_FLOOR: inPlace = parent->position.y >= leader->position.y; break;
            case CMODE_ROOF: inPlace = parent->position.y <= leader->position.y; break;
        }

        this->angle += 3;
        if (this->angle >= 0x200 || (this->angle >= 368 && inPlace)) {
            this->abilityValues[0] = 1;
            this->drawFX &= ~FX_SCALE;
            this->scale.x    = 0x200;
            this->scale.y    = 0x200;
            this->position.x = parent->position.x;
        }
    }

    if (leader->classID == sVars->classID) {
        if (!leader->state.Matches(&Player::State_Death) && !leader->state.Matches(&Player::State_Drown)
            && !leader->state.Matches(&Player::State_TubeRoll)) {
            if (abs(maxDistance) <= 0x40000 && abs(sVars->targetLeaderPosition.y - parent->position.y) < 0x20000)
                FinishedReturnToPlayer(leader);
        }
    }
}
void Player::State_ReturnToPlayer()
{
    SET_CURRENT_STATE();

    Entity *parent = (Entity *)this->abilityPtrs[0];
    Player *leader = nullptr;
    if (this->playerID)
        leader = GameObject::Get<Player>(SLOT_PLAYER1);
    // else
    //     leader = (EntityPlayer *)Zone->entityStorage[1];

    this->animator.SetAnimation(this->aniFrames, ANI_JUMP, false, 0);
    if (leader->CheckValidState()) {
        parent->position.x = leader->position.x;
        parent->position.y = leader->position.y;

        switch (globals->gravityDir) {
            default: break;
            case CMODE_FLOOR: parent->position.y -= 0x180000; break;
            case CMODE_ROOF: parent->position.y += 0x180000; break;
        }
    }

    switch (globals->gravityDir) {
        default: break;
        case CMODE_FLOOR: this->velocity.y += 0x4800; break;
        case CMODE_ROOF: this->velocity.y -= 0x4800; break;
    }

    this->onGround   = false;
    this->velocity.x = 0;
    this->abilityValues[0] += this->velocity.y;
    this->position.x = parent->position.x;
    this->position.y = parent->position.y + this->abilityValues[0];

    if (this->scale.x > 0x200) {
        this->scale.x -= 10;
        if (this->scale.x <= 0x200) {
            this->drawFX &= ~FX_SCALE;
            this->scale.x = 0x200;
        }
        this->scale.y = this->scale.x;
    }

    if (this->velocity.y >= 0) {
        bool32 inPlace = false;
        switch (globals->gravityDir) {
            default: break;
            case CMODE_FLOOR: inPlace = this->position.y >= parent->position.y; break;
            case CMODE_ROOF: inPlace = this->position.y <= parent->position.y; break;
        }

        if (inPlace) {
            this->drawFX &= ~FX_SCALE;
            this->scale.x = 0x200;
            this->scale.y = 0x200;
            FinishedReturnToPlayer(leader);
        }
    }
}
void Player::State_HoldRespawn()
{
    SET_CURRENT_STATE();

    if (++this->abilityValues[0] == 2) {
        Player *leader = GameObject::Get<Player>(SLOT_PLAYER1);

        if (leader->classID == sVars->classID) {
            Dust *dust             = GameObject::Create<Dust>(true, leader->position.x, leader->position.y);
            dust->visible          = false;
            dust->active           = ACTIVE_NEVER;
            dust->isPermanent      = true;
            dust->position.y       = (screenInfo->position.y - 128) << 16;
            this->tileCollisions   = TILECOLLISION_NONE;
            this->interaction      = false;
            this->forceHoldRespawn = false;
            this->drawGroup        = Zone::sVars->playerDrawGroup[1] + 1;
            this->angle            = 128;

            if ((this->characterID != ID_TAILS && this->characterID != ID_KNUCKLES) || globals->gameMode == MODE_ENCORE) {
                this->state.Set(&Player::State_ReturnToPlayer);
                this->drawFX |= FX_SCALE;
                this->scale.x = 0x400;
                this->scale.y = 0x400;

                this->abilityValues[0] = 0x100000 - leader->position.y + ((screenInfo->size.y + screenInfo->position.y) << 16);
                if (this->abilityValues[0] < 0xA80000)
                    this->velocity.y = this->abilityValues[0] / -12;
                else
                    this->velocity.y = -0xE0000;
            }
            else {
                this->state.Set(&Player::State_FlyToPlayer);
                this->abilityValues[0] = 0;
            }
            this->abilityPtrs[0] = dust;
        }
    }
}
void Player::State_Victory()
{
    SET_CURRENT_STATE();

    this->applyJumpCap = false;
    this->drownTimer   = 0;

    if (!this->onGround) {
        Player::HandleAirFriction();
    }

    if (this->onGround) {
        if (this->groundedStore) {
            this->groundVel = 0;
            sVars->sfxFlying.Stop();
            this->animator.SetAnimation(this->aniFrames, ANI_VICTORY, false, 0);
        }
    }
}
void Player::State_TubeRoll()
{
    SET_CURRENT_STATE();

    HandleGroundRotation();

    bool32 left  = this->left;
    bool32 right = this->right;
    if (this->controlLock > 0) {
        this->left  = false;
        this->right = false;
        this->controlLock--;
    }

    HandleRollDeceleration();

    this->left         = left;
    this->right        = right;
    this->applyJumpCap = false;

    if (!this->onGround) {
        this->state.Set(&Player::State_TubeAirRoll);
        Gravity_True();
    }
    else {
        if (this->characterID == ID_TAILS)
            this->animator.speed = 120;
        else
            this->animator.speed = ((abs(this->groundVel) * 0xF0) / 0x60000) + 0x30;

        if (this->animator.speed > 0xF0)
            this->animator.speed = 0xF0;

        Gravity_False();
    }
}
void Player::State_TubeAirRoll()
{
    SET_CURRENT_STATE();

    HandleGroundRotation();

    bool32 left  = this->left;
    bool32 right = this->right;
    if (this->controlLock > 0) {
        this->left  = false;
        this->right = false;
        this->controlLock--;
    }

    HandleAirFriction();

    this->left         = left;
    this->right        = right;
    this->applyJumpCap = false;

    this->nextGroundState.Set(&Player::State_TubeRoll);

    if (!this->onGround) {
        Gravity_True();
    }
    else {
        this->state.Set(&Player::State_TubeRoll);

        Gravity_False();
    }
}

void Player::State_TransportTube()
{
    SET_CURRENT_STATE();

    this->onGround = false;

    if (this->characterID == ID_TAILS) {
        this->tailAnimator.SetAnimation(this->tailFrames, 1, false, 0);

        if (this->zdepth != 1 && (this->zdepth == 2 || !this->onGround)) {
            this->tailRotation = Math::ATan2(this->velocity.x, this->velocity.y) << 1;
            if (this->direction & FLIP_X)
                this->tailRotation = (this->tailRotation + 256) & 0x1FE;
            this->tailDirection = this->direction;
        }
        else {
            this->tailRotation = this->rotation;

            if (!this->groundVel)
                this->tailDirection = this->direction;
            else if (this->groundVel > 0)
                this->tailDirection &= ~FLIP_X;
            else if (this->groundVel < 0)
                this->tailDirection |= FLIP_X;
        }
    }
}

void Player::State_WaterCurrent() { SET_CURRENT_STATE(); }

void Player::State_GroundFalse()
{
    SET_CURRENT_STATE();
    State_Ground();
}

void Player::State_Transform()
{
    SET_CURRENT_STATE();

    this->position.x += Zone::sVars->autoScrollSpeed;
    this->invincibleTimer = 60;
    ++this->timer;

    if (this->animator.GetFrameID() == '0') {
        sVars->sfxTransform2.Play();
        this->superState = Player::SuperStateSuper;
        UpdatePhysicsState();
        this->abilityValues[4] = true;
    }

    if (!this->abilityValues[5] && this->animator.GetFrameID() == '1') {
        this->velocity.x       = this->abilityValues[6];
        this->velocity.y       = this->abilityValues[7];
        this->abilityValues[6] = 0;
        this->abilityValues[7] = 0;
        this->abilityValues[5] = true;

        if (this->hyperAbilityState == Player::HyperStateNone) {
            SuperSparkle *sparkle = GameObject::Get<SuperSparkle>(this->playerID + sVars->maxPlayerCount);
            sparkle->Reset(SuperSparkle::sVars->classID, this);
            sparkle->timer = 13;

            if (globals->useManiaBehavior) {
                ImageTrail *trail = GameObject::Get<ImageTrail>(this->playerID + sVars->maxPlayerCount * 2);
                trail->Reset(ImageTrail::sVars->classID, this);
            }
        }
        else {
            SuperSparkle *sparkle = GameObject::Get<SuperSparkle>(this->playerID + sVars->maxPlayerCount);
            sparkle->Reset(SuperSparkle::sVars->classID, this);
            sparkle->timer = 13;

            ImageTrail *trail = GameObject::Get<ImageTrail>(this->playerID + sVars->maxPlayerCount * 2);
            trail->Reset(ImageTrail::sVars->classID, this);
        }

    }

    bool32 done = false;
    if (this->abilityValues[5]) {
        this->interaction = true;
        if (!this->onGround) {
            this->stateGravity.Set(&Player::Gravity_True);
            State_Air();
            done = true;
        }
        else {
            done = true;
        }
    }
    else {
        done = true;
    }

    if (done) {
        if (!this->abilityValues[4]) {
            this->superBlendTimer = 0;
            this->superTableIndex = 0;
            this->velocity.y      = 0;
            this->stateGravity.Set(&Player::Gravity_NULL);
        }
    }

    bool32 finished = false;
    if (this->animator.frameID == this->animator.frameCount - 1) {
        if (!this->abilityValues[5]) {
            this->superState = Player::SuperStateSuper;
            UpdatePhysicsState();
        }
        finished = true;
    }

    if (finished || (this->abilityValues[5] && this->onGround)) {
        this->isTransforming   = false;
        this->abilityValues[6] = 0;
        this->abilityValues[7] = 0;
        this->abilityValues[4] = 0;
        this->abilityValues[5] = 0;
        this->interaction      = true;
        this->timer            = 0;
        this->outtaHereTimer   = 0;

        if (this->onGround) {
            this->state.Set(&Player::State_Ground);
        }
        else {
            this->state.Set(&Player::State_Air);
            this->animator.SetAnimation(this->aniFrames, ANI_WALK, false, 0);
        }
    }
}

void Player::State_StartSuper()
{
    SET_CURRENT_STATE();

    if (this->characterID == ID_SONIC)
        this->aniFrames = sVars->superFrames;

    this->superState = Player::SuperStateSuper;

    GameObject::Get(this->playerID + sVars->maxPlayerCount)->Destroy();
    this->invincibleTimer = 60;
    this->superState      = Player::SuperStateSuper;
    this->UpdatePhysicsState();

    this->state.SetAndRun(&Player::State_Ground, this);
}

void Player::State_SuperFlying()
{
    SET_CURRENT_STATE();

    Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);

    if (player1->onGround) {
        player1->state.Set(&Player::State_Ground);
        player1->animator.SetAnimation(player1->aniFrames, ANI_IDLE, false, 0);
    }
    else {
        if (player1->up) {
            if (this->velocity.y > -player1->topSpeed) {
                this->velocity.y -= player1->acceleration;

                if (player1->velocity.y > 0)
                    this->velocity.y -= player1->acceleration;
            }
        }
        else if (player1->down) {
            if (this->velocity.y < player1->topSpeed) {
                this->velocity.y += player1->acceleration;

                if (player1->velocity.y < 0)
                    this->velocity.y += player1->acceleration;
            }
        }
        else {
            if (player1->velocity.y) {
                if (player1->velocity.y < 0) {
                    player1->velocity.y += player1->deceleration;

                    if (player1->velocity.y > 0)
                        player1->velocity.y = 0;
                }
                else {
                    player1->velocity.y -= player1->deceleration;

                    if (player1->velocity.y < 0)
                        player1->velocity.y = 0;
                }
            }
        }

        int32 velY = player1->velocity.y;
        if (velY >= -player1->topSpeed) {
            if (velY > player1->topSpeed)
                player1->velocity.y = velY - (player1->acceleration >> 1) - player1->acceleration;
        }
        else {
            player1->velocity.y = player1->acceleration + velY + (player1->acceleration >> 1);
        }

        if (player1->left) {
            if (player1->characterID == ID_KNUCKLES && this->direction == FLIP_NONE)
                player1->animator.SetAnimation(player1->aniFrames, ANI_GLIDE, false, 0);

            if (this->velocity.x > -player1->topSpeed) {
                this->velocity.x -= player1->acceleration;

                if (player1->velocity.x > 0)
                    this->velocity.x -= player1->acceleration;
            }

            player1->direction = FLIP_X;
        }
        else if (player1->right) {
            if (player1->characterID == ID_KNUCKLES && this->direction == FLIP_X)
                player1->animator.SetAnimation(player1->aniFrames, ANI_IDLE, false, 0);

            if (this->velocity.x < player1->topSpeed) {
                this->velocity.x += player1->acceleration;

                if (player1->velocity.x < 0)
                    this->velocity.x += player1->acceleration;
            }

            player1->direction = FLIP_NONE;
        }
        else {
            if (player1->velocity.x) {
                if (player1->velocity.x < 0) {
                    player1->velocity.x += player1->deceleration;

                    if (player1->velocity.x > 0)
                        player1->velocity.x = 0;
                }
                else {
                    player1->velocity.x -= player1->deceleration;

                    if (player1->velocity.x < 0)
                        player1->velocity.x = 0;
                }
            }
        }

        int32 velX = player1->velocity.x;
        if (velX >= -player1->topSpeed) {
            if (velX > player1->topSpeed)
                player1->velocity.x = velX - (player1->acceleration >> 1) - player1->acceleration;
        }
        else {
            player1->velocity.x = player1->acceleration + velX + (player1->acceleration >> 1);
        }

        if (player1->abilityValues[0]) {
            player1->abilityValues[0]--;
        }
        else if (player1->jumpPress) {
            if (player1->rings > 10)
                DoSuperDash(player1);
        }
    }
}

// Gravity States
void Player::Gravity_NULL() { SET_CURRENT_STATE(); }
void Player::Gravity_True()
{
    SET_CURRENT_STATE();

    if (this->camera) {
        this->camera->offset.y       = 0x200000;
        this->camera->disableYOffset = true;
    }

    this->velocity.y += this->gravityStrength;

    switch (globals->gravityDir) {
        case CMODE_FLOOR:
            if (this->velocity.y < this->jumpCap && this->animator.animationID == ANI_JUMP && !this->jumpHold && this->applyJumpCap) {
                this->velocity.x -= this->velocity.x >> 5;
                this->velocity.y = this->jumpCap;
            }
            break;

        case CMODE_ROOF:
            if (this->velocity.y > -this->jumpCap && this->animator.animationID == ANI_JUMP && !this->jumpHold && this->applyJumpCap) {
                this->velocity.x -= this->velocity.x >> 5;
                this->velocity.y = -this->jumpCap;
            }
            break;

        default: break;
    }

    if (this->animator.rotationStyle != Animator::RotateStaticFrames) {
        this->rotation += globals->gravityDir << 7;

        if (this->rotation >= 0x100) {
            if (this->rotation < 0x200)
                this->rotation += 4;
        }
        else if (this->rotation > 0)
            this->rotation -= 4;
        else
            this->rotation = 0;
        this->rotation -= globals->gravityDir << 7;
    }
    else {
        if (!globals->gravityDir) {
            if (this->rotation >= 0x100) {
                if (this->rotation < 0x200)
                    this->rotation += 4;
            }
            else if (this->rotation > 0)
                this->rotation -= 4;
            else
                this->rotation = 0;
        }
        else {
            int32 newRotation = this->rotation + (this->rotation < 0 ? 0x200 : 0);
            this->rotation    = newRotation - ((newRotation >= 0 ? newRotation >= 0 : (newRotation + 0x1FF)) & -0x200);

            if (this->rotation < 0x100)
                this->rotation += 4;
            else if (this->rotation > 0x100)
                this->rotation -= 4;
            else
                this->rotation = 0x100;
        }
    }

    this->collisionMode = globals->gravityDir;
    this->pushing       = 0;
}
void Player::Gravity_False()
{
    SET_CURRENT_STATE();

    if (this->camera)
        this->camera->disableYOffset = false;

    this->jumpAbilityState = 0;
}

// Input Helpers
void Player::DisableInputs(bool32 backupState)
{
    this->up        = false;
    this->down      = false;
    this->left      = false;
    this->right     = false;
    this->jumpPress = false;
    this->jumpHold  = false;

    if (backupState) {
        if (!this->stateInput.Matches(&Player::Input_NULL))
            this->stateInput.Copy(&this->storedStateInput);

        this->stateInput.Set(&Player::Input_NULL);
    }
}

bool32 Player::CheckP2KeyPress()
{
    if (globals->gameMode == MODE_ENCORE)
        return false;

    if (this->controllerID > Input::CONT_P4 || sVars->disableP2KeyCheck)
        return false;

    ControllerState *controller = &controllerInfo[this->controllerID];

    return controller->keyUp.down || controller->keyDown.down || controller->keyLeft.down || controller->keyRight.down || controller->keyA.down
           || controller->keyB.down || controller->keyC.down || controller->keyX.down;
}

void Player::HandleSidekickRespawn()
{
    Player *leader = GameObject::Get<Player>(SLOT_PLAYER1);

    if (leader->classID == sVars->classID && (leader->drawGroup != 2 || this->drawGroup != 2)) {
        int32 rx = abs((this->position.x >> 0x10) - screenInfo->position.x - screenInfo->center.x);
        int32 ry = abs((this->position.y >> 0x10) - screenInfo->position.y - screenInfo->center.y);

        if (rx >= screenInfo->center.x + 96 || ry >= screenInfo->center.y + 96)
            ++sVars->respawnTimer;
        else
            sVars->respawnTimer = 0;

        if (sVars->respawnTimer >= 240) {
            sVars->respawnTimer = 0;
            this->state.Set(&Player::State_HoldRespawn);
            this->forceHoldRespawn = true;
            this->position.x       = -0x400000;
            this->position.y       = -0x400000;
            this->abilityValues[0] = 0;
            this->drawFX &= ~FX_SCALE;
            this->nextAirState.Set(nullptr);
            this->nextGroundState.Set(nullptr);
            this->stateInput.Set(&Player::GetDelayedInput);
            this->scale.x        = 0x200;
            this->scale.y        = 0x200;
            this->velocity.x     = 0;
            this->velocity.y     = 0;
            this->groundVel      = 0;
            this->tileCollisions = TILECOLLISION_NONE;
            this->interaction    = false;
            this->blinkTimer     = 0;
            this->visible        = true;
            this->drownTimer     = 0;
        }
    }
}

void Player::GetDelayedInput()
{
    SET_CURRENT_STATE();

    Player *leader = GameObject::Get<Player>(SLOT_PLAYER1);

    sVars->upState <<= 1;
    sVars->upState |= leader->up;
    sVars->upState &= 0xFFFF;

    sVars->downState <<= 1;
    sVars->downState |= leader->down;
    sVars->downState &= 0xFFFF;

    sVars->leftState <<= 1;
    sVars->leftState |= leader->left;
    sVars->leftState &= 0xFFFF;

    sVars->rightState <<= 1;
    sVars->rightState |= leader->right;
    sVars->rightState &= 0xFFFF;

    sVars->jumpPressState <<= 1;
    sVars->jumpPressState |= leader->jumpPress;
    sVars->jumpPressState &= 0xFFFF;

    sVars->jumpHoldState <<= 1;
    sVars->jumpHoldState |= leader->jumpHold;
    sVars->jumpHoldState &= 0xFFFF;

    if (leader->state.Matches(&Player::State_FlyCarried)) {
        sVars->downState <<= 15;
        sVars->leftState <<= 15;
        sVars->rightState <<= 15;
        sVars->jumpPressState <<= 15;
        sVars->jumpHoldState <<= 15;
    }

    if (!this->stateInput.Matches(&Player::Input_AI_WaitForP1)) {
        this->up        = sVars->upState >> 15;
        this->down      = sVars->downState >> 15;
        this->left      = sVars->leftState >> 15;
        this->right     = sVars->rightState >> 15;
        this->jumpPress = sVars->jumpPressState >> 15;
        this->jumpHold  = sVars->jumpHoldState >> 15;
    }

    if (leader->CheckValidState()) {
        sVars->leaderPositionBuffer[sVars->lastLeaderPosID].x = leader->position.x;
        sVars->leaderPositionBuffer[sVars->lastLeaderPosID].y = leader->position.y;
        sVars->lastLeaderPosID                                = (sVars->lastLeaderPosID + 1) & 0xF;
        sVars->nextLeaderPosID                                = (sVars->nextLeaderPosID + 1) & 0xF;
        if (!leader->onGround && leader->groundedStore) {
            sVars->targetLeaderPosition.x = leader->position.x;
            sVars->targetLeaderPosition.y = leader->position.y;
        }
        else {
            sVars->targetLeaderPosition.x = sVars->leaderPositionBuffer[sVars->nextLeaderPosID].x;
            sVars->targetLeaderPosition.y = sVars->leaderPositionBuffer[sVars->nextLeaderPosID].y;
        }
    }
    else {
        int32 pos = sVars->lastLeaderPosID - 1;
        if (pos < 0)
            pos += 16;

        sVars->targetLeaderPosition.x = sVars->leaderPositionBuffer[pos].x;
        sVars->targetLeaderPosition.y = sVars->leaderPositionBuffer[pos].y;
    }
}

// Input States
void Player::Input_Gamepad()
{
    SET_CURRENT_STATE();

    if (this->controllerID <= Input::CONT_P4) {
        ControllerState *controller = &controllerInfo[this->controllerID];
        AnalogState *stick          = &analogStickInfoL[this->controllerID];

        this->up    = controller->keyUp.down;
        this->down  = controller->keyDown.down;
        this->left  = controller->keyLeft.down;
        this->right = controller->keyRight.down;

        this->up |= stick->keyUp.down;
        this->down |= stick->keyDown.down;
        this->left |= stick->keyLeft.down;
        this->right |= stick->keyRight.down;

        if (this->left && this->right) {
            this->left  = false;
            this->right = false;
        }
        this->jumpPress = controller->keyA.press || controller->keyB.press || controller->keyC.press || controller->keyX.press;
        this->jumpHold  = controller->keyA.down || controller->keyB.down || controller->keyC.down || controller->keyX.down;

        if (globals->gameMode == MODE_ENCORE && controller->keyY.press)
            sVars->sfxSwapFail.Play();

        Input_NULL();

        if (controller->keyStart.press || unknownInfo->pausePress) {
            if (sceneInfo->state == ENGINESTATE_REGULAR) {
                PauseMenu *pauseMenu = GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);
                if (!pauseMenu->classID) {
                    GameObject::Reset(SLOT_PAUSEMENU, PauseMenu::sVars->classID, nullptr);
                    pauseMenu->triggerPlayer = (uint8)this->Slot();
                    if (globals->gameMode == MODE_COMPETITION)
                        pauseMenu->disableRestart = true;
                }
            }
        }
    }
}

void Player::Input_GamepadAssist()
{
    SET_CURRENT_STATE();

    if (this->controllerID <= Input::CONT_P4) {
        if (Input::IsInputSlotAssigned(this->controllerID)) {
            ControllerState *controller = &controllerInfo[this->controllerID];
            AnalogState *stick          = &analogStickInfoL[this->controllerID];

            this->up    = controller->keyUp.down;
            this->down  = controller->keyDown.down;
            this->left  = controller->keyLeft.down;
            this->right = controller->keyRight.down;

            this->up |= stick->keyUp.down;
            this->down |= stick->keyDown.down;
            this->left |= stick->keyLeft.down;
            this->right |= stick->keyRight.down;

            if (this->left && this->right) {
                this->left  = false;
                this->right = false;
            }

            this->jumpPress = controller->keyA.press || controller->keyB.press || controller->keyC.press || controller->keyX.press;
            this->jumpHold  = controller->keyA.down || controller->keyB.down || controller->keyC.down || controller->keyX.down;

            if (this->right || this->up || this->down || this->left) {
                sVars->aiInputSwapTimer = 0;
            }
            else if (++sVars->aiInputSwapTimer >= 600) {
                this->stateInput.Set(&Player::Input_AI_Follow);
                Input::AssignInputSlotToDevice(this->controllerID, Input::INPUT_AUTOASSIGN);
            }
        }
        else {
            this->stateInput.Set(&Player::Input_AI_Follow);
            Input::AssignInputSlotToDevice(this->controllerID, Input::INPUT_AUTOASSIGN);
        }
    }
}

void Player::Input_NULL()
{
    SET_CURRENT_STATE();

    if (this->controllerID == Input::CONT_P1 && sceneInfo->debugMode) {
        if (!this->state.Matches(&Player::State_Transform) && controllerInfo[Input::CONT_P1].keyX.press && globals->gameMode != MODE_TIMEATTACK) {
            this->classID    = DebugMode::sVars->classID;
            this->velocity.x = 0;
            this->velocity.y = 0;
            this->groundVel  = 0;
            this->drawGroup  = Zone::sVars->playerDrawGroup[1];
            this->animator.SetAnimation(this->aniFrames, ANI_FALL, true, 0);
            Stage::SetEngineState(ENGINESTATE_REGULAR);
            this->jumpPress = false;
            this->jumpHold  = false;

            if (!this->stateInput.Matches(&Player::Input_AI_WaitForP1) && !this->stateInput.Matches(&Player::Input_AI_SpindashPt1)
                && !this->stateInput.Matches(&Player::Input_AI_SpindashPt2) && !this->stateInput.Matches(&Player::Input_AI_Follow)) {
                this->stateInput = this->storedStateInput;
            }
            else {
                this->timer = -15;
                this->stateInput.Set(&Player::Input_AI_WaitForP1);
            }
            this->visible                 = true;
            this->active                  = ACTIVE_NORMAL;
            DebugMode::sVars->debugActive = true;
        }
        // TEMP: TODO REMOVE
        else if (controllerInfo[1].keySelect.press) {
            this->characterID <<= 1;
            if (this->characterID > ID_KNUCKLES)
                this->characterID = ID_SONIC;

            ChangeCharacter(this->characterID);
        }
    }

    if (!this->playerID)
        sVars->finishedInput = true;
}

void Player::Input_AI_WaitForP1()
{
    SET_CURRENT_STATE();

    GetDelayedInput();

    Player *leader = GameObject::Get<Player>(SLOT_PLAYER1);
    if ((abs(leader->position.x - this->position.x) >> 16) >= 48) {
        this->stateInput.Set(&Player::Input_AI_Follow);
        this->timer          = 0;
        this->outtaHereTimer = 0;
    }

    if (leader->up || leader->down || leader->left || leader->right || leader->jumpPress || leader->jumpHold) {
        if (this->timer <= 0) {
            if (this->timer)
                this->timer = -this->timer;
            else
                this->timer = 15;
        }
    }

    this->timer--;

    if (!this->timer)
        this->stateInput.Set(&Player::Input_AI_Follow);

    if (CheckP2KeyPress())
        this->stateInput.Set(&Player::Input_GamepadAssist);
}

void Player::Input_AI_Follow()
{
    SET_CURRENT_STATE();

    Player *leader = GameObject::Get<Player>(SLOT_PLAYER1);
    GetDelayedInput();

    if (this->state.Matches(&Player::State_TailsFlight) && leader->state.Matches(&Player::State_FlyCarried)) {
        this->up        = leader->up;
        this->down      = leader->down;
        this->left      = leader->left;
        this->right     = leader->right;
        this->jumpHold  = leader->jumpHold;
        this->jumpPress = leader->jumpPress;
    }
    else if (leader->classID == sVars->classID && !leader->state.Matches(&Player::State_FlyCarried)) {
        if (((this->angle + 16) & 0xE0) == (this->angle + (globals->gravityDir << 6))) {
            if (this->left && this->position.x < leader->position.x - 0x80000 && this->velocity.x <= 0)
                this->left = false;
            if (this->right && this->position.x > leader->position.x + 0x80000 && this->velocity.x >= 0)
                this->right = false;
        }

        int32 leaderPos = sVars->targetLeaderPosition.x;
        if (leader->onGround || leader->groundedStore) {
            if (abs(leader->groundVel) < 0x20000) {

                switch (globals->gravityDir) {
                    default: break;

                    case CMODE_FLOOR:
                        if (!(leader->direction & FLIP_X))
                            leaderPos -= 0x200000;
                        else
                            leaderPos += 0x200000;
                        break;

                    case CMODE_ROOF:
                        if (!(leader->direction & FLIP_X))
                            leaderPos += 0x200000;
                        else
                            leaderPos -= 0x200000;
                        break;
                }
            }
        }

        int32 distance = leaderPos - this->position.x;
        if (distance) {
            if (distance >= 0) {
                if (distance >= 0x300000) {
                    this->left  = false;
                    this->right = true;
                }

                if (this->groundVel && !(this->direction & FLIP_X))
                    this->position.x += (0xC0 * Math::Cos256(this->angle));
            }
            else {
                if (distance <= -0x300000) {
                    this->right = false;
                    this->left  = true;
                }

                if (this->groundVel && (this->direction & FLIP_X))
                    this->position.x -= (0xC0 * Math::Cos256(this->angle));
            }
        }

        uint8 autoJump = 0;
        if (this->animator.animationID == ANI_PUSH) {
            if (leader->direction == this->direction && leader->animator.animationID == ANI_PUSH)
                sVars->autoJumpTimer = 0;
            else
                autoJump = ++sVars->autoJumpTimer < 30 ? 1 : 0;
        }
        else {
            if (this->position.y - sVars->targetLeaderPosition.y <= 0x200000) {
                sVars->autoJumpTimer = 0;
                if (this->controlLock > 0 && abs(this->groundVel) < 0x8000)
                    this->stateInput.Set(&Player::Input_AI_SpindashPt1);
                autoJump = 2;
            }
            else {
                ++sVars->autoJumpTimer;
                autoJump = sVars->autoJumpTimer < 64 ? 1 : 0;
            }
        }

        if (autoJump == 1) {
            this->jumpHold = true;
        }
        else if (autoJump == 0) {
            if (this->onGround) {
                if (!this->jumpHold)
                    this->jumpPress = true;

                this->jumpHold = true;
            }

            this->applyJumpCap   = false;
            sVars->autoJumpTimer = 0;
        }

        if (this->controlLock > 0 && abs(this->groundVel) < 0x8000)
            this->stateInput.Set(&Player::Input_AI_SpindashPt1);
    }

    if (CheckP2KeyPress())
        this->stateInput.Set(&Player::Input_GamepadAssist);
}

void Player::Input_AI_SpindashPt1()
{
    SET_CURRENT_STATE();

    this->up        = false;
    this->down      = false;
    this->left      = false;
    this->right     = false;
    this->jumpPress = false;
    this->jumpHold  = false;

    if (!this->controlLock && this->onGround && this->groundVel < 0x4000) {
        this->groundVel = 0;
        this->stateInput.Matches(&Player::Input_AI_SpindashPt2);

        sVars->autoJumpTimer = 1;
        if (this->animator.animationID != ANI_SPINDASH) {
            this->down = true;
            if (this->position.x >= sVars->targetLeaderPosition.x)
                this->direction |= FLIP_X;
            else
                this->direction &= ~FLIP_X;
        }
    }

    if (CheckP2KeyPress())
        this->stateInput.Set(&Player::Input_GamepadAssist);
}

void Player::Input_AI_SpindashPt2()
{
    SET_CURRENT_STATE();

    if (this->collisionMode) {
        this->stateInput.Set(&Player::Input_AI_Follow);
    }
    else {
        if (sVars->autoJumpTimer >= 64) {
            sVars->autoJumpTimer = 0;
            this->down           = false;
            this->jumpPress      = false;
            this->stateInput.Set(&Player::Input_AI_Follow);
        }
        else {
            this->down      = true;
            this->jumpPress = !(sVars->autoJumpTimer & 0xF);
            ++sVars->autoJumpTimer;
        }

        if (CheckP2KeyPress())
            this->stateInput.Set(&Player::Input_GamepadAssist);
    }
}

// Collision
RSDK::Hitbox *Player::GetHitbox()
{
    Hitbox *playerHitbox = this->animator.GetHitbox(0);
    return playerHitbox ? playerHitbox : &Player::fallbackHitbox;
}
RSDK::Hitbox *Player::GetAltHitbox()
{
    Hitbox *playerHitbox = this->animator.GetHitbox(1);
    return playerHitbox ? playerHitbox : &Player::fallbackHitbox;
}
bool32 Player::CheckCollisionTouch(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox)
{
    if (this->isGhost)
        return false;

    Hitbox *playerHitbox = this->outerBox ? this->outerBox : this->GetHitbox();

    return entity->CheckCollisionTouchBox(entityHitbox, this, playerHitbox);
}
bool32 Player::CheckCollisionTouchDefault(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox)
{
    if (this->isGhost)
        return false;

    return entity->CheckCollisionTouchBox(entityHitbox, this, &Player::fallbackHitbox);
}

void Player::BoxCollisionResponce(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox, uint32 side)
{
    switch (side) {
        default:
        case C_NONE: break;

        case C_TOP: {
            this->controlLock   = 0;
            this->collisionMode = globals->gravityDir;

            int32 colPos[2];
            if (entity->direction & FLIP_X) {
                colPos[0] = entity->position.x - (entityHitbox->right << 16);
                colPos[1] = entity->position.x - (entityHitbox->left << 16);
            }
            else {
                colPos[0] = entity->position.x + (entityHitbox->left << 16);
                colPos[1] = entity->position.x + (entityHitbox->right << 16);
            }

            int32 sensorX1 = this->position.x + this->sensorX[0];
            int32 sensorX3 = this->position.x + this->sensorX[2];
            int32 sensorX2 = this->position.x + this->sensorX[1];
            int32 sensorX4 = this->position.x + this->sensorX[3];
            int32 sensorX5 = this->position.x + this->sensorX[4];

            if (sensorX1 >= colPos[0] && sensorX1 <= colPos[1])
                this->flailing |= 1;
            if (sensorX2 >= colPos[0] && sensorX2 <= colPos[1])
                this->flailing |= 2;
            if (sensorX3 >= colPos[0] && sensorX3 <= colPos[1])
                this->flailing |= 4;
            if (sensorX4 >= colPos[0] && sensorX4 <= colPos[1])
                this->flailing |= 8;
            if (sensorX5 >= colPos[0] && sensorX5 <= colPos[1])
                this->flailing |= 0x10;
            break;
        }

        case C_LEFT: this->controlLock = 0; break;

        case C_RIGHT: this->controlLock = 0; break;

        case C_BOTTOM: break;
    }
}

uint8 Player::CheckCollisionBox(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox)
{
    if (this->isGhost)
        return C_NONE;

    Hitbox *playerHitbox = this->outerBox ? this->outerBox : this->GetHitbox();

    switch (entity->CheckCollisionBox(entityHitbox, this, playerHitbox, true)) {
        default:
        case C_NONE: return C_NONE;

        case C_TOP:
            if (globals->gravityDir == CMODE_FLOOR)
                BoxCollisionResponce(entity, entityHitbox, C_TOP);

            if (entity->velocity.y <= 0)
                this->collisionFlagV |= 1;
            return C_TOP;

        case C_LEFT:
            if (globals->gravityDir == CMODE_LWALL || globals->gravityDir == CMODE_RWALL) {
                BoxCollisionResponce(entity, entityHitbox, C_LEFT);
            }
            else {
                this->controlLock = 0;
                if (this->left && this->onGround) {
                    // Bug Details:
                    // if you spindash while touching the left side of an object that uses this func to collide
                    // and if you hold left & are on ground you'll move left
                    // thats the -0x8000 vel being applied every frame via ProcessObjectMovement, since spindash state doesn't expect you to have any
                    // x velocity unless it says so, so its ignored due to the way collision works, this only happens on the left, as standing idle
                    // against a solid object on the right doesn't count as colliding, while standing on the left does Fix (idk why you'd want to):
                    // just place a check to make sure player->state isn't Player_State_Spindash here (example fix in the line below, only the
                    // velocity needs to be checked for to fix the bug) if (player->state != Player_State_Spindash)
                    this->groundVel = -0x8000;
                    this->position.x &= 0xFFFF0000;
                }
            }
            return C_LEFT;

        case C_RIGHT:
            if (globals->gravityDir == CMODE_LWALL || globals->gravityDir == CMODE_RWALL) {
                BoxCollisionResponce(entity, entityHitbox, C_RIGHT);
            }
            else {
                this->controlLock = 0;
                if (this->right && this->onGround)
                    this->groundVel = 0x8000;
            }

            return C_RIGHT;

        case C_BOTTOM:
            if (globals->gravityDir == CMODE_ROOF)
                BoxCollisionResponce(entity, entityHitbox, C_BOTTOM);
            return C_BOTTOM;
    }
}
bool32 Player::CheckCollisionPlatform(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox)
{
    if (this->isGhost)
        return false;

    Hitbox *playerHitbox = this->outerBox ? this->outerBox : this->GetHitbox();

    if (entity->CheckCollisionPlatform(entityHitbox, this, playerHitbox, true)) {
        this->controlLock   = 0;
        this->collisionMode = globals->gravityDir;

        int32 colPos[2];
        if (entity->direction & FLIP_X) {
            colPos[0] = entity->position.x - (entityHitbox->right << 16);
            colPos[1] = entity->position.x - (entityHitbox->left << 16);
        }
        else {
            colPos[0] = entity->position.x + (entityHitbox->left << 16);
            colPos[1] = entity->position.x + (entityHitbox->right << 16);
        }

        int32 sensorX1 = this->position.x + this->sensorX[0];
        int32 sensorX3 = this->position.x + this->sensorX[2];
        int32 sensorX2 = this->position.x + this->sensorX[1];
        int32 sensorX4 = this->position.x + this->sensorX[3];
        int32 sensorX5 = this->position.x + this->sensorX[4];

        if (sensorX1 >= colPos[0] && sensorX1 <= colPos[1])
            this->flailing |= 0x01;
        if (sensorX2 >= colPos[0] && sensorX2 <= colPos[1])
            this->flailing |= 0x02;
        if (sensorX3 >= colPos[0] && sensorX3 <= colPos[1])
            this->flailing |= 0x04;
        if (sensorX4 >= colPos[0] && sensorX4 <= colPos[1])
            this->flailing |= 0x08;
        if (sensorX5 >= colPos[0] && sensorX5 <= colPos[1])
            this->flailing |= 0x10;

        switch (globals->gravityDir) {
            default: break;

            case CMODE_FLOOR:
                if (entity->velocity.y <= 0)
                    this->collisionFlagV |= 1;
                break;

            case CMODE_ROOF:
                if (entity->velocity.y >= 0)
                    this->collisionFlagV |= 2;
                break;
        }

        return true;
    }

    return false;
}

// Damage
void Player::Hit(bool32 forceKill)
{
    uint8 hurtType = Player::HurtNone;
    if (this->sidekick) {
        hurtType = Player::HurtShield;
    }
    else {
        Shield *shield = GameObject::Get<Shield>(sVars->maxPlayerCount + this->Slot());
        if (shield->classID == Shield::sVars->classID) {
            this->shield = Shield::None;
            shield->Destroy();
            hurtType = Player::HurtShield;
        }
        else {
            hurtType = (this->rings <= 0) + Player::HurtRingLoss;
        }

        if (!sVars->gotHit[this->playerID])
            sVars->gotHit[this->playerID] = true;
    }

    this->nextAirState.Set(nullptr);
    this->nextGroundState.Set(nullptr);
    if (globals->coolBonus[this->playerID] > 0)
        globals->coolBonus[this->playerID] -= 1000;

    if (forceKill)
        hurtType = Player::HurtDie;

    switch (hurtType) {
        default: break;

        case Player::HurtShield: // Hurt, no rings (shield/P2/etc)
            this->state.Set(&Player::State_Hurt);
            this->animator.SetAnimation(this->aniFrames, ANI_HURT, false, 0);
            if (globals->gravityDir == CMODE_ROOF)
                this->velocity.y = 0x40000;
            else if (globals->gravityDir == CMODE_FLOOR)
                this->velocity.y = -0x40000;
            this->onGround       = false;
            this->tileCollisions = globals->tileCollisionMode;
            this->blinkTimer     = 120;
            if (this->underwater) {
                this->velocity.x >>= 1;
                this->velocity.y = -0x20000;
            }
            sVars->sfxHurt.Play();
            this->stateGravity.Set(&Player::Gravity_NULL);
            break;

        case Player::HurtRingLoss: // Hurt, lost rings
            this->state.Set(&Player::State_Hurt);
            this->animator.SetAnimation(this->aniFrames, ANI_HURT, false, 0);
            if (globals->gravityDir == CMODE_ROOF)
                this->velocity.y = 0x40000;
            else if (globals->gravityDir == CMODE_FLOOR)
                this->velocity.y = -0x40000;
            this->onGround       = false;
            this->tileCollisions = globals->tileCollisionMode;
            this->blinkTimer     = 120;
            if (this->underwater) {
                this->velocity.x >>= 1;
                this->velocity.y = -0x20000;
            }

            if (this->hyperRing)
                Ring::LoseHyperRings(&this->position, this->rings, this->collisionPlane);
            else
                Ring::LoseRings(&this->position, this->rings, this->collisionPlane, this->drawGroup);

            this->hyperRing     = false;
            this->rings         = 0;
            this->ringExtraLife = 100;
            sVars->sfxLoseRings.Play();
            this->stateGravity.Set(&Player::Gravity_NULL);
            break;

        case Player::HurtDie: // Hurt, dies.
            this->deathType = Player::DeathDie_Sfx;
            this->stateGravity.Set(&Player::Gravity_NULL);
            break;
    }
}
bool32 Player::Hurt(RSDK::GameObject::Entity *entity, bool32 forceKill)
{
    if (this->state.Matches(&Player::State_Drown) || this->state.Matches(&Player::State_Hurt) || this->state.Matches(&Player::State_Death)
        || this->invincibleTimer || this->blinkTimer)
        return false;

    if (this->position.x > entity->position.x)
        this->velocity.x = 0x20000;
    else
        this->velocity.x = -0x20000;
    this->Hit(forceKill);
    return true;
}
bool32 Player::FireHurt(RSDK::GameObject::Entity *entity)
{
    if (this->shield == Shield::Fire)
        return false;

    return Hurt(entity);
}
bool32 Player::LightningHurt(RSDK::GameObject::Entity *entity)
{
    if (this->shield == Shield::Lightning)
        return false;

    return Hurt(entity);
}
bool32 Player::CheckAttacking(RSDK::GameObject::Entity *entity)
{
    int32 anim       = this->animator.animationID;
    bool32 attacking = this->invincibleTimer != 0 || anim == ANI_JUMP || anim == ANI_SPINDASH;
    switch (this->characterID) {
        case ID_SONIC: attacking |= anim == ANI_DROPDASH; break;

        case ID_TAILS:
            if (!attacking && entity) {
                attacking = anim == ANI_FLY || anim == ANI_FLY_TIRED || anim == ANI_FLY_LIFT || anim == ANI_FLY_LIFT_DOWN;
                switch (globals->gravityDir) {
                    default: break;

                    case CMODE_FLOOR:
                        if (this->position.y <= entity->position.y)
                            return false;
                        break;

                    case CMODE_ROOF:
                        if (this->position.y >= entity->position.y)
                            return false;
                        break;
                }
            }
            break;

        case ID_KNUCKLES: attacking |= anim == ANI_GLIDE || anim == ANI_GLIDE_SLIDE; break;
        default: break;
    }

    return attacking;
}

bool32 Player::CheckBadnikTouch(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox, bool32 enableHyperList)
{
    if (this->isGhost)
        return false;

    Hitbox *playerHitbox = this->GetHitbox();

    Shield *shield = GameObject::Get<Shield>(sVars->maxPlayerCount + this->playerID);
    if (shield->classID == Shield::sVars->classID && shield->state.Matches(&Shield::State_Insta)) {
        Hitbox tempHitbox = sVars->instaShieldHitbox;

        if (this->isChibi) {
            tempHitbox.left   = (playerHitbox->left << 1) - (playerHitbox->left >> 1);
            tempHitbox.top    = (playerHitbox->top << 1) - (playerHitbox->top >> 1);
            tempHitbox.right  = (playerHitbox->right << 1) - (playerHitbox->right >> 1);
            tempHitbox.bottom = (playerHitbox->bottom << 1) - (playerHitbox->bottom >> 1);
        }
        playerHitbox = &tempHitbox;
    }

    if (enableHyperList && (this->hyperAbilityState != Player::HyperStateNone || (Water::sVars && Water::sVars->isLightningFlashing))) {
        Vector2 range = { 0, 0 };

        if (entity->position.CheckOnScreen(&range)) {
            for (int32 i = 0; i < Zone::sVars->hyperListCount; ++i) {
                if (Zone::sVars->hyperList[i].classID == entity->classID) {
                    if (Zone::sVars->hyperList[i].hyperSlamTarget && this->hyperAbilityState == Player::HyperStateHyperSlam)
                        return true;
                    else if (Zone::sVars->hyperList[i].hyperDashTarget && this->hyperAbilityState == Player::HyperStateHyperDash)
                        return true;

                    if (Zone::sVars->hyperList[i].superFlickyTarget) {
                        uint16 slot = entity->Slot();

                        for (int32 i = 0; i < 0x80; ++i) {
                            if (Zone::sVars->flickyAttackList[i].slotID == slot) {
                                if (Zone::sVars->flickyAttackList[i].classID == entity->classID) {
                                    if (Zone::sVars->flickyAttackList[i].isTargeted) {
                                        Zone::sVars->flickyAttackList[i].hitbox   = *entityHitbox;
                                        Zone::sVars->flickyAttackList[i].position = entity->position;
                                        Zone::sVars->flickyAttackList[i].timer    = 8;
                                        Zone::sVars->flickyAttackList[i].slotID   = slot;
                                        Zone::sVars->flickyAttackList[i].classID  = entity->classID;
                                        break;
                                    }
                                }
                                else {
                                    Zone::sVars->flickyAttackList[i].slotID        = -1;
                                    Zone::sVars->flickyAttackList[i].classID       = TYPE_NONE;
                                    Zone::sVars->flickyAttackList[i].isTargeted    = 0;
                                    Zone::sVars->flickyAttackList[i].hitbox.left   = 0;
                                    Zone::sVars->flickyAttackList[i].hitbox.top    = 0;
                                    Zone::sVars->flickyAttackList[i].hitbox.right  = 0;
                                    Zone::sVars->flickyAttackList[i].hitbox.bottom = 0;
                                    Zone::sVars->flickyAttackList[i].position.x    = 0;
                                    Zone::sVars->flickyAttackList[i].position.y    = 0;
                                    Zone::sVars->flickyAttackList[i].timer         = -1;
                                }
                            }

                            if (Zone::sVars->flickyAttackList[i].slotID == -1) {
                                Zone::sVars->flickyAttackList[i].slotID     = slot;
                                Zone::sVars->flickyAttackList[i].classID    = entity->classID;
                                Zone::sVars->flickyAttackList[i].isTargeted = false;
                                Zone::sVars->flickyAttackList[i].hitbox     = *entityHitbox;
                                Zone::sVars->flickyAttackList[i].position   = entity->position;
                                Zone::sVars->flickyAttackList[i].timer      = 8;
                                break;
                            }
                        }

                        for (auto flicky : GameObject::GetEntities<SuperFlicky>(FOR_ACTIVE_ENTITIES)) {
                            if (flicky->state.Matches(&SuperFlicky::State_Active) && !flicky->attackDelay) {
                                if (flicky->attackListPos != -1) {
                                    if (Zone::sVars->flickyAttackList[flicky->attackListPos].isTargeted) {
                                        Entity *target = GameObject::Get(Zone::sVars->flickyAttackList[flicky->attackListPos].slotID);

                                        if (target->classID == Zone::sVars->flickyAttackList[flicky->attackListPos].classID) {
                                            if (flicky->CheckCollisionTouchBox(&SuperFlicky::sVars->hitbox, entity,
                                                                               &Zone::sVars->flickyAttackList[flicky->attackListPos].hitbox))
                                                return true;
                                        }
                                        else {
                                            Zone::sVars->flickyAttackList[flicky->attackListPos].slotID        = -1;
                                            Zone::sVars->flickyAttackList[flicky->attackListPos].classID       = TYPE_NONE;
                                            Zone::sVars->flickyAttackList[flicky->attackListPos].isTargeted    = false;
                                            Zone::sVars->flickyAttackList[flicky->attackListPos].hitbox.left   = 0;
                                            Zone::sVars->flickyAttackList[flicky->attackListPos].hitbox.top    = 0;
                                            Zone::sVars->flickyAttackList[flicky->attackListPos].hitbox.right  = 0;
                                            Zone::sVars->flickyAttackList[flicky->attackListPos].hitbox.bottom = 0;
                                            Zone::sVars->flickyAttackList[flicky->attackListPos].position.x    = 0;
                                            Zone::sVars->flickyAttackList[flicky->attackListPos].position.y    = 0;
                                            Zone::sVars->flickyAttackList[flicky->attackListPos].timer         = -1;
                                            flicky->attackDelay                                                = 120;
                                            flicky->attackListPos                                              = -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return entity->CheckCollisionTouchBox(entityHitbox, this, playerHitbox);
}
bool32 Player::CheckBadnikBreak(RSDK::GameObject::Entity *badnik, bool32 destroy)
{
    bool32 isAttacking   = false;
    bool32 isRolling     = this->characterID == ID_SONIC && this->state.Matches(&Player::State_Roll);
    bool32 enableRebound = false;
    // TODO: check type stuff

    if (!isAttacking) {
        isAttacking = CheckAttacking(badnik);

        if (isAttacking)
            enableRebound = true;
    }

    if (isAttacking) {
        Player *player = this;
        if (globals->gameMode != MODE_COMPETITION)
            player = GameObject::Get<Player>(SLOT_PLAYER1);

        BadnikHelpers::BadnikBreakUnseeded(badnik, false, true);

        if (enableRebound) {
            int32 yVel = 0;
            switch (globals->gravityDir) {
                default: break;

                case CMODE_FLOOR:
                    if (this->velocity.y <= 0)
                        yVel = this->velocity.y + 0x10000;
                    else if (this->position.y >= badnik->position.y)
                        yVel = this->velocity.y - 0x10000;
                    else
                        yVel = -(this->velocity.y + 2 * this->gravityStrength);
                    break;

                case CMODE_ROOF:
                    if (this->velocity.y >= 0)
                        yVel = this->velocity.y + 0x10000;
                    else if (this->position.y <= badnik->position.y)
                        yVel = this->velocity.y - 0x10000;
                    else
                        yVel = (this->velocity.y + 2 * this->gravityStrength);
                    break;
            }
            this->velocity.y = yVel;
        }

        ScoreBonus *scoreBonus       = GameObject::Create<ScoreBonus>(0, badnik->position.x, badnik->position.y);
        scoreBonus->drawGroup        = Zone::sVars->objectDrawGroup[1];
        scoreBonus->animator.frameID = player->scoreBonus;

        if (player->scoreBonus < 15) {
            int32 scoreBonuses[] = { 100, 200, 500, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 10000 };
            player->GiveScore(scoreBonuses[player->scoreBonus++]);
        }

        if (destroy) {
            badnik->Destroy();
            badnik->active = ACTIVE_DISABLED;
        }
        return true;
    }
    else {
        if (!this->uncurlTimer) {
            Hurt(badnik);
        }
    }

    return false;
}
bool32 Player::CheckBossHit(RSDK::GameObject::Entity *entity, bool32 enableHyperList)
{
    if (enableHyperList && this->hyperAbilityState != Player::HyperStateNone) {
        for (int32 i = 0; i < Zone::sVars->hyperListCount; ++i) {
            if (Zone::sVars->hyperList[i].classID == entity->classID) {
                if (Zone::sVars->hyperList[i].hyperSlamTarget && this->hyperAbilityState == Player::HyperStateHyperSlam)
                    return true;
                else if (Zone::sVars->hyperList[i].hyperDashTarget && this->hyperAbilityState == Player::HyperStateHyperDash)
                    return true;

                if (Zone::sVars->hyperList[i].superFlickyTarget) {
                    for (auto flicky : GameObject::GetEntities<SuperFlicky>(FOR_ACTIVE_ENTITIES)) {
                        if (flicky->state.Matches(&SuperFlicky::State_Active) && !flicky->attackDelay) {
                            if (flicky->attackListPos != -1) {
                                if (Zone::sVars->flickyAttackList[flicky->attackListPos].isTargeted) {
                                    Entity *target = GameObject::Get(Zone::sVars->flickyAttackList[flicky->attackListPos].slotID);

                                    if (target->classID == Zone::sVars->flickyAttackList[flicky->attackListPos].classID) {
                                        if (flicky->CheckCollisionTouchBox(&SuperFlicky::sVars->hitbox, entity,
                                                                           &Zone::sVars->flickyAttackList[flicky->attackListPos].hitbox))
                                            return true;
                                    }
                                    else {
                                        Zone::sVars->flickyAttackList[flicky->attackListPos].slotID        = -1;
                                        Zone::sVars->flickyAttackList[flicky->attackListPos].classID       = TYPE_NONE;
                                        Zone::sVars->flickyAttackList[flicky->attackListPos].isTargeted    = false;
                                        Zone::sVars->flickyAttackList[flicky->attackListPos].hitbox.left   = 0;
                                        Zone::sVars->flickyAttackList[flicky->attackListPos].hitbox.top    = 0;
                                        Zone::sVars->flickyAttackList[flicky->attackListPos].hitbox.right  = 0;
                                        Zone::sVars->flickyAttackList[flicky->attackListPos].hitbox.bottom = 0;
                                        Zone::sVars->flickyAttackList[flicky->attackListPos].position.x    = 0;
                                        Zone::sVars->flickyAttackList[flicky->attackListPos].position.y    = 0;
                                        Zone::sVars->flickyAttackList[flicky->attackListPos].timer         = -1;
                                        flicky->attackDelay                                                = 120;
                                        flicky->attackListPos                                              = -1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    if (CheckAttacking(entity)) {
        this->groundVel  = -this->groundVel;
        this->velocity.x = -this->velocity.x;
        this->velocity.y = -(this->velocity.y + 2 * this->gravityStrength);

        if (this->characterID == ID_KNUCKLES && this->animator.animationID == ANI_GLIDE) {
            this->animator.SetAnimation(this->aniFrames, ANI_GLIDE_DROP, false, 0);
            this->state.Set(&Player::State_KnuxGlideDrop);
        }

        return true;
    }
    else {
        Hurt(entity);
    }

    return false;
}
bool32 Player::ProjectileHurt(RSDK::GameObject::Entity *entity)
{
    Shield *shield = GameObject::Get<Shield>(sVars->maxPlayerCount + this->playerID);
    int32 anim     = this->animator.animationID;

    bool32 deflected = false;
    switch (this->characterID) {
        case ID_SONIC: break;

        case ID_TAILS:
            if (anim == ANI_FLY || anim == ANI_FLY_TIRED || anim == ANI_FLY_LIFT || anim == ANI_FLY_LIFT_DOWN) {
                switch (globals->gravityDir) {
                    default: break;

                    case CMODE_FLOOR:
                        if (this->position.y > entity->position.y)
                            deflected = true;
                        break;

                    case CMODE_ROOF:
                        if (this->position.y < entity->position.y)
                            deflected = true;
                        break;
                }
            }
            break;

        case ID_KNUCKLES: deflected = anim == ANI_GLIDE || anim == ANI_GLIDE_LAND || anim == ANI_GLIDE_SLIDE; break;
    }

    if (this->shield == Shield::Bubble || this->shield == Shield::Fire || this->shield == Shield::Lightning
        || shield->state.Matches(&Shield::State_Insta))
        deflected = true;

    if (deflected) {
        int32 angle         = Math::ATan2(this->position.x - entity->position.x, this->position.y - entity->position.y);
        entity->velocity.x  = -0x800 * Math::Cos256(angle);
        entity->velocity.y  = -0x800 * Math::Sin256(angle);
        entity->interaction = false;
        return true;
    }

    Hurt(entity);

    return false;
}

#if RETRO_INCLUDE_EDITOR
void Player::EditorDraw()
{
    this->animator.frameID = this->characterID & 7;
    this->animator.DrawSprite(nullptr, false);
}

void Player::EditorLoad() { sVars->sonicFrames.Load("Editor/PlayerIcons.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void Player::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Player);

    // clang-format off
    int32 sonicPhysicsTable[] = { 
        0x60000, 0x00C00, 0x01800, 0x00600, 0x08000, 0x00600, 0x68000, -0x40000, 
        0x30000, 0x00600, 0x00C00, 0x00300, 0x04000, 0x00300, 0x38000, -0x20000, 
        0xA0000, 0x03000, 0x06000, 0x01800, 0x10000, 0x00600, 0x80000, -0x40000, 
        0x50000, 0x01800, 0x03000, 0x00C00, 0x08000, 0x00300, 0x38000, -0x20000, 
        0xC0000, 0x01800, 0x03000, 0x00C00, 0x08000, 0x00600, 0x68000, -0x40000, 
        0x60000, 0x00C00, 0x01800, 0x00600, 0x04000, 0x00300, 0x38000, -0x20000, 
        0xC0000, 0x01800, 0x03000, 0x00C00, 0x08000, 0x00600, 0x80000, -0x40000, 
        0x60000, 0x00C00, 0x01800, 0x00600, 0x04000, 0x00300, 0x38000, -0x20000 
    };

    int32 tailsPhysicsTable[] = { 
        0x60000, 0x00C00, 0x01800, 0x00600, 0x08000, 0x00600, 0x68000, -0x40000, 
        0x30000, 0x00600, 0x00C00, 0x00300, 0x04000, 0x00300, 0x38000, -0x20000, 
        0xA0000, 0x03000, 0x06000, 0x01800, 0x10000, 0x00600, 0x80000, -0x40000, 
        0x50000, 0x01800, 0x03000, 0x00C00, 0x08000, 0x00300, 0x38000, -0x20000, 
        0xC0000, 0x01800, 0x03000, 0x00C00, 0x08000, 0x00600, 0x68000, -0x40000, 
        0x60000, 0x00C00, 0x01800, 0x00600, 0x04000, 0x00300, 0x38000, -0x20000, 
        0xC0000, 0x01800, 0x03000, 0x00C00, 0x08000, 0x00600, 0x80000, -0x40000, 
        0x60000, 0x00C00, 0x01800, 0x00600, 0x04000, 0x00300, 0x38000, -0x20000 
    };

    int32 knuxPhysicsTable[] = { 
        0x60000, 0x00C00, 0x01800, 0x00600, 0x08000, 0x00600, 0x60000, -0x40000, 
        0x30000, 0x00600, 0x00C00, 0x00300, 0x04000, 0x00300, 0x30000, -0x20000, 
        0xA0000, 0x03000, 0x06000, 0x01800, 0x10000, 0x00600, 0x60000, -0x40000, 
        0x50000, 0x01800, 0x03000, 0x00C00, 0x08000, 0x00300, 0x30000, -0x20000, 
        0xC0000, 0x01800, 0x03000, 0x00C00, 0x08000, 0x00600, 0x60000, -0x40000, 
        0x60000, 0x00C00, 0x01800, 0x00600, 0x04000, 0x00300, 0x30000, -0x20000, 
        0xC0000, 0x01800, 0x03000, 0x00C00, 0x08000, 0x00600, 0x60000, -0x40000, 
        0x60000, 0x00C00, 0x01800, 0x00600, 0x08000, 0x00300, 0x30000, -0x20000 
    };
    // clang-format on

    memcpy(sVars->sonicPhysicsTable, sonicPhysicsTable, sizeof(sonicPhysicsTable));
    memcpy(sVars->tailsPhysicsTable, tailsPhysicsTable, sizeof(tailsPhysicsTable));
    memcpy(sVars->knuxPhysicsTable, knuxPhysicsTable, sizeof(knuxPhysicsTable));

    // ---------------
    // SUPER SONIC
    // ---------------

    color superSonicPalette[] = { 0x11004F, 0x0F16AD, 0x1D2EE2, 0x225BF1, 0x4281F7, 0x48B9F7, 0xF0C001, 0xF0D028, 0xF0E040,
                                  0xF0E860, 0xF0E898, 0xF0E8D0, 0xF0D898, 0xF0E0B0, 0xF0E8C0, 0xF0F0D8, 0xF0F0F0, 0xF0F0F8 };

    memcpy(sVars->superSonicPalette, superSonicPalette, sizeof(superSonicPalette));

    // ---------------
    // SUPER TAILS
    // ---------------

    color superTailsPalette[] = { 0x560020, 0x8F001B, 0xC61800, 0xE24F05, 0xFD7300, 0xFFA001, 0xF03830, 0xF06848, 0xF09860,
                                  0xF0B868, 0xF0C870, 0xF0D870, 0xF03830, 0xF06848, 0xF09860, 0xF0B868, 0xF0C870, 0xF0D870 };

    memcpy(sVars->superTailsPalette, superTailsPalette, sizeof(superTailsPalette));

    // ---------------
    // SUPER KNUCKLES
    // ---------------

    color superKnuxPalette[] = { 0x400015, 0x7D0025, 0xD00840, 0xE82858, 0xFD4C21, 0xF5924D, 0xF05878, 0xF06090, 0xF080A0,
                                 0xF098B0, 0xF0B0C8, 0xB30626, 0xE12808, 0xF06090, 0xF080A0, 0xF098B0, 0xF0B0C8, 0xF0C0C8 };

    memcpy(sVars->superKnuxPalette, superKnuxPalette, sizeof(superKnuxPalette));

    float chargeSpeeds[13] = { 1.0f, 1.0614f, 1.1255f, 1.1926f, 1.263f, 1.337f, 1.415f, 1.4975f, 1.585f, 1.6781f, 1.7776f, 1.8845f, 2.0f };
    memcpy(sVars->spindashChargeSpeeds, chargeSpeeds, sizeof(chargeSpeeds));

    sVars->instaShieldHitbox.left   = -24;
    sVars->instaShieldHitbox.top    = -24;
    sVars->instaShieldHitbox.right  = 24;
    sVars->instaShieldHitbox.bottom = 24;

    sVars->shieldHitbox.left   = -24;
    sVars->shieldHitbox.top    = -24;
    sVars->shieldHitbox.right  = 24;
    sVars->shieldHitbox.bottom = 24;

    sVars->ringExtraLife = 100;
    sVars->savedLives    = 3;
    sVars->savedScore1UP = 50000;

    sVars->sonicFrames.Init();
    sVars->superFrames.Init();
    sVars->tailsFrames.Init();
    sVars->tailsTailsFrames.Init();
    sVars->knuxFrames.Init();

    sVars->sfxJump.Init();
    sVars->sfxRoll.Init();
    sVars->sfxCharge.Init();
    sVars->sfxRelease.Init();
    sVars->sfxPeelCharge.Init();
    sVars->sfxPeelRelease.Init();
    sVars->sfxDropdash.Init();
    sVars->sfxLoseRings.Init();
    sVars->sfxHurt.Init();
    sVars->sfxSkidding.Init();
    sVars->sfxGrab.Init();
    sVars->sfxFlying.Init();
    sVars->sfxTired.Init();
    sVars->sfxLand.Init();
    sVars->sfxSlide.Init();
    sVars->sfxTransform2.Init();
    sVars->sfxSwap.Init();
    sVars->sfxSwapFail.Init();
    sVars->sfxEarthquake.Init();
}
#endif

void Player::Serialize() { RSDK_EDITABLE_VAR(Player, VAR_INT32, characterID); }

} // namespace GameLogic
