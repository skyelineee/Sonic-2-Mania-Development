#pragma once
#include "S2M.hpp"
#include "Camera.hpp"

namespace GameLogic
{

struct Player : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum PlayerAnis {
        ANI_IDLE,
        ANI_BORED_1,
        ANI_BORED_2,
        ANI_LOOK_UP,
        ANI_CROUCH,
        ANI_WALK,
        ANI_FALL,
        ANI_JOG,
        ANI_RUN,
        ANI_DASH,
        ANI_JUMP,
        ANI_SPRING,
        ANI_SPRING_CS,
        ANI_SKID,
        ANI_SKID_TURN,
        ANI_SPINDASH,
        ANI_PUSH,
        ANI_HURT,
        ANI_DIE,
        ANI_DROWN,
        ANI_BALANCE_1,
        ANI_BALANCE_2,
        ANI_FAN,
        ANI_VICTORY,
        ANI_HANG,
        ANI_HANG_MOVE,
        ANI_POLE_SWING_H,
        ANI_POLE_SWING_V,
        ANI_SHIMMY_IDLE,
        ANI_SHIMMY_MOVE,
        ANI_BREATHE,
        ANI_CLING,
        ANI_FLUME,
        ANI_AIRCURL,
        ANI_TRANSFORM,
        ANI_ABILITY_0,
        ANI_ABILITY_1,
        ANI_ABILITY_2,
        ANI_ABILITY_3,
        ANI_ABILITY_4,
        ANI_ABILITY_5,
        ANI_ABILITY_6,
        ANI_ABILITY_7,
        ANI_ABILITY_8,
        ANI_ABILITY_9,
        ANI_ABILITY_10,
        ANI_ABILITY_11,
        ANI_ABILITY_12,

        // Sonic Ability Anim Aliases
        ANI_DROPDASH            = ANI_ABILITY_0,
        ANI_PEELOUT             = ANI_ABILITY_1,

        // Tails Ability Anim Aliases
        ANI_FLY                  = ANI_ABILITY_0,
        ANI_FLY_TIRED            = ANI_ABILITY_1,
        ANI_FLY_LIFT             = ANI_ABILITY_2,
        ANI_FLY_LIFT_DOWN        = ANI_ABILITY_3,
        ANI_FLY_LIFT_TIRED       = ANI_ABILITY_4,
        ANI_SWIM                 = ANI_ABILITY_5,
        ANI_SWIM_TIRED           = ANI_ABILITY_6,
        ANI_SWIM_LIFT            = ANI_ABILITY_7,

        // Knux Ability Anim Aliases
        ANI_LEDGE_PULL_UP = ANI_ABILITY_0,
        ANI_GLIDE         = ANI_ABILITY_1,
        ANI_GLIDE_DROP    = ANI_ABILITY_2,
        ANI_GLIDE_LAND    = ANI_ABILITY_3,
        ANI_GLIDE_SLIDE   = ANI_ABILITY_4,
        ANI_CLIMB_IDLE    = ANI_ABILITY_5,
        ANI_CLIMB_UP      = ANI_ABILITY_6,
        ANI_CLIMB_DOWN    = ANI_ABILITY_7,

    };

    enum DeathTypes {
        DeathNone,
        DeathDie_Sfx,
        DeathDie_NoSfx,
        DeathDrown,
    };

    enum HurtTypes {
        HurtNone,
        HurtShield,
        HurtRingLoss,
        HurtDie,
    };

    enum SuperStates {
        SuperStateNone,
        SuperStateFadeIn,
        SuperStateSuper,
        SuperStateFadeOut,
        SuperStateDone,
    };

    enum SpriteTypes {
        ClassicSprites,
        ManiaSprites,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 sonicPhysicsTable[64];
        int32 tailsPhysicsTable[64];
        int32 knuxPhysicsTable[64];

        color superSonicPalette[18];
        color superTailsPalette[18];
        color superKnuxPalette[18];

        color *activeSuperSonicPalette;
        color *activeSuperSonicPalette_Water;

        color *activeSuperTailsPalette;
        color *activeSuperTailsPalette_Water;

        color *activeSuperKnuxPalette;
        color *activeSuperKnuxPalette_Water;

        float spindashChargeSpeeds[13];

        RSDK::Hitbox instaShieldHitbox;
        RSDK::Hitbox shieldHitbox;

        int32 cantSwap;
        int32 playerCount;
        int32 activePlayerCount;

        uint16 upState;
        uint16 downState;
        uint16 leftState;
        uint16 rightState;
        uint16 jumpPressState;
        uint16 jumpHoldState;

        int32 nextLeaderPosID;
        int32 lastLeaderPosID;
        RSDK::Vector2 leaderPositionBuffer[16];
        RSDK::Vector2 targetLeaderPosition;

        int32 autoJumpTimer;
        int32 respawnTimer;
        int32 aiInputSwapTimer;
        bool32 disableP2KeyCheck;

        int32 rings;
        int32 ringExtraLife;
        int32 powerups;
        int32 savedLives;
        int32 savedScore;
        int32 savedScore1UP;

        RSDK::SpriteAnimation sonicFrames;
        RSDK::SpriteAnimation superFrames;
        RSDK::SpriteAnimation tailsFrames;
        RSDK::SpriteAnimation tailsTailsFrames;
        RSDK::SpriteAnimation knuxFrames;

        RSDK::SoundFX sfxJump;
        RSDK::SoundFX sfxRoll;
        RSDK::SoundFX sfxCharge;
        RSDK::SoundFX sfxRelease;
        RSDK::SoundFX sfxPeelCharge;
        RSDK::SoundFX sfxPeelRelease;
        RSDK::SoundFX sfxDropdash;
        RSDK::SoundFX sfxLoseRings;
        RSDK::SoundFX sfxHurt;
        RSDK::SoundFX sfxSkidding;
        RSDK::SoundFX sfxGrab;
        RSDK::SoundFX sfxFlying;
        bool32 playingFlySFX;
        RSDK::SoundFX sfxTired;
        bool32 playingTiredSFX;
        RSDK::SoundFX sfxLand;
        RSDK::SoundFX sfxSlide;
        RSDK::SoundFX sfxTransform2;
        RSDK::SoundFX sfxSwap;
        RSDK::SoundFX sfxSwapFail;

        bool32 gotHit[PLAYER_COUNT];
        RSDK::StateMachine<Player> configureGhostCB;
        bool32 (*canSuperCB)(bool32 isHUD);
        int32 superDashCooldown;
        uint16 lookUpDelay;
        uint16 lookUpDistance;
        bool32 showHitbox;
        bool32 finishedInput;
    };

    static RSDK::Hitbox fallbackHitbox;

    // ==============================
    // INSTANCE VARS
    // ==============================
    int32 rings;
    int32 lives;
    int32 score;
    int32 ringExtraLife;
    int32 score1UP;
    bool32 hyperRing;
    uint8 shield;
    RSDK::StateMachine<Player> state;
    RSDK::StateMachine<Player> nextAirState;
    RSDK::StateMachine<Player> nextGroundState;
    RSDK::StateMachine<Player> stateGravity;
    Camera *camera;
    RSDK::Animator animator;
    RSDK::Animator tailAnimator;
    int32 minJogVelocity;
    int32 minRunVelocity;
    int32 minDashVelocity;
    int32 unused;
    int32 tailRotation;
    int32 tailDirection;
    RSDK::SpriteAnimation aniFrames;
    RSDK::SpriteAnimation tailFrames;
    uint16 animationReserve;
    uint16 playerID;
    RSDK::Hitbox *outerBox;
    RSDK::Hitbox *innerBox;
    int32 characterID;
    int32 timer;
    int32 outtaHereTimer;
    int32 abilityTimer;
    int32 spindashCharge;
    int32 abilityValue;
    int32 drownTimer;
    int32 invincibleTimer;
    int32 speedShoesTimer;
    int32 blinkTimer;
    int32 scrollDelay;
    int32 skidding;
    int32 pushing;
    int32 underwater;
    bool32 groundedStore;
    bool32 invertGravity;
    bool32 isChibi;
    bool32 isTransforming;
    bool32 disableGravity;
    int32 superState;
    int32 superRingLossTimer;
    int32 superBlendAmount;
    int32 superBlendState;
    int32 superBlendTimer;
    uint8 superColorIndex;
    uint8 superColorCount;
    int8 superTableIndex;
    bool32 sidekick;
    int32 scoreBonus;
    int32 jumpOffset;
    int32 collisionFlagH;
    int32 collisionFlagV;
    int32 topSpeed;
    int32 acceleration;
    int32 deceleration;
    int32 airAcceleration;
    int32 airDeceleration;
    int32 skidSpeed;
    int32 rollingFriction;
    int32 rollingDeceleration;
    int32 gravityStrength;
    int32 abilitySpeed;
    int32 jumpStrength;
    int32 jumpCap;
    bool32 flailing;
    int32 sensorX[5];
    int32 sensorY;
    RSDK::Vector2 moveLayerPosition;
    RSDK::Vector2 lastMoveLayerPosition;
    RSDK::StateMachine<Player> stateInputReplay;
    RSDK::StateMachine<Player> stateInput;
    uint8 controllerID;
    int32 controlLock;
    bool32 up;
    bool32 down;
    bool32 left;
    bool32 right;
    bool32 jumpPress;
    bool32 jumpHold;
    bool32 applyJumpCap;
    int32 jumpAbilityState;
    RSDK::StateMachine<Player> stateAbility;
    RSDK::StateMachine<Player> statePeelout;
    int32 flyCarryTimer;
    RSDK::Vector2 flyCarrySidekickPos;
    RSDK::Vector2 flyCarryLeaderPos;
    uint8 deathType;
    bool32 forceHoldRespawn;
    bool32 isGhost;
    int32 abilityValues[8];
    void *abilityPtrs[8];
    int32 uncurlTimer;
    int32 spriteType;
    bool32 disableGroundAnims;
    RSDK::StateMachine<Player> storedStateInput;
    RSDK::Vector2 field_3D0;
    bool32 disableTileCollisions;

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

    // Misc
    static void LoadSprites();
    static void LoadSpritesVS();
    void LoadPlayerSprites();

    static void DrawSprite(Player *self, RSDK::Animator *animator);
    bool32 CheckValidState();

    static void SaveValues();
    void GiveScoreBonus(RSDK::Vector2 position);
    void GiveScore(int32 score);
    void GiveRings(int32 amount, bool32 playSfx);
    void GiveLife();
    void UpdatePhysicsState();
    void ApplyShield();
    void ChangeCharacter(int32 character);
    void InvertGravity();
    bool32 TryTransform(bool32 fastTransform, bool32 force);

    static Player *GetNearestPlayerX();
    static Player *GetNearestPlayerXY();

    void HandleIdleAnimation_Classic();
    void HandleGroundAnimation_Classic();
    void HandleIdleAnimation();
    void HandleGroundAnimation();
    void HandleGroundMovement();
    void HandleGroundRotation();
    void HandleAirFriction();
    void HandleRollDeceleration();
    void CheckStartFlyCarry(Player *leader);
    void HandleSuperColors_Sonic(bool32 updatePalette);
    void HandleSuperColors_Tails(bool32 updatePalette);
    void HandleSuperColors_Knux(bool32 updatePalette);
    void HandleSuperForm();
    void DoSuperDash(Player *player);
    void FinishedReturnToPlayer(Player *leader);
    void ResetBoundaries();
    void HandleDeath();

    // Actions
    void Action_Jump();
    void Action_Roll();
    void Action_Spindash();
    void Action_Peelout();
    void Action_DblJumpSonic();
    void Action_DblJumpTails();
    void Action_DblJumpKnux();
    void Action_SuperDash();

    // Movement States
    void State_Static();
    void State_Ground();
    void State_Air();
    void State_LookUp();
    void State_Crouch();
    void State_Roll();
    void State_Spindash();
    void State_Peelout();
    void State_Hurt();
    void State_Death();
    void State_Drown();
    void State_DeathHold();
    void State_DropDash();
    void State_BubbleBounce();
    void State_TailsFlight();
    void State_FlyCarried();
    void State_KnuxGlideLeft();
    void State_KnuxGlideRight();
    void State_KnuxGlideDrop();
    void State_KnuxGlideSlide();
    void State_KnuxWallClimb();
    void State_KnuxLedgePullUp();
    void State_FlyToPlayer();
    void State_ReturnToPlayer();
    void State_HoldRespawn();
    void State_Victory();
    void State_TubeRoll();
    void State_TubeAirRoll();
    void State_TransportTube();
    void State_WaterCurrent();
    void State_GroundFalse();
    void State_Transform();
    void State_StartSuper();
    void State_SuperFlying();

    // Gravity States
    void Gravity_NULL();
    void Gravity_True();
    void Gravity_False();

    // Input Helpers
    void DisableInputs(bool32 backupState);
    bool32 CheckP2KeyPress();
    void HandleSidekickRespawn();
    void GetDelayedInput();

    // Input States
    void Input_Gamepad();
    void Input_GamepadAssist();
    void Input_NULL();
    void Input_AI_WaitForP1();
    void Input_AI_Follow();
    void Input_AI_SpindashPt1();
    void Input_AI_SpindashPt2();

    // Collision
    RSDK::Hitbox *GetHitbox();
    RSDK::Hitbox *GetAltHitbox();
    bool32 CheckCollisionTouch(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox);
    bool32 CheckCollisionTouchDefault(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox);
    void BoxCollisionResponce(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox, uint32 side);
    uint8 CheckCollisionBox(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox);
    bool32 CheckCollisionPlatform(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox);
    bool32 CheckMightyUnspin(int32 bounceDistance, bool32 checkHammerDrop, int32 *uncurlTimer);

    // Damage
    void Hit(bool32 forceKill = false);
    bool32 Hurt(RSDK::GameObject::Entity *entity, bool32 forceKill = false);
    bool32 FireHurt(RSDK::GameObject::Entity *entity);
    bool32 CheckAttacking(RSDK::GameObject::Entity *entity);
    bool32 CheckBadnikTouch(RSDK::GameObject::Entity *entity, RSDK::Hitbox *entityHitbox);
    bool32 CheckBadnikBreak(RSDK::GameObject::Entity *badnik, bool32 destroy);
    bool32 CheckBossHit(RSDK::GameObject::Entity *entity);
    bool32 LightningHurt(RSDK::GameObject::Entity *entity);
    bool32 ProjectileHurt(RSDK::GameObject::Entity *entity);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Player);
};
} // namespace GameLogic