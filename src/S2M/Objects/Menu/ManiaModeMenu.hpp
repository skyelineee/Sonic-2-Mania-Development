#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"
#include "UIButtonPrompt.hpp"

namespace GameLogic
{

struct ManiaModeMenu : RSDK::GameObject::Entity {

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
        int32 unused;
        UIControl *saveSelectMenu;
        UIControl *noSaveMenu;
        UIControl *secretsMenu;
        UIButtonPrompt *delSavePrompt;
        bool32 inSecretsMenu;
        int32 saveSelLastButtonID;
    };

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

    static void Initialize();
    static bool32 InitAPI();
    static void InitLocalization(bool32 success);
    static int32 GetActiveMenu();
    static void ChangeMenuBG();
    static void ChangeMenuTrack();
    static void StartReturnToTitle();
    static void ReturnToTitle();
    static void HandleUnlocks();
    static void SetupActions();
    static void HandleMenuReturn();

    // ==============================
    // STATES
    // ==============================

    void State_HandleTransition();

    void MovePromptCB();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ManiaModeMenu);
};
} // namespace GameLogic