#pragma once
#include "S2M.hpp"
#include "Helpers/FXFade.hpp"

namespace GameLogic
{

struct MenuSetup : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum MainMenuIDs {
        Main,
        TimeAttack,
        SaveSelect,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 unused1;
        bool32 initializedMenuReturn;
        bool32 initializedMenu;
        bool32 initializedAPI;
        bool32 initializedSaves;
        int32 unused2;
        int32 unused3;
        FXFade *fxFade;
    };

    RSDK::StateMachine<MenuSetup> state;
    RSDK::StateMachine<MenuSetup> callback;
    int32 timer;
    int32 delay;
    int32 fadeShift;
    int32 fadeTimer;
    int32 fadeColor;

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

    template <typename T> static inline void StartTransition(void (T::*callback)(), int32 delay)
    {
        MenuSetup *menuSetup = RSDK::GameObject::Create<MenuSetup>(nullptr, -0x100000, -0x100000);

        menuSetup->active    = RSDK::GameObject::ACTIVE_ALWAYS;
        menuSetup->fadeColor = 0x000000;
        menuSetup->fadeShift = 5;
        menuSetup->delay     = delay;
        // menuSetup->state.Set(&ManiaModeMenu::State_HandleTransition);
        menuSetup->callback.Set(&callback);
    }

    static void SaveFileCB(bool32 success);

    // ==============================
    // STATES
    // ==============================

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(MenuSetup);
};
} // namespace GameLogic
