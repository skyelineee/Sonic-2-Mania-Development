// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: OOZSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "OOZSetup.hpp"
#include "Global/Player.hpp"
#include "Global/Animals.hpp"
#include "Global/Ring.hpp"
#include "Global/Explosion.hpp"
#include "Global/Zone.hpp"
#include "Helpers/Soundboard.hpp"
#include "Helpers/RPCHelpers.hpp"
#include "HTZ/Sol.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(OOZSetup);

void OOZSetup::Update() {}

void OOZSetup::LateUpdate() {}

void OOZSetup::StaticUpdate()
{
    for (auto setup : GameObject::GetEntities<OOZSetup>(FOR_ALL_ENTITIES)) {
        RSDKTable->AddDrawListRef(Zone::sVars->playerDrawGroup[0] + 1, RSDKTable->GetEntitySlot(setup));
    }

    sVars->palTimer += 128;
    if (sVars->palTimer >= 256) {
        sVars->palTimer -= 256;

        paletteBank[3].Rotate(216, 223, false);
        paletteBank[4].Rotate(216, 223, false);
        paletteBank[5].Rotate(216, 223, false);
        paletteBank[6].Rotate(216, 223, false);
    }

    paletteBank[1].SetLimitedFade(3, 4, sVars->palTimer, 216, 223);
    paletteBank[2].SetLimitedFade(5, 6, sVars->palTimer, 216, 223);
    paletteBank[0].Copy(1, 216, 216, 8);

    if (!(Zone::sVars->timer & 3)) {
        ++sVars->background1->deformationOffset;

        sVars->oilAniTimer += 16;
        sVars->oilAniTimer %= 160;

        RSDKTable->DrawAniTiles(sVars->aniTiles, 760, 0, sVars->oilAniTimer, 128, 16);
        RSDKTable->DrawAniTiles(sVars->aniTiles, 757, 136, sVars->oilAniTimer, 16, 16);
        RSDKTable->DrawAniTiles(sVars->aniTiles, 758, 128, sVars->oilAniTimer, 32, 16);
    }

    if (--sVars->glowLightAniTimer <= 0) {
        ++sVars->glowLightAniFrame;

        sVars->glowLightAniFrame &= 7;
        sVars->glowLightAniTimer = sVars->glowLightAniDurations[sVars->glowLightAniFrame];

        RSDKTable->DrawAniTiles(sVars->aniTiles, 756, 16 * sVars->glowLightAniFrame, 160, 16, 16);
    }

    sVars->swimmingPlayerCount = 0;
    OOZSetup *setup            = GameObject::Get<OOZSetup>(sceneInfo->entitySlot);
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 playerID = RSDKTable->GetEntitySlot(player);
        if (!player->state.Matches(&Player::State_Static)) {
            Hitbox *playerHitbox = player->GetHitbox();
            Tile tile =
                Zone::sVars->fgLayer[0].GetTile(player->position.x >> 20, ((playerHitbox->bottom << 16) + player->position.y - 0x10000) >> 20);
            if (tile.id == (uint16)-1)
                tile = Zone::sVars->fgLayer[1].GetTile(player->position.x >> 20, ((playerHitbox->bottom << 16) + player->position.y - 0x10000) >> 20);

            int32 tileFlags = tile.GetFlags(player->collisionPlane);
            if (tileFlags != OOZ_TFLAGS_NORMAL) {
                if (player->shield == Player::Shield_Fire && player->superState != Player::SuperStateSuper && tileFlags != OOZ_TFLAGS_OILFALL) {
                    int32 tx = (player->position.x & 0xFFF00000) + 0x70000;
                    int32 ty = player->position.y + ((playerHitbox->bottom + 8) << 16);
                    if (tileFlags == OOZ_TFLAGS_OILPOOL) {
                        ty &= 0xFFF00000;
                        ty -= 0xC0000;
                        if (setup->StartFire(tx, ty, player->angle)) {
                            Sol *sol        = GameObject::Create<Sol>(INT_TO_VOID(true), tx - 0x10000, ty);
                            sol->velocity.x = -0x40000;
                            sol->mainAnimator.SetAnimation(Sol::sVars->aniFrames, 3, true, 0);
                            sol->state.Set(&Sol::State_OilFlame);

                            sol             = GameObject::Create<Sol>(INT_TO_VOID(true), tx + 0x10000, ty);
                            sol->velocity.x = 0x40000;
                            sol->mainAnimator.SetAnimation(Sol::sVars->aniFrames, 3, true, 0);
                            sol->state.Set(&Sol::State_OilFlame);
                        }
                    }
                    else if (player->onGround) {
                        ty &= 0xFFFF0000;
                        if (setup->StartFire(tx, ty, player->angle)) {
                            ty -= 0x80000;
                            Sol *sol        = GameObject::Create<Sol>(INT_TO_VOID(true), tx - 0x10000, ty);
                            sol->velocity.x = -0x40000;
                            sol->mainAnimator.SetAnimation(Sol::sVars->aniFrames, 3, true, 0);
                            sol->state.Set(&Sol::State_FireballOilFlame);

                            sol             = GameObject::Create<Sol>(INT_TO_VOID(true), tx + 0x10000, ty);
                            sol->velocity.x = 0x40000;
                            sol->mainAnimator.SetAnimation(Sol::sVars->aniFrames, 3, true, 0);
                            sol->state.Set(&Sol::State_FireballOilFlame);
                        }
                    }
                }

                switch (tileFlags) {
                    case OOZ_TFLAGS_NORMAL:
                    default: sVars->activePlayers &= ~(1 << playerID); break;

                    case OOZ_TFLAGS_OILPOOL:
                        if (!player->sidekick)
                            sVars->swimmingPlayerCount++;
                        sVars->activePlayers &= ~(1 << playerID);
                        if (player->velocity.y < 0) {
                            player->velocity.y += 0x3800;
                        }
                        else {
                            player->interaction    = true;
                            player->tileCollisions = TILECOLLISION_DOWN;
                            player->state.Set(&OOZSetup::PlayerState_OilPool);
                        }
                        break;

                    case OOZ_TFLAGS_OILSTRIP:
                        if (!player->state.Matches(&Player::State_BubbleBounce)) {
                            sVars->activePlayers &= ~(1 << playerID);
                            if (player->onGround) {
                                player->interaction    = true;
                                player->tileCollisions = TILECOLLISION_DOWN;
                                player->state.Set(&OOZSetup::PlayerState_OilStrip);
                            }
                        }
                        break;

                    case OOZ_TFLAGS_OILSLIDE:
                        if (!player->state.Matches(&Player::State_BubbleBounce)) {
                            sVars->activePlayers &= ~(1 << playerID);
                            if (player->onGround) {
                                player->interaction    = true;
                                player->tileCollisions = TILECOLLISION_DOWN;
                                if (!player->angle)
                                    player->state.Set(&OOZSetup::PlayerState_OilStrip);
                                else
                                    player->state.Set(&OOZSetup::PlayerState_OilSlide);
                            }
                        }
                        break;

                    case OOZ_TFLAGS_OILFALL:
                        if (!player->sidekick)
                            sVars->swimmingPlayerCount++;

                        sVars->activePlayers |= 1 << playerID;
                        player->interaction    = true;
                        player->tileCollisions = TILECOLLISION_DOWN;
                        if (player->velocity.y < 0)
                            player->velocity.y += 0xC000;
                        else
                            player->state.Set(&OOZSetup::PlayerState_OilFall);
                        break;
                }
            }
            else {
                sVars->activePlayers &= ~(1 << playerID);
            }
        }
    }

    if (sVars->useSmogEffect && sceneInfo->timeEnabled) {
        if (sVars->smogTimer < 0x10000)
            sVars->smogTimer++;
    }
    else {
        if (sVars->smogTimer > 0)
            sVars->smogTimer -= 32;
    }

    for (auto ring : GameObject::GetEntities<Ring>(FOR_ACTIVE_ENTITIES)) {
        if (ring->state.Matches(&Ring::State_Lost)) {
            Tile tile = Zone::sVars->fgLayer[0].GetTile(ring->position.x >> 20, (ring->position.y + 0xE0000) >> 20);
            if (tile.id == (uint16)-1)
                tile = Zone::sVars->fgLayer[1].GetTile(ring->position.x >> 20, (ring->position.y + 0xE0000) >> 20);

            if (tile.GetFlags(ring->collisionPlane) == OOZ_TFLAGS_OILPOOL) {
                ring->velocity.x -= ring->velocity.x >> 4;
                ring->velocity.y = 0x2800;
                ring->drawGroup  = Zone::sVars->objectDrawGroup[1];
                if (ring->alpha > 0x40) {
                    ring->alpha     = 0x40;
                    ring->inkEffect = INK_ALPHA;
                }
            }
        }
    }

    setup->HandleActiveFlames();

    sVars->animator.Process();
}

void OOZSetup::Draw()
{
    if (sceneInfo->currentDrawGroup != this->drawGroup) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if ((1 << RSDKTable->GetEntitySlot(player)) & sVars->activePlayers)
                sVars->animator.DrawSprite(&player->position, false);
        }
    }

    if (this->type)
        Graphics::FillScreen(0xC0C0E8, sVars->smogTimer >> 5, sVars->smogTimer >> 5, sVars->smogTimer >> 4);
    else
        OOZSetup::Draw_Flames();
}

void OOZSetup::Create(void *data)
{
    this->active    = ACTIVE_ALWAYS;
    this->visible   = true;
    this->drawFX    = FX_ROTATE;
    this->drawGroup = this->type ? 14 : Zone::sVars->objectDrawGroup[0];
}

void OOZSetup::StageLoad()
{
    sVars->aniTiles = RSDKTable->LoadSpriteSheet("OOZ/AniTiles.gif", SCOPE_STAGE);

    sVars->background1 = RSDKTable->GetTileLayer(0);
    for (int32 i = 0; i < 0x400; ++i) {
        sVars->background1->deformationData[i] = sVars->deformData[i & 0x3F];
    }

    sVars->smogTimer           = 0;
    sVars->useSmogEffect       = true;
    sVars->swimmingPlayerCount = 0;

    Animals::sVars->animalTypes[0] = Animals::Pocky;
    Animals::sVars->animalTypes[1] = Animals::Pecky;

    memset(sVars->flameTimers, 0, sizeof(sVars->flameTimers));
    memset(sVars->flameTimerPtrs, 0, sizeof(sVars->flameTimerPtrs));
    sVars->flameCount = 0;

    sVars->solFrames.Load("OOZ/Sol.bin", SCOPE_STAGE);
    sVars->flameAnimator.SetAnimation(sVars->solFrames, 3, true, 0);

    sVars->splashFrames.Load("OOZ/Splash.bin", SCOPE_STAGE);
    sVars->animator.SetAnimation(sVars->splashFrames, 0, true, 0);

    if (Stage::CheckSceneFolder("OOZ")) {
        if (globals->gameMode != MODE_TIMEATTACK) {
            const char *playingAsText  = "";
            const char *characterImage = "";
            const char *characterText  = "";
            switch (GET_CHARACTER_ID(1)) {
                case ID_SONIC:
                    playingAsText  = "Playing as Sonic";
                    characterImage = "sonic";
                    characterText  = "Sonic";
                    break;
                case ID_TAILS:
                    playingAsText  = "Playing as Tails";
                    characterImage = "tails";
                    characterText  = "Tails";
                    break;
                case ID_KNUCKLES:
                    playingAsText  = "Playing as Knuckles";
                    characterImage = "knuckles";
                    characterText  = "Knuckles";
                    break;
            }
            SetPresence(playingAsText, "In Oil Ocean", "ooz", "Oil Ocean", characterImage, characterText);
        }
    }

    // oh my god i finally learned how to actually use soundboard
    int32 sfxID = Soundboard::LoadSfx(OOZSetup::SfxCheck_Slide, nullptr);
    if (sfxID >= 0)
        Soundboard::sVars->sfxFadeOutDuration[sfxID] = 30;

    sfxID = Soundboard::LoadSfx(OOZSetup::SfxCheck_OilSwim, nullptr);
    if (sfxID >= 0)
        Soundboard::sVars->sfxFadeOutDuration[sfxID] = 30;

    sfxID = Soundboard::LoadSfx(OOZSetup::SfxCheck_Flame2, nullptr);
    if (sfxID >= 0)
        Soundboard::sVars->sfxFadeOutDuration[sfxID] = 30;
}

Soundboard::SoundInfo OOZSetup::SfxCheck_Flame2()
{
    int32 count = 0;

    for (auto sol : GameObject::GetEntities<Sol>(FOR_ACTIVE_ENTITIES)) {
        if (sol->isFlameFX)
            count++;
    }

    for (int32 i = 0; i < sVars->flameCount; ++i) {
        if (sVars->flameTimerPtrs[i])
            count++;
    }

    // return count > 0;
    SoundFX flameSFX;
    flameSFX.Get("Stage/Flame2.wav");
    Soundboard::SoundInfo info = {};
    info.playFlags             = count > 0;
    info.sfx                   = flameSFX;
    info.loopPoint             = true;

    return info;
}

Soundboard::SoundInfo OOZSetup::SfxCheck_Slide()
{
    int32 count = 0;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (!player->sidekick) {
            if (player->state.Matches(&OOZSetup::PlayerState_OilSlide) || player->state.Matches(&OOZSetup::PlayerState_OilStrip))
                ++count;
        }
    }

    // return count > 0;
    SoundFX slideSFX;
    slideSFX.Get("OOZ/Slide.wav");
    Soundboard::SoundInfo info = {};
    info.playFlags             = count > 0;
    info.sfx                   = slideSFX;
    info.loopPoint             = 12382;

    return info;
}

Soundboard::SoundInfo OOZSetup::SfxCheck_OilSwim()
{
    // return sVars->swimmingPlayerCount > 0;
    SoundFX swimSFX;
    swimSFX.Get("OOZ/OilSwim.wav");
    Soundboard::SoundInfo info = {};
    info.playFlags             = sVars->swimmingPlayerCount > 0;
    info.sfx                   = swimSFX;
    info.loopPoint             = true;

    return info;
}

void OOZSetup::Draw_Flames()
{
    for (int32 i = 0; i < sVars->flameCount; ++i) {
        if (sVars->flameTimerPtrs[i]) {
            this->rotation               = 2 * (sVars->flamePositions[i].x & 0xFF);
            sVars->flameAnimator.frameID = sVars->flamePositions[i].y & 0xFF;
            sVars->flameAnimator.DrawSprite(&sVars->flamePositions[i], false);
        }
    }
}

void OOZSetup::HandleActiveFlames()
{
    for (int32 i = 0; i < sVars->flameCount; ++i) {
        if (sVars->flameTimerPtrs[i]) {
            --(*sVars->flameTimerPtrs[i]);

            if (!*sVars->flameTimerPtrs[i]) {
                sVars->flameTimerPtrs[i] = nullptr;
                Sol *sol                 = GameObject::Create<Sol>(INT_TO_VOID(true), sVars->flamePositions[i].x, sVars->flamePositions[i].y);
                sol->isFlameFX           = true;
                sol->rotation            = 2 * (sVars->flamePositions[i].x & 0xFF);
                sol->mainAnimator.SetAnimation(Sol::sVars->aniFrames, 2, true, 0);
                sol->state.Set(&Sol::State_FlameDissipate);
            }
            else {
                int32 frame      = sVars->flamePositions[i].y & 0xFF;
                int32 frameTimer = (sVars->flamePositions[i].y >> 8) & 0xFF;

                if (++frameTimer >= 3) {
                    frameTimer = 0;
                    if (++frame > 10)
                        frame = 1;
                }

                // likewise, this too is evil, using the lower 2 bytes to store frame info
                sVars->flamePositions[i].y = frame | (frameTimer << 8) | (sVars->flamePositions[i].y & 0xFFFF0000);
            }

            Vector2 storePos = this->position;
            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                this->position = sVars->flamePositions[i];
                if (player->CheckCollisionTouch(this, &Sol::sVars->hitboxBadnik)) {
                    this->position = storePos;
                    if (player->shield != Player::Shield_Fire) {
                        player->Hurt(this);
                    }
                }
            }
            this->position = storePos;
        }
    }
}

bool32 OOZSetup::StartFire(int32 posX, int32 posY, int32 angle)
{
    int32 pos = (posX >> 20) + (posY >> 20 << 10);

    if (pos < 0x20000) {
        if (!sVars->flameTimers[pos]) {
            int32 i = 0;
            for (; i < 399; ++i) {
                if (!sVars->flameTimerPtrs[i])
                    break;
            }
            // if we get to 399 active flames just use that slot over and over

            sVars->flameTimerPtrs[i]   = &sVars->flameTimers[pos];
            sVars->flamePositions[i].x = posX;
            sVars->flamePositions[i].y = posY;
            sVars->flamePositions[i].x &= 0xFFFF0000;
            sVars->flamePositions[i].y &= 0xFFFF0000;
            // this is so evil, using the bottom byte of the XPos to store the angle
            sVars->flamePositions[i].x |= angle;

            if (i + 1 > sVars->flameCount)
                sVars->flameCount = i + 1;

            sVars->flameTimers[pos]                                                                                               = 0xF0;
            GameObject::Create<Explosion>(INT_TO_VOID(Explosion::Type2), this->position.x, this->position.y - 0x60000)->drawGroup = this->drawGroup;

            return true;
        }
    }

    return false;
}

void OOZSetup::PlayerState_OilPool()
{
    Player *player = (Player *)this;

    int32 top            = player->topSpeed;
    int32 acc            = player->acceleration;
    player->topSpeed     = (player->topSpeed >> 1) + (player->topSpeed >> 3);
    player->acceleration = (player->acceleration >> 1) + (player->acceleration >> 3);

    if (player->groundVel <= 0) {
        if (player->groundVel < -player->topSpeed) {
            player->groundVel += 0x3800;
        }
    }
    else if (player->groundVel > player->topSpeed) {
        player->groundVel -= 0x3800;
    }

    player->position.y += 0x10000;

    player->up       = false;
    player->down     = false;
    player->onGround = true;

    player->State_Ground();

    player->topSpeed     = top;
    player->acceleration = acc;

    if (player->jumpPress) {
        player->Action_Jump();
    }
}

void OOZSetup::PlayerState_OilStrip()
{
    Player *player = (Player *)this;

    int32 acc  = player->acceleration;
    int32 top  = player->topSpeed;
    int32 skid = player->skidSpeed;
    int32 dec  = player->deceleration;

    Animator animator;
    memcpy(&animator, &player->animator, sizeof(Animator));

    player->position.y += 0x10000;

    player->skidSpeed    = player->skidSpeed >> 3;
    player->topSpeed     = (player->topSpeed >> 1) + (player->topSpeed >> 2);
    player->acceleration = 0x400;
    player->deceleration = player->deceleration >> 3;

    player->up       = false;
    player->down     = false;
    player->skidding = 0;
    player->onGround = true;

    player->State_Ground();

    if ((animator.animationID == Player::ANI_HURT || animator.animationID == Player::ANI_FLUME || animator.animationID == Player::ANI_RUN
         || animator.animationID == Player::ANI_WALK)
        && player->groundedStore && player->onGround) {
        if (abs(player->groundVel) >= 0x20000) {
            memcpy(&player->animator, &animator, sizeof(Animator));
            if (player->animator.timer >= 3)
                player->animator.timer = 256;

            // v5U processes the angles slightly differently(?)
            // this fixes it
            if ((player->angle >= 0x3C && player->angle <= 0x40) || (player->angle >= 0xC0 && player->angle <= 0xC4)) {
                player->onGround = false;
                if (player->angle >= 0x3C && player->angle <= 0x40) {
                    player->angle = 0x40;
                }
                else {
                    player->angle = 0xC0;
                }
                player->state.Set(&Player::State_Air);
            }
        }
    }

    player->acceleration = acc;
    player->topSpeed     = top;
    player->skidSpeed    = skid;
    player->deceleration = dec;

    if (player->jumpPress) {
        player->Action_Jump();
    }
}

void OOZSetup::PlayerState_OilSlide()
{
    Player *player = (Player *)this;

    if (!player->onGround) {
        player->state.Set(&Player::State_Air);
        player->HandleAirMovement();
    }
    else {
        player->Gravity_False();

        if (player->angle) {
            if (player->angle <= 0x80) {
                if (player->groundVel < 0x80000)
                    player->groundVel += 0x4000;
            }
            else {
                if (player->groundVel > -0x80000)
                    player->groundVel -= 0x4000;
            }
        }

        player->groundVel += Math::Sin256(player->angle) << 13 >> 8;
        player->controlLock = 30;
        player->direction   = player->groundVel < 0;

        player->animator.SetAnimation(player->aniFrames, Player::ANI_FLUME, false, 0);
    }

    if (player->jumpPress) {
        player->Action_Jump();

        if (player->angle <= 0x80) {
            if (player->velocity.x < 0)
                player->velocity.x += ((player->jumpStrength + (player->jumpStrength >> 1)) * Math::Sin256(player->angle)) >> 8;
        }
        else {
            if (player->velocity.x > 0)
                player->velocity.x += ((player->jumpStrength + (player->jumpStrength >> 1)) * Math::Sin256(player->angle)) >> 8;
        }
    }
}

void OOZSetup::PlayerState_OilFall()
{
    Player *player = (Player *)this;

    int32 top = player->topSpeed;
    int32 acc = player->acceleration;

    player->topSpeed     = (player->topSpeed >> 2) + (player->topSpeed >> 3);
    player->acceleration = (player->acceleration >> 2) + (player->acceleration >> 3);

    if (player->groundVel <= 0) {
        if (player->groundVel < -player->topSpeed)
            player->groundVel += 0x3800;
    }
    else {
        if (player->groundVel > player->topSpeed)
            player->groundVel -= 0x3800;
    }

    player->up            = false;
    player->down          = false;
    player->groundedStore = false;
    player->onGround      = false;
    player->velocity.y    = 0x10000;

    if (abs(player->velocity.x) > 0x20000) {
        if (player->velocity.x <= 0)
            player->velocity.x += 0xC000;
        else
            player->velocity.x -= 0xC000;
    }

    if (!player->left && !player->right)
        player->velocity.x -= player->velocity.x >> 4;

    player->animator.SetAnimation(player->aniFrames, Player::ANI_JUMP, false, 0);

    player->HandleGroundMovement();

    if (player->camera)
        player->camera->disableYOffset = false;

    player->jumpAbilityState = 0;
    player->nextAirState.Set(&Player::State_Air);

    if (player->jumpPress) {
        player->Action_Jump();

        player->jumpAbilityState = 0;
        player->timer            = 0;
    }

    player->topSpeed     = top;
    player->acceleration = acc;
}

#if RETRO_REV0U
void OOZSetup::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(OOZSetup);

    int32 deformData[] = { 1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3,
                           1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3 };

    memcpy(sVars->deformData, deformData, sizeof(deformData));

    int32 glowLightAniDurations[] = { 60, 60, 3, 3, 3, 3, 3, 3, 4 };

    memcpy(sVars->glowLightAniDurations, glowLightAniDurations, sizeof(glowLightAniDurations));

    sVars->glowLightAniTimer = 60;
}
#endif

#if RETRO_INCLUDE_EDITOR
void OOZSetup::EditorDraw() {}

void OOZSetup::EditorLoad()
{
    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Manage Flames", OOZSETUP_FLAMES);
    RSDK_ENUM_VAR("Manage Fade", OOZSETUP_FADE);
}
#endif

void OOZSetup::Serialize() { RSDK_EDITABLE_VAR(OOZSetup, VAR_UINT8, type); }

} // namespace GameLogic