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
        TimeAttackMain,
        TimeAttackElse,
        SaveSelect,
        OptionsMain,
        OptionsElse,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        bool32 initializedMenuReturn;
        bool32 initializedMenu;
        bool32 initializedAPI;
        bool32 initializedSaves;
        int32 unused2;
        int32 unused3;
        FXFade *fxFade;
        int32 paletteTimer;
    };

    RSDK::StateMachine<MenuSetup> state;
    void (*callback)();
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

    static void StartTransition(void (*callback)(), int32 delay);

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
