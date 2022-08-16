#pragma once
#include "S2M.hpp"

#include "Global/Player.hpp"

namespace GameLogic
{

struct CutsceneSeq : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum SkipTypes {
        SkipDisabled,
        SkipReload,
        SkipNext,
        SkipCallback,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    bool32 (*currentState)(CutsceneSeq *host);
    uint8 stateID;
    int32 timer;
    int32 storedValue; // never reset, unlike timer & the 8 values
    int32 values[8];
    RSDK::GameObject::Entity *activeEntity;  // the entity that called StartSequence
    RSDK::GameObject::Entity *managerEntity; // the host entity of the sequence
    bool32 (*cutsceneStates[0x40])(CutsceneSeq *host);
    RSDK::Vector2 points[8];
    int32 fadeWhite;
    int32 fadeBlack;
    int32 skipType;
    void (*skipCallback)(void);

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

    // Initializes a new state with ID of `nextState`
    void NewState(int32 nextState);
    // Sets the cutscene's skip type (and callback if applicable)
    static void SetSkipType(uint8 type, void (*skipCallback)());
    // Checks if the cutscene was skipped
    void CheckSkip(uint8 skipType, void (*skipCallback)());
    // Does a foreach loop for the entity of type `type`
    static RSDK::GameObject::Entity *GetEntity(int32 type);
    // Locks control of the selected player
    static void LockPlayerControl(Player *player);
    // Locks Control of all players
    static void LockAllPlayerControl();
    // Sets up a cutscene sequence, the cutscene object should be passed as 'manager', then the cutscene states should be passed in order, make sure
    // to end the states with StateMachine_None to tell it when to stop reading states
    static void StartSequence(RSDK::GameObject::Entity *manager, ...);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(CutsceneSeq);
};
} // namespace GameLogic