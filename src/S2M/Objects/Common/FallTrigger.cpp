// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: FallTrigger Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "FallTrigger.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(FallTrigger);

void FallTrigger::Update() 
{ 
    this->visible = DebugMode::sVars->debugActive; 
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->CheckCollisionTouch(this, &this->touchHitbox)) {
            player->animator.SetAnimation(player->aniFrames, Player::ANI_FLUME, false, 0);
            player->state.Set(&Player::State_Air); // this makes sure the player is falling anytime theyre in the hitbox
        }
    }
}
void FallTrigger::LateUpdate() {}
void FallTrigger::StaticUpdate() {}
void FallTrigger::Draw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 17);
    this->animator.DrawSprite(&this->position, false);
}

void FallTrigger::Create(void *data)
{
    this->active        = ACTIVE_BOUNDS;
    this->visible       = false;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->drawGroup     = Zone::sVars->hudDrawGroup;
}

void FallTrigger::StageLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }

#if RETRO_INCLUDE_EDITOR
void FallTrigger::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 17);
    this->animator.DrawSprite(&this->position, false);
}

void FallTrigger::EditorLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }
#endif

void FallTrigger::Serialize()
{ 
    // lol customizable hitbox
    RSDK_EDITABLE_VAR(FallTrigger, VAR_INT16, touchHitbox.left);
    RSDK_EDITABLE_VAR(FallTrigger, VAR_INT16, touchHitbox.top);
    RSDK_EDITABLE_VAR(FallTrigger, VAR_INT16, touchHitbox.right);
    RSDK_EDITABLE_VAR(FallTrigger, VAR_INT16, touchHitbox.bottom);
}

} // namespace GameLogic