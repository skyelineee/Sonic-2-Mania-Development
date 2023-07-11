#pragma once

#include "S2M.hpp"

namespace GameLogic
{

// Object Class
struct Sega : RSDK::GameObject::Entity {

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxSega;
        RSDK::SoundFX sfxRing;
        RSDK::SoundFX sfxRelease;
    };

    int32 timer;
    RSDK::StateMachine<Sega> state;
    RSDK::Vector2 sonicPos;
    RSDK::Animator segaAnimator;
    RSDK::Animator sonicAnimator;
    RSDK::Animator trailAnimator;
    RSDK::Animator textAnimator;

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Update();
    void LateUpdate();
    void Draw();
    static void StaticUpdate();
    static void StageLoad();
    static void Serialize();
#if RETRO_INCLUDE_EDITOR
    void EditorDraw();
    static void EditorLoad();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    //States
    void State_SonicRunIn();
    void State_SonicRunOut();
    void State_OriginalGameBy();
    void State_PresentsText();

    RSDK_DECLARE(Sega);
};
} // namespace GameLogic