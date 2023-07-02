#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UICharButton : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum UICharButtonCharacterIds {
        UICHARBUTTON_SONIC,
        UICHARBUTTON_TAILS,
        UICHARBUTTON_KNUX,
    };

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

    S2M_UI_ITEM_BASE(UICharButton);
    uint8 characterID;
    int32 buttonBounceOffset;
    int32 buttonBounceVelocity;
    RSDK::Animator playerAnimator;

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

    void DrawButton();

    void SelectedCB();
    bool32 CheckButtonEnterCB();
    bool32 CheckSelectedCB();
    void ButtonEnterCB();
    void ButtonLeaveCB();

    // ==============================
    // STATES
    // ==============================

    void State_HandleButtonLeave();
    void State_HandleButtonEnter();
    void State_Selected();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UICharButton);
};
} // namespace GameLogic