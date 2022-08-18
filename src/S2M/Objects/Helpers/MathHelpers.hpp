#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct MathHelpers : RSDK::GameObject::Entity {

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
    static void LerpToPos(RSDK::Vector2 *pos, int32 percent, int32 posX, int32 posY);
    static void Lerp(RSDK::Vector2 *pos, int32 percent, int32 startX, int32 startY, int32 endX, int32 endY);
    static void LerpSin1024(RSDK::Vector2 *pos, int32 percent, int32 startX, int32 startY, int32 endX, int32 endY);
    static void Lerp2Sin1024(RSDK::Vector2 *pos, int32 percent, int32 startX, int32 startY, int32 endX, int32 endY);
    static void LerpSin512(RSDK::Vector2 *pos, int32 percent, int32 startX, int32 startY, int32 endX, int32 endY);

    // Bezier/Sqrt
    static RSDK::Vector2 GetBezierPoint(int32 percent, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, int32 x4, int32 y4);
    static int32 SquareRoot(uint32 num);
    static int32 Distance(RSDK::Vector2 point1, RSDK::Vector2 point2);
    static int32 GetBezierCurveLength(int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, int32 x4, int32 y4);

    // "Collisions"
    static bool32 PointInHitbox(int32 thisX, int32 thisY, int32 otherX, int32 otherY, int32 direction, RSDK::Hitbox *hitbox);
    static bool32 PositionBoxesIntersect(int32 otherX1, int32 otherY1, int32 otherX2, int32 otherY2, int32 thisX1, int32 thisY1, int32 thisX2,
                                              int32 thisY2);
    static int32 GetInteractionDir(int32 otherX1, int32 otherY1, int32 otherX2, int32 otherY2, int32 thisX, int32 thisY);
    static bool32 CheckValidIntersect(int32 otherX1, int32 otherY1, int32 otherX2, int32 otherY2, int32 thisX, int32 thisY);
    static int32 CheckPositionOverlap(int32 otherX1, int32 otherY1, int32 otherX2, int32 otherY2, int32 thisX1, int32 thisY1, int32 thisX2,
                                           int32 thisY2);

    static int32 GetEdgeDistance(int32 distance, int32 radius);
    static bool32 ConstrainToBox(RSDK::Vector2 *pos, int32 x, int32 y, RSDK::Vector2 boxPos, RSDK::Hitbox hitbox);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(MathHelpers);
};
} // namespace GameLogic