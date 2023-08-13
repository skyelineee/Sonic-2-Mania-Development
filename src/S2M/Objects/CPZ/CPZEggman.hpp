#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct CPZEggman : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum CPZEggmanAnimations {
        Invisible,
        Idle,
        Laugh,
        Hit,
        Toasted,
    };

    enum CPZFlameAnimations {
        Inactive = 11,
        Active = 12,
        Explode = 13,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX bossHitSFX;
        RSDK::Hitbox hitbox;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::Vector2 originPos;
    int32 timer;
    int32 health;
    int32 invincibilityTimer;
    bool32 exploding;
    RSDK::StateMachine<CPZEggman> state;
    RSDK::Animator seatAnimator;
    RSDK::Animator eggmanAnimator;
    RSDK::Animator mobileAnimator;
    RSDK::Animator flameAnimator;

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

    void Oscillate();

    // STATES

    void State_AwaitPlayer();
    void State_BossFight();
    void State_Explode();
    void State_DefeatFall();
    void State_DefeatRise();
    void State_Flee();
    void State_Escape();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(CPZEggman);
};
} // namespace GameLogic