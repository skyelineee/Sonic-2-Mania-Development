#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Stegway : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxBadnik;
        RSDK::Hitbox hitboxRange;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxRev;
        RSDK::SoundFX sfxRelease;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<Stegway> state;
    RSDK::Vector2 startPos;
    uint8 startDir;
    int32 timer;
    int32 unused1;
    RSDK::Animator mainAnimator;
    RSDK::Animator wheelAnimator;
    RSDK::Animator jetAnimator;
    bool32 showJet;
    int32 unused2;
    bool32 noFloor;

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

    void CheckOffScreen();
    void HandlePlayerInteractions();
    void SetupAnims(char type, bool32 force);

    void State_Init();
    void State_Moving();
    void State_Turn();
    void State_RevUp();
    void State_RevRelease();
    void State_Dash();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Stegway);
};
} // namespace GameLogic
