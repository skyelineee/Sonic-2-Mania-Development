#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIText;
struct UIPicture;

struct LevelSelect : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum LevelSelectPlayerIDs {
        LSELECT_PLAYER_NONE,
        LSELECT_PLAYER_SONIC,
        LSELECT_PLAYER_TAILS,
        LSELECT_PLAYER_KNUCKLES,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 bgAniDuration;         // 240;
        int32 bgAniDurationTable[4];//, { 240, 3, 3, 3 });
        //{ 9, 0, 0, 1, 255 }); // says there's 9 values here in static obj & IDA but only 5 are loaded so it is what it is ig
        int32 bgAniFrame;
        RSDK::SoundFX sfxFail;
        RSDK::SoundFX sfxRing;
        RSDK::SoundFX sfxEmerald;
        RSDK::SoundFX sfxContinue;
        RSDK::SoundFX sfxMedalGot;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<LevelSelect> state;
    RSDK::StateMachine<LevelSelect> stateDraw;
    int32 timer;
    int32 labelID;
    int32 leaderCharacterID;
    int32 sidekickCharacterID;
    UIText *zoneNameLabels[32];
    UIText *stageIDLabels[32];
    UIPicture *zoneIcon;
    UIPicture *player1Icon;
    UIPicture *player2Icon;
    int32 labelCount;
    int32 unused1;
    int32 unused2;
    int32 unused3;
    int32 unused4;

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

    void Draw_Fade();

    void State_Init();
    void State_FadeIn();
    void State_Navigate();
    void State_FadeOut();

    void ManagePlayerIcon();
    void SetLabelHighlighted(bool32 highlight);
    void HandleColumnChange();
    void HandleNewStagePos();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(LevelSelect);
};
} // namespace GameLogic
