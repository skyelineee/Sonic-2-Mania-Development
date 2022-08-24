#pragma once
#include "S2M.hpp"

#include "Common/Platform.hpp"

namespace GameLogic
{

struct Animals : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Types {
        Flicky,
        Ricky,
        Pocky,
        Pecky,
        Picky,
        Cucky,
        Rocky,
        Becky,
        Locky,
        Tocky,
        Wocky,
        Micky,
        SuperFlicky, // Not a proper type per-se, but it appears here on the anim list so
    };

    enum Behaviours {
        BehaveFree,
        BehaveFollow,
        BehaveFixed,
        BehaveIntro,
        BehaveFlyFollow,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        int32 animalTypes[2];
        bool32 hasPlatform;
        bool32 hasBridge;
        int32 hitboxes[12];
        int32 gravityStrength[12];
        int32 yVelocity[12];
        int32 xVelocity[12];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<Animals> state;
    int32 timer;
    int32 type;
    int32 behaviour;
    RSDK::Animator animator;
    RSDK::Hitbox hitboxAnimal;
    RSDK::Vector2 startPos;

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
    void CheckDirection();
    bool32 CheckPlatformCollision(Platform *platform);
    bool32 CheckGroundCollision();

    void State_Fall();
    void State_Fly();
    void State_Bounce();
    void State_Placed();
    void State_IntroFly();
    void State_IntroBounce();
    void State_FlyAround();
    void State_WaitBounce();
    void State_FlyAway();
    void State_MissionPlaced();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Animals);
};
} // namespace GameLogic