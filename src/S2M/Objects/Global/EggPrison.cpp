// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: EggPrison Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "EggPrison.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "DebugMode.hpp"
#include "Music.hpp"
#include "StarPost.hpp"
#include "ActClear.hpp"
#include "Explosion.hpp"
#include "Animals.hpp"
#include "Ring.hpp"
#include "Debris.hpp"
#include "Helpers/BadnikHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(EggPrison);

void EggPrison::Update()
{
    this->state.Run(this);

    if (this->type == EggPrison::InAir) {
        this->propellerAnimator.Process();

        HoverMovement();

        if (!this->notSolid) {
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                if (player->CheckCollisionBox(this, &this->hitboxSolid) == C_TOP) {
                    player->position.x += this->velocity.x;
                    player->position.y += 0x10000;
                }

                if (this->state.Matches(&EggPrison::State_Idle)) {
                    if (player->CheckCollisionBox(this, &this->hitboxButton) == C_BOTTOM) {
                        this->velocity.x = 0;
                        this->active     = ACTIVE_NORMAL;
                        this->state.Set(&EggPrison::State_OpenPt1);
                        this->buttonPos = -0x80000;
                    }
                    else {
                        if (!player->CheckBadnikTouch(this, &this->hitboxButtonTrigger)) {
                            if (this->buttonPos < 0)
                                this->buttonPos += 0x10000;
                        }
                        else {
                            Hitbox *playerHitbox = player->GetHitbox();
                            this->buttonPos      = ((playerHitbox->top - 48) << 16) - this->position.y + player->position.y;
                            this->buttonPos      = CLAMP(this->buttonPos, -0x80000, 0);
                            this->buttonPos &= 0xFFFF0000;
                        }
                    }
                }
                else {
                    player->CheckCollisionBox(this, &this->hitboxButton);
                }
            }
        }
    }
    else {
        if (!this->notSolid) {
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                player->CheckCollisionBox(this, &this->hitboxSolid);

                if (this->state.Matches(&EggPrison::State_Idle)) {
                    if (player->CheckCollisionBox(this, &this->hitboxButton) == C_TOP) {
                        this->buttonPos = 0x80000;

                        if (this->type < EggPrison::Animals)
                            sceneInfo->timeEnabled = false;

                        if (this->type == EggPrison::Spring) {
                            int32 anim = player->animator.animationID;
                            if ((anim >= Player::ANI_WALK && anim <= Player::ANI_DASH))
                                player->animationReserve = player->animator.animationID;
                            else
                                player->animationReserve = Player::ANI_WALK;
                            player->state.Set(&Player::State_Air);
                            player->onGround   = false;
                            player->velocity.y = -0xA0000;
                            player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                            sVars->sfxSpring.Play();
                        }
                        else {
                            this->active = ACTIVE_NORMAL;
                            this->state.Set(&EggPrison::State_OpenPt1);
                        }
                    }
                    else {
                        if (player->CheckBadnikTouch(this, &this->hitboxButtonTrigger)) {
                            Hitbox *playerHitbox = player->GetHitbox();

                            this->buttonPos = ((playerHitbox->bottom + 48) << 16) - this->position.y + player->position.y;
                            this->buttonPos = CLAMP(this->buttonPos, 0, 0x80000);
                            this->buttonPos &= 0xFFFF0000;
                        }
                        else if (this->buttonPos > 0)
                            this->buttonPos -= 0x10000;
                    }
                }
                else {
                    player->CheckCollisionBox(this, &this->hitboxButton);
                }
            }
        }
    }
}
void EggPrison::LateUpdate() {}
void EggPrison::StaticUpdate() {}
void EggPrison::Draw()
{
    Vector2 drawPos;

    if (this->type == EggPrison::InAir) {
        drawPos.x = this->position.x;
        drawPos.y = this->position.y + this->buttonPos;

        this->direction = FLIP_Y;
        this->buttonAnimator.DrawSprite(&drawPos, false);

        this->direction = FLIP_Y;
        this->capsuleAnimator.DrawSprite(nullptr, false);
        this->panelAnimator.DrawSprite(nullptr, false);

        this->direction = FLIP_NONE;
        this->propellerAnimator.DrawSprite(nullptr, false);

        this->direction = FLIP_X;
        this->propellerAnimator.DrawSprite(nullptr, false);

        this->direction = FLIP_NONE;
    }
    else {
        drawPos.x = this->position.x;
        drawPos.y = this->position.y + this->buttonPos;

        this->buttonAnimator.DrawSprite(&drawPos, false);
        this->capsuleAnimator.DrawSprite(nullptr, false);
        this->panelAnimator.DrawSprite(nullptr, false);
    }
}

void EggPrison::Create(void *data)
{
    if (globals->gameMode < MODE_TIMEATTACK || this->type >= EggPrison::InAir) {
        this->drawFX = FX_FLIP;

        if (!sceneInfo->inEditor) {
            if (data)
                this->type = VOID_TO_INT(data);

            this->hitboxSolid.left   = -32;
            this->hitboxSolid.top    = -24;
            this->hitboxSolid.right  = 32;
            this->hitboxSolid.bottom = 32;

            if (this->type == EggPrison::InAir) {
                this->hitboxButton.left   = -16;
                this->hitboxButton.top    = 24;
                this->hitboxButton.right  = 16;
                this->hitboxButton.bottom = 28;

                this->hitboxButtonTrigger.left   = -15;
                this->hitboxButtonTrigger.top    = 24;
                this->hitboxButtonTrigger.right  = 15;
                this->hitboxButtonTrigger.bottom = 48;
            }
            else {
                this->hitboxButton.left   = -16;
                this->hitboxButton.top    = -38;
                this->hitboxButton.right  = 16;
                this->hitboxButton.bottom = -24;

                this->hitboxButtonTrigger.left   = -15;
                this->hitboxButtonTrigger.top    = -48;
                this->hitboxButtonTrigger.right  = 15;
                this->hitboxButtonTrigger.bottom = -24;
            }

            this->capsuleAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->buttonAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

            if (globals->gameSpriteStyle == GAME_SM) {
                if (this->type == EggPrison::InAir)
                    this->panelAnimator.SetAnimation(sVars->aniFrames, 2, true, 1);
                else
                    this->panelAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
            }

            if (this->type == EggPrison::InAir)
                this->propellerAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);

            this->velocity.x = 0x10000;
            this->velocity.y = 0x4000;

            this->state.Set(&EggPrison::State_Init);
            this->drawGroup = Zone::sVars->objectDrawGroup[0];

            this->state.Set(&EggPrison::State_Init);
            this->active        = ACTIVE_BOUNDS;
            this->updateRange.x = 0x800000;
            this->updateRange.y = 0x800000;
            this->visible       = true;
            this->enableFlyAway = false;
        }
    }
    else {
        this->Destroy();
    }
}

void EggPrison::StageLoad()
{
    sVars->aniFrames.Load("Global/EggPrison.bin", SCOPE_STAGE);

    sVars->sfxDestroy.Get("Global/Destroy.wav");
    sVars->sfxSpring.Get("Global/Spring.wav");

    DebugMode::AddObject(sVars->classID, &EggPrison::DebugSpawn, &EggPrison::DebugDraw);
}

void EggPrison::DebugSpawn()
{
    EggPrison *capsule = GameObject::Create<EggPrison>(0, this->position.x, this->position.y);
    capsule->type      = DebugMode::sVars->itemType;
}
void EggPrison::DebugDraw()
{
    DebugMode::sVars->itemTypeCount = 6;

    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void EggPrison::HoverMovement()
{
    SET_CURRENT_STATE();

    bool32 hitFloor = false;
    if (this->checkTileCollisions) {
        if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, -0x300000, 0x900000, false)
            || this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0x300000, 0x900000, false)) {
            this->originY -= this->velocity.y;
            hitFloor = true;
        }
    }

    if (!hitFloor) {
        if (this->originY < (screenInfo->position.y + 64) << 16)
            this->originY += this->velocity.y;
    }

    if (!this->state.Matches(&EggPrison::State_FlyAway)) {
        if (this->velocity.x > 0) {
            if (this->position.x <= (screenInfo->position.x + screenInfo->size.x - 48) << 16) {
                if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_LWALL, 0, 0x400000, 0, true))
                    this->velocity.x = -this->velocity.x;
            }
            else {
                this->velocity.x = -this->velocity.x;
            }
        }
        else if (this->position.x < (screenInfo->position.x + 48) << 16) {
            this->velocity.x = -this->velocity.x;
        }
        else {
            if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_RWALL, 0, -0x400000, 0, true))
                this->velocity.x = -this->velocity.x;
        }
    }

    this->position.x += this->velocity.x;
    this->position.y = BadnikHelpers::Oscillate(this, this->originY, 4, 10);
}

void EggPrison::State_Init()
{
    SET_CURRENT_STATE();

    this->originY = this->position.y;
    this->state.Set(&EggPrison::State_Idle);
}

void EggPrison::State_Idle()
{
    SET_CURRENT_STATE();

    for (int32 p = 0; p < Player::sVars->playerCount && this->type < EggPrison::Animals && !this->dontSetBounds; ++p) {
        Player *player = GameObject::Get<Player>(p);
        if (!player->sidekick) {
            if (abs(this->position.x - player->position.x) < 0x1000000) {
                if (abs(this->position.y - player->position.y) < 0x1000000 && this->position.x - (Zone::sVars->cameraBoundsR[p] << 16) < 0x1000000) {
                    Zone::sVars->playerBoundActiveL[p] = true;
                    Zone::sVars->playerBoundActiveR[p] = true;

                    if (this->type == EggPrison::OnGround) {
                        Zone::sVars->cameraBoundsL[p] = (this->position.x >> 0x10) - screenInfo[p].center.x;
                        Zone::sVars->cameraBoundsR[p] = (this->position.x >> 0x10) + screenInfo[p].center.x;
                    }
                }
            }
        }
    }
}

void EggPrison::State_OpenPt1()
{
    SET_CURRENT_STATE();

    this->panelAnimator.SetAnimation(nullptr, 0, true, 0);
    this->state.Set(&EggPrison::State_OpenPt2);

    switch (this->type) {
        default:
        case EggPrison::OnGround:
        case EggPrison::InAir:
        case EggPrison::Animals:
        case EggPrison::Spring:
            for (int32 a = 0; a < 10; ++a) {
                int32 x                     = this->position.x + (((Math::Rand(0, 48) & -4) - 24) << 16);
                int32 y                     = this->position.y + 0x40000;
                GameLogic::Animals *animals = GameObject::Create<GameLogic::Animals>(Animals::sVars->animalTypes[a & 1] + 1, x, y);

                animals->timer = 4 * a;
                animals->state.Set(&Animals::State_Placed);
                animals->behaviour = Animals::BehaveFollow;
                animals->direction = (a ^ (a >> 1)) & 1;
            }
            break;

        case EggPrison::Rings: {
            int32 angle = 0x90;
            for (int32 r = 0; r < 6; ++r) {
                int32 x    = this->position.x + (((Math::Rand(0, 48) & -4) - 24) << 16);
                int32 y    = this->position.y;
                Ring *ring = GameObject::Create<Ring>(this, x, y);

                ring->velocity.x     = Math::Cos256(angle) << 9;
                ring->velocity.y     = Math::Sin256(angle) << 9;
                ring->animator.speed = 512;
                ring->collisionPlane = 0;
                ring->inkEffect      = INK_ALPHA;
                if (angle != 0xB0)
                    angle += 8;
                else
                    angle = 0xD0;
                ring->alpha = 0x100;
                ring->state.Set(&Ring::State_Lost);
                ring->stateDraw.Set(&Ring::Draw_Normal);
            }
            break;
        }

        case EggPrison::Enemies: {
            // Technosqueek *technosqueek       = GameObject::Create<Technosqueek>(nullptr, this->position.x - 0x80000, this->position.y);
            // technosqueek->velocity.x         = -0x30000;
            // technosqueek->velocity.y         = -0x40000;
            // technosqueek->active             = ACTIVE_NORMAL;
            // technosqueek->state.Set(&Technosqueek::State_Fall);
            //
            // technosqueek             = GameObject::Create<Technosqueek>(nullptr, this->position.x + 0x80000, this->position.y);
            // technosqueek->velocity.x = 0x30000;
            // technosqueek->velocity.y = -0x40000;
            // technosqueek->active     = ACTIVE_NORMAL;
            // technosqueek->direction  = FLIP_X;
            // technosqueek->state.Set(&Technosqueek::State_Fall);
            //
            // Blaster *blaster       = GameObject::Create<Blaster>(nullptr, this->position.x - 0x180000, this->position.y);
            // blaster->velocity.x    = -0x30000;
            // blaster->velocity.y    = -0x30000;
            // blaster->active        = ACTIVE_NORMAL;
            // blaster->state.Set(&Blaster::State_Fall);
            //
            // blaster             = GameObject::Create<Blaster>(nullptr, this->position.x + 0x180000, this->position.y);
            // blaster->velocity.x = 0x30000;
            // blaster->velocity.y = -0x30000;
            // blaster->active     = ACTIVE_NORMAL;
            // blaster->direction  = FLIP_X;
            // blaster->state.Set(&Blaster::State_Fall);
            break;
        }
    }
}

void EggPrison::State_OpenPt2()
{
    SET_CURRENT_STATE();

    if (!(this->timer % 3)) {
        int32 x = this->position.x + (Math::Rand(-24, 24) << 16);
        int32 y = this->position.y + (Math::Rand(-24, 24) << 16);

        int32 type = Explosion::Type0;
        if (globals->useManiaBehavior)
            type = (Math::Rand(0, 256) > 192) ? Explosion::Type3 : Explosion::Type1;

        Explosion *explosion = GameObject::Create<Explosion>(type, x, y);
        explosion->drawGroup = Zone::sVars->objectDrawGroup[1];
        sVars->sfxDestroy.Play();
    }

    if (++this->timer == 20) {
        this->timer = 0;
        if (this->type < EggPrison::Animals) {
            this->state.Set(&EggPrison::State_Wait);
            Music::FadeOut(0.05f);
        }
        else {
            this->state.Set(nullptr);
        }
    }
}

void EggPrison::State_Wait()
{
    SET_CURRENT_STATE();

    if (++this->timer == 60) {
        this->timer = 0;
        if (this->enableFlyAway && this->type == EggPrison::InAir)
            this->state.Set(&EggPrison::State_FlyAway);
        else
            this->state.Set(nullptr);
        Zone::sVars->shouldRecoverPlayers = false;
        GameObject::Reset(SLOT_ACTCLEAR, ActClear::sVars->classID, nullptr);
    }
}

void EggPrison::State_FlyAway()
{
    SET_CURRENT_STATE();

    if (globals->useManiaBehavior) {
        if (this->velocity.x > -0x30000)
            this->velocity.x -= 0x1000;
    }
    else {
        if (this->velocity.x > -0x10000)
            this->velocity.x -= 0x1000;
    }

    if (!this->CheckOnScreen(nullptr))
        this->Destroy();
}

#if RETRO_INCLUDE_EDITOR
void EggPrison::EditorDraw()
{
    this->capsuleAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->buttonAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

    if (this->type == EggPrison::InAir) {
        this->panelAnimator.SetAnimation(sVars->aniFrames, 2, true, 1);
        this->propellerAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);

        this->direction = FLIP_Y;
        this->buttonAnimator.DrawSprite(nullptr, false);

        this->direction = FLIP_Y;
        this->capsuleAnimator.DrawSprite(nullptr, false);

        this->direction = FLIP_NONE;
        this->propellerAnimator.DrawSprite(nullptr, false);

        this->direction = FLIP_X;
        this->propellerAnimator.DrawSprite(nullptr, false);

        this->direction = FLIP_NONE;
    }
    else {
        this->panelAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);

        this->buttonAnimator.DrawSprite(nullptr, false);
        this->capsuleAnimator.DrawSprite(nullptr, false);
    }
}

void EggPrison::EditorLoad()
{
    sVars->aniFrames.Load("Global/EggPrison.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("On Ground");
    RSDK_ENUM_VAR("In Air");
    RSDK_ENUM_VAR("Animals");
    RSDK_ENUM_VAR("Rings");
    RSDK_ENUM_VAR("Enemies");
    RSDK_ENUM_VAR("Spring");
    RSDK_ENUM_VAR("Hyudoro");
}
#endif

#if RETRO_REV0U
void EggPrison::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(EggPrison);

    sVars->aniFrames.Init();

    sVars->sfxDestroy.Init();
    sVars->sfxSpring.Init();
}
#endif

void EggPrison::Serialize()
{
    RSDK_EDITABLE_VAR(EggPrison, VAR_INT32, type);
    RSDK_EDITABLE_VAR(EggPrison, VAR_BOOL, dontSetBounds);
}

} // namespace GameLogic