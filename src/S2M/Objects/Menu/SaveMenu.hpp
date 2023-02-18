#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"

namespace GameLogic
{

struct SaveMenu : RSDK::GameObject::Entity {

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
    static void HandleUnlocks();
    static void SetupActions();
    static void HandleMenuReturn(int32 slot);
    static int32 GetMedalMods();
    static void SaveFileCB(bool32 success);
    static void SecretsTransitionCB();
    static void LeaveSecretsMenu();

    void SaveSel_MenuUpdateCB();
    static void SaveSel_YPressCB();
    void SaveButton_ActionCB();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SaveMenu);
};
} // namespace GameLogic
