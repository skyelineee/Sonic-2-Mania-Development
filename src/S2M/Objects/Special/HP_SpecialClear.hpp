#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct HP_SpecialClear : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum Messages {
        MessageNone,
        MessageFailed,
        MessageEmeraldGet,
        MessageAllEmeralds,
        MessageCanChangeForm,
        MessageToHiddenPalace,
    };

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
        RSDK::SoundFX sfxWarp;
        RSDK::SoundFX sfxEvent;
        bool32 hiddenPalaceEnabled;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<HP_SpecialClear> state;
    RSDK::StateMachine<HP_SpecialClear> stateDraw;
    RSDK::Animator uiElementsAnimator;
    RSDK::Animator numbersAnimator;
    RSDK::Animator emeraldsAnimator;
    RSDK::Animator messageAnimator;
    int32 score;
    int32 score1UP;
    int32 lives;
    int32 timer;
    int32 textTopPos;
    int32 textMidPos;
    int32 textBottomPos;
    int32 scorePos;
    int32 ringPos[2];
    int32 gemPos;
    int32 ringBonus[2];
    int32 gemBonus;
    int32 resultsTextTop;
    int32 resultsTextMid;
    int32 resultsTextBottom;
    int32 emeraldAlpha;
    int16 alphaTimer;
    color fillColor;

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
    
    void GiveScoreBonus(int32 score);
    void DrawNumbers(RSDK::Vector2 *drawPos, int32 value, int32 digitCount);

    // States
    void State_FadeIn();
    void State_FadeIdle();
    void State_EnterText();
    void State_EnterEmeralds();
    void State_TallyScore();
    void State_Failed();
    void State_FadeOut();
    void State_ExitResults();
    void State_WinShowReward();
    void State_WinEnterText();
    void State_WinEvent();
    void State_WinIdle();

    // Draw States
    void Draw_FadeIn();
    void Draw_Results();
    void Draw_FadeOut();
    void Draw_Exit();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(HP_SpecialClear);
};
} // namespace GameLogic