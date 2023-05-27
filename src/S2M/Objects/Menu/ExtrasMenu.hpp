#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"
#include "UIDiorama.hpp"

namespace GameLogic
{

struct ExtrasMenu : RSDK::GameObject::Entity {

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
        UIControl *extrasControl;
        int32 cheatCode[8];
        UIDiorama *diorama;
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

    void ProcessInputs();

    static void Start_BossRush();
    static void BossRush_ActionCB();

    static void Start_MusicPlayer();
    static void MusicPlayer_ActionCB();

    static void Start_LevelSelect();
    static void LevelSelect_ActionCB();

    static void Start_ExtraLevels();
    static void ExtraLevels_ActionCB();

    static void Start_Credits();
    static void Credits_ActionCB();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ExtrasMenu);
};
} // namespace GameLogic