#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct MenuParam : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum CreditsSelectionType {
        CREDITS_SELECTION_NONE,
        CREDITS_SELECTION_EXTRAS,
    };

    enum ExtrasSelectionIDs {
        EXTRAS_SELECTION_BSS,
        EXTRAS_SELECTION_PUYO,
        EXTRAS_SELECTION_DAGARDEN,
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

    uint8 puyoSelection;
    uint8 bssSelection;
    char menuTag[0x100];
    int32 menuSelection;
    bool32 startedTAAttempt; // only used in pre-plus, goes unused by the time plus rolled around
    bool32 inTimeAttack;
    int32 replayID;
    int32 characterID;
    int32 zoneID;
    int32 actID;
    int32 timeAttackRank;
    bool32 isEncoreMode;
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

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
    static void StageLoad();
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    // ==============================
    // STATES
    // ==============================

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(MenuParam);
};
} // namespace GameLogic
