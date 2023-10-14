#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIPicture;

struct ThanksSetup : RSDK::GameObject::Entity {

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
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<ThanksSetup> state;
    RSDK::StateMachine<ThanksSetup> stateDraw;
    int32 timer;

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

    void State_ThanksForPlaying();
    void State_FadeOut();

    void Draw_Fade();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ThanksSetup);
};
} // namespace GameLogic
