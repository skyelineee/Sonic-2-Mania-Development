#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct HUD : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum OffsetTypes {
        OffsetScore,
        OffsetTime,
        OffsetRings,
        OffsetLife,
    };

    enum KeyIDs {
        KeyA,
        KeyB,
        KeyX,
        KeyY,
        KeyStart,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SpriteAnimation superButtonFrames;
        RSDK::SoundFX sfxClick;
        RSDK::SoundFX sfxStarpost;
        uint8 showTAPrompt;
        uint8 replaySaveEnabled;
        int32 screenBorderType[PLAYER_COUNT];
        int32 swapCooldown;
        int32 stockFlashTimers[PLAYER_COUNT];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<HUD> state;
    RSDK::Vector2 scoreOffset;
    RSDK::Vector2 timeOffset;
    RSDK::Vector2 ringsOffset;
    RSDK::Vector2 lifeOffset;

    int32 moveTimer;

    int32 lifeFrameIDs[PLAYER_COUNT];
    int32 lives[PLAYER_COUNT];
    int32 actionPromptPos;
    RSDK::StateMachine<HUD> vsStates[PLAYER_COUNT];
    RSDK::Vector2 vsScoreOffsets[PLAYER_COUNT];
    RSDK::Vector2 vsTimeOffsets[PLAYER_COUNT];
    RSDK::Vector2 vsRingsOffsets[PLAYER_COUNT];
    RSDK::Vector2 vsLifeOffsets[PLAYER_COUNT];
    int32 vsMaxOffsets[PLAYER_COUNT];
    int32 screenID;
    int32 timeFlashFrame;
    int32 ringFlashFrame;
    uint8 enableTimeFlash;
    uint8 enableRingFlash;
    RSDK::Animator hudElementsAnimator;
    RSDK::Animator numbersAnimator;
    RSDK::Animator hyperNumbersAnimator;
    RSDK::Animator lifeIconAnimator;
    RSDK::Animator lifeNumbersAnimator;
    RSDK::Animator lifeNamesAnimator;
    RSDK::Animator playerIDAnimator;
    RSDK::Animator superIconAnimator;
    RSDK::Animator superButtonAnimator;
    RSDK::Animator thumbsUpIconAnimator;
    RSDK::Animator thumbsUpButtonAnimator;
    RSDK::Animator replayClapAnimator;
    RSDK::Animator saveReplayButtonAnimator;
    bool32 showMilliseconds;

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

    void DrawLifeNumbers(RSDK::Vector2 *drawPos, int32 value, int32 digitCount);
    void DrawNumbersBase10(RSDK::Vector2 *drawPos, int32 value, int32 digitCount);
    void DrawNumbersBase16(RSDK::Vector2 *drawPos, int32 value);
    void DrawNumbersHyperRing(RSDK::Vector2 *drawPos, int32 value);

    void GetButtonFrame(RSDK::Animator *animator, int32 buttonID);
    void GetActionButtonFrames();

    void State_MoveIn();
    void State_MoveOut();

    static void MoveIn(HUD *hud);
    static void MoveOut(HUD *hud);

    static void EnableRingFlash();
    static int32 CharacterIndexFromID(int32 characterID);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(HUD);
};
} // namespace GameLogic