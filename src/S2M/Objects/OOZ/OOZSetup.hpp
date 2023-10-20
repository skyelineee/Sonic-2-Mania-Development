#pragma once
#include "S2M.hpp"
#include "Helpers/Soundboard.hpp"

namespace GameLogic
{

struct OOZSetup : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum OOZWarpDoorEffects { OOZ_WARPDOOR_EFFECT_TO_SUB = 1, OOZ_WARPDOOR_EFFECT_FROM_SUB = 2 };
    enum OOZSetupTypes { OOZSETUP_FLAMES, OOZSETUP_FADE };

    enum TileFlagsOOZ { OOZ_TFLAGS_NORMAL, OOZ_TFLAGS_OILPOOL, OOZ_TFLAGS_OILSTRIP, OOZ_TFLAGS_OILSLIDE, OOZ_TFLAGS_OILFALL };

    enum GenericTriggerTypesOOZ { OOZ_GENERICTRIGGER_ACHIEVEMENT };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 palTimer;
        uint16 aniTiles;
        RSDK::TileLayer *background1;
        int32 deformData[64];
        int32 oilAniTimer;
        int32 glowLightAniFrame;
        int32 glowLightAniTimer;
        int32 glowLightAniDurations[9];
        int32 swimmingPlayerCount;
        int32 smogTimer;
        int32 useSmogEffect;
        uint8 flameTimers[0x20000];
        uint8 *flameTimerPtrs[400];
        RSDK::Vector2 flamePositions[400];
        uint16 flameCount;
        uint8 activePlayers;
        RSDK::Animator flameAnimator;
        RSDK::Animator animator;
        RSDK::SpriteAnimation solFrames;
        RSDK::SpriteAnimation splashFrames;
        bool32 hasAchievement;
        Entity *cutscenePtr;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    uint8 type;

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

    static Soundboard::SoundInfo SfxCheck_Flame2();
    static Soundboard::SoundInfo SfxCheck_Slide();
    static Soundboard::SoundInfo SfxCheck_OilSwim();

    void Draw_Flames();
    void HandleActiveFlames();
    bool32 StartFire(int32 posX, int32 posY, int32 angle);

    void PlayerState_OilPool();
    void PlayerState_OilStrip();
    void PlayerState_OilSlide();
    void PlayerState_OilFall();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(OOZSetup);
};
} // namespace GameLogic