// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Collectable Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Collectable.hpp"
#include "HP_Setup.hpp"
#include "HP_Halfpipe.hpp"
#include "HP_Player.hpp"
#include "HP_Message.hpp"
#include "HP_SpecialClear.hpp"
#include "Global/Zone.hpp"
#include "Global/Music.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Collectable);

void HP_Collectable::Update() { this->state.Run(this); }
void HP_Collectable::LateUpdate() {}
void HP_Collectable::StaticUpdate() {}
void HP_Collectable::Draw() { this->stateDraw.Run(this); }

void HP_Collectable::Create(void *data)
{
    if (data)
        this->type = (uint8)VOID_TO_INT(data) - 1;

    this->visible = true;
    this->active  = ACTIVE_NEVER;

    if (!sceneInfo->inEditor) {
        this->drawGroup = Zone::sVars->objectDrawGroup[0];

        this->angle = (FROM_FIXED(this->position.x) - 448) & 0x1FF;

        this->localPos.x = (Math::Sin512(this->angle) * -48) >> 1;
        this->localPos.y = (Math::Cos512(this->angle) * -48) >> 1;

        this->shadowAnimator.SetAnimation(HP_Halfpipe::sVars->shadowFrames, 0, true, 0);
        this->animator.SetAnimation(sVars->aniFrames, this->type, true, 0);

        this->stateDraw.Set(nullptr);
        switch (this->type) {
            default: break;

            case HP_Collectable::Ring:
                this->state.Set(&HP_Collectable::State_Ring);
                if (this->localPos.y < -0x400) {
                    this->shadowsEnabled = true;

                    this->localShadowPos.x = (Math::Sin512(this->angle) * -60) >> 1;
                    this->localShadowPos.y = (Math::Cos512(this->angle) * -60) >> 1;

                    this->shadowAnimator.frameID = 0;
                    if (this->angle > 32 && this->angle < 480) {
                        if (this->angle < 256)
                            this->shadowAnimator.frameID = 1;
                        else
                            this->shadowAnimator.frameID = 2;
                    }
                }
                break;

            case HP_Collectable::Bomb:
                this->state.Set(&HP_Collectable::State_Bomb);
                if (this->localPos.y < -0x800) {
                    this->shadowsEnabled = true;

                    this->localShadowPos.x = (Math::Sin512(this->angle) * -60) >> 1;
                    this->localShadowPos.y = (Math::Cos512(this->angle) * -60) >> 1;

                    this->shadowAnimator.frameID = 0;
                    if (this->angle > 32 && this->angle < 480) {
                        if (this->angle < 256)
                            this->shadowAnimator.frameID = 1;
                        else
                            this->shadowAnimator.frameID = 2;
                    }
                }
                break;

            case HP_Collectable::Emerald:
                this->state.Set(&HP_Collectable::State_EmeraldTrigger);
                this->drawFX = FX_SCALE;
                break;
        }
    }
}

void HP_Collectable::StageLoad()
{
    sVars->aniFrames.Load("Special/StageObjects.bin", SCOPE_STAGE);

    // if (globals->secrets & SECRET_NOITEMS) {
    //     for (auto ring : GameObject::GetEntities<HP_Collectable>(FOR_ALL_ENTITIES)) ring->Destroy();
    // }

    sVars->sfxRing.Get("Global/Ring.wav");
    sVars->sfxBomb.Get("Stage/LedgeBreak.wav");
    sVars->sfxEmerald.Get("Special/Emerald.wav"); // TODO (this one's for you AChickMcNuggie...): does not exist rn ig???
}

void HP_Collectable::State_Ring()
{
    SET_CURRENT_STATE();

    this->Draw_Ring();

    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);

    if (this->localPos.z < halfpipe->playerZPos) {
        this->Destroy();
    }
    else {
        for (auto player : GameObject::GetEntities<HP_Player>(FOR_ACTIVE_ENTITIES)) {
            if (player->localPos.z > this->localPos.z - 0x20000 && player->localPos.z < this->localPos.z + 0x20000) {
                int32 rx = player->localPos.x - this->localPos.x;
                int32 rz = player->localPos.y - this->localPos.y;

                if (rx * rx + rz * rz < TO_FIXED(196)) {
                    player->GiveRings(1, true);

                    this->animator.SetAnimation(sVars->aniFrames, HP_Collectable::RingSparkle, true, 0);
                    this->state.Set(&HP_Collectable::State_Sparkle);
                }
            }
        }
    }
}

void HP_Collectable::State_Bomb()
{
    SET_CURRENT_STATE();

    this->Draw_Bomb();

    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);

    if (this->localPos.z < halfpipe->playerZPos) {
        this->Destroy();
    }
    else {
        for (auto player : GameObject::GetEntities<HP_Player>(FOR_ACTIVE_ENTITIES)) {
            if (player->localPos.z > this->localPos.z - 0x20000 && player->localPos.z < this->localPos.z + 0x20000) {
                int32 rx = player->localPos.x - this->localPos.x;
                int32 rz = player->localPos.y - this->localPos.y;

                if (rx * rx + rz * rz < TO_FIXED(196)) {
                    player->Hurt();
                    sVars->sfxBomb.Play();
                    this->animator.SetAnimation(sVars->aniFrames, HP_Collectable::Explosion, true, 0);
                    this->state.Set(&HP_Collectable::State_Explosion);
                }
            }
        }
    }
}
void HP_Collectable::State_EmeraldTrigger()
{
    SET_CURRENT_STATE();

    HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
    HP_Player *player2 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER2);
    int32 totalRings   = player1->rings + (GET_CHARACTER_ID(2) ? player2->rings : 0);

    GameObject::Reset(SLOT_HP_MESSAGE, 0, 0);

    for (auto player : GameObject::GetEntities<HP_Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->localPos.z > this->localPos.z - 0x20000 && player->localPos.z < this->localPos.z + 0x20000) {
            this->scale.x = 0x000;
            this->scale.y = 0x000;
            this->drawGroup = Zone::sVars->hudDrawGroup;
            this->animator.frameID = Zone::sVars->actID % 7;

            this->notEnoughRings = true;
            if (totalRings >= HP_Setup::sVars->ringCounts[HP_Setup::sVars->checkpointID]) {
                this->notEnoughRings = false;
            }

            this->emeraldPos.x = 0x14000;

            this->state.Set(&HP_Collectable::State_EmeraldTryCollect);
            Music::FadeOut(0.0078125f); //  4 / 0x200
        }
    }

    if (Zone::sVars->actID == HP_Setup::Act7) {
        int32 totalRings     = player1->rings + (GET_CHARACTER_ID(2) ? player2->rings : 0);
        if (totalRings >= HP_Setup::sVars->ringCounts[HP_Setup::sVars->checkpointID]) {
            this->type = (this->type + 1) & 1;
            if (!this->type)
                paletteBank[0].Rotate(129, 141, false);
        }
    }
}
void HP_Collectable::State_EmeraldTryCollect()
{
    SET_CURRENT_STATE();

    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);

    this->emeraldPos.y = Math::Sin512(this->angle) - 0x1C00;
    this->angle        = (this->angle + 12) & 0x1FF;

    if (this->notEnoughRings) {
        if (this->scale.x < 0x180) {
            this->scale.x += 4;
            this->emeraldPos.x -= 0x200;
        }
        else {
            Music::Stop();

            HP_Message *message = GameObject::Get<HP_Message>(SLOT_HP_MESSAGE);
            message->Reset(HP_Message::sVars->classID, 0);
            message->position.y = TO_FIXED(116);
            message->SetMessage(&HP_Message::State_NotEnough, 0, 120, false, "NOT ENOUGH", "RINGS ... ", nullptr);

            this->state.Set(&HP_Collectable::State_EmeraldHandleFinish);
        }

        if (halfpipe[0].moveSpeed > 4) {
            if (!this->animator.timer)
                halfpipe[0].moveSpeed--;

            this->animator.timer = (this->animator.timer + 1) & 7;
        }
    }
    else {
        if (Zone::sVars->actID == HP_Setup::Act7) {
            this->type = (this->type + 1) & 1;
            if (!this->type)
                paletteBank[0].Rotate(129, 141, false);
        }

        if (this->scale.x < 0x200) {
            this->scale.x += 4;
            this->emeraldPos.x -= 0x200;
        }
        else {
            Music::Stop();
            sVars->sfxEmerald.Play();
            HP_Setup::sVars->gotEmerald = true;
            this->state.Set(&HP_Collectable::State_EmeraldHandleFinish);
        }
    }
    this->scale.y = this->scale.x;

    this->Draw_Emerald();
}
void HP_Collectable::State_EmeraldHandleFinish()
{
    SET_CURRENT_STATE();
    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);

    this->emeraldPos.y = Math::Sin512(this->angle) - 0x1C00;
    this->angle        = (this->angle + 12) & 0x1FF;

    this->Draw_Emerald();

    if (this->notEnoughRings) {
        if (halfpipe[0].moveSpeed > 4) {
            if (!this->animator.timer)
                halfpipe[0].moveSpeed--;

            this->animator.timer = (this->animator.timer + 1) & 7;
        }

        if (this->scale.x > 0) {
            this->scale.x -= 2;
            this->emeraldPos.x += 0x100;
        }
    }
    else {
        if (Zone::sVars->actID == HP_Setup::Act7) {
            this->type = (this->type + 1) & 1;
            if (!this->type) 
                paletteBank[0].Rotate(129, 141, false);
        }
    }

    this->scale.y = this->scale.x;

    if (++this->timer == 180) {
        GameObject::Reset(SLOT_HP_SPECIALCLEAR, HP_SpecialClear::sVars->classID, 0);
    }
}

void HP_Collectable::State_Sparkle()
{
    SET_CURRENT_STATE();
    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);

    // Draw Sprite
    HP_Halfpipe::DrawSprite(this->worldPos.x, this->worldPos.y, this->worldPos.z, this->drawFX, 0x200, 0x200, 0, &this->animator, sVars->aniFrames, false);

    this->animator.Process();

    if (this->localPos.z < halfpipe->playerZPos || this->animator.frameID == this->animator.frameCount - 1) 
        this->Destroy();
}
void HP_Collectable::State_Lost()
{
    SET_CURRENT_STATE();

    this->velocity.y += this->gravityStrength;

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    if (this->scale.x < 0x200)
        this->scale.x += 0x10;
    this->scale.y = this->scale.x;

    this->animator.Process();

    if (this->timer++ >= 90)
        this->Destroy();
}
void HP_Collectable::State_Explosion()
{
    SET_CURRENT_STATE();
    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);

    // Draw Sprite
    HP_Halfpipe::DrawSprite(this->worldPos.x, this->worldPos.y, this->worldPos.z, this->drawFX, 0x200, 0x200, 0, &this->animator, sVars->aniFrames,
                            false);

    this->animator.Process();

    if (this->localPos.z < halfpipe->playerZPos || this->animator.frameID == this->animator.frameCount - 1)
        this->Destroy();
}

void HP_Collectable::Draw_Ring()
{
    SET_CURRENT_STATE();

    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);

    int32 scale = MAX(this->position.y - (halfpipe->position.y - 0xE00000), 0);
    if (scale > 0)
        scale >>= 14;
    scale += 0x400;

    // Draw Shadow
    if (this->shadowsEnabled) {
        HP_Halfpipe::DrawShadow(this->worldShadowPos.x, this->worldShadowPos.y, this->worldShadowPos.z, scale, scale, &this->shadowAnimator, false);
    }

    // Draw Sprite
    this->animator.frameID = Zone::sVars->ringFrame >> 1;
    HP_Halfpipe::DrawTexture(this->worldPos.x, this->worldPos.y, this->worldPos.z, scale, scale, &this->animator, sVars->aniFrames, false);
}
void HP_Collectable::Draw_Bomb()
{
    SET_CURRENT_STATE();

    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);

    int32 scale = MAX(this->position.y - (halfpipe->position.y - 0xE00000), 0);
    if (scale > 0)
        scale >>= 14;
    scale += 0x400;

    // Draw Shadow
    if (this->shadowsEnabled) {
        HP_Halfpipe::DrawShadow(this->worldShadowPos.x, this->worldShadowPos.y, this->worldShadowPos.z, scale, scale, &this->shadowAnimator, false);
    }

    scale = MAX(this->position.y - (halfpipe->position.y - 0x1000000), 0);
    if (scale > 0)
        scale >>= 14;
    scale += 0x600;

    // Draw Sprite
    HP_Halfpipe::DrawTexture(this->worldPos.x, this->worldPos.y, this->worldPos.z, scale, scale, &this->animator, sVars->aniFrames, false);
}
void HP_Collectable::Draw_Emerald()
{
    SET_CURRENT_STATE();

    auto *scene3D = &HP_Halfpipe::sVars->scene3D;

    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);
    HP_Halfpipe::MatrixRotateXYZ(&scene3D->matWorld, halfpipe->worldRotation.x, halfpipe->worldRotation.y, halfpipe->worldRotation.z);
    HP_Halfpipe::MatrixTranslateXYZ(&scene3D->matTemp, halfpipe->worldTranslation.x, halfpipe->worldTranslation.y, halfpipe->worldTranslation.z);
    HP_Halfpipe::MatrixMultiply(&scene3D->matWorld, &scene3D->matWorld, &scene3D->matTemp);

    // Draw Shadow
    HP_Halfpipe::DrawShadow(0, -0x3800, this->emeraldPos.x, 0x600, 0x600, &this->shadowAnimator, true);

    // Draw Emerald
    HP_Halfpipe::DrawSprite(0, this->emeraldPos.y, this->emeraldPos.x, this->drawFX, this->scale.x, this->scale.y, this->rotation, &this->animator,
                            sVars->aniFrames, true);
}

void HP_Collectable::Draw_LostRing()
{
    SET_CURRENT_STATE();

    Vector2 drawPos = this->position;
    drawPos.x += TO_FIXED(screenInfo[sceneInfo->currentScreenID].center.x);

    this->animator.DrawSprite(&drawPos, true);
}

void HP_Collectable::LoseRings(RSDK::Vector2 *position, int32 lossAngle, int32 rings, uint8 drawGroup)
{
    HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);

    int32 ringCount = CLAMP(rings, 0, 10);

    int32 angle = 0x180 - (ringCount >> 1 << 5) + (lossAngle >> 7);
    int32 x     = FROM_FIXED(position->x) * 224;
    int32 y     = (FROM_FIXED(position->y) * -220) + 0x600000 + (FROM_FIXED(player1->position.y) * -48);

    for (int32 i = 0; i < ringCount; ++i) {
        HP_Collectable *ring  = GameObject::Create<HP_Collectable>(1 + HP_Collectable::Ring, x, y);
        ring->velocity.x      = Math::Cos512(angle) << 9;
        ring->velocity.y      = Math::Sin512(angle) << 10;
        ring->gravityStrength = 0x7000 + Math::Rand(0, 0x2000);
        ring->scale.x         = 0x140;
        ring->scale.y         = 0x140;
        ring->drawFX          = FX_SCALE;
        ring->animator.speed += Math::Rand(0, 128);
        ring->inkEffect   = INK_ALPHA;
        ring->alpha       = 0x100;
        ring->active      = ACTIVE_NORMAL;
        ring->isPermanent = true;
        ring->state.Set(&HP_Collectable::State_Lost);
        ring->stateDraw.Set(&HP_Collectable::Draw_LostRing);
        ring->drawGroup = drawGroup;

        angle += 0x10;
    }
}

#if RETRO_INCLUDE_EDITOR
void HP_Collectable::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, HP_Collectable::EditorIcons, true, this->type);
    this->animator.DrawSprite(nullptr, false);
}

void HP_Collectable::EditorLoad()
{
    sVars->aniFrames.Load("Special/StageObjects.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Ring");
    RSDK_ENUM_VAR("Bomb");
    RSDK_ENUM_VAR("Emerald");
}
#endif

#if RETRO_REV0U
void HP_Collectable::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(HP_Collectable);

    sVars->aniFrames.Init();

    sVars->sfxRing.Init();
    sVars->sfxBomb.Init();
    sVars->sfxEmerald.Init();
}
#endif

void HP_Collectable::Serialize() { RSDK_EDITABLE_VAR(HP_Collectable, VAR_UINT8, type); }

} // namespace GameLogic