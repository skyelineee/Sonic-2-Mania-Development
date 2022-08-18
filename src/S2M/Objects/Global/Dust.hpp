#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Dust : RSDK::GameObject::Entity {

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
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<Dust> state;
    int32 timer;
    RSDK::GameObject::Entity *parent;
    RSDK::Animator animator;

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

    void State_SpinDash();
    void State_DustTrail();
    void State_DustPuff();
    void State_DustPuff_Collide();
    void State_GlideTrail();
    void State_Static();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Dust);
};
} // namespace GameLogic