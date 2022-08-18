#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct BadnikHelpers : RSDK::GameObject::Entity {

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
    static void BadnikBreak(RSDK::GameObject::Entity *badnik, bool32 destroy, bool32 spawnAnimals);
    static void BadnikBreakUnseeded(RSDK::GameObject::Entity *badnik, bool32 destroy, bool32 spawnAnimals);
    static int32 Oscillate(RSDK::GameObject::Entity *self, int32 origin, int32 speed, int32 amplitude);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(BadnikHelpers);
};
} // namespace GameLogic