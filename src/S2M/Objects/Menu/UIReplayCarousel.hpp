#pragma once
#include "S2M.hpp"
#include "UIButtonPrompt.hpp"

namespace GameLogic
{

struct UIReplayCarousel : RSDK::GameObject::Entity {

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
        UIButtonPrompt *prompt;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    S2M_UI_ITEM_BASE(UIReplayCarousel);
    RSDK::String createdAtText[4];
    RSDK::StateMachine<UIReplayCarousel> stateDraw;
    int16 curReplayID;
    uint16 sortedRowCount;
    bool32 isMoving;
    int32 curViewOffset;
    int32 targetViewOffset;
    uint8 visibleReplayCount;
    uint8 visibleReplayOffset;
    int32 popoverPos;
    uint8 prevReplayOffset;
    RSDK::Animator detailsAnimator;
    RSDK::Animator createTimeAnimator;
    RSDK::Animator buttonAnimator;
    RSDK::Animator buttonZoneAnimator;
    int32 language;
    bool32 currentlySelected;

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

    void ProcessButtonCB();
    void HandlePositions();
    void SetupButtonCallbacks();
    void HandleTouchPositions();
    void TouchedCB();
    void SetupVisibleReplayButtons();
    void DrawStageInfo(int32 drawX, int32 drawY, uint8 act, uint8 characterID, int32 score, int32 replayID);
    void DrawReplayInfo(int32 drawX, int32 drawY, uint8 zoneID, uint8 act, uint8 characterID, int32 score,
                        int32 replayID);
    void StartAction();
    void SetUnselected();

    void Draw_Loading();
    void Draw_NoReplays();
    void Draw_Carousel();

    void State_Init();
    void State_Unselected();
    void State_Selected();
    void State_StartAction();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIReplayCarousel);
};
} // namespace GameLogic