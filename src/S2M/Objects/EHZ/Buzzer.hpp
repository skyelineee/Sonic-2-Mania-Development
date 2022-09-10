#pragma once
#include "S2M.hpp"

namespace GameLogic
{

enum Anims { Flying, Shooting, Invisible };

struct Buzzer : RSDK::GameObject::Entity {

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxBadnik;
        RSDK::Hitbox hitboxProjectile;
        RSDK::SpriteAnimation aniFrames;
        RSDK::Animator animator;
    };

    RSDK::StateMachine<Buzzer> state;
    uint8 shotRange;
    int32 timer;
    int32 hasShot;
    uint8 detectedPlayer;
    RSDK::Vector2 startPos;
    uint8 startDir;
    RSDK::GameObject::Entity *projectile;
    RSDK::Animator animator;
    RSDK::Animator thrustAnimator;
    RSDK::Hitbox hitboxRange;
    int32 shootTimer;

    // EVENTS //

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
    static void StageLoad();
    static void Serialize();
#if RETRO_INCLUDE_EDITOR
    void EditorDraw();
    static void EditorLoad();
#endif

    // FUNCTIONS //

    void DebugDraw();
    void DebugSpawn();

    void CheckOffScreen();
    void CheckPlayerCollisions();

    // STATES //

    void State_Init();
    void State_Flying();
    void State_Idle();
    void State_Shooting();

    // PROJECTILE STATES //
    void State_ProjectileCharge();
    void State_ProjectileShot();

    RSDK_DECLARE(Buzzer);
};
} // namespace GameLogic
