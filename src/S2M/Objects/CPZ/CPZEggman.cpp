// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: CPZEggman Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "CPZEggman.hpp"
#include "ChemicalDropper.hpp"
#include "Global/Zone.hpp"
#include "Global/Player.hpp"
#include "Global/Explosion.hpp"
#include "Global/Music.hpp"
#include "Helpers/BadnikHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(CPZEggman);

void CPZEggman::Update()
{
    if (this->health != 0) {
        if (this->invincibilityTimer > 0) {
            this->invincibilityTimer--;
            int32 invincible = GET_BIT(this->invincibilityTimer, 0);
            if (invincible) { // Palette flashing when hit
                paletteBank[0].SetEntry(189, 0xE0E0E0);
            }
            else {
                paletteBank[0].SetEntry(189, 0x000000);
            }
        }

        for (auto currentPlayer : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (this->invincibilityTimer == 0) { // Invulnerable?
                if (currentPlayer->animator.animationID == Player::ANI_HURT || currentPlayer->animator.animationID == Player::ANI_DIE
                    || currentPlayer->animator.animationID == Player::ANI_DROWN) {
                    if (this->eggmanAnimator.animationID != Laugh) {
                        this->eggmanAnimator.SetAnimation(sVars->aniFrames, Laugh, false, 0);
                    }
                }

                if (currentPlayer->CheckBadnikTouch(this, &sVars->hitbox) && currentPlayer->CheckBossHit(this)) {
                    this->health--;
                    if (this->health == 0) {
                        currentPlayer->score += 1000;
                        this->eggmanAnimator.SetAnimation(sVars->aniFrames, Toasted, false, 0);
                        this->flameAnimator.SetAnimation(sVars->aniFrames, Inactive, false, 0);
                        this->exploding          = true;
                        this->originPos.y        = this->position.y;
                        ChemicalDropper *dropper = GameObject::Get<ChemicalDropper>(sceneInfo->entitySlot + 1);
                        this->state.Set(&CPZEggman::State_Explode); // this->state++
                        dropper->state.Set(&ChemicalDropper::State_Destroyed);
                    }
                    else {
                        this->eggmanAnimator.SetAnimation(sVars->aniFrames, Hit, false, 0);
                        this->invincibilityTimer = 32;
                        sVars->bossHitSFX.Play(false, 255);
                    }
                }
            }
        }
    }

    if (this->exploding == true) {
        if ((Zone::sVars->timer & 7) == 0) {
            Vector2 explosionPos;
            explosionPos.x       = this->position.x + Math::Rand(TO_FIXED(-24), TO_FIXED(24));
            explosionPos.y       = this->position.y + Math::Rand(TO_FIXED(-24), TO_FIXED(48));
            Explosion *explosion = GameObject::Create<Explosion>(INT_TO_VOID(Explosion::Type2), explosionPos.x, explosionPos.y);
            explosion->drawGroup = 4;
            explosion->sVars->sfxExplosion.Play(false, 255);
        }
    }

    // if eggman is on the hit or laugh animation, it checks for when the animator reaches the final frame (via the frame count) and sets it back to
    // idle when it gets there
    if (this->eggmanAnimator.animationID == Hit || this->eggmanAnimator.animationID == Laugh) {
        if (this->eggmanAnimator.frameID == this->eggmanAnimator.frameCount - 1) {
            this->eggmanAnimator.SetAnimation(sVars->aniFrames, Idle, false, 0);
        }
        if (this->health == 0) { // way of fixing an issue of him still laughing and getting stuck on the idle animation if he dies while hes in the
                                 // middle of playing it
            this->eggmanAnimator.SetAnimation(sVars->aniFrames, Toasted, false, 0);
        }
    }

    this->state.Run(this);
    this->flameAnimator.Process();
    this->mobileAnimator.Process();
    this->eggmanAnimator.Process();
}

void CPZEggman::LateUpdate() {}
void CPZEggman::StaticUpdate() {}
void CPZEggman::Draw()
{
    this->flameAnimator.DrawSprite(nullptr, false);
    this->seatAnimator.DrawSprite(nullptr, false);
    this->eggmanAnimator.DrawSprite(nullptr, false);
    this->mobileAnimator.DrawSprite(nullptr, false);
}

void CPZEggman::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_BOUNDS;
        this->drawFX    = FX_FLIP;
        this->visible   = true;
        this->drawGroup = 2;
        this->seatAnimator.SetAnimation(sVars->aniFrames, 5, true, 0);
        this->mobileAnimator.SetAnimation(sVars->aniFrames, 6, true, 0);
        this->state.Set(&CPZEggman::State_AwaitPlayer);
    }
}

void CPZEggman::StageLoad()
{
    sVars->aniFrames.Load("Eggman/EggMobile.bin", SCOPE_STAGE);
    sVars->bossHitSFX.Get("Stage/BossHit.wav");
    sVars->hitbox.left   = -24;
    sVars->hitbox.top    = -24;
    sVars->hitbox.right  = 24;
    sVars->hitbox.bottom = 20;

    Zone::AddToHyperList(sVars->classID, true, true, true);
}

void CPZEggman::Oscillate() { this->position.y = BadnikHelpers::Oscillate(this, this->originPos.y, 2, 10); }

void CPZEggman::State_AwaitPlayer()
{
    Zone::sVars->playerBoundActiveL[0] = true;
    Zone::sVars->playerBoundActiveR[0] = true;
    Zone::sVars->cameraBoundsL[0]      = FROM_FIXED(this->position.x) - screenInfo->center.x;
    Zone::sVars->cameraBoundsR[0]      = FROM_FIXED(this->position.x) + screenInfo->center.x;

    this->eggmanAnimator.SetAnimation(sVars->aniFrames, Idle, false, 0);
    Music::ClearMusicStack();
    Music::PlayOnFade(Music::TRACK_EGGMAN2, 0.025f);
    this->originPos.y = this->position.y;

    // object[+1].type = TypeName[Chemical Dropper];
    ChemicalDropper *dropper = GameObject::Get<ChemicalDropper>(sceneInfo->entitySlot + 1);
    GameObject::Reset(sceneInfo->entitySlot + 1, ChemicalDropper::sVars->classID,
                      INT_TO_VOID(true)); // this resets it so the objects creation can be properly started, it does nothing otherwise
    dropper->boundsL = this->position.x - 0x700000;
    dropper->boundsR = this->position.x + 0x700000;
    this->position.x += (screenInfo->center.x + 256) << 16;
    dropper->position.x  = this->position.x;
    dropper->position.y  = this->position.y;
    dropper->originPos.x = this->position.x;
    dropper->originPos.y = this->position.y;
    dropper->main        = dropper;

    this->flameAnimator.SetAnimation(sVars->aniFrames, Active, true, 0);
    this->health = 8;
    this->active = ACTIVE_NORMAL;
    this->state.Set(&CPZEggman::State_BossFight);
}

void CPZEggman::State_BossFight() {}

void CPZEggman::State_Explode()
{
    this->timer++;
    if (this->timer == 180) {
        this->timer = 0;
        this->eggmanAnimator.SetAnimation(sVars->aniFrames, Toasted, true, 0);
        this->exploding = false;
        this->state.Set(&CPZEggman::State_DefeatFall);
        Music::ClearMusicStack();
        Music::PlayOnFade(Music::TRACK_STAGE, 0.025f);
        Vector2 layerSize;
        Zone::sVars->fgLayer[0].Size(&layerSize, true); // gets the layer size of the fg and sets layerSize to it
        Zone::sVars->cameraBoundsR[0] = layerSize.x;
    }
}

void CPZEggman::State_DefeatFall()
{
    this->position.y += this->velocity.y;
    this->velocity.y += 0x1800;
    this->timer++;
    if (this->timer == 38) {
        this->velocity.y = 0;
        this->timer      = 0;
        this->state.Set(&CPZEggman::State_DefeatRise);
    }
}

void CPZEggman::State_DefeatRise()
{
    if (this->timer < 48) {
        this->position.y += this->velocity.y;
        this->velocity.y -= 0x800;
        this->timer++;
    }
    else {
        this->timer       = 0;
        this->velocity.y  = 0;
        this->originPos.y = this->position.y;
        this->state.Set(&CPZEggman::State_Flee);
    }
}

void CPZEggman::State_Flee()
{
    this->position.y = this->originPos.y;
    CPZEggman::Oscillate();

    if (this->timer < 8) {
        this->timer++;
    }
    else {
        this->timer = 0;
        this->eggmanAnimator.SetAnimation(sVars->aniFrames, Toasted, true, 0);
        this->flameAnimator.SetAnimation(sVars->aniFrames, Explode, true, 0);
        this->direction = FLIP_X; // FACING_LEFT in v4
        this->state.Set(&CPZEggman::State_Escape);
    }
}

void CPZEggman::State_Escape()
{
    this->position.x += 0x40000;
    this->originPos.y -= 0x4000;
    CPZEggman::Oscillate();
    if (!this->CheckOnScreen(nullptr)) {
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void CPZEggman::EditorDraw() {}

void CPZEggman::EditorLoad() {}
#endif

void CPZEggman::Serialize() {}

} // namespace GameLogic