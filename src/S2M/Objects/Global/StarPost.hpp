#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct StarPost : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitbox;
        bool32 hasAchievement;
        RSDK::Vector2 playerPositions[PLAYER_COUNT];
        uint8 playerDirections[PLAYER_COUNT];
        uint16 postIDs[PLAYER_COUNT];
        uint8 storedMinutes;
        uint8 storedSeconds;
        uint8 storedMilliseconds;
        uint8 interactablePlayers;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxStarPost;
        RSDK::SoundFX sfxWarp;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<StarPost> state;
    int32 id;
    bool32 vsRemove;
    int32 ballSpeed;
    int32 timer;
    int32 starTimer;
    int32 bonusStageID;
    RSDK::Vector2 starAngle;
    int32 starRadius;
    RSDK::Vector2 ballPos;
    RSDK::Animator poleAnimator;
    RSDK::Animator ballAnimator;
    RSDK::Animator starAnimator;
    RSDK::Hitbox hitboxStars;
    uint8 interactedPlayers;

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

    static void ResetStarPosts();

    void CheckBonusStageEntry();
    void CheckCollisions();

    // States
    void State_Idle();
    void State_Spinning();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(StarPost);
};
} // namespace GameLogic