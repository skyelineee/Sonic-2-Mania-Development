#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIButtonLabel : RSDK::GameObject::Entity {

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

    uint8 type;
    int32 listID;
    int32 frameID;
    RSDK::Animator frameAnimator;
    RSDK::Animator textAnimator;
    RSDK::SpriteAnimation textSprite;

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
    
    RSDK_DECLARE(UIButtonLabel);
};
} // namespace GameLogic