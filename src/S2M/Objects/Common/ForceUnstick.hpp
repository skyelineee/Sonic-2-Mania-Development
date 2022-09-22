#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct ForceUnstick : RSDK::GameObject::Entity {

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
        RSDK::Animator animator;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    uint8 width;
    uint8 height;
    bool32 breakClimb;
    RSDK::Hitbox hitbox;

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

    void DrawSprites();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ForceUnstick);
};
} // namespace GameLogic