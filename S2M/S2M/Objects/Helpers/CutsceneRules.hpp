#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct CutsceneRules : RSDK::GameObject::Entity {

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
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    static bool32 CheckAct1();
    static bool32 CheckAct2();
    static bool32 CheckOutroAct2();
    static bool32 CheckOutroAct1();
    static bool32 CheckStageReload();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(CutsceneRules);
};
} // namespace GameLogic