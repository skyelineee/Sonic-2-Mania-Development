#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Music : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum MusicTracks {
        TRACK_NONE         = -1,
        TRACK_STAGE        = 0,
        TRACK_INVINCIBLE   = 1,
        TRACK_SNEAKERS     = 2,
        TRACK_MINIBOSS     = 3,
        TRACK_HBHBOSS      = 4,
        TRACK_EGGMAN1      = 5,
        TRACK_EGGMAN2      = 6,
        TRACK_ACTCLEAR     = 7,
        TRACK_DROWNING     = 8,
        TRACK_GAMEOVER     = 9,
        TRACK_SUPER        = 10,
        TRACK_HBHMISCHIEF  = 11,
        TRACK_SOUNDTEST    = 12,
        TRACK_1UP          = 13,
    };

    enum TrackPriorityValues {
        PriorityNone    = 0,
        PriorityAny     = 10,
        PriorityPowerup = 100,
        PrioritySuper   = 1000,
        PriorityDrown   = 1000,
        Priority1UP     = 10000,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        char trackNames[16][32];
        uint32 trackLoops[16];
        uint32 trackStartPos;
        int32 channelID;
        int32 activeTrack;
        int32 nextTrack;
        int32 prevTrack;
        int32 restartTrackID;
        RSDK::SpriteAnimation aniFrames;
        RSDK::String currentTrack;
        int32 disableNextTrack;
        int32 field_274;
        RSDK::StateMachine<Music> musicSpeedUp;
        RSDK::StateMachine<Music> musicSlowDown;
        bool32 isSpedUp;
        int32 field_2AC;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<Music> state;
    RSDK::String trackFile;
    RSDK::String soundTestTitle;
    int32 trackID;
    uint32 trackLoop;
    bool32 playOnLoad;
    bool32 restartTrack;
    int32 timer;
    int32 trackPriority;
    uint32 trackStartPos;
    float volume;
    float fadeSpeed;
    RSDK::Animator animator;

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

    static void SetMusicTrack(uint8 track, const char *path, uint32 loopPoint);
    static void SwapMusicTrack(uint8 track, const char *swapTrack, uint32 loopPoint, float ratio);

    static void Stop();
    static void Pause();
    static void Resume();
    static bool32 IsPlaying();

    static void PlayJingle(uint8 trackID);
    static void PlayTrack(uint8 trackID);
    void Play();
    static void PlayAutoMusicQueuedTrack(uint8 trackID);
    void HandleMusicStack_Powerups();
    static bool32 CheckMusicStack_Active();
    void GetNextTrackStartPos();
    static void JingleFadeOut(uint8 trackID, bool32 transitionFade);

    void FinishJingle();
    static void ClearMusicStack();

    static void PlayOnFade(uint8 trackID, float fadeSpeed);
    static void FadeOut(float fadeSpeed);

    void State_PlayOnLoad();
    void State_Jingle();
    void State_FadeTrackIn();
    void State_JingleFade();
    void State_StopOnFade();
    void State_PlayOnFade();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Music);
};
} // namespace GameLogic