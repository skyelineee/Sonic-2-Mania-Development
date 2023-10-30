#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Splats : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum SplatsTypes {
        SPLATS_BOUNCEAROUND,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxBadnik;
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Splats> state;
    uint8 bounceCount;
    uint8 activeCount;
    uint8 numActive;
    uint16 minDelay;
    int32 delay;
    bool32 isOnScreen;
    Splats *parent;
    RSDK::Vector2 startPos;
    int32 startDir;
    RSDK::Animator mainAnimator;
    RSDK::Animator splashAnimator;

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

    void DebugDraw();
    void DebugSpawn();

    void CheckPlayerCollisions();
    void CheckOffScreen();

    void State_Init();
    void State_BounceAround();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Splats);
};
} // namespace GameLogic