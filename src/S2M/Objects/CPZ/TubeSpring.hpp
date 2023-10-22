#pragma once
#include "S2M.hpp"
#include "Global/Spring.hpp"

namespace GameLogic
{

struct TubeSpring : RSDK::GameObject::Entity {

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
        RSDK::SoundFX sfxSpring;
        RSDK::SoundFX sfxExit;
        RSDK::Hitbox hitbox;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    // "Inherits" Spring
    RSDK::StateMachine<TubeSpring> state;
    Spring::Types type;
    RSDK::FlipFlags flipFlag;
    uint8 planeFilter;
    int32 timer;
    RSDK::Animator animator;
    RSDK::Hitbox hitbox;
    bool32 passThrough;
    bool32 twirl;
    uint8 swap;
    int32 sfxTimer;

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

    void Spring_State_Vertical();

    void State_Idle();
    void State_Springing();
    void State_Pullback();

    bool32 HandleInteractions(bool32 setState);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TubeSpring);
};
} // namespace GameLogic