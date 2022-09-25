#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct HP_Collectable : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum Types {
        Ring,
        Bomb,
        Emerald,
        RingSparkle,
        Explosion,
        EditorIcons,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxRing;
        RSDK::SoundFX sfxBomb;
        RSDK::SoundFX sfxEmerald;
        int32 pan;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<HP_Collectable> state;
    RSDK::StateMachine<HP_Collectable> stateDraw;
    RSDK::Animator animator;
    RSDK::Animator shadowAnimator;
    uint8 type;
    int32 timer;
    Vector3 localPos;
    Vector3 localShadowPos;
    Vector3 worldPos;
    Vector3 worldShadowPos;
    bool32 shadowsEnabled;
    int32 gravityStrength;
    bool32 notEnoughRings;
    RSDK::Vector2 emeraldPos;

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

    // States
    void State_Ring();
    void State_Bomb();
    void State_EmeraldTrigger();
    void State_EmeraldTryCollect();
    void State_EmeraldHandleFinish();
    void State_Sparkle();
    void State_Lost();
    void State_Explosion();

    // Drawing
    void Draw_Ring();
    void Draw_Bomb();
    void Draw_Emerald();
    void Draw_LostRing();

    // Misc
    static void LoseRings(RSDK::Vector2 *position, int32 lossAngle, int32 rings, uint8 drawGroup);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(HP_Collectable);
};
} // namespace GameLogic