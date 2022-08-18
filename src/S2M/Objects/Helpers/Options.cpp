// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Options Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Options.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_STATIC_VARS(Options);

void Options::StageLoad()
{
    sVars->changed = false;

    if (SKU->platform == PLATFORM_DEV || SKU->platform == PLATFORM_PC) {
        Reload();
    }
    else {
        Options *options = GetOptionsRAM();

        options->vSync           = false;
        options->windowed        = false;
        options->windowBorder    = false;
        options->tripleBuffering = false;
    }
}


Options *Options::GetOptionsRAM() { return (Options *)globals->optionsRAM; }

void Options::Reload()
{
    Options *options = GetOptionsRAM();

    options->overrideLanguage = true;

    options->screenShader    = Graphics::GetVideoSetting(VIDEOSETTING_SHADERID) % 4;
    options->volMusic        = Graphics::GetVideoSetting(VIDEOSETTING_STREAM_VOL);
    options->volSfx          = Graphics::GetVideoSetting(VIDEOSETTING_SFX_VOL);
    options->language        = Graphics::GetVideoSetting(VIDEOSETTING_LANGUAGE);
    options->vSync           = Graphics::GetVideoSetting(VIDEOSETTING_VSYNC);
    options->windowBorder    = Graphics::GetVideoSetting(VIDEOSETTING_BORDERED);
    options->windowed        = Graphics::GetVideoSetting(VIDEOSETTING_WINDOWED);
    options->tripleBuffering = Graphics::GetVideoSetting(VIDEOSETTING_TRIPLEBUFFERED);

    GetWinSize();
}
void Options::GetWinSize()
{
    Options *options = GetOptionsRAM();

    bool32 windowed = Graphics::GetVideoSetting(VIDEOSETTING_WINDOWED);
    if (!windowed) {
        options->windowSize = 4;
    }
    else {
        int32 width = Graphics::GetVideoSetting(VIDEOSETTING_WINDOW_WIDTH);

        if (width <= WIDE_SCR_XSIZE)
            options->windowSize = 0;
        else if (width <= WIDE_SCR_XSIZE * 2)
            options->windowSize = 1;
        else if (width <= WIDE_SCR_XSIZE * 3)
            options->windowSize = 2;
        else
            options->windowSize = 3;
    }
}

#if RETRO_REV0U
void Options::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(Options); }
#endif

} // namespace GameLogic