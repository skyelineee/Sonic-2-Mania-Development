#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct ChemicalBall : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum ChemicalBallTypes { CHEMICALBALL_ARC, CHEMICALBALL_VERTICAL };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::Hitbox hitboxBall;
        int32 arcOffsets[196];
        int32 moveOffsets[98];
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxBloop;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<ChemicalBall> state;
    uint8 type;
    uint8 interval;
    uint8 intervalOffset;
    int32 unused;
    RSDK::Vector2 startPos;
    RSDK::Vector2 drawPos;
    RSDK::Animator animator;
    int32 unused2;
    bool32 master;

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

    void CheckHit();
    void State_MoveArc();
    void State_MoveVertical();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ChemicalBall);
};
} // namespace GameLogic
