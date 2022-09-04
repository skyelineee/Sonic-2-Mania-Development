#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SignPost : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Types {
        RunPast,
        Drop,
        Competition,
        Decoration,
    };

    enum AnimIDs {
        AniSonic,
        AniTails,
        AniKnux,
        AniEggman,
        AniPost,
        AniSpin,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::Hitbox hitboxSignPost;
        RSDK::Hitbox hitboxItemBox;
        RSDK::SoundFX sfxSignPost;
        RSDK::SoundFX sfxSignPost2P;
        RSDK::SoundFX sfxTwinkle;
        RSDK::SoundFX sfxBubbleBounce;
        RSDK::SoundFX sfxSlide;
        uint8 maxPlayerCount;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<SignPost> state;
    uint8 type;
    RSDK::Vector2 vsBoundsSize;
    RSDK::Vector2 vsBoundsOffset;
    int32 vsExtendTop;
    int32 vsExtendBottom;
    int32 timer;
    int32 gravityStrength;
    int32 spinCount;
    int32 spinSpeed;
    int32 maxAngle;
    int32 sparkleType;
    int32 itemBounceCount;
    RSDK::Vector2 playerPosStore[PLAYER_COUNT];
    RSDK::Animator eggPlateAnimator;
    RSDK::Animator facePlateAnimator;
    RSDK::Animator sidebarAnimator;
    RSDK::Animator postTopAnimator;
    RSDK::Animator standAnimator;
    uint8 activePlayers;
    bool32 spawnedByDebugMode;

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

    void State_Done();
    void State_InitDelay();
    static void Spawn(int32 x);
    void State_Fall_Classic();
    void HandleSpin();
    void HandleSparkles();
    void State_Sparkle_Classic();
    void HandleCamBounds();
    void CheckTouch();
    void State_SpinVS();
    void State_Spin();
    void State_FlyUp();
    void State_Wait();
    void State_Falling();
    void State_FakeFalling();
    void State_Landed_Classic();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SignPost);
};
} // namespace GameLogic