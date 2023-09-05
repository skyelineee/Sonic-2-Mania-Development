#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Pendulum : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitbox;
        RSDK::SoundFX sfxImpact;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 spikeBallPosFG;
    int32 spikeBallFrameFG;
    int32 spikeBallPosBG;
    int32 spikeBallFrameBG;
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

    void DebugDraw();
    void DebugSpawn();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Pendulum);
};
} // namespace GameLogic
