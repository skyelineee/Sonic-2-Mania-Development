#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SuperFlicky : RSDK::GameObject::Entity {

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
        uint16 state;
        int32 targetPlayerID;
        int32 activeFlickyCount;
        RSDK::Hitbox hitbox;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<SuperFlicky> state;
    RSDK::StateMachine<SuperFlicky> stateStored;
    RSDK::Animator animator;
    RSDK::Vector2 startPos;
    int32 targetSlot;
    int32 flickyID;
    RSDK::Vector2 targetVelocity;
    RSDK::Vector2 targetPosition;
    int32 attackDelay;
    int32 attackListPos;
    int32 superBlendMode;
    int32 superBlendTimer;
    uint8 superPaletteIndex;

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

    void HandleSuperColors(bool32 updatePalette);
    void HandleAttack();

    void State_Init();
    void State_Active();
    void State_Restore();
    void State_Drop();
    void State_FlyAway();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SuperFlicky);
};
} // namespace GameLogic