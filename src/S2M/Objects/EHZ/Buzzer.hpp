#pragma once
#include "S2M.hpp"

namespace GameLogic
{

enum Anims { Idle, Flying, Shooting, Invisible };

struct Buzzer : RSDK::GameObject::Entity {

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxBadnik;
        RSDK::Hitbox hitboxProjectile;
        RSDK::SpriteAnimation aniFrames;
    };

    RSDK::StateMachine<Buzzer> state;
    uint8 shotRange;
    int32 timer;
    int32 hasShot;
    RSDK::Vector2 startPos;
    uint8 startDir;
    RSDK::GameObject::Entity *projectile;
    RSDK::Animator animator;
    RSDK::Hitbox hitboxRange;
    int32 shootTimer;
    int32 temp0;

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

    void CheckPlayerCollisions();

    // STATES //

    void State_Init();
    void State_Flying();
    void State_Idle();
    void State_Shooting();
    void State_Invisible();

    // PROJECTILE STATES //
    void State_ProjectileCharge();
    void State_ProjectileShot();

    RSDK_DECLARE(Buzzer);
};
} // namespace GameLogic
