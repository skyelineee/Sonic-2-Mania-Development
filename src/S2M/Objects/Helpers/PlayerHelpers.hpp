#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct PlayerHelpers : RSDK::GameObject::Entity {

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
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    // ==============================
    // EVENTS
    // ==============================

#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif

    // ==============================
    // FUNCTIONS
    // ==============================
    static bool32 IsPaused();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(PlayerHelpers);
};
} // namespace GameLogic