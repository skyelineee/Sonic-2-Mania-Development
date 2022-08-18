// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: FXTileModifier Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "FXTileModifier.hpp"
#include "Global/Zone.hpp"
#include "Global/DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(FXTileModifier);

void FXTileModifier::Update()
{
    this->position.x &= 0xFFF00000;
    this->position.y &= 0xFFF00000;
    this->visible = DebugMode::sVars->debugActive;

    if (this->activated) {
        this->active = ACTIVE_NORMAL;
        this->timer--;

        if (this->timer <= 0) {
            int32 rangeX = this->copyRange.x >> 16;
            int32 rangeY = this->copyRange.y >> 16;

            char nameBuffer[32];
            if (this->dst1.length) {
                this->dst1.CStr(nameBuffer);
                if (this->src1.length) {
                    SceneLayer dstLayer;
                    dstLayer.Get(nameBuffer);

                    this->src1.CStr(nameBuffer);
                    SceneLayer srcLayer;
                    srcLayer.Get(nameBuffer);

                    int32 dstX = (this->position.x & 0xFFF80000) >> 20;
                    int32 dstY = (this->position.y & 0xFFF80000) >> 20;

                    int32 srcX = (this->position.x & 0xFFF80000) >> 20;
                    int32 srcY = (this->position.y & 0xFFF80000) >> 20;

                    if (this->copyOrigin.x >= 0)
                        srcX = this->copyOrigin.x >> 16;

                    if (this->copyOrigin.y >= 0)
                        srcY = this->copyOrigin.y >> 16;

                    SceneLayer::Copy(dstLayer, dstX, dstY, srcLayer, srcX, srcY, rangeX, rangeY);
                }
                else {
                    SceneLayer dstLayer;
                    dstLayer.Get(nameBuffer);

                    int32 dstX = (this->position.x & 0xFFF80000) >> 20;
                    int32 dstY = (this->position.y & 0xFFF80000) >> 20;
                    for (int32 y = 0; y < rangeY; ++y) {
                        for (int32 x = 0; x < rangeX; ++x) {
                            dstLayer.SetTile(dstX + x, dstY + y, -1);
                        }
                    }
                }
            }

            if (this->dst2.length) {
                this->dst2.CStr(nameBuffer);
                if (this->src2.length) {
                    SceneLayer dstLayer;
                    dstLayer.Get(nameBuffer);

                    this->src2.CStr(nameBuffer);
                    SceneLayer srcLayer;
                    srcLayer.Get(nameBuffer);

                    int32 dstX = (this->position.x & 0xFFF80000) >> 20;
                    int32 dstY = (this->position.y & 0xFFF80000) >> 20;

                    int32 srcX = (this->position.x & 0xFFF80000) >> 20;
                    int32 srcY = (this->position.y & 0xFFF80000) >> 20;

                    if (this->copyOrigin.x >= 0)
                        srcX = this->copyOrigin.x >> 16;

                    if (this->copyOrigin.y >= 0)
                        srcY = this->copyOrigin.y >> 16;

                    SceneLayer::Copy(dstLayer, dstX, dstY, srcLayer, srcX, srcY, rangeX, rangeY);
                }
                else {
                    SceneLayer dstLayer;
                    dstLayer.Get(nameBuffer);

                    int32 dstX = (this->position.x & 0xFFF80000) >> 20;
                    int32 dstY = (this->position.y & 0xFFF80000) >> 20;
                    for (int32 y = 0; y < rangeY; ++y) {
                        for (int32 x = 0; x < rangeX; ++x) {
                            dstLayer.SetTile(dstX + x, dstY + y, -1);
                        }
                    }
                }
            }

            if (this->timer <= 0) {
                this->copyOrigin.x += this->activationAddOffset.x & 0xFFFF0000;
                this->copyOrigin.y += this->activationAddOffset.y & 0xFFFF0000;

                this->timer = this->activationTime;
                this->activationCount--;
            }

            if (this->activationCount <= 0)
                this->Destroy();
        }
    }
}
void FXTileModifier::LateUpdate() {}
void FXTileModifier::StaticUpdate() {}
void FXTileModifier::Draw()
{
    Animator animator;

    this->position.x &= 0xFFF00000;
    this->position.y &= 0xFFF00000;

    if (this->src1.length)
        animator.SetAnimation(sVars->aniFrames, 0, true, 13);
    else
        animator.SetAnimation(sVars->aniFrames, 0, true, 14);
    animator.DrawSprite(&this->position, false);

    if (this->src2.length)
        animator.SetAnimation(sVars->aniFrames, 0, true, 15);
    else
        animator.SetAnimation(sVars->aniFrames, 0, true, 16);
    animator.DrawSprite(&this->position, false);

    int32 rangeX = 16 * this->copyRange.x;
    int32 rangeY = 16 * this->copyRange.y;
    Graphics::DrawLine(this->position.x, this->position.y, this->position.x + rangeX, this->position.y, 0x00FF00, 0x00, INK_NONE, false);
    Graphics::DrawLine(this->position.x + rangeX, this->position.y, this->position.x + rangeX, this->position.y + rangeY, 0x00FF00, 0x00, INK_NONE,
                       false);
    Graphics::DrawLine(this->position.x + rangeX, this->position.y + rangeY, this->position.x, this->position.y + rangeY, 0x00FF00, 0x00, INK_NONE,
                       false);
    Graphics::DrawLine(this->position.x, this->position.y + rangeY, this->position.x, this->position.y, 0x00FF00, 0x00, INK_NONE, false);
}

void FXTileModifier::Create(void *data)
{
    if (sceneInfo->inEditor) {
        this->updateRange.x = 0x800000;
        this->updateRange.y = 0x800000;
    }
    else {
        this->drawGroup = Zone::sVars->hudDrawGroup;
        this->position.x &= 0xFFF80000;
        this->position.y &= 0xFFF80000;
        if (this->updateRange.x < 0 || this->updateRange.y < 0)
            this->active = ACTIVE_NORMAL;
        else
            this->active = ACTIVE_BOUNDS;
    }
}

void FXTileModifier::StageLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }

FXTileModifier *FXTileModifier::GetNearest(RSDK::GameObject::Entity *self)
{
    if (!self)
        return nullptr;

    int32 targetDistance           = 0x7FFFFFFF;
    FXTileModifier *targetModifier = nullptr;
    for (auto modifier : GameObject::GetEntities<FXTileModifier>(FOR_ACTIVE_ENTITIES)) {
        int32 distX = abs(modifier->position.x - self->position.x);
        int32 distY = abs(modifier->position.y - self->position.y);
        if (distX + distY < targetDistance) {
            targetDistance = distX + distY;
            targetModifier = modifier;
        }
    }

    return targetModifier;
}

void FXTileModifier::Find(uint8 type)
{
    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];
    Vector2 position;
    position.x = screen->size.x << 16;
    position.y = screen->size.y << 16;

    int32 targetDistance           = 0x7FFFFFFF;
    FXTileModifier *targetModifier = nullptr;
    switch (type) {
        case 0:
            for (auto modifier : GameObject::GetEntities<FXTileModifier>(FOR_ACTIVE_ENTITIES)) {
                modifier->activated = true;
            }
            break;

        case 1:
            for (auto modifier : GameObject::GetEntities<FXTileModifier>(FOR_ACTIVE_ENTITIES)) {
                if (modifier->activated)
                    modifier->activated = false;
                else
                    modifier->activated = true;
            }
            break;

        case 2:
            for (auto modifier : GameObject::GetEntities<FXTileModifier>(FOR_ACTIVE_ENTITIES)) {
                if (modifier->active == ACTIVE_NORMAL || modifier->CheckOnScreen(&modifier->updateRange))
                    modifier->activated = true;
                else
                    modifier->activated = true;
            }
            break;

        case 3:
            for (auto modifier : GameObject::GetEntities<FXTileModifier>(FOR_ACTIVE_ENTITIES)) {
                if (!modifier->activated)
                    modifier->activated = false;
                else
                    modifier->active = ACTIVE_NORMAL;
            }
            break;

        case 4:
            for (auto modifier : GameObject::GetEntities<FXTileModifier>(FOR_ACTIVE_ENTITIES)) {
                if (modifier->CheckOnScreen(&position))
                    modifier->activated = true;
            }
            break;

        case 5:
            for (auto modifier : GameObject::GetEntities<FXTileModifier>(FOR_ACTIVE_ENTITIES)) {
                if (modifier->CheckOnScreen(&position))
                    modifier->activated = true;
            }
            break;

        case 6:
            targetModifier = GetNearest(sVars->targetModifier);

            if (targetModifier) {
                targetModifier->active    = ACTIVE_NORMAL;
                targetModifier->activated = true;
            }
            break;

        case 7:
            for (auto modifier : GameObject::GetEntities<FXTileModifier>(FOR_ACTIVE_ENTITIES)) {
                int32 distX = abs(modifier->position.x - sVars->targetModifier->position.x);
                if (distX < targetDistance) {
                    targetDistance = distX;
                    targetModifier = modifier;
                }
            }

            if (targetModifier) {
                targetModifier->active    = ACTIVE_NORMAL;
                targetModifier->activated = true;
            }
            break;

        case 8:
            for (auto modifier : GameObject::GetEntities<FXTileModifier>(FOR_ACTIVE_ENTITIES)) {
                int32 distY = abs(modifier->position.y - sVars->targetModifier->position.y);
                if (distY < targetDistance) {
                    targetDistance = distY;
                    targetModifier = modifier;
                }
            }

            if (targetModifier) {
                targetModifier->active    = ACTIVE_NORMAL;
                targetModifier->activated = true;
            }
            break;

        default: break;
    }
}

#if RETRO_INCLUDE_EDITOR
void FXTileModifier::EditorDraw()
{
    Animator animator;

    this->position.x &= 0xFFF00000;
    this->position.y &= 0xFFF00000;

    if (this->src1.length)
        animator.SetAnimation(sVars->aniFrames, 0, true, 13);
    else
        animator.SetAnimation(sVars->aniFrames, 0, true, 14);
    animator.DrawSprite(&this->position, false);

    if (this->src2.length)
        animator.SetAnimation(sVars->aniFrames, 0, true, 15);
    else
        animator.SetAnimation(sVars->aniFrames, 0, true, 16);
    animator.DrawSprite(&this->position, false);

    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        int32 rangeX = 16 * this->copyRange.x;
        int32 rangeY = 16 * this->copyRange.y;
        Graphics::DrawLine(this->position.x, this->position.y, this->position.x + rangeX, this->position.y, 0x00FF00, 0x00, INK_NONE, false);
        Graphics::DrawLine(this->position.x + rangeX, this->position.y, this->position.x + rangeX, this->position.y + rangeY, 0x00FF00, 0x00,
                           INK_NONE, false);
        Graphics::DrawLine(this->position.x + rangeX, this->position.y + rangeY, this->position.x, this->position.y + rangeY, 0x00FF00, 0x00,
                           INK_NONE, false);
        Graphics::DrawLine(this->position.x, this->position.y + rangeY, this->position.x, this->position.y, 0x00FF00, 0x00, INK_NONE, false);

        RSDK_DRAWING_OVERLAY(false);
    }
}

void FXTileModifier::EditorLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void FXTileModifier::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(FXTileModifier);

    sVars->aniFrames.Init();
}
#endif

void FXTileModifier::Serialize()
{
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_STRING, src1);
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_STRING, dst1);
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_STRING, src2);
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_STRING, dst2);
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_VECTOR2, copyOrigin);
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_VECTOR2, copyRange);
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_VECTOR2, updateRange);
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_INT32, activationTime);
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_INT32, activationCount);
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_VECTOR2, activationAddOffset);
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_INT32, parameter);
    RSDK_EDITABLE_VAR(FXTileModifier, VAR_BOOL, activated);
}

} // namespace GameLogic