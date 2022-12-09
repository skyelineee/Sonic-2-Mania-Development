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
    int32 stageFinishTimer;
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
    RSDK::Vector2 gotThroughPos;
    RSDK::Vector2 timeBonusPos;
    RSDK::Vector2 ringBonusPos;
    RSDK::Vector2 coolBonusPos;
    RSDK::Vector2 totalScorePos;
    Player *targetPlayer;
    RSDK::Animator hudElementsAnimator;
    RSDK::Animator numbersAnimator;
    RSDK::Animator playerNameAnimator;
    RSDK::Animator playerNameCardAnimator;
    RSDK::Animator gotThroughAnimator;
    RSDK::Animator actNumAnimator;
    RSDK::Animator actNumCardAnimator;
    RSDK::Animator timeElementsAnimator;
    RSDK::Animator scoreNumCardAnimator;
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

    void State_EnterText();
    void State_AdjustText();
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