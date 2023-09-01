#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Plunger : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum DirectionTypes {
        Diagonal,
        Vertical,
    };

    enum ReleaseTypes {
        Lock,
        Release,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox diagonalHitbox;
        RSDK::Hitbox verticalHitbox;
        RSDK::Hitbox dynamicVertHitbox;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxPlunger;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 compression;
    int32 releaseType;
    int32 type;
    bool32 plungerActive;
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

    RSDK_DECLARE(Plunger);
};
} // namespace GameLogic
