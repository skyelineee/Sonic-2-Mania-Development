#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct ARZSetup : RSDK::GameObject::Entity {

    struct Static : RSDK::GameObject::Static {
        int32 paletteTimer;
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
    void EditorDraw();
    static void EditorLoad();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    RSDK_DECLARE(ARZSetup);
};
} // namespace GameLogic
#pragma once
