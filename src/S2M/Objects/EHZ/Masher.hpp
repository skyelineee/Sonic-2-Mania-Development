#pragma once
#include "S2M.hpp"

namespace GameLogic
{
struct Masher : RSDK::GameObject::Entity {

    enum Anims { Idle, Biting };

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxBadnik;
        RSDK::SpriteAnimation aniFrames;
    };

    RSDK::StateMachine<Masher> state;
    RSDK::Animator animator;
    RSDK::Vector2 startPos;

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

    // States //

    void State_Jumping();

    RSDK_DECLARE(Masher);
};
} // namespace GameLogic