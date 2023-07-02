#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIRankButton : RSDK::GameObject::Entity {

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
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    S2M_UI_ITEM_BASE(UIRankButton);
    bool32 showsName;
    RSDK::LeaderboardEntry *leaderboardEntry;
    RSDK::String rankText;
    RSDK::String nameTimeText;
    RSDK::LeaderboardEntry *prevLeaderboardEntry;
    int32 prevLeaderboardEntryStatus;
    RSDK::Vector2 popoverPos;
    bool32 hasChanged; // never actually set to true, but who knows!
    int32 rank;
    int32 score;
    int32 replayID;
    RSDK::Vector2 size;
    int32 bgEdgeSize; // just a guess but maybe its right, its unused anyways
    int32 buttonBounceOffset;
    int32 buttonBounceVelocity;
    bool32 textVisible;
    RSDK::Animator textAnimator; // unused, despite being set in Create
    RSDK::Animator fontAnimator;
    RSDK::Animator replayIconAnimator;
    RSDK::Animator rankingAnimator;
    RSDK::Animator detailIcon;
    RSDK::SpriteAnimation textFrames;

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

    static void SetRankText(UIRankButton *button, int32 rank);
    static void SetTimeAttackRank(UIRankButton *button, int32 rank, int32 score, int32 replayID);
    static void SetupLeaderboardRank(UIRankButton *button, RSDK::LeaderboardEntry *entry);

    void DrawSprites();

    bool32 CheckButtonEnterCB();
    bool32 CheckSelectedCB();
    void ButtonEnterCB();
    void ButtonLeaveCB();
    static void FailCB();
    void SelectedCB();

    // ==============================
    // STATES
    // ==============================

    void State_HandleButtonLeave();
    void State_HandleButtonEnter();
    void State_Selected();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIRankButton);
};
} // namespace GameLogic