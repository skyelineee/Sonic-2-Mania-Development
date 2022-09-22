#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct ScreenWrap : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum Types {
        Vertical,
        Horizontal,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::Animator animator;
        ScreenWrap *activeVWrap;
        ScreenWrap *activeHWrap;
        int32 timer;
        RSDK::StateMachine<ScreenWrap> stateWrapUp;
        RSDK::StateMachine<ScreenWrap> stateWrapDown;
        RSDK::StateMachine<ScreenWrap> stateWrapLeft;
        RSDK::StateMachine<ScreenWrap> stateWrapRight;
        uint8 field_A0;
        uint8 handlingWrap;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::Vector2 drawPos;
    int32 vBoundary;
    int16 vWrapMode;
    int16 vWrapState;
    bool32 setVBounds;
    RSDK::Vector2 buffer;
    bool32 deactivated;
    uint8 type;
    RSDK::StateMachine<ScreenWrap> state;
    int32 hWrapPlayerID;
    int32 hWrapDistance;

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
    static bool32 CheckCompetitionWrap();
    static void WrapTileLayer(uint8 layerID, bool32 right);
    static bool32 Unknown1(RSDK::GameObject::Entity *entity1, RSDK::GameObject::Entity *entity2);
    void HandleVWrap(bool32 noPlayer, int32 direction);
    static void HandleHWrap(void *state, bool32 noPlayer);

    void State_Vertical();
    void State_Horizontal();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ScreenWrap);
};
} // namespace GameLogic