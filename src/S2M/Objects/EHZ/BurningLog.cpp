// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: BurningLog Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "BurningLog.hpp"
#include "Global/Player.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(BurningLog);

void BurningLog::Update()
{
    this->animator.Process();

    if (this->CheckOnScreen(&this->updateRange)) {
        this->position.y += this->velocity.y;
        this->velocity.y += 0x3800;
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            if (this->velocity.y < 0x380000)
                player->CheckCollisionPlatform(this, &sVars->hitboxPlatform);

            if (player->CheckCollisionTouch(this, &sVars->hitboxFlame)) {
                player->FireHurt(this);
            }
        }
    }
    else {
        this->Destroy();
    }
}

void BurningLog::LateUpdate() {}

void BurningLog::StaticUpdate() {}

void BurningLog::Draw()
{
    this->animator.DrawSprite(nullptr, false);
}

void BurningLog::Create(void *data)
{
    this->active        = ACTIVE_NORMAL;
    this->visible       = true;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x10000000;
    this->drawGroup     = Zone::sVars->objectDrawGroup[0];

    if (data)
        this->timer = VOID_TO_INT(data);

    this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
}

void BurningLog::StageLoad()
{
    if (Stage::CheckSceneFolder("EHZ"))
        sVars->aniFrames.Load("EHZ/Fireball.bin", SCOPE_STAGE);

    sVars->hitboxPlatform.left   = -8;
    sVars->hitboxPlatform.top    = -8;
    sVars->hitboxPlatform.right  = 8;
    sVars->hitboxPlatform.bottom = 8;

    sVars->hitboxFlame.left   = -8;
    sVars->hitboxFlame.top    = -16;
    sVars->hitboxFlame.right  = 8;
    sVars->hitboxFlame.bottom = 8;
}

#if RETRO_INCLUDE_EDITOR
void BurningLog::EditorDraw() { BurningLog::Draw(); }

void BurningLog::EditorLoad() { sVars->aniFrames.Load("EHZ/Fireball.bin", SCOPE_STAGE); }
#endif

void BurningLog::Serialize() { RSDK_EDITABLE_VAR(BurningLog, VAR_ENUM, timer); }
} // namespace GameLogic