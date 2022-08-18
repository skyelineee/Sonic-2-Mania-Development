// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: PlayerHelpers Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "PlayerHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(PlayerHelpers);

bool32 PlayerHelpers::IsPaused()
{
    GameObject::Get(SLOT_PAUSEMENU);
    return false;
}


#if RETRO_REV0U
void PlayerHelpers::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(PlayerHelpers); }
#endif

} // namespace GameLogic