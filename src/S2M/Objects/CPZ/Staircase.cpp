// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Staircase Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Staircase.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Staircase);

void Staircase::Update() { this->state.Run(this); }

void Staircase::LateUpdate() {}

void Staircase::StaticUpdate() {}

void Staircase::Draw() { this->stateDraw.Run(this); }

void Staircase::Create(void *data)
{
    this->visible       = true;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x1000000;
    this->updateRange.y = 0x1000000;

    // Why is a static value being initialized in Create?
    sVars->blockHitbox.top    = -16;
    sVars->blockHitbox.left   = -16;
    sVars->blockHitbox.bottom = 16;
    sVars->blockHitbox.right  = 16;

    int32 add = this->direction ? -0x200000 : 0x200000;

    for (int32 i = 0; i < STAIRCASE_STAIR_COUNT; ++i) {
        this->blockPos[i].x = this->position.x + i * add;
        this->blockPos[i].y = this->position.y;
    }

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    this->state.Set(&Staircase::State_Idle);
    this->stateDraw.Set(&Staircase::Draw_Blocks);
}

void Staircase::StageLoad()
{
    sVars->aniFrames.Load("CPZ/Staircase.bin", SCOPE_STAGE);
    
    // What a name tbh
    // apparently this sound doesnt actually exist in the game?? oh well
    sVars->sfxBlockStop.Get("CPZ/CPZ2HitBlocksStop.wav");
    Soundboard::LoadSfx(Staircase::SfxCheck_HitBlocks, nullptr);
}

Soundboard::SoundInfo Staircase::SfxCheck_HitBlocks()
{
    SoundFX hitBlocksSFX;
    hitBlocksSFX.Get("CPZ/CPZ2HitBlocks.wav");
    Soundboard::SoundInfo info = {};
    info.sfx                   = hitBlocksSFX;
    info.loopPoint             = true;

    for (auto entity : GameObject::GetEntities<Staircase>(FOR_ACTIVE_ENTITIES))
    {
        if (entity->stateDraw.Matches(&Staircase::Draw_Shake))
            //return true;
            info.playFlags = true;
            return info;
    }

    //return false;
    info.playFlags = false;
    return info;
}

void Staircase::State_Idle()
{
    Vector2 posStore = this->position;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        for (int32 i = 0; i < STAIRCASE_STAIR_COUNT; i++) {
            this->position = this->blockPos[i];
            int32 side     = player->CheckCollisionBox(this, &sVars->blockHitbox);
            if (side == C_BOTTOM) {
                // bumpable would prolly be used here :D
                this->active    = ACTIVE_NORMAL;
                this->timer     = 59;
                this->state.Set(&Staircase::State_Wait);
                this->stateDraw.Set(&Staircase::Draw_Shake);
                if (player->onGround)
                    player->deathType = Player::DeathDie_Sfx;
            }
            else if (side == C_TOP) {
                this->active = ACTIVE_NORMAL;
                this->timer  = 32;
                this->state.Set(&Staircase::State_Wait);
            }
        }
    }

    this->position = posStore;
}

void Staircase::State_Wait()
{
    Vector2 posStore = this->position;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        for (int32 i = 0; i < STAIRCASE_STAIR_COUNT; i++) {
            this->position = this->blockPos[i];
            player->CheckCollisionBox(this, &sVars->blockHitbox);
        }
    }
    this->position = posStore;

    if (--this->timer < 0) {
        this->timer = 128;
        if (!sVars->sfxBlockStop.IsPlaying())
            sVars->sfxBlockStop.Play(false, 255);
        this->state    .Set(&Staircase::State_MoveBlocks);
        this->stateDraw.Set(&Staircase::Draw_Blocks);
    }

    if (!this->CheckOnScreen(&this->updateRange))
        Staircase::Create(nullptr);
}

void Staircase::State_MoveBlocks()
{
    int32 add        = this->type ? -0x4000 : 0x4000;
    Vector2 posStore = this->position;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        for (int32 i = 0; i < STAIRCASE_STAIR_COUNT; i++) {
            this->position = this->blockPos[i];
            if (player->CheckCollisionBox(this, &sVars->blockHitbox) == C_TOP)
                player->position.y += add * (i + 1);
        }
    }
    this->position = posStore;

    for (int32 i = 0; i < STAIRCASE_STAIR_COUNT; ++i) this->blockPos[i].y += (i + 1) * add;

    if (--this->timer <= 0)
        this->state.Set(&Staircase::State_MovedBlocks);

    if (!this->CheckOnScreen(&this->updateRange))
        Staircase::Create(nullptr);
}

void Staircase::State_MovedBlocks()
{
    Vector2 posStore = this->position;
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        for (int32 i = 0; i < STAIRCASE_STAIR_COUNT; i++) {
            this->position = this->blockPos[i];
            if (player->CheckCollisionBox(this, &sVars->blockHitbox) == C_BOTTOM && player->onGround)
                player->deathType = Player::DeathDie_Sfx;
        }
    }
    this->position = posStore;

    if (!this->CheckOnScreen(&this->updateRange))
        Staircase::Create(nullptr);
}

void Staircase::Draw_Blocks()
{
    for (int32 i = 0; i < STAIRCASE_STAIR_COUNT; i++) {
        this->animator.DrawSprite(&this->blockPos[i], false);
    }
}

void Staircase::Draw_Shake()
{
    for (int32 i = 0; i < STAIRCASE_STAIR_COUNT; i++) {
        Vector2 drawPos = this->blockPos[i];
        drawPos.y += ((this->timer - i * STAIRCASE_STAIR_COUNT) << 14) & 0x10000;
        this->animator.DrawSprite(&drawPos, false);
    }
}

#if RETRO_INCLUDE_EDITOR
void Staircase::EditorDraw()
{
    int32 addX = this->direction ? -0x200000 : 0x200000;

    for (int32 i = 0; i < STAIRCASE_STAIR_COUNT; ++i) {
        this->blockPos[i].x = this->position.x + i * addX;
        this->blockPos[i].y = this->position.y;
    }

    Staircase::Draw_Blocks();

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        this->inkEffect = INK_BLEND;

        int32 addY = this->type ? -0x200000 : 0x200000;
        for (int32 i = 0; i < STAIRCASE_STAIR_COUNT; ++i) this->blockPos[i].y += (i + 1) * addY;

        Staircase::Draw_Blocks();

        this->inkEffect = INK_NONE;

        RSDK_DRAWING_OVERLAY(false);
    }
}

void Staircase::EditorLoad()
{
    sVars->aniFrames.Load("CPZ/Staircase.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Right", FLIP_NONE);
    RSDK_ENUM_VAR("Left", FLIP_X);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Down", STAIRCASE_DOWN);
    RSDK_ENUM_VAR("Up", STAIRCASE_UP);
}
#endif

void Staircase::Serialize()
{
    RSDK_EDITABLE_VAR(Staircase, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(Staircase, VAR_UINT8, type);
    // fun fact: why is this never used
    RSDK_EDITABLE_VAR(Staircase, VAR_BOOL, bumpable);
}

} // namespace GameLogic