// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: InvincibleStars Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "InvincibleStars.hpp"
#include "Player.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(InvincibleStars);

void InvincibleStars::Update()
{
    this->starAnimator[0].Process();
    this->starAnimator[1].Process();
    this->starAnimator[2].Process();
    this->starAnimator[3].Process();
    this->starAnimator[4].Process();

    this->starFrame[0] = (this->starAngle[0] + 1) % 12;
    this->starFrame[1] = (this->starAngle[1] + 1) % 10;

    for (int32 i = 8 - 1; i > 0; --i) {
        this->starPos[i].x = this->starPos[i - 1].x;
        this->starPos[i].y = this->starPos[i - 1].y;
    }

    Player *player = GameObject::Get<Player>(this->playerSlot);

    this->starPos[0].x = player->position.x;
    this->starPos[0].y = player->position.y;

    Hitbox *playerHitbox = player->GetHitbox();
    if (player->direction & FLIP_X)
        this->starPos[0].x += (playerHitbox->left << 15) - (playerHitbox->right << 15) - (playerHitbox->left << 16);
    else
        this->starPos[0].x += ((playerHitbox->right + 2 * playerHitbox->left) << 15) - (playerHitbox->left << 15);

    if ((player->direction & FLIP_Y) && !player->invertGravity)
        this->starPos[0].y += (playerHitbox->top << 15) - (playerHitbox->bottom << 15) - (playerHitbox->top << 16);
    else
        this->starPos[0].y += ((playerHitbox->bottom + 2 * playerHitbox->top) << 15) - (playerHitbox->top << 15);

    if (player->direction) {
        this->starAngle[0] -= 144;
        this->starAngle[1] -= 16;
    }
    else {
        this->starAngle[0] += 144;
        this->starAngle[1] += 16;
    }

    this->starAngle[0] &= 0x1FF;
    this->starAngle[1] &= 0x1FF;

    if (player->invincibleTimer < 32)
        this->alpha = 8 * player->invincibleTimer;

    this->drawGroup  = player->drawGroup;
    this->visible    = player->visible;
    this->starOffset = 11;

    if (player->superState == Player::SuperStateSuper)
        this->active = ACTIVE_NEVER;
}
void InvincibleStars::LateUpdate() {}
void InvincibleStars::StaticUpdate() {}
void InvincibleStars::Draw()
{
    Player *player = GameObject::Get<Player>(this->playerSlot);
    if (player->isChibi) {
        this->drawFX |= FX_SCALE;
        this->scale.x = 0x100;
        this->scale.y = 0x100;
    }
    else {
        if (player->drawFX & FX_SCALE)
            this->drawFX |= FX_SCALE;
        else
            this->drawFX &= ~FX_SCALE;
        this->scale.x = player->scale.x;
        this->scale.y = player->scale.y;
    }

    Vector2 drawPos;
    drawPos.x = (Math::Cos512(this->starAngle[1] + 0x74) << this->starOffset) + this->starPos[7].x;
    drawPos.y = (Math::Sin512(this->starAngle[1] + 0x74) << this->starOffset) + this->starPos[7].y;
    this->starAnimator[3].DrawSprite(&drawPos, false);

    drawPos.x = (Math::Cos512(this->starAngle[1] + 0x174) << this->starOffset) + this->starPos[7].x;
    drawPos.y = (Math::Sin512(this->starAngle[1] + 0x174) << this->starOffset) + this->starPos[7].y;
    this->starAnimator[3].DrawSprite(&drawPos, false);

    drawPos.x = (Math::Cos512(this->starAngle[1]) << this->starOffset) + this->starPos[5].x;
    drawPos.y = (Math::Sin512(this->starAngle[1]) << this->starOffset) + this->starPos[5].y;
    this->starAnimator[2].DrawSprite(&drawPos, false);

    drawPos.x = (Math::Cos512(this->starAngle[1] + 0x100) << this->starOffset) + this->starPos[5].x;
    drawPos.y = (Math::Sin512(this->starAngle[1] + 0x100) << this->starOffset) + this->starPos[5].y;
    this->starAnimator[2].DrawSprite(&drawPos, false);

    drawPos.x = (Math::Cos512(this->starAngle[1] + 0xA8) << this->starOffset) + this->starPos[3].x;
    drawPos.y = (Math::Sin512(this->starAngle[1] + 0xA8) << this->starOffset) + this->starPos[3].y;
    this->starAnimator[1].DrawSprite(&drawPos, false);

    drawPos.x = (Math::Cos512(this->starAngle[1] + 0x1A8) << this->starOffset) + this->starPos[3].x;
    drawPos.y = (Math::Sin512(this->starAngle[1] + 0x1A8) << this->starOffset) + this->starPos[3].y;
    this->starAnimator[1].DrawSprite(&drawPos, false);

    drawPos.x = (Math::Cos512(this->starAngle[0]) << this->starOffset) + this->starPos[0].x;
    drawPos.y = (Math::Sin512(this->starAngle[0]) << this->starOffset) + this->starPos[0].y;
    this->starAnimator[0].DrawSprite(&drawPos, false);

    // changing animator for big red star lol
    drawPos.x = (Math::Cos512(this->starAngle[0] + 0x100) << this->starOffset) + this->starPos[0].x;
    drawPos.y = (Math::Sin512(this->starAngle[0] + 0x100) << this->starOffset) + this->starPos[0].y;
    this->starAnimator[4].DrawSprite(&drawPos, false);
}

void InvincibleStars::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        Player *player = (Player *)data;

        this->active     = ACTIVE_NORMAL;
        this->visible    = true;
        this->playerSlot = player->Slot();
        for (int32 i = 0; i < 8; ++i) {
            this->starPos[i].x = player->position.x;
            this->starPos[i].y = player->position.y;
        }

        this->drawFX       = FX_FLIP;
        this->inkEffect    = globals->useManiaBehavior ? INK_ADD : INK_ALPHA;
        this->starAngle[0] = 180;
        this->starAngle[1] = 0;
        this->alpha        = 0xFF;
        this->starAnimator[0].SetAnimation(sVars->aniFrames, 0, true, 0);
        this->starAnimator[1].SetAnimation(sVars->aniFrames, 1, true, 0);
        this->starAnimator[2].SetAnimation(sVars->aniFrames, 2, true, 0);
        this->starAnimator[3].SetAnimation(sVars->aniFrames, 3, true, 0);
        this->starAnimator[4].SetAnimation(sVars->aniFrames, 4, true, 0);
    }
}

void InvincibleStars::StageLoad()
{
    sVars->aniFrames.Load("Global/Invincible.bin", SCOPE_STAGE);
}

#if RETRO_INCLUDE_EDITOR
void InvincibleStars::EditorDraw() {}

void InvincibleStars::EditorLoad() {}
#endif

#if RETRO_REV0U
void InvincibleStars::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(InvincibleStars);

    sVars->aniFrames.Init();
}
#endif

void InvincibleStars::Serialize() {}

} // namespace GameLogic