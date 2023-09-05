#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct WaterGush : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum WaterGushOrientations {
        WATERGUSH_UP,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxGush;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<WaterGush> state;
    int32 length;
    int32 speed;
    uint8 activePlayers;
    RSDK::Vector2 startPos;
    int32 unused;
    int32 gushPos;
    int32 gravityStrength;
    bool32 activated;
    bool32 finishedExtending;
    RSDK::Hitbox hitboxRange;
    RSDK::Hitbox hitboxGush;
    RSDK::Animator plumeAnimator;
    RSDK::Animator topAnimator;

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

    void SetupHitboxes();
    void DrawSprites();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(WaterGush);
};
} // namespace GameLogic
