// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: DrawHelpers Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(DrawHelpers);


void DrawHelpers::DrawHitboxOutline(int32 x, int32 y, RSDK::Hitbox *hitbox, uint8 direction, uint32 color)
{
    int16 left, top, right, bottom;

    if (direction & FLIP_X) {
        right = -hitbox->right;
        left  = -hitbox->left;
    }
    else {
        left  = hitbox->left;
        right = hitbox->right;
    }

    if (direction & FLIP_Y) {
        top    = -hitbox->top;
        bottom = -hitbox->bottom;
    }
    else {
        top    = hitbox->top;
        bottom = hitbox->bottom;
    }

    Graphics::DrawLine(x + (left << 16), y + (top << 16), x + (right << 16), y + (top << 16), color, 0xFF, INK_NONE, false);
    Graphics::DrawLine(x + (right << 16), y + (top << 16), x + (right << 16), y + (bottom << 16), color, 0xFF, INK_NONE, false);
    Graphics::DrawLine(x + (right << 16), y + (bottom << 16), x + (left << 16), y + (bottom << 16), color, 0xFF, INK_NONE, false);
    Graphics::DrawLine(x + (left << 16), y + (bottom << 16), x + (left << 16), y + (top << 16), color, 0xFF, INK_NONE, false);
}

void DrawHelpers::DrawPath(RSDK::GameObject::Entity *entity, RSDK::Hitbox *hitbox, uint32 color)
{
    int16 left = 0, top = 0, right = 0, bottom = 0;

    switch (entity->collisionMode) {
        case CMODE_FLOOR:
        case CMODE_ROOF:
            left   = hitbox->left;
            top    = hitbox->top;
            right  = hitbox->right;
            bottom = hitbox->bottom;
            break;

        case CMODE_LWALL:
        case CMODE_RWALL:
            left   = hitbox->top;
            top    = hitbox->left;
            right  = hitbox->bottom;
            bottom = hitbox->right;
            break;
    }

    if (entity->direction & FLIP_X) {
        int32 store = -left;
        left        = -right;
        right       = store;
    }

    if (entity->direction & FLIP_Y) {
        int32 store = -top;
        top         = -bottom;
        bottom      = store;
    }

    int32 x = entity->position.x;
    int32 y = entity->position.y;
    Graphics::DrawLine(x + (left << 16), y + (top << 16), x + (right << 16), y + (top << 16), color, 0xFF, INK_NONE, false);
    Graphics::DrawLine(x + (right << 16), y + (top << 16), x + (right << 16), y + (bottom << 16), color, 0xFF, INK_NONE, false);
    Graphics::DrawLine(x + (right << 16), y + (bottom << 16), x + (left << 16), y + (bottom << 16), color, 0xFF, INK_NONE, false);
    Graphics::DrawLine(x + (left << 16), y + (bottom << 16), x + (left << 16), y + (top << 16), color, 0xFF, INK_NONE, false);
}

void DrawHelpers::DrawArrowAdditive(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color)
{
    int32 angle = Math::ATan2(x1 - x2, y1 - y2);
    Graphics::DrawLine(x1, y1, x2, y2, color, 0x7F, INK_ADD, false);
    Graphics::DrawLine(x2, y2, x2 + (Math::Cos256(angle + 12) << 12), y2 + (Math::Sin256(angle + 12) << 12), color, 0x7F, INK_ADD, false);
    Graphics::DrawLine(x2, y2, x2 + (Math::Cos256(angle - 12) << 12), y2 + (Math::Sin256(angle - 12) << 12), color, 0x7F, INK_ADD, false);
}

void DrawHelpers::DrawLine(RSDK::Vector2 position, RSDK::Vector2 targetPos, color color)
{
    Graphics::DrawLine(position.x, position.y, targetPos.x, targetPos.y, color, 0x7F, INK_NONE, false);
}

void DrawHelpers::DrawCross(int32 x, int32 y, int32 sizeX, int32 sizeY, uint32 color)
{
    if (x || y) {
        Graphics::DrawLine(x - (sizeX >> 1), y - (sizeY >> 1), x + (sizeX >> 1), y + (sizeY >> 1), color, 0x7F, INK_NONE, false);
        Graphics::DrawLine(x + (sizeX >> 1), y - (sizeY >> 1), x - (sizeX >> 1), y + (sizeY >> 1), color, 0x7F, INK_NONE, false);
    }
}

void DrawHelpers::Load()
{
    if (sceneInfo->inEditor) {
        SpriteAnimation ani;
        ani.Load("Dev/Font.bin", SCOPE_STAGE);
    }
    else if (!sVars->aniFrames.Loaded()) {
        sVars->aniFrames.Load("Dev/Font.bin", SCOPE_STAGE);
    }
    bool32 wasEffected            = sceneInfo->effectGizmo;
    sceneInfo->effectGizmo = wasEffected != 0;
}

// Custom ones!!

// Adds alpha & ink effect in params for extra customizability
void DrawHelpers::DrawArrow(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color, uint32 inkEffect, uint32 alpha)
{
    int32 angle = Math::ATan2(x1 - x2, y1 - y2);
    Graphics::DrawLine(x1, y1, x2, y2, color, alpha, inkEffect, false);
    Graphics::DrawLine(x2, y2, x2 + (Math::Cos256(angle + 12) << 12), y2 + (Math::Sin256(angle + 12) << 12), color, alpha, inkEffect, false);
    Graphics::DrawLine(x2, y2, x2 + (Math::Cos256(angle - 12) << 12), y2 + (Math::Sin256(angle - 12) << 12), color, alpha, inkEffect, false);
}

void DrawHelpers::DrawRectOutline(int32 x, int32 y, int32 sizeX, int32 sizeY, uint32 color)
{
    Vector2 drawPos;

    drawPos.x = x - (sizeX >> 1);
    drawPos.y = y - (sizeY >> 1);
    Graphics::DrawLine(drawPos.x - 0x10000, drawPos.y - 0x10000, drawPos.x + sizeX, drawPos.y - 0x10000, color, 0x00, INK_NONE, false);
    Graphics::DrawLine(drawPos.x - 0x10000, drawPos.y + sizeY, drawPos.x + sizeX, drawPos.y + sizeY, color, 0x00, INK_NONE, false);
    Graphics::DrawLine(drawPos.x - 0x10000, drawPos.y - 0x10000, drawPos.x - 0x10000, drawPos.y + sizeY, color, 0x00, INK_NONE, false);
    Graphics::DrawLine(drawPos.x + sizeX, drawPos.y - 0x10000, drawPos.x + sizeX, drawPos.y + sizeY, color, 0x00, INK_NONE, false);
}

void DrawHelpers::DrawArenaBounds(RSDK::GameObject::Entity *self, int32 left, int32 top, int32 right, int32 bottom, uint8 sideMasks, uint32 color)
{
    left <<= 16;
    top <<= 16;
    right <<= 16;
    bottom <<= 16;

    // left
    if (sideMasks & 1) {
        Graphics::DrawLine(self->position.x + left, self->position.y + top, self->position.x + left, self->position.y + bottom, color, 0, INK_NONE, false);
    }

    // top
    if (sideMasks & 2) {
        Graphics::DrawLine(self->position.x + left, self->position.y + top, self->position.x + right, self->position.y + top, color, 0, INK_NONE, false);
    }

    // right
    if (sideMasks & 4) {
        Graphics::DrawLine(self->position.x + right, self->position.y + top, self->position.x + right, self->position.y + bottom, color, 0, INK_NONE,
                      false);
    }

    // bottom
    if (sideMasks & 8) {
        Graphics::DrawLine(self->position.x + left, self->position.y + bottom, self->position.x + right, self->position.y + bottom, color, 0, INK_NONE,
                      false);
    }
}

#if RETRO_REV0U
void DrawHelpers::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(DrawHelpers);

    sVars->aniFrames.Init();
}
#endif

} // namespace GameLogic
