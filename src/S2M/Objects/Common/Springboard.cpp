// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Springboard Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Springboard.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Springboard);

void Springboard::Update()
{
    this->animator.Process();

    // bounceDelay is unused, but if it was used, it'd prolly be "if (!--this->bounceDelay) {" around this foreach loop.
    // source: it was exactly like that in S2 '13
    for (auto playerPtr : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        if (playerPtr->velocity.y >= 0 && ((1 << RSDKTable->GetEntitySlot(playerPtr)) & this->activePlayers)) {
            int32 pos = CLAMP((playerPtr->position.x - this->position.x + 0x1C0000) >> 17, 0, 28);

            if ((this->direction & FLIP_X))
                pos = 28 - pos;

            if (pos >= 8) {
                if (this->animator.frameID == 3)
                    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

                if (this->animator.frameID == 2) {
                    int32 anim = playerPtr->animator.animationID;
                    if (anim == Player::ANI_WALK || (anim > Player::ANI_FALL && anim <= Player::ANI_DASH))
                        playerPtr->animationReserve = playerPtr->animator.animationID;
                    else
                        playerPtr->animationReserve = Player::ANI_WALK;

                    playerPtr->state.Set(&Player::State_Air);
                    playerPtr->onGround       = false;
                    playerPtr->tileCollisions = TILECOLLISION_DOWN;
                    playerPtr->animator.SetAnimation(playerPtr->aniFrames, Player::ANI_SPRING_CS, true, 1);
                    playerPtr->groundVel    = playerPtr->velocity.x;
                    playerPtr->velocity.y   = sVars->springPower[MIN(2 * pos - 16, 39)] - playerPtr->gravityStrength - this->force;
                    playerPtr->applyJumpCap = false;

                    sVars->sfxSpring.Play(false, 0xFF);
                }
            }
        }
    }

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        int32 playerID = RSDKTable->GetEntitySlot(player);

        int32 playerGndVel = player->groundVel;
        int32 playerVelX   = player->velocity.x;
        int32 springPos    = CLAMP((player->position.x - this->position.x + 0x1C0000) >> 17, 0, 27);
        bool32 bounced     = false;

        if (!this->direction) {
            int32 hitboxTop = 0;

            if (this->animator.frameID <= 2)
                hitboxTop = sVars->heightsFlat[springPos];
            else if (this->animator.frameID == 3)
                hitboxTop = sVars->heightsReady[springPos];

            Hitbox hitbox;
            hitbox.left   = -28;
            hitbox.top    = -hitboxTop;
            hitbox.right  = 28;
            hitbox.bottom = 8;

            uint8 collision = 0;
            if (!((1 << playerID) & this->activePlayers))
                bounced = collision = player->CheckCollisionBox(this, &hitbox);
            else
                bounced = collision = player->CheckCollisionPlatform(this, &hitbox);

            bounced = collision == C_TOP;
            switch (collision) {
                case C_LEFT:
                    player->groundVel  = playerGndVel;
                    player->velocity.x = playerVelX;
                    // [Fallthrough]
                case C_NONE:
                case C_RIGHT:
                case C_BOTTOM:
                    if (player->velocity.y >= 0 && ((1 << playerID) & this->activePlayers)) {
                        Hitbox *playerHitbox = player->GetHitbox();
                        player->position.y   = this->position.y - (playerHitbox->bottom << 16) - (hitboxTop << 16);

                        if (!bounced)
                            bounced = player->position.x > this->position.x;
                    }
                    break;

                case C_TOP:
#if MANIA_USE_PLUS
                    if (player->state == Player_State_MightyHammerDrop)
                        player->state = Player_State_Air;
#endif
                    break;

                default: break;
            }

            player->flailing = false;
        }
        else if (this->direction == FLIP_X) {
            int32 pos       = abs(springPos - 27);
            int32 hitboxTop = 0;

            if (this->animator.frameID <= 2)
                hitboxTop = sVars->heightsFlat[pos];
            else if (this->animator.frameID == 3)
                hitboxTop = sVars->heightsReady[pos];

            Hitbox hitbox;
            hitbox.left   = -28;
            hitbox.top    = -hitboxTop;
            hitbox.right  = 28;
            hitbox.bottom = 8;

            uint8 collision = 0;
            if (!((1 << playerID) & this->activePlayers))
                collision = player->CheckCollisionBox(this, &hitbox);
            else
                collision = player->CheckCollisionPlatform(this, &hitbox);

            bounced = collision == C_TOP;
            switch (collision) {
                case C_NONE:
                case C_LEFT:
                case C_BOTTOM: break;

                case C_TOP:
#if MANIA_USE_PLUS
                    if (player->state == Player_State_MightyHammerDrop) {
                        player->state = Player_State_Air;
                    }
#endif
                    break;

                case C_RIGHT:
                    player->groundVel  = playerGndVel;
                    player->velocity.x = playerVelX;
                    break;

                default: break;
            }

            if (player->velocity.y >= 0 && ((1 << playerID) & this->activePlayers)) {
                Hitbox *playerHitbox = player->GetHitbox();
                player->position.y   = this->position.y - (playerHitbox->bottom << 16) - (hitboxTop << 16);
                if (!bounced)
                    bounced = player->position.x < this->position.x;
            }

            player->flailing = false;
        }

        if (bounced) {
            this->activePlayers |= (1 << playerID);
            if (springPos >= 8 && !this->bounceDelay)
                this->bounceDelay = 6;
        }
        else {
            this->activePlayers &= ~(1 << playerID);
        }
    }
}

void Springboard::LateUpdate() {}

void Springboard::StaticUpdate() {}

void Springboard::Draw()
{
    this->animator.DrawSprite(nullptr, false);
}

void Springboard::Create(void *data)
{
    this->visible   = true;
    this->drawGroup = Zone::sVars->objectDrawGroup[0];
    this->drawFX |= FX_FLIP;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x400000;
    this->updateRange.y = 0x400000;

    if (!sceneInfo->inEditor) {
        this->force = (this->force + 8) << 15;
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 3);
    }
}

void Springboard::StageLoad()
{
    sVars->aniFrames.Load("ARZ/Springboard.bin", SCOPE_STAGE);
                    
    sVars->sfxSpring.Get("Global/Spring.wav");

    DebugMode::AddObject(sVars->classID, &Springboard::DebugSpawn, &Springboard::DebugDraw);
}

void Springboard::DebugSpawn()
{
    GameObject::Create<Springboard>(nullptr, this->position.x, this->position.y);
}
void Springboard::DebugDraw(void)
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

#if RETRO_INCLUDE_EDITOR
void Springboard::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, false, 3);

    Springboard::Draw();
}

void Springboard::EditorLoad()
{
    sVars->aniFrames.Load("ARZ/Springboard.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Right");
    RSDK_ENUM_VAR("Left");
}
#endif

#if RETRO_REV0U
void Springboard::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Springboard);

    int32 forces[] = { -0x40000, -0xA0000, -0x80000 };
    memcpy(sVars->forces, forces, sizeof(forces));

    int32 springPower[] = { 0x00000,  0x00000,  0x00000,  0x00000,  0x00000,  0x00000,  0x00000,  0x00000,  -0x10000, -0x10000,
                            -0x10000, -0x10000, -0x10000, -0x10000, -0x10000, -0x10000, -0x10000, -0x10000, -0x10000, -0x10000,
                            -0x10000, -0x10000, -0x10000, -0x10000, -0x20000, -0x20000, -0x20000, -0x20000, -0x20000, -0x20000,
                            -0x20000, -0x20000, -0x30000, -0x30000, -0x30000, -0x30000, -0x30000, -0x30000, -0x40000, -0x40000 };
    memcpy(sVars->springPower, springPower, sizeof(springPower));

    int32 heightsReady[] = { 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 17, 18, 19, 20, 20, 21, 21, 22, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24 };
    memcpy(sVars->heightsReady, heightsReady, sizeof(heightsReady));

    int32 heightsFlat[] = { 8, 9, 10, 11, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 14, 14, 15, 15, 16, 16, 16, 16, 15, 15, 14, 14, 13, 13 };
    memcpy(sVars->heightsFlat, heightsFlat, sizeof(heightsFlat));
}
#endif

void Springboard::Serialize()
{
    RSDK_EDITABLE_VAR(Springboard, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(Springboard, VAR_ENUM, force);
}

} // namespace GameLogic