// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Snowflakes Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "Snowflakes.hpp"
#include "Global/Zone.hpp"
#include "SWZSetup.hpp"
#include "Global/HUD.hpp"
#include "Global/ActClear.hpp"
#include "EHZ/EHZSetup.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Snowflakes);

void Snowflakes::CreateSnowflake(int32 i, bool32 top)
{
    int32 scrX = screenInfo->position.x % screenInfo->size.x;
    int32 posX = (scrX + ZONE_RAND(0, screenInfo->size.x)) % screenInfo->size.x;

    if (top) {
        this->positions[i].y = (screenInfo->position.y - 5) << 16;
    }
    else {
        this->positions[i].y = (screenInfo->position.y + ZONE_RAND(-0x80, screenInfo->size.y + 0x80)) << 16;
    }

    this->positions[i].x = posX << 16;
    this->frameIDs[i]    = 0;
    this->priority[i]    = ZONE_RAND(0, 10) > 7;

    if (this->priority[i]) {
        this->animIDs[i] = 2 * (ZONE_RAND(0, 10) > 7) + 2;
    }
    else {
        int32 type       = ZONE_RAND(0, 10);
        this->animIDs[i] = type > 8 ? 3 : (type > 4 ? 1 : 0);
    }

    this->angles[i] = ZONE_RAND(0, 256);
    ++sVars->count;
}

void Snowflakes::Update()
{
    for (int32 i = 0; i < 0x40; ++i) {
        if (!this->positions[i].x && !this->positions[i].y) {
            CreateSnowflake(i);
        }
    }

    Vector2 range;
    range.x = 0x800000;
    range.y = 0x800000;

    for (int32 i = 0; i < 0x40; ++i) {
        if (this->positions[i].x || this->positions[i].y) {
            Vector2 pos = Snowflakes::HandleWrap(i);
            int32 angle = Math::Sin256(this->angles[i]) << 6;

            this->positions[i].x += angle;
            this->positions[i].y += 0x8000;
            pos.x += angle;
            pos.y += 0x8000;

            this->frameIDs[i]++;

            if (this->animIDs[i] <= 2)
                this->frameIDs[i] %= 12;

            if (this->animIDs[i] <= 3)
                this->frameIDs[i] %= 24;

            if (this->animIDs[i] <= 4)
                this->frameIDs[i] %= 48;

            ++this->angles[i];
            this->angles[i] &= 0xFF;

            if (!pos.CheckOnScreen(&range)) {
                this->positions[i].x = 0;
                this->positions[i].y = 0;
                --sVars->count;
            }
        }
    }

    this->animator.Process();
}

void Snowflakes::LateUpdate()
{
    if (!spawned) {
        for (int32 i = 0; i < 0x40; ++i) {
            CreateSnowflake(i, false);
        }
        spawned = true;
    }
}

void Snowflakes::StaticUpdate()
{
    for (auto snowflake : GameObject::GetEntities<Snowflakes>(FOR_ACTIVE_ENTITIES)) {
        RSDKTable->AddDrawListRef(Zone::sVars->objectDrawGroup[1], RSDKTable->GetEntitySlot(snowflake));
    }
}

void Snowflakes::Draw()
{
    int32 drawHigh  = Zone::sVars->objectDrawGroup[1];
    int32 drawGroup = sceneInfo->currentDrawGroup;

    for (int32 i = 0; i < 0x40; ++i) {
        if (this->positions[i].x || this->positions[i].y) {
            int32 priority = this->priority[i];

            if ((priority || drawGroup != drawHigh) && (priority != 1 || drawGroup == drawHigh)) {
                Vector2 drawPos = Snowflakes::HandleWrap(i);
                this->direction = FLIP_NONE;
                int32 angle     = Math::Sin256(this->angles[i]) << 6;

                if (this->animIDs[i] <= 2) {
                    this->animator.SetAnimation(sVars->aniFrames, this->animIDs[i], true, this->frameIDs[i] >> 2);
                    this->animator.DrawSprite(&drawPos, false);
                }
                else {
                    int32 frame = 0;

                    if (this->animIDs[i] == 4) {
                        frame = this->frameIDs[i] / 3;

                        if (angle > 0)
                            this->direction = FLIP_X;
                    }
                    else if (this->animIDs[i] == 3) {
                        frame = this->frameIDs[i] >> 2;

                        if (angle > 0)
                            this->direction = FLIP_X;
                    }

                    this->animator.SetAnimation(sVars->aniFrames, this->animIDs[i], true, frame);
                    this->animator.DrawSprite(&drawPos, false);
                }
            }
        }
    }
}

void Snowflakes::Create(void *data)
{

    if (globals->atlEnabled) {
        SWZSetup::Static *store = SWZSetup::sVars;
        if (sVars->holiday) {
            store = (SWZSetup::Static *)EHZSetup::sVars; // this works bc same spot
        }
        Vector2 prePos = position;
        GameObject::Copy(this, store->snowflakeStorage, true);
        position = prePos;
        for (int i = 0; i < 0x40; ++i) {
            if (positions[i].x || positions[i].y) {
                positions[i].y += store->snowflakeYOff;
            }
        }
        basis        = store->snowflakeBasis;
        addend       = store->snowflakeAddend;
        sVars->count = store->snowflakeCount;
        spawned      = true;
    }
    else {
        this->active        = ACTIVE_NORMAL;
        this->drawGroup     = Zone::sVars->objectDrawGroup[0];
        this->visible       = true;
        this->drawFX        = FX_FLIP;
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;

        this->basis  = 0;
        this->addend = 0;

        spawned = false;
    }
}

void Snowflakes::StageLoad()
{
    if (Stage::CheckSceneFolder("SWZ")) {
        sVars->aniFrames.Load("SWZ/Leaves.bin", SCOPE_STAGE);
        sVars->holiday = false;
    }
    else if (Stage::CheckSceneFolder("HEHZ")) {
        sVars->aniFrames.Load("HEHZ/Snowflakes.bin", SCOPE_STAGE);
        sVars->holiday = true;
    }

    sVars->active = ACTIVE_ALWAYS;
    sVars->count  = 0;
}

Vector2 Snowflakes::HandleWrap(int32 id)
{
    int32 x = this->positions[id].x;
    int32 y = this->positions[id].y;

    int32 shift = 7;
    if (!this->priority[id])
        shift = 6;

    int32 screenPosX  = screenInfo->position.x + (basis - addend);
    int32 screenSizeX = screenInfo->size.x;
    int32 newX        = x - (((screenPosX) << 8) << shift);
    while (newX < 0) newX += screenInfo->size.x << 16;

    int32 posX = screenPosX / screenInfo->size.x;
    if ((newX % (screenInfo->size.x << 16)) >> 16 < screenPosX % screenInfo->size.x)
        posX = screenPosX / screenInfo->size.x + 1;

    int32 posY = 0;
    if (y > (screenInfo->size.y + screenInfo->position.y) << 16)
        posY = -screenInfo->size.y;

    Vector2 pos;
    pos.x = (posX * screenInfo->size.x << 16) + (newX % (screenInfo->size.x << 16)) - ((basis - addend) << 16);
    pos.y = (posY << 16) + y;
    return pos;
}

#if RETRO_INCLUDE_EDITOR
void Snowflakes::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 5, true, 0);
    this->animator.DrawSprite(nullptr, false);
}

void Snowflakes::EditorLoad(void) { sVars->aniFrames.Load("PSZ2/Snowflakes.bin", SCOPE_STAGE); }
#endif

void Snowflakes::Serialize(void) {}
} // namespace GameLogic