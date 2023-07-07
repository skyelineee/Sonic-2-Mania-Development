#pragma once
#include "S2M.hpp"

#include "Player.hpp"

namespace GameLogic
{

struct ActClear : RSDK::GameObject::Entity {

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
        RSDK::SoundFX sfxScoreAdd;
        RSDK::SoundFX sfxScoreTotal;
        RSDK::SoundFX sfxEvent;
        uint8 bufferMoveEnabled;
        uint8 isSavingGame;
        uint8 disableResultsInput;
        int32 displayedActID;
        uint8 finished;
        uint8 forceNoSave;
        RSDK::StateMachine<ActClear> bufferMove_CB;
        RSDK::StateMachine<ActClear> saveReplay_CB;
        uint8 hasSavedReplay;
        uint8 disableTimeBonus;
        uint8 actClearActive;
        bool32 disableJingle;
        bool32 disableVictory;
        int32 victoryTimer;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<ActClear> state;
    int32 timer;
    int32 topTextSpeed;
    int32 timeBonusSpeed;
    int32 ringBonusSpeed;
    int32 coolBonusSpeed;
    int32 totalScoreSpeed;
    int32 stageFinishTimer;
    int32 slidingInTimer;
    int32 slidingOutTimer;
    int32 timeBonus;
    int32 ringBonus;
    int32 coolBonus;
    int32 totalScore;
    int32 time;
    int32 newRecordTimer;
    uint8 achievedRank;
    uint8 isNewRecord;
    uint8 showCoolBonus;
    RSDK::Vector2 playerNamePos;
    RSDK::Vector2 gotPos;
    RSDK::Vector2 throughPos;
    RSDK::Vector2 actPos;
    RSDK::Vector2 timeBonusPos;
    RSDK::Vector2 ringBonusPos;
    RSDK::Vector2 coolBonusPos;
    RSDK::Vector2 totalScorePos;
    RSDK::Vector2 timeNumPos;
    RSDK::Vector2 ringNumPos;
    RSDK::Vector2 coolNumPos;
    RSDK::Vector2 totalNumPos;
    RSDK::Vector2 triangleLeftPos;
    RSDK::Vector2 triangleRightPos;
    Player *targetPlayer;
    RSDK::Animator hudElementsAnimator;
    RSDK::Animator numbersAnimator;
    RSDK::Animator playerNameAnimator;
    RSDK::Animator gotThroughAnimator;
    RSDK::Animator actNumAnimator;
    RSDK::Animator timeElementsAnimator;
    RSDK::Animator scoreCardAnimator;
    RSDK::Animator trianglesLeftAnimator;
    RSDK::Animator trianglesRightAnimator;
    RSDK::Animator checkerboardBGAnimator;
    int32 listPos;

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

    void State_EnterResults();
    void State_ScoreShownDelay();
    void State_TallyScore();
    void State_SaveGameProgress();
    void State_ShowResultsTA();
    void State_WaitForSave();
    void State_EndEvent();
    void State_RecoverPlayers();

    void DrawNumbers(RSDK::Vector2 *drawPos, int32 value, int32 digitCount);
    void DrawTime(RSDK::Vector2 *drawPos, int32 mins, int32 secs, int32 millisecs);

    static void CheckPlayerVictory();
    static void SetupForceOnScreenP2();
    static void SaveGameCallback(bool32 success);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ActClear);
};
} // namespace GameLogic