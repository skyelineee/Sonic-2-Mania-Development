#pragma once

#include "S2M.hpp"
#include "S2M.hpp"

namespace GameLogic
{

// Object Class
struct UIPicture : RSDK::GameObject::Entity {

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
    };

    int32 listID;
    int32 frameID;
    int32 tag;
    uint8 zonePalette;
    int32 zoneID;
    RSDK::Animator animator;

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Update();
    void LateUpdate();
    void Draw();
    static void StaticUpdate();
    static void StageLoad();
    static void Serialize();
#if RETRO_INCLUDE_EDITOR
    void EditorDraw();
    static void EditorLoad();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    RSDK_DECLARE(UIPicture);
};
} // namespace GameLogic