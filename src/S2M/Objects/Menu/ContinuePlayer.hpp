#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct ContinuePlayer : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum ContinuePlayerAniIDs {
        CONTPLR_ANI_IDLE_SONIC,
        CONTPLR_ANI_REACT_SONIC,
        CONTPLR_ANI_IDLE_TAILS,
        CONTPLR_ANI_REACT_TAILS,
        CONTPLR_ANI_IDLE_KNUX,
        CONTPLR_ANI_REACT_KNUX,
        CONTPLR_ANI_IDLE_AI,
        CONTPLR_ANI_ICON,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SpriteAnimation playerAniFrames;
        RSDK::SpriteAnimation tailAniFrames;
        RSDK::SoundFX sfxRoll;
        RSDK::SoundFX sfxCharge;
        RSDK::SoundFX sfxRelease;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<ContinuePlayer> state;
    bool32 isPlayer2;
    int32 timer;
    int32 unused1;
    int32 unused2;
    RSDK::Animator animator;
    RSDK::SpriteAnimation aniFrames;

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

    void HandleDashAnim();

    void State_Idle();
    void State_ChargeDash();
    void State_DashRelease();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ContinuePlayer);
};
} // namespace GameLogic