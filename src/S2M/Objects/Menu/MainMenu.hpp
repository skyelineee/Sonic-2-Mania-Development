#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"
#include "UIDiorama.hpp"
#include "UIButtonPrompt.hpp"

namespace GameLogic
{

struct MainMenu : RSDK::GameObject::Entity {

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
        UIControl *menuControl;
        UIButtonPrompt *confirmPrompt;
        UIDiorama *diorama;
    };

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

    static void Initialize();
    static bool32 BackPressCB_ReturnToTitle();
    static void ExitGame();
    static void ExitButton_ActionCB();
    static void StartExitGame();
    void MenuButton_ActionCB();
    static void HandleUnlocks();
    static void SetupActions();
    static void MenuSetupCB();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(MainMenu);
};
} // namespace GameLogic
