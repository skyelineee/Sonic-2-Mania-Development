#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"

namespace GameLogic
{

struct UIDiorama : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum UIDioramaIDs {
        UIDIORAMA_ADVENTURE,
        UIDIORAMA_TIMEATTACK,
        UIDIORAMA_OPTIONS,
        UIDIORAMA_EXTRAS,
        UIDIORAMA_EXIT,
    } ;

    // ==============================
    // STRUCTS
    // ==============================

    struct UIDiorama_StateInfo_Adventure {
        RSDK::Animator ehzAnimator;
    };

    struct UIDiorama_StateInfo_TimeAttack {
        RSDK::Animator tempAnimator;
    };

    struct UIDiorama_StateInfo_Options {
        RSDK::Animator tempAnimator1;
    };

    struct UIDiorama_StateInfo_Extras {
        RSDK::Animator tempAnimator2;
    };

    struct UIDiorama_StateInfo_Exit {
        RSDK::Animator tempAnimator3;
    };

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
    };

    int32 dioramaID;
    int32 lastDioramaID;
    int32 timer;
    uint8 parentActivity;
    bool32 needsSetup;
    RSDK::StateMachine<UIDiorama> state;
    RSDK::StateMachine<UIDiorama> stateDraw;
    UIControl *parent;
    int32 maskColor;
    RSDK::Vector2 dioramaPos;
    RSDK::Vector2 dioramaSize;
    RSDK::Animator staticAnimator;
    int32 values[16];
    RSDK::Animator animators[5];

    // ==============================
    // INSTANCE VARS
    // ==============================

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

    void ChangeDiorama(uint8 dioramaID);

    // ==============================
    // STATES
    // ==============================

    void State_Adventure();
    void State_TimeAttack();
    void State_Options();
    void State_Extras();
    void State_Exit();

    // ==============================
    // DRAW STATES
    // ==============================

    void Draw_Adventure();
    void Draw_TimeAttack();
    void Draw_Options();
    void Draw_Extras();
    void Draw_Exit();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIDiorama);
};
} // namespace GameLogic