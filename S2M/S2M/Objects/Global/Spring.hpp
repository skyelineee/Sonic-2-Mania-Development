#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Spring : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Types {
        YellowVertical,
        RedVertical,
        YellowHorizontal,
        RedHorizontal,
        YellowDiagonal,
        RedDiagonal,

        TubeSpring = 0xFF,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxSpring;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<Spring> state;
    int32 type;
    int32 flipFlag;
    uint8 planeFilter;
    bool32 twirl;
    uint8 swap;
    int32 sfxTimer;
    RSDK::Animator animator;
    RSDK::Hitbox hitbox;
    bool32 passThrough;
    int16 velOverride;

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

    void DebugSpawn();
    void DebugDraw();

    void State_Vertical();
    void State_Horizontal();
    void State_Diagonal();
    void State_Falling();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Spring);
};
} // namespace GameLogic