#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct EHZSetup : RSDK::GameObject::Entity {

    struct Static : RSDK::GameObject::Static {
        int32 paletteTimer;
    };

    RSDK::Vector2 storePos;

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
    void EditorDraw();
    static void EditorLoad();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    void StageFinish_EndAct1();
    void HandleActTransition();
    void StageFinish_EndAct2();

    RSDK_DECLARE(EHZSetup);
};
} // namespace GameLogic
