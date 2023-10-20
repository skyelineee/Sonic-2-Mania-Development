#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Soundboard : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum PlayFlags {
        PlayNever,
        PlayOnStart,
        PlayOnFinish,
    };

    // ==============================
    // STRUCTS
    // ==============================
    struct SoundInfo {
        uint16 playFlags;
        RSDK::SoundFX sfx;
        uint16 loopPoint;
    };

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 sfxCount;
        RSDK::SoundFX sfxList[32];
        int32 sfxLoopPoint[32];
        SoundInfo (*sfxCheckCallback[32])();
        void (*sfxUpdateCallback[32])(int32 sfxID);
        bool32 sfxIsPlaying[32];
        int32 sfxChannel[32];
        int32 sfxPlayingTimer[32];
        int32 sfxFadeOutTimer[32];
        int32 sfxFadeOutDuration[32];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
    static void StageLoad();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    static uint8 LoadSfx(SoundInfo (*checkCallback)(), void (*updateCallback)(int32 sfxID));
    static uint8 LoadSfxName(const char *sfxName, uint32 loopPoint, SoundInfo (*checkCallback)(), void (*updateCallback)(int32 sfxID)); // mania version of function

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Soundboard);
};
} // namespace GameLogic