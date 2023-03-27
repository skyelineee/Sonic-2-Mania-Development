#pragma once
#include "S2M.hpp"
#include "UISaveSlot.hpp"

namespace GameLogic
{

struct SaveMenuIconBG : RSDK::GameObject::Entity {

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
    };

    RSDK::Animator staticAnimator;
    RSDK::Animator zoneIconAnimator;
    RSDK::Animator characterIconAnimator;
    int32 lastZoneIcon;
    int32 timer;

    // ==============================
    // INSTANCE VARS
    // ==============================

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

    RSDK_DECLARE(SaveMenuIconBG);
};
} // namespace GameLogic