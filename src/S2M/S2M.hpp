#pragma once

#include "Game.hpp"

// Structs
struct Vector3 {
    int32 x;
    int32 y;
    int32 z;
};

// Enums

enum GameModes {
    MODE_NOSAVE,
    MODE_MANIA,
    MODE_ENCORE,
    MODE_TIMEATTACK,
    MODE_COMPETITION,
};

enum PlayerIDs {
    ID_NONE     = 0 << 0,
    ID_SONIC    = 1 << 0,
    ID_TAILS    = 1 << 1,
    ID_KNUCKLES = 1 << 2,

    ID_TAILS_ASSIST    = ID_TAILS << 8,
    ID_KNUCKLES_ASSIST = ID_KNUCKLES << 8, // custom-added, can be used to check if "& knux" is active
    ID_DEFAULT_PLAYER  = ID_SONIC | ID_TAILS_ASSIST,
};

#define GET_CHARACTER_ID(playerNum)            (((globals->playerID >> (8 * ((playerNum)-1))) & 0xFF))
#define CHECK_CHARACTER_ID(characterID, plrID) (((globals->playerID >> (8 * ((plrID)-1))) & 0xFF) == (characterID))

enum SaveSlots { NO_SAVE_SLOT = 255 };

enum PlaneFilterTypes {
    PLANEFILTER_NONE,
    PLANEFILTER_AL, // - Plane A, Low Layer
    PLANEFILTER_BL, // - Plane B, Low Layer
    PLANEFILTER_AH, // - Plane A, High Layer
    PLANEFILTER_BH, // - Plane B, High Layer
};

enum ReservedEntities {
    SLOT_PLAYER1     = 0,
    SLOT_PLAYER2     = 1,
    SLOT_PLAYER3     = 2,
    SLOT_PLAYER4     = 3,
    SLOT_POWERUP1    = 4,
    SLOT_POWERUP2    = 5,
    SLOT_POWERUP3    = 6,
    SLOT_POWERUP4    = 7,
    SLOT_POWERUP1_2  = 8,
    SLOT_POWERUP2_2  = 9,
    SLOT_POWERUP3_2  = 10,
    SLOT_POWERUP4_2  = 11,
    SLOT_BSS_SETUP   = 8,
    SLOT_PBL_SETUP   = 8,
    SLOT_UFO_SETUP   = 8,
    SLOT_MUSIC       = 9,
    SLOT_BSS_HUD     = 10,
    SLOT_UFO_CAMERA  = 10,
    SLOT_PBL_CAMERA  = 10,
    SLOT_BSS_MESSAGE = 11,
    SLOT_UFO_HUD     = 11,
    SLOT_ZONE        = 12,
    // 13 = ???
    // 14 = ???
    SLOT_CUTSCENESEQ             = 15,
    SLOT_PAUSEMENU               = 16,
    SLOT_GAMEOVER                = 16,
    SLOT_ACTCLEAR                = 16,
    SLOT_PAUSEMENU_UICONTROL     = 17,
    SLOT_PAUSEMENU_BUTTON1       = 18,
    SLOT_PAUSEMENU_BUTTON2       = 19,
    SLOT_PAUSEMENU_BUTTON3       = 20,
    SLOT_DIALOG                  = 21,
    SLOT_DIALOG_UICONTROL        = 22,
    SLOT_DIALOG_BUTTONS          = 23,
    SLOT_DIALOG_BUTTON2          = 24,
    SLOT_DIALOG_BUTTON3          = 25,
    SLOT_POPOVER                 = 26,
    SLOT_POPOVER_UICONTROL       = 27,
    SLOT_POPOVER_BUTTONS         = 28,
    SLOT_POPOVER_BUTTON2         = 29,
    SLOT_POPOVER_BUTTON3         = 30,
    SLOT_POPOVER_BUTTON4         = 31,
    SLOT_BIGBUBBLE_P1            = 32,
    SLOT_BIGBUBBLE_P2            = 33,
    SLOT_BIGBUBBLE_P3            = 34,
    SLOT_BIGBUBBLE_P4            = 36,
    SLOT_BSS_HORIZON             = 32,
    SLOT_UFO_SPEEDLINES          = 34,
    SLOT_UFO_PLASMA              = 36,
    SLOT_REPLAYRECORDER_PLAYBACK = 36,
    SLOT_REPLAYRECORDER_RECORD   = 37,
    SLOT_MUSICSTACK_START        = 40,
    //[41-47] are part of the music stack
    SLOT_MUSICSTACK_END = 48,
    SLOT_CAMERA1        = 60,
    SLOT_CAMERA2        = 61,
    SLOT_CAMERA3        = 62,
    SLOT_CAMERA4        = 63,

    SLOT_HP_HALFPIPE     = 0,
    SLOT_HP_BG           = 1,
    SLOT_HP_PLAYER1      = 2,
    SLOT_HP_PLAYER2      = 3,
    SLOT_HP_HUD          = 10,
    SLOT_HP_MESSAGE      = 11,
    SLOT_HP_SPECIALCLEAR = 20,
};

enum GameCheats {
    SECRET_RICKYMODE      = 1 << 0,
    SECRET_SUPERDASH      = 1 << 1,
    SECRET_REGIONSWAP     = 1 << 2,
    SECRET_CAMERATRACKING = 1 << 3,
    SECRET_RANDOMITEMS    = 1 << 4,
    SECRET_BLUESHIELDMODE = 1 << 5,
    SECRET_NOITEMS        = 1 << 6,
};

enum MedalMods {
    MEDAL_DEBUGMODE   = 1 << 0,
    MEDAL_ANDKNUCKLES = 1 << 1,
    MEDAL_PEELOUT     = 1 << 2,
    MEDAL_INSTASHIELD = 1 << 3,
    MEDAL_NODROPDASH  = 1 << 4,
    MEDAL_NOTIMEOVER  = 1 << 5,
    MEDAL_NOLIVES     = 1 << 6,
};

enum GameTypes {
    GAME_S1,
    GAME_CD,
    GAME_S2,
    GAME_SM,
    GAME_S3K,
    GAME_S3,
    GAME_SK,
};

enum PresenceTypes {
    PRESENCE_GENERIC,
    PRESENCE_MENU,
    PRESENCE_MANIA,
    PRESENCE_TA,
    PRESENCE_TITLE,
};

// Global Variables

// forward declare
struct GlobalVariables;

extern GlobalVariables *globals;

struct GlobalVariables {
    struct Constructor {
        Constructor()
        {
#if RETRO_REV0U
            RegisterGlobals((void **)&globals, sizeof(GlobalVariables), &GlobalVariables::Init);
#else
            RegisterGlobals((void **)&globals, sizeof(GlobalVariables));
#endif
        }
    };

    static Constructor c;

#if RETRO_REV0U
    static void Init(void *g);
#endif

    // Global Variables
    int32 gameMode;
    int32 playerID; // active character IDs (usually leader & sidekick)
    int32 specialCleared;
    int32 specialRingID;
    int32 atlEnabled;
    int32 atlEntityCount;
    int32 atlEntitySlot[0x20];
    int32 atlEntityData[0x4000];
    int32 saveLoaded;
    int32 saveRAM[0x4000];
    int32 saveSlotID;
    int32 noSaveSlot[0x400];
    int32 menuParam[0x4000];
    int32 itemMode;
    int32 suppressTitlecard;
    int32 suppressAutoMusic;
    int32 competitionSession[0x4000];
    int32 medalMods;
    int32 parallaxOffset[0x100];
    int32 enableIntro;
    int32 optionsLoaded;
    int32 optionsRAM[0x80];
    int32 medallionDebug;
    int32 notifiedAutosave;
    int32 recallEntities;
    int32 restartRings;
    int32 restart1UP;
    int32 restartPowerups;
    RSDK::Vector2 restartPos[4];
    int32 restartSlot[4];
    int32 restartDir[4];
    int32 restartMinutes;
    int32 restartSeconds;
    int32 restartMilliseconds;
    int32 tempMinutes;
    int32 tempSeconds;
    int32 tempMilliseconds;
    int32 restartScore;
    int32 restartScore1UP;
    int32 restartLives[4];
    int32 restartMusicID;
    int32 restartFlags;
    int32 tempFlags;
    int32 continues;
    int32 initCoolBonus;
    int32 coolBonus[4];
    int32 stock;
    int32 characterFlags;
    int32 vapeMode;
    int32 secrets;
    int32 superSecret;
    int32 superMusicEnabled;
    int32 playerSpriteStyle;
    int32 gameSpriteStyle;
    int32 ostStyle;
    int32 starpostStyle;
    int32 gravityDir;
    bool32 useManiaBehavior;
    int32 tileCollisionMode;
    int32 persistentTimer;
    int32 atlCameraBoundsL[PLAYER_COUNT];
    int32 atlCameraBoundsR[PLAYER_COUNT];
    int32 atlCameraBoundsT[PLAYER_COUNT];
    int32 atlCameraBoundsB[PLAYER_COUNT];
    RSDK::Vector2 atlCameraPos[PLAYER_COUNT];
    RSDK::Vector2 atlOffset;
};

// Game Helpers

namespace RSDK
{

// just the default keymappings for rn, these should be added to game api input.hpp but I cba
enum KeyMappings {
    KEYMAP_AUTO_MAPPING = -1,
    KEYMAP_NO_MAPPING   = 0,
};

template <typename R> struct Action
{

    R (Action::*action)();

    inline void Init() { action = nullptr; }

    template <typename T> inline bool Set(R (T::*action)())
    {
        // converts from T:: -> Action:: without the compiler interfering :]
        union
        {
            R (T::*in)();
            R (Action::*out)();
        };
        in = action;

        this->action = out;
        return true;
    }

    inline bool Set(R (*action)())
    {
        // converts from T:: -> Action:: without the compiler interfering :]
        union
        {
            R (*in)();
            R (Action::*out)();
        };
        in = action;

        this->action = out;
        return true;
    }

    template <typename T> inline R SetAndRun(R (T::*action)(), void *self = nullptr)
    {
        bool applied = Set(action);

        if (applied)
            return Run(self);

        return R();
    }

    template <typename T> inline R SetAndRun(R (*action)(), void *self = nullptr)
    {
        bool applied = Set(action);

        if (applied)
            return Run(self);

        return R();
    }

    inline R Run(void *self)
    {
        if (action)
        {
        	return (((Action *)self)->*action)();
        }

        return R();
    }

    template <typename T> inline bool Matches(void *other)
    {
        // converts from Action:: -> void (*)() without the compiler interfering :]
        union
        {
            R *in;
            R (Action::*out)();
        };
        in = other;

        return action == out;
    }

    template <typename T> inline bool Matches(R (T::*other)()) { return action == (R (Action::*)())other; }

    inline bool Matches(Action *other)
    {
        if (other == nullptr)
            return action == nullptr;
        else
            return action == other->action;
    }

    inline void Copy(Action *other)
    { 
        if (other == nullptr)
            this->action = nullptr;
        else
            this->action = other->action;
    }

    // Equals
    inline void operator=(const Action &rhs) { this->Copy((Action *)&rhs); }

    // Conditionals
    inline bool operator==(const Action &rhs) { return this->Matches((Action *)&rhs); }
    inline bool operator!=(const Action &rhs) { return !(*this == rhs); }
};

} // namespace RSDK

#define isMainGameMode() (globals->gameMode == MODE_MANIA || globals->gameMode == MODE_ENCORE)

#define S2M_UI_ITEM_BASE(type)                                                                                                                       \
    RSDK::StateMachine<type> state;                                                                                                                  \
    RSDK::Action<void> processButtonCB;                                                                                                              \
    RSDK::Action<bool32> touchCB;                                                                                                                    \
    RSDK::Action<void> actionCB;                                                                                                                     \
    RSDK::Action<void> selectedCB;                                                                                                                   \
    RSDK::Action<void> failCB;                                                                                                                       \
    RSDK::Action<void> buttonEnterCB;                                                                                                                \
    RSDK::Action<void> buttonLeaveCB;                                                                                                                \
    RSDK::Action<bool32> checkButtonEnterCB;                                                                                                         \
    RSDK::Action<bool32> checkSelectedCB;                                                                                                            \
    int32 timer;                                                                                                                                     \
    RSDK::Vector2 startPos;                                                                                                                          \
    Entity *parent;                                                                                                                                  \
    RSDK::Vector2 touchPosSizeS;                                                                                                                     \
    RSDK::Vector2 touchPosOffsetS;                                                                                                                   \
    bool32 touchPressed;                                                                                                                             \
    RSDK::Vector2 touchPosSizeM[4];   /*size of the touchPos: in 16-bit shifted format*/                                                             \
    RSDK::Vector2 touchPosOffsetM[4]; /*offset of the touchPos: 0,0 is entity pos, negative is left/up, positive is right/down*/                     \
    RSDK::Action<void> touchPosCallbacks[4];                                                                                                         \
    int32 touchPosCount;                                                                                                                             \
    int32 touchPosID;                                                                                                                                \
    bool32 isSelected;                                                                                                                               \
    bool32 disabled;
