#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct LogHelpers : RSDK::GameObject::Entity {

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
    static void PrintText(const char *message);
    static void PrintString(RSDK::String *message);
    static void PrintInt32(const char *message, int32 value);
    static void Print(const char *message, ...);
    static void PrintVector2(const char *message, RSDK::Vector2 value);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(LogHelpers);
};
} // namespace GameLogic