#pragma once
#include "S2M.hpp"

namespace GameLogic
{

#define BOUNCEPLANT_NODE_COUNT (8)

struct BouncePlant : RSDK::GameObject::Entity {

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
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxBouncePlant;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<BouncePlant> state;
    int32 recoilDuration;
    RSDK::Vector2 drawPos[BOUNCEPLANT_NODE_COUNT];
    RSDK::Vector2 nodeStartPos[BOUNCEPLANT_NODE_COUNT];
    int32 recoilVelocity[BOUNCEPLANT_NODE_COUNT];
    RSDK::Vector2 stoodPos;
    bool32 stood;
    bool32 instantRecoil;
    int32 depression;
    int32 centerX;
    RSDK::Animator nodeAnimator;
    RSDK::Animator decorAnimators[BOUNCEPLANT_NODE_COUNT];

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

    void SetupNodePositions();
    int32 GetNodeStandY(int32 x);
    int32 GetNodeY(int32 x);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(BouncePlant);
};
} // namespace GameLogic
