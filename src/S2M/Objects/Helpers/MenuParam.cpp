// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: MenuParam Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "MenuParam.hpp"
#include "LogHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(MenuParam);

MenuParam *MenuParam::GetMenuParam()
{ 
	return (MenuParam *)globals->menuParam;
}

#if RETRO_REV0U
void MenuParam::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(MenuParam); }
#endif

} // namespace GameLogic