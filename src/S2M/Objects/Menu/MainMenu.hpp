#pragma once
#include "S2M.hpp"

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

    void Initialize();
    bool32 BackPressCB_ReturnToTitle();
    void ExitGame();
    void ExitButton_ActionCB();
    void StartExitGame();
    void MenuButton_ActionCB();
    void HandleUnlocks();
    void SetupActions();
    void MenuSetupCB();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(MainMenu);
};
} // namespace GameLogic
