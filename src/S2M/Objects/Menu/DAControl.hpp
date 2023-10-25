#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct DAControl : RSDK::GameObject::Entity {

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

    RSDK::StateMachine<DAControl> state;
    int32 unused1;
    int32 unused2;
    int32 unused3;
    RSDK::String text;
    RSDK::Animator backPlateAnimator;
    RSDK::Animator buttonAnimator;
    RSDK::Animator optionsAnimator[5];
    RSDK::Animator textAnimator;

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

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(DAControl);
};
} // namespace GameLogic
