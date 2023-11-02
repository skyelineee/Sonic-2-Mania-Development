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
        bool32 holiday;
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 basis;
    int32 addend;
    bool32 spawned;
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
    void CreateSnowflake(int32 id, bool32 top = true);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Snowflakes);
};
} // namespace GameLogic
