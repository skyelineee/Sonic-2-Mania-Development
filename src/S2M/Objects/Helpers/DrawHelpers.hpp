#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct DrawHelpers : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    // ==============================
    // EVENTS
    // ==============================

#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif

    // ==============================
    // FUNCTIONS
    // ==============================
    static void DrawHitboxOutline(int32 x, int32 y, RSDK::Hitbox *hitbox, uint8 direction, uint32 color);
    static void DrawPath(RSDK::GameObject::Entity *entity, RSDK::Hitbox *hitbox, uint32 color);
    static void DrawArrowAdditive(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color);
    static void DrawLine(RSDK::Vector2 position, RSDK::Vector2 targetPos, color color);
    static void DrawCross(int32 x, int32 y, int32 sizeX, int32 sizeY, uint32 color);
    static void Load();

    // Custom ones!!
    static void DrawArrow(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color, uint32 inkEffect, uint32 alpha);
    static void DrawRectOutline(int32 x, int32 y, int32 sizeX, int32 sizeY, uint32 color);
    static void DrawArenaBounds(RSDK::GameObject::Entity *self, int32 left, int32 top, int32 right, int32 bottom, uint8 sideMasks, uint32 color);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(DrawHelpers);
};
} // namespace GameLogic