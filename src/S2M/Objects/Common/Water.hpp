#pragma once
#include "S2M.hpp"

#include "Global/Player.hpp"

namespace GameLogic
{

struct Water : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Types {
        WaterLevel,
        Pool,
        Bubbler,
        HeightTrigger,
        Splash,
        Bubble,
        Countdown,
    };

    enum Priorities {
        PriorityLowest,
        PriorityLow,
        PriorityHigh,
        PriorityHighest,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 waterLevel;
        int32 newWaterLevel;
        int32 targetWaterLevel;
        int32 waterMoveSpeed;
        int32 constBubbleTimer[PLAYER_COUNT];
        int32 randBubbleTimer[PLAYER_COUNT];
        int32 unused1[PLAYER_COUNT];
        int32 bubbleSizes[18];
        RSDK::SpriteAnimation aniFrames;
        RSDK::SpriteAnimation wakeFrames;
        RSDK::Hitbox hitboxPlayerBubble;
        RSDK::Hitbox hitboxPoint;
        RSDK::SoundFX sfxSplash;
        RSDK::SoundFX sfxBreathe;
        RSDK::SoundFX sfxWarning;
        RSDK::SoundFX sfxDrownAlert;
        RSDK::SoundFX sfxDrown;
        RSDK::SoundFX sfxSkim;
        RSDK::SoundFX sfxDNAGrab;
        RSDK::SoundFX sfxDNABurst;
        RSDK::SoundFX sfxWaterLevelL;
        RSDK::SoundFX sfxWaterLevelR;
        int32 waterLevelChannelL;
        int32 waterLevelChannelR;
        int32 unused2;
        bool32 playingWaterLevelSfx;
        bool32 moveWaterLevel;
        int32 waterLevelVolume;
        uint8 waterPalette;
        color flashColorStorage[0x100];
        uint8 isLightningFlashing;
        uint8 disableWaterSplash;
        int32 wakePosX[PLAYER_COUNT];
        uint8 wakeDir[PLAYER_COUNT];
        RSDK::Animator wakeAnimator;
        int32 unused3;
        bool32 playingSkimSfx;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<Water> state;
    RSDK::StateMachine<Water> stateDraw;
    uint8 type;
    RSDK::GameObject::Entity *childPtr;
    int32 bubbleX;
    int8 bubbleType1;
    uint8 dudsRemaining;
    uint8 bubbleFlags;
    int8 bubbleType2;
    uint8 numDuds;
    int32 countdownID;
    uint8 allowBreathe;
    RSDK::Vector2 size;
    RSDK::Vector2 height;
    int32 speed;
    int32 buttonTag;
    uint8 r;
    uint8 g;
    uint8 b;
    uint8 priority;
    bool32 destroyOnTrigger;
    RSDK::Hitbox hitbox;
    int32 timer;
    uint8 activePlayers;
    uint8 releasedPlayers;
    RSDK::Vector2 bubbleOffset;
    RSDK::Vector2 bubbleVelocity;
    RSDK::GameObject::Entity *taggedButton;
    bool32 surfaceWaves;
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
    static void DrawHook_ApplyWaterPalette();
    static void DrawHook_RemoveWaterPalette();

    // Utils
    void SetupTagLink();
    static void SpawnBubble(Player *player, int32 id);
    static void SpawnCountDownBubble(Player *player, int32 id, uint8 bubbleID);
    void BubbleFloatBehavior();
    static void ApplyHeightTriggers();

    void State_Water();
    void State_Pool();
    void State_Splash();
    void BubbleFinishPopBehavior();
    void State_BubbleBreathed();
    void State_Bubbler();
    void State_Countdown();
    void State_CountdownFollow();
    void State_HeightTrigger();

    // Draw States
    void Draw_Water();
    void Draw_Pool();
    void Draw_Splash();
    void Draw_Countdown();
    void Draw_Bubbler();
    void Draw_Bubble();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Water);
};
} // namespace GameLogic