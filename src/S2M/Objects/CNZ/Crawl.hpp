#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Player;

struct Crawl : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum CrawlAnims {
        Idle,
        Move,
        Defend,
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

    int32 timer;
    RSDK::StateMachine<Crawl> state;
    RSDK::StateMachine<Crawl> prevState;
    RSDK::Animator animator;
    RSDK::Vector2 startPos;
    bool32 foundTarget;

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

    // STATES
    void State_Setup();
    void State_Moving();
    void State_Idle();
    void State_Defend();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Crawl);
};
} // namespace GameLogic
