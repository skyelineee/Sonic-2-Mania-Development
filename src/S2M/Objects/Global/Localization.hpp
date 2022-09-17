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
        DataSelect,
        NoSave,
        Delete,
        Clear,
        Zone,
        GrandPrix,
        MatchRace,
        TimeAttack,
        Exit,
        AzureLake,
        BalloonPark,
        ChromeGadget,
        DesertPalace,
        EndlessMine,
        Records,
        LapX,
        Dashes,
        Total,
        BestTime,
        TryAgain,
        PlayingSonic3,
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

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Localization);
};
} // namespace GameLogic