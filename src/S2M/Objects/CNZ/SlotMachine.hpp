#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SlotMachine : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum ActiveSlots {
        Slot_L,
        Slot_M,
        Slot_R,
    };

    enum SpinTimerAliases {
        SlotTimer_Spin = 0, // Spinning, attempting to stop
        SlotTimer_Slow = 3, // Stopping in progress, Bounce it back for a few frames
        SlotTimer_Stop = 6, // Stop Slot Movement all together
        SlotTimer_Calc = 7, // Calculating Rewards (Only happens on slot R)
    };

    enum SlotDisplayFrameIDs {
        Sonic,
        Tails,
        Ring,
        Bar,
        Jackpot,
        Eggman,
        Knux, // Only Sonic OR Knux can appear at any one time
        Super, // (Unused)
    };

    enum SlotMachineAnims {
        Inactive,
        Active,
    };

    enum SlotMachineTypes {
        ComboMachine,
        PrizeMachine,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 prizeLTable[8];
        int32 prizeMTable[8];
        int32 prizeRTable[8];
        int32 slotRewardTable[6];
        int32 randomizeTable[28];
        int32 slotPrizeL;
        int32 slotPrizeM;
        int32 slotPrizeR;
        RSDK::Hitbox hitboxEntry;
        RSDK::Hitbox hitboxLeaveL;
        RSDK::Hitbox hitboxLeaveR;
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxGrab;
        RSDK::SoundFX sfxSlot;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<SlotMachine> state;
    RSDK::Animator animator;
    int32 timer;
    int32 slotSpinTimer;
    int32 spinSpeedL;
    int32 spinSpeedM;
    int32 spinSpeedR;
    int32 prizeTablePosL;
    int32 prizeTablePosM;
    int32 prizeTablePosR;
    int32 targetSpinSpeedL;
    int32 targetSpinSpeedM;
    int32 targetSpinSpeedR;
    int32 activeSlot;
    int32 rewardRingCount;
    int32 type;

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
    void PlayRingSFX();

    // STATES
    void State_Idle();
    void State_Ready();
    void State_Hold();
    void State_Unused();
    void State_Reward();
    void State_Wait();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SlotMachine);
};
} // namespace GameLogic
