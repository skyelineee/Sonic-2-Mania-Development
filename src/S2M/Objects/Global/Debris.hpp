#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Debris : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Types {
        Spawner,
        Move,
        Fall,
        Idle,
    };

    enum OffsetModes {
        OffsetFixedPoint16,
        OffsetWhole,
        OffsetFixedPoint8,
    };

    enum VelocityModes {
        VelocityFixedPoint16,
        VelocityWhole,
        VelocityFixedPoint8,
    };

    enum FlickerModes {
        FlickerInactive,
        FlickerInvisible,
        FlickerNone,
        FlickerNormal,
        FlickerRand,
    };

    enum AnimationMOdes {
        AnimateNone,
        AnimateNormal,
        AnimateRandFrame,
    };

    // ==============================
    // STRUCTS
    // ==============================
    struct Info {
        int32 listID;
        int32 frame;
        int32 xOffset;
        int32 yOffset;
        int32 xVel;
        int32 yVel;
        int32 unknown;
    };


    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SpriteAnimation aniFrames2;
        Info info[20];
        RSDK::Vector2 velocities[38];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<Debris> state;
    RSDK::Animator animator1;
    RSDK::Animator animator2;
    RSDK::Animator animator3;
    Info *entries;
    uint16 entryCount;
    int32 spawnGravityStrength;
    uint16 spawnOffsetMode;
    uint16 spawnVelocityMode;
    int32 spawnAnimationMode;
    int32 spawnFlickerMode;
    int32 field_F4;
    int16 initDelay;
    int32 gravityStrength;
    bool32 flicker;
    bool32 animate;
    uint8 rotSpeed;
    int32 targetRotation;
    int32 field_110;
    int32 duration;
    int32 hiddenDuration;
    uint8 type;
    RSDK::Vector2 startPos;
    bool32 harmful;
    int32 field_12C;
    int32 field_130;
    int32 field_134;
    bool32 screenRelative;
    RSDK::StateMachine<Debris> stateDraw;

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

    void StateDraw_Default();

    void State_Init();
    void State_Move();
    void State_Fall();

    void CheckPlayerCollisions();
    static Debris *CreateFromEntries(Debris::Info *entries, uint16 entryCount, RSDK::Vector2 pos, int32 gravityStrength, uint16 offsetMode,
                                      uint16 velocityMode, int32 animationMode, int32 flickerMode);
    void State_Destroy();
    void VelocityFromID(uint8 id);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Debris);
};
} // namespace GameLogic