#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct DebugMode : RSDK::GameObject::Entity {

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
        int16 classIDs[256];
        RSDK::StateMachine<DebugMode> draw[256];
        RSDK::StateMachine<DebugMode> spawn[256];
        RSDK::Animator animator;
        int32 itemID;
        int32 itemCount;
        uint8 debugActive;
        uint8 itemType;
        uint8 itemTypeCount;
        int32 unused1;
        int32 unused2;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<DebugMode> state;
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

    void State_None();

    template <typename T> static inline void AddObject(uint16 id, void (T::*spawn)(), void (T::*draw)())
    {
        if (sVars->itemCount < 0x100) {
            sVars->classIDs[sVars->itemCount] = id;
            sVars->draw[sVars->itemCount].Set(draw);
            sVars->spawn[sVars->itemCount].Set(spawn);
            sVars->itemCount++;
        }
    }

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(DebugMode);
};
} // namespace GameLogic