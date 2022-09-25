// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Player Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Player.hpp"
#include "HP_Setup.hpp"
#include "HP_Halfpipe.hpp"
#include "HP_Collectable.hpp"
#include "Global/Zone.hpp"
#include "Global/PauseMenu.hpp"
#include "Global/Music.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Player);

void HP_Player::Update()
{
    this->stateInput.Run(this);

    if (this->invincibilityTimer > 0) {
        this->invincibilityTimer--;
        this->isVisible = !(this->invincibilityTimer & 4);
    }

    this->state.Run(this);

    if (globals->gameMode == MODE_COMPETITION) {
        int32 leadingPlayer = (this->Slot() - SLOT_HP_PLAYER1) ^ this->playerID ^ 1;
        if (leadingPlayer == 0) {
            if (this->zpos < 0x1000)
                this->zpos += 64;
        }
        else {
            if (this->zpos > 0x400)
                this->zpos -= 64;
        }
    }

    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);
    this->localPos.x      = FROM_FIXED(this->position.x) >> 1;
    this->localPos.y      = FROM_FIXED(this->position.y) >> 1;
    this->localPos.z      = (this->zpos << 8) + halfpipe->playerZPos + 0xC0000;

    this->animator.Process();

    this->Draw_Normal();

    if (this->characterID == ID_TAILS) {
        if (this->animator.animationID == 0)
            this->tailAnimator.SetAnimation(this->tailFrames, 0, false, 0);
        else
            this->tailAnimator.SetAnimation(nullptr, 0, false, 0);

        this->tailAnimator.Process();
    }
}
void HP_Player::LateUpdate() {}
void HP_Player::StaticUpdate() {}
void HP_Player::Draw() {}

void HP_Player::Create(void *data)
{
    if (sceneInfo->inEditor) {
        this->animator.SetAnimation(sVars->sonicFrames, HP_Player::ANI_RUN, true, 0);
        this->characterID = ID_SONIC;
    }
    else {
        this->playerID = this->Slot() - SLOT_HP_PLAYER1;

        // Handle character specific stuff
        switch (this->characterID) {
            default:
            case ID_SONIC:
                this->aniFrames = sVars->sonicFrames;
                this->tailFrames.Init();

                this->xBoundsR = 88 << 8;
                break;

            case ID_TAILS:
                this->aniFrames  = sVars->tailsFrames;
                this->tailFrames = sVars->tailSpriteFrames;

                this->xBoundsR = 90 << 8;
                break;

            case ID_KNUCKLES:
                this->aniFrames = sVars->knuxFrames;
                this->tailFrames.Init();

                this->xBoundsR = 88 << 8;
                break;
        }

        this->active         = ACTIVE_NORMAL;
        this->tileCollisions = globals->tileCollisionMode;
        this->visible        = true;
        this->isVisible      = true;
        this->drawGroup      = Zone::sVars->playerDrawGroup[0];
        this->scale.x        = 0x200;
        this->scale.y        = 0x200;
        this->controllerID   = Input::CONT_P1 + this->playerID;
        this->state.Set(&HP_Player::State_Ground);
        this->onGround  = true;
        this->xBoundsL  = -this->xBoundsR;
        this->groundPos = (this->xBoundsR * this->xBoundsR) - 0x10000;

        // Handle Input Setup
        if (!this->playerID || globals->gameMode == MODE_COMPETITION) {
            this->zpos = 0x1000;
            this->stateInput.Set(&HP_Player::Input_Gamepad);
        }
        else {
            this->zpos = 0x400;
            Input::AssignInputSlotToDevice(this->controllerID, Input::INPUT_AUTOASSIGN);
            this->stateInput.Set(&HP_Player::Input_AI_Follow);
            this->sidekick = true;
        }

        analogStickInfoL[this->controllerID].deadzone = 0.3f;

        this->drawFX = FX_ROTATE;
        this->animator.SetAnimation(this->aniFrames, HP_Player::ANI_RUN, true, 0);
        this->shadowAnimator.SetAnimation(HP_Halfpipe::sVars->shadowFrames, 0, true, 0);
    }
}

void HP_Player::StageLoad()
{
    if (globals->gameMode == MODE_COMPETITION)
        HP_Player::LoadSpritesVS();
    else
        HP_Player::LoadSprites();

    sVars->upState        = false;
    sVars->downState      = false;
    sVars->leftState      = false;
    sVars->rightState     = false;
    sVars->jumpPressState = false;
    sVars->jumpHoldState  = false;

    sVars->sfxJump.Get("Global/Jump.wav");
    sVars->sfxLoseRings.Get("Global/LoseRings.wav");
}

void HP_Player::LoadSprites()
{
    for (auto spawn : GameObject::GetEntities<HP_Player>(FOR_ALL_ENTITIES)) {
        int32 playerID = GET_CHARACTER_ID(1);

        if (spawn->characterID & playerID) {
            HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);

            spawn->characterID = GET_CHARACTER_ID(1);
            spawn->LoadPlayerSprites();
            spawn->Copy(player1, true);
        }
        else {
            spawn->Destroy();
        }
    }

    if (GET_CHARACTER_ID(2) > 0) {
        HP_Player *leader   = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
        HP_Player *sidekick = GameObject::Get<HP_Player>(SLOT_HP_PLAYER2);

        sidekick->characterID = GET_CHARACTER_ID(2);
        sidekick->LoadPlayerSprites();
        sidekick->classID     = HP_Player::sVars->classID;
    }
}
void HP_Player::LoadSpritesVS()
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

void HP_Player::LoadPlayerSprites()
{
    switch (this->characterID) {
        default:
        case ID_SONIC:
            sVars->sonicFrames.Load("Special/Sonic.bin", SCOPE_STAGE);
            break;

        case ID_TAILS:
            sVars->tailsFrames.Load("Special/Tails.bin", SCOPE_STAGE);
            sVars->tailSpriteFrames.Load("Special/TailSprite.bin", SCOPE_STAGE);
            break;

        case ID_KNUCKLES:
            sVars->knuxFrames.Load("Special/Knux.bin", SCOPE_STAGE);
            break;
    }
}

bool32 HP_Player::CheckP2KeyPress()
{
    if (globals->gameMode == MODE_ENCORE)
        return false;

    if (this->controllerID > Input::CONT_P4 || sVars->disableP2KeyCheck)
        return false;

    ControllerState *controller = &controllerInfo[this->controllerID];

    return controller->keyUp.down || controller->keyDown.down || controller->keyLeft.down || controller->keyRight.down || controller->keyA.down
           || controller->keyB.down || controller->keyC.down || controller->keyX.down;
}

void HP_Player::GetDelayedInput()
{
    SET_CURRENT_STATE();

    HP_Player *leader = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);

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

    this->up        = sVars->upState >> 15;
    this->down      = sVars->downState >> 15;
    this->left      = sVars->leftState >> 15;
    this->right     = sVars->rightState >> 15;
    this->jumpPress = sVars->jumpPressState >> 15;
    this->jumpHold  = sVars->jumpHoldState >> 15;
}

void HP_Player::Input_Gamepad()
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

        if (controller->keySelect.press) {
            HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
            HP_Player *player2 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER2);

            int32 totalRings = player1->rings + (GET_CHARACTER_ID(2) ? player2->rings : 0);
            player1->rings += HP_Setup::sVars->ringCounts[HP_Setup::sVars->checkpointID] - totalRings;
        }
    }
}
void HP_Player::Input_GamepadAssist()
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
                this->stateInput.Set(&HP_Player::Input_AI_Follow);
                Input::AssignInputSlotToDevice(this->controllerID, Input::INPUT_AUTOASSIGN);
            }
        }
        else {
            this->stateInput.Set(&HP_Player::Input_AI_Follow);
            Input::AssignInputSlotToDevice(this->controllerID, Input::INPUT_AUTOASSIGN);
        }
    }
}
void HP_Player::Input_AI_Follow()
{
    SET_CURRENT_STATE();

    HP_Player *leader = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
    GetDelayedInput();

    if (CheckP2KeyPress())
        this->stateInput.Set(&HP_Player::Input_GamepadAssist);
}
void HP_Player::State_Ground()
{
    SET_CURRENT_STATE();

    if (this->right) {
        this->groundVel -= 48;
        if (this->groundVel < -0x600) {
            this->groundVel = -0x600;

            this->automoveActive = false;
            this->timer          = 0;
        }
    }
    else if (this->left) {
        this->groundVel += 48;
        if (this->groundVel > 0x600) {
            this->groundVel = 0x600;

            this->automoveActive = false;
            this->timer          = 0;
        }
    }
    else {
        if (!this->automoveActive) {
            this->timer          = 60;
            this->automoveActive = true;
        }

        this->groundVel -= this->groundVel >> 4;
    }

    int32 speed = this->groundVel;
    if (this->rotation) {
        speed = (speed / 256) * 128;
    }
    else {
        speed >>= 1;
    }

    this->angle    = (this->angle + speed) & 0xFFFF;
    this->rotation = this->angle >> 7;
    

	// Wait for it....
    if (this->timer > 0) 
        this->timer--;
    else // Move to center
        this->groundVel += (Math::Sin256(this->rotation >> 1) * -80) >> 9;

    this->position.x = TO_FIXED((Math::Sin512(this->rotation) * this->xBoundsL) >> 9);
    this->position.y = TO_FIXED((Math::Cos512(this->rotation) * this->xBoundsL) >> 9);

    this->animator.SetAnimation(this->aniFrames, HP_Player::ANI_RUN, false, 0);

    int32 shadowAngle   = this->rotation;
    this->changedLeader = false;

    if (this->jumpPress) {
        this->groundVel  = 0;
        this->velocity.x = (Math::Sin512(this->rotation) * 0x5A0) >> 9;
        this->velocity.y = (Math::Cos512(this->rotation) * 0x980) >> 9;
        this->state.Set(&HP_Player::State_Air);
        this->rotation = 0;
        this->animator.SetAnimation(this->aniFrames, HP_Player::ANI_JUMP, true, 0);
        this->onGround = false;
        sVars->sfxJump.Play();
    }
    else {
        if (this->rotation > 152 && this->rotation < 360) {
            if (this->groundVel < 128 && this->groundVel > -128) {
                this->velocity.x = this->groundVel;
                this->velocity.y = 0;
                this->groundVel  = 0;
                this->state.Set(&HP_Player::State_Air);
            }
        }
    }

    if (shadowAngle > 0x80 && shadowAngle < 0x180)
        shadowAngle = -(shadowAngle + 0x100) & 0x1FF;

    this->localShadowPos.x = (Math::Sin512(shadowAngle) * -52) >> 1;
    this->localShadowPos.y = (Math::Cos512(shadowAngle) * -52) >> 1;

    this->shadowAnimator.frameID = 0;

    if (shadowAngle < 256) {
        if (shadowAngle > 112)
            this->shadowAnimator.frameID = 4;
        else if (shadowAngle > 39)
            this->shadowAnimator.frameID = 1;
    }
    else {
        if (shadowAngle < 400)
            this->shadowAnimator.frameID = 3;
        else if (shadowAngle < 473)
            this->shadowAnimator.frameID = 2;
    }

    if (this->rotation < 40 || this->rotation > 472)
        this->rotation = 0;
}
void HP_Player::State_Air()
{
    SET_CURRENT_STATE();

    if (globals->gameMode == MODE_COMPETITION) {
        if (!this->changedLeader) {
            sVars->leadingPlayer ^= 1;
            this->changedLeader = true;
        }
    }

    this->position.x += TO_FIXED(this->velocity.x);
    if (this->position.x <= TO_FIXED(this->xBoundsL))
        this->position.x = TO_FIXED(this->xBoundsL);

    if (this->position.x >= TO_FIXED(this->xBoundsR))
        this->position.x = TO_FIXED(this->xBoundsR);

    if (this->left)
        this->velocity.x -= 32;

    if (this->right)
        this->velocity.x += 32;

    this->position.y += TO_FIXED(this->velocity.y);
    this->velocity.y -= 88;

    if (this->position.y < 0) {
        int32 rx = FROM_FIXED(this->position.x);
        int32 ry = FROM_FIXED(this->position.y);

        if (rx * rx + ry * ry > this->groundPos) {
            this->state.Set(&HP_Player::State_Ground);
            this->onGround = true;
            this->angle    = (Math::ATan2(ry, rx) + 0x80) << 8;
        }
    }

    if (this->rotation < 0x100) {
        if (this->rotation > 0)
            this->rotation -= 8;
        else
            this->rotation = 0;
    }
    else {
        if (this->rotation < 0x200)
            this->rotation += 8;
        else
            this->rotation = 0;
    }

    this->localShadowPos.x = (FROM_FIXED(this->position.x) * 144) >> 8;

    int32 shadowYPosTable[] = { -128,       -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -127, -127, -127,
                                -127 - 127, -127, -127, -127, -126, -126, -126, -126, -126, -126, -125, -125, -125, -125, -124,
                                -124 - 124, -124, -123, -123, -123, -123, -122, -122, -122, -121, -121, -121, -120, -120, -119,
                                -119 - 119, -118, -118, -117, -117, -116, -116, -116, -115, -115, -114, -114, -113, -112, -112,
                                -111 - 111, -110, -110, -109, -108, -108, -107, -106, -106, -105, -104, -104, -103, -102, -101,
                                -101 - 100, -99,  -98,  -97,  -96,  -96,  -95,  -94,  -93,  -92,  -91,  -90,  -89,  -88,  -87,
                                -85 - 84,   -83,  -82,  -81,  -79,  -78,  -77,  -75,  -74,  -72,  -71,  -69,  -68,  -66,  -64,
                                -62 - 61,   -59,  -57,  -54,  -52,  -50,  -47,  -45,  -42,  -39,  -35,  -31,  -27,  -21,  -13,
                                0 };

    int32 temp0       = abs((FROM_FIXED(this->position.x) * 45) >> 13);
    this->localShadowPos.y = shadowYPosTable[temp0] * 105;

    this->shadowAnimator.frameID = 0;

    if (shadowYPosTable[temp0] > -112) {
        if (shadowYPosTable[temp0] > -24) {
            if (this->position.x < 0)
                this->shadowAnimator.frameID = 4;
            else
                this->shadowAnimator.frameID = 3;
        }
        else {
            if (this->position.x < 0)
                this->shadowAnimator.frameID = 1;
            else
                this->shadowAnimator.frameID = 2;
        }
    }
}
void HP_Player::State_Hurt()
{
    SET_CURRENT_STATE();

    if (globals->gameMode == MODE_COMPETITION) {
        if (this->playerID == 0) {
            if (!this->changedLeader) {
                sVars->leadingPlayer = (this->Slot() - SLOT_HP_PLAYER1) ^ this->playerID ^ 1;
                this->changedLeader  = true;
            }
        }
    }

    if (this->spinTimer < 512) {
        this->spinTimer += 16;
        this->rotation += 16;
    }
    else {
        this->groundVel          = 0;
        this->spinTimer          = 0;
        this->invincibilityTimer = 60;
        if (this->onGround)
            this->state.Set(&HP_Player::State_Ground);
        else
            this->state.Set(&HP_Player::State_Air);
    }
}
void HP_Player::Draw_Normal()
{
    SET_CURRENT_STATE();

    auto *scene3D = &HP_Halfpipe::sVars->scene3D;

    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);
    HP_Halfpipe::MatrixRotateXYZ(&scene3D->matWorld, halfpipe->worldRotation.x, halfpipe->worldRotation.y, halfpipe->worldRotation.z);
    HP_Halfpipe::MatrixTranslateXYZ(&scene3D->matTemp, halfpipe->worldTranslation.x, halfpipe->worldTranslation.y, halfpipe->worldTranslation.z);
    HP_Halfpipe::MatrixMultiply(&scene3D->matWorld, &scene3D->matWorld, &scene3D->matTemp);

    int32 scaleX = 0x580, scaleY = 0x580;
    if (this->zpos > 0x800) {
        scaleX = 0x600;
        scaleY = 0x600;
    }

    // Draw Shadow
    HP_Halfpipe::DrawShadow(this->localShadowPos.x, this->localShadowPos.y, this->zpos, scaleX, scaleY, &this->shadowAnimator, true);

    if (this->isVisible) {
        // Draw Player
        HP_Halfpipe::DrawSprite(FROM_FIXED(this->position.x) >> 1, FROM_FIXED(this->position.y) >> 1, this->zpos, this->drawFX, 0x200, 0x200,
                                this->rotation, &this->animator, this->aniFrames, true);

        if (this->characterID == ID_TAILS && this->animator.animationID == 0) {
            // Draw Tails sprite
            HP_Halfpipe::DrawSprite(FROM_FIXED(this->position.x) >> 1, FROM_FIXED(this->position.y) >> 1, this->zpos, this->drawFX, 0x200, 0x200,
                                    this->rotation, &this->tailAnimator, this->tailFrames, true);
        }
    }

    // If active, draw 1P/2P Tag
    // TODO: lol
}
bool32 HP_Player::Hurt()
{
    if (this->state.Matches(&HP_Player::State_Hurt) || this->invincibilityTimer)
        return false;

    this->Hit();

    return true;
}
void HP_Player::Hit()
{
    if (this->rings > 0) {
        HP_Collectable::LoseRings(&this->position, this->angle, this->rings, this->drawGroup);

        this->rings = MAX(this->rings - 10, 0);
        sVars->sfxLoseRings.Play();
    }

    if (this->onGround)
        this->state.Set(&HP_Player::State_Hurt);
    else
        this->invincibilityTimer = 60;
}

void HP_Player::GiveRings(int32 amount, bool32 playSfx)
{
    HP_Player *player = this;
    // if (player->sidekick)
    //     player = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);

    player->rings = CLAMP(player->rings + amount, 0, 999);

    if (playSfx) {
        if (HP_Collectable::sVars->pan) {
            int32 channel = HP_Collectable::sVars->sfxRing.Play();
            channels[channel].SetAttributes(1.0, -1.0, 1.0);
            HP_Collectable::sVars->pan = 0;
        }
        else {
            int32 channel = HP_Collectable::sVars->sfxRing.Play();
            channels[channel].SetAttributes(1.0, 1.0, 1.0);
            HP_Collectable::sVars->pan = 1;
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void HP_Player::EditorDraw()
{
    this->animator.frameID = this->characterID & 7;
    this->animator.DrawSprite(nullptr, false);
}

void HP_Player::EditorLoad() { sVars->sonicFrames.Load("Editor/PlayerIcons.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void HP_Player::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(HP_Player);

    sVars->sonicFrames.Init();
    sVars->tailsFrames.Init();
    sVars->tailSpriteFrames.Init();
    sVars->knuxFrames.Init();

    sVars->sfxJump.Init();
    sVars->sfxLoseRings.Init();
}
#endif

void HP_Player::Serialize() { RSDK_EDITABLE_VAR(HP_Player, VAR_ENUM, characterID); }

} // namespace GameLogic