// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Announcer Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Announcer.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Announcer);

void Announcer::Update() { this->state.Run(this); }
void Announcer::LateUpdate() {}
void Announcer::StaticUpdate() {}
void Announcer::Draw() { this->animator.DrawSprite(&this->position, false); }

void Announcer::Create(void *data)
{
    this->active        = ACTIVE_NORMAL;
    this->updateRange.x = 0x800000;
    this->updateRange.x = 0x800000;
    this->visible       = true;
    this->drawGroup     = 1;

    if (sceneInfo->inEditor)
        this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
    else
        this->state.Set(&Announcer::State_Init);
}

void Announcer::StageLoad() { sVars->aniFrames.Load("Announcer.bin", SCOPE_STAGE); }

void Announcer::State_Init()
{
    SET_CURRENT_STATE();

    this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
    this->state.Set(&Announcer::State_Idle);
}

void Announcer::State_Idle() { SET_CURRENT_STATE(); }

#if RETRO_INCLUDE_EDITOR
void Announcer::EditorDraw() { this->animator.DrawSprite(&this->position, false); }

void Announcer::EditorLoad() { sVars->aniFrames.Load("Announcer.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void Announcer::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Announcer);

    sVars->aniFrames.Init();
}
#endif

void Announcer::Serialize() {}

} // namespace GameLogic