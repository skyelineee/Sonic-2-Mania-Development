#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct PushSpring : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum PushSpringTypes { PUSHSPRING_V, PUSHSPRING_H };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxPush;
        RSDK::SoundFX sfxSpring;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<PushSpring> state;
    RSDK::StateMachine<PushSpring> stateDraw;
    RSDK::StateMachine<PushSpring> stateCollide;
    uint8 type;
    int32 pushOffset;
    int32 timer;
    RSDK::Hitbox hitbox;
    bool32 beingPushed;
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
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    void Collide_Top();
    void Collide_Bottom();
    void Collide_Left();
    void Collide_Right();

    void Draw_Top();
    void Draw_Bottom();
    void Draw_Left();
    void Draw_Right();

    void State_WaitForPushed();
    void State_BeingPushed();
    void State_PushRecoil();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(PushSpring);
};
} // namespace GameLogic