#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Snowflakes : RSDK::GameObject::Entity {

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
        uint8 count;
        int32 unused;
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 unused1;
    int32 unused2;
    int32 unused3;
    RSDK::Animator animator;
    uint8 animIDs[0x40];
    uint16 frameIDs[0x40];
    RSDK::Vector2 positions[0x40];
    uint8 priority[0x40];
    uint16 angles[0x40];

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

    RSDK::Vector2 HandleWrap(int32 id);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Snowflakes);
};
} // namespace GameLogic
