#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct EggPrison : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Types {
        OnGround,
        InAir,
        Animals,
        Rings,
        Enemies,
        Spring,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxDestroy;
        RSDK::SoundFX sfxSpring;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<EggPrison> state;
    int32 type;
    int32 timer;
    int32 originY;
    int32 buttonPos;
    int32 notSolid;
    int32 checkTileCollisions;
    RSDK::Hitbox hitboxSolid;
    RSDK::Hitbox hitboxButton;
    RSDK::Hitbox hitboxButtonTrigger;
    RSDK::Animator capsuleAnimator;
    RSDK::Animator propellerAnimator;
    RSDK::Animator panelAnimator;
    RSDK::Animator buttonAnimator;
    bool32 dontSetBounds;
    bool32 enableFlyAway;

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

    void DebugSpawn();
    void DebugDraw();

    void HoverMovement();

    void State_Init();
    void State_Idle();
    void State_OpenPt1();
    void State_OpenPt2();
    void State_Wait();
    void State_FlyAway();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(EggPrison);
};
} // namespace GameLogic