#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct GameOver : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 activeScreens;
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<GameOver> state;
    int32 timer;
    int32 playerID;
    int32 targetPos;
    RSDK::Vector2 gameOverPos;
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

    void State_MoveIn();
    void State_WaitComp();
    void State_Wait();
    void State_MoveOut();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(GameOver);
};
} // namespace GameLogic