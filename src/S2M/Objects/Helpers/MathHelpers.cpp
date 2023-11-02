// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: MathHelpers Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "MathHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(MathHelpers);

void MathHelpers::LerpToPos(RSDK::Vector2 *pos, int32 percent, int32 posX, int32 posY)
{
    if (percent < 0) {
        pos->x = 0;
        pos->y = 0;
    }
    else if (percent >= 0x100) {
        pos->x = posX;
        pos->y = posY;
    }
    else {
        pos->x = percent * (posX >> 8);
        pos->y = percent * (posY >> 8);
    }
}
void MathHelpers::Lerp(RSDK::Vector2 *pos, int32 percent, int32 startX, int32 startY, int32 endX, int32 endY)
{
    if (percent < 0) {
        pos->x = startX;
        pos->y = startY;
    }
    else if (percent >= 0x100) {
        pos->x = endX;
        pos->y = endY;
    }
    else {
        pos->x = startX + percent * ((endX - startX) >> 8);
        pos->y = startY + percent * ((endY - startY) >> 8);
    }
}
void MathHelpers::LerpSin1024(RSDK::Vector2 *pos, int32 percent, int32 startX, int32 startY, int32 endX, int32 endY)
{
    if (percent < 0) {
        pos->x = startX;
        pos->y = startY;
    }
    else if (percent >= 0x100) {
        pos->x = endX;
        pos->y = endY;
    }
    else {
        int32 lerpPercent = (Math::Sin1024(percent + 0x300) >> 2) + 0x100;
        pos->x            = startX + lerpPercent * ((endX - startX) >> 8);
        pos->y            = startY + lerpPercent * ((endY - startY) >> 8);
    }
}
void MathHelpers::Lerp2Sin1024(RSDK::Vector2 *pos, int32 percent, int32 startX, int32 startY, int32 endX, int32 endY)
{
    if (percent < 0) {
        pos->x = startX;
        pos->y = startY;
    }
    else if (percent >= 0x100) {
        pos->x = endX;
        pos->y = endY;
    }
    else {
        int32 lerpPercent = Math::Sin1024(percent) >> 2;
        pos->x            = startX + lerpPercent * ((endX - startX) >> 8);
        pos->y            = startY + lerpPercent * ((endY - startY) >> 8);
    }
}
void MathHelpers::LerpSin512(RSDK::Vector2 *pos, int32 percent, int32 startX, int32 startY, int32 endX, int32 endY)
{
    if (percent < 0) {
        pos->x = startX;
        pos->y = startY;
    }
    else if (percent >= 0x100) {
        pos->x = endX;
        pos->y = endY;
    }
    else {
        int32 lerpPercent = (Math::Sin512(percent + 0x180) >> 2) + 0x80;
        pos->x            = startX + lerpPercent * ((endX - startX) >> 8);
        pos->y            = startY + lerpPercent * ((endY - startY) >> 8);
    }
}

// Bezier/Sqrt
RSDK::Vector2 MathHelpers::GetBezierPoint(int32 percent, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, int32 x4, int32 y4)
{
    int32 invPercent = 0x10000 - percent;
    int32 point1     = invPercent * ((uint32)(invPercent * invPercent) >> 16) >> 16;
    int32 point2     = percent * ((uint32)(invPercent * invPercent) >> 16) >> 16;
    int32 point3     = invPercent * ((uint32)(percent * percent) >> 16) >> 16;
    int32 point4     = percent * ((uint32)(percent * percent) >> 16) >> 16;

    Vector2 resultPos;
    resultPos.x =
        point4 * (x4 >> 16) + point3 * (x3 >> 16) + point2 * (x2 >> 16) + point1 * (x1 >> 16) + 2 * point2 * (x2 >> 16) + 2 * point3 * (x3 >> 16);
    resultPos.y =
        point4 * (y4 >> 16) + point3 * (y3 >> 16) + point2 * (y2 >> 16) + point1 * (y1 >> 16) + 2 * point2 * (y2 >> 16) + 2 * point3 * (y3 >> 16);
    return resultPos;
}
int32 MathHelpers::SquareRoot(uint32 num)
{
    int32 rem = 1 << 30; // 1 << 31 would result in the value having to be unsigned, so this is the max
    while (rem > num) rem >>= 2;

    uint32 root = 0;
    while (rem) {
        if (num >= rem + root) {
            num -= rem + root;
            root += rem << 1;
        }

        rem >>= 2;
        root >>= 1;
    }

    return num <= root ? root : (root + 1);
}
int32 MathHelpers::Distance(RSDK::Vector2 point1, RSDK::Vector2 point2)
{
    int32 distanceX = abs(point2.x - point1.x) >> 16;
    int32 distanceY = abs(point2.y - point1.y) >> 16;

    return SquareRoot((distanceX) * (distanceX) + (distanceY) * (distanceY)) << 16;
}
int32 MathHelpers::GetBezierCurveLength(int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, int32 x4, int32 y4)
{
    int32 lastX = x1;
    int32 lastY = y1;

    int32 length = 0;
    // 0x10000 = 1.0
    // 0xCCC == 0.05
    for (int32 percent = 0xCCC; percent <= 0x10000; percent += 0xCCC) {
        Vector2 point = GetBezierPoint(percent, x1, y1, x2, y2, x3, y3, x4, y4);

        int32 distX = abs(point.x - lastX);
        int32 distY = abs(point.y - lastY);
        length += SquareRoot((distX >> 16) * (distX >> 16) + (distY >> 16) * (distY >> 16)) << 16;
        lastX = point.x;
        lastY = point.y;
    }
    return length;
}

// "Collisions"
bool32 MathHelpers::PointInHitbox(int32 thisX, int32 thisY, int32 otherX, int32 otherY, int32 direction, RSDK::Hitbox *hitbox)
{
    int32 left, top, right, bottom;

    if ((direction & FLIP_X)) {
        left  = -hitbox->left;
        right = -hitbox->right;
    }
    else {
        right = hitbox->right;
        left  = hitbox->left;
    }

    if ((direction & FLIP_Y)) {
        bottom = -hitbox->bottom;
        top    = -hitbox->top;
    }
    else {
        bottom = hitbox->bottom;
        top    = hitbox->top;
    }
    int32 hitboxX2 = right;
    if (left < right)
        hitboxX2 = left;
    int32 hitboxX1 = left;
    if (right > left)
        hitboxX1 = right;

    int32 hitboxY1 = top;
    int32 hitboxY2 = bottom;
    if (top < bottom)
        hitboxY2 = top;
    if (bottom > top)
        hitboxY1 = bottom;
    return otherX >= thisX + (hitboxX2 << 16) && otherX <= thisX + (hitboxX1 << 16) && otherY >= thisY + (hitboxY2 << 16)
           && otherY <= thisY + (hitboxY1 << 16);
}
bool32 MathHelpers::PositionBoxesIntersect(int32 otherX1, int32 otherY1, int32 otherX2, int32 otherY2, int32 thisX1, int32 thisY1, int32 thisX2,
                                           int32 thisY2)
{
    int32 left_other   = MIN(otherX1, otherX2);
    int32 top_other    = MIN(otherY1, otherY2);
    int32 right_other  = MAX(otherX1, otherX2);
    int32 bottom_other = MAX(otherY1, otherY2);

    int32 left_this   = MIN(thisX1, thisX2);
    int32 top_this    = MIN(thisY1, thisY2);
    int32 right_this  = MAX(thisX1, thisX2);
    int32 bottom_this = MAX(thisY1, thisY2);

    return left_other <= right_this && right_other >= left_this && top_other <= bottom_this && bottom_other >= top_this;
}
int32 MathHelpers::GetInteractionDir(int32 otherX1, int32 otherY1, int32 otherX2, int32 otherY2, int32 thisX, int32 thisY)
{
    int32 dir = ((thisY - otherY1) >> 16) * ((otherX2 - otherX1) >> 16) - ((thisX - otherX1) >> 16) * ((otherY2 - otherY1) >> 16);
    return dir > 0 ? 1 : dir < 0 ? -1 : 0;
}
bool32 MathHelpers::CheckValidIntersect(int32 otherX1, int32 otherY1, int32 otherX2, int32 otherY2, int32 thisX, int32 thisY)
{
    if (otherX2 > otherX1) {
        if (thisX < otherX1 || thisX > otherX2)
            return false;
        return true;
    }

    if (otherX2 < otherX1) {
        if (thisX < otherX1 || thisX > otherX1)
            return false;
        return true;
    }

    if (otherY2 > otherY1) {
        if (thisY < otherY1 || thisY > otherY2)
            return false;
        return true;
    }

    if (otherY2 < otherY1) {
        if (thisY < otherY2 || thisY > otherY1)
            return false;
        return true;
    }

    if (thisX < otherX1 || thisY < otherY1)
        return false;
    return true;
}
int32 MathHelpers::CheckPositionOverlap(int32 otherX1, int32 otherY1, int32 otherX2, int32 otherY2, int32 thisX1, int32 thisY1, int32 thisX2,
                                        int32 thisY2)
{
    // Creates "hitboxes" from the positions and does a quick check to see if they overlap
    if (!PositionBoxesIntersect(otherX1, otherY1, otherX2, otherY2, thisX1, thisY1, thisX2, thisY2))
        return false;

    if (otherX1 == otherX2 && otherY1 == otherY2) {
        if (otherX1 != thisX1 || otherY1 != thisY1) {
            if (otherX1 == thisX2 && otherY1 == thisY2)
                return true;
            return false;
        }
        return true;
    }

    if (thisX1 == thisX2 && thisY1 == thisY2) {
        if (thisX1 == otherX1 && thisY1 == otherY1)
            return true;

        if (thisX1 == otherX2 && thisY1 == otherY2)
            return true;

        return false;
    }

    int32 thisInteractDir1 = GetInteractionDir(otherX1, otherY1, otherX2, otherY2, thisX1, thisY1);
    int32 thisInteractDir2 = GetInteractionDir(otherX1, otherY1, otherX2, otherY2, thisX2, thisY2);

    if (thisInteractDir1) {
        if (thisInteractDir1 == thisInteractDir2)
            return false;
    }
    else if (!thisInteractDir2) {
        if (CheckValidIntersect(otherX1, otherY1, otherX2, otherY2, thisX1, thisY1)
            || CheckValidIntersect(otherX1, otherY1, otherX2, otherY2, thisX2, thisY2)
            || CheckValidIntersect(thisX1, thisY1, thisX2, thisY2, otherX1, otherY1)
            || CheckValidIntersect(thisX1, thisY1, thisX2, thisY2, otherX2, otherY2)) {
            return true;
        }

        return false;
    }

    int32 otherInteractDir1 = GetInteractionDir(thisX1, thisY1, thisX2, thisY2, otherX1, otherY1);
    if (!otherInteractDir1)
        return true;

    int32 otherInteractDir2 = GetInteractionDir(thisX1, thisY1, thisX2, thisY2, otherX2, otherY2);
    if (otherInteractDir1 == otherInteractDir2)
        return false;

    return true;
}

int32 MathHelpers::GetEdgeDistance(int32 distance, int32 radius)
{
    uint32 dist = abs(distance);
    uint32 rad  = abs(radius);

    uint32 result1 = (dist >> 16) * (rad >> 16) << 16;
    uint32 result2 = (dist >> 16) * (rad & 0xFFFF);
    uint32 result3 = (dist & 0xFFFF) * (rad >> 16);
    uint32 result4 = (dist & 0xFFFF) * (rad & 0xFFFF) >> 16;

    uint32 edgeDistance = result1 + result2 + result3 + result4;
    if ((radius ^ ~distance) >= 0) // if the signs do not match
        return -(int32)edgeDistance;
    else
        return edgeDistance;
}
bool32 MathHelpers::ConstrainToBox(RSDK::Vector2 *pos, int32 x, int32 y, RSDK::Vector2 boxPos, RSDK::Hitbox hitbox)
{
    int32 left   = MIN(hitbox.left, hitbox.right);
    int32 right  = MAX(hitbox.right, hitbox.left);
    int32 top    = MIN(hitbox.top, hitbox.bottom);
    int32 bottom = MAX(hitbox.bottom, hitbox.top);

    int32 boxPosLeft   = boxPos.x + (left << 16);
    int32 boxPosTop    = boxPos.y + (top << 16);
    int32 boxPosRight  = boxPos.x + (right << 16);
    int32 boxPosBottom = boxPos.y + (bottom << 16);

    if (x > boxPosLeft && x < boxPosRight && y > boxPosTop && y < boxPosBottom)
        return false;

    // Check if they're on the same pixel on x axis (ignores subpixel positions)
    if (!((x ^ boxPos.x) & 0xFFFF0000)) {
        if (pos) {
            pos->x = x & 0xFFFF0000;
            if (y <= boxPos.y)
                pos->y = boxPosTop;
            else
                pos->y = boxPosBottom;
        }
        return true;
    }

    // Check if they're on the same pixel on y axis (ignores subpixel positions)
    if (!((y ^ boxPos.y) & 0xFFFF0000)) {
        if (pos) {
            if (x <= boxPos.x)
                pos->x = boxPosLeft;
            else
                pos->x = boxPosRight;
            pos->y = y & 0xFFFF0000;
        }
        return true;
    }

    double div   = 1.0f / 65536.0f;
    int32 radius = (int32)((((boxPos.y - y) * div) / ((boxPos.x - x) * div)) * 65536.0f);
    if (!radius)
        return false;

    int32 posY = 0;
    if (x <= boxPos.x) {
        posY = y + GetEdgeDistance(boxPosLeft - x, radius);
        if (boxPosTop <= posY && posY <= boxPosBottom) {
            if (pos) {
                pos->x = boxPosLeft;
                pos->y = posY;
            }
            return true;
        }
    }

    if (x >= boxPos.x) {
        posY = y + GetEdgeDistance(boxPosRight - x, radius);
        if (boxPosTop <= posY && posY <= boxPosBottom) {
            if (pos) {
                pos->x = boxPosRight;
                pos->y = posY;
            }
            return true;
        }
    }

    if (y <= boxPos.y) {
        radius = (int32)((((boxPosTop - y) * div) / (radius * div)) * -65536.0);
        if (boxPosLeft <= x - radius && x - radius <= boxPosRight) {
            if (pos) {
                pos->x = x - radius;
                pos->y = boxPosTop;
            }
            return true;
        }
    }

    if (y >= boxPos.y) {
        radius = (int32)(x - ((((boxPosBottom - y) * div) / (radius * div)) * -65536.0f));
        if (boxPosLeft <= radius && radius <= boxPosRight) {
            if (pos) {
                pos->x = radius;
                pos->y = boxPosBottom;
            }
            return true;
        }
    }

    return false;
}

#if RETRO_REV0U
void MathHelpers::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(MathHelpers); }
#endif

} // namespace GameLogic