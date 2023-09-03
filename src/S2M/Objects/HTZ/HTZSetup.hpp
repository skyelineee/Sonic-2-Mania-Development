#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct HTZSetup : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum TileFlagsHTZ { HTZ_TFLAGS_NORMAL, HTZ_TFLAGS_LAVA };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 fadeTimer;
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

    static void GetTileInfo(int32 x, int32 y, int32 moveOffsetX, int32 moveOffsetY, int32 cPlane, int32 *tile, uint8 *flags);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(HTZSetup);
};
} // namespace GameLogic
#pragma once
