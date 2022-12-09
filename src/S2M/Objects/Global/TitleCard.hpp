#pragma once
#include "S2M.hpp"

#include "Helpers/FXFade.hpp"

namespace GameLogic
{

struct TitleCard : RSDK::GameObject::Entity {

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
        RSDK::StateMachine<TitleCard> finishedCB;
        RSDK::StateMachine<TitleCard> suppressCB;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<TitleCard> state;
    RSDK::StateMachine<TitleCard> stateDraw;
    int32 actionTimer;
    int32 moveTimer;
    int32 moveTimer1;
    int32 moveTimer2;
    int32 timer;
    bool32 enableIntro;
    RSDK::String zoneName;
    RSDK::Vector2 decorationPos;
    RSDK::Vector2 bluePiecePos;
    RSDK::Vector2 yellowPiecePos;
    RSDK::Vector2 redPiecePos;
    RSDK::Vector2 zonePos;
    RSDK::Vector2 zoneNamePos;
    RSDK::Vector2 actNumPos;
    RSDK::Animator decorationAnimator;
    RSDK::Animator zoneNameAnimator;
    RSDK::Animator actNumbersAnimator;
    RSDK::Animator bluePieceAnimator;
    RSDK::Animator yellowPieceAnimator;
    RSDK::Animator redPieceAnimator;
    RSDK::Animator zoneAnimator;
    uint8 actID;
    uint8 decorationFrame;

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
    void ChangeTitleColors();
    void SetupTitleWords();
    void HandleCamera();

    // States
    void State_SetupBGElements();
    void State_OpeningBG();
    void State_EnterTitle();
    void State_ShowingTitle();
    void State_SlideAway();
    void State_Supressed();

    // Draw States
    void Draw_SlideIn();
    void Draw_ShowTitleCard();
    void Draw_SlideAway();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TitleCard);
};
} // namespace GameLogic