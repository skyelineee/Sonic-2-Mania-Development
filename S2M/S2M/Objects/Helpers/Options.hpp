#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Options : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        Entity *loadEntityPtr;
        void (*loadCallback)(bool32 success);
        Entity *saveEntityPtr;
        void (*saveCallback)(bool32 success);
        bool32 changed;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    uint8 padding[0x54];
    uint8 language;
    int32 overrideLanguage;
    int32 screenShader;
    bool32 overrideShader;
    int32 volMusic;
    bool32 overrideMusicVol;
    int32 volSfx;
    bool32 overrideSfxVol;
    bool32 vSync;
    uint8 windowSize;
    bool32 windowBorder;
    bool32 windowed;
    bool32 tripleBuffering;

    // ==============================
    // EVENTS
    // ==============================

    static void StageLoad();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif

    // ==============================
    // FUNCTIONS
    // ==============================
    static Options* GetOptionsRAM();
    static void Reload();
    static void GetWinSize();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Options);
};
} // namespace GameLogic