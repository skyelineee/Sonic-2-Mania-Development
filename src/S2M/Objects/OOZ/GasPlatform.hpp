#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct GasPlatform : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum GasPlatformTypes {
        GASPLATFORM_INTERVAL,
        GASPLATFORM_CONTACT,
        GASPLATFORM_BOSS,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Vector2 stoodPos[4];
        RSDK::Hitbox hitboxGas;
        RSDK::Vector2 range;
        RSDK::SoundFX sfxGasPop;
        RSDK::SoundFX sfxSpring;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    uint16 interval;
    uint16 intervalOffset;
    RSDK::Animator gasAnimator;
    // mania platform base
    RSDK::StateMachine<GasPlatform> state;
    RSDK::StateMachine<GasPlatform> stateCollide;
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

    void PopPlatform();
    void State_Popped();
    void State_SpringCooldown();
    void State_Shaking();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(GasPlatform);
};
} // namespace GameLogic