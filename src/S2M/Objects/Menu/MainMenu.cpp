// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: MainMenu Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "MainMenu.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(MainMenu);

void MainMenu::Update() {}
void MainMenu::LateUpdate() {}
void MainMenu::StaticUpdate()
{

}
void MainMenu::Draw() {}

void MainMenu::Create(void *data) {}

void MainMenu::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void MainMenu::EditorDraw() {}

void MainMenu::EditorLoad() {}
#endif

void MainMenu::Serialize() {}

} // namespace GameLogic