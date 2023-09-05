#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Player;

struct Jellygnite : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum JellygniteAnimIDs {
        JELLYGNITE_ANI_FLOATING,
        JELLYGNITE_ANI_ANGRY,
        JELLYGNITE_ANI_FLASHING,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitbox;
        RSDK::Hitbox hitboxUnused;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxGrab;
        RSDK::SoundFX sfxElectrify;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Jellygnite> state;
    RSDK::Vector2 startPos;
    uint8 startDir;
    uint8 timer;
    uint8 grabDelay;
    int32 frontTentacleAngle;
    int32 oscillateAngle;
    Player *grabbedPlayer;
    uint8 shakeTimer;
    uint8 shakeCount;
    uint8 lastShakeFlags;
    uint8 prevAnimationID;
    RSDK::Animator bodyAnimator;
    RSDK::Animator frontTentacleAnimator;
    RSDK::Animator backTentacleAnimator;

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

    void DebugSpawn();
    void DebugDraw();

    void SetupAnimations(uint8 animationID);

    void CheckPlayerCollisions();
    void HandlePlayerStruggle();
    bool32 CheckInWater(Player *player);

    void DrawBackTentacle();
    void DrawFrontTentacle();

    void State_Init();
    void State_Swimming();
    void State_GrabbedPlayer();
    void State_Explode();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Jellygnite);
};
} // namespace GameLogic
