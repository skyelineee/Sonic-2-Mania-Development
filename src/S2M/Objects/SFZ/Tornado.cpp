// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Tornado Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Tornado.hpp"
#include "TornadoPath.hpp"
#include "SCZSetup.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Explosion.hpp"
#include "Global/StarPost.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Tornado);

void Tornado::Update()
{
    this->prevPosY = this->position.y;
    this->state.Run(this);

    this->animatorTornado.frameID = this->turnAngle >> 4;

    this->animatorPropeller.Process();
    this->animatorPilot.Process();
    this->animatorFlame.Process();
}

void Tornado::LateUpdate() {}

void Tornado::StaticUpdate() {}

void Tornado::Draw()
{
    this->animatorPilot.DrawSprite(nullptr, false);
    this->animatorPropeller.DrawSprite(nullptr, false);
    this->animatorTornado.DrawSprite(nullptr, false);

    if (this->showFlame) {
        Vector2 drawPos;
        drawPos = this->position;
        drawPos.y += sVars->flameOffsets[this->animatorTornado.frameID];
        this->animatorFlame.DrawSprite(&drawPos, false);
    }

    //if (!MSZSetup->usingRegularPalette) {
    //    Vector2 drawPos;
    //    drawPos.x = this->position.x + this->knuxPos.x;
    //    drawPos.y = this->position.y + this->knuxPos.y;
    //    this->animatorKnux.DrawSprite(&drawPos, false);
    //}
}

void Tornado::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->visible       = true;
        this->updateRange.x = 0x1000000;
        this->updateRange.y = 0x1000000;
        this->drawFX        = FX_FLIP;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->movePos       = this->position;
        this->turnAngle     = 48;
        this->isStood       = true;
        this->offsetX       = 0x80000;
        this->active        = ACTIVE_BOUNDS;
        this->state.Set(&Tornado::State_Init);
        this->knuxPos.x     = -0x140000;
        this->knuxPos.y     = -0x160000;

        this->animatorTornado.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->animatorPropeller.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->animatorFlame.SetAnimation(sVars->aniFrames, 2, true, 0);

        if (CHECK_CHARACTER_ID(ID_TAILS, 1)
        ) {
            this->animatorPilot.SetAnimation(sVars->aniFrames, 3, true, 0);
        }
        else {
            this->animatorPilot.SetAnimation(sVars->aniFrames, 6, true, 0);
        }

        Player *player2 = GameObject::Get<Player>(SLOT_PLAYER2);
        if (player2->classID == Player::sVars->classID)
            player2->state.Set(&SCZSetup::PlayerState_Pilot);
    }
}

void Tornado::StageLoad()
{
    sVars->aniFrames.Load("SCZ/Tornado.bin", SCOPE_STAGE);

    sVars->sfxExplosion.Get("Stage/Explosion2.wav");
    sVars->sfxImpact.Get("Stage/Impact5.wav");
}

void Tornado::State_Init()
{
    this->active = ACTIVE_NORMAL;

    this->state.Set(&Tornado::State_SetupControl);
    this->state.Run(this);
}

void Tornado::State_SetupControl()
{
    Tornado::HandlePlayerCollisions();

    this->state.Set(&Tornado::State_PlayerControlled);
}

void Tornado::HandlePlayerCollisions()
{
    Player *player1       = GameObject::Get<Player>(SLOT_PLAYER1);
    Hitbox *hitbox        = this->animatorTornado.GetHitbox(0);

    this->prevPosY     = this->position.y;
    this->turnAngle    = 32;
    player1->drawGroup = this->drawGroup + 1;
    this->prevPosY &= 0xFFFF0000;
    this->moveVelocityY = (this->position.y & 0xFFFF0000) - this->prevPosY;
    this->position.y    = this->prevPosY;
    this->isStood       = false;

    int32 velY = player1->velocity.y;
    if (player1->CheckCollisionPlatform(this, hitbox)) {
        player1->position.x += TornadoPath::sVars->moveVel.x;
        player1->position.y += this->moveVelocityY;
        player1->flailing = false;
        this->isStood     = true;

        if (velY > 0x10000) {
            this->collideTimer = 0;
            this->gravityForce = 0x20000;
            this->mode         = TORNADO_MODE_LAND;
        }
    }

    Camera *camera = TornadoPath::sVars->camera;
    if (camera) {
        int32 screenX = camera->position.x - (screenInfo->center.x << 16) + 0xC0000;
        if (player1->position.x < screenX)
            player1->position.x = screenX;

        int32 screenY = ((screenInfo->center.x - 12) << 16) + camera->position.x;
        if (player1->position.x > screenY)
            player1->position.x = screenY;

        if (player1->classID == Player::sVars->classID) {
            int32 deathBounds = (camera->position.y + ((screenInfo[camera->screenID].center.y + 16) << 16));
            if (player1->position.y > deathBounds)
                player1->deathType = Player::DeathDie_Sfx;
        }
    }
}

void Tornado::State_PlayerControlled()
{
    Player *player1       = GameObject::Get<Player>(SLOT_PLAYER1);
    Hitbox *hitbox        = this->animatorTornado.GetHitbox(TornadoPath::sVars->hitboxID);

    this->prevPosY     = this->position.y;
    player1->drawGroup = this->turnAngle < 0x20 ? this->drawGroup : (this->drawGroup + 1);

    if (player1->state.Matches(&Player::State_Roll)) {
        player1->groundVel  = CLAMP(player1->groundVel, -this->offsetX, this->offsetX);
        player1->velocity.x = player1->groundVel;
    }

    switch (this->mode) {
        case TORNADO_MODE_IDLE:
            if (player1->velocity.y < 0 && player1->jumpPress) {
                if (abs(TornadoPath::sVars->moveVel.x) > 0x40000) {
                    if (abs(player1->velocity.x) < abs(TornadoPath::sVars->moveVel.x) && !TornadoPath::sVars->hitboxID)
                        player1->velocity.x = abs(TornadoPath::sVars->moveVel.x);
                }

                ++this->mode;
                this->gravityForce = 0x20000;
                this->storeY       = this->position.y;
            }
            else {
                if (this->velocity.y > 0) {
                    if (this->turnAngle < 0x40)
                        this->turnAngle += 4;
                }
                else if (this->velocity.y >= 0) {
                    if (this->turnAngle > 0x20)
                        this->turnAngle -= 4;
                    else if (this->turnAngle < 0x20)
                        this->turnAngle += 4;
                }
                else if (this->turnAngle > 0)
                    this->turnAngle -= 4;
            }
            break;

        case TORNADO_MODE_JUMP_RECOIL: // jump force pushes tornado downwards
            this->gravityForce -= 0x2000;
            this->position.y += this->gravityForce;

            if (this->gravityForce < 0)
                this->mode = TORNADO_MODE_JUMP_REBOUND;
            break;

        case TORNADO_MODE_JUMP_REBOUND: // tornado returns to its original position
            this->gravityForce -= 0x1000;
            this->position.y += this->gravityForce;

            if (this->position.y <= this->storeY) {
                this->position.y   = this->storeY;
                this->mode         = TORNADO_MODE_IDLE;
                this->gravityForce = 0;
            }
            break;

        case TORNADO_MODE_LAND:
            this->gravityForce -= 0x2000;
            this->position.y += this->gravityForce;

            if (++this->collideTimer == 24) {
                this->mode         = TORNADO_MODE_IDLE;
                this->gravityForce = 0;
            }
            break;

        default: break;
    }

    if (this->isStood) {
        this->position.x += TornadoPath::sVars->moveVel.x;
        this->position.y += TornadoPath::sVars->moveVel.y;

        if (!player1->stateInput.Matches(nullptr)) {
            this->velocity.y = 0;

            if (player1->up)
                this->velocity.y = -0x10000;
            else if (player1->down)
                this->velocity.y = 0x10000;
        }
    }
    else if (TornadoPath::sVars->hitboxID == 1) {
        this->position.x += TornadoPath::sVars->moveVel.x;
        this->position.y += TornadoPath::sVars->moveVel.y;
    }
    else {
        this->velocity.y = 0;
    }

    if (this->position.y < (screenInfo->position.y + 72) << 16)
        this->position.y = (screenInfo->position.y + 72) << 16;
    if (this->position.y > (screenInfo->size.y + screenInfo->position.y - 32) << 16)
        this->position.y = (screenInfo->size.y + screenInfo->position.y - 32) << 16;

    int32 storeX  = this->position.x;
    int32 storeY  = this->position.y + this->velocity.y;
    this->isStood = false;
    this->prevPosY &= 0xFFFF0000;
    this->moveVelocityY = (storeY & 0xFFFF0000) - this->prevPosY;
    this->position.y    = this->prevPosY;
    int32 velY          = player1->velocity.y;
    int32 posX          = this->position.x;

    if (player1->CheckCollisionPlatform(this, hitbox)) {
        player1->position.x += TornadoPath::sVars->moveVel.x;
        player1->position.y += this->moveVelocityY;
        player1->flailing = false;
        this->isStood     = true;

        if (velY > 0x10000) {
            this->collideTimer = 0;
            this->gravityForce = 0x20000;
            this->mode         = TORNADO_MODE_LAND;
        }
    }
    else if (TornadoPath::sVars->hitboxID == 1) {
        player1->position.x += TornadoPath::sVars->moveVel.x;
    }

    int32 offsetX = 0;
    this->position.x += 0x1E0000;

    if (abs(posX + 0x1E0000 - player1->position.x) > 0x100000) {
        offsetX = this->offsetX;

        if (player1->position.x <= posX + 0x1E0000) {
            offsetX   = -offsetX;
            int32 pos = player1->position.x - (posX + 0x1E0000) + 0x100000;
            if (pos > offsetX)
                offsetX = pos;
        }
        else {
            int32 pos = player1->position.x - (posX + 0x1E0000) - 0x100000;
            if (pos < offsetX)
                offsetX = pos;
        }
    }

    this->position.x = storeX + offsetX;
    this->position.y = storeY;

    Camera *camera = TornadoPath::sVars->camera;
    if (camera) {
        int32 screenX = camera->position.x - (screenInfo->center.x << 16) + 0xC0000;
        if (player1->position.x < screenX)
            player1->position.x = screenX;

        int32 screenY = ((screenInfo->center.x - 12) << 16) + camera->position.x;
        if (player1->position.x > screenY)
            player1->position.x = screenY;

        if (player1->classID == Player::sVars->classID) {
            if (player1->position.y > (camera->position.y + ((screenInfo[camera->screenID].center.y + 16) << 16)))
                player1->deathType = Player::DeathDie_Sfx;
        }
    }

    if (player1->state.Matches(&Player::State_TailsFlight)) {
        if (player1->position.y < ((screenInfo->position.y + 20) << 16) && player1->velocity.y < 0) {
            player1->velocity.y   = 0;
            player1->abilitySpeed = 0x8000;
        }
    }
}

void Tornado::State_Mayday()
{
    this->position.x += TornadoPath::sVars->moveVel.x;
    this->position.y += TornadoPath::sVars->moveVel.y;

    if (!(Zone::sVars->timer % 3)) {
        if (this->onGround)
            sVars->sfxExplosion.Play(false, 255);

        if (Zone::sVars->timer & 4) {
            Explosion *explosion = GameObject::Create<Explosion>(INT_TO_VOID((Math::Rand(0, 256) > 192) + 2),
                                                       (Math::Rand(-32, 32) << 16) + this->position.x, (Math::Rand(-16, 16) << 16) + this->position.y);
            explosion->drawGroup       = Zone::sVars->objectDrawGroup[1];
        }
    }
}

void Tornado::State_FlyAway_Right()
{
    this->position.x += 0x30000;
    this->position.y -= 0x30000;

    this->active = ACTIVE_BOUNDS;
}

void Tornado::State_FlyAway_Left()
{
    this->velocity.x -= 0x1800;
    this->position.x += this->velocity.x;
    this->position.y -= 0x8000;

    this->active = ACTIVE_BOUNDS;
}

#if RETRO_INCLUDE_EDITOR
void Tornado::EditorDraw()
{
    this->animatorTornado.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->animatorPropeller.SetAnimation(sVars->aniFrames, 1, true, 0);
    this->animatorFlame.SetAnimation(sVars->aniFrames, 2, true, 0);

    this->animatorPilot.DrawSprite(nullptr, false);
    this->animatorPropeller.DrawSprite(nullptr, false);
    this->animatorTornado.DrawSprite(nullptr, false);
}

#if RETRO_REV0U
void Tornado::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Tornado);

    int32 flameOffsets[] = { -0x20000, -0x10000, 0, 0x10000, 0x20000 };

    memcpy(sVars->flameOffsets, flameOffsets, sizeof(flameOffsets));
}
#endif

void Tornado::EditorLoad() { sVars->aniFrames.Load("SCZ/Tornado.bin", SCOPE_STAGE); }
#endif

void Tornado::Serialize() {}

} // namespace GameLogic