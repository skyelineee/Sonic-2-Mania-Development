#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct PetalPile : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum PetalPileLayerIDs {
        PETALPILE_FGHIGH,
        PETALPILE_FGLOW,
    };

    enum PetalPilePatterns {
        PETALPILE_PATTERN_0,
        PETALPILE_PATTERN_1,
        PETALPILE_PATTERN_2,
        PETALPILE_PATTERN_3,
        PETALPILE_PATTERN_4,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 patternSize[5];
        int32 pattern1[14];
        int32 pattern2[12];
        int32 pattern3[12];
        int32 pattern4[20];
        int32 pattern5[6];
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxPetals;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<PetalPile> state;
    RSDK::StateMachine<PetalPile> stateDraw;
    uint16 timer;
    int8 leafPattern;
    int8 tileLayer;
    RSDK::Vector2 pileSize;
    RSDK::Vector2 maxSpeed;
    bool32 emitterMode;
    uint8 layerID;
    RSDK::Vector2 velStore;
    RSDK::Vector2 distance; // dunno if this is actually a vec2 but the .y is always set to 0 so maybe?
    int32 petalVel;
    int32 unused;
    int8 petalDir;
    int32 petalRadius;
    int32 petalOffset;
    bool32 noRemoveTiles;
    RSDK::Hitbox hitbox;
    RSDK::Animator animator;

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

    int32 GetLeafPattern(RSDK::Vector2 *patternPtr);

    void State_Init();
    void State_HandleInteractions();
    void State_SetupEmitter();
    void State_Emitter();

    void StateLeaf_Setup();
    void StateLeaf_Delay();
    void StateLeaf_HandleVelocity();
    void StateLeaf_Fall();

    void Draw_Leaf();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(PetalPile);
};
} // namespace GameLogic
