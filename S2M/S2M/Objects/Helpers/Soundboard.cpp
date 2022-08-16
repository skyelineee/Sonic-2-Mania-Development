// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Soundboard Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Soundboard.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Soundboard);

void Soundboard::Update() {}
void Soundboard::LateUpdate() {}
void Soundboard::StaticUpdate()
{
    for (int32 s = 0; s < MIN(sVars->sfxCount, 32); ++s) {
        bool32 shouldStopSfx = true;
        SoundInfo info       = {};

        if (sVars->sfxCheckCallback[s] && sceneInfo->state == ENGINESTATE_REGULAR) {
            info = sVars->sfxCheckCallback[s]();
            if (info.playFlags & PlayOnStart) {
                shouldStopSfx             = false;
                sVars->sfxFadeOutTimer[s] = 0;

                if (!sVars->sfxIsPlaying[s]) {
                    sVars->sfxChannel[s]      = info.sfx.Play(info.loopPoint);
                    sVars->sfxIsPlaying[s]    = true;
                    sVars->sfxPlayingTimer[s] = 0;
                }

                if (sVars->sfxFadeOutDuration[s] > 0)
                    channels[sVars->sfxChannel[s]].SetAttributes(1.0, 0.0, 1.0);

                if (sVars->sfxUpdateCallback[s]) {
                    sVars->sfxUpdateCallback[s](s);
                    ++sVars->sfxPlayingTimer[s];
                }
            }
        }

        if (shouldStopSfx) {
            if (sVars->sfxIsPlaying[s]) {
                sVars->sfxFadeOutTimer[s] = 0;

                if (!sVars->sfxFadeOutDuration[s]) {
                    if (info.playFlags & PlayOnFinish)
                        info.sfx.Play();
                    else
                        sVars->sfxList[s].Stop();
                }

                sVars->sfxIsPlaying[s]    = false;
                sVars->sfxPlayingTimer[s] = 0;
            }
            else if (sVars->sfxFadeOutDuration[s] > 0 && sVars->sfxChannel[s] > 0) {
                if (sVars->sfxFadeOutTimer[s] >= sVars->sfxFadeOutDuration[s]) {
                    sVars->sfxList[s].Stop();
                }
                else {
                    channels[sVars->sfxChannel[s]].SetAttributes(1.0f - (sVars->sfxFadeOutTimer[s] / (float)sVars->sfxFadeOutDuration[s]), 0.0, 1.0);
                    ++sVars->sfxFadeOutTimer[s];
                }
            }
        }
    }
}
void Soundboard::Draw() {}

void Soundboard::Create(void *data) {}

void Soundboard::StageLoad()
{
    sVars->active   = ACTIVE_ALWAYS;
    sVars->sfxCount = 0;

    for (int32 i = 0; i < 32; ++i) {
        sVars->sfxList[i].Init();
        sVars->sfxLoopPoint[i]       = 0;
        sVars->sfxCheckCallback[i]   = nullptr;
        sVars->sfxUpdateCallback[i]  = nullptr;
        sVars->sfxIsPlaying[i]       = false;
        sVars->sfxPlayingTimer[i]    = 0;
        sVars->sfxFadeOutDuration[i] = 0;
    }
}

uint8 Soundboard::LoadSfx(SoundInfo (*checkCallback)(), void (*updateCallback)(int32 sfxID))
{
    if (!sVars)
        return -1;
    if (sVars->sfxCount >= 32)
        return -1;

    int32 sfxID = sVars->sfxCount;

    sVars->sfxCheckCallback[sfxID]   = checkCallback;
    sVars->sfxUpdateCallback[sfxID]  = updateCallback;
    sVars->sfxFadeOutDuration[sfxID] = 0;
    sVars->sfxList[sfxID].Stop();

    ++sVars->sfxCount;
    return sfxID;
}

#if RETRO_INCLUDE_EDITOR
void Soundboard::EditorDraw() {}

void Soundboard::EditorLoad() {}
#endif

#if RETRO_REV0U
void Soundboard::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Soundboard);

    for (int32 s = 0; s < 32; ++s) sVars->sfxList[s].Init();
}
#endif

void Soundboard::Serialize() {}

} // namespace GameLogic