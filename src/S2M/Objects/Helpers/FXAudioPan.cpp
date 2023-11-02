// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: FXAudioPan Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "FXAudioPan.hpp"
#include "MathHelpers.hpp"
#include "DrawHelpers.hpp"

#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(FXAudioPan);

void FXAudioPan::Update() { this->visible = DebugMode::sVars->debugActive; }
void FXAudioPan::LateUpdate() {}
void FXAudioPan::StaticUpdate() {}
void FXAudioPan::Draw()
{
    Graphics::DrawLine(this->position.x - TILE_SIZE * this->size.x, this->position.y - TILE_SIZE * this->size.y,
                       this->position.x + TILE_SIZE * this->size.x, this->position.y - TILE_SIZE * this->size.y, 0xFFFF00, 0xFF, INK_NONE, false);
    Graphics::DrawLine(this->position.x - TILE_SIZE * this->size.x, this->position.y + TILE_SIZE * this->size.y,
                       this->position.x + TILE_SIZE * this->size.x, this->position.y + TILE_SIZE * this->size.y, 0xFFFF00, 0xFF, INK_NONE, false);
    Graphics::DrawLine(this->position.x - TILE_SIZE * this->size.x, this->position.y - TILE_SIZE * this->size.y,
                       this->position.x - TILE_SIZE * this->size.x, this->position.y + TILE_SIZE * this->size.y, 0xFFFF00, 0xFF, INK_NONE, false);
    Graphics::DrawLine(this->position.x + TILE_SIZE * this->size.x, this->position.y - TILE_SIZE * this->size.y,
                       this->position.x + TILE_SIZE * this->size.x, this->position.y + TILE_SIZE * this->size.y, 0xFFFF00, 0xFF, INK_NONE, false);

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 1);
    this->animator.DrawSprite(&this->position, false);

    if (this->sfxActive)
        DrawHelpers::DrawCross(this->sfxPos.x, this->sfxPos.y, 0x200000, 0x200000, 0xFF00FF);
}

void FXAudioPan::Create(void *data)
{
    this->active        = ACTIVE_BOUNDS;
    this->visible       = 0;
    this->updateRange.x = TILE_SIZE * this->size.x;
    this->updateRange.y = TILE_SIZE * this->size.y;
    this->sfxPos.x      = 0;
    this->sfxPos.y      = 0;
    this->drawGroup     = Zone::sVars->hudDrawGroup;

    if (this->soundName.length) {
        char sfxBuffer[0x20];
        this->soundName.CStr(sfxBuffer);
        this->sfxID.Get(sfxBuffer);
    }
}

void FXAudioPan::StageLoad()
{
    sVars->active = ACTIVE_ALWAYS;
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);

    Soundboard::LoadSfx(FXAudioPan::CheckCB, FXAudioPan::UpdateCB);
}

Soundboard::SoundInfo FXAudioPan::CheckCB()
{
    int32 worldCenterX = (screenInfo->position.x + screenInfo->center.x) << 16;
    int32 worldCenterY = (screenInfo->position.y + screenInfo->center.y) << 16;
    Vector2 worldPos(worldCenterX, worldCenterY);

    sVars->activeCount1 = 0;
    sVars->activeCount2 = 0;
    int32 count         = 0;

    RSDK::SoundFX sfx;
    sfx.Init();
    uint32 loopPoint = 0;

    for (auto sound : GameObject::GetEntities<FXAudioPan>(FOR_ALL_ENTITIES)) {
        if (sound->sfxID.Loaded())
            sfx = sound->sfxID;

        sound->sfxActive = false;
        sound->sfxPos.x  = 0;
        sound->sfxPos.y  = 0;

        Hitbox hitbox;
        hitbox.left   = -(sound->size.x >> 12);
        hitbox.top    = -(sound->size.y >> 12);
        hitbox.right  = sound->size.x >> 12;
        hitbox.bottom = sound->size.y >> 12;

        if (MathHelpers::PointInHitbox(sound->position.x, sound->position.y, worldPos.x, worldPos.y, FLIP_NONE, &hitbox)) {
            sound->sfxPos.x  = worldCenterX;
            sound->sfxPos.y  = worldCenterY;
            loopPoint        = sound->loopPos;
            sound->sfxActive = true;
            ++count;
        }
        else {
            Vector2 sfxPos(sound->position.x, sound->position.y);

            if (MathHelpers::Distance(sfxPos, worldPos) - 8 * sound->size.y - 8 * sound->size.x <= 0x2800000) {
                if (MathHelpers::ConstrainToBox(&sound->sfxPos, worldCenterX, worldCenterY, sound->position, hitbox)) {
                    if (MathHelpers::Distance(sound->sfxPos, worldPos) <= 0x2800000) {
                        loopPoint        = sound->loopPos;
                        sound->sfxActive = true;
                        ++count;
                        break;
                    }
                }
            }
        }
    }

    sVars->activeCount1 = count;
    sVars->activeCount2 = count;

    Soundboard::SoundInfo info = {};
    info.playFlags             = (uint16)(sVars->activeCount1 > 0 ? Soundboard::PlayOnStart : Soundboard::PlayNever);
    info.sfx                   = sfx;
    info.loopPoint             = loopPoint;

    return info;
}
void FXAudioPan::UpdateCB(int32 sfxID)
{
    int32 worldCenterX = (screenInfo->position.x + screenInfo->center.x) << 16;
    int32 worldCenterY = (screenInfo->position.y + screenInfo->center.y) << 16;
    Vector2 worldPos(worldCenterX, worldCenterY);

    int32 worldLeft  = worldCenterX - (screenInfo->center.x << 16);
    int32 worldRight = worldCenterX + (screenInfo->center.x << 16);

    float pan        = 0.0f;
    float volDivisor = 0.0f;
    int32 dist       = 0x7FFF0000;

    for (auto sound : GameObject::GetEntities<FXAudioPan>(FOR_ALL_ENTITIES)) {
        if (sound->sfxActive) {
            int16 sqRoot   = MIN(MathHelpers::Distance(sound->sfxPos, worldPos) >> 16, 640);
            float volume   = (sqRoot / -640.0f) + 1.0f;
            float distance = -1.0;
            if (sound->sfxPos.x > worldLeft) {
                distance = 1.0;
                if (sound->sfxPos.x < worldRight) {
                    distance = (((sound->sfxPos.x - worldCenterX) >> 16) / (float)screenInfo->center.x);
                }
            }

            volDivisor += volume;
            if (dist >= (sqRoot << 16))
                dist = (sqRoot << 16);
            pan += volume * distance;
        }
    }

    float div = FABS(volDivisor);
    if (div > 0)
        pan /= volDivisor;

    float volume = MIN(dist >> 16, 640) / -640.f + 1;
    channels[Soundboard::sVars->sfxChannel[sfxID]].SetAttributes(volume, CLAMP(pan, -1.0f, 1.0f), 1.0f);
}

uint8 FXAudioPan::PlayDistancedSfx(RSDK::SoundFX sfx, uint32 loopPoint, uint32 priority, RSDK::Vector2 position)
{
    int32 worldCenterX = (screenInfo->position.x + screenInfo->center.x) << 16;
    int32 worldCenterY = (screenInfo->position.y + screenInfo->center.y) << 16;
    Vector2 worldPos(worldCenterX, worldCenterY);

    int32 worldLeft  = worldCenterX - (screenInfo->center.x << 16);
    int32 worldRight = worldCenterX + (screenInfo->center.x << 16);

    float pan        = 0.0f;
    float volDivisor = 0.0f;
    int32 dist       = 0x7FFF0000;

    {
        int16 d        = MIN(MathHelpers::Distance(position, worldPos), 640);
        float volume   = (d / -640.0f) + 1.0f;
        float distance = -1.0;
        if (position.x > worldLeft) {
            distance = 1.0;
            if (position.x < worldRight) {
                distance = (((position.x - worldCenterX) >> 16) / (float)screenInfo->center.x);
            }
        }

        volDivisor += volume;
        if (dist >= (d << 16))
            dist = (d << 16);
        pan += volume * distance;
    }

    float div = FABS(volDivisor);
    if (div > 0)
        pan /= volDivisor;

    float volume  = (float)MIN(dist >> 16, 640);
    uint8 channel = sfx.Play(loopPoint, priority);
    channels[channel].SetAttributes((volume / -640.0f) + 1.0f, CLAMP(pan, -1.0f, 1.0f), 1.0f);
    return channel;
}

void FXAudioPan::HandleAudioDistance(uint32 channelID, RSDK::Vector2 position)
{
    int32 worldCenterX = (screenInfo->position.x + screenInfo->center.x) << 16;
    int32 worldCenterY = (screenInfo->position.y + screenInfo->center.y) << 16;
    Vector2 worldPos(worldCenterX, worldCenterY);

    int32 worldLeft  = worldCenterX - (screenInfo->center.x << 16);
    int32 worldRight = worldCenterX + (screenInfo->center.x << 16);

    float pan        = 0.0f;
    float volDivisor = 0.0f;
    int32 dist       = 0x7FFF0000;

    {
        int16 d        = MIN(MathHelpers::Distance(position, worldPos), 640);
        float volume   = (d / -640.0f) + 1.0f;
        float distance = -1.0;
        if (position.x > worldLeft) {
            distance = 1.0;
            if (position.x < worldRight) {
                distance = (((position.x - worldCenterX) >> 16) / (float)screenInfo->center.x);
            }
        }

        volDivisor += volume;
        if (dist >= (d << 16))
            dist = (d << 16);
        pan += volume * distance;
    }

    float div = FABS(volDivisor);
    if (div > 0)
        pan /= volDivisor;

    float volume = (float)MIN(dist >> 16, 640);
    channels[channelID].SetAttributes((volume / -640.0f) + 1.0f, CLAMP(pan, -1.0f, 1.0f), 1.0f);
}

#if RETRO_INCLUDE_EDITOR
void FXAudioPan::EditorDraw()
{
    Graphics::DrawLine(this->position.x - TILE_SIZE * this->size.x, this->position.y - TILE_SIZE * this->size.y,
                       this->position.x + TILE_SIZE * this->size.x, this->position.y - TILE_SIZE * this->size.y, 0xFFFF00, 0xFF, INK_NONE, false);
    Graphics::DrawLine(this->position.x - TILE_SIZE * this->size.x, this->position.y + TILE_SIZE * this->size.y,
                       this->position.x + TILE_SIZE * this->size.x, this->position.y + TILE_SIZE * this->size.y, 0xFFFF00, 0xFF, INK_NONE, false);
    Graphics::DrawLine(this->position.x - TILE_SIZE * this->size.x, this->position.y - TILE_SIZE * this->size.y,
                       this->position.x - TILE_SIZE * this->size.x, this->position.y + TILE_SIZE * this->size.y, 0xFFFF00, 0xFF, INK_NONE, false);
    Graphics::DrawLine(this->position.x + TILE_SIZE * this->size.x, this->position.y - TILE_SIZE * this->size.y,
                       this->position.x + TILE_SIZE * this->size.x, this->position.y + TILE_SIZE * this->size.y, 0xFFFF00, 0xFF, INK_NONE, false);

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 1);
    this->animator.DrawSprite(&this->position, false);
}

void FXAudioPan::EditorLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void FXAudioPan::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(FXAudioPan);

    sVars->aniFrames.Init();

    sVars->field_1E = -1;
    sVars->field_20 = -1;
}
#endif

void FXAudioPan::Serialize()
{
    RSDK_EDITABLE_VAR(FXAudioPan, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(FXAudioPan, VAR_STRING, soundName);
    RSDK_EDITABLE_VAR(FXAudioPan, VAR_UINT32, loopPos);
}

} // namespace GameLogic