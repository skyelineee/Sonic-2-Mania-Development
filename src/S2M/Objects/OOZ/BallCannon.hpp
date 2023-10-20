#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct BallCannon : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum BallCannonTypes {
        BALLCANNON_CANNON,
        BALLCANNON_CORKV,
        BALLCANNON_CORKH,
    };

    enum BallCannonAngles {
        // Clockwise by default
        BALLCANNON_DIR_RIGHT_CW,
        BALLCANNON_DIR_DOWN_CW,
        BALLCANNON_DIR_LEFT_CW,
        BALLCANNON_DIR_UP_CW,
        BALLCANNON_DIR_DOWN_CCW,
        BALLCANNON_DIR_LEFT_CCW,
        BALLCANNON_DIR_UP_CCW,
        BALLCANNON_DIR_RIGHT_CCW,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxCannon;
        RSDK::Hitbox hitboxCorkBlock;
        RSDK::Hitbox hitboxCorkEntry;
        RSDK::SpriteAnimation aniFrames;
        int32 corkDebrisOffset[32];
        //{ -0xC0000, -0xC0000, -0x40000, -0xC0000, 0x40000,  -0xC0000, 0xC0000, -0xC0000, -0xC0000, -0x40000, -0x40000,
        //  -0x40000, 0x40000,  -0x40000, 0xC0000,  -0x40000, -0xC0000, 0x40000, -0x40000, 0x40000,  0x40000,  0x40000,
        //  0xC0000,  0x40000,  -0xC0000, 0xC0000,  -0x40000, 0xC0000,  0x40000, 0xC0000,  0xC0000,  0xC0000 });
        int32 corkDebrisVelocity[32];
        //{ -0x40000, -0x40000, -0x20000, -0x40000, 0x20000,  -0x40000, 0x40000, -0x40000, -0x3C000, -0x20000, -0x1C000,
        //  -0x20000, 0x1C000,  -0x20000, 0x3C000,  -0x20000, -0x38000, 0x20000, -0x18000, 0x20000,  0x18000,  0x20000,
        //  0x38000,  0x20000,  -0x34000, 0x40000,  -0x14000, 0x40000,  0x14000, 0x40000,  0x34000,  0x40000 });
        RSDK::SoundFX sfxLedgeBreak;
        RSDK::SoundFX sfxFire;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<BallCannon> state;
    uint8 type;
    bool32 exit;
    uint8 unused1;
    uint8 activePlayers;
    uint8 playerTimers[PLAYER_COUNT];
    int32 unused2;
    int32 rotationSpeed;
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

    void CheckPlayerEntry();
    void State_Idle();
    void State_Inserted();
    void State_Turning();
    void State_EjectPlayer();
    void State_CorkBlocked();
    void State_CorkOpened();
    void State_CorkDebris();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(BallCannon);
};
} // namespace GameLogic
