#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Localization : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum GameStrings {
        TestString,
        FeatureUnimplemented,
        RestartWarning,
        QuitWarningLoseProgress,
        QuitWarning,
        DeletePopup,
        PlaceholderLogin,
        SaveLoadFail,
        CorruptSave,
        NoSaveSpace,
        NoXBoxProfile,
        SignOutDetected,
        ReconnectController,
        ReconnectWirelessController,
        AutoSaveNotif,
        KeyAlreadyBound,
        KeyAlreadyBoundP1,
        KeyAlreadyBoundP2,
        ApplyChangedSettings,
        VideoChangesApplied,
        SteamOverlayUnavailable,
        SaveReplay,
        NoReplaySpace,
        DeleteReplay,
        ErrorLoadingReplay,
        CannotLoadReplay,
        Loading,
        NoReplays,
        AreYouSure,
        AreYouSureSave,
        ResetTimesWarning,
        ReturningToTitle,
        SelectATrack,
        SelectDataToErase,
        RPC_Playing,
        RPC_Menu,
        RPC_Mania,
        RPC_TA,
        RPC_Title,
        Default,
        StringCount,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::String text;
        RSDK::String strings[StringCount];
        bool32 loaded;
        uint8 language;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

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

    static void LoadStrings();
    static void GetString(RSDK::String *string, uint8 id);
    static void GetZoneName(RSDK::String *string, uint8 zone);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Localization);
};
} // namespace GameLogic