// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: CNZSpotlight Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "CNZSpotlight.hpp"
#include "Global/Zone.hpp"
#include "Global/PlaneSwitch.hpp"
#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(CNZSpotlight);

void CNZSpotlight::Update()
{
    this->angle    = (this->angle + this->speed) & 0x1FF;
    this->rotation = Math::Sin512(this->angle) >> 2;

    Vector2 *vertices = this->spotlightVertices;
    for (int32 i = 0; i < 8; ++i) {
        this->drawVertices[i].x = vertices[i].x;
        this->drawVertices[i].y = vertices[i].y;

        int32 distX             = (this->drawVertices[i].x - this->originPos.x) >> 4;
        int32 distY             = (this->drawVertices[i].y - this->originPos.y) >> 4;
        this->drawVertices[i].x = this->originPos.x + (distY * Math::Sin1024(this->rotation) >> 6) + (distX * Math::Cos1024(this->rotation) >> 6);
        this->drawVertices[i].y = this->originPos.y + (distY * Math::Cos1024(this->rotation) >> 6) - (distX * Math::Sin1024(this->rotation) >> 6);
    }

    if (this->flashSpeed)
        this->alpha = (Math::Sin256((Zone::sVars->timer + this->offset) << this->flashSpeed) >> 2) + 0xC0;
}

void CNZSpotlight::LateUpdate() {}

void CNZSpotlight::StaticUpdate() {}

void CNZSpotlight::Draw()
{
    Vector2 *drawVertex = this->drawVertices;
    int32 screenX       = this->position.x - (screenInfo[sceneInfo->currentScreenID].position.x << 16);

    for (int32 i = 0; i < 6; i += 2) {
        Vector2 vertices[4];
        color colors[4];

        vertices[0].x = screenX + drawVertex[i + 0].x;
        vertices[0].y = drawVertex[i + 0].y;
        colors[0]     = this->colorTable[i];

        vertices[1].x = screenX + drawVertex[i + 1].x;
        vertices[1].y = drawVertex[i + 1].y;
        colors[1]     = this->colorTable[i + 1];

        vertices[2].x = screenX + drawVertex[i + 3].x;
        vertices[2].y = drawVertex[i + 3].y;
        colors[2]     = this->colorTable[i + 3];

        vertices[3].x = screenX + drawVertex[i + 2].x;
        vertices[3].y = drawVertex[i + 2].y;
        colors[3]     = this->colorTable[i + 2];

        Graphics::DrawBlendedFace(vertices, colors, 4, this->alpha, INK_ADD);
    }
}

void CNZSpotlight::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->visible = true;

        switch (this->drawFlag) {
            default: break;

            case CNZSPOTLIGHT_DRAW_LOW: this->drawGroup = Zone::sVars->objectDrawGroup[0]; break;
            case CNZSPOTLIGHT_DRAW_HIGH: this->drawGroup = Zone::sVars->objectDrawGroup[1]; break;
            case CNZSPOTLIGHT_DRAW_LOWER: this->drawGroup = Zone::sVars->objectDrawGroup[0] - 1; break;
        }

        this->angle = this->offset;

        int32 sizes[3] = { 4, 8, 16 };

        this->alpha       = 0x100;
        this->originPos.y = 272 << 16;

        this->spotlightVertices[0].x = -(sizes[this->size] << 16);
        this->spotlightVertices[0].y = -(192 << 16);

        this->spotlightVertices[1].x = -(sizes[this->size] << 16);
        this->spotlightVertices[1].y = 256 << 16;

        this->spotlightVertices[2].x = -0x4000 * sizes[this->size];
        this->spotlightVertices[2].y = -(192 << 16);

        this->spotlightVertices[3].x = -0x4000 * sizes[this->size];
        this->spotlightVertices[3].y = 256 << 16;

        this->spotlightVertices[4].x = sizes[this->size] << 14;
        this->spotlightVertices[4].y = -(192 << 16);

        this->spotlightVertices[5].x = sizes[this->size] << 14;
        this->spotlightVertices[5].y = 256 << 16;

        this->spotlightVertices[6].x = sizes[this->size] << 16;
        this->spotlightVertices[6].y = -(192 << 16);

        this->spotlightVertices[7].x = sizes[this->size] << 16;
        this->spotlightVertices[7].y = 256 << 16;

        Vector2 *vertex = this->spotlightVertices;
        for (int32 i = 0; i < 8; i += 2) {
            vertex->x = 720 * (vertex->x >> 8);

            vertex += 2;
        }

        this->colorTable = this->spotlightColor != CNZSPOTLIGHT_CLR_CYAN ? sVars->pinkSpotlightColors : sVars->cyanSpotlightColors;

        this->active        = ACTIVE_XBOUNDS;
        this->updateRange.x = 0x1000000;
        this->updateRange.y = 0x800000;
    }
}

void CNZSpotlight::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void CNZSpotlight::EditorDraw()
{
    this->updateRange.x = 0x1000000;
    this->updateRange.y = 0x800000;
    this->animator.SetAnimation(PlaneSwitch::sVars->aniFrames, 0, true, 4);

    this->animator.DrawSprite(nullptr, false);

    RSDK_DRAWING_OVERLAY(true);
    uint32 color[] = { 0x40D080, 0xE850D8 };
    this->rotation = Math::Sin512(this->offset & 0x1FF) >> 2;

    uint32 sizes[] = { 0xC00, 0x1200, 0x1800 };

    Vector2 drawPos;
    drawPos.x = (sizes[this->size] * Math::Cos1024(this->rotation - 0x100)) + this->position.x;
    drawPos.y = (sizes[this->size] * Math::Sin1024(this->rotation - 0x100)) + this->position.y;

    DrawHelpers::DrawArrow(this->position.x, this->position.y, drawPos.x, drawPos.y, color[this->spotlightColor], INK_NONE, 0xFF);

    RSDK_DRAWING_OVERLAY(false);
}

void CNZSpotlight::EditorLoad()
{
    sVars->aniFrames.Load("Global/PlaneSwitch.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, spotlightColor);
    RSDK_ENUM_VAR("Cyan");
    RSDK_ENUM_VAR("Pink");

    RSDK_ACTIVE_VAR(sVars, size);
    RSDK_ENUM_VAR("Small");
    RSDK_ENUM_VAR("Medium");
    RSDK_ENUM_VAR("Large");

    RSDK_ACTIVE_VAR(sVars, drawFlag);
    RSDK_ENUM_VAR("On Object Group (Low)");
    RSDK_ENUM_VAR("On Object Group (High)");
    RSDK_ENUM_VAR("Behind Object Group (Low)");
}
#endif

void CNZSpotlight::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(CNZSpotlight);

    int32 cyanSpotlightColors[] = { 0x000020, 0x000020, 0x40D080, 0x40D080, 0x40D080, 0x40D080, 0x000020, 0x000020 };

    memcpy(sVars->cyanSpotlightColors, cyanSpotlightColors, sizeof(cyanSpotlightColors));

    int32 pinkSpotlightColors[] = { 0x000020, 0x000020, 0xE850D8, 0xE850D8, 0xE850D8, 0xE850D8, 0x000020, 0x000020 };

    memcpy(sVars->pinkSpotlightColors, pinkSpotlightColors, sizeof(pinkSpotlightColors));

}

void CNZSpotlight::Serialize()
{
    RSDK_EDITABLE_VAR(CNZSpotlight, VAR_UINT8, spotlightColor);
    RSDK_EDITABLE_VAR(CNZSpotlight, VAR_UINT8, size);
    RSDK_EDITABLE_VAR(CNZSpotlight, VAR_UINT8, drawFlag);
    RSDK_EDITABLE_VAR(CNZSpotlight, VAR_UINT8, speed);
    RSDK_EDITABLE_VAR(CNZSpotlight, VAR_UINT8, offset);
    RSDK_EDITABLE_VAR(CNZSpotlight, VAR_UINT8, flashSpeed);
}

} // namespace GameLogic