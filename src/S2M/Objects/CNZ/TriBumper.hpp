#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Player;

struct TriBumper : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum TriBumperTypes {
        SMALL_L,
        SMALL_R,
        LARGE_U,
        LARGE_D,
        LARGE_L,
        LARGE_R,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox smallHitbox;
        RSDK::Hitbox largeUDHitbox;
        RSDK::Hitbox largeLRHitbox;
        RSDK::SoundFX sfxBumper;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    Player *player;
    int32 type;

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

    void SetPlayerVelocity(int32 pos, int32 heightVel);
    void HandleBump();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TriBumper);
};
} // namespace GameLogic
