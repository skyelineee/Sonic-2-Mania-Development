#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct WallBumper : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum WallBumperTypes {
        WALLBUMPER_H,
        WALLBUMPER_V,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxBouncer;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 type;
    int32 size;
    bool32 reverse;
    bool32 destructible;
    RSDK::Animator animator;
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
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    void DebugSpawn();
    void DebugDraw();

    void HandleInteractions();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(WallBumper);
};
} // namespace GameLogic
