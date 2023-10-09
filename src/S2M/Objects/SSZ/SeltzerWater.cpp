// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SeltzerWater Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "SeltzerWater.hpp"
#include "Global/Zone.hpp"
#include "Common/PlatformNode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SeltzerWater);

void SeltzerWater::Update()
{
    this->state.Run(this);

    if (this->scale.x < 0x200) {
        this->scale.x += 0x10;
        this->scale.y = this->scale.x;

        if (this->scale.x == 0x200)
            this->drawFX = FX_NONE;
    }
}

void SeltzerWater::LateUpdate() {}

void SeltzerWater::StaticUpdate() {}

void SeltzerWater::Draw() { this->animator.DrawSprite(nullptr, false); }

void SeltzerWater::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->animator.SetAnimation(sVars->aniFrames, 4, true, Math::Rand(0, 8));

        this->active        = ACTIVE_NORMAL;
        this->updateRange.x = 0x100000;
        this->updateRange.y = 0x100000;
        this->visible       = true;
        this->drawFX        = FX_SCALE;
        this->scale.x       = 0x80;
        this->scale.y       = 0x80;
        this->offsetAngle   = Math::Rand(0, 256);
        this->state.Set(&SeltzerWater::State_Sprayed);
    }
}

void SeltzerWater::StageLoad() { sVars->aniFrames.Load("SSZ/Seltzer.bin", SCOPE_STAGE); }

void SeltzerWater::State_Sprayed()
{
    this->animator.Process();

    PlatformNode *node = GameObject::Get<PlatformNode>(this->nodeSlot);
    if (node->classID == PlatformNode::sVars->classID) {
        int32 x = (this->position.x - node->position.x) >> 16;
        int32 y = (this->position.y - node->position.y) >> 16;

        this->angle      = Math::ATan2(x, y);
        this->velocity.x = -(this->oscillateRadius * Math::Cos256(this->angle));
        this->velocity.y = (Math::Sin256(this->offsetAngle) << 4) - this->oscillateRadius * Math::Sin256(this->angle);
        this->position.x += this->velocity.x;
        this->position.y += this->velocity.y;

        if (x * x + y * y < 0x50)
            ++this->nodeSlot;

        this->offsetAngle++;
    }
    else {
        this->state.Set(&SeltzerWater::State_Falling);
        this->gravityStrength = 0x3800;
    }
}

void SeltzerWater::State_Falling()
{
    this->animator.Process();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += this->gravityStrength;

    if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x60000, true)) {
        this->animator.SetAnimation(sVars->aniFrames, 5, true, 0);
        this->state.Set(&SeltzerWater::State_Splash);
    }
    else {
        if (!this->CheckOnScreen(nullptr))
            this->Destroy();
    }
}

void SeltzerWater::State_Splash()
{
    this->animator.Process();

    if (this->animator.frameID == this->animator.frameCount - 1)
        this->Destroy();
}

#if RETRO_INCLUDE_EDITOR
void SeltzerWater::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 4, true, 3);
    SeltzerWater::Draw();
}

void SeltzerWater::EditorLoad() { sVars->aniFrames.Load("SSZ/Seltzer.bin", SCOPE_STAGE); }
#endif

void SeltzerWater::Serialize() {}

} // namespace GameLogic