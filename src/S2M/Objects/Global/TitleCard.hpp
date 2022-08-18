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
    int32 timer;
    RSDK::Vector2 decorationPos;
    int32 stripPos[4];
    RSDK::Vector2 vertMovePos[2];
    RSDK::Vector2 vertTargetPos[2];
    RSDK::Vector2 word2DecorVerts[4];
    RSDK::Vector2 word1DecorVerts[4];
    RSDK::Vector2 zoneDecorVerts[4];
    RSDK::Vector2 stripVertsBlue[4];  
    RSDK::Vector2 stripVertsRed[4];   
    RSDK::Vector2 stripVertsOrange[4];
    RSDK::Vector2 stripVertsGreen[4]; 
    RSDK::Vector2 bgLCurtainVerts[4];
    RSDK::Vector2 bgRCurtainVerts[4];
    RSDK::String zoneName;
    int32 zoneCharPos[4];
    int32 zoneCharVel[4];
    int32 zoneXPos;
    RSDK::Vector2 charPos[20];
    int32 charVel[20];
    int32 titleCardWord2;
    int32 word1Width;
    int32 word2Width;
    int32 word1XPos;
    int32 word2XPos;
    uint8 actID;
    int32 actNumScale;
    RSDK::Vector2 actNumPos;
    bool32 enableIntro;
    RSDK::Animator decorationAnimator;
    RSDK::Animator nameLetterAnimator;
    RSDK::Animator zoneLetterAnimator;
    RSDK::Animator actNumbersAnimator;
    color colors[5];

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
    void SetupColors();
    void SetupVertices();
    void SetupTitleWords();
    void HandleWordMovement();
    void HandleZoneCharMovement();
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