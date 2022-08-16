// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ReplayRecorder Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "ReplayRecorder.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ReplayRecorder);

void ReplayRecorder::Update() { this->state.Run(this); }
void ReplayRecorder::LateUpdate() {}
void ReplayRecorder::StaticUpdate() {}
void ReplayRecorder::Draw() { this->animator.DrawSprite(&this->position, false); }

void ReplayRecorder::Create(void *data)
{
    this->active        = ACTIVE_NORMAL;
    this->updateRange.x = 0x800000;
    this->updateRange.x = 0x800000;
    this->visible       = true;
    this->drawGroup     = 1;

    if (sceneInfo->inEditor)
        this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
    else
        this->state.Set(&ReplayRecorder::State_Init);
}

void ReplayRecorder::StageLoad() { sVars->aniFrames.Load("ReplayRecorder.bin", SCOPE_STAGE); }

void ReplayRecorder::State_Init()
{
    SET_CURRENT_STATE();

    this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
    this->state.Set(&ReplayRecorder::State_Idle);
}

void ReplayRecorder::State_Idle() { SET_CURRENT_STATE(); }

#if RETRO_INCLUDE_EDITOR
void ReplayRecorder::EditorDraw() { this->animator.DrawSprite(&this->position, false); }

void ReplayRecorder::EditorLoad() { sVars->aniFrames.Load("ReplayRecorder.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void ReplayRecorder::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(ReplayRecorder);

    sVars->aniFrames.Init();
}
#endif

void ReplayRecorder::Serialize() {}

} // namespace GameLogic