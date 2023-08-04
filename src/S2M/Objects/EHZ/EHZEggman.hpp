#pragma once
#include "S2M.hpp"
#include "EggDriller.hpp"

namespace GameLogic
{

struct EHZEggman : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum EHZEggmanAnimations {
        Invisible,
        Idle,
        Laugh,
        Hit,
        Defeated,
        Toasted,
        Panic,
    };

    enum EHZHeliAnimations {
        Active = 7,
        Stop = 8,
        Retracting = 9,
        None = 10,
        Extending = 11,
        Exit = 12,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX helicopterSFX;
        RSDK::SoundFX bossHitSFX;
        RSDK::Hitbox hitbox;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 timer;
    int32 heliSFXTimer;
    int32 invincibilityTimer;
    int32 health;
    bool32 exploding;
    bool32 didHitPlayer;
    RSDK::StateMachine<EHZEggman> state;
    RSDK::Animator helicopterAnimator;
    RSDK::Animator eggmanAnimator;
    RSDK::Animator mobileAnimator;
    //EggDriller *backWheel;   // object[-1]
    //EggDriller *car;         // object[+1]
    //EggDriller *drill;       // object[+2]
    //EggDriller *frontWheel;  // object[+3]
    //EggDriller *frontWheel2; // object[+4]

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

    void PlayHeliSFX();

    void State_AwaitPlayer();
    void State_FlyIn();
    void State_EnterCar();
    void State_StartCar();
    void State_InCar();
    void State_Explode();
    void State_ExitCar();
    void State_Flee();
    void State_Escape();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(EHZEggman);
};
} // namespace GameLogic