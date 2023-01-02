#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIHeading : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum UIHeadingIDs {
        UIHEADING_MAINMENU,
        UIHEADING_SAVESELECT,
        UIHEADING_TIMEATTACK,
        UIHEADING_RESULTS,
        UIHEADING_OPTIONS,
        UIHEADING_EXTRAS,
        UIHEADING_PLAYERSEL,
        UIHEADING_ZONESEL,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation textFrames;
    };

    RSDK::StateMachine<UIHeading> state;
    int32 unused;
    RSDK::Vector2 startPos;
    int32 headingID;
    RSDK::Animator headingAnimator;
    RSDK::Animator textAnimator;
    RSDK::SpriteAnimation textFrames;

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

    static void LoadSprites();

    // ==============================
    // STATES
    // ==============================

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIHeading);
};
} // namespace GameLogic