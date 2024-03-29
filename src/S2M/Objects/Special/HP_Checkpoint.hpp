#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct HP_Checkpoint : RSDK::GameObject::Entity {

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
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxStarPost;
        RSDK::SoundFX sfxFail;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<HP_Checkpoint> state;
    RSDK::StateMachine<HP_Checkpoint> stateDraw;
    RSDK::Animator iconAnimator;
    RSDK::Animator emblemAnimator;
    RSDK::Animator ringAnimator;
    int32 ringCountSonic;
    int32 ringCountKnux;
    int32 ringCount2P;
    int32 timer;
    bool32 thumbPos;
    bool32 failed;
    Vector3 localPos;
    Vector3 localShadowPos;
    Vector3 worldPos;
    Vector3 worldShadowPos;

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
    void State_Init();
    void State_AwaitPlayer();
    void State_ShowResults();
    void State_ExitMessage();
    void State_ShowNewRingTarget();
    void State_FadeOut();
    void State_GoToResults();

    // Draw Stuff
    void DrawRings();
    void Draw_Results();
    void Draw_Fade();
    void Draw_Exit();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(HP_Checkpoint);
};
} // namespace GameLogic