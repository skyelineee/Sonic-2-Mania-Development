#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Crate : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum CrateFrameIDs {
        CRATE_FRAME_BLUE,
        CRATE_FRAME_ORANGE_BROKEN,
        CRATE_FRAME_ORANGE,
        CRATE_FRAME_BLUE_BG,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxExplosion2;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Crate> state;
    RSDK::StateMachine<Crate> stateCollide;
    int32 type;
    RSDK::Vector2 amplitude;
    int32 speed;
    bool32 hasTension;
    int8 frameID;
    uint8 collision;
    RSDK::Vector2 tileOrigin;
    RSDK::Vector2 centerPos;
    RSDK::Vector2 drawPos;
    RSDK::Vector2 collisionOffset;
    int32 stood;
    int32 timer;
    int32 stoodAngle;
    uint8 stoodPlayers;
    uint8 pushPlayersL;
    uint8 pushPlayersR;
    RSDK::Hitbox hitbox;
    RSDK::Animator animator;
    int32 childCount;
    bool32 ignoreItemBox;

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

    static void Break(Crate *entity);
    static void MoveY(Crate *self, int32 offset);
    bool32 Collide();

    void State_None();
    void State_ApplyGravity();
    void State_WaitToFall();
    void State_Fall();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Crate);
};
} // namespace GameLogic
#pragma once
