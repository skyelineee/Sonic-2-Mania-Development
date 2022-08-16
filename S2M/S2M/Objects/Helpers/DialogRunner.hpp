#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct DialogRunner : RSDK::GameObject::Entity {

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
        int32 authForbidden;
        bool32 signedOut;
        int32 unused1;
        int32 unused2;
        bool32 isAutoSaving;
        void *activeCallback;
        int32 unused3;
        int32 unused4;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<DialogRunner> state;
    int32 timer;
    int32 unused[20];
    bool32 useGenericText;
    void(*callback)();

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Update();

    static void StageLoad();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(DialogRunner);
};
} // namespace GameLogic