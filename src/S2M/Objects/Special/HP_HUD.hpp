#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct HP_HUD : RSDK::GameObject::Entity {

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
        bool32 setupDebugFlags;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::Animator hudElementsAnimator;
    RSDK::Animator numbersAnimator;
    RSDK::Animator tagAnimator;

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

    void DrawRingCount(uint16 playerSlot, RSDK::Vector2 *drawPos);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(HP_HUD);
};
} // namespace GameLogic