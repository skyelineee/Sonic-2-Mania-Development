#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SaveGame : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum EmeraldCheckTypes {
        EmeraldNone,
        EmeraldChaosOnly,
        EmeraldChaosOnly2,
        EmeraldSuperOnly,
        EmeraldSuperOnly2,
        EmeraldUnused,
        EmeraldUnused2,
        EmeraldBoth,
        EmeraldBoth2,
        EmeraldSuper,
        EmeraldSuper2,
        EmeraldAny,
        EmeraldAny2,
    };

    enum RecallStates {
        RecallNormal,
        RecallDisabled,
        RecallBrokenItemBox,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        void *loadEntityPtr;
        void (*loadCallback)(bool32 success);
        void *saveEntityPtr;
        void (*saveCallback)(bool32 success);
        SaveGame *saveRAM;
        void *unknown;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    int32 saveState;
    int32 characterID;
    int32 zoneID;
    int32 lives;
    int32 score;
    int32 score1UP;
    int32 chaosEmeralds;
    int32 continues;
    int32 storedStageID;
    uint16 nextSpecialStage;
    int32 collectedSpecialRings;
    int32 medalMods;
    int32 zoneTimes[32];
    uint8 characterFlags;
    int32 stock;
    int32 playerID;

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

    static void LoadSaveData();
    static void RecallCollectedEntities();
    static void LoadFileCB(int32 status);
    static void SaveFileCB(int32 status);
    static SaveGame *GetSaveDataPtr(uint8 saveSlot);
    bool32 CheckDisableRestart();
    static SaveGame *GetSaveRAM();
    static void LoadFile(void (*callback)(bool32 success));
    static void SaveFile(void (*callback)(bool32 success));
    static void SaveProgress();
    static void ClearCollectedSpecialRings();
    static void ClearNoSave();
    static void ClearSaveSlot(uint8 slotID, void (*callback)(bool32 success));
    static void ClearRestartData();
    static void StoreStageState();
    static void SavePlayerState();
    static bool32 GetEmerald(uint8 emerald);
    static void GiveEmerald(uint8 emeraldID);
    static void LoadGameState();
    static void SaveGameState();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SaveGame);
};
} // namespace GameLogic