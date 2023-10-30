#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct EHZSetup : RSDK::GameObject::Entity {

    struct Static : RSDK::GameObject::Static {
        int32 paletteTimer;
        RSDK::TileLayer *background;
        int32 deformation[64];
        bool32 cutsceneActivated;
        int32 cutsceneTimer;
    };

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif
    static void StageLoad();
    static void Serialize();
#if RETRO_INCLUDE_EDITOR
    void EditorDraw();
    static void EditorLoad();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    static void StageFinish_EndAct1();
    static void HandleActTransition();
    static void StageFinish_EndAct2();

    RSDK_DECLARE(EHZSetup);
};
} // namespace GameLogic
