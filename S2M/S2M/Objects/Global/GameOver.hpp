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
    RSDK::Vector2 barPos;
    RSDK::Vector2 verts[4];
    RSDK::Vector2 letterPositions[8];
    RSDK::Vector2 finalOffsets[8];
    RSDK::Vector2 letterPosMove[8];
    int32 letterBounceCount[8];
    int32 letterRotations[8];
    int32 letterRotateSpeed[8];
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

    void State_MoveIn_Mania();
    void State_WaitComp_Mania();
    void State_Wait_Mania();
    void State_MoveOut_Mania();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(GameOver);
};
} // namespace GameLogic