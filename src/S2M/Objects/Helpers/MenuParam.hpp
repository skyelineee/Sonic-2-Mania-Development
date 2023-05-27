#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct MenuParam : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum ExtrasSelectionIDs {
        EXTRAS_SELECTION_BOSSRUSH,
        EXTRAS_SELECTION_MUSICPLAYER,
        EXTRAS_SELECTION_LEVELSELECT,
        EXTRAS_SELECTION_EXTRALEVELS,
        EXTRAS_SELECTION_CREDITS,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    char menuTag[256];
    int32 menuSelection;
    bool32 startedTAAttempt;
    bool32 inTimeAttack;
    int32 replayID;
    int32 characterID;
    int32 zoneID;
    int32 actID;
    int32 timeAttackRank;
    int32 replayUUID;
    bool32 viewReplay;
    bool32 showGhost;
    uint8 selectedReplay;
    uint8 replayRankID;
    int32 unused2;
    int32 unused3;
    int32 unused4;
    int32 unused5;
    int32 vsZoneID;
    int32 vsActID;
    int32 creditsReturnToMenu;

    // ==============================
    // EVENTS
    // ==============================

#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif

    // ==============================
    // FUNCTIONS
    // ==============================
    static MenuParam *GetMenuParam();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(MenuParam);
};
} // namespace GameLogic