// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: PlatformControl Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "PlatformControl.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(PlatformControl);

void PlatformControl::Update()
{
    this->active = ACTIVE_NORMAL;

    int32 startNodeSlot = this->Slot() + 1;
    int32 platformSlot  = startNodeSlot + this->nodeCount;

    if (this->isActive) {
        for (int32 c = 0; c < this->childCount; ++c) {
            Platform *platform = GameObject::Get<Platform>(platformSlot);
            PlatformNode *node = GameObject::Get<PlatformNode>(platform->speed);

            int32 finishDir = 0;
            if (platform->velocity.x <= 0) {
                if (platform->drawPos.x <= node->position.x)
                    finishDir = 1;
            }
            else {
                if (platform->drawPos.x >= node->position.x)
                    finishDir = 1;
            }

            if (platform->velocity.y <= 0) {
                if (platform->drawPos.y <= node->position.y)
                    finishDir |= 2;
            }
            else {
                if (platform->drawPos.y >= node->position.y)
                    finishDir |= 2;
            }

            if (finishDir == (1 | 2)) {
                platform->timer = node->nodeFlag;

                if (platform->direction < 4) {
                    int32 nodeID = ++platform->speed - startNodeSlot;
                    if (nodeID >= this->nodeCount) {
                        switch (this->type) {
                            case Circuit: platform->speed = startNodeSlot; break;

                            case Reverse:
                                --platform->speed;
                                platform->direction = platform->direction ^ 4;
                                break;

                            case Teleport: {
                                PlatformNode *startNode = GameObject::Get<PlatformNode>(startNodeSlot);
                                platform->drawPos.x     = startNode->position.x;
                                platform->drawPos.y     = startNode->position.y;
                                platform->speed         = startNodeSlot + 1;
                                break;
                            }

                            case Stop:
                                this->speed      = 0;
                                this->hasStopped = true;
                                break;
                        }
                    }
                }
                else {
                    int32 nodeID = --platform->speed - startNodeSlot;
                    if (nodeID < 0) {
                        switch (this->type) {
                            default:
                            case Stop: break;

                            case Circuit: platform->speed = startNodeSlot + (this->nodeCount - 1); break;

                            case Reverse:
                                platform->direction = platform->direction ^ 4;
                                platform->speed     = platform->speed + 1;
                                break;

                            case Teleport: {
                                PlatformNode *startNode = GameObject::Get<PlatformNode>(startNodeSlot + this->nodeCount - 1);
                                platform->drawPos.x     = startNode->position.x;
                                platform->drawPos.y     = startNode->position.y;
                                platform->speed         = startNodeSlot + (this->nodeCount - 2);
                                break;
                            }
                        }
                    }
                }

                ManagePlatformVelocity(platform, GameObject::Get<PlatformNode>(platform->speed));
            }

            platformSlot += platform->childCount + 1;
        }
    }
    else {
        Button *button = this->taggedButton;

        if (button && button->currentlyActive)
            this->setActive = true;

        if (this->setActive) {
            for (int32 c = 0; c < this->childCount; ++c) {
                Platform *platform = GameObject::Get<Platform>(platformSlot);
                if (platform->state.Matches(&Platform::State_PathStop))
                    platform->state.Set(&Platform::State_Path);

                if (platform->state.Matches(&Platform::State_PathReact)) {
                    this->setActive = false;
                    return;
                }

                platform->speed += startNodeSlot;
                platform->active = ACTIVE_NORMAL;
                ManagePlatformVelocity(platform, GameObject::Get<PlatformNode>(platform->speed));
                platformSlot += platform->childCount + 1;
            }

            this->isActive = true;
        }
    }

    if (this->resetOffScreen) {
        if (this->taggedButton && !this->CheckOnScreen(nullptr) && !this->playingPulleySfx) {
            this->active = ACTIVE_BOUNDS;

            for (int32 c = 0; c < this->childCount; ++c) {
                Platform *platform = GameObject::Get<Platform>(platformSlot);
                if (platform->state.Matches(&Platform::State_Path)) {
                    platform->state.Set(&Platform::State_PathStop);
                    platform->speed -= startNodeSlot;
                    platform->active = ACTIVE_BOUNDS;
                }

                PlatformNode *node = GameObject::Get<PlatformNode>(startNodeSlot);
                platform->drawPos  = node->position;
                platform->speed    = 0;

                platformSlot += platform->childCount + 1;
            }

            this->hasStopped = false;
            this->isActive   = false;
        }
    }
    else {
        if (!this->CheckOnScreen(nullptr) && !this->playingPulleySfx) {
            this->active = ACTIVE_BOUNDS;

            int32 slot = startNodeSlot + this->nodeCount;
            for (int32 c = 0; c < this->childCount; ++c) {
                Platform *platform = GameObject::Get<Platform>(slot);
                if (platform->state.Matches(&Platform::State_Path)) {
                    platform->speed -= startNodeSlot;
                    platform->state.Set(&Platform::State_PathStop);
                    platform->active = ACTIVE_BOUNDS;
                }
                slot += platform->childCount + 1;
            }

            this->isActive = false;
        }
    }
}
void PlatformControl::LateUpdate() {}
void PlatformControl::StaticUpdate() {}
void PlatformControl::Draw()
{
    Animator animator;
    animator.SetAnimation(sVars->aniFrames, 0, true, 19);
    animator.DrawSprite(nullptr, false);
}

void PlatformControl::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active = ACTIVE_BOUNDS;

        int32 id = this->Slot() + 1;
        for (int32 i = 0; i < this->nodeCount; ++i) {
            PlatformNode *node = GameObject::Get<PlatformNode>(id++);

            if (abs(node->position.x - this->position.x) > this->updateRange.x)
                this->updateRange.x = abs(node->position.x - this->position.x);

            if (abs(node->position.y - this->position.y) > this->updateRange.y)
                this->updateRange.y = abs(node->position.y - this->position.y);
        }

        if (!this->speed)
            this->speed = 4;

        this->updateRange.x += 0x800000;
        this->updateRange.y += 0x800000;

        this->taggedButton = nullptr;

        Button *taggedButton = GameObject::Get<Button>(this->Slot() - 1);
        if (this->buttonTag > 0) {
            bool32 foundButton = false;
            if (Button::sVars) {
                for (auto button : GameObject::GetEntities<Button>(FOR_ALL_ENTITIES))
                {
                    if (button->tag == this->buttonTag) {
                        taggedButton = button;
                        foundButton  = true;
                        break;
                    }
                }
            }
        }
        
        if (taggedButton) {
            if ((Button::sVars && taggedButton->classID == Button::sVars->classID)) {
                this->taggedButton = taggedButton;
                if (this->updateRange.y < 0x800000 + abs(this->position.x - taggedButton->position.x)) {
                    this->updateRange.y = 0x800000 + abs(this->position.x - taggedButton->position.x);
                }
                if (this->updateRange.y < 0x800000 + abs(this->position.y - taggedButton->position.y)) {
                    this->updateRange.y = 0x800000 + abs(this->position.y - taggedButton->position.y);
                }
            }
        }
    }
}

void PlatformControl::StageLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }

void PlatformControl::ManagePlatformVelocity(Platform *platform, PlatformNode *node)
{
    int32 distX = abs((node->position.x - platform->drawPos.x) >> 16);
    int32 distY = abs((node->position.y - platform->drawPos.y) >> 16);
    
    if (distY >= distX) {
        if (distY)
            platform->velocity.x = this->speed * (((distX << 16) / distY) >> 2);
        else
            platform->velocity.x = 0;
    
        platform->velocity.y = this->speed << 14;
    }
    else {
        platform->velocity.x = this->speed << 14;
    
        if (distX)
            platform->velocity.y = this->speed * (((distY << 16) / distX) >> 2);
        else
            platform->velocity.y = 0;
    }
    
    if (node->position.x < platform->drawPos.x)
        platform->velocity.x = -platform->velocity.x;
    if (node->position.y < platform->drawPos.y)
        platform->velocity.y = -platform->velocity.y;
}

#if RETRO_INCLUDE_EDITOR
void PlatformControl::EditorDraw()
{
    Animator animator;
    animator.SetAnimation(sVars->aniFrames, 0, true, 19);
    animator.DrawSprite(nullptr, false);
}

void PlatformControl::EditorLoad()
{
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Circuit");
    RSDK_ENUM_VAR("Reverse");
    RSDK_ENUM_VAR("Teleport");
    RSDK_ENUM_VAR("Stop");
}
#endif

#if RETRO_REV0U
void PlatformControl::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(PlatformControl); }
#endif

void PlatformControl::Serialize()
{
    RSDK_EDITABLE_VAR(PlatformControl, VAR_ENUM, nodeCount);
    RSDK_EDITABLE_VAR(PlatformControl, VAR_ENUM, childCount);
    RSDK_EDITABLE_VAR(PlatformControl, VAR_ENUM, speed);
    RSDK_EDITABLE_VAR(PlatformControl, VAR_BOOL, setActive);
    RSDK_EDITABLE_VAR(PlatformControl, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(PlatformControl, VAR_ENUM, buttonTag);
    RSDK_EDITABLE_VAR(PlatformControl, VAR_BOOL, resetOffScreen);
}

} // namespace GameLogic