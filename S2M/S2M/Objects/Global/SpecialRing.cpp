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
// #include "Common/Water.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SpecialRing);

void SpecialRing::Update() { this->state.Run(this); }
void SpecialRing::LateUpdate() {}
void SpecialRing::StaticUpdate() {}
void SpecialRing::Draw()
{

    if (this->state.Matches(&SpecialRing::State_Flash)) {
        this->direction = this->warpAnimator.frameID > 8;
        this->warpAnimator.DrawSprite(NULL, false);
    }
    else {
        sVars->sceneIndex.Prepare();
        if (this->enabled)
            sVars->sceneIndex.AddModel(sVars->modelIndex, Graphics::Scene3D::SolidColor_Shaded_Blended, &this->matWorld, &this->matNormal, 0xF0F000);
        else
            sVars->sceneIndex.AddModel(sVars->modelIndex, Graphics::Scene3D::Wireframe_Shaded, &this->matWorld, &this->matNormal, 0x609090);
        sVars->sceneIndex.Draw();
    }
}

void SpecialRing::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active        = ACTIVE_BOUNDS;
        this->visible       = true;
        this->updateRange.x = (144 << 16);
        this->updateRange.y = (144 << 16);
        this->drawFX        = FX_FLIP;
        if (this->planeFilter > 0 && ((uint8)this->planeFilter - 1) & 2)
            this->drawGroup = Zone::sVars->objectDrawGroup[1];
        else
            this->drawGroup = Zone::sVars->objectDrawGroup[0];
        this->state.Set(&SpecialRing::State_Idle);
    }
}

void SpecialRing::StageLoad()
{
    sVars->aniFrames.Load("Global/SpecialRing.bin", SCOPE_STAGE);

    sVars->modelIndex.Load("Global/SpecialRing.bin", SCOPE_STAGE);
    sVars->sceneIndex.Create("View:SpecialRing", 512, SCOPE_STAGE);

    sVars->hitbox.left   = -18;
    sVars->hitbox.top    = -18;
    sVars->hitbox.right  = 18;
    sVars->hitbox.bottom = 18;

    // sets diffuse colour (overrides)
    sVars->sceneIndex.SetDiffuseColor(0xA0, 0xA0, 0xA0);
    // sets diffuse intensity (0-8 means more diffuse, any more and it'll start darkening to black (9-12), any greater than 11 is full black)
    sVars->sceneIndex.SetDiffuseIntensity(8, 8, 8);
    // sets specular (highlight) intensity (16-0, 16 = none, 0 = all)
    sVars->sceneIndex.SetSpecularIntensity(14, 14, 14);

    sVars->sfxSpecialRing.Get("Global/SpecialRing.wav");
    sVars->sfxSpecialWarp.Get("Global/SpecialWarp.wav");

    DebugMode::AddObject(sVars->classID, &SpecialRing::DebugSpawn, &SpecialRing::DebugDraw);

    for (auto ring : GameObject::GetEntities<SpecialRing>(FOR_ALL_ENTITIES)) {
        if (ring->id <= 0 || globals->gameMode == MODE_TIMEATTACK || globals->gameMode == MODE_COMPETITION || (globals->secrets & SECRET_NOITEMS)) {
            ring->enabled = false;
        }
        else {
            ring->enabled = !(SaveGame::sVars->saveRAM->collectedSpecialRings & (1 << ((16 * Zone::sVars->actID) + ring->id - 1)));
            if (globals->specialRingID == ring->id) {
                for (int32 p = 0; p < Player::sVars->playerCount; ++p) {
                    Player *player = GameObject::Get<Player>(p);

                    player->position.x = ring->position.x;
                    player->position.y = ring->position.y + 0x100000;
                    if (!p) {
                        Player *player2 = GameObject::Get<Player>(SLOT_PLAYER2);
                        if (globals->gameMode != MODE_COMPETITION) {
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
                }

                sceneInfo->milliseconds = globals->tempMilliseconds;
                sceneInfo->seconds      = globals->tempSeconds;
                sceneInfo->minutes      = globals->tempMinutes;
            }
        }
    }
}

void SpecialRing::DebugSpawn() { GameObject::Create<SpecialRing>(true, this->position.x, this->position.y); }

void SpecialRing::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 16);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void SpecialRing::State_Idle()
{
    SET_CURRENT_STATE();

    this->angleZ = (this->angleZ + 4) & 0x3FF;
    this->angleY = (this->angleY + 4) & 0x3FF;

    Vector2 range;
    range.x = (128 << 16);
    range.y = (128 << 16);
    if (!this->CheckOnScreen(&range))
        this->scale.x = 0;

    if (this->scale.x >= 0x140)
        this->scale.x = 0x140;
    else
        this->scale.x += ((0x168 - this->scale.x) >> 5);

    this->matTransform.ScaleXYZ(this->scale.x, this->scale.x, this->scale.x);
    this->matTransform.TranslateXYZ(this->position.x, this->position.y, 0, false);
    this->matWorld.RotateXYZ(0, this->angleY, this->angleZ);
    Matrix::Multiply(&this->matWorld, &this->matWorld, &this->matTransform);
    this->matTempRot.RotateX(0x1E0);
    this->matNormal.RotateXYZ(0, this->angleY, this->angleZ);
    Matrix::Multiply(&this->matNormal, &this->matNormal, &this->matTempRot);

    if (this->enabled && this->scale.x > 0x100) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if ((this->planeFilter <= 0 || player->collisionPlane == (((uint8)this->planeFilter - 1) & 1)) && !player->sidekick) {
                if (player->CheckCollisionTouch(this, &sVars->hitbox) && sceneInfo->timeEnabled) {
                    this->sparkleRadius = (16 << 16);
                    this->state.Set(&SpecialRing::State_Flash);

                    SaveGame *saveRAM = SaveGame::GetSaveRAM();
                    // rings spawned via debug mode give you 50 rings, always
                    if (saveRAM->chaosEmeralds != 0b01111111 && this->id) {
                        player->visible        = false;
                        player->active         = ACTIVE_NEVER;
                        sceneInfo->timeEnabled = false;
                    }
                    else {
                        player->GiveRings(50, true);
                    }

                    if (this->id > 0) {
                        if (saveRAM->chaosEmeralds != 0b01111111)
                            globals->specialRingID = this->id;

                        saveRAM->collectedSpecialRings |= 1 << (16 * Zone::sVars->actID - 1 + this->id);
                    }

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
            sparkle->animator.SetAnimation(sVars->aniFrames, i % 3 + 2, true, 0);
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

    if (SaveGame::GetSaveRAM()->chaosEmeralds == 0b01111111 || !this->id) {
        this->Destroy();
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

        SaveGame *saveRAM      = SaveGame::GetSaveRAM();
        saveRAM->storedStageID = sceneInfo->listPos;
        Stage::SetScene("Special Stage", "");
        sceneInfo->listPos += saveRAM->nextSpecialStage;
        if (globals->gameMode == MODE_ENCORE)
            sceneInfo->listPos += 7;
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
