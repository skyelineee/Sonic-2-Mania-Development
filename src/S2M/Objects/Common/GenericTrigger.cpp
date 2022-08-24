// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: GenericTrigger Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "GenericTrigger.hpp"

#include "Global/Player.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(GenericTrigger);

void GenericTrigger::Update()
{
    if (this->interaction) {
        this->visible   = DebugMode::sVars->debugActive;
        this->triggered = false;

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            if ((!player->sidekick || this->allPlayers) && player->CheckCollisionTouch(this, &this->hitbox)) {
                this->triggered = true;
                sVars->playerID = player->playerID;
                sVars->trigger  = this;
                sVars->callbacks[this->triggerID].Run(this);
            }
        }

        if (this->triggered) {
            this->activationLimit--;
            if (this->activationLimit <= 0) {
                if (!this->activationLimit)
                    this->Destroy();
            }
        }
    }
}
void GenericTrigger::LateUpdate() {}
void GenericTrigger::StaticUpdate() {}
void GenericTrigger::Draw()
{
    DrawHelpers::DrawRectOutline(this->position.x, this->position.y, this->size.x, this->size.y, 0x00FFFF);

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 17);
    this->animator.DrawSprite(nullptr, false);
}

void GenericTrigger::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->triggerID &= 0xF;

        this->updateRange.x = this->size.x;
        this->updateRange.y = this->size.y;
        this->active        = ACTIVE_BOUNDS;
        this->drawGroup     = Zone::sVars->hudDrawGroup;

        this->hitbox.left   = -(this->size.x >> 16);
        this->hitbox.top    = -(this->size.y >> 16);
        this->hitbox.right  = (this->size.x >> 16);
        this->hitbox.bottom = (this->size.y >> 16);
    }
}

void GenericTrigger::StageLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }

#if RETRO_INCLUDE_EDITOR
void GenericTrigger::EditorDraw()
{
    DrawHelpers::DrawRectOutline(this->position.x, this->position.y, this->size.x, this->size.y, 0x00FFFF);

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 17);
    this->animator.DrawSprite(nullptr, false);
}

void GenericTrigger::EditorLoad()
{
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);

    // if (EHZSetup::sVars) {
    //     // RSDK_ACTIVE_VAR(sVars, triggerID);
    //     // RSDK_ENUM_VAR("your variable goes here");
    // }
    // else if (CPZSetup::sVars) {
    //     // RSDK_ACTIVE_VAR(sVars, triggerID);
    //     // RSDK_ENUM_VAR("your variable goes here");
    // }
    // and so on
}
#endif

#if RETRO_REV0U
void GenericTrigger::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(GenericTrigger); }
#endif

void GenericTrigger::Serialize()
{
    RSDK_EDITABLE_VAR(GenericTrigger, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(GenericTrigger, VAR_UINT8, triggerID);
    RSDK_EDITABLE_VAR(GenericTrigger, VAR_INT32, activationLimit);
    RSDK_EDITABLE_VAR(GenericTrigger, VAR_INT32, parameter);
    RSDK_EDITABLE_VAR(GenericTrigger, VAR_BOOL, allPlayers);
}

} // namespace GameLogic