#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct TippingPlatform : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum TippingPlatformBossIDs {
        TIPPINGPLATFORM_NONE,
        TIPPINGPLATFORM_PLAYER,
        TIPPINGPLATFORM_EGGMAN,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    //MANIA_PLATFORM_BASE;
    RSDK::StateMachine<TippingPlatform> state;
    RSDK::StateMachine<TippingPlatform> stateCollide;
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

    uint8 interval;
    uint8 intervalOffset;
    uint8 duration;
    uint8 bossID;

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

    void State_Tipping_Boss();
    void State_RestorePlatform();
    void State_Tipping();
    void State_Restore();
    void State_Tipping_Delay();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TippingPlatform);
};
} // namespace GameLogic
