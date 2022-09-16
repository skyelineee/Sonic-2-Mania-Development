#pragma once
#include "S2M.hpp"
#include "Global/Player.hpp"

namespace GameLogic
{
struct Coconuts : RSDK::GameObject::Entity {

    enum Anims { Idle, Moving, Throw, Coconut };

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxBadnik;
        RSDK::Hitbox hitboxCoconut;
        RSDK::SpriteAnimation aniFrames;
        int32 throwDelays[6];
        RSDK::SoundFX sfxDrop;
    };

    int32 timer;
    int32 throwID;
    RSDK::Vector2 startPos;
    uint8 startDir;
    Player* targetPlayer;
    int32 targetDelay;
    int32 targetDistance;
    int32 temp0;
    int32 temp1;
    RSDK::GameObject::Entity *coconut;
    RSDK::Animator animator;
    RSDK::StateMachine<Coconuts> state;

    // EVENTS //

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif
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
    void State_AwaitPlayer();
    void State_Moving();
    void State_Throwing();
    void State_HasThrown();
    void State_Coconut();

    RSDK_DECLARE(Coconuts);
};
} // namespace GameLogic