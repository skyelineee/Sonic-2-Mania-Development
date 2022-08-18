// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TestObject Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "TestObject.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TestObject);

void TestObject::Update() {}
void TestObject::LateUpdate() {}
void TestObject::StaticUpdate() {}
void TestObject::Draw() {}

void TestObject::Create(void *data) {}

void TestObject::StageLoad() { Dev::Print(Dev::PRINT_NORMAL, "Test, wow"); }

#if RETRO_INCLUDE_EDITOR
void TestObject::EditorDraw() {}

void TestObject::EditorLoad() {}
#endif

#if RETRO_REV0U
void TestObject::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(TestObject); }
#endif

void TestObject::Serialize() {}

} // namespace GameLogic