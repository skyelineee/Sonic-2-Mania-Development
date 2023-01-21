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

    enum EmeraldBits {
        ChaosEmerald       = 0,
        SuperEmerald       = 8,
        AllowSuperEmeralds = 16, // set when you reach MHZ or later I think
        VisitedHPZ         = 17, // set when you finish visit HPZ after a special stage
    };

    enum RecallStates {
        RecallNormal,
        RecallDisabled,
        RecallBrokenItemBox,
    };

    enum SaveStates {
        SaveEmpty,
        SaveActive,
        SaveCompleted,
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
    int32 collectedEmeralds;
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
    static void SaveLoadedCB(bool32 success);
    static void SaveProgress();
    static void ClearNoSave();
    static void ClearSaveSlot(uint8 slotID, void (*callback)(bool32 success));
    static void ClearRestartData();
    static void StoreStageState();
    static void SavePlayerState();
    static void LoadGameState();
    static void SaveGameState();
    static bool32 GetEmeralds(EmeraldCheckTypes type);
    static void SetEmeralds(EmeraldCheckTypes type);
    static bool32 GetEmerald(uint8 emerald);
    static void SetEmerald(uint8 emeraldID);
    static void ClearCollectedSpecialRings();
    static bool32 GetCollectedSpecialRing(uint8 id);
    static void SetCollectedSpecialRing(uint8 id);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SaveGame);
};
} // namespace GameLogic