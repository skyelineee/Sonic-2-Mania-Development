#pragma once
#include "S2M.hpp"
#include "Global/Player.hpp"

namespace GameLogic
{

// no clue what this is...
// most signatures are recognisable like "SCN" for scenes
// but as far as I can tell, this one is just random numbers?
#define REPLAY_SIGNATURE (0xF6057BED)

#define REPLAY_MAX_FRAMECOUNT (37447)

struct ReplayRecorder : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum ReplayFrameInfoTypes {
        REPLAY_INFO_NONE,
        REPLAY_INFO_STATECHANGE,
        REPLAY_INFO_USEFLAGS,
        REPLAY_INFO_PASSEDGATE,
    };

    enum ReplayFrameChangeTypes {
        REPLAY_CHANGED_NONE    = 0 << 0,
        REPLAY_CHANGED_INPUT   = 1 << 0,
        REPLAY_CHANGED_POS     = 1 << 1,
        REPLAY_CHANGED_VEL     = 1 << 2,
        REPLAY_CHANGED_GIMMICK = 1 << 3,
        REPLAY_CHANGED_DIR     = 1 << 4,
        REPLAY_CHANGED_ROT     = 1 << 5,
        REPLAY_CHANGED_ANIM    = 1 << 6,
        REPLAY_CHANGED_FRAME   = 1 << 7,
    };

    // ==============================
    // STRUCTS
    // ==============================

    struct ReplayHeader {
        uint32 signature;
        int32 version;
        bool32 isPacked;
        bool32 isNotEmpty;
        int32 frameCount;
        int32 startingFrame;
        int32 zoneID;
        int32 act;
        int32 characterID;
        int32 oscillation;
        int32 bufferSize;
        float averageFrameSize;
        int32 unused;
    };

    struct ReplayFrame {
        uint8 info;
        uint8 changedValues;
        uint8 inputs;
        uint8 direction;
        RSDK::Vector2 position;
        RSDK::Vector2 velocity;
        int32 rotation;
        uint8 anim;
        uint8 frame;
    };

    struct Replay {
        ReplayHeader header;
        ReplayFrame frames[REPLAY_MAX_FRAMECOUNT];
        // there's an extra 4 bytes here, but they're just padding to make the size correct
        int32 padding;
    };


    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::StateMachine<ReplayRecorder> actions[64];
        int32 frameCounter;
        Replay *recordBuffer;
        Replay *playbackBuffer;
        ReplayFrame *recordingFrames;
        ReplayFrame *playbackFrames;
        ReplayRecorder *recordingManager;
        ReplayRecorder *playbackManager;
        bool32 initialized;
        int32 startRecording;
        int32 startPlayback;
        int32 savedReplay;
        int32 startedRecording;
        int32 isReplaying;
        int32 hasSetupGhostVS;
        int32 hasSetupGhostView;
        int32 passedStartLine;
        int32 reachedGoal;
        int32 packedStartFrame;
        void *fileBuffer;
        void (*loadCallback)(bool32 success);
        char filename[0x100];
        void (*saveCallback)(bool32 success);
        int32 replayID;
        int32 replayRowID;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<ReplayRecorder> state;
    RSDK::StateMachine<ReplayRecorder> stateLate;
    RSDK::StateMachine<Player> ghostPlayerState;
    RSDK::StateMachine<Player> prevPlayerState;
    RSDK::StateMachine<Player> curPlayerState;
    uint16 animID;
    uint16 frameID;
    Player *player;
    int32 paused;
    int32 changeFlags;
    int32 isGhostPlayback;
    int32 replayFrame;
    int32 maxFrameCount;
    int32 replayStopDelay;
    uint8 storedInputs;
    uint8 storedDirection;
    RSDK::Vector2 storedPos;
    RSDK::Vector2 storedVel;
    int32 storedRotation;
    int32 storedSpeed;
    uint8 storedAnim;
    uint16 storedFrame;
    int32 ghostAlpha;

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

    void TitleCardCB();
    static void Resume(ReplayRecorder *recorder);
    void StartCB();
    void FinishCB();
    void Buffer_Move();
    static void SaveReplayDLG_NoCB();
    static void SaveReplayDLG_YesCB();
    static void SaveReplayDLG_CB();
    void SaveReplay();
    static void SaveFile_Replay(bool32 success);
    static void SaveCallback_ReplayDB(bool32 success);
    static void SaveCallback_TimeAttackDB(bool32 success);
    static void Buffer_PackInPlace(int32 *tempWriteBuffer);
    static void Buffer_Unpack(int32 *readBuffer, int32 *tempReadBuffer);
    static void Buffer_LoadFile(const char *fileName, void *buffer, void (*callback)(bool32 success));
    static void Buffer_SaveFile(const char *fileName, int32 *buffer, void (*callback)(bool32 success));
    static void LoadReplayCallback(int32 status);
    static void SaveReplayCallback(int32 status);
    void ConfigureGhost_CB();
    static void SetupActions();
    static void SetupWriteBuffer();
    void DrawGhostDisplay();
    static void Record(ReplayRecorder *recorder, Player *player);
    static void StartRecording(Player *player);
    static void Play(Player *player);
    static void Rewind(ReplayRecorder *recorder);
    static void Seek(ReplayRecorder *recorder, uint32 frame);
    static void SeekFunc(ReplayRecorder *recorder);
    static void Stop(ReplayRecorder *recorder);
    static void SetGimmickState(ReplayRecorder *recorder, bool32 allowSpriteChanges);
    static void ForceApplyFramePtr(ReplayRecorder *recorder, ReplayFrame *framePtr);
    static void ApplyFramePtr(ReplayRecorder *recorder, ReplayFrame *framePtr);
    static bool32 CheckPlayerGimmickState(ReplayRecorder *recorder);
    void PackFrame(ReplayFrame *recording);
    void PlayBackInput();
    static void Pause(ReplayRecorder *recorder);
    void PlayerState_PlaybackReplay();
    void Late_Playback();
    void Late_RecordFrames();

    void State_SetupPlayback();
    void State_Playback();
    void State_Record();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ReplayRecorder);
};
} // namespace GameLogic