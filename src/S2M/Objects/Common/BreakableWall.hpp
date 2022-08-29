#pragma once
#include "Game.hpp"

#include "Global/Player.hpp"

namespace GameLogic
{

struct BreakableWall : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum Types {
        Wall,
        Floor,
        BurrowFloor,
        BurrowFloor2,
        BurrowFloorUp,
        Ceiling,
    };

    enum TileTypes {
        TileNone,
        TileFixed,
        TileDynamic,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Animator animator;
        RSDK::SpriteAnimation aniFrames;
        int32 unused1[16];
        int32 unused2[16];
        int32 breakOffsets[16];
        int32 breakVelocitiesL[8];
        int32 breakVelocitiesR[8];
        RSDK::SoundFX sfxLedgeBreak;
        RSDK::SceneLayer farPlaneLayer;
        RSDK::SceneLayer scratchLayer;
        int32 disableScoreBonus;
        int32 field_138;
        int32 breakMode;
        int32 hasSetupConfig;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<BreakableWall> state;
    RSDK::StateMachine<BreakableWall> stateDraw;
    uint8 type;
    bool32 onlyKnux;
    RSDK::Vector2 size;
    RSDK::Tile tileInfo;
    RSDK::SceneLayer targetLayer;
    int32 timer;
    RSDK::Vector2 tilePos;
    int32 tileRotation;
    int32 gravityStrength;
    RSDK::Hitbox hitbox;
    RSDK::Vector2 startPos;
    bool32 useLayerDrawGroup;
    uint8 targetDrawGroup;
    bool32 reverseX;
    bool32 reverseY;
    uint8 topOffset;

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

    // States
    void State_PieceWait();
    void State_Piece();
    void State_Wall();
    void State_Floor();
    void State_BurrowFloor();
    void State_BurrowFloorUp();
    void State_Ceiling();

    // Draw States
    void State_DrawWall();
    void State_DrawFloor();
    void State_DrawPiece();

    // Breaking
    void CheckBreak_Wall();
    void CheckBreak_Floor();
    void CheckBreak_BurrowFloor();
    void CheckBreak_BurrowFloorUp();
    void CheckBreak_Ceiling();

    void Break(uint8 direction);

    // Misc
    void GiveScoreBonus(Player *player);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(BreakableWall);
};
} // namespace GameLogic