#pragma once
#include "Game.hpp"

#include <vector>

namespace GameLogic
{

struct CollapsingPlatform : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Types {
        Left,
        Right,
        Center,
        LR,
        LRC,
        LR_Rv,
        LRC_Rv,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Animator animator;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxLedgeBreak;
        CollapsingPlatform *targetPlatform;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<CollapsingPlatform> state;
    RSDK::Vector2 size;
    bool32 respawn;
    RSDK::SceneLayer targetLayer;
    uint8 type;
    int32 delay;
    bool32 eventOnly;
    bool32 mightyOnly;
    int32 buttonTag;
    uint8 shift;
    int32 unused1;
    int32 collapseDelay;
    RSDK::Tile storedTiles[256];
    RSDK::Hitbox hitboxTrigger;
    int32 stoodPos;
    RSDK::Vector2 originPos;
    bool32 useLayerDrawGroup;
    bool32 hasSetupTagLink;
    std::vector<bool32 *> taggedButtons;
    float timerMultipler;

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

    void SetupTiles();
    void SetupHWrapTiles();

    void SetupTagLink();

    void CrumbleFromLeft();
    void CrumbleFromRight();
    void CrumbleFromCenter();
    void CrumbleFromLeftRight();
    void CrumbleFromLeftRightCenter();
    void CrumbleFromLeftRightCenter_Reverse();
    void CrumbleDestroy();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(CollapsingPlatform);
};
} // namespace GameLogic