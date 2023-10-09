// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Pinata Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Pinata.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Global/ScoreBonus.hpp"
#include "Global/Explosion.hpp"
#include "Global/Debris.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Pinata);

void Pinata::Update() { this->state.Run(this); }

void Pinata::LateUpdate() {}

void Pinata::StaticUpdate() {}

void Pinata::Draw() { this->animator.DrawSprite(nullptr, false); }

void Pinata::Create(void *data)
{
    this->drawGroup     = this->priority != PINATA_PRIO_HIGH ? Zone::sVars->objectDrawGroup[0] : Zone::sVars->objectDrawGroup[1];
    this->visible       = true;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x400000;
    this->updateRange.y = 0x400000;
    this->state.Set(&Pinata::State_CheckPlayerCollisions);

    this->animator.SetAnimation(sVars->aniFrames, 3, true, 0);
}

void Pinata::StageLoad()
{
    sVars->aniFrames.Load("SSZ/Pinata.bin", SCOPE_STAGE);

    sVars->hitboxPinata.left   = -12;
    sVars->hitboxPinata.top    = -16;
    sVars->hitboxPinata.right  = 12;
    sVars->hitboxPinata.bottom = 16;

    sVars->sfxPinata.Get("Stage/Pinata.wav");

    DebugMode::AddObject(sVars->classID, &Pinata::DebugSpawn, &Pinata::DebugDraw);
}

void Pinata::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void Pinata::DebugSpawn()
{
    GameObject::Create<Pinata>(nullptr, this->position.x, this->position.y);
}

void Pinata::State_CheckPlayerCollisions()
{
    this->animator.Process();

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (player->animator.animationID != Player::ANI_HURT && player->CheckBadnikTouch(this, &sVars->hitboxPinata)) {
            sVars->sfxPinata.Play(false, 0xFF);

            if (player->state.Matches(&Player::State_FlyCarried))
                GameObject::Get<Player>(SLOT_PLAYER2)->flyCarryTimer = 30;

            int32 anim = player->animator.animationID;
            if (anim != Player::ANI_FLY && anim != Player::ANI_FLY_LIFT_TIRED) {
                if (!player->state.Matches(&Player::State_TailsFlight)) {
                    if (!player->state.Matches(&Player::State_DropDash))
                        player->state.Set(&Player::State_Air);

                    if (anim != Player::ANI_JUMP && anim != Player::ANI_JOG && anim != Player::ANI_RUN && anim != Player::ANI_DASH)
                        player->animator.animationID = Player::ANI_WALK;
                }
            }

            if (player->animator.animationID != Player::ANI_FLY)
                player->applyJumpCap = false;

            if (player->velocity.y > -0x80000)
                player->velocity.y = -0x80000;

            player->onGround       = false;
            player->tileCollisions = TILECOLLISION_DOWN;
            GameObject::Create<ScoreBonus>(nullptr, this->position.x, this->position.y)->animator.frameID = 16;
            player->GiveScore(10);
            GameObject::Create<Explosion>(INT_TO_VOID(Explosion::Type3), this->position.x, this->position.y - 0x100000)->drawGroup =
                Zone::sVars->objectDrawGroup[1];

            for (int32 i = 0; i < 6; ++i) {
                int32 x              = this->position.x + Math::Rand(-0x80000, 0x80000);
                int32 y              = this->position.y + Math::Rand(-0x80000, 0x80000);
                Debris *debris       = GameObject::Create<Debris>(nullptr, x, y);

                debris->state.Set(&Debris::State_Fall);
                debris->gravityStrength = 0x4000;
                debris->velocity.x      = Math::Rand(0, 0x20000);
                if (debris->position.x < this->position.x)
                    debris->velocity.x = -debris->velocity.x;

                debris->velocity.y = Math::Rand(-0x40000, -0x10000);
                debris->drawFX     = FX_FLIP;
                debris->direction  = i & 3;
                debris->drawGroup  = Zone::sVars->objectDrawGroup[1];
                debris->animator1.SetAnimation(sVars->aniFrames, 0, true, Math::Rand(0, 4));
            }

            this->state.Set(&Pinata::State_Destroyed);
            this->visible = false;
            this->active  = ACTIVE_NORMAL;
        }
    }
}

void Pinata::State_Destroyed()
{
    Vector2 range = { TO_FIXED(256), TO_FIXED(256) };
    if (!this->CheckOnScreen(&range)) {
        this->state.Set(&Pinata::State_CheckPlayerCollisions);
        this->visible = true;
        this->active  = ACTIVE_BOUNDS;
    }
}

#if RETRO_INCLUDE_EDITOR
void Pinata::EditorDraw() { Pinata::Draw(); }

void Pinata::EditorLoad()
{
    sVars->aniFrames.Load("SSZ/Pinata.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, priority);
    RSDK_ENUM_VAR("High");
    RSDK_ENUM_VAR("Low");
}
#endif

void Pinata::Serialize()
{
    RSDK_EDITABLE_VAR(Pinata, VAR_BOOL, respawn);
    RSDK_EDITABLE_VAR(Pinata, VAR_ENUM, priority);
}

} // namespace GameLogic