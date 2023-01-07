#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UILoadingIcon : RSDK::GameObject::Entity {

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
        int32 timer;
        UILoadingIcon *activeSpinner;
    };

    RSDK::StateMachine<UILoadingIcon> state;
    int32 timer;
    bool32 fadedIn;
    RSDK::Animator animator;

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

    static void StartWait();
    static void FinishWait();
    void State_Show();
    void State_Hide();

    // ==============================
    // STATES
    // ==============================

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UILoadingIcon);
};
} // namespace GameLogic
