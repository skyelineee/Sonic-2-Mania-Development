#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct CPZSetup : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum DecorationTypesCPZ { CPZ_DECORATION_WARNSIGN};

    enum ParallaxSpriteAniIDsCPZ {
        CPZ_PARALLAXSPRITE_GIRDER,
        CPZ_PARALLAXSPRITE_CHEMICAL1,
        CPZ_PARALLAXSPRITE_CHEMICAL2,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        uint16 aniTiles;
        RSDK::TileLayer *background;
        int32 deformation[64];
        int32 bgTowerLightPalIndex;
        int32 rainbowLightPalIndex;
        int32 chemLiquidPalIndex1;
        int32 chemLiquidPalIndex2;
        int32 chemLiquidPalIndex3;
        uint16 aniTileFrame;
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

    void StageFinish_EndAct1(void);
    void StageFinish_EndAct2(void);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(CPZSetup);
};
} // namespace GameLogic
