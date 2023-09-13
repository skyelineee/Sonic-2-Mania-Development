// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: RPUpdate Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

// lol this object is tiny
// its only purpose to is to run RPCallback(); every frame so the rich presence can update
// this object HAS to be in any stage with a setup that calls SetPresence();, otherwise it wont update properly

#include "RPUpdate.hpp"
#include "Helpers/RPCHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(RPUpdate);

void RPUpdate::StaticUpdate() { RPCallback(); }

} // namespace GameLogic