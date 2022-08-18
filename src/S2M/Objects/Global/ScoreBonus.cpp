// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ScoreBonus Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "ScoreBonus.hpp"
#include "Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ScoreBonus);

void ScoreBonus::Update()
{
    this->position.y -= 0x20000;

    if (!--this->timer)
        this->Destroy();
}
void ScoreBonus::LateUpdate() {}
void ScoreBonus::StaticUpdate() {}
void ScoreBonus::Draw() { this->animator.DrawSprite(&this->position, false); }

void ScoreBonus::Create(void *data)
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);

    if (!sceneInfo->inEditor) {
        this->active     = ACTIVE_NORMAL;
        this->visible    = true;
        this->drawGroup  = Zone::sVars->objectDrawGroup[0];
        this->velocity.y = -0x30000;
        this->timer      = 24;
    }
}

void ScoreBonus::StageLoad() { sVars->aniFrames.Load("Global/ScoreBonus.bin", SCOPE_STAGE); }

#if RETRO_INCLUDE_EDITOR
void ScoreBonus::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->animator.DrawSprite(&this->position, false);
}

void ScoreBonus::EditorLoad() { sVars->aniFrames.Load("Global/ScoreBonus.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void ScoreBonus::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(ScoreBonus);

    sVars->aniFrames.Init();
}
#endif

void ScoreBonus::Serialize() {}

} // namespace GameLogic