// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: AIZRockPile Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "AIZRockPile.hpp"
#include "Global/Player.hpp"
#include "Global/Shield.hpp"
#include "Global/Debris.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(AIZRockPile);

void AIZRockPile::Update()
{
    Hitbox *hitbox = this->animator.GetHitbox(1);

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (!this->onlyKnux || player->characterID == ID_KNUCKLES) {
            int32 cMode     = player->collisionMode;
            int32 playerX   = player->position.x;
            int32 playerY   = player->position.y;
            int32 xVelocity = player->velocity.x;
            int32 yVelocity = player->velocity.y;
            int32 jumping   = player->animator.animationID == Player::ANI_JUMP;
            int32 groundVel = player->groundVel;

            if (this->smashSides || this->smashTop) {
                int32 side = player->CheckCollisionBox(this, hitbox);
                if (this->smashSides && (side == C_LEFT || side == C_RIGHT)) {
                    if (side == C_LEFT || side == C_RIGHT) {
                        bool32 canBreak = jumping && player->onGround && abs(groundVel) >= 0x48000;
                        if (player->shield == Player::Shield_Fire) {
                            Shield *shield = GameObject::Get<Shield>(Player::sVars->playerCount + RSDKTable->GetEntitySlot(player));
                            canBreak |= shield->shieldAnimator.animationID == Shield::AniFireAttack;
                        }

                        canBreak |= player->characterID == ID_SONIC && player->superState == Player::SuperStateSuper;
                        if (!player->sidekick && (canBreak || (player->characterID == ID_KNUCKLES))) {
                            player->position.x = playerX;
                            player->position.y = playerY;
                            player->velocity.x = xVelocity;
                            player->velocity.y = yVelocity;
                            player->groundVel  = groundVel;
                            if (xVelocity <= 0)
                                AIZRockPile::SpawnRocks(this->rockSpeedsL);
                            else
                                AIZRockPile::SpawnRocks(this->rockSpeedsR);

                            return;
                        }
                    }
                }

                if (this->smashTop && side == C_TOP) {
                    bool32 canBreak = jumping;
                    canBreak |= player->characterID == ID_SONIC && player->animator.animationID == Player::ANI_DROPDASH;
                    if (player->groundedStore && cMode != CMODE_FLOOR && cMode != CMODE_ROOF)
                        canBreak = false;

                    if (canBreak && !player->sidekick) {
                        player->onGround = false;
                        player->velocity.y = -0x30000;
                        AIZRockPile::SpawnRocks(this->rockSpeedsT);
                        return;
                    }
                }
            }
        }

        player->CheckCollisionBox(this, hitbox);
    }
}

void AIZRockPile::LateUpdate() {}

void AIZRockPile::StaticUpdate() {}

void AIZRockPile::Draw()
{
    this->animator.DrawSprite(nullptr, false);
}

void AIZRockPile::Create(void *data)
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    if (!sceneInfo->inEditor) {
        this->rockPositions = sVars->rockPositions_small;
        this->rockSpeedsT   = sVars->rockSpeedsT_small;
        this->rockSpeedsL   = sVars->rockSpeedsL_small;
        this->rockSpeedsR   = sVars->rockSpeedsR_small;

        this->active        = ACTIVE_BOUNDS;
        this->visible       = true;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
    }
}

void AIZRockPile::StageLoad()
{
    sVars->aniFrames.Load("HTZ/RockPile.bin", SCOPE_STAGE);

    sVars->sfxBreak.Get("Stage/LedgeBreak3.wav");

    DebugMode::AddObject(sVars->classID, &AIZRockPile::DebugSpawn, &AIZRockPile::DebugDraw);
}

void AIZRockPile::DebugSpawn() 
{
    AIZRockPile *rockPile = GameObject::Create<AIZRockPile>(nullptr, this->position.x, this->position.y);
    rockPile->smashTop    = true;
    rockPile->smashSides  = true;
    rockPile->onlyKnux    = false;
}

void AIZRockPile::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void AIZRockPile::SpawnRocks(int32 *speeds)
{
    for (int32 i = 0; i < 4; ++i) {
        //Debris *debris = GameObject::Create<Debris>(&Debris::State_Fall, this->position.x + this->rockPositions[2 * i], this->position.y + this->rockPositions[(2 * i) + 1]); // just using Debris::State_Fall doesnt work
        Debris *debris = GameObject::Create<Debris>(INT_TO_VOID(Debris::Fall), this->position.x + this->rockPositions[2 * i], this->position.y + this->rockPositions[(2 * i) + 1]); // instead using int to void and setting the type to fall
        debris->animator1.SetAnimation(sVars->aniFrames, 1, true, 0);
        debris->velocity.x      = speeds[2 * i];
        debris->velocity.y      = speeds[(2 * i) + 1];
        debris->drawGroup       = Zone::sVars->objectDrawGroup[1];
        debris->updateRange.x   = 0x800000;
        debris->updateRange.y   = 0x800000;
        debris->gravityStrength = 0x1800;
    }

    sVars->sfxBreak.Play(false, 255);
    this->Destroy();
}

#if RETRO_INCLUDE_EDITOR
void AIZRockPile::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    AIZRockPile::Draw();
}

void AIZRockPile::EditorLoad()
{
    sVars->aniFrames.Load("HTZ/RockPile.bin", SCOPE_STAGE);
}
#endif

#if RETRO_REV0U
void AIZRockPile::StaticLoad(Static* sVars)
{
    RSDK_INIT_STATIC_VARS(AIZRockPile);

    int32 rockPositions_small[] = { -0x40000, -0x40000, 0xC0000, -0x40000, -0xC0000, 0x40000, 0xC0000, 0x40000 };
    memcpy(sVars->rockPositions_small, rockPositions_small, sizeof(rockPositions_small));

    int32 rockSpeedsT_small[] = { -0x10000, -0x20000, 0x10000, -0x1E000, -0x1B000, -0x1C000, 0x1C000, -0x1C000 };
    memcpy(sVars->rockSpeedsT_small, rockSpeedsT_small, sizeof(rockSpeedsT_small));

    int32 rockSpeedsL_small[] = { -0x30000, -0x30000, -0x2C000, -0x28000, -0x2C000, -0x28000, -0x28000, -0x20000 };
    memcpy(sVars->rockSpeedsL_small, rockSpeedsL_small, sizeof(rockSpeedsL_small));

    int32 rockSpeedsR_small[] = { 0x2C000, -0x28000, 0x30000, -0x30000, 0x28000, -0x20000, 0x2C000, -0x28000 };
    memcpy(sVars->rockSpeedsR_small, rockSpeedsR_small, sizeof(rockSpeedsR_small));
}
#endif

void AIZRockPile::Serialize()
{
    RSDK_EDITABLE_VAR(AIZRockPile, VAR_BOOL, smashTop);
    RSDK_EDITABLE_VAR(AIZRockPile, VAR_BOOL, smashSides);
    RSDK_EDITABLE_VAR(AIZRockPile, VAR_BOOL, onlyKnux);
}

} // namespace GameLogic