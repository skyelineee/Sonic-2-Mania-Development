#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct HPZSetup : RSDK::GameObject::Entity
{
    enum TileFlagsHPZ { HPZ_TFLAGS_NORMAL, HPZ_TFLAGS_SLIDE };

    struct Static : RSDK::GameObject::Static {
        int32 paletteTimer;
        int32 waterslidingPlayerCount;
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

    static void GetTileInfo(int32 x, int32 y, int32 moveOffsetX, int32 moveOffsetY, int32 cPlane, int32 *tile, uint8 *flags);

    RSDK_DECLARE(HPZSetup);
};
} // namespace GameLogic
#pragma once
#pragma once
