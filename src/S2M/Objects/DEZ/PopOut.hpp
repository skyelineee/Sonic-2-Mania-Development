#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct PopOut : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum PopOutChildTypes {
        POPOUT_CHILD_SPRING_YELLOW,
        POPOUT_CHILD_SPRING_RED,
        POPOUT_CHILD_SPIKES,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        bool32 hasButton;
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    uint8 orientation;
    uint8 tag;
    bool32 manualTrigger;
    int32 delay;
    RSDK::Vector2 startPos;
    int32 unused;
    int32 childType;
    bool32 shouldAppear;
    int32 appearTimer;
    int32 timer;
    RSDK::Vector2 appearVelocity;
    RSDK::Hitbox hitboxSolid;
    RSDK::Hitbox hitboxRange;
    RSDK::Animator mountAnimator;
    RSDK::Animator childAnimator;

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

    void SetupHitboxes();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(PopOut);
};
} // namespace GameLogic
