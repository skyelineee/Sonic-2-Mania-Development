#pragma once
#include "S2M.hpp"

#include "Player.hpp"
#include "Common/Platform.hpp"

namespace GameLogic
{

struct Ring : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Types {
        Normal,
        Combi,
        Sparkle1,
        Sparkle2,
        Sparkle3,
    };

    enum MoveTypes {
        Fixed,
        Linear,
        Circle,
        Track,
        Path,
        Unknown_5,
        Unknown_6
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitbox;
        int32 pan;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxRing;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Ring> state;
    RSDK::StateMachine<Ring> stateDraw;
    Ring::Types type;
    int32 planeFilter;
    int32 ringAmount;
    int32 timer;
    int32 maxFrameCount;
    int32 sparkleType;
    Player *storedPlayer;
    Ring::MoveTypes moveType;
    RSDK::Vector2 amplitude;
    int32 speed;
    RSDK::Vector2 drawPos;
    RSDK::Animator animator;
    int32 respawnTime;
    RSDK::Vector2 startPos;

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

    void DebugSpawn();
    void DebugDraw();

    // ==============================
    // FUNCTIONS
    // ==============================

    void Collect();

    static void LoseRings(RSDK::Vector2 *position, int32 rings, uint8 cPlane, uint8 drawGroup);
    static void LoseHyperRings(RSDK::Vector2 *position, int32 rings, uint8 cPlane);
    static void FakeLoseRings(RSDK::Vector2 *position, int32 ringCount, uint8 drawGroup);

    int32 CheckPlatformCollisions(Platform *platform);
    void CheckObjectCollisions(int32 x, int32 y);

    void State_Normal();
    void State_Linear();
    void State_Circular();
    void State_Path();
    void State_Track();
    void State_Lost();
    void State_LostFX();
    void State_Combi();
    void State_Attracted();
    void State_Sparkle();

    void Draw_Normal();
    void Draw_Oscillating();
    void Draw_Sparkle();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Ring);
};
} // namespace GameLogic