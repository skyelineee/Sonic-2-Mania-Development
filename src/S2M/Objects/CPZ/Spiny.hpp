#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Spiny : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum SpinyTypes {
        SPINY_FLOOR,
        SPINY_WALL,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxSpiny;
        RSDK::Hitbox hitboxRange;
        RSDK::Hitbox hitboxShot;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxShot;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Spiny> state;
    RSDK::Animator animator;
    int32 unused;
    uint8 type;
    RSDK::Vector2 startPos;
    uint8 startDir;
    int32 shotVelocity;
    int32 dirChangeTimer;
    int32 moveTimer;

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

    void DebugSpawn();
    void DebugDraw();

    void CheckPlayerCollisions();
    void CheckOffScreen();

    void State_Init();
    void State_Floor();
    void State_Shoot_Floor();
    void State_Wall();
    void State_Shoot_Wall();

    void State_Shot();
    void State_ShotDisappear();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Spiny);
};
} // namespace GameLogic