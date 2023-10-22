// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TubeSpring Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "TubeSpring.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Common/ScreenWrap.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TubeSpring);

void TubeSpring::Update()
{
    if (this->sfxTimer > 0)
        this->sfxTimer--;

    if (this->timer > 0)
        this->timer--;

    this->state.Run(this);

    this->animator.Process();
}

void TubeSpring::LateUpdate() {}

void TubeSpring::StaticUpdate() {}

void TubeSpring::Draw() { this->animator.DrawSprite(nullptr, false); }

void TubeSpring::Create(void *data)
{
    // ig this kinda works????
    Spring *spring = (Spring *)this;
    spring->Create(nullptr);
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->drawGroup      = Zone::sVars->objectDrawGroup[1];
    this->velocity.y     = !this->type ? -0x100000 : -0xA8000;
    this->type           = Spring::TubeSpring;
    this->animator.speed = 0;
    this->state.Set(&TubeSpring::State_Idle);
}

void TubeSpring::StageLoad()
{
    sVars->aniFrames.Load("CPZ/TubeSpring.bin", SCOPE_STAGE);

    sVars->sfxSpring.Get("Global/Spring.wav");
    sVars->sfxExit.Get("Tube/Exit.wav");

    sVars->hitbox.left   = -16;
    sVars->hitbox.top    = 12;
    sVars->hitbox.right  = 16;
    sVars->hitbox.bottom = 48;
}

// needed to port this state from the spring object cause i cant call it the same way you could in C lol
void TubeSpring::Spring_State_Vertical()
{
    SET_CURRENT_STATE();

    if (this->direction == FLIP_NONE) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if ((!this->planeFilter || player->collisionPlane == ((uint8)(this->planeFilter - 1) & 1)) && this->interaction) {
                int32 side = ((this->type == Spring::TubeSpring && player->velocity.y < -0x50000) || this->passThrough)
                                 ? player->CheckCollisionPlatform(this, &this->hitbox)
                                 : player->CheckCollisionBox(this, &this->hitbox);

                bool32 collided = side == C_TOP;

                if (!collided && player->onGround) {
                    if (player->velocity.y >= 0) {
                        Hitbox hitbox;
                        hitbox.left   = this->hitbox.left;
                        hitbox.top    = this->hitbox.top - 8;
                        hitbox.right  = this->hitbox.right;
                        hitbox.bottom = this->hitbox.top;
                        if (player->CheckCollisionTouch(this, &hitbox)) {
                            collided = true;
                        }
                    }
                }

                if (collided) {
                    int32 anim = player->animator.animationID;

                    player->animationReserve = Player::ANI_WALK;
                    if (anim > Player::ANI_WALK && anim <= Player::ANI_DASH)
                        player->animationReserve = player->animator.animationID;

                    if (player->state.Matches(&Player::State_TubeAirRoll) || player->state.Matches(&Player::State_TubeRoll)) {
                        player->state.Set(&Player::State_TubeAirRoll);
                    }
                    else {
                        if (anim == Player::ANI_JUMP)
                            player->position.y -= player->jumpOffset;

                        if (!globals->gravityDir) {
                            if (this->twirl) {
                                if (player->spriteType) {
                                    player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                                }
                                else {
                                    player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, true, 0);
                                }
                            }
                            else {
                                player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                            }
                        }

                        player->state.Set(&Player::State_Air);
                    }

                    player->onGround       = false;
                    player->velocity.y     = this->velocity.y;
                    player->tileCollisions = globals->tileCollisionMode;
                    player->rotation       = (globals->tileCollisionMode != 1) << 8;

                    this->animator.speed   = 0x80;
                    this->animator.timer   = 0;
                    this->animator.frameID = 1;

                    if (!this->sfxTimer) {
                        sVars->sfxSpring.Play();
                        this->sfxTimer = 8;
                    }

                    if (this->swap)
                        player->collisionPlane = this->swap - 1;
                }
            }
        }
    }
    else {
        if (globals->gravityDir == CMODE_ROOF) {

            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                if ((!this->planeFilter || player->collisionPlane == ((uint8)(this->planeFilter - 1) & 1)) && this->interaction) {
                    int32 side = ((this->type == Spring::TubeSpring && player->velocity.y > 0x50000) || this->passThrough)
                                     ? (player->CheckCollisionPlatform(this, &this->hitbox) * C_BOTTOM)
                                     : player->CheckCollisionBox(this, &this->hitbox);

                    bool32 collided = side == C_BOTTOM;

                    if (!collided && player->onGround) {
                        if (player->velocity.y <= 0) {
                            Hitbox hitbox;
                            hitbox.left   = this->hitbox.left;
                            hitbox.top    = this->hitbox.bottom;
                            hitbox.right  = this->hitbox.right;
                            hitbox.bottom = this->hitbox.bottom + 8;
                            if (player->CheckCollisionTouch(this, &hitbox)) {
                                collided = true;
                            }
                        }
                    }

                    if (collided) {
                        int32 anim = player->animator.animationID;

                        player->animationReserve = Player::ANI_WALK;
                        if (anim > Player::ANI_WALK && anim <= Player::ANI_DASH)
                            player->animationReserve = player->animator.animationID;

                        if (player->state.Matches(&Player::State_TubeAirRoll) || player->state.Matches(&Player::State_TubeRoll)) {
                            player->state.Set(&Player::State_TubeAirRoll);
                        }
                        else {
                            if (anim == Player::ANI_JUMP)
                                player->position.y -= player->jumpOffset;

                            if (!globals->gravityDir) {
                                if (this->twirl) {
                                    if (player->spriteType) {
                                        player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                                    }
                                    else {
                                        player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, true, 0);
                                    }
                                }
                                else {
                                    player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING, true, 0);
                                }
                            }

                            player->state.Set(&Player::State_Air);
                        }

                        player->applyJumpCap   = 0;
                        player->onGround       = false;
                        player->velocity.y     = this->velocity.y;
                        player->tileCollisions = globals->tileCollisionMode;
                        player->rotation       = (globals->tileCollisionMode != 1) << 8;

                        this->animator.speed   = 0x80;
                        this->animator.timer   = 0;
                        this->animator.frameID = 1;

                        if (!this->sfxTimer) {
                            sVars->sfxSpring.Play();
                            this->sfxTimer = 8;
                        }

                        if (this->swap)
                            player->collisionPlane = this->swap - 1;
                    }
                }
            }
        }
        else {
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                if ((!this->planeFilter || player->collisionPlane == ((uint8)(this->planeFilter - 1) & 1)) && this->interaction) {
                    if (player->CheckCollisionBox(this, &this->hitbox) == C_BOTTOM) {

                        if (player->animator.animationID == Player::ANI_JUMP && globals->gravityDir == 2)
                            player->position.y += player->jumpOffset;

                        if (player->state.Matches(&Player::State_TubeAirRoll) || player->state.Matches(&Player::State_TubeRoll)) {
                            player->state.Set(&Player::State_TubeAirRoll);
                        }
                        else {
                            player->state.Set(&Player::State_Air);
                        }

                        player->applyJumpCap   = 0;
                        player->onGround       = false;
                        player->velocity.y     = this->velocity.y;
                        player->tileCollisions = globals->tileCollisionMode;
                        player->rotation       = (globals->tileCollisionMode != 1) << 8;

                        this->animator.speed   = 0x80;
                        this->animator.timer   = 0;
                        this->animator.frameID = 1;

                        if (!this->sfxTimer) {
                            sVars->sfxSpring.Play();
                            this->sfxTimer = 8;
                        }

                        if (this->swap)
                            player->collisionPlane = this->swap - 1;
                    }
                }
            }
        }
    }

    ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Spring::State_Vertical), true);
}

void TubeSpring::State_Idle() { TubeSpring::HandleInteractions(true); }

void TubeSpring::State_Springing()
{
    if (!TubeSpring::HandleInteractions(false) && this->animator.frameID == this->animator.frameCount - 1) {
        this->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
        this->state.Set(&TubeSpring::State_Pullback);
        TubeSpring::State_Pullback();
    }
}
void TubeSpring::State_Pullback()
{
    TubeSpring::HandleInteractions(false);

    if (this->animator.frameID == this->animator.frameCount - 1) {
        this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->state.Set(&TubeSpring::State_Idle);
    }
}

bool32 TubeSpring::HandleInteractions(bool32 setState)
{
    TubeSpring::Spring_State_Vertical();

    bool32 sprung = false;
    for (int32 i = 0; i < Player::sVars->playerCount; i++) {
        Player *player = GameObject::Get<Player>(i);

        if (player->CheckValidState()) {
            bool32 interactStore = player->interaction;
            player->interaction  = true;
            if (!player->CheckCollisionTouch(this, &sVars->hitbox)) {
                player->interaction = interactStore;
                continue;
            }

            if (setState) {
                this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);
                sVars->sfxExit.Play(false, 255);
                this->state.Set(&TubeSpring::State_Springing);
            }

            sprung = true;
            if (player->state.Matches(&Player::State_Static) || player->state.Matches(&Player::State_TransportTube)) {
                player->drawGroup      = Zone::sVars->playerDrawGroup[0];
                player->tileCollisions = TILECOLLISION_DOWN;
                player->interaction    = true;
                player->applyJumpCap   = false;
                player->state.Set(&Player::State_Air);
                if (player->velocity.y < -0x80000)
                    player->velocity.y = -0x80000;
            }
        }
    }

    return sprung;
}

#if RETRO_INCLUDE_EDITOR
void TubeSpring::EditorDraw()
{
    Spring *spring = (Spring *)this;
    spring->Create(nullptr);
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    TubeSpring::Draw();
}

void TubeSpring::EditorLoad()
{
    sVars->aniFrames.Load("CPZ/TubeSpring.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Strong", SPRING_VERT_YELLOW);
    RSDK_ENUM_VAR("Weak", SPRING_VERT_RED);

    // technically unused, FX_FLIP isn't ever set on the animator
    RSDK_ACTIVE_VAR(sVars, flipFlag);
    RSDK_ENUM_VAR("No Flip", FLIP_NONE);
    RSDK_ENUM_VAR("Flip X", FLIP_X);
    RSDK_ENUM_VAR("Flip Y", FLIP_Y);
    RSDK_ENUM_VAR("Flip XY", FLIP_XY);
}
#endif

void TubeSpring::Serialize()
{
    RSDK_EDITABLE_VAR(TubeSpring, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(TubeSpring, VAR_ENUM, flipFlag);
}
} // namespace GameLogic