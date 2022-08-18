#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct PlaneSwitch : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum OnPathModes {
        Either,
        GroundOnly,
        AirOnly,
    };

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
    int32 flags;
    int32 size;
    uint8 onPath;
    int32 negAngle;
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
    void DrawSprites();
    void CheckCollisions(RSDK::GameObject::Entity *other, int32 flags, int32 size, bool32 switchDrawOrder, uint8 low, uint8 high);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(PlaneSwitch);
};
} // namespace GameLogic