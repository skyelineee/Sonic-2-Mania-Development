// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TilePlatform Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "TilePlatform.hpp"
#include "Global/Zone.hpp"
#include "Global/ItemBox.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TilePlatform);

void TilePlatform::Update() { ((Platform *)this)->Update(); }
void TilePlatform::LateUpdate() {}
void TilePlatform::StaticUpdate() {}
void TilePlatform::Draw() { Graphics::DrawTile(this->tiles, this->size.x >> 20, this->size.y >> 20, &this->drawPos, nullptr, false); }

void TilePlatform::Create(void *data)
{
    this->collision = Platform::C_Tiled;

    ((Platform *)this)->Create(nullptr);

    if (!sceneInfo->inEditor) {
        this->updateRange.x += 2 * this->size.x;
        this->updateRange.y += 2 * this->size.y;

        this->hitbox.left   = -(this->size.x >> 17);
        this->hitbox.top    = -(this->size.y >> 17);
        this->hitbox.right  = this->size.x >> 17;
        this->hitbox.bottom = this->size.y >> 17;

        this->size.x += 0x80000;
        this->size.y += 0x80000;
        int32 originX = (this->targetPos.x >> 20) - (this->size.x >> 21);
        int32 originY = (this->targetPos.y >> 20) - (this->size.y >> 21);

        int32 h = this->size.y >> 20;
        int32 w = this->size.x >> 20;

        int32 id = 0;
        if ((this->size.y & 0xFFF00000) > 0) {
            for (int32 y = 0; y < h; ++y) {
                if ((this->size.x & 0xFFF00000) > 0) {
                    for (int32 x = 0; x < w; ++x) {
                        id              = x + y * (this->size.x >> 20);
                        this->tiles[id] = sVars->moveLayer.GetTile(originX + x, originY + y);
                    }
                }
            }
        }
    }
}

void TilePlatform::StageLoad() { sVars->moveLayer = Zone::sVars->moveLayer; }

#if RETRO_INCLUDE_EDITOR
void TilePlatform::EditorDraw()
{
    Vector2 drawPos;

    drawPos.x = this->position.x - (this->size.x >> 17 << 16) - 0x100000;
    drawPos.y = this->position.y - (this->size.y >> 17 << 16) - 0x100000;
    sVars->animator.DrawSprite(&drawPos, false);

    drawPos.y = this->position.y + (this->size.y >> 17 << 16) + 0x100000;
    sVars->animator.DrawSprite(&drawPos, false);

    drawPos.x = this->position.x + (this->size.x >> 17 << 16) + 0x100000;
    sVars->animator.DrawSprite(&drawPos, false);

    drawPos.y = this->position.y - (this->size.y >> 17 << 16) - 0x100000;
    sVars->animator.DrawSprite(&drawPos, false);

    Hitbox hitbox;
    hitbox.left   = -(this->size.x >> 17);
    hitbox.top    = -(this->size.y >> 17);
    hitbox.right  = this->size.x >> 17;
    hitbox.bottom = this->size.y >> 17;

    DrawHelpers::DrawPath(this, &hitbox, 0xE0E0E0);
}

void TilePlatform::EditorLoad()
{
    sVars->animator.SetAnimation(ItemBox::sVars->aniFrames, 2, true, 0);
    sVars->animator.frameID = 10;

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Fixed");
    RSDK_ENUM_VAR("Fall");
    RSDK_ENUM_VAR("Linear");
    RSDK_ENUM_VAR("Circular");
    RSDK_ENUM_VAR("Swing");
    RSDK_ENUM_VAR("Path");
    RSDK_ENUM_VAR("Push");
}
#endif

#if RETRO_REV0U
void TilePlatform::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(TilePlatform);

    sVars->aniFrames.Init();
}
#endif

void TilePlatform::Serialize()
{
    RSDK_EDITABLE_VAR(TilePlatform, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(TilePlatform, VAR_VECTOR2, amplitude);
    RSDK_EDITABLE_VAR(TilePlatform, VAR_ENUM, speed);
    RSDK_EDITABLE_VAR(TilePlatform, VAR_BOOL, hasTension);
    RSDK_EDITABLE_VAR(TilePlatform, VAR_VECTOR2, targetPos);
    RSDK_EDITABLE_VAR(TilePlatform, VAR_ENUM, childCount);
    RSDK_EDITABLE_VAR(TilePlatform, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(TilePlatform, VAR_INT32, angle);
}

} // namespace GameLogic