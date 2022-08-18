// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: StarPost Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "StarPost.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "SaveGame.hpp"
#include "DebugMode.hpp"
#include "Music.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(StarPost);

void StarPost::Update() { this->state.Run(this); }
void StarPost::LateUpdate() {}
void StarPost::StaticUpdate() {}
void StarPost::Draw()
{
    this->poleAnimator.DrawSprite(&this->position, false);

    this->ballPos.x = this->position.x - 0x280 * Math::Cos1024(this->angle);
    this->ballPos.y = this->position.y - 0x280 * Math::Sin1024(this->angle) - 0xE0000;
    this->ballAnimator.DrawSprite(&this->ballPos, false);

    Vector2 drawPos;
    if (this->bonusStageID > 0) {
        int32 angleX    = this->starAngle.x;
        int32 amplitude = 3 * Math::Sin512(this->starAngle.y);
        for (int32 i = 0; i < 4; ++i) {
            drawPos.x = this->position.x + ((Math::Sin512(angleX) << 12) * this->starRadius >> 7);
            drawPos.y = (((amplitude * Math::Sin512(angleX)) + (Math::Cos512(angleX) << 10)) * this->starRadius >> 7) + this->position.y - 0x320000;
            this->starAnimator.DrawSprite(&drawPos, false);
            angleX += 128;
        }
    }
}

void StarPost::Create(void *data)
{

    if (globals->gameMode == MODE_TIMEATTACK || (globals->gameMode == MODE_COMPETITION && this->vsRemove)) {
        this->Destroy();
    }
    else {
        if (!sceneInfo->inEditor) {
            this->visible       = true;
            this->drawGroup     = Zone::sVars->objectDrawGroup[0];
            this->active        = ACTIVE_BOUNDS;
            this->updateRange.x = 0x400000;
            this->updateRange.y = 0x400000;
            this->state.Set(&StarPost::State_Idle);
            this->angle = 256;
        }

        this->poleAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        if (this->interactedPlayers) {
            this->ballAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
            this->ballAnimator.speed = 64;
        }
        else {
            this->ballAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        }
        this->starAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
        this->ballPos.x = this->position.x;
        this->ballPos.y = this->position.y - 0x180000;
    }
}

void StarPost::StageLoad()
{
    sVars->aniFrames.Load("Global/StarPost.bin", SCOPE_STAGE);

    sVars->hitbox.left   = -8;
    sVars->hitbox.top    = -44;
    sVars->hitbox.right  = 8;
    sVars->hitbox.bottom = 20;

    sVars->interactablePlayers = (1 << Player::sVars->playerCount) - 1;

    DebugMode::AddObject(sVars->classID, &StarPost::DebugSpawn, &StarPost::DebugDraw);

    for (int32 p = 0; p < Player::sVars->playerCount; ++p) {
        if (sVars->postIDs[p]) {
            Player *player          = GameObject::Get<Player>(p);
            StarPost *savedStarPost = GameObject::Get<StarPost>(sVars->postIDs[p]);

            for (auto starPost : GameObject::GetEntities<StarPost>(FOR_ALL_ENTITIES)) {
                if (starPost->id < savedStarPost->id && !starPost->interactedPlayers) {
                    starPost->interactedPlayers = sVars->interactablePlayers;
                    starPost->ballAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
                }
            }

            if (!globals->specialRingID) {
                if (globals->gameMode < MODE_TIMEATTACK) {
                    int32 ms = sceneInfo->milliseconds;
                    int32 s  = sceneInfo->seconds;
                    int32 m  = sceneInfo->minutes;
                    if (!(ms || s || m) || ms != globals->tempMilliseconds || s != globals->tempSeconds || m != globals->tempMinutes) {
                        sceneInfo->milliseconds = sVars->storedMilliseconds;
                        sceneInfo->seconds      = sVars->storedSeconds;
                        sceneInfo->minutes      = sVars->storedMinutes;
                    }
                }

                player->position.x = sVars->playerPositions[p].x;
                player->position.y = sVars->playerPositions[p].y + 0x100000;
                player->direction  = sVars->playerDirections[p];

                if (!p) {
                    Player *sidekick = GameObject::Get<Player>(SLOT_PLAYER2);
                    if (globals->gameMode != MODE_COMPETITION) {
                        sidekick->position.x = player->position.x;
                        sidekick->position.y = player->position.y;
                        sidekick->direction  = player->direction;
                        if (player->direction)
                            sidekick->position.x += 0x100000;
                        else
                            sidekick->position.x -= 0x100000;

                        for (int32 p = 0; p < 0x10; ++p) {
                            Player::sVars->leaderPositionBuffer[p].x = player->position.x;
                            Player::sVars->leaderPositionBuffer[p].y = player->position.y;
                        }
                    }
                }
            }

            savedStarPost->interactedPlayers = sVars->interactablePlayers;
        }

        if (globals->gameMode == MODE_COMPETITION || globals->gameMode == MODE_ENCORE) {
            Player *player              = GameObject::Get<Player>(p);
            sVars->playerPositions[p].x = player->position.x;
            sVars->playerPositions[p].y = player->position.y - 0x100000;
            sVars->playerDirections[p]  = player->direction;
        }
    }

    sVars->sfxStarPost.Get("Global/StarPost.wav");
    sVars->sfxWarp.Get("Global/SpecialWarp.wav");

    sVars->active = ACTIVE_ALWAYS;
}

void StarPost::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 1);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}
void StarPost::DebugSpawn() { GameObject::Create<StarPost>(nullptr, this->position.x, this->position.y); }
void StarPost::ResetStarPosts()
{
    for (int32 i = 0; i < Player::sVars->playerCount; ++i) sVars->postIDs[i] = 0;

    sVars->storedMilliseconds = 0;
    sVars->storedSeconds      = 0;
    sVars->storedMinutes      = 0;
}
void StarPost::CheckBonusStageEntry()
{
    this->starAngle.y += 4;
    this->starAngle.y &= 0x1FF;
    this->starAngle.x += 18;
    this->starAngle.x &= 0x1FF;

    if (this->starTimer > 472)
        --this->starRadius;
    else if (this->starTimer < 0x80)
        ++this->starRadius;

    if (++this->starTimer == 600) {
        this->starTimer    = 0;
        this->bonusStageID = 0;
        this->active       = ACTIVE_BOUNDS;
    }

    this->starAnimator.frameID = (this->starAngle.y >> 3) & 3;

    this->hitboxStars.left   = -(this->starRadius >> 2);
    this->hitboxStars.top    = -48;
    this->hitboxStars.right  = this->starRadius >> 2;
    this->hitboxStars.bottom = -40;

    if (this->starTimer >= 60) {
        if (!globals->recallEntities) {
            Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);
            if (player1->CheckCollisionTouch(this, &this->hitboxStars)) {
                SaveGame::StoreStageState();
                sVars->sfxWarp.Play(false, 0xFE);
                Stage::SetEngineState(ENGINESTATE_FROZEN);

                {
                    Zone::GoBonus_Pinball();
                }
            }
        }
    }
}
void StarPost::CheckCollisions()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 playerID = player->Slot();
        if (!((1 << playerID) & this->interactedPlayers) && !player->sidekick) {
            if (player->CheckCollisionTouch(this, &sVars->hitbox)) {
                this->state.Set(&StarPost::State_Spinning);

                for (auto starPost : GameObject::GetEntities<StarPost>(FOR_ALL_ENTITIES)) {
                    if (starPost->id < this->id && !starPost->interactedPlayers) {
                        starPost->interactedPlayers = 1 << playerID;
                        starPost->ballAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
                    }
                }

                sVars->postIDs[playerID]           = sceneInfo->entitySlot;
                sVars->playerPositions[playerID].x = this->position.x;
                sVars->playerPositions[playerID].y = this->position.y;
                sVars->playerDirections[playerID]  = this->direction;
                if (globals->gameMode < MODE_TIMEATTACK) {
                    sVars->storedMilliseconds = sceneInfo->milliseconds;
                    sVars->storedSeconds      = sceneInfo->seconds;
                    sVars->storedMinutes      = sceneInfo->minutes;
                }

                int32 playerVelocity = player->onGround ? player->groundVel : player->velocity.x;
                if (globals->useManiaBehavior) {
                    int32 ballSpeed = -12 * (playerVelocity >> 17);

                    if (ballSpeed >= 0)
                        ballSpeed += 32;
                    else
                        ballSpeed -= 32;

                    if (!this->ballSpeed) {
                        this->ballSpeed = ballSpeed;
                    }
                    else if (this->ballSpeed <= 0) {
                        if (ballSpeed < this->ballSpeed) {
                            this->ballSpeed = ballSpeed;
                        }
                        else if (ballSpeed > 0) {
                            ballSpeed += this->ballSpeed;
                            this->ballSpeed = ballSpeed;
                        }
                    }
                    else {
                        if (ballSpeed > this->ballSpeed) {
                            this->ballSpeed = ballSpeed;
                        }
                        else if (ballSpeed < 0) {
                            ballSpeed += this->ballSpeed;
                            this->ballSpeed = ballSpeed;
                        }
                    }
                }
                else {
                    if (playerVelocity < 0)
                        this->ballSpeed = 64;
                    else
                        this->ballSpeed = -64;
                }

                this->timer = 0;
                if (globals->gameMode < MODE_TIMEATTACK) {
                    int32 quota = 0;

                    bool32 spawnStars = false;

                    {
                        if (globals->gameMode == MODE_ENCORE)
                            quota = 50;
                        else
                            quota = 25;

                        if (player->rings >= quota) {
                            this->bonusStageID = ((player->rings - 20) / 15 % 3) + 1;

                            switch (this->bonusStageID) {
                                default: break;

                                case 1:
                                    this->starAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
                                    spawnStars = true;
                                    break;

                                case 2:
                                    this->starAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
                                    spawnStars = true;
                                    break;

                                case 3:
                                    this->starAnimator.SetAnimation(sVars->aniFrames, 5, true, 0);
                                    spawnStars = true;
                                    break;
                            }
                        }
                    }

                    if (spawnStars) {
                        this->starTimer   = 0;
                        this->starAngle.y = 0;
                        this->starAngle.x = 0;
                        this->starRadius  = 0;
                    }
                }

                if (!this->interactedPlayers) {
                    this->ballAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
                    this->ballAnimator.speed = 0;
                }

                this->interactedPlayers |= 1 << playerID;
                this->active = ACTIVE_NORMAL;
                sVars->sfxStarPost.Play();
            }
        }
    }
}
void StarPost::State_Idle()
{
    if (this->interactedPlayers < sVars->interactablePlayers)
        StarPost::CheckCollisions();

    if (this->bonusStageID > 0)
        StarPost::CheckBonusStageEntry();

    this->ballAnimator.Process();
}
void StarPost::State_Spinning()
{
    if (this->interactedPlayers < sVars->interactablePlayers)
        StarPost::CheckCollisions();

    this->angle += this->ballSpeed;
    if (!sVars->hasAchievement && this->timer == 10) {
        sVars->hasAchievement = true;
    }

    bool32 isIdle = false;
    if (globals->useManiaBehavior) {
        if (this->ballSpeed <= 0) {
            if (this->angle <= -0x300) {
                ++this->timer;
                this->angle += 0x400;

                this->ballSpeed += 8;
                if (this->ballSpeed > -32)
                    this->ballSpeed = -32;

                isIdle = this->ballSpeed == -32;
            }
        }
        else {
            if (this->angle >= 0x500) {
                ++this->timer;
                this->angle -= 0x400;

                this->ballSpeed -= 8;
                if (this->ballSpeed < 32)
                    this->ballSpeed = 32;

                isIdle = this->ballSpeed == 32;
            }
        }
    }
    else {
        if (this->ballSpeed <= 0) {
            if (this->angle <= -0x300) {
                this->angle += 0x400;
                this->timer++;
            }
        }
        else {
            if (this->angle >= 0x500) {
                this->angle -= 0x400;
                this->timer++;
            }
        }
        isIdle = this->timer == 2;
    }

    if (isIdle) {
        this->state.Set(&StarPost::State_Idle);
        this->ballAnimator.speed = 64;
        this->ballSpeed          = 0;
        this->angle              = 0x100;

        if (!this->bonusStageID)
            this->active = ACTIVE_BOUNDS;
    }

    if (this->bonusStageID > 0)
        StarPost::CheckBonusStageEntry();

    this->ballAnimator.Process();
}

#if RETRO_INCLUDE_EDITOR
void StarPost::EditorDraw()
{
    this->ballPos.x = this->position.x;
    this->ballPos.y = this->position.y - 0x180000;

    this->poleAnimator.DrawSprite(&this->position, false);
    this->ballAnimator.DrawSprite(&this->ballPos, false);
}

void StarPost::EditorLoad()
{
    sVars->aniFrames.Load("Global/StarPost.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Right");
    RSDK_ENUM_VAR("Left");
}
#endif

#if RETRO_REV0U
void StarPost::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(StarPost);

    sVars->aniFrames.Init();

    sVars->sfxStarPost.Init();
    sVars->sfxWarp.Init();
}
#endif

void StarPost::Serialize()
{
    RSDK_EDITABLE_VAR(StarPost, VAR_ENUM, id);
    RSDK_EDITABLE_VAR(StarPost, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(StarPost, VAR_BOOL, vsRemove);
}

} // namespace GameLogic