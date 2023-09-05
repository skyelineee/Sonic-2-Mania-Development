#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Player;

struct SpinBooster : RSDK::GameObject::Entity {

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
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    uint8 autoGrip;
    uint8 bias;
    int32 size;
    int32 boostPower;
    bool32 boostAlways;
    bool32 forwardOnly;
    bool32 playSound;
    bool32 allowTubeInput;
    uint8 activePlayers;
    int32 unused;
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
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    int32 GetRollDir(Player *player);
    void HandleRollDir(Player *player);
    void ApplyRollVelocity(Player *player);
    void DrawArrow(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color);
    void DrawSprites();
    void HandleForceRoll(Player *player);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SpinBooster);
};
} // namespace GameLogic
