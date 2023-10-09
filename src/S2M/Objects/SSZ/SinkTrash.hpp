#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SinkTrash : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum {
        SINKTRASH_SINK,
        SINKTRASH_SOLID,
        SINKTRASH_DECOR,
    } SinkTrashTypes;

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

    int32 type;
    RSDK::Vector2 size;
    RSDK::Vector2 trashPos[64];
    uint8 trashFrame[64];
    uint8 trashDir[64];
    RSDK::Hitbox hitboxTrash;
    RSDK::Hitbox hitboxDebris;
    RSDK::Animator trashAnimator;
    RSDK::Animator topAnimator;
    RSDK::Animator mainAnimator;

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

    RSDK_DECLARE(SinkTrash);
};
} // namespace GameLogic