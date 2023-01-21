// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Options Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Options.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Global/Localization.hpp"
#include "Menu/UIHeading.hpp"
#include "Menu/UIWidgets.hpp"

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

    LogHelpers::Print("optionsPtr->screenShader = %d", options->screenShader);
    LogHelpers::Print("optionsPtr->volMusic = %d", options->volMusic);
    LogHelpers::Print("optionsPtr->volSfx = %d", options->volSfx);
    LogHelpers::Print("optionsPtr->language = %d", options->language);
    LogHelpers::Print("optionsPtr->overrideLanguage = %d", options->overrideLanguage);
    LogHelpers::Print("optionsPtr->vsync = %d", options->vSync);
    LogHelpers::Print("optionsPtr->tripleBuffering = %d", options->tripleBuffering);
    LogHelpers::Print("optionsPtr->windowBorder = %d", options->windowBorder);
    LogHelpers::Print("optionsPtr->windowed = %d", options->windowed);
    LogHelpers::Print("optionsPtr->windowSize = %d", options->windowSize);
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

void Options::LoadCallback(bool32 success)
{
    if (success) {
        Localization::sVars->loaded = false;

        Localization::LoadStrings();
        UIWidgets::ApplyLanguage();
        UIHeading::LoadSprites();
    }
}

void Options::LoadOptionsBin()
{
    if (SKU->platform && SKU->platform != PLATFORM_DEV) {
        if (globals->optionsLoaded != STATUS_CONTINUE) {
            if (globals->optionsLoaded == STATUS_OK) {
                Options::LoadCallback(true);
            }
            else {
                globals->optionsLoaded = STATUS_CONTINUE;
                sVars->loadEntityPtr   = (Entity *)sceneInfo->entity;
                sVars->loadCallback    = Options::LoadCallback;
                APITable->LoadUserFile("Options.bin", globals->optionsRAM, 0x200, Options::LoadOptionsCallback);
            }
        }
    }
    else {
        globals->optionsLoaded = STATUS_OK;
        Options::LoadCallback(true);
    }
}

void Options::SaveOptionsBin(void (*callback)(bool32 success))
{
    if (sVars->changed) {
        if (SKU->platform && SKU->platform != PLATFORM_DEV) {
            if (globals->optionsLoaded == STATUS_OK) {
                sVars->saveEntityPtr   = (Entity *)sceneInfo->entity;
                sVars->saveCallback    = callback;

                APITable->SaveUserFile("Options.bin", globals->optionsRAM, 0x200, Options::SaveOptionsCallback, false);
            }
            else {
                sVars->changed = false;
                if (callback)
                    callback(false);
            }

            return;
        }
        else {
            Graphics::SetVideoSetting(VIDEOSETTING_CHANGED, true);
            Options::Reload();
        }
    }

    if (callback)
        callback(true);
}

void Options::SetLanguage(int32 language)
{
    Options *options = GetOptionsRAM();

    if (language >= 0) {
        options->language         = language;
        options->overrideLanguage = true;
    }
    else {
        options->language         = -1;
        options->overrideLanguage = false;
    }

    if (SKU->platform == PLATFORM_PC || SKU->platform == PLATFORM_DEV)
        Graphics::SetVideoSetting(VIDEOSETTING_LANGUAGE, language);

    sVars->changed = true;
}

void Options::LoadValuesFromSettings(Options *options)
{
    Localization::sVars->language = options->overrideLanguage ? options->language : SKU->language;

    if (!options->overrideShader)
        options->screenShader = Graphics::GetVideoSetting(VIDEOSETTING_SHADERID) % 4;

    if (!options->overrideMusicVol)
        options->volMusic = Graphics::GetVideoSetting(VIDEOSETTING_STREAM_VOL);

    if (!options->overrideSfxVol)
        options->volSfx = Graphics::GetVideoSetting(VIDEOSETTING_SFX_VOL);
}

void Options::LoadOptionsCallback(int32 status)
{
    Options *options = GetOptionsRAM();
    bool32 success      = false;

    if (status == STATUS_OK || status == STATUS_NOTFOUND) {
        success                = true;
        globals->optionsLoaded = STATUS_OK;

        LogHelpers::Print("dataPtr.language = %d", options->language);
        LogHelpers::Print("dataPtr.overrideLanguage = %d", options->overrideLanguage);
        Options::LoadValuesFromSettings(GetOptionsRAM());

        Graphics::SetVideoSetting(VIDEOSETTING_SHADERID, options->screenShader);
        Graphics::SetVideoSetting(VIDEOSETTING_STREAM_VOL, options->volMusic);
        Graphics::SetVideoSetting(VIDEOSETTING_SFX_VOL, options->volSfx);
    }
    else {
        success                = false;
        globals->optionsLoaded = STATUS_ERROR;
    }

    if (sVars->loadCallback) {
        Entity *entStore = (Entity *)sceneInfo->entity;
        if (sVars->loadEntityPtr)
            sceneInfo->entity = sVars->loadEntityPtr;
        sVars->loadCallback(success);
        sceneInfo->entity = entStore;

        sVars->loadCallback  = nullptr;
        sVars->loadEntityPtr = nullptr;
    }
}

void Options::SaveOptionsCallback(int32 status)
{
    sVars->changed = false;
    if (sVars->saveCallback) {
        Entity *entStore = (Entity *)sceneInfo->entity;
        if (sVars->saveEntityPtr)
            sceneInfo->entity = sVars->saveEntityPtr;

        sVars->saveCallback(status == STATUS_OK);
        sceneInfo->entity = entStore;

        sVars->saveCallback  = nullptr;
        sVars->saveEntityPtr = nullptr;
    }
}

#if RETRO_REV0U
void Options::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(Options); }
#endif

} // namespace GameLogic