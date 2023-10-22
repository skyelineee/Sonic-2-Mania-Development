#pragma once
#include "S2M.hpp"
#include "Helpers/Soundboard.hpp"

namespace GameLogic
{

#define STAIRCASE_STAIR_COUNT (4)

struct Staircase : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum StaircaseTypes {
        STAIRCASE_DOWN,
        STAIRCASE_UP,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox blockHitbox;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxBlockStop;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Staircase> state;
    RSDK::StateMachine<Staircase> stateDraw;
    RSDK::Vector2 blockPos[STAIRCASE_STAIR_COUNT];
    int32 timer;
    RSDK::Animator animator;
    uint8 type;
    bool32 bumpable;

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

    static Soundboard::SoundInfo SfxCheck_HitBlocks();

    void State_Idle();
    void State_Wait();
    void State_MoveBlocks();
    void State_MovedBlocks();
         
    void Draw_Blocks();
    void Draw_Shake();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Staircase);
};
} // namespace GameLogic