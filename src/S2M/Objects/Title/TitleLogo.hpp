#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct TitleLogo : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum TitleLogoTypes {
        TITLELOGO_EMBLEM,
        TITLELOGO_RIBBON,
        TITLELOGO_GAMETITLE,
        TITLELOGO_COPYRIGHT,
        TITLELOGO_PRESSSTART,
    };

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

    int32 type;
    RSDK::StateMachine<TitleLogo> state;
    bool32 showRibbonCenter;
    int32 timer;
    RSDK::Animator mainAnimator;
    RSDK::Animator ribbonCenterAnimator;

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

    void SetupPressStart();
    void State_Ribbon();
    void State_PressButton();
    void State_HandleSetup();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TitleLogo);
};
} // namespace GameLogic