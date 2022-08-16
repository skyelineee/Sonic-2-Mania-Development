// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SuperSparkle Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "SuperSparkle.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "Debris.hpp"
#include "Ring.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SuperSparkle);

void SuperSparkle::Update() {}
void SuperSparkle::LateUpdate()
{
    Player *player = GameObject::Get<Player>(this->parentSlot);

    this->state.Run(this);

    if (player->superState != Player::SuperStateSuper || player->active == ACTIVE_NEVER)
        this->Destroy();
}
void SuperSparkle::StaticUpdate() {}
void SuperSparkle::Draw() {}

void SuperSparkle::Create(void *data)
{
    if (sceneInfo->inEditor != 1) {
        this->active     = ACTIVE_NORMAL;
        this->parentSlot = ((GameObject::Entity *)data)->Slot();

        Player *player = GameObject::Get<Player>(this->parentSlot);
        if (player->classID == Player::sVars->classID) {
            this->state.Set(&SuperSparkle::State_SuperSparkle);
        }
    }
}

void SuperSparkle::StageLoad() { sVars->aniFrames.Load("Global/SuperSparkle.bin", SCOPE_STAGE); }

void SuperSparkle::State_SuperSparkle()
{
    SET_CURRENT_STATE();

    Player *player = GameObject::Get<Player>(this->parentSlot);

    if (!player) {
        this->Destroy();
    }
    else {
        if (player->groundedStore)
            this->canSpawnSparkle = abs(player->velocity.y) + abs(player->velocity.x) > (6 << 16);

        if (this->canSpawnSparkle) {
            if (++this->timer == 12) {
                this->timer = 0;

                Debris *sparkle   = GameObject::Create<Debris>(Debris::Move, player->position.x, player->position.y);
                sparkle->duration = 16;
                if (globals->useManiaBehavior)
                    sparkle->inkEffect = INK_ADD;
                sparkle->alpha     = 0x100;
                sparkle->drawGroup = Zone::sVars->objectDrawGroup[1];
                sparkle->drawGroup = player->drawGroup;
                sparkle->animator1.SetAnimation(sVars->aniFrames, 0, true, 0);
            }
        }
        else {
            this->timer = 0;
        }

        if (globals->useManiaBehavior && player->characterID == ID_SONIC && !(Zone::sVars->timer & 7)) {
            int32 x = player->position.x + Math::Rand(-(12 << 16), (12 << 16));
            int32 y = player->position.y + Math::Rand(-(18 << 16), (18 << 16));

            Ring *sparkle = GameObject::Create<Ring>(NULL, x, y);
            sparkle->state.Set(&Ring::State_Sparkle);
            sparkle->stateDraw.Set(&Ring::Draw_Sparkle);
            sparkle->active     = ACTIVE_NORMAL;
            sparkle->visible    = false;
            sparkle->velocity.y = -(1 << 16);
            sparkle->drawGroup  = player->drawGroup;
            sparkle->animator.SetAnimation(Ring::sVars->aniFrames, Zone::sVars->timer % 3 + 2, true, 0);
            int32 cnt = sparkle->animator.frameCount;
            if (sparkle->animator.animationID == 2) {
                sparkle->alpha = 0xE0;
                cnt >>= 1;
            }
            sparkle->maxFrameCount  = cnt - 1;
            sparkle->animator.speed = Math::Rand(6, 8);
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void SuperSparkle::EditorDraw() {}

void SuperSparkle::EditorLoad() { sVars->aniFrames.Load("Global/SuperSparkle.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void SuperSparkle::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(SuperSparkle);

    sVars->aniFrames.Init();
}
#endif

void SuperSparkle::Serialize() {}

} // namespace GameLogic