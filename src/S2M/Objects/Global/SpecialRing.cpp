// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SpecialRing Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "SpecialRing.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "DebugMode.hpp"
#include "Music.hpp"
#include "SaveGame.hpp"
#include "ItemBox.hpp"
#include "Ring.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SpecialRing);

void SpecialRing::Update() { this->state.Run(this); }
void SpecialRing::LateUpdate() {}
void SpecialRing::StaticUpdate() {}
void SpecialRing::Draw()
{
    this->inkEffect = INK_NONE;
    if (this->state.Matches(&SpecialRing::State_Flash)) {
        this->direction = this->warpAnimator.frameID > 8;
        this->warpAnimator.DrawSprite(NULL, false);
    }
    else {
        if (!this->disableHPZ || this->super) {
            // do super colors
        }

        if (!this->enabled) {
            this->inkEffect = INK_ALPHA;
        }
        this->ringAnimator.DrawSprite(nullptr, false);
    }
}

void SpecialRing::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->disableHPZ = true;

        if (SaveGame::GetEmerald(SaveGame::AllowSuperEmeralds)) {
            this->super = true;

            if (SaveGame::GetEmeralds(SaveGame::EmeraldAny)) {
                this->super      = false;
                this->disableHPZ = false;
            }

            if (this->super) {
                if (!SaveGame::GetEmerald(SaveGame::VisitedHPZ))
                    this->disableHPZ = false;
            }
        }

        if (this->enabled)
            this->ringAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        else
            this->ringAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

        if (data) {
            this->ringAnimator.SetAnimation(sVars->aniFrames, 0, true, 0); // if its created through debug mode, set the animation to unused ring
        }

        this->active        = ACTIVE_BOUNDS;
        this->visible       = true;
        this->updateRange.x = TO_FIXED(144);
        this->updateRange.y = TO_FIXED(144);
        this->drawFX        = FX_FLIP;
        this->inkEffect     = INK_NONE;
        this->alpha         = 192;
        if (this->planeFilter > 0 && ((uint8)this->planeFilter - 1) & 2)
            this->drawGroup = Zone::sVars->objectDrawGroup[1];
        else
            this->drawGroup = Zone::sVars->objectDrawGroup[0];
        this->state.Set(&SpecialRing::State_Idle);

        this->warpAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    }
}

void SpecialRing::StageLoad()
{
    sVars->aniFrames.Load("Global/SpecialRing.bin", SCOPE_STAGE);

    sVars->hitbox.left   = -18;
    sVars->hitbox.top    = -18;
    sVars->hitbox.right  = 18;
    sVars->hitbox.bottom = 18;

    sVars->sfxSpecialRing.Get("Global/SpecialRing.wav");
    sVars->sfxSpecialWarp.Get("Global/SpecialWarp.wav");

    DebugMode::AddObject(sVars->classID, &SpecialRing::DebugSpawn, &SpecialRing::DebugDraw);

    for (auto ring : GameObject::GetEntities<SpecialRing>(FOR_ALL_ENTITIES)) {
        if (ring->id <= 0 || globals->gameMode == MODE_TIMEATTACK || (globals->secrets & SECRET_NOITEMS)) {
            ring->enabled = false;
        }
        else {
            ring->enabled = !SaveGame::GetCollectedSpecialRing(ring->id);
            if (globals->specialRingID == ring->id) {
                for (int32 p = 0; p < Player::sVars->playerCount; ++p) {
                    Player *player = GameObject::Get<Player>(p);

                    player->position.x = ring->position.x;
                    player->position.y = ring->position.y + 0x100000;
                    if (!p) {
                        Player *player2 = GameObject::Get<Player>(SLOT_PLAYER2);
                        player2->position.x = player->position.x;
                        player2->position.y = player->position.y;
                        player2->direction  = player->direction;
                        if (player->direction)
                            player2->position.x += 0x100000;
                        else
                            player2->position.x -= 0x100000;

                        for (int32 f = 0; f < 0x10; ++f) {
                            Player::sVars->leaderPositionBuffer[f].x = player->position.x;
                            Player::sVars->leaderPositionBuffer[f].y = player->position.y;
                        }
                    }
                }

                sceneInfo->milliseconds = globals->tempMilliseconds;
                sceneInfo->seconds      = globals->tempSeconds;
                sceneInfo->minutes      = globals->tempMinutes;
            }
        }
    }
}

void SpecialRing::DebugSpawn() 
{ 
    SpecialRing *specialRing = GameObject::Create<SpecialRing>(INT_TO_VOID(true), this->position.x, this->position.y);
    specialRing->enabled     = true;
}

void SpecialRing::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 13);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void SpecialRing::State_Idle()
{
    SET_CURRENT_STATE();

    Vector2 range;
    range.x = (128 << 16);
    range.y = (128 << 16);
    // leaving scale here cause it sorta acts as a timer to enter the ring
    if (!this->CheckOnScreen(&range)) {
        this->scale.x      = 0;
        // sets the frame back to 0 when it leaves the screen
        if (this->enabled)
            this->ringAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        else
            this->ringAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    }
    else 
        this->ringAnimator.Process(); // processes (plays) the animation when its on screen

    if (this->scale.x >= 0x140)
        this->scale.x = 0x140;
    else
        this->scale.x += ((0x168 - this->scale.x) >> 5);

    if (this->enabled && this->scale.x > 0x100) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if ((this->planeFilter <= 0 || player->collisionPlane == (((uint8)this->planeFilter - 1) & 1)) && !player->sidekick) {
                if (player->CheckCollisionTouch(this, &sVars->hitbox) && sceneInfo->timeEnabled) {
                    this->sparkleRadius = (16 << 16);
                    this->state.Set(&SpecialRing::State_Flash);

                    SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();

                    // rings spawned via debug mode give you 50 rings, always
                    if (!SaveGame::AllChaosEmeralds() && this->id) {
                        player->visible        = false;
                        player->active         = ACTIVE_NEVER;
                        sceneInfo->timeEnabled = false;
                    }
                    else {
                        player->GiveRings(50, true);
                    }

                    if (this->id > 0) {
                        if (!SaveGame::AllChaosEmeralds())
                            globals->specialRingID = this->id;

                        SaveGame::SetCollectedSpecialRing(this->id);
                    }

                    if (this->id >= 1) {
                        globals->specialRingID = this->id;
                        SaveGame::SetCollectedSpecialRing(this->id);
                    }

                    // rings spawned via debug mode give you 50 rings, always
                    /*if (SaveGame::GetEmeralds(SaveGame::EmeraldBoth)
                        || (SaveGame::GetEmeralds(SaveGame::EmeraldAny) && !SaveGame::GetEmerald(SaveGame::AllowSuperEmeralds)
                            && !SaveGame::GetEmerald(SaveGame::VisitedHPZ))) {
                        player->GiveRings(50, true);
                    }
                    else {
                        player->visible        = false;
                        player->active         = ACTIVE_NEVER;
                        sceneInfo->timeEnabled = false;
                    }*/

                    sVars->sfxSpecialRing.Play(false, 0xFE);
                }
            }
        }
    }
}

void SpecialRing::State_Flash()
{
    SET_CURRENT_STATE();

    this->warpAnimator.Process();

    if (!(Zone::sVars->timer & 3)) {
        for (int32 i = 0; i < 3; ++i) {
            int32 x       = this->position.x + Math::Rand(-(32 << 16), (2 << 16)) + this->sparkleRadius;
            int32 y       = this->position.y + Math::Rand(-(32 << 16), (32 << 16));
            Ring *sparkle = GameObject::Create<Ring>(NULL, x, y);

            sparkle->state.Set(&Ring::State_Sparkle);
            sparkle->stateDraw.Set(&Ring::Draw_Sparkle);
            sparkle->active    = ACTIVE_NORMAL;
            sparkle->visible   = false;
            sparkle->drawGroup = Zone::sVars->objectDrawGroup[0];
            sparkle->animator.SetAnimation(Ring::sVars->aniFrames, i % 3 + 2, true, 0);
            int32 cnt = sparkle->animator.frameCount;
            if (sparkle->animator.animationID == 2) {
                sparkle->alpha = 0xE0;
                cnt >>= 1;
            }
            sparkle->maxFrameCount  = cnt - 1;
            sparkle->animator.speed = Math::Rand(6, 8);
            sparkle->timer          = 2 * i;
        }

        this->sparkleRadius -= (8 << 16);
    }

    SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();
    // rings spawned via debug mode give you 50 rings, always
    if (SaveGame::AllChaosEmeralds() || !this->id) {
        this->warpTimer++;
        if (this->warpTimer == 20) {
            this->Destroy();
        }
    }
    else if (this->warpAnimator.frameID == this->warpAnimator.frameCount - 1) {
        this->warpTimer = 0;
        this->visible   = false;
        this->state.Set(&SpecialRing::State_Warp);
    }
}

void SpecialRing::State_Warp()
{
    SET_CURRENT_STATE();

    if (++this->warpTimer == 30) {
        SaveGame::SaveGameState();
        sVars->sfxSpecialWarp.Play(false, 0xFE);
        this->Destroy();

        SaveGame::SaveRAM *saveRAM = SaveGame::GetSaveRAM();
        saveRAM->storedStageID = sceneInfo->listPos;
        if (this->disableHPZ) {
            Stage::SetScene("Special Stage", "");
            sceneInfo->listPos += saveRAM->nextSpecialStage;
        }
        else {
            // Stage::SetScene("Adventure Mode", "Hidden Palace Zone");
            Stage::SetScene("Special Stage", "");
            sceneInfo->listPos += 7 + saveRAM->nextSpecialStage;
        }

        Zone::StartFadeOut(10, 0xF0F0F0);
        Music::Stop();
    }
}

#if RETRO_INCLUDE_EDITOR
void SpecialRing::EditorDraw() { this->warpAnimator.DrawSprite(nullptr, false); }

void SpecialRing::EditorLoad() { sVars->aniFrames.Load("Global/SpecialRing.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void SpecialRing::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(SpecialRing);

    sVars->aniFrames.Init();

    sVars->sfxSpecialRing.Init();
    sVars->sfxSpecialWarp.Init();
}
#endif

void SpecialRing::Serialize()
{
    RSDK_EDITABLE_VAR(SpecialRing, VAR_INT32, id);
    RSDK_EDITABLE_VAR(SpecialRing, VAR_INT32, planeFilter);
}

} // namespace GameLogic
