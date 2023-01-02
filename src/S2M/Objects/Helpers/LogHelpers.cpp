// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: LogHelpers Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "LogHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(LogHelpers);

void LogHelpers::PrintText(const char *message)
{
    if (!sceneInfo->inEditor)
        Dev::Print(Dev::PRINT_NORMAL, message);
}

void LogHelpers::PrintString(String *message)
{
    if (!sceneInfo->inEditor)
        Dev::PrintString(Dev::PRINT_NORMAL, message);
}

void LogHelpers::PrintInt32(const char *message, int32 value)
{
    if (!sceneInfo->inEditor)
        Dev::Print(Dev::PRINT_NORMAL, message, value);
}
void LogHelpers::Print(const char *message, ...)
{
    if (!sceneInfo->inEditor) {
        char messageText[0x100];

        va_list args;
        va_start(args, message);
        vsprintf(messageText, message, args);

        Dev::Print(Dev::PRINT_NORMAL, messageText);

        va_end(args);
    }
}
void LogHelpers::PrintVector2(const char *message, RSDK::Vector2 value)
{
    if (!sceneInfo->inEditor)
        Dev::Print(Dev::PRINT_NORMAL, message, &value);
}

#if RETRO_REV0U
void LogHelpers::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(LogHelpers); }
#endif

} // namespace GameLogic