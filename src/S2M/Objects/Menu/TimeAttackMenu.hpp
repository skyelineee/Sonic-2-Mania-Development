#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"
#include "UIDialog.hpp"
#include "UIReplayCarousel.hpp"
#include "UITABanner.hpp"
#include "UIButtonPrompt.hpp"
#include "UICarousel.hpp"

namespace GameLogic
{

struct TimeAttackMenu : RSDK::GameObject::Entity {

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
        UIControl *timeAttackControl;
        UIControl *taZoneSelControl;
        UIButtonPrompt *switchModePrompt;
        UIControl *taDetailsControl;
        UITABanner *detailsBanner;
        UIButtonPrompt *topRankPrompt;
        UITABanner *leaderboardsBanner;
        UIControl *replaysControl;
        UIReplayCarousel *replayCarousel;
        UIButtonPrompt *replayPrompt;
        UIDialog *connectingDlg;
        bool32 prevIsUser;
        int32 isUser;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<TimeAttackMenu> state;
    RSDK::StateMachine<TimeAttackMenu> callback;
    int32 delay;

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

    static void Initialize();
    static void HandleUnlocks();
    static void SetupActions();
    static void HandleMenuReturn();
    static void DeleteReplayActionCB();
    static void ConfirmDeleteReplay_Yes_CB();
    static void DeleteReplayCB(bool32 success);
    void ReplayCarousel_ActionCB();
    static void WatchReplay(int32 row, bool32 showGhost);
    static void ReplayLoad_CB(bool32 success);
    static void WatchReplayActionCB_ReplaysMenu();
    static void ChallengeReplayActionCB_ReplaysMenu();
    static void WatchReplayCB_RanksMenu();
    static void ChallengeReplayCB_RanksMenu();
    static void LoadScene_Fadeout();
    static void MenuSetupCB_Replay();
    static void MenuUpdateCB_Replay();
    static void MenuUpdateCB();
    static void ReplayButton_ActionCB();
    static void YPressCB_Replay();
    static void SortReplayChoiceCB();
    static void SetupDetailsView();
    void TAModule_ActionCB();
    static void StartTAAttempt();
    static void LoadScene();
    static void YPressCB_ZoneSel();
    static bool32 BackPressCB_ZoneSel();
    static void YPressCB_Details();
    static void ResetTimes_YesCB();
    static void XPressCB_Details();
    void TAZoneModule_ActionCB();
    void RankButton_ActionCB();
    static void MenuSetupCB_Details();
    static void TAZoneModule_ChoiceChangeCB();
    void CharButton_ActionCB();
    static void TransitionToDetailsCB();
    void SetupLeaderboardsCarousel(UICarousel *carousel);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TimeAttackMenu);
};
} // namespace GameLogic