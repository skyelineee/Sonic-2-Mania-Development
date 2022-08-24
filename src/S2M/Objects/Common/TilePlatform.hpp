#pragma once
#include "Game.hpp"

#include "Platform.hpp"

namespace GameLogic
{

struct TilePlatform : RSDK::GameObject::Entity {

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
        int32 unused;
        RSDK::Animator animator;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SceneLayer moveLayer;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<TilePlatform> state;
    RSDK::StateMachine<TilePlatform> stateCollide;
    int32 type;
    RSDK::Vector2 amplitude;
    int32 speed;
    bool32 hasTension;
    int8 frameID;
    uint8 collision;
    RSDK::Vector2 targetPos;
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

    RSDK::Vector2 size;
    RSDK::Tile tiles[0x100];

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
    static void StageLoad();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif
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

    RSDK_DECLARE(TilePlatform);
};
} // namespace GameLogic