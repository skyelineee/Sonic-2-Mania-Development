#pragma once
#include "S2M.hpp"

namespace GameLogic
{

// kinda just adding it here since its kinda relevant, may move elsewhere idk
struct StageFolderInfo {
    char stageFolder[64];
    char spriteFolder[16];
    int8 zoneID;
    uint8 actID;
    bool32 noActID;
    bool32 isSavable;
    int32 listPos;
};

extern StageFolderInfo stageList[];
extern char dynamicPath[0x40];
extern int32 dynamicPathActID;
extern int32 dynamicPathUnknown;

extern void StrCopy(char *dest, uint32 bufferSize, const char *src);
extern void StrAppend(char *dest, uint32 bufferSize, const char *src);

#define RSDK_DYNAMIC_PATH(name)                                                                                                                      \
    GameLogic::StrCopy(GameLogic::dynamicPath, sizeof(GameLogic::dynamicPath), stageList[Zone::sVars->folderListPos].spriteFolder);                  \
    GameLogic::StrAppend(GameLogic::dynamicPath, sizeof(GameLogic::dynamicPath), "/");                                                               \
    GameLogic::StrAppend(GameLogic::dynamicPath, sizeof(GameLogic::dynamicPath), name);                                                              \
    GameLogic::StrAppend(GameLogic::dynamicPath, sizeof(GameLogic::dynamicPath), ".bin");

#define RSDK_DYNAMIC_PATH_ACTID(name)                                                                                                                \
    GameLogic::StrCopy(GameLogic::dynamicPath, sizeof(GameLogic::dynamicPath), stageList[Zone::sVars->folderListPos].spriteFolder);                  \
    GameLogic::StrAppend(GameLogic::dynamicPath, sizeof(GameLogic::dynamicPath), "/");                                                               \
    GameLogic::StrAppend(GameLogic::dynamicPath, sizeof(GameLogic::dynamicPath), name);                                                              \
    if (Zone::sVars->useFolderIDs) {                                                                                                                 \
        GameLogic::dynamicPathActID   = '1' + Zone::sVars->actID;                                                                                    \
        GameLogic::dynamicPathUnknown = 0;                                                                                                           \
        GameLogic::StrAppend(GameLogic::dynamicPath, sizeof(GameLogic::dynamicPath), (const char *)&GameLogic::dynamicPathActID);                    \
    }                                                                                                                                                \
    GameLogic::StrAppend(GameLogic::dynamicPath, sizeof(GameLogic::dynamicPath), ".bin"); 

struct Zone : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum ZoneIDs {
        Invalid = -1,
        OWZ,
        EHZ,
        CPZ,
        ARZ,
        SWZ,
        CNZ,
        HTZ,
        MCZ,
        HPZ,
        SSZ,
        OOZ,
        MTZ,
        CCZ,
        SCZ,
        WFZ,
        DEZ
        // nuggie has finished this task.
    };

    enum ActIDs {
        Act1,
        Act2,
        Act3,
        ActNone,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 actID;
        RSDK::StateMachine<Zone> stageFinishCallback;
        bool32 shouldRecoverPlayers;
        RSDK::StateMachine<Zone> vsSwapCB[16];
        int32 vsSwapCBCount;
        int32 playerSwapEnabled[PLAYER_COUNT];
        uint8 swapPlayerID;
        uint8 swapPlayerCount;
        uint8 preSwapPlayerIDs[PLAYER_COUNT];
        uint8 swappedPlayerIDs[PLAYER_COUNT];
        int32 listPos;
        int32 prevListPos;
        int32 ringFrame;
        int32 timer;
        int32 cameraBoundsL[PLAYER_COUNT];
        int32 cameraBoundsR[PLAYER_COUNT];
        int32 cameraBoundsT[PLAYER_COUNT];
        int32 cameraBoundsB[PLAYER_COUNT];
        int32 playerBoundsL[PLAYER_COUNT];
        int32 playerBoundsR[PLAYER_COUNT];
        int32 playerBoundsT[PLAYER_COUNT];
        int32 playerBoundsB[PLAYER_COUNT];
        int32 deathBoundary[PLAYER_COUNT];
        bool32 playerBoundActiveL[PLAYER_COUNT];
        bool32 playerBoundActiveR[PLAYER_COUNT];
        bool32 playerBoundActiveT[PLAYER_COUNT];
        bool32 playerBoundActiveB[PLAYER_COUNT];
        int32 autoScrollSpeed;
        bool32 setATLBounds;
        bool32 gotTimeOver;
        RSDK::StateMachine<Zone> timeOverCallback;
        uint8 collisionLayers;
        RSDK::SceneLayer fgLayer[2]; // { lowPriority, highPriority }
        RSDK::SceneLayer moveLayer;
        RSDK::SceneLayer scratchLayer;
        int32 fgLayerMask[2]; // { lowPriority, highPriority }
        int32 moveLayerMask;
        uint8 fgDrawGroup[2];     // { lowPriority, highPriority }
        uint8 objectDrawGroup[2]; // { lowPriority, highPriority }
        uint8 playerDrawGroup[2]; // { lowPriority, highPriority }
        uint8 hudDrawGroup;
        RSDK::SoundFX sfxFail;
        bool32 swapGameMode;
        bool32 teleportActionActive;
        int32 randSeed;
        uint16 folderListPos;
        bool32 useFolderIDs;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<Zone> state;
    RSDK::StateMachine<Zone> stateDraw;
    int32 screenID;
    int32 timer;
    int32 fadeSpeed;
    int32 fadeColor;

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

    void HandlePlayerBounds();

    static void StoreEntities(RSDK::Vector2 offset);
    static void ReloadEntities(RSDK::Vector2 offset, bool32 setATLBounds);

    static void StartFadeOut(int32 fadeSpeed, int32 fadeColor);
    static void StartFadeOut_MusicFade(int32 fadeSpeed, int32 fadeColor);
    static void GoBonus_Pinball();
    static void StartTeleportAction();

    static void RotateOnPivot(RSDK::Vector2 *position, RSDK::Vector2 *pivot, int32 angle);

    void Draw_Fade();

    void State_Fade();

    static void ApplyWorldBounds();

    static int32 CurrentID();
    static bool32 CurrentStageSaveable();
    static int32 GetListPos(uint8 zone, uint8 act);

    static void GetTileInfo(int32 x, int32 y, int32 moveOffsetX, int32 moveOffsetY, int32 cPlane, RSDK::Tile *tile, uint8 *flags);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Zone);
};
} // namespace GameLogic