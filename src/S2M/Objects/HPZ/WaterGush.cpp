// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: WaterGush Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "WaterGush.hpp"
#include "Global/Player.hpp"
#include "Global/DebugMode.hpp"
#include "Global/Zone.hpp"
#include "Global/Music.hpp"
#include "Global/Debris.hpp"
#include "Common/Water.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(WaterGush);

void WaterGush::Update()
{
    WaterGush::SetupHitboxes();

    bool32 wasActivated = this->activated;
    this->direction     = FLIP_NONE;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) 
    {
        int32 playerID = RSDKTable->GetEntitySlot(player);
        if (!((1 << playerID) & this->activePlayers)) {
            if (player->CheckCollisionTouch(this, &this->hitboxGush)) {
                this->active = ACTIVE_NORMAL;

                if (!player->sidekick)
                    this->activated = true;

                sVars->sfxGush.Play(false, 255);
                this->activePlayers |= 1 << playerID;

                player->animator.SetAnimation(player->aniFrames, Player::ANI_HURT, true, 6);
                player->nextGroundState.Set(nullptr);
                player->nextAirState.Set(nullptr);
                player->velocity.x      = 0;
                player->velocity.y      = 0;
                player->tileCollisions  = TILECOLLISION_NONE;
                player->onGround        = false;
                player->state.Set(&Player::State_Static);
            }
        }

        if (((1 << playerID) & this->activePlayers)) {
            int32 xDir = 0;
            int32 yDir = 0;

            int32 offsetX = 0;
            int32 offsetY = 0;
            yDir    = -1;
            offsetX = this->position.x;
            offsetY = this->position.y - this->gushPos - 0x140000;

            player->position.x += (offsetX - player->position.x) >> 2;
            player->position.y += (offsetY - player->position.y) >> 2;
            player->state.Set(&Player::State_Static);

            if ((!player->CheckCollisionTouch(this, &this->hitboxRange) && !player->CheckCollisionTouch(this, &this->hitboxGush))
                || this->finishedExtending) {
                this->activePlayers &= ~(1 << playerID);
                player->state.Set(&Player::State_Air);
                player->tileCollisions = TILECOLLISION_DOWN;
                player->onGround       = false;
                player->velocity.x     = xDir * (abs(this->speed) << 15);
                player->velocity.y     = yDir * (abs(this->speed) << 15);
            }
        }
    }

    if (this->activated) {
        if (!wasActivated) {
            Water::sVars->sfxSplash.Play(false, 255);
        }

        if (this->finishedExtending) {
            this->gravityStrength += 0x3800;

            if (this->gushPos > 0)
                this->gushPos -= this->gravityStrength;

            this->gushPos = MAX(this->gushPos, 0);
            if (!this->gushPos) {
                this->activated         = false;
                this->finishedExtending = false;
                this->gravityStrength   = 0;
            }
        }
        else {
            if (this->gushPos < this->length << 22)
                this->gushPos += (abs(this->speed) << 15);

            this->gushPos = MIN(this->length << 22, this->gushPos);
            if (this->gushPos == this->length << 22)
                this->finishedExtending = true;
        }
    }

    if (!this->CheckOnScreen(NULL)) {
        this->gushPos           = 0;
        this->activated         = false;
        this->finishedExtending = false;
        this->gravityStrength   = 0;
        this->active            = ACTIVE_BOUNDS;
    }

    this->plumeAnimator.Process();
    this->topAnimator.Process();
}

void WaterGush::LateUpdate() {}

void WaterGush::StaticUpdate() {}

void WaterGush::Draw()
{
    if (this->gushPos > 0)
        WaterGush::DrawSprites();
}

void WaterGush::Create(void *data)
{
    this->active     = ACTIVE_BOUNDS;
    this->drawGroup  = Zone::sVars->objectDrawGroup[0];
    this->startPos.x = this->position.x;
    this->startPos.y = this->position.y;
    this->visible    = true;
    this->drawFX     = FX_FLIP;

    this->updateRange.x = 0x800000;

    this->updateRange.y = 0x800000;
    this->updateRange.y = (this->length + 2) << 22;

    if (!this->speed)
        this->speed = 16;

    WaterGush::SetupHitboxes();

    this->direction = FLIP_NONE;
    this->plumeAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->topAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
}

void WaterGush::StageLoad()
{
    sVars->aniFrames.Load("HPZ/WaterGush.bin", SCOPE_STAGE);
    sVars->sfxGush.Get("Stage/WaterGush.wav");
}

void WaterGush::SetupHitboxes()
{
    this->hitboxRange.left   = -32;
    this->hitboxRange.top    = -20 - (this->gushPos >> 16);
    this->hitboxRange.right  = 32;
    this->hitboxRange.bottom = 0;

    this->hitboxGush.left   = -32;
    this->hitboxGush.top    = 0;
    this->hitboxGush.right  = 32;
    this->hitboxGush.bottom = 16;
}

void WaterGush::DrawSprites()
{
    uint8 storeDir     = this->direction;
    Vector2 drawPosTop = this->position;

    int32 offsetX = 0;
    int32 offsetY = 0;
    this->direction = FLIP_NONE;
    drawPosTop.y -= this->gushPos;
    offsetX = 0;
    offsetY = 0x400000;

    Vector2 drawPos = drawPosTop;
    if (this->gushPos > 0) {
        int32 count = ((this->gushPos - 1) >> 22) + 1;
        for (int32 i = 0; i < count; ++i) {
            drawPos.x += offsetX;
            drawPos.y += offsetY;
            this->plumeAnimator.DrawSprite(&drawPos, false);
        }
    }

    this->topAnimator.DrawSprite(&drawPosTop, false);

    this->direction = storeDir;
}

#if RETRO_INCLUDE_EDITOR
void WaterGush::EditorDraw()
{
    this->updateRange.x = 0x800000;

    this->updateRange.y = 0x800000;
    this->updateRange.y = (this->length + 2) << 22;

    this->direction = FLIP_NONE;
    this->plumeAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->topAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

    this->gushPos = this->length << 22;

    WaterGush::DrawSprites();
}

void WaterGush::EditorLoad(void)
{
    sVars->aniFrames.Load("HPZ/WaterGush.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, orientation);
    RSDK_ENUM_VAR("Up");
    RSDK_ENUM_VAR("Right");
    RSDK_ENUM_VAR("Left");
}
#endif

void WaterGush::Serialize(void)
{
    RSDK_EDITABLE_VAR(WaterGush, VAR_UINT32, length);
    RSDK_EDITABLE_VAR(WaterGush, VAR_INT32, speed);
}

} // namespace GameLogic