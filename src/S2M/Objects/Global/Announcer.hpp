#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Announcer : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxThree;
        RSDK::SoundFX sfxTwo;
        RSDK::SoundFX sfxOne;
        RSDK::SoundFX sfxGo;
        RSDK::SoundFX sfxGoal;
        RSDK::SoundFX sfxNewRecordTop;
        RSDK::SoundFX sfxNewRecordMid;
        RSDK::SoundFX sfxSonic;
        RSDK::SoundFX sfxTails;
        RSDK::SoundFX sfxKnuckles;
        bool32 finishedCountdown;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<Announcer> state;
    RSDK::StateMachine<Announcer> stateDraw;
    int32 timer;
    int32 playerID;
    int32 screen;
    RSDK::Vector2 drawOffset;
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
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    static void StartCountdown();
    static void AnnounceGoal(int32 screen);

    void Draw_Countdown();
    void Draw_Finished();

    void State_Countdown();
    void State_Finished();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Announcer);
};
} // namespace GameLogic