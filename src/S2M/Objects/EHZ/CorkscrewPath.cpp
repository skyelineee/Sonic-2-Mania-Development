// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: CorkscrewPath Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "CorkscrewPath.hpp"
#include "Global/Player.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(CorkscrewPath);

void CorkscrewPath::Update()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        int32 playerID = RSDKTable->GetEntitySlot(player);

        if (abs(this->position.x - player->position.x) >> 16 > this->xSize) {
            this->activePlayers &= ~playerID;
            if (player->animator.animationID != Player::ANI_SPRING_CS)
                player->direction &= ~FLIP_Y;
        }
        else {
            int32 corkscrewPos = this->xSize + ((player->position.x - this->position.x) >> 16);
            int32 frame        = 24 * corkscrewPos / this->period;
            int32 yOffset      = this->amplitude * Math::Cos1024((corkscrewPos << 10) / this->period);

            if (!(playerID & this->activePlayers)) {
                if (abs(yOffset + this->position.y - player->position.y) >= 0x100000) {
                    this->activePlayers &= ~playerID;
                }
                else if (abs(player->groundVel) > 0x40000 && player->velocity.y > -0x40000 && player->groundedStore) {
                    player->position.y = this->position.y + yOffset;
                    player->velocity.y = 0;
                    player->onGround   = true;

                    if (player->animator.animationID != Player::ANI_JUMP) {
                        if (player->groundVel < 0) {
                            player->direction |= FLIP_Y;
                            player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, true, sVars->frameTable[frame]);
                        }
                        else {
                            player->direction &= ~FLIP_Y;
                            player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, true, frame);
                        }
                    }
                }
                else {
                    this->activePlayers &= ~playerID;
                }
            }
            else if (abs(player->groundVel) > 0x40000 && player->groundedStore && abs(yOffset + player->position.y - player->position.y) < 0x100000) {
                this->activePlayers |= playerID;
                player->position.y = yOffset + this->position.y;
                player->velocity.y = 0;
                player->onGround   = true;

                if (player->animator.animationID != Player::ANI_JUMP) {
                    if (player->groundVel < 0) {
                        player->direction |= FLIP_Y;
                        player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, true, sVars->frameTable[frame]);
                    }
                    else {
                        player->direction &= ~FLIP_Y;
                        player->animator.SetAnimation(player->aniFrames, Player::ANI_SPRING_CS, true, frame);
                    }
                }
            }
        }
    }
}

void CorkscrewPath::LateUpdate() {}

void CorkscrewPath::StaticUpdate() {}

void CorkscrewPath::Draw() {}

void CorkscrewPath::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->amplitude <<= 6;
        this->period        = abs(this->period);
        this->xSize         = this->period >> 1;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = abs(this->period) << 15;
        this->updateRange.y = this->amplitude << 3;
    }
}

void CorkscrewPath::StageLoad() {}

void CorkscrewPath::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(CorkscrewPath);

    int32 frameTable[] = { 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13 };

    memcpy(sVars->frameTable, frameTable, sizeof(frameTable));
}

#if RETRO_INCLUDE_EDITOR
void CorkscrewPath::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 4);
    this->animator.DrawSprite(nullptr, false);

    // Bounds

    Vector2 size;
    size.x = abs(this->period) << 15;
    size.y = (this->amplitude << 6) * Math::Cos1024(0);

    DrawHelpers::DrawRectOutline(this->position.x, this->position.y, size.x << 1, size.y << 1, 0xFFFF00);
}

void CorkscrewPath::EditorLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }
#endif

void CorkscrewPath::Serialize()
{
    RSDK_EDITABLE_VAR(CorkscrewPath, VAR_ENUM, period);
    RSDK_EDITABLE_VAR(CorkscrewPath, VAR_ENUM, amplitude);
    RSDK_EDITABLE_VAR(CorkscrewPath, VAR_ENUM, angle);
}

} // namespace GameLogic