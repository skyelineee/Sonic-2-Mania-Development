// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ChemicalBall Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "ChemicalBall.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/TimeAttackGate.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ChemicalBall);

void ChemicalBall::Update()
{
    if (globals->gameMode == MODE_TIMEATTACK && TimeAttackGate::sVars && TimeAttackGate::sVars->isFinished)
        this->Destroy();

    this->state.Run(this);
}

void ChemicalBall::LateUpdate(void) {}

void ChemicalBall::StaticUpdate(void) {}

void ChemicalBall::Draw() { this->animator.DrawSprite(&this->drawPos, false); }

void ChemicalBall::Create(void *data)
{
    this->visible       = true;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];
    this->startPos      = this->position;
    this->active        = ACTIVE_BOUNDS;
    this->updateRange.x = 0x1000000;
    this->updateRange.y = 0x1000000;

    if (!this->interval)
        this->interval = 128;

    this->drawPos = this->position;

    if (this->direction == FLIP_NONE)
        this->position.x += 0x320000;
    else
        this->position.x -= 0x320000;

    if (this->type == CHEMICALBALL_VERTICAL)
        this->position.y -= 0x480000;

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    if (!this->type)
        this->state.Set(&ChemicalBall::State_MoveArc);
    else
        this->state.Set(&ChemicalBall::State_MoveVertical);
}

void ChemicalBall::StageLoad()
{
    sVars->aniFrames.Load("CPZ/ChemicalBall.bin", SCOPE_STAGE);

    sVars->hitboxBall.left   = -8;
    sVars->hitboxBall.top    = -8;
    sVars->hitboxBall.right  = 8;
    sVars->hitboxBall.bottom = 8;

    sVars->sfxBloop.Get("Stage/Bloop.wav");
}

void ChemicalBall::CheckHit()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        Vector2 posStore = this->position;
        this->position   = this->drawPos;
        if (player->CheckCollisionTouch(this, &sVars->hitboxBall)) {
            this->position = posStore;
            player->Hurt(this);
        }
        else {
            this->position = posStore;
        }
    }
}

void ChemicalBall::State_MoveArc()
{
    int32 timer = (Zone::sVars->timer + this->intervalOffset) % (2 * this->interval);
    if (timer < this->interval == (this->direction != FLIP_NONE)) {
        if (!this->direction)
            timer -= this->interval;

        if (timer >= 96) {
            this->drawPos.x = sVars->arcOffsets[0];
            this->drawPos.y = sVars->arcOffsets[1];
        }
        else {
            this->drawPos.x = sVars->arcOffsets[2 * (97 - timer)];
            this->drawPos.y = sVars->arcOffsets[2 * (97 - timer) + 1];
        }
    }
    else {
        if (this->direction == FLIP_X)
            timer -= this->interval;

        if (timer >= 97) {
            this->drawPos.x = sVars->arcOffsets[194];
            this->drawPos.y = sVars->arcOffsets[195];
        }
        else {
            this->drawPos.x = sVars->arcOffsets[2 * timer];
            this->drawPos.y = sVars->arcOffsets[2 * timer + 1];
        }
    }

    this->drawPos.x += this->position.x;
    this->drawPos.y += this->position.y;

    if (!timer || timer == this->interval)
        sVars->sfxBloop.Play(false, 255);

    ChemicalBall::CheckHit();
}

void ChemicalBall::State_MoveVertical()
{
    int32 timer = (Zone::sVars->timer + this->intervalOffset) % this->interval;
    if (timer < 98) {
        if (timer >= 49) {
            if (!this->direction)
                this->drawPos.x = -0x320000;
            else
                this->drawPos.x = 0x2E0000;
            this->drawPos.y = sVars->moveOffsets[97 - timer];
        }
        else {
            if (!this->direction)
                this->drawPos.x = 0x2E0000;
            else
                this->drawPos.x = -0x320000;
            this->drawPos.y = sVars->moveOffsets[timer];
        }
    }
    this->drawPos.x += this->position.x;
    this->drawPos.y += this->position.y;

    if (timer == 27 || timer == 48)
        sVars->sfxBloop.Play(false, 255);

    ChemicalBall::CheckHit();
}

#if RETRO_REV0U
void ChemicalBall::StaticLoad(Static *sVars) 
{ 
    RSDK_INIT_STATIC_VARS(ChemicalBall);

    int32 arcOffsets[] = { -0x320000, 0, -0x320000, -0x48000,  -0x31F500, -0x8E800,  -0x31DF00, -0xD3800,  -0x31BE00, -0x117000, -0x319200, -0x159000,
                          -0x315B00, -0x199800, -0x311900, -0x1D8800, -0x30CC00, -0x216000, -0x307400, -0x252000, -0x301100, -0x28C800, -0x2FA300, -0x2C5800,
                          -0x2F2A00, -0x2FD000, -0x2EA600, -0x333000, -0x2E1700, -0x367800, -0x2D7D00, -0x39A800, -0x2CD800, -0x3CC000, -0x2C2800, -0x3FC000,
                          -0x2B6D00, -0x42A800, -0x2AA700, -0x457800, -0x29D600, -0x483000, -0x28FA00, -0x4AD000, -0x281300, -0x4D5800, -0x272100, -0x4FC800,
                          -0x262400, -0x522000, -0x251C00, -0x546000, -0x240900, -0x568800, -0x22EB00, -0x589800, -0x21C200, -0x5A9000, -0x208E00, -0x5C7000,
                          -0x1F4F00, -0x5E3800, -0x1E0500, -0x5FE800, -0x1CB000, -0x618000, -0x1B5000, -0x630000, -0x19E500, -0x646800, -0x186F00, -0x65B800,
                          -0x16EE00, -0x66F000, -0x156200, -0x681000, -0x13CB00, -0x691800, -0x122900, -0x6A0800, -0x107C00, -0x6AE000, -0xEC400,  -0x6BA000,
                          -0xD0100,  -0x6C4800, -0xB3300,  -0x6CD800, -0x95A00,  -0x6D5000, -0x77600,  -0x6DB000, -0x58700,  -0x6DF800, -0x38D00,  -0x6E2800,
                          -0x18800,  -0x6E4000, 0x8800,    -0x6E4000, 0x28D00,   -0x6E2800, 0x48700,   -0x6DF800, 0x67600,   -0x6DB000, 0x85A00,   -0x6D5000,
                          0xA3300,   -0x6CD800, 0xC0100,   -0x6C4800, 0xDC400,   -0x6BA000, 0xF7C00,   -0x6AE000, 0x112900,  -0x6A0800, 0x12CB00,  -0x691800,
                          0x146200,  -0x681000, 0x15EE00,  -0x66F000, 0x176F00,  -0x65B800, 0x18E500,  -0x646800, 0x1A5000,  -0x630000, 0x1BB000,  -0x618000,
                          0x1D0500,  -0x5FE800, 0x1E4F00,  -0x5E3800, 0x1F8E00,  -0x5C7000, 0x20C200,  -0x5A9000, 0x21EB00,  -0x589800, 0x230900,  -0x568800,
                          0x241C00,  -0x546000, 0x252400,  -0x522000, 0x262100,  -0x4FC800, 0x271300,  -0x4D5800, 0x27FA00,  -0x4AD000, 0x28D600,  -0x483000,
                          0x29A700,  -0x457800, 0x2A6D00,  -0x42A800, 0x2B2800,  -0x3FC000, 0x2BD800,  -0x3CC000, 0x2C7D00,  -0x39A800, 0x2D1700,  -0x367800,
                          0x2DA600,  -0x333000, 0x2E2A00,  -0x2FD000, 0x2EA300,  -0x2C5800, 0x2F1100,  -0x28C800, 0x2F7400,  -0x252000, 0x2FCC00,  -0x216000,
                          0x301900,  -0x1D8800, 0x305B00,  -0x199800, 0x309200,  -0x159000, 0x30BE00,  -0x117000, 0x30DF00,  -0xD3800,  0x30F500,  -0x8E800,
                          0x310000,  -0x48000,  0x310000,  0 };

    memcpy(sVars->arcOffsets, arcOffsets, sizeof(arcOffsets));

    int32 moveOffsets[] = { -0x264000, -0x262800, -0x25F800, -0x25B000, -0x255000, -0x24D800, -0x244800, -0x23A000, -0x22E000, -0x220800, -0x211800,
                          -0x201000, -0x1EF000, -0x1DB800, -0x1C6800, -0x1B0000, -0x198000, -0x17E800, -0x163800, -0x147000, -0x129000, -0x109800,
                          -0xE8800,  -0xC6000,  -0xA2000,  -0x7C800,  -0x55800,  -0x2D000,  -0x3000,   0x28800,   0x55800,   0x84000,   0xB4000,
                          0xE5800,   0x118800,  0x14D000,  0x183000,  0x1BA800,  0x1F3800,  0x22E000,  0x26A000,  0x2A7800,  0x2E6800,  0x327000,
                          0x369000,  0x3AC800,  0x3F1800,  0x438000,  0x480000,  0x480000,  0x438000,  0x3F1800,  0x3AC800,  0x369000,  0x327000,
                          0x2E6800,  0x2A7800,  0x26A000,  0x22E000,  0x1F3800,  0x1BA800,  0x183000,  0x14D000,  0x118800,  0xE5800,   0xB4000,
                          0x84000,   0x55800,   0x28800,   -0x3000,   -0x2D000,  -0x55800,  -0x7C800,  -0xA2000,  -0xC6000,  -0xE8800,  -0x109800,
                          -0x129000, -0x147000, -0x163800, -0x17E800, -0x198000, -0x1B0000, -0x1C6800, -0x1DB800, -0x1EF000, -0x201000, -0x211800,
                          -0x220800, -0x22E000, -0x23A000, -0x244800, -0x24D800, -0x255000, -0x25B000, -0x25F800, -0x262800, -0x264000 };

    memcpy(sVars->moveOffsets, moveOffsets, sizeof(moveOffsets));
}
#endif

#if RETRO_INCLUDE_EDITOR
void ChemicalBall::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    this->drawPos = this->position;
    if (this->direction == FLIP_NONE)
        this->drawPos.x += 0x320000;
    else
        this->drawPos.x -= 0x320000;

    if (this->type == CHEMICALBALL_VERTICAL)
        this->drawPos.y -= 0x480000;

    ChemicalBall::Draw();

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        this->drawPos   = this->position;
        this->inkEffect = INK_BLEND;
        ChemicalBall::Draw();
        this->inkEffect = INK_NONE;

        RSDK_DRAWING_OVERLAY(false);
    }
}

void ChemicalBall::EditorLoad()
{
    sVars->aniFrames.Load("CPZ/ChemicalBall.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Arc", CHEMICALBALL_ARC);
    RSDK_ENUM_VAR("Vertical", CHEMICALBALL_VERTICAL);

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("No Flip", FLIP_NONE);
    RSDK_ENUM_VAR("Flipped", FLIP_X);
}
#endif

void ChemicalBall::Serialize()
{
    RSDK_EDITABLE_VAR(ChemicalBall, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(ChemicalBall, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(ChemicalBall, VAR_UINT8, interval);
    RSDK_EDITABLE_VAR(ChemicalBall, VAR_UINT8, intervalOffset);
    RSDK_EDITABLE_VAR(ChemicalBall, VAR_BOOL, master); // unused, (looks to be set on the first instance of arc types)
}
} // namespace GameLogic