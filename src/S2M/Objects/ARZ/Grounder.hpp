#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Player;

struct Grounder : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum GrounderAnims {
        Moving,
        Turning,
        Appear,
        Brick,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::Hitbox hitboxBadnik;
        RSDK::Hitbox hiddenHitbox;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::Vector2 startPos;
    RSDK::Animator animator;
    RSDK::Animator brickAnimator;
    RSDK::StateMachine<Grounder> state;
    int32 timer;
    int32 targetPlayerDistance;
    int32 type;
    int32 brickFrame;
    bool32 isBrick;
    Player *targetPlayer;

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

    void DebugSpawn();
    void DebugDraw();

    // STATES
    void State_Hidden();
    void State_Appear();
    void State_Moving();
    void State_Turning();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Grounder);
};
} // namespace GameLogic