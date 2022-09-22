// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Halfpipe Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Halfpipe.hpp"
#include "HP_Setup.hpp"
#include "HP_Player.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Halfpipe);

void HP_Halfpipe::Update()
{
    sVars->scene3D.vertexCount = sVars->prevVertexCount;
    sVars->scene3D.faceCount   = sVars->faceCount;
}
void HP_Halfpipe::LateUpdate() {}
void HP_Halfpipe::StaticUpdate() {}
void HP_Halfpipe::Draw()
{
    sVars->scene3D.matWorld.TranslateXYZ(this->halfpipeTranslation.x << 8, this->halfpipeTranslation.y << 8, this->halfpipeTranslation.z << 8, true);
    sVars->scene3D.matView.RotateXYZ(this->halfpipeRotation.x, this->halfpipeRotation.y, this->halfpipeRotation.z);
    sVars->scene3D.matView.Inverse();
    sVars->scene3D.matTemp.RotateXYZ(-8, 0, 0);
    sVars->scene3D.matView *= sVars->scene3D.matTemp;

    HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
    int32 x            = (player1->position.x * -0x40) >> 9;
    int32 y            = ((player1->position.y * -0x60) >> 9) - 0x200;

    sVars->scene3D.matTemp.TranslateXYZ(x, y, 0, true);
    sVars->scene3D.matView *= sVars->scene3D.matTemp;

    // Draw3DScene();
}

void HP_Halfpipe::Create(void *data)
{
    this->active  = ACTIVE_NORMAL;
    this->visible = true;

    this->position.x              = TO_FIXED(448);
    this->position.y              = TO_FIXED(0);
    this->playerZPos              = 0;
    this->stageZPos               = 0;
    this->moveStep                = 0;
    this->transformIDHalfpipe     = 0;
    this->nextTransformIDHalfpipe = 1;
    this->transformIDWorld        = 2;
    this->nextTransformIDWorld    = 3;
    this->moveSpeed               = 17;
}

void HP_Halfpipe::StageLoad()
{
    sVars->vertexCount        = 0;
    sVars->tranformPos        = 0;
    sVars->prevVertexCount    = 0x1000;
    sVars->tempTranslate.x    = 0;
    sVars->tempTranslate.y    = 0;
    sVars->tempTranslate.z    = 0;
    sVars->worldRotate.x      = 0;
    sVars->worldRotate.y      = 0;
    sVars->worldRotate.z      = 0;
    sVars->checkpointTexCount = 0;

    // Destroy any halfpipe entities placed in the scene
    for (auto zone : GameObject::GetEntities<HP_Halfpipe>(FOR_ALL_ENTITIES)) zone->Destroy();
    // ... and ensure we have a halfipie entity in the correct reserved slot
    GameObject::Reset(SLOT_HP_HALFPIPE, sVars->classID, nullptr);

    int32 vertexTableID = 0;
    for (int32 i = 0; i < 4; ++i) {
        SetupFaces(vertexTableID++, sVars->faceTable1);
        SetupFaces(vertexTableID++, sVars->faceTable2);
        SetupFaces(vertexTableID++, sVars->faceTable2);
        SetupFaces(vertexTableID++, sVars->faceTable2);
        SetupFaces(vertexTableID++, sVars->faceTable3);
        SetupFaces(vertexTableID++, sVars->faceTable4);
        SetupFaces(vertexTableID++, sVars->faceTable5);
        SetupFaces(vertexTableID++, sVars->faceTable6);
    }

    sVars->scene3D.vertexCount = sVars->vertexCount;
    sVars->prevVertexCount     = sVars->vertexCount;
    sVars->faceCount           = sVars->scene3D.faceCount;
    sVars->vertexCount         = 0;
    sVars->scene3D.projectionX = 216;
    sVars->scene3D.projectionY = 216;
    sVars->scene3D.fogStrength = 0x50;
    sVars->scene3D.fogColor    = 0x000000;
}

void HP_Halfpipe::SetupFaces(int32 vertexTableID, int32 *faceTable)
{ 
    /*TODO*/
    SetupVertices(faceTable);
}
void HP_Halfpipe::SetupVertices(int32 *faceTable) { /*TODO*/ }

void HP_Halfpipe::TransformVertexBuffer()
{
    memcpy(&sVars->scene3D.matFinal, &sVars->scene3D.matWorld, sizeof(RSDK::Matrix));
    sVars->scene3D.matFinal *= sVars->scene3D.matView;

    RSDK::Matrix *m = &sVars->scene3D.matFinal;
    for (int v = 0; v < sVars->scene3D.vertexCount; ++v) {
        int32 vx = sVars->scene3D.vertexBuffer[v].x;
        int32 vy = sVars->scene3D.vertexBuffer[v].y;
        int32 vz = sVars->scene3D.vertexBuffer[v].z;

        sVars->scene3D.vertexBufferT[v].x = (vx * m->values[0][0] >> 8) + (vy * m->values[1][0] >> 8) + (vz * m->values[2][0] >> 8) + m->values[3][0];
        sVars->scene3D.vertexBufferT[v].y = (vx * m->values[0][1] >> 8) + (vy * m->values[1][1] >> 8) + (vz * m->values[2][1] >> 8) + m->values[3][1];
        sVars->scene3D.vertexBufferT[v].z = (vx * m->values[0][2] >> 8) + (vy * m->values[1][2] >> 8) + (vz * m->values[2][2] >> 8) + m->values[3][2];
    }
}

void HP_Halfpipe::SortDrawList(int32 first, int32 last)
{
    if (first < last) {
        int32 i = first;
        int32 j = last;

        int32 index = sVars->scene3D.drawList[i].index;
        int32 depth = sVars->scene3D.drawList[i].depth;

        while (i < j) {
            while (sVars->scene3D.drawList[j].depth <= depth && i < j) j--;
            sVars->scene3D.drawList[i].index = sVars->scene3D.drawList[j].index;
            sVars->scene3D.drawList[i].depth = sVars->scene3D.drawList[j].depth;

            while (sVars->scene3D.drawList[i].depth >= depth && i < j) i++;
            sVars->scene3D.drawList[j].index = sVars->scene3D.drawList[i].index;
            sVars->scene3D.drawList[j].depth = sVars->scene3D.drawList[i].depth;
        }
        sVars->scene3D.drawList[i].index = index;
        sVars->scene3D.drawList[i].depth = depth;

        SortDrawList(first, i - 1);
        SortDrawList(j + 1, last);
    }
}
void HP_Halfpipe::Draw3DScene()
{
    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    TransformVertexBuffer();

    Vertex *vertices = sVars->scene3D.vertexBufferT;
    Vertex *vertexBuffer = sVars->scene3D.vertexBuffer;

    for (int32 i = 0; i < sVars->scene3D.faceCount; ++i) {
        Face *face       = &sVars->scene3D.faceBuffer[sVars->scene3D.drawList[i].index];

        sVars->scene3D.drawList[i].depth = (vertices[face->d].z + vertices[face->c].z + vertices[face->b].z + vertices[face->a].z) >> 2;
        sVars->scene3D.drawList[i].index = i;
    }

    SortDrawList(0, sVars->scene3D.faceCount - 1);

    RSDK::Vector2 faceVerts[4];
    RSDK::Vector2 faceUVs[4];
    for (int32 i = 0; i < sVars->scene3D.faceCount; ++i) {
        Face *face = &sVars->scene3D.faceBuffer[sVars->scene3D.drawList[i].index];
        memset(faceVerts, 0, 4 * sizeof(Vertex));

        switch (face->flag) {
            default: break;
            case FACE_FLAG_TEXTURED_3D:
                if (vertices[face->a].z > 0 && vertices[face->b].z > 0 && vertices[face->c].z > 0 && vertices[face->d].z > 0) {
                    faceVerts[0].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->a].x / vertices[face->a].z;
                    faceVerts[0].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->a].y / vertices[face->a].z;
                    faceVerts[1].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->b].x / vertices[face->b].z;
                    faceVerts[1].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->b].y / vertices[face->b].z;
                    faceVerts[2].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->c].x / vertices[face->c].z;
                    faceVerts[2].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->c].y / vertices[face->c].z;
                    faceVerts[3].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->d].x / vertices[face->d].z;
                    faceVerts[3].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->d].y / vertices[face->d].z;

                    faceUVs[0].x = vertexBuffer[face->a].u;
                    faceUVs[0].y = vertexBuffer[face->a].v;
                    faceUVs[1].x = vertexBuffer[face->b].u;
                    faceUVs[1].y = vertexBuffer[face->b].v;
                    faceUVs[2].x = vertexBuffer[face->c].u;
                    faceUVs[2].y = vertexBuffer[face->c].v;
                    faceUVs[3].x = vertexBuffer[face->d].u;
                    faceUVs[3].y = vertexBuffer[face->d].v;

                    // DrawTexturedFace(quad, spriteSheetID);
                }
                break;
            case FACE_FLAG_TEXTURED_2D:
                if (vertices[face->a].z >= 0 && vertices[face->b].z >= 0 && vertices[face->c].z >= 0
                    && vertices[face->d].z >= 0) {
                    faceVerts[0].x = vertices[face->a].x;
                    faceVerts[0].y = vertices[face->a].y;
                    faceVerts[1].x = vertices[face->b].x;
                    faceVerts[1].y = vertices[face->b].y;
                    faceVerts[2].x = vertices[face->c].x;
                    faceVerts[2].y = vertices[face->c].y;
                    faceVerts[3].x = vertices[face->d].x;
                    faceVerts[3].y = vertices[face->d].y;

                    faceUVs[0].x = vertexBuffer[face->a].u;
                    faceUVs[0].y = vertexBuffer[face->a].v;
                    faceUVs[1].x = vertexBuffer[face->b].u;
                    faceUVs[1].y = vertexBuffer[face->b].v;
                    faceUVs[2].x = vertexBuffer[face->c].u;
                    faceUVs[2].y = vertexBuffer[face->c].v;
                    faceUVs[3].x = vertexBuffer[face->d].u;
                    faceUVs[3].y = vertexBuffer[face->d].v;

                    // DrawTexturedFace(quad, spriteSheetID);
                }
                break;
            case FACE_FLAG_COLORED_3D:
                if (vertices[face->a].z > 0 && vertices[face->b].z > 0 && vertices[face->c].z > 0 && vertices[face->d].z > 0) {
                    faceVerts[0].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->a].x / vertices[face->a].z;
                    faceVerts[0].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->a].y / vertices[face->a].z;
                    faceVerts[1].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->b].x / vertices[face->b].z;
                    faceVerts[1].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->b].y / vertices[face->b].z;
                    faceVerts[2].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->c].x / vertices[face->c].z;
                    faceVerts[2].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->c].y / vertices[face->c].z;
                    faceVerts[3].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->d].x / vertices[face->d].z;
                    faceVerts[3].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->d].y / vertices[face->d].z;

                    Graphics::DrawFace(faceVerts, 4, (face->color >> 16) & 0xFF, (face->color >> 8) & 0xFF, (face->color >> 0) & 0xFF,
                                       (face->color >> 24) & 0xFF, INK_ALPHA);
                }
                break;
            case FACE_FLAG_COLORED_2D:
                if (vertices[face->a].z >= 0 && vertices[face->b].z >= 0 && vertices[face->c].z >= 0
                    && vertices[face->d].z >= 0) {
                    faceVerts[0].x = vertices[face->a].x;
                    faceVerts[0].y = vertices[face->a].y;
                    faceVerts[1].x = vertices[face->b].x;
                    faceVerts[1].y = vertices[face->b].y;
                    faceVerts[2].x = vertices[face->c].x;
                    faceVerts[2].y = vertices[face->c].y;
                    faceVerts[3].x = vertices[face->d].x;
                    faceVerts[3].y = vertices[face->d].y;

                    Graphics::DrawFace(faceVerts, 4, (face->color >> 16) & 0xFF, (face->color >> 8) & 0xFF, (face->color >> 0) & 0xFF,
                                       (face->color >> 24) & 0xFF, INK_ALPHA);
                }
                break;
            case FACE_FLAG_FADED:
                if (vertices[face->a].z > 0 && vertices[face->b].z > 0 && vertices[face->c].z > 0 && vertices[face->d].z > 0) {
                    faceVerts[0].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->a].x / vertices[face->a].z;
                    faceVerts[0].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->a].y / vertices[face->a].z;
                    faceVerts[1].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->b].x / vertices[face->b].z;
                    faceVerts[1].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->b].y / vertices[face->b].z;
                    faceVerts[2].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->c].x / vertices[face->c].z;
                    faceVerts[2].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->c].y / vertices[face->c].z;
                    faceVerts[3].x = screen->center.x + sVars->scene3D.projectionX * vertices[face->d].x / vertices[face->d].z;
                    faceVerts[3].y = screen->center.y - sVars->scene3D.projectionY * vertices[face->d].y / vertices[face->d].z;

                    color fogColor = sVars->scene3D.fogColor;
                    int32 fogStr = 0;
                    if ((sVars->scene3D.drawList[i].depth - 0x8000) >> 8 >= 0)
                        fogStr = (sVars->scene3D.drawList[i].depth - 0x8000) >> 8;

                    if (fogStr > sVars->scene3D.fogStrength)
                        fogStr = sVars->scene3D.fogStrength;

                    // hope this works well lol
                    Graphics::DrawFace(faceVerts, 4, (fogColor >> 16) & 0xFF, (fogColor >> 8) & 0xFF, (fogColor >> 0) & 0xFF, 0xFF,
                                       INK_NONE);

                    Graphics::DrawFace(faceVerts, 4, (face->color >> 16) & 0xFF, (face->color >> 8) & 0xFF, (face->color >> 0) & 0xFF, 0xFF - fogStr,
                                       INK_ALPHA);

                    // DrawFadedFace(quad, face->color, fogColor, 0xFF - fogStr);
                }
                break;
            case FACE_FLAG_TEXTURED_C:
                if (vertices[face->a].z > 0) {
                    // [face->a].uv == sprite center
                    // [face->b].uv == ???
                    // [face->c].uv == sprite extend (how far to each edge X & Y)
                    // [face->d].uv == unused

                    faceVerts[0].x = screen->center.x + sVars->scene3D.projectionX * (vertices[face->a].x - vertexBuffer[face->b].u) / vertices[face->a].z;
                    faceVerts[0].y = screen->center.y - sVars->scene3D.projectionY * (vertices[face->a].y + vertexBuffer[face->b].v) / vertices[face->a].z;
                    faceVerts[1].x = screen->center.x + sVars->scene3D.projectionX * (vertices[face->a].x + vertexBuffer[face->b].u) / vertices[face->a].z;
                    faceVerts[1].y = screen->center.y - sVars->scene3D.projectionY * (vertices[face->a].y + vertexBuffer[face->b].v) / vertices[face->a].z;
                    faceVerts[2].x = screen->center.x + sVars->scene3D.projectionX * (vertices[face->a].x - vertexBuffer[face->b].u) / vertices[face->a].z;
                    faceVerts[2].y = screen->center.y - sVars->scene3D.projectionY * (vertices[face->a].y - vertexBuffer[face->b].v) / vertices[face->a].z;
                    faceVerts[3].x = screen->center.x + sVars->scene3D.projectionX * (vertices[face->a].x + vertexBuffer[face->b].u) / vertices[face->a].z;
                    faceVerts[3].y = screen->center.y - sVars->scene3D.projectionY * (vertices[face->a].y - vertexBuffer[face->b].v) / vertices[face->a].z;

                    faceUVs[0].x = vertexBuffer[face->a].u - vertexBuffer[face->c].u;
                    faceUVs[0].y = vertexBuffer[face->a].v - vertexBuffer[face->c].v;
                    faceUVs[1].x = vertexBuffer[face->a].u + vertexBuffer[face->c].u;
                    faceUVs[1].y = vertexBuffer[face->a].v - vertexBuffer[face->c].v;
                    faceUVs[2].x = vertexBuffer[face->a].u - vertexBuffer[face->c].u;
                    faceUVs[2].y = vertexBuffer[face->a].v + vertexBuffer[face->c].v;
                    faceUVs[3].x = vertexBuffer[face->a].u + vertexBuffer[face->c].u;
                    faceUVs[3].y = vertexBuffer[face->a].v + vertexBuffer[face->c].v;

                    // DrawTexturedFace(quad, face->color);
                }
                break;
            case FACE_FLAG_TEXTURED_C_BLEND:
                if (vertices[face->a].z > 0) {
                    // See above, its the same just blended

                    faceVerts[0].x = screen->center.x + sVars->scene3D.projectionX * (vertices[face->a].x - vertexBuffer[face->b].u) / vertices[face->a].z;
                    faceVerts[0].y = screen->center.y - sVars->scene3D.projectionY * (vertices[face->a].y + vertexBuffer[face->b].v) / vertices[face->a].z;
                    faceVerts[1].x = screen->center.x + sVars->scene3D.projectionX * (vertices[face->a].x + vertexBuffer[face->b].u) / vertices[face->a].z;
                    faceVerts[1].y = screen->center.y - sVars->scene3D.projectionY * (vertices[face->a].y + vertexBuffer[face->b].v) / vertices[face->a].z;
                    faceVerts[2].x = screen->center.x + sVars->scene3D.projectionX * (vertices[face->a].x - vertexBuffer[face->b].u) / vertices[face->a].z;
                    faceVerts[2].y = screen->center.y - sVars->scene3D.projectionY * (vertices[face->a].y - vertexBuffer[face->b].v) / vertices[face->a].z;
                    faceVerts[3].x = screen->center.x + sVars->scene3D.projectionX * (vertices[face->a].x + vertexBuffer[face->b].u) / vertices[face->a].z;
                    faceVerts[3].y = screen->center.y - sVars->scene3D.projectionY * (vertices[face->a].y - vertexBuffer[face->b].v) / vertices[face->a].z;

                    faceUVs[0].x = vertexBuffer[face->a].u - vertexBuffer[face->c].u;
                    faceUVs[0].y = vertexBuffer[face->a].v - vertexBuffer[face->c].v;
                    faceUVs[1].x = vertexBuffer[face->a].u + vertexBuffer[face->c].u;
                    faceUVs[1].y = vertexBuffer[face->a].v - vertexBuffer[face->c].v;
                    faceUVs[2].x = vertexBuffer[face->a].u - vertexBuffer[face->c].u;
                    faceUVs[2].y = vertexBuffer[face->a].v + vertexBuffer[face->c].v;
                    faceUVs[3].x = vertexBuffer[face->a].u + vertexBuffer[face->c].u;
                    faceUVs[3].y = vertexBuffer[face->a].v + vertexBuffer[face->c].v;

                    // DrawTexturedFaceBlended(quad, face->color);
                }
                break;

            case FACE_FLAG_3DSPRITE:
                if (vertices[face->a].z > 0) {
                    // [face->a].x = xpos
                    // [face->a].y = ypos
                    // [face->a].z = zpos
                    // [face->a].u = aniFrames
                    // [face->a].v = drawFX
                    // [face->b].u == animID
                    // [face->b].v == frameID
                    // [face->c].u == unused
                    // [face->c].v == rotation
                    // [face->d].u == scale.x
                    // [face->d].v == scale.y

                    Vector2 drawPos;
                    drawPos.x = TO_FIXED(screen->center.x + sVars->scene3D.projectionX * vertices[face->a].x / vertices[face->a].z);
                    drawPos.y = TO_FIXED(screen->center.y - sVars->scene3D.projectionY * vertices[face->a].y / vertices[face->a].z);

                    SpriteAnimation aniFrames;
                    aniFrames.aniFrames = vertexBuffer[face->a].u;
                    this->animator.SetAnimation(aniFrames, vertexBuffer[face->b].v, true, vertexBuffer[face->b].u);

                    this->drawFX   = vertexBuffer[face->a].v;
                    this->rotation = vertexBuffer[face->c].v;
                    this->scale.x  = vertexBuffer[face->d].u;
                    this->scale.y  = vertexBuffer[face->d].v;

                    this->animator.DrawSprite(&drawPos, false);
                }
                break;
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void HP_Halfpipe::EditorDraw() {}

void HP_Halfpipe::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_Halfpipe::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(HP_Halfpipe);

    int32 vertexOffsetTable[] = { 19, 19, 19, 19, 17, 17, 45, 17 };
    memcpy(sVars->vertexOffsetTable, vertexOffsetTable, sizeof(vertexOffsetTable));

    int32 matTransformTable[] = { 0, 2, 4, 6, 8, 10, 12, 14, 16, -2, -4, -6, -8, -10, -12, -14, -16 };
    memcpy(sVars->matTransformTable, matTransformTable, sizeof(matTransformTable));

    int32 vertexTable1[] = {
        38, -0x4000, 0x1100,  0, -0x4400, 0xD00,   0, -0x3C00, 0xD00,   0, -0x4000, 0x900,   0, -0x4000, -0x91C,  0, -0x3A37, -0x1A96,
        0,  -0x305E, -0x29E9, 0, -0x2298, -0x35D7, 0, -0x1208, -0x3D68, 0, 0,       -0x4000, 0, 0x1208,  -0x3D68, 0, 0x2298,  -0x35D7,
        0,  0x305E,  -0x29E9, 0, 0x3A37,  -0x1A96, 0, 0x4000,  -0x91C,  0, 0x4000,  0x900,   0, 0x4000,  0x1100,  0, 0x3C00,  0xD00,
        0,  0x4400,  0xD00,   0, -0x4000, 0x1100,  0, -0x4400, 0xD00,   0, -0x3C00, 0xD00,   0, -0x4000, 0x900,   0, -0x4000, -0x91C,
        0,  -0x3A37, -0x1A96, 0, -0x305E, -0x29E9, 0, -0x2298, -0x35D7, 0, -0x1208, -0x3D68, 0, 0,       -0x4000, 0, 0x1208,  -0x3D68,
        0,  0x2298,  -0x35D7, 0, 0x305E,  -0x29E9, 0, 0x3A37,  -0x1A96, 0, 0x4000,  -0x91C,  0, 0x4000,  0x900,   0, 0x4000,  0x1100,
        0,  0x3C00,  0xD00,   0, 0x4400,  0xD00,   0,
    };
    memcpy(sVars->vertexTable1, vertexTable1, sizeof(vertexTable1));

    int32 vertexTable2[] = {
        34, -0x4000, 0x900,   0, -0x4000, -0x91C,  0, -0x3A37, -0x1A96, 0, -0x305E, -0x29E9, 0, -0x2298, -0x35D7, 0, -0x2059, -0x36DE,
        0,  -0x1449, -0x3C5D, 0, -0x1208, -0x3D68, 0, 0,       -0x4000, 0, 0x1208,  -0x3D68, 0, 0x1449,  -0x3C5D, 0, 0x2059,  -0x36DE,
        0,  0x2298,  -0x35D7, 0, 0x305E,  -0x29E9, 0, 0x3A37,  -0x1A96, 0, 0x4000,  -0x91C,  0, 0x4000,  0x900,   0, -0x4000, 0x900,
        0,  -0x4000, -0x91C,  0, -0x3A37, -0x1A96, 0, -0x305E, -0x29E9, 0, -0x2298, -0x35D7, 0, -0x1ED7, -0x378F, 0, -0x15CB, -0x3BAF,
        0,  -0x1208, -0x3D68, 0, 0,       -0x4000, 0, 0x1208,  -0x3D68, 0, 0x15CB,  -0x3BAF, 0, 0x1ED7,  -0x378F, 0, 0x2298,  -0x35D7,
        0,  0x305E,  -0x29E9, 0, 0x3A37,  -0x1A96, 0, 0x4000,  -0x91C,  0, 0x4000,  0x900,   0,
    };
    memcpy(sVars->vertexTable2, vertexTable2, sizeof(vertexTable2));

    int32 vertexTable3[] = { 51, -0x4000, 0x900,   0, -0x4000, -0x91C,  0, -0x3A37, -0x1A96, 0, -0x305E, -0x29E9, 0, -0x2298, -0x35D7,
                             0,  -0x1ED7, -0x378F, 0, -0x15CB, -0x3BAF, 0, -0x1208, -0x3D68, 0, 0,       -0x4000, 0, 0x1208,  -0x3D68,
                             0,  0x15CB,  -0x3BAF, 0, 0x1ED7,  -0x378F, 0, 0x2298,  -0x35D7, 0, 0x305E,  -0x29E9, 0, 0x3A37,  -0x1A96,
                             0,  0x4000,  -0x91C,  0, 0x4000,  0x900,   0, -0x4000, 0x900,   0, -0x4000, -0x91C,  0, -0x3A37, -0x1A96,
                             0,  -0x305E, -0x29E9, 0, -0x2298, -0x35D7, 0, -0x1E46, -0x37D1, 0, -0x165C, -0x3B6D, 0, -0x1208, -0x3D68,
                             0,  0,       -0x4000, 0, 0x1208,  -0x3D68, 0, 0x165C,  -0x3B6D, 0, 0x1E46,  -0x37D1, 0, 0x2298,  -0x35D7,
                             0,  0x305E,  -0x29E9, 0, 0x3A37,  -0x1A96, 0, 0x4000,  -0x91C,  0, 0x4000,  0x900,   0, -0x4000, 0x900,
                             0,  -0x4000, -0x91C,  0, -0x3A37, -0x1A96, 0, -0x305E, -0x29E9, 0, -0x2298, -0x35D7, 0, -0x1D55, -0x383F,
                             0,  -0x174D, -0x3AFF, 0, -0x1208, -0x3D68, 0, 0,       -0x4000, 0, 0x1208,  -0x3D68, 0, 0x174D,  -0x3AFF,
                             0,  0x1D55,  -0x383F, 0, 0x2298,  -0x35D7, 0, 0x305E,  -0x29E9, 0, 0x3A37,  -0x1A96, 0, 0x4000,  -0x91C,
                             0,  0x4000,  0x900,   0 };
    memcpy(sVars->vertexTable3, vertexTable3, sizeof(vertexTable3));

    int32 vertexTable4[] = {
        79, -0x3800, 0xA00,   0, -0x3800, 0xA00,   0, -0x3800, 0xA00,   0, -0x3800, 0xA00,   0, -0x2E00, 0x2400,  0, -0x2E00, 0x2400,
        0,  -0x2E00, 0x2400,  0, -0x2E00, 0x2400,  0, -0x1A00, 0x3600,  0, -0x1A00, 0x3600,  0, -0x1A00, 0x3600,  0, -0x1A00, 0x3600,
        0,  0,       0x4000,  0, 0,       0x4000,  0, 0,       0x4000,  0, 0,       0x4000,  0, 0x1A00,  0x3600,  0, 0x1A00,  0x3600,
        0,  0x1A00,  0x3600,  0, 0x1A00,  0x3600,  0, 0x2E00,  0x2400,  0, 0x2E00,  0x2400,  0, 0x2E00,  0x2400,  0, 0x2E00,  0x2400,
        0,  0x3800,  0xA00,   0, 0x3800,  0xA00,   0, 0x3800,  0xA00,   0, 0x3800,  0xA00,   0, -0x4000, 0x900,   0, -0x4000, -0x91C,
        0,  -0x3A37, -0x1A96, 0, -0x305E, -0x29E9, 0, -0x2298, -0x35D7, 0, -0x1D55, -0x383F, 0, -0x174D, -0x3AFF, 0, -0x1208, -0x3D68,
        0,  0,       -0x4000, 0, 0x1208,  -0x3D68, 0, 0x174D,  -0x3AFF, 0, 0x1D55,  -0x383F, 0, 0x2298,  -0x35D7, 0, 0x305E,  -0x29E9,
        0,  0x3A37,  -0x1A96, 0, 0x4000,  -0x91C,  0, 0x4000,  0x900,   0, -0x4000, 0x900,   0, -0x4000, -0x91C,  0, -0x3A37, -0x1A96,
        0,  -0x305E, -0x29E9, 0, -0x2298, -0x35D7, 0, -0x1C64, -0x38AD, 0, -0x183E, -0x3A91, 0, -0x1208, -0x3D68, 0, 0,       -0x4000,
        0,  0x1208,  -0x3D68, 0, 0x183E,  -0x3A91, 0, 0x1C64,  -0x38AD, 0, 0x2298,  -0x35D7, 0, 0x305E,  -0x29E9, 0, 0x3A37,  -0x1A96,
        0,  0x4000,  -0x91C,  0, 0x4000,  0x900,   0, -0x4000, 0x900,   0, -0x4000, -0x91C,  0, -0x3A37, -0x1A96, 0, -0x305E, -0x29E9,
        0,  -0x2298, -0x35D7, 0, -0x1BD3, -0x38EF, 0, -0x18CF, -0x3A50, 0, -0x1208, -0x3D68, 0, 0,       -0x4000, 0, 0x1208,  -0x3D68,
        0,  0x18CF,  -0x3A50, 0, 0x1BD3,  -0x38EF, 0, 0x2298,  -0x35D7, 0, 0x305E,  -0x29E9, 0, 0x3A37,  -0x1A96, 0, 0x4000,  -0x91C,
        0,  0x4000,  0x900,   0,
    };
    memcpy(sVars->vertexTable4, vertexTable4, sizeof(vertexTable4));

    int32 vertexTable5[] = { 34, -0x4000, 0x900,   0, -0x4000, -0x91C,  0, -0x3A37, -0x1A96, 0, -0x305E, -0x29E9, 0, -0x2298, -0x35D7,
                             0,  -0x1BD3, -0x38EF, 0, -0x18CF, -0x3A50, 0, -0x1208, -0x3D68, 0, 0,       -0x4000, 0, 0x1208,  -0x3D68,
                             0,  0x18CF,  -0x3A50, 0, 0x1BD3,  -0x38EF, 0, 0x2298,  -0x35D7, 0, 0x305E,  -0x29E9, 0, 0x3A37,  -0x1A96,
                             0,  0x4000,  -0x91C,  0, 0x4000,  0x900,   0, -0x4000, 0x900,   0, -0x4000, -0x91C,  0, -0x3A37, -0x1A96,
                             0,  -0x305E, -0x29E9, 0, -0x2298, -0x35D7, 0, -0x1A51, -0x39A0, 0, -0x1A51, -0x39A0, 0, -0x1208, -0x3D68,
                             0,  0,       -0x4000, 0, 0x1208,  -0x3D68, 0, 0x1A51,  -0x39A0, 0, 0x1A51,  -0x39A0, 0, 0x2298,  -0x35D7,
                             0,  0x305E,  -0x29E9, 0, 0x3A37,  -0x1A96, 0, 0x4000,  -0x91C,  0, 0x4000,  0x900,   0 };
    memcpy(sVars->vertexTable5, vertexTable5, sizeof(vertexTable5));

    int32 faceTable1[] = { 22, 0,  1,  3,  2,  3,  0,  1,  20, 19, 2,  0,  2,  21, 19, 2,  1,  20, 22, 3,  2,  2,  21, 22, 3,  2,  3,  22,
                           23, 4,  1,  4,  23, 24, 5,  0,  5,  24, 25, 6,  0,  6,  25, 26, 7,  0,  7,  26, 27, 8,  1,  8,  27, 28, 9,  0,
                           9,  28, 29, 10, 0,  10, 29, 30, 11, 1,  11, 30, 31, 12, 0,  12, 31, 32, 13, 0,  13, 32, 33, 14, 0,  14, 33, 34,
                           15, 1,  16, 17, 15, 18, 3,  16, 35, 36, 17, 2,  17, 36, 15, 34, 2,  16, 35, 37, 18, 2,  18, 37, 34, 15, 2 };
    memcpy(sVars->faceTable1, faceTable1, sizeof(faceTable1));

    int32 faceTable2[] = { 20, 0,  1,  20, 19, 2,  0,  2,  21, 19, 2,  1,  20, 22, 3,  2,  2,  21, 22, 3,  2,  3,  22, 23, 4,  1,
                           4,  23, 24, 5,  0,  5,  24, 25, 6,  0,  6,  25, 26, 7,  0,  7,  26, 27, 8,  1,  8,  27, 28, 9,  0,  9,
                           28, 29, 10, 0,  10, 29, 30, 11, 1,  11, 30, 31, 12, 0,  12, 31, 32, 13, 0,  13, 32, 33, 14, 0,  14, 33,
                           34, 15, 1,  16, 35, 36, 17, 2,  17, 36, 15, 34, 2,  16, 35, 37, 18, 2,  18, 37, 34, 15, 2 };
    memcpy(sVars->faceTable2, faceTable2, sizeof(faceTable2));

    int32 faceTable3[] = { 16, 0,  17, 18, 1,  1,  1,  18, 19, 2,  0,  2,  19, 20, 3,  0,  3,  20, 21, 4,  0,  4,  21, 22, 5,  1,  5,
                           22, 23, 6,  2,  6,  23, 24, 7,  1,  7,  24, 25, 8,  0,  8,  25, 26, 9,  0,  9,  26, 27, 10, 1,  10, 27, 28,
                           11, 2,  11, 28, 29, 12, 1,  12, 29, 30, 13, 0,  13, 30, 31, 14, 0,  14, 31, 32, 15, 0,  15, 32, 33, 16, 1 };
    memcpy(sVars->faceTable3, faceTable3, sizeof(faceTable3));

    int32 faceTable4[] = {
        32, 0,  17, 18, 1,  1,  1,  18, 19, 2,  0,  2,  19, 20, 3,  0,  3,  20, 21, 4,  0,  4,  21, 22, 5,  1,  5,  22, 23, 6,  2,  6,  23,
        24, 7,  1,  7,  24, 25, 8,  0,  8,  25, 26, 9,  0,  9,  26, 27, 10, 1,  10, 27, 28, 11, 2,  11, 28, 29, 12, 1,  12, 29, 30, 13, 0,
        13, 30, 31, 14, 0,  14, 31, 32, 15, 0,  15, 32, 33, 16, 1,  17, 34, 35, 18, 1,  18, 35, 36, 19, 1,  19, 36, 37, 20, 1,  20, 37, 38,
        21, 1,  21, 38, 39, 22, 1,  22, 39, 40, 23, 2,  23, 40, 41, 24, 1,  24, 41, 42, 25, 1,  25, 42, 43, 26, 1,  26, 43, 44, 27, 1,  27,
        44, 45, 28, 2,  28, 45, 46, 29, 1,  29, 46, 47, 30, 1,  30, 47, 48, 31, 1,  31, 48, 49, 32, 1,  32, 49, 50, 33, 1,
    };
    memcpy(sVars->faceTable4, faceTable4, sizeof(faceTable4));

    int32 faceTable5[] = {
        39, 0,  1,  2,  3,  5,  4,  5,  6,  7,  5,  8,  9,  10, 11, 5,  12, 13, 14, 15, 5,  16, 17, 18, 19, 5,  20, 21, 22, 23, 5,  24, 25,
        26, 27, 5,  28, 45, 46, 29, 1,  29, 46, 47, 30, 1,  30, 47, 48, 31, 1,  31, 48, 49, 32, 1,  32, 49, 50, 33, 1,  33, 50, 51, 34, 2,
        34, 51, 52, 35, 1,  35, 52, 53, 36, 1,  36, 53, 54, 37, 1,  37, 54, 55, 38, 1,  38, 55, 56, 39, 2,  39, 56, 57, 40, 1,  40, 57, 58,
        41, 1,  41, 58, 59, 42, 1,  42, 59, 60, 43, 1,  43, 60, 61, 44, 1,  45, 62, 63, 46, 1,  46, 63, 64, 47, 0,  47, 64, 65, 48, 0,  48,
        65, 66, 49, 0,  49, 66, 67, 50, 1,  50, 67, 68, 51, 2,  51, 68, 69, 52, 1,  52, 69, 70, 53, 0,  53, 70, 71, 54, 0,  54, 71, 72, 55,
        1,  55, 72, 73, 56, 2,  56, 73, 74, 57, 1,  57, 74, 75, 58, 0,  58, 75, 76, 59, 0,  59, 76, 77, 60, 0,  60, 77, 78, 61, 1,
    };
    memcpy(sVars->faceTable5, faceTable5, sizeof(faceTable5));

    int32 faceTable6[] = { 16, 0,  17, 18, 1,  1,  1,  18, 19, 2,  0,  2,  19, 20, 3,  0,  3,  20, 21, 4,  0,  4,  21, 22, 5,  1,  5,
                           22, 23, 6,  2,  6,  23, 24, 7,  1,  7,  24, 25, 8,  0,  8,  25, 26, 9,  0,  9,  26, 27, 10, 1,  10, 27, 28,
                           11, 2,  11, 28, 29, 12, 1,  12, 29, 30, 13, 0,  13, 30, 31, 14, 0,  14, 31, 32, 15, 0,  15, 32, 33, 16, 1 };
    memcpy(sVars->faceTable6, faceTable6, sizeof(faceTable6));
}
#endif

void HP_Halfpipe::Serialize() {}

} // namespace GameLogic