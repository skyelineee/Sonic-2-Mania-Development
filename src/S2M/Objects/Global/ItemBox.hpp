#pragma once
#include "S2M.hpp"

#include "Player.hpp"
#include "Common/Platform.hpp"

namespace GameLogic
{

struct ItemBox : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum Types {
        Ring,
        BlueShield,
        BubbleShield,
        FireShield,
        LightningShield,
        Invincible,
        Sneaker,
        ExtraLife_Sonic,
        ExtraLife_Tails,
        ExtraLife_Knux,
        Eggman,
        HyperRing,
        Swap,
        Random,
        Super,
        Coin,
        Stock,
        Banana,
        TypeCount,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxItemBox;
        RSDK::Hitbox hitboxHidden;
        int32 brokenFrame;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxDestroy;
        RSDK::SoundFX sfxTeleport;
        RSDK::SoundFX sfxHyperRing;
        RSDK::SoundFX sfxPowerDown;
        RSDK::SoundFX sfxRecovery;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<ItemBox> state;
    int32 type;
    RSDK::Vector2 contentsPos;
    RSDK::Vector2 moveOffset;
    int32 contentsSpeed;
    Entity *storedEntity;
    int32 timer;
    bool32 isFalling;
    bool32 isContents;
    bool32 hidden;
    int32 planeFilter;
    bool32 lrzConvPhys;
    RSDK::Animator boxAnimator;
    RSDK::Animator contentsAnimator;
    RSDK::Animator overlayAnimator;
    RSDK::Animator debrisAnimator;
    Entity *parent;
    int32 startTime;

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

    void DebugDraw();
    void DebugSpawn();

    void State_Done();
    void State_Break();
    void State_IconFinish();
    void State_Idle();
    void State_Falling();

    void CheckHit();
    void GivePowerup();
    void Break(Player *player);
    bool32 HandleFallingCollision();
    bool32 HandlePlatformCollision(Platform *platform);
    void HandleObjectCollisions();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ItemBox);
};
} // namespace GameLogic