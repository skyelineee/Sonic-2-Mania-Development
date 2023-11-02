// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Water Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Water.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Music.hpp"
#include "Global/PauseMenu.hpp"
#include "Global/Shield.hpp"
#include "Global/Dust.hpp"
#include "Common/Button.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Water);

void Water::Update() { this->state.Run(this); }
void Water::LateUpdate() {}
void Water::StaticUpdate()
{
    PauseMenu *pauseMenu = GameObject::Get<PauseMenu>(SLOT_PAUSEMENU);

    if (pauseMenu->classID != PauseMenu::sVars->classID) {
        if (sVars->newWaterLevel == sVars->targetWaterLevel) {
            sVars->moveWaterLevel = 0;

            if (sVars->waterLevelVolume > 0)
                sVars->waterLevelVolume--;
            sVars->waterLevelVolume = CLAMP(sVars->waterLevelVolume, 0, 30);
        }
        else {
            if (Stage::CheckSceneFolder("HCZ") && sVars->moveWaterLevel) {
                ++sVars->waterLevelVolume;
            }

            if (sVars->newWaterLevel >= sVars->targetWaterLevel) {
                sVars->newWaterLevel -= sVars->waterMoveSpeed;
                if (sVars->newWaterLevel < sVars->targetWaterLevel)
                    sVars->newWaterLevel = sVars->targetWaterLevel;
            }
            else {
                sVars->newWaterLevel += sVars->waterMoveSpeed;
                if (sVars->newWaterLevel > sVars->targetWaterLevel)
                    sVars->newWaterLevel = sVars->targetWaterLevel;
            }
        }
    }

    if (sVars->playingWaterLevelSfx) {
        if (sVars->waterLevelVolume > 30)
            sVars->waterLevelVolume = 30;
        float volume = sVars->waterLevelVolume / 30.0f;
        channels[sVars->waterLevelChannelL].SetAttributes(volume, -1.0, 1.0);
        channels[sVars->waterLevelChannelR].SetAttributes(volume, 1.0, 1.0);
    }

    if (sVars->waterLevelVolume > 0) {
        if (!sVars->playingWaterLevelSfx) {
            sVars->waterLevelChannelL = sVars->sfxWaterLevelL.Play(true);
            channels[sVars->waterLevelChannelL].SetAttributes(0.0, -1.0, 1.0);

            sVars->waterLevelChannelR = sVars->sfxWaterLevelR.Play(true);
            channels[sVars->waterLevelChannelR].SetAttributes(0.0, 1.0, 1.0);

            sVars->playingWaterLevelSfx = true;
        }
    }
    else if (!sVars->waterLevelVolume && sVars->playingWaterLevelSfx) {
        sVars->sfxWaterLevelL.Stop();
        sVars->sfxWaterLevelR.Stop();
        sVars->playingWaterLevelSfx = false;
    }

    bool32 wakeActive = false;
    for (int32 p = 0; p < PLAYER_COUNT; ++p) wakeActive |= sVars->wakePosX[p] > 0;

    if (wakeActive) {
        if (sceneInfo->state != ENGINESTATE_FROZEN)
            sVars->wakeAnimator.Process();

        if (!sVars->playingSkimSfx) {
            sVars->sfxSkim.Play(3570);
            sVars->playingSkimSfx = true;
        }
    }
    else if (sVars->playingSkimSfx) {
        sVars->sfxSkim.Stop();
        sVars->playingSkimSfx = false;
    }
}
void Water::Draw() { this->stateDraw.Run(this); }

void Water::Create(void *data)
{
    this->visible = true;
    if (!sceneInfo->inEditor) {
        if (data)
            this->type = (uint8)VOID_TO_INT(data);

        switch (this->type) {
            case Water::WaterLevel:
                this->active    = ACTIVE_NORMAL;
                this->drawGroup = Zone::sVars->hudDrawGroup - 1;
                if (this->surfaceWaves) {
                    this->inkEffect = INK_ADD;
                    this->alpha     = 0xA0;
                    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
                }
                this->state.Set(&Water::State_Water);
                this->stateDraw.Set(&Water::Draw_Water);
                this->size.x >>= 16;
                sVars->newWaterLevel    = this->position.y;
                sVars->targetWaterLevel = this->position.y;
                break;

            case Water::Pool:
                this->active = ACTIVE_BOUNDS;
                this->drawFX = FX_FLIP;
                switch (this->priority) {
                    case Water::PriorityLowest: this->drawGroup = Zone ::sVars->objectDrawGroup[0] - 1; break;
                    case Water::PriorityLow: this->drawGroup = Zone::sVars->playerDrawGroup[0]; break;
                    case Water::PriorityHigh: this->drawGroup = Zone::sVars->playerDrawGroup[1]; break;
                    case Water::PriorityHighest: this->drawGroup = Zone::sVars->hudDrawGroup - 1; break;
                    default: break;
                }

                this->updateRange.x = this->size.x >> 1;
                this->updateRange.y = this->size.y >> 1;

                this->hitbox.right  = (this->size.x >> 17);
                this->hitbox.left   = -(this->size.x >> 17);
                this->hitbox.bottom = (this->size.y >> 17);
                this->hitbox.top    = -(this->size.y >> 17);

                if (this->surfaceWaves) {
                    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
                    this->inkEffect = INK_ADD;
                    this->alpha     = 0xB8;
                }

                this->state.Set(&Water::State_Pool);
                this->stateDraw.Set(&Water::Draw_Pool);
                break;

            case Water::Bubbler:
                this->drawGroup     = Zone::sVars->objectDrawGroup[0];
                this->inkEffect     = INK_ADD;
                this->alpha         = 0x100;
                this->active        = ACTIVE_BOUNDS;
                this->updateRange.x = 0x100000;
                this->updateRange.y = 0x100000;

                this->animator.SetAnimation(sVars->aniFrames, 2, true, 0);

                this->state.Set(&Water::State_Bubbler);
                this->stateDraw.Set(&Water::Draw_Bubbler);
                break;

            case Water::HeightTrigger:
                this->active  = ACTIVE_BOUNDS;
                this->visible = false;
                if (this->buttonTag > 0) {
                    this->active = ACTIVE_NORMAL;
                    SetupTagLink();
                }

                this->updateRange.x = this->size.x >> 1;
                this->updateRange.y = this->size.y >> 1;

                this->state.Set(&Water::State_HeightTrigger);
                break;

            case Water::Splash:
                this->active    = ACTIVE_NORMAL;
                this->drawGroup = Zone::sVars->hudDrawGroup - 2;
                this->animator.SetAnimation(sVars->aniFrames, 1, true, 0);

                this->state.Set(&Water::State_Splash);
                this->stateDraw.Set(&Water::Draw_Splash);
                break;

            case Water::Bubble:
                this->active        = ACTIVE_NORMAL;
                this->drawGroup     = Zone::sVars->playerDrawGroup[1];
                this->drawFX        = FX_SCALE;
                this->inkEffect     = INK_ADD;
                this->alpha         = 0x100;
                this->updateRange.x = 0x800000;
                this->updateRange.y = 0x800000;
                this->isPermanent   = true;

                this->scale.x = 0x200;
                this->scale.y = 0x200;
                this->animator.SetAnimation(sVars->aniFrames, 5, true, 0);

                this->state.Set(&Water::BubbleFinishPopBehavior);
                this->stateDraw.Set(&Water::Draw_Bubble);
                break;

            case Water::Countdown:
                this->active        = ACTIVE_NORMAL;
                this->drawGroup     = Zone::sVars->playerDrawGroup[1];
                this->drawFX        = FX_SCALE;
                this->inkEffect     = INK_ADD;
                this->alpha         = 0x100;
                this->updateRange.x = 0x800000;
                this->updateRange.y = 0x800000;

                this->scale.x = 0x200;
                this->scale.y = 0x200;
                this->animator.SetAnimation(sVars->aniFrames, 7, true, 0);

                this->state.Set(&Water::State_Countdown);
                this->stateDraw.Set(&Water::Draw_Countdown);
                break;

            default: break;
        }
    }
}

void Water::StageLoad()
{
    sVars->active = ACTIVE_ALWAYS;

    sVars->aniFrames.Load("Global/Water.bin", SCOPE_STAGE);

    sVars->waterLevel       = 0x7FFFFFFF;
    sVars->newWaterLevel    = sVars->waterLevel;
    sVars->targetWaterLevel = sVars->waterLevel;

    sVars->isLightningFlashing = false;
    sVars->disableWaterSplash  = false;

    sVars->hitboxPlayerBubble.left   = -2;
    sVars->hitboxPlayerBubble.top    = -2;
    sVars->hitboxPlayerBubble.right  = 2;
    sVars->hitboxPlayerBubble.bottom = 2;

    sVars->hitboxPoint.left   = -1;
    sVars->hitboxPoint.top    = -1;
    sVars->hitboxPoint.right  = 1;
    sVars->hitboxPoint.bottom = 1;

    sVars->sfxSplash.Get("Stage/Splash.wav");
    sVars->sfxBreathe.Get("Stage/Breathe.wav");
    sVars->sfxWarning.Get("Stage/Warning.wav");
    sVars->sfxDrown.Get("Stage/Drown.wav");
    sVars->sfxDrownAlert.Get("Stage/DrownAlert.wav");
    sVars->sfxSkim.Get("HCZ/Skim.wav");

    if (Stage::CheckSceneFolder("HCZ")) {
        sVars->wakeFrames.Load("HCZ/Wake.bin", SCOPE_STAGE);

        sVars->sfxWaterLevelL.Get("HCZ/WaterLevel_L.wav");
        sVars->sfxWaterLevelR.Get("HCZ/WaterLevel_R.wav");
        sVars->sfxDNAGrab.Get("CPZ/DNAGrab.wav");
        sVars->sfxDNABurst.Get("CPZ/DNABurst.wav");

        sVars->wakeAnimator.SetAnimation(sVars->wakeFrames, 0, true, 0);
    }
}

void Water::DrawHook_ApplyWaterPalette()
{
    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];
    int32 waterDrawPos = CLAMP((sVars->waterLevel >> 0x10) - screen->position.y, 0, screen->size.y);

    paletteBank[sVars->waterPalette].SetActivePalette(waterDrawPos, screen->size.y);
    screenInfo[sceneInfo->currentScreenID].waterDrawPos = waterDrawPos;
}

void Water::DrawHook_RemoveWaterPalette() { paletteBank[0].SetActivePalette(0, screenInfo[sceneInfo->currentScreenID].size.y); }

void Water::SetupTagLink()
{
    this->taggedButton = NULL;
    if (this->buttonTag > 0) {
        if (Button::sVars) {
            for (auto button : GameObject::GetEntities<Button>(FOR_ALL_ENTITIES)) {
                if (button->tag == this->buttonTag) {
                    this->taggedButton = button;
                    break;
                }
            }
        }

        if (this->taggedButton) {
            if (this->updateRange.x < TO_FIXED(128) + abs(this->position.x - this->taggedButton->position.x))
                this->updateRange.x = TO_FIXED(128) + abs(this->position.x - this->taggedButton->position.x);

            if (this->updateRange.y < TO_FIXED(128) + abs(this->position.y - this->taggedButton->position.y))
                this->updateRange.y = TO_FIXED(128) + abs(this->position.y - this->taggedButton->position.y);
        }
    }
}

void Water::SpawnBubble(Player *player, int32 id)
{
    if (sVars->constBubbleTimer[id] <= 0) {
        sVars->constBubbleTimer[id] = 60;
        if (Math::Rand(0, 3) == 1)
            sVars->randBubbleTimer[id] = Math::Rand(0, 16) + 8;
        else
            sVars->randBubbleTimer[id] = 0x200;
    }
    else {
        sVars->constBubbleTimer[id]--;
        if (sVars->randBubbleTimer[id] <= 0)
            sVars->randBubbleTimer[id] = 0x200;
        else
            return;
    }

    Water *bubble = GameObject::Create<Water>(Water::Bubble, player->position.x, player->position.y);
    if (player->direction) {
        bubble->position.x -= TO_FIXED(6);
        bubble->angle = 0x100;
    }
    else {
        bubble->position.x += TO_FIXED(6);
    }

    bubble->childPtr   = player;
    bubble->bubbleX    = bubble->position.x;
    bubble->velocity.y = -0x8800;
    bubble->drawGroup  = player->drawGroup + 1;
}

void Water::SpawnCountDownBubble(Player *player, int32 id, uint8 bubbleID)
{
    Water *bubble = GameObject::Create<Water>(Water::Countdown, player->position.x, player->position.y);
    if (player->direction) {
        bubble->position.x -= 0x60000;
        bubble->angle = 0x100;
    }
    else {
        bubble->position.x += 0x60000;
    }
    bubble->bubbleX     = bubble->position.x;
    bubble->velocity.y  = -0x8800;
    bubble->childPtr    = player;
    bubble->countdownID = bubbleID;
    bubble->drawGroup   = Zone::sVars->playerDrawGroup[1] + 1;
}

void Water::State_Water()
{
    SET_CURRENT_STATE();

    this->animator.Process();

    sVars->waterLevel = (this->size.x * Math::Sin512(2 * Zone::sVars->timer)) + sVars->newWaterLevel;

    for (int32 playerID = 0; playerID < Player::sVars->playerCount; ++playerID) {
        Player *player = GameObject::Get<Player>(playerID);

        if (player->state.Matches(&Player::State_FlyToPlayer) && player->abilityPtrs[0]) {
            player->position.x = ((Entity *)player->abilityPtrs[0])->position.x;
            player->position.y = ((Entity *)player->abilityPtrs[0])->position.y;
        }

        sVars->wakePosX[playerID] = 0;

        bool32 canEnterWater = true;
        if (!player->CheckValidState() || player->state.Matches(&Player::State_TransportTube)) {
            if (!player->state.Matches(&Player::State_FlyToPlayer))
                canEnterWater = false;
        }

        if (canEnterWater) {
            Water *waterPtr   = nullptr;
            uint16 underwater = 0;
            for (auto pool : GameObject::GetEntities<Water>(FOR_ACTIVE_ENTITIES)) {
                if (pool->type == Water::Pool) {
                    if (pool->CheckCollisionTouchBox(&pool->hitbox, player, &sVars->hitboxPoint)) {
                        waterPtr       = pool;
                        pool->childPtr = player;
                        underwater     = pool->Slot();
                    }
                    else if (pool->childPtr == player) {
                        pool->childPtr = nullptr;
                        if (!waterPtr)
                            waterPtr = pool;
                    }
                }
            }

            if (player->position.y > sVars->waterLevel)
                underwater = true;

            if (!player->CheckValidState()) {
                if (!player->state.Matches(&Player::State_FlyToPlayer))
                    underwater = false;
            }

            int32 waterID = 0;
            if (!player->isGhost)
                waterID = underwater;

            if (!waterID) {
                if (player->underwater) {
                    Water *waterSection = nullptr;
                    if (player->underwater > 1)
                        waterSection = GameObject::Get<Water>(player->underwater);

                    player->underwater = false;
                    player->UpdatePhysicsState();

                    if (player->velocity.y) {
                        if (!sVars->disableWaterSplash) {
                            if (waterSection) {
                                Water *splash     = GameObject::Create<Water>(Water::Splash, player->position.x,
                                                                          waterSection->position.y - (waterSection->size.y >> 1));
                                splash->childPtr  = waterSection;
                                splash->drawGroup = player->drawGroup;
                            }
                            else {
                                GameObject::Create<Water>(Water::Splash, player->position.x, sVars->waterLevel);
                            }

                            sVars->sfxSplash.Play();
                        }

                        for (auto countdown : GameObject::GetEntities<Water>(FOR_ALL_ENTITIES)) {
                            if (countdown->type == Water::Countdown && countdown->childPtr == player) {
                                countdown->Destroy();
                            }
                        }

                        if (player->velocity.y >= -0x40000) {
                            player->velocity.y <<= 1;
                            if (player->velocity.y < -0x100000)
                                player->velocity.y = -0x100000;
                        }
                    }
                }
                else {
                    // if we're not underwater already but we would otherwise be
                    if (abs(player->groundVel) >= 0x78000) {
                        Hitbox *playerHitbox = player->GetHitbox();
                        if (abs(player->position.y + (playerHitbox->bottom << 16) - sVars->waterLevel) <= 0x40000 && player->groundedStore) {
                            sVars->wakePosX[playerID] = player->position.x;
                            sVars->wakeDir[playerID]  = player->groundVel < 0;
                            if (!player->onGround) {
                                player->onGround   = true;
                                player->position.y = sVars->waterLevel - (playerHitbox->bottom << 16);
                            }
                        }
                    }
                }

                if (!player->sidekick && player->drownTimer >= 1080) {
                    player->drownTimer = 0;
                    Music::JingleFadeOut(Music::TRACK_DROWNING, false);
                }
            }
            else {
                bool32 notUnderwater = player->underwater == 0;
                player->underwater   = waterID;

                if (notUnderwater) {
                    player->UpdatePhysicsState();
                    if (player->velocity.y) {
                        if (!sVars->disableWaterSplash) {
                            if (waterPtr) {
                                Water *splash =
                                    GameObject::Create<Water>(Water::Splash, player->position.x, waterPtr->position.y - (waterPtr->size.x >> 1));
                                splash->drawGroup = player->drawGroup;
                                splash->childPtr  = waterPtr;
                            }
                            else {
                                GameObject::Create<Water>(Water::Splash, player->position.x, sVars->waterLevel);
                            }

                            sVars->sfxSplash.Play();
                        }

                        player->velocity.y >>= 2;
                    }

                    player->velocity.x >>= 1;

                    player->drownTimer                = 0;
                    sVars->constBubbleTimer[playerID] = 52;
                    sVars->unused1[playerID]          = 0;
                }
                else {
                    if (player->invincibleTimer <= 0) {
                        Shield *shield = GameObject::Get<Shield>(Player::sVars->maxPlayerCount + player->Slot());

                        if (player->shield == Shield::Fire) {
                            // fire underwater! puff out this shield!
                            player->shield = Shield::None;
                            shield->Destroy();

                            Dust *puff = GameObject::Create<Dust>(0, player->position.x - 0xC0000, sVars->waterLevel);
                            puff->state.Set(&Dust::State_DustPuff);
                            puff->drawGroup      = player->drawGroup;
                            puff->velocity.y     = -0x20000;
                            puff->animator.timer = Math::Rand(0, 3);

                            puff = GameObject::Create<Dust>(0, player->position.x, sVars->waterLevel);
                            puff->state.Set(&Dust::State_DustPuff);
                            puff->drawGroup      = player->drawGroup;
                            puff->velocity.y     = -0x20000;
                            puff->animator.timer = Math::Rand(0, 3);

                            puff = GameObject::Create<Dust>(0, player->position.x + 0xC0000, sVars->waterLevel);
                            puff->state.Set(&Dust::State_DustPuff);
                            puff->drawGroup      = player->drawGroup;
                            puff->velocity.y     = -0x20000;
                            puff->animator.timer = Math::Rand(0, 3);

                            puff = GameObject::Create<Dust>(0, player->position.x + 0x180000, sVars->waterLevel);
                            puff->state.Set(&Dust::State_DustPuff);
                            puff->drawGroup      = player->drawGroup;
                            puff->velocity.y     = -0x20000;
                            puff->animator.timer = Math::Rand(0, 3);
                        }

                        if (player->shield == Shield::Lightning) {
                            // lightning underwater, give the palette a good ZAP!
                            player->shield = Shield::None;

                            color flashColor = paletteBank[sVars->waterPalette].GetEntry(62); // brightest color
                            for (int32 i = 0; i < 0x100; ++i) {
                                sVars->flashColorStorage[i] = paletteBank[sVars->waterPalette].GetEntry(i);
                                paletteBank[sVars->waterPalette].SetEntry(i, flashColor);
                            }
                            shield->state.Set(&Shield::State_LightningFlash);
                            sVars->isLightningFlashing = true;
                        }
                    }

                    if (player->shield != Shield::Bubble) {
                        Water::SpawnBubble(player, playerID);

                        bool32 playAlertSfx = false;
                        switch (++player->drownTimer) {
                            default: break;

                            case 360:
                            case 660:
                            case 960:
                                if (!player->sidekick)
                                    sVars->sfxWarning.Play();
                                break;

                            case 1080:
                                if (!player->sidekick) {
                                    Music::PlayJingle(Music::TRACK_DROWNING);
                                }

                                Water::SpawnCountDownBubble(player, playerID, 5);
                                playAlertSfx = true;
                                break;

                            case 1140: playAlertSfx = true; break;

                            case 1200:
                                Water::SpawnCountDownBubble(player, playerID, 4);
                                playAlertSfx = true;
                                break;

                            case 1260: playAlertSfx = true; break;

                            case 1320:
                                Water::SpawnCountDownBubble(player, playerID, 3);
                                playAlertSfx = true;
                                break;

                            case 1380: playAlertSfx = true; break;

                            case 1440:
                                Water::SpawnCountDownBubble(player, playerID, 2);
                                playAlertSfx = true;
                                break;

                            case 1500: playAlertSfx = true; break;

                            case 1560:
                                Water::SpawnCountDownBubble(player, playerID, 1);
                                playAlertSfx = true;
                                break;

                            case 1620: playAlertSfx = true; break;

                            case 1680:
                                Water::SpawnCountDownBubble(player, playerID, 0);
                                playAlertSfx = true;
                                break;

                            case 1740: playAlertSfx = true; break;

                            case 1800:
                                player->deathType = Player::DeathDrown;
                                player->drawGroup = Zone::sVars->playerDrawGroup[1];
                                playAlertSfx      = true;
                                break;
                        }
                    }
                }
            }
        }

        if (this->state.Matches(&Player::State_FlyToPlayer) && player->abilityPtrs[0]) {
            this->position.x = player->position.x;
            this->position.y = player->position.y;
        }
    }
}

void Water::State_Pool()
{
    SET_CURRENT_STATE();

    if (this->surfaceWaves)
        this->animator.Process();
}

void Water::State_Splash()
{
    SET_CURRENT_STATE();

    Water *water = (Water *)this->childPtr;
    if (water) {
        if (water != (Water *)1)
            this->position.y = water->position.y - (water->size.y >> 1);
    }
    else {
        this->position.y = sVars->waterLevel;
    }

    this->animator.Process();

    if (this->animator.frameID == this->animator.frameCount - 1)
        this->Destroy();
}

void Water::BubbleFloatBehavior()
{
    SET_CURRENT_STATE();

    if (this->bubbleVelocity.x || this->bubbleVelocity.y) {
        this->position.x += this->bubbleVelocity.x;
        this->position.y += this->bubbleVelocity.y;
    }
    else {
        this->position.x += this->velocity.x;
        this->position.y += this->velocity.y;
    }

    this->position.x += this->bubbleOffset.x;
    this->position.y += this->bubbleOffset.y;

    int32 anim = this->animator.animationID;
    if ((anim == 3 && this->animator.frameID < 12) || anim == 4 || anim == 5 || (anim == 7 && !this->activePlayers)) {
        this->position.x = (Math::Sin512(this->angle) << 9) + this->bubbleX;
        this->angle      = (this->angle + 4) & 0x1FF;
    }

    if (this->position.y < sVars->waterLevel) {
        bool32 inWater = false;
        for (auto pool : GameObject::GetEntities<Water>(FOR_ACTIVE_ENTITIES)) {
            if (pool->type == Water::Pool && pool->CheckCollisionTouchBox(&pool->hitbox, this, &sVars->hitboxPoint))
                inWater = true;
        }

        if (!inWater) {
            if (this->animator.animationID == 3 && this->animator.frameID > 12) {
                this->animator.SetAnimation(sVars->aniFrames, 6, false, 0);
                this->velocity.y = 0;
            }
            else {
                this->Destroy();
            }
        }
    }
}

void Water::ApplyHeightTriggers()
{
    for (auto trigger : GameObject::GetEntities<Water>(FOR_ACTIVE_ENTITIES)) {
        if (trigger->type == Water::HeightTrigger)
            trigger->State_HeightTrigger();
        sVars->waterLevel = sVars->targetWaterLevel;
    }
}

void Water::BubbleFinishPopBehavior()
{
    SET_CURRENT_STATE();

    Player *player = (Player *)this->childPtr;

    if (this->animator.animationID == 6 && this->animator.frameID == this->animator.frameCount - 1)
        this->Destroy();

    if (player && player->state.Matches(&Player::State_WaterCurrent) && this->animator.frameID < 3)
        this->bubbleX += 0x40000;

    if (this->speed != -1) {
        if (this->speed) {
            this->bubbleX += this->velocity.x;
            this->velocity.x += this->speed;
        }

        Water::BubbleFloatBehavior();

        if (this->tileCollisions) {
            if (!this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x100000, false)) {
                while (this->TileCollision(Zone::sVars->collisionLayers, CMODE_ROOF, 0, 0, -0x100000, false)
                       && this->TileCollision(Zone::sVars->collisionLayers, CMODE_ROOF, 0, 0, -0x100000, false)) {
                    this->position.y += 0x10000;
                }
            }
            else {
                while (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x100000, 0)) this->position.y -= 0x10000;
            }
        }
    }

    this->animator.Process();

    if (this->animator.frameID >= 13 || this->animator.animationID == 7) {

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (player->CheckValidState() || player->state.Matches(&Player::State_FlyToPlayer)) {
                if (player->shield != Shield::Bubble && player->underwater && player->position.x >= this->position.x - TO_FIXED(16)
                    && player->position.x <= this->position.x + TO_FIXED(16)) {

                    bool32 inWater = false;
                    if (player->animator.animationID == Player::ANI_FAN) {
                        if (player->position.y >= this->position.y - TO_FIXED(16))
                            inWater = (player->position.y <= this->position.y + TO_FIXED(16));
                    }
                    else {
                        if (player->position.y > this->position.y)
                            inWater = (player->position.y <= this->position.y + TO_FIXED(16));
                    }

                    if (inWater) {
                        bool32 inBubble = false;
                        if (!(this->bubbleFlags & 1) && player->sidekick) {
                            this->bubbleFlags |= 1;
                            inBubble = true;
                        }

                        if (!player->sidekick || inBubble) {
                            if (!inBubble) {
                                this->state.Set(&Water::State_BubbleBreathed);
                                this->countdownID = 0;
                                this->velocity.y  = 0;
                                this->childPtr    = player;
                            }

                            // if (!player->state.Matches(&Current::PState_Right) && !player->state.Matches(&Current::PState_Left)
                            //     && !player->state.Matches(&Current::PState_Up) && !player->state.Matches(&Current::PState_Down)) {
                            player->velocity.x = 0;
                            player->velocity.y = 0;
                            player->groundVel  = 0;
                            bool32 canBreathe  = true;

                            int32 anim = player->animator.animationID;
                            if (player->characterID == ID_TAILS) {
                                canBreathe = anim != Player::ANI_FLY && anim != Player::ANI_FLY_TIRED && anim != Player::ANI_FLY_LIFT
                                             && anim != Player::ANI_SWIM && anim != Player::ANI_SWIM_LIFT;
                            }
                            else if (player->characterID == ID_KNUCKLES) {
                                canBreathe = anim != Player::ANI_LEDGE_PULL_UP && anim != Player::ANI_GLIDE && anim != Player::ANI_GLIDE_SLIDE
                                             && anim != Player::ANI_CLIMB_IDLE && anim != Player::ANI_CLIMB_UP && anim != Player::ANI_CLIMB_DOWN;
                            }

                            if (canBreathe && (anim != Player::ANI_FAN && anim != Player::ANI_CLING)) {
                                player->animator.SetAnimation(player->aniFrames, Player::ANI_BREATHE, false, 0);

                                if (!player->sidekick)
                                    this->allowBreathe = true;
                            }

                            if (player->state.Matches(&Player::State_FlyCarried)) {
                                player->state.Set(&Player::State_Air);
                                GameObject::Get<Player>(SLOT_PLAYER2)->flyCarryTimer = 30;
                            }
                            // }

                            player->drownTimer = 0;
                            if (!player->sidekick)
                                Music::JingleFadeOut(Music::TRACK_DROWNING, false);

                            sVars->sfxBreathe.Play();
                        }
                    }
                }
            }
        }
    }

    if (this->speed != -1) {
        if (!this->CheckOnScreen(&this->updateRange))
            this->Destroy();
    }
}

void Water::State_BubbleBreathed()
{
    SET_CURRENT_STATE();

    Player *player = (Player *)this->childPtr;
    if (player->state.Matches(&Player::State_Hurt) || !player->CheckValidState())
        this->allowBreathe = false;

    if (this->speed) {
        this->position.x += this->velocity.x;
        this->velocity.x += this->speed;
    }

    this->animator.Process();

    this->scale.x -= 0x18;
    this->scale.y -= 0x18;

    if (this->scale.x > 0) {
        if (this->allowBreathe)
            player->animator.SetAnimation(player->aniFrames, Player::ANI_BREATHE, false, 0);
    }
    else {
        this->scale.x = 0;
        this->scale.y = 0;

        if (this->allowBreathe)
            player->animator.SetAnimation(player->aniFrames, Player::ANI_WALK, false, 0);

        this->Destroy();
    }
}

void Water::State_Bubbler()
{
    SET_CURRENT_STATE();

    this->visible = false;
    if (this->position.y > sVars->waterLevel)
        this->visible = true;

    for (auto pool : GameObject::GetEntities<Water>(FOR_ACTIVE_ENTITIES)) {
        if (pool->type == Water::Pool && pool->CheckCollisionTouchBox(&pool->hitbox, this, &sVars->hitboxPoint))
            this->visible = true;
    }

    if (this->visible && this->CheckOnScreen(&this->updateRange)) {
        if (!this->countdownID) {
            if (!this->bubbleFlags) {
                this->bubbleFlags = 1;
                int32 rand        = Math::Rand(0, 0x10000);
                this->bubbleType1 = rand % 6;
                this->bubbleType2 = rand & 12;

                if (!this->dudsRemaining--) {
                    this->bubbleFlags |= 2;
                    this->dudsRemaining = this->numDuds;
                }
            }

            this->countdownID = Math::Rand(0, 32);

            Water *bubble = GameObject::Create<Water>(Water::Bubble, this->position.x, this->position.y - 0x20000);

            int32 bubbleSize = sVars->bubbleSizes[this->bubbleType1 + this->bubbleType2];
            if (globals->useManiaBehavior) {
                bubble->animator.loopIndex  = bubbleSize;
                bubble->animator.frameCount = bubbleSize + 1;
            }
            else {
                int32 anim = bubbleSize - 2;
                if (anim < 0)
                    anim = bubbleSize - 1;
                bubble->animator.SetAnimation(sVars->aniFrames, 5 - (anim >> 1), true, 0);
            }

            bubble->position.x += Math::Rand(-8, 9) << 16;
            bubble->velocity.y = -0x8800;
            bubble->angle      = 2 * Math::Rand(0, 256);

            bubble->childPtr = nullptr;
            bubble->bubbleX  = bubble->position.x;
            if (this->bubbleFlags & 2 && (!Math::Rand(0, 4) || !this->bubbleType1) && !(this->bubbleFlags & 4)) {
                bubble->animator.SetAnimation(sVars->aniFrames, 3, false, 0);
                bubble->isPermanent = true;
                this->bubbleFlags |= 4;
            }

            if (--this->bubbleType1 < 0) {
                this->bubbleFlags = 0;
                this->countdownID += Math::Rand(0, 128) + 128;
            }
        }
        else {
            --this->countdownID;
        }
    }

    this->animator.Process();
}

void Water::State_Countdown()
{
    SET_CURRENT_STATE();

    Player *player = (Player *)this->childPtr;

    if (player->animator.animationID == Player::ANI_FAN) {
        this->bubbleX += player->velocity.x;
        this->position.y += player->velocity.y;
    }

    this->animator.Process();

    bool32 isActive = false;
    if (this->position.y >= sVars->waterLevel) {
        isActive = true;
    }
    else {
        for (auto pool : GameObject::GetEntities<Water>(FOR_ACTIVE_ENTITIES)) {
            if (pool->type == Water::Pool && pool->CheckCollisionTouchBox(&pool->hitbox, this, &sVars->hitboxPoint)) {
                isActive = true;
            }
        }
    }

    if ((this->animator.animationID != 7 || this->animator.frameID != this->animator.frameCount - 1) && isActive) {
        this->position.y += this->velocity.y;
        this->position.x = (Math::Sin512(this->angle) << 9) + this->bubbleX;
        this->angle      = (this->angle + 4) & 0x1FF;
    }
    else {
        this->animator.SetAnimation(sVars->aniFrames, 8 + this->countdownID, true, 0);

        if (player->camera) {
            this->size.x = (this->position.x & 0xFFFF0000) - (player->camera->position.x & 0xFFFF0000);
            this->size.y = (this->position.y & 0xFFFF0000) - (player->camera->position.y & 0xFFFF0000);
        }
        else {
            this->size.x = (this->position.x & 0xFFFF0000) - (player->position.x & 0xFFFF0000);
            this->size.y = (this->position.y & 0xFFFF0000) - (player->position.y & 0xFFFF0000);
        }

        this->state.Set(&Water::State_CountdownFollow);
    }
}

void Water::State_CountdownFollow()
{
    SET_CURRENT_STATE();

    this->animator.Process();

    if (this->angle >= 0x280) {
        this->scale.x -= 8;
        this->scale.y -= 8;
        if (this->scale.x <= 0)
            this->Destroy();
    }
    else {
        this->scale.x = (Math::Sin256(this->angle) >> 1) + 0x200;
        this->scale.y = (Math::Sin256(this->angle + 0x80) >> 1) + 0x200;

        this->angle += 6;
    }
}

void Water::State_HeightTrigger()
{
    SET_CURRENT_STATE();

    bool32 activated = false;

    if (this->taggedButton) {
        Button *button = (Button *)this->taggedButton;
        if (button->currentlyActive) {
            sVars->moveWaterLevel = true;
            activated             = true;
        }
    }

    int32 px = this->position.x + 1;
    if (!activated) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if (!player->Slot()) {
                if (abs(this->position.x - player->position.x) < this->updateRange.x) {
                    if (abs(this->position.y - player->position.y) < this->updateRange.y) {
                        activated = true;
                        px        = player->position.x;
                    }
                }
            }
        }
    }

    if (!activated) {
        for (auto player : GameObject::GetEntities<DebugMode>(FOR_ACTIVE_ENTITIES)) {
            if (!player->Slot()) {
                if (abs(this->position.x - player->position.x) < this->updateRange.x) {
                    if (abs(this->position.y - player->position.y) < this->updateRange.y) {
                        activated = true;
                        px        = player->position.x;
                    }
                }
            }
        }
    }

    if (activated) {
        if (this->taggedButton)
            sVars->targetWaterLevel = this->position.y;
        else if (px <= this->position.x)
            sVars->targetWaterLevel = this->height.x;
        else
            sVars->targetWaterLevel = this->height.y;

        if (this->speed == -1) {
            sVars->newWaterLevel  = sVars->targetWaterLevel;
            sVars->waterMoveSpeed = 0;
        }
        else {
            sVars->waterMoveSpeed = this->speed << 15;
        }

        if (this->destroyOnTrigger)
            this->Destroy();
    }
}

void Water::Draw_Water()
{
    SET_CURRENT_STATE();

    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    Vector2 drawPos;
    drawPos.y = sVars->waterLevel;
    if (this->surfaceWaves) {
        this->inkEffect    = INK_ADD;
        SpriteFrame *frame = this->animator.GetFrame(sVars->aniFrames);

        drawPos.x  = ((screen->position.x / frame->width * frame->width) << 16) + (frame->width << 15);
        int32 size = screen->size.x / frame->width;
        for (int32 i = size + 3; i > 0; --i) {
            this->animator.DrawSprite(&drawPos, false);
            drawPos.x += frame->width << 16;
        }
    }

    this->drawFX |= FX_FLIP;
    this->inkEffect = INK_NONE;
    for (int32 p = 0; p < PLAYER_COUNT; ++p) {
        if (sVars->wakePosX[p] > 0) {
            this->direction = sVars->wakeDir[p];
            drawPos.x       = sVars->wakePosX[p];
            sVars->wakeAnimator.DrawSprite(&drawPos, false);
        }
    }

    this->drawFX &= ~FX_FLIP;
    this->inkEffect = INK_ALPHA;
}

void Water::Draw_Pool()
{
    SET_CURRENT_STATE();

    Vector2 drawPos;
    drawPos.y = this->position.y - (this->size.y >> 1);
    if (this->surfaceWaves) {
        SpriteFrame *frame = this->animator.GetFrame(sVars->aniFrames);

        drawPos.x  = this->position.x - (this->size.x >> 1) + (frame->width << 15);
        int32 size = (this->size.x >> 16) / frame->width;
        for (int32 i = size + 1; i > 0; --i) {
            this->animator.DrawSprite(&drawPos, false);
            drawPos.x += frame->width << 16;
        }
    }

    Graphics::DrawRect(this->position.x - (this->size.x >> 1), this->position.y - (this->size.y >> 1), this->size.x, this->size.y,
                       (this->r << 16) | (this->g << 8) | this->b, 0x100, INK_SUB, false);
}

void Water::Draw_Splash()
{
    SET_CURRENT_STATE();

    this->animator.DrawSprite(nullptr, false);
}

void Water::Draw_Countdown()
{
    SET_CURRENT_STATE();

    Vector2 drawPos;

    if (this->state.Matches(&Water::State_Countdown)) {
        drawPos.x = this->position.x;
        drawPos.y = this->position.y;
    }
    else {
        Player *player = (Player *)this->childPtr;
        if (player->camera) {
            drawPos.x = player->camera->position.x + this->size.x;
            drawPos.y = player->camera->position.y + this->size.y;
        }
        else {
            drawPos.x = player->position.x + this->size.x;
            drawPos.y = player->position.y + this->size.y;
        }
    }

    this->animator.DrawSprite(&drawPos, false);
}

void Water::Draw_Bubbler(void)
{
    SET_CURRENT_STATE();

    this->animator.DrawSprite(nullptr, false);
}

void Water::Draw_Bubble()
{
    SET_CURRENT_STATE();

    this->animator.DrawSprite(nullptr, false);
}

#if RETRO_INCLUDE_EDITOR
void Water::EditorDraw()
{
    Vector2 drawPos = this->position;
    color color     = (this->r << 16) | (this->g << 8) | this->b;

    int32 sx, sy;

    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    switch (this->type) {
        case Water::WaterLevel:
            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->animator.DrawSprite(nullptr, false);
            break;

        case Water::Pool:

            this->updateRange.x = this->size.x >> 1;
            this->updateRange.y = this->size.y >> 1;

            // clang-format off
            sy = this->size.y >> 17 << 16;
            sx = this->size.x >> 17 << 16;
            Graphics::DrawLine(this->position.x - sx, this->position.y - sy, this->position.x + sx, this->position.y - sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x - sx, this->position.y + sy, this->position.x + sx, this->position.y + sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x - sx, this->position.y - sy, this->position.x - sx, this->position.y + sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x + sx, this->position.y - sy, this->position.x + sx, this->position.y + sy, color, 0xFF, INK_NONE, false);

            sx = (this->size.x >> 17 << 16) - 0x10000;
            sy = (this->size.y >> 17 << 16) - 0x10000;
            Graphics::DrawLine(this->position.x - sx, this->position.y - sy, this->position.x + sx, this->position.y - sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x - sx, this->position.y + sy, this->position.x + sx, this->position.y + sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x - sx, this->position.y - sy, this->position.x - sx, this->position.y + sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x + sx, this->position.y - sy, this->position.x + sx, this->position.y + sy, color, 0xFF, INK_NONE, false);

            sx = (this->size.x >> 17 << 16) - 0x20000;
            sy = (this->size.y >> 17 << 16) - 0x20000;
            Graphics::DrawLine(this->position.x - sx, this->position.y - sy, this->position.x + sx, this->position.y - sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x - sx, this->position.y + sy, this->position.x + sx, this->position.y + sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x - sx, this->position.y - sy, this->position.x - sx, this->position.y + sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x + sx, this->position.y - sy, this->position.x + sx, this->position.y + sy, color, 0xFF, INK_NONE, false);

            sx = (this->size.x >> 17 << 16) - 0x30000;
            sy = (this->size.y >> 17 << 16) - 0x30000;
            Graphics::DrawLine(this->position.x - sx, this->position.y - sy, this->position.x + sx, this->position.y - sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x - sx, this->position.y + sy, this->position.x + sx, this->position.y + sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x - sx, this->position.y - sy, this->position.x - sx, this->position.y + sy, color, 0xFF, INK_NONE, false);
            Graphics::DrawLine(this->position.x + sx, this->position.y - sy, this->position.x + sx, this->position.y + sy, color, 0xFF, INK_NONE, false);
            // clang-format on

            this->drawFX = FX_FLIP;
            this->animator.SetAnimation(sVars->aniFrames, 6, true, 0);

            RSDK_DRAWING_OVERLAY(true);
            drawPos.x       = position.x - (this->size.x >> 1);
            drawPos.y       = position.y - (this->size.y >> 1);
            this->direction = FLIP_X;
            this->animator.DrawSprite(&drawPos, false);

            drawPos.x += this->size.x;
            this->direction = FLIP_NONE;
            this->animator.DrawSprite(&drawPos, false);

            drawPos.y += this->size.y;
            this->direction = FLIP_Y;
            this->animator.DrawSprite(&drawPos, false);

            drawPos.x -= this->size.x;
            this->direction = FLIP_XY;
            this->animator.DrawSprite(&drawPos, false);
            RSDK_DRAWING_OVERLAY(false);

            this->direction = FLIP_NONE;
            break;

        case Water::Bubbler:
            this->animator.SetAnimation(sVars->aniFrames, 2, true, 0);
            this->animator.DrawSprite(nullptr, false);
            break;

        case Water::HeightTrigger:

            this->updateRange.x = this->size.x >> 1;
            this->updateRange.y = this->size.y >> 1;

            if (showGizmos()) {
                RSDK_DRAWING_OVERLAY(true);
                // clang-format off
                sx = this->size.x >> 1;
                sy = this->size.y >> 1;
                Graphics::DrawLine(this->position.x - sx, this->position.y - sy, this->position.x + sx, this->position.y - sy, 0xFFFF00, 0xFF, INK_NONE, false);
                Graphics::DrawLine(this->position.x - sx, this->position.y + sy, this->position.x + sx, this->position.y + sy, 0xFFFF00, 0xFF, INK_NONE, false);
                Graphics::DrawLine(this->position.x - sx, this->position.y - sy, this->position.x - sx, this->position.y + sy, 0xFFFF00, 0xFF, INK_NONE, false);
                Graphics::DrawLine(this->position.x + sx, this->position.y - sy, this->position.x + sx, this->position.y + sy, 0xFFFF00, 0xFF, INK_NONE, false);
                // clang-format on
                RSDK_DRAWING_OVERLAY(false);
            }

            this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
            this->animator.DrawSprite(&drawPos, false);
            break;

        default: break;
    }
}

void Water::EditorLoad()
{
    sVars->aniFrames.Load("Global/Water.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Water Level");
    RSDK_ENUM_VAR("Pool");
    RSDK_ENUM_VAR("Bubbler");
    RSDK_ENUM_VAR("Height Trigger");

    RSDK_ACTIVE_VAR(sVars, priority);
    RSDK_ENUM_VAR("Lowest");
    RSDK_ENUM_VAR("Low");
    RSDK_ENUM_VAR("High");
    RSDK_ENUM_VAR("Highest");
}
#endif

#if RETRO_REV0U
void Water::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Water);

    sVars->aniFrames.Init();
    sVars->wakeFrames.Init();

    sVars->sfxSplash.Init();
    sVars->sfxBreathe.Init();
    sVars->sfxWarning.Init();
    sVars->sfxDrownAlert.Init();
    sVars->sfxDrown.Init();
    sVars->sfxSkim.Init();
    sVars->sfxDNAGrab.Init();
    sVars->sfxDNABurst.Init();
    sVars->sfxWaterLevelL.Init();
    sVars->sfxWaterLevelR.Init();

    sVars->bubbleSizes[0]  = 2;
    sVars->bubbleSizes[1]  = 4;
    sVars->bubbleSizes[2]  = 2;
    sVars->bubbleSizes[3]  = 2;
    sVars->bubbleSizes[4]  = 2;
    sVars->bubbleSizes[5]  = 2;
    sVars->bubbleSizes[6]  = 4;
    sVars->bubbleSizes[7]  = 2;
    sVars->bubbleSizes[8]  = 4;
    sVars->bubbleSizes[9]  = 2;
    sVars->bubbleSizes[10] = 2;
    sVars->bubbleSizes[11] = 4;
    sVars->bubbleSizes[12] = 2;
    sVars->bubbleSizes[13] = 4;
    sVars->bubbleSizes[14] = 2;
    sVars->bubbleSizes[15] = 2;
    sVars->bubbleSizes[16] = 4;
    sVars->bubbleSizes[17] = 2;
}
#endif

void Water::Serialize()
{
    RSDK_EDITABLE_VAR(Water, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(Water, VAR_UINT8, numDuds);
    RSDK_EDITABLE_VAR(Water, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(Water, VAR_VECTOR2, height);
    RSDK_EDITABLE_VAR(Water, VAR_ENUM, speed);
    RSDK_EDITABLE_VAR(Water, VAR_ENUM, buttonTag);
    RSDK_EDITABLE_VAR(Water, VAR_UINT8, r);
    RSDK_EDITABLE_VAR(Water, VAR_UINT8, g);
    RSDK_EDITABLE_VAR(Water, VAR_UINT8, b);
    RSDK_EDITABLE_VAR(Water, VAR_UINT8, priority);
    RSDK_EDITABLE_VAR(Water, VAR_BOOL, destroyOnTrigger);
    RSDK_EDITABLE_VAR(Water, VAR_BOOL, surfaceWaves);
}

} // namespace GameLogic
