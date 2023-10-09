#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SeltzerBottle : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxBottle;
        RSDK::Hitbox hitboxButton;
        RSDK::SceneLayer seltzerPathLayer;
        uint16 seltzerPathLayerMask;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SpriteAnimation nullFrames;
        RSDK::SoundFX sfxSpray;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<SeltzerBottle> state;
    int32 sprayTime;
    int32 timer;
    int32 waterLevel;
    RSDK::Animator unusedAnimator;
    RSDK::Animator bottleAnimator;
    RSDK::Animator buttonAnimator;
    RSDK::Animator waterAnimator;
    RSDK::Animator sprayAnimator;

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
    
    void State_Spraying();
    void State_TryReset();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SeltzerBottle);
};
} // namespace GameLogic
