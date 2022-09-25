// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Halfpipe Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Halfpipe.hpp"
#include "HP_Setup.hpp"
#include "HP_Player.hpp"
#include "HP_Collectable.hpp"
#include "HP_Checkpoint.hpp"
#include "HP_Message.hpp"
#include "Global/Zone.hpp"

#include "Helpers/LogHelpers.hpp"

using namespace RSDK;

// Faded blending
#define setPixelFaded(pixel, fogClr, fadedColor, alpha)                                                                                              \
    int32 R = (fog_fbufferBlend[(fogClr & 0xF800) >> 11] + fog_pixelBlend[(pixel & 0xF800) >> 11]) << 11;                                            \
    int32 G = (fog_fbufferBlend[(fogClr & 0x7E0) >> 6] + fog_pixelBlend[(pixel & 0x7E0) >> 6]) << 6;                                                 \
    int32 B = fog_fbufferBlend[fogClr & 0x1F] + fog_pixelBlend[pixel & 0x1F];                                                                        \
                                                                                                                                                     \
    fadedColor = R | G | B;

// 50% alpha, but way faster
#define setPixelBlend(pixel, frameBufferClr) frameBufferClr = ((pixel >> 1) & 0x7BEF) + ((frameBufferClr >> 1) & 0x7BEF)

// Alpha blending
#define setPixelAlpha(pixel, frameBufferClr, alpha)                                                                                                  \
    int32 R = (fbufferBlend[(frameBufferClr & 0xF800) >> 11] + pixelBlend[(pixel & 0xF800) >> 11]) << 11;                                            \
    int32 G = (fbufferBlend[(frameBufferClr & 0x7E0) >> 6] + pixelBlend[(pixel & 0x7E0) >> 6]) << 6;                                                 \
    int32 B = fbufferBlend[frameBufferClr & 0x1F] + pixelBlend[pixel & 0x1F];                                                                        \
                                                                                                                                                     \
    frameBufferClr = R | G | B;

// Additive Blending
#define setPixelAdditive(pixel, frameBufferClr)                                                                                                      \
    int32 R = MIN((blendTablePtr[(pixel & 0xF800) >> 11] << 11) + (frameBufferClr & 0xF800), 0xF800);                                                \
    int32 G = MIN((blendTablePtr[(pixel & 0x7E0) >> 6] << 6) + (frameBufferClr & 0x7E0), 0x7E0);                                                     \
    int32 B = MIN(blendTablePtr[pixel & 0x1F] + (frameBufferClr & 0x1F), 0x1F);                                                                      \
                                                                                                                                                     \
    frameBufferClr = R | G | B;

// Subtractive Blending
#define setPixelSubtractive(pixel, frameBufferClr)                                                                                                   \
    int32 R = MAX((frameBufferClr & 0xF800) - (subBlendTable[(pixel & 0xF800) >> 11] << 11), 0);                                                     \
    int32 G = MAX((frameBufferClr & 0x7E0) - (subBlendTable[(pixel & 0x7E0) >> 6] << 6), 0);                                                         \
    int32 B = MAX((frameBufferClr & 0x1F) - subBlendTable[pixel & 0x1F], 0);                                                                         \
                                                                                                                                                     \
    frameBufferClr = R | G | B;

// Only draw if framebuffer clr IS maskColor
#define setPixelMasked(pixel, frameBufferClr)                                                                                                        \
    if (frameBufferClr == maskColor)                                                                                                                 \
        frameBufferClr = pixel;

// Only draw if framebuffer clr is NOT maskColor
#define setPixelUnmasked(pixel, frameBufferClr)                                                                                                      \
    if (frameBufferClr != maskColor)                                                                                                                 \
        frameBufferClr = pixel;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Halfpipe);

void HP_Halfpipe::Update()
{
    sVars->scene3D.vertexCount = sVars->prevVertexCount;
    sVars->scene3D.faceCount   = sVars->faceCount;

    if (true) {
        this->moveStep += this->moveSpeed;
        this->playerZPos += this->moveSpeed << 13;
        if (this->moveStep >= 64) {
            this->moveStep -= 64;
            HP_Halfpipe::SetupVertices();

            this->transformIDHalfpipe     = (this->transformIDHalfpipe + 1) % 40;
            this->nextTransformIDHalfpipe = (this->nextTransformIDHalfpipe + 1) % 40;
            this->transformIDWorld        = (this->transformIDWorld + 1) % 40;
            this->nextTransformIDWorld    = (this->nextTransformIDWorld + 1) % 40;
        }
    }

    // ========================
    // HALFPIPE TRANSFORMATIONS
    // ========================

    // Translation
    this->halfpipeTranslation.x = sVars->translateTable[this->transformIDHalfpipe].x;
    this->halfpipeTranslation.x += ((sVars->translateTable[this->nextTransformIDHalfpipe].x - this->halfpipeTranslation.x) * this->moveStep) >> 6;
    this->halfpipeTranslation.x = -this->halfpipeTranslation.x;

    this->halfpipeTranslation.y = sVars->translateTable[this->transformIDHalfpipe].y;
    this->halfpipeTranslation.y += ((sVars->translateTable[this->nextTransformIDHalfpipe].y - this->halfpipeTranslation.y) * this->moveStep) >> 6;
    this->halfpipeTranslation.y = -this->halfpipeTranslation.y;

    this->halfpipeTranslation.z = sVars->translateTable[this->transformIDHalfpipe].z;
    this->halfpipeTranslation.z += ((sVars->translateTable[this->nextTransformIDHalfpipe].z - this->halfpipeTranslation.z) * this->moveStep) >> 6;
    this->halfpipeTranslation.z = -this->halfpipeTranslation.z;

    // Rotation

    int32 rotation = sVars->rotateTable[this->transformIDHalfpipe].x;
    rotation += ((sVars->rotateTable[this->nextTransformIDHalfpipe].x - rotation) * this->moveStep) >> 6;
    rotation <<= 1;

    if (this->newHalfpipeRotation.x != rotation) {
        int32 distance = (rotation - this->newHalfpipeRotation.x) >> 1;

        if (!distance) {
            if (this->newHalfpipeRotation.x < rotation)
                this->newHalfpipeRotation.x++;
            else
                this->newHalfpipeRotation.x--;
        }
        else {
            this->newHalfpipeRotation.x += distance;
        }
    }

    rotation = sVars->rotateTable[this->transformIDHalfpipe].y;
    rotation += ((sVars->rotateTable[this->nextTransformIDHalfpipe].y - rotation) * this->moveStep) >> 6;
    rotation <<= 1;

    if (this->newHalfpipeRotation.y != rotation) {
        int32 distance = (rotation - this->newHalfpipeRotation.y) >> 1;

        if (!distance) {
            if (this->newHalfpipeRotation.y < rotation)
                this->newHalfpipeRotation.y++;
            else
                this->newHalfpipeRotation.y--;
        }
        else {
            this->newHalfpipeRotation.y += distance;
        }
    }

    rotation = sVars->rotateTable[this->transformIDHalfpipe].z;
    rotation += ((sVars->rotateTable[this->nextTransformIDHalfpipe].z - rotation) * this->moveStep) >> 6;
    rotation <<= 1;

    if (this->newHalfpipeRotation.z != rotation) {
        int32 distance = (rotation - this->newHalfpipeRotation.z) >> 1;

        if (!distance) {
            if (this->newHalfpipeRotation.z < rotation)
                this->newHalfpipeRotation.z++;
            else
                this->newHalfpipeRotation.z--;
        }
        else {
            this->newHalfpipeRotation.z += distance;
        }
    }

    this->halfpipeRotation.x = this->newHalfpipeRotation.x >> 1;
    this->halfpipeRotation.y = this->newHalfpipeRotation.y >> 1;
    this->halfpipeRotation.z = this->newHalfpipeRotation.z >> 1;

    // ========================
    // WORLD TRANSFORMATIONS
    // ========================

    // Translation
    this->worldTranslation.x = sVars->translateTable[this->transformIDWorld].x;
    this->worldTranslation.x += ((sVars->translateTable[this->nextTransformIDWorld].x - this->worldTranslation.x) * this->moveStep) >> 6;

    this->worldTranslation.y = sVars->translateTable[this->transformIDWorld].y;
    this->worldTranslation.y += ((sVars->translateTable[this->nextTransformIDWorld].y - this->worldTranslation.y) * this->moveStep) >> 6;

    this->worldTranslation.z = sVars->translateTable[this->transformIDWorld].z;
    this->worldTranslation.z += ((sVars->translateTable[this->nextTransformIDWorld].z - this->worldTranslation.z) * this->moveStep) >> 6;

    // Rotation

    this->worldRotation.x = sVars->rotateTable[this->transformIDWorld].x;
    this->worldRotation.x += ((sVars->rotateTable[this->nextTransformIDWorld].x - this->worldRotation.x) * this->moveStep) >> 6;

    this->worldRotation.y = sVars->rotateTable[this->transformIDWorld].y;
    this->worldRotation.y += ((sVars->rotateTable[this->nextTransformIDWorld].y - this->worldRotation.y) * this->moveStep) >> 6;

    this->worldRotation.z = sVars->rotateTable[this->transformIDWorld].z;
    this->worldRotation.z += ((sVars->rotateTable[this->nextTransformIDWorld].z - this->worldRotation.z) * this->moveStep) >> 6;
}
void HP_Halfpipe::LateUpdate() {}
void HP_Halfpipe::StaticUpdate()
{
    // gotta be done here to prevent double Create() calls that come if you spawn an entity in StageLoad()
    if (!sVars->initialized) {
        // ... and ensure we have a halfpipe entity in the correct reserved slot
        GameObject::Reset(SLOT_HP_HALFPIPE, sVars->classID, nullptr);

        sVars->sortSlot = RESERVE_ENTITY_COUNT;
        for (int32 i = 0; i < 4; ++i) {
            SetupFaces(sVars->faceTable1);
            SetupFaces(sVars->faceTable2);
            SetupFaces(sVars->faceTable2);
            SetupFaces(sVars->faceTable2);
            SetupFaces(sVars->faceTable3);
            SetupFaces(sVars->faceTable4);
            SetupFaces(sVars->faceTable5);
            SetupFaces(sVars->faceTable6);
        }

        sVars->scene3D.vertexCount = sVars->vertexCount;
        sVars->prevVertexCount     = sVars->vertexCount;
        sVars->faceCount           = sVars->scene3D.faceCount;
        sVars->vertexCount         = 0;
        sVars->scene3D.projectionX = 216;
        sVars->scene3D.projectionY = 216;
        sVars->scene3D.fogStrength = 0x50;
        sVars->scene3D.fogColor    = 0x000000;

        sVars->initialized = true;
    }
}
void HP_Halfpipe::Draw()
{
    MatrixTranslateXYZ(&sVars->scene3D.matWorld, this->halfpipeTranslation.x, this->halfpipeTranslation.y, this->halfpipeTranslation.z);
    MatrixRotateXYZ(&sVars->scene3D.matView, this->halfpipeRotation.x, this->halfpipeRotation.y, this->halfpipeRotation.z);
    sVars->scene3D.matView.Inverse();
    MatrixRotateXYZ(&sVars->scene3D.matTemp, -8, 0, 0);
    MatrixMultiply(&sVars->scene3D.matView, &sVars->scene3D.matView, &sVars->scene3D.matTemp);

    HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
    int32 x            = (FROM_FIXED(player1->position.x) * -0x40) >> 9;
    int32 y            = ((FROM_FIXED(player1->position.y) * -0x60) >> 9) - 0x200;

    MatrixTranslateXYZ(&sVars->scene3D.matTemp, x, y, 0);
    MatrixMultiply(&sVars->scene3D.matView, &sVars->scene3D.matView, &sVars->scene3D.matTemp);

    Draw3DScene();
}

void HP_Halfpipe::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawGroup = Zone::sVars->objectDrawGroup[0];

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

        this->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    }
}

void HP_Halfpipe::StageLoad()
{
    Mod::Engine::GetRGB32To16Buffer(&sVars->rgb32To16_R, &sVars->rgb32To16_G, &sVars->rgb32To16_B);
    sVars->blendLookupTable    = Mod::Engine::GetBlendLookupTable();
    sVars->subtractLookupTable = Mod::Engine::GetSubtractLookupTable();

    sVars->aniFrames.Load("Special/Halfpipe.bin", SCOPE_STAGE);
    sVars->shadowFrames.Load("Special/Shadow.bin", SCOPE_STAGE);

    HP_Setup::sVars->controlLayer.Get("Control");

    Vector2 stageSize;
    HP_Setup::sVars->controlLayer.Size(&stageSize, true);
    sVars->stageSize = TO_FIXED(stageSize.y);

    sVars->initialized        = false;
    sVars->vertexTableID      = 0;
    sVars->vertexCount        = 0;
    sVars->transformPos       = 0;
    sVars->prevVertexCount    = HP_VERTEXBUFFER_SIZE;
    sVars->tempTranslate.x    = 0;
    sVars->tempTranslate.y    = 0;
    sVars->tempTranslate.z    = 0;
    sVars->worldRotate.x      = 0;
    sVars->worldRotate.y      = 0;
    sVars->worldRotate.z      = 0;
    sVars->checkpointTexCount = 0;
    memset(&sVars->scene3D, 0, sizeof(sVars->scene3D));

    // Destroy any halfpipe entities placed in the scene
    for (auto halfpipe : GameObject::GetEntities<HP_Halfpipe>(FOR_ALL_ENTITIES)) halfpipe->Destroy();
}

void HP_Halfpipe::SetupFaces(int32 *faceTable)
{
    int32 faceID = sVars->scene3D.faceCount;

    RSDK::Animator texAnimator;
    texAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);

    SpriteFrame *frame = texAnimator.GetFrame(sVars->aniFrames);

    FaceTable *faceList = (FaceTable *)faceTable;

    for (int32 f = 0; f < faceList->count; ++f) {
        sVars->scene3D.faceBuffer[faceID].a = sVars->vertexCount + faceList->faces[f].a;
        sVars->scene3D.faceBuffer[faceID].b = sVars->vertexCount + faceList->faces[f].b;
        sVars->scene3D.faceBuffer[faceID].c = sVars->vertexCount + faceList->faces[f].c;
        sVars->scene3D.faceBuffer[faceID].d = sVars->vertexCount + faceList->faces[f].d;

        switch (faceList->faces[f].type) {
            case FaceTable::Entry::StageColor1:
                sVars->scene3D.faceBuffer[faceID].flag  = HP_Halfpipe::FaceFaded;
                sVars->scene3D.faceBuffer[faceID].color = HP_Setup::sVars->stageColor1;
                break;

            case FaceTable::Entry::StageColor2:
                sVars->scene3D.faceBuffer[faceID].flag  = HP_Halfpipe::FaceFaded;
                sVars->scene3D.faceBuffer[faceID].color = HP_Setup::sVars->stageColor2;
                break;

            case FaceTable::Entry::Arrow: // arrows on the ground & "rails" on the side of the halfpipe
                sVars->scene3D.faceBuffer[faceID].flag  = HP_Halfpipe::FaceFaded;
                sVars->scene3D.faceBuffer[faceID].color = HP_Setup::sVars->arrowColor | (0xFF << 24);
                break;

            case FaceTable::Entry::RailEnd: // edges of the rail bits on the side of the halfpipe
                sVars->scene3D.faceBuffer[faceID].flag  = HP_Halfpipe::FaceFaded;
                sVars->scene3D.faceBuffer[faceID].color = HP_Setup::sVars->railEdgeColor | (0xFF << 24);
                break;

            case FaceTable::Entry::LoopRing:
                // Basically a bare-bones ver of DrawTexture() lol
                sVars->scene3D.faceBuffer[faceID].flag = HP_Halfpipe::FaceTexturedC;

                int32 index                             = sVars->scene3D.faceBuffer[faceID].a;
                sVars->scene3D.vertexBuffer[index].u    = frame->sprX + (frame->width >> 1);
                sVars->scene3D.vertexBuffer[index].v    = frame->sprY + (frame->height >> 1);

                if (!(sVars->checkpointTexCount % 7)) {
                    int32 tablePos = sVars->checkpointTexCount / 7;
                    if (tablePos < 4)
                        sVars->checkpointFaceTable[tablePos] = index;
                }
                sVars->checkpointTexCount++;
                sVars->checkpointTexCount++;

                index                                = sVars->scene3D.faceBuffer[faceID].b;
                sVars->scene3D.vertexBuffer[index].u = 0x800; // scale
                sVars->scene3D.vertexBuffer[index].v = 0x800; // scale

                index                                = sVars->scene3D.faceBuffer[faceID].c;
                sVars->scene3D.vertexBuffer[index].u = frame->width >> 1;
                sVars->scene3D.vertexBuffer[index].v = frame->height >> 1;
                break;
        }

        faceID++;
        sVars->scene3D.faceCount++;
    }

    SetupVertices();
}
void HP_Halfpipe::SetupVertices()
{
    HP_Halfpipe *halfpipe = GameObject::Get<HP_Halfpipe>(SLOT_HP_HALFPIPE);

    int32 tx = FROM_FIXED(halfpipe->position.x) + 376;
    int32 ty = FROM_FIXED(halfpipe->position.y);

    // check for "end marker"
    if (HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0) == 1) {
        ty                   = (ty + 0x80) & 0xFF80;
        halfpipe->position.y = TO_FIXED(ty);
    }
    tx += 16;

    uint8 flag = 0;
    switch (((sVars->worldRotate.y + 0x40) & 0x1FF) >> 7) {
        case 0:
            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.x = sVars->matTransformTable[flag];
            tx += 16;

            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.y = sVars->matTransformTable[flag];
            tx += 16;

            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.z = sVars->matTransformTable[flag];
            break;

        case 1:
            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.z = sVars->matTransformTable[flag];
            tx += 16;

            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.y = sVars->matTransformTable[flag];
            tx += 16;

            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.x = sVars->matTransformTable[flag];
            break;

        case 2:
            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.x = -sVars->matTransformTable[flag];
            tx += 16;

            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.y = sVars->matTransformTable[flag];
            tx += 16;

            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.z = -sVars->matTransformTable[flag];
            break;

        case 3:
            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.z = -sVars->matTransformTable[flag];
            tx += 16;

            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.y = sVars->matTransformTable[flag];
            tx += 16;

            flag                  = HP_Setup::sVars->controlLayer.GetTile(tx >> 4, ty >> 4).GetFlags(0);
            sVars->rotateOffset.x = -sVars->matTransformTable[flag];
            break;
    }

    halfpipe->position.y += TO_FIXED(8);
    if (halfpipe->position.y >= sVars->stageSize)
        halfpipe->position.y -= sVars->stageSize;

    halfpipe->stageZPos += TO_FIXED(8);
    if (halfpipe->stageZPos >= sVars->stageSize)
        halfpipe->stageZPos -= sVars->stageSize;

    MatrixRotateXYZ(&sVars->scene3D.matWorld, sVars->worldRotate.x, sVars->worldRotate.y, sVars->worldRotate.z);
    MatrixTranslateXYZ(&sVars->scene3D.matTemp, sVars->tempTranslate.x, sVars->tempTranslate.y, sVars->tempTranslate.z);
    MatrixMultiply(&sVars->scene3D.matWorld, &sVars->scene3D.matWorld, &sVars->scene3D.matTemp);

    Vertex vertBuffer[2];

    while (true) {
        HP_Collectable *stageEntity = GameObject::Get<HP_Collectable>(sVars->sortSlot);
        if (stageEntity->position.y < halfpipe->position.y) {

            Vector3 *localPos       = nullptr;
            Vector3 *localShadowPos = nullptr;
            Vector3 *worldPos       = nullptr;
            Vector3 *worldShadowPos = nullptr;
            bool32 shadowsEnabled   = false;
            if (HP_Collectable::sVars && stageEntity->classID == HP_Collectable::sVars->classID) {
                localPos       = &stageEntity->localPos;
                localShadowPos = &stageEntity->localShadowPos;
                worldPos       = &stageEntity->worldPos;
                worldShadowPos = &stageEntity->worldShadowPos;
                shadowsEnabled = stageEntity->shadowsEnabled;
            }
            else if (HP_Checkpoint::sVars && stageEntity->classID == HP_Checkpoint::sVars->classID) {
                HP_Checkpoint *checkpoint = (HP_Checkpoint *)stageEntity;
                localPos                  = &checkpoint->localPos;
                localShadowPos            = &checkpoint->localShadowPos;
                worldPos                  = &checkpoint->worldPos;
                worldShadowPos            = &checkpoint->worldShadowPos;
            }
            else if (HP_Message::sVars && stageEntity->classID == HP_Message::sVars->classID) {
                HP_Message *message = (HP_Message *)stageEntity;
                localPos            = &message->localPos;
                localShadowPos      = &message->localShadowPos;
                worldPos            = &message->worldPos;
                worldShadowPos      = &message->worldShadowPos;
            }
            else { // unhandled entity type, ignore it
                sVars->sortSlot++;
                continue;
            }

            stageEntity->active = ACTIVE_NORMAL;
            if (shadowsEnabled) {
                vertBuffer[0].x = localPos->x;
                vertBuffer[0].y = localPos->y;
                vertBuffer[0].z = 0;
                vertBuffer[1].x = localShadowPos->x;
                vertBuffer[1].y = localShadowPos->y;
                vertBuffer[1].z = 0;
                TransformVertices(&sVars->scene3D.matWorld, vertBuffer, 0, 2);

                localPos->z       = halfpipe->stageZPos;
                worldPos->x       = vertBuffer[0].x;
                worldPos->y       = vertBuffer[0].y;
                worldPos->z       = vertBuffer[0].z;
                worldShadowPos->x = vertBuffer[1].x;
                worldShadowPos->y = vertBuffer[1].y;
                worldShadowPos->z = vertBuffer[1].z;
            }
            else {
                vertBuffer[0].x = localPos->x;
                vertBuffer[0].y = localPos->y;
                vertBuffer[0].z = 0;
                TransformVertices(&sVars->scene3D.matWorld, vertBuffer, 0, 1);

                localPos->z = halfpipe->stageZPos;
                worldPos->x = vertBuffer[0].x;
                worldPos->y = vertBuffer[0].y;
                worldPos->z = vertBuffer[0].z;
            }

            sVars->sortSlot++;
        }
        else {
            break;
        }
    }

    int32 tableID            = sVars->vertexTableID++ & 7;
    VertexTable *vertexTable = sVars->vertexTablePtrs[tableID];

    int32 count  = vertexTable->count;
    int32 vertID = sVars->vertexCount;
    for (int32 i = 0; i < count; ++i) {
        sVars->scene3D.vertexBuffer[vertID].x = vertexTable->vertices[i].x;
        sVars->scene3D.vertexBuffer[vertID].y = vertexTable->vertices[i].y;
        sVars->scene3D.vertexBuffer[vertID].z = vertexTable->vertices[i].z;
        vertID++;
    }

    sVars->translateTable[sVars->transformPos].x = sVars->tempTranslate.x;
    sVars->translateTable[sVars->transformPos].y = sVars->tempTranslate.y;
    sVars->translateTable[sVars->transformPos].z = sVars->tempTranslate.z;
    sVars->rotateTable[sVars->transformPos].x    = sVars->worldRotate.x;
    sVars->rotateTable[sVars->transformPos].y    = sVars->worldRotate.y;
    sVars->rotateTable[sVars->transformPos].z    = sVars->worldRotate.z;

    int32 offset = sVars->vertexCount + sVars->vertexOffsetTable[tableID];
    TransformVertices(&sVars->scene3D.matWorld, sVars->scene3D.vertexBuffer, sVars->vertexCount, sVars->vertexOffsetTable[tableID]);
    switch (tableID) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 7:
            vertBuffer[0].x = 0;
            vertBuffer[0].y = 0;
            vertBuffer[0].z = 0x2000;
            TransformVertices(&sVars->scene3D.matWorld, vertBuffer, 0, 1);

            sVars->tempTranslate.x = vertBuffer[0].x;
            sVars->tempTranslate.y = vertBuffer[0].y;
            sVars->tempTranslate.z = vertBuffer[0].z;
            sVars->worldRotate.x += sVars->rotateOffset.x;
            sVars->worldRotate.y += sVars->rotateOffset.y;
            sVars->worldRotate.z += sVars->rotateOffset.z;

            MatrixRotateXYZ(&sVars->scene3D.matWorld, sVars->worldRotate.x, sVars->worldRotate.y, sVars->worldRotate.z);
            MatrixTranslateXYZ(&sVars->scene3D.matTemp, sVars->tempTranslate.x, sVars->tempTranslate.y, sVars->tempTranslate.z);
            MatrixMultiply(&sVars->scene3D.matWorld, &sVars->scene3D.matWorld, &sVars->scene3D.matTemp);
            TransformVertices(&sVars->scene3D.matWorld, sVars->scene3D.vertexBuffer, offset, vertID - offset);
            break;

        case 5:
            vertBuffer[0].x = 0;
            vertBuffer[0].y = 0;
            vertBuffer[0].z = 0xC00;
            vertBuffer[1].x = 0;
            vertBuffer[1].y = 0;
            vertBuffer[1].z = 0x2000;
            TransformVertices(&sVars->scene3D.matWorld, vertBuffer, 0, 2);

            sVars->tempTranslate.x = vertBuffer[0].x;
            sVars->tempTranslate.y = vertBuffer[0].y;
            sVars->tempTranslate.z = vertBuffer[0].z;
            sVars->worldRotate.x += (sVars->rotateOffset.x * 12) >> 5;
            sVars->worldRotate.y += (sVars->rotateOffset.y * 12) >> 5;
            sVars->worldRotate.z += (sVars->rotateOffset.z * 12) >> 5;

            MatrixRotateXYZ(&sVars->scene3D.matWorld, sVars->worldRotate.x, sVars->worldRotate.y, sVars->worldRotate.z);
            MatrixTranslateXYZ(&sVars->scene3D.matTemp, sVars->tempTranslate.x, sVars->tempTranslate.y, sVars->tempTranslate.z);
            MatrixMultiply(&sVars->scene3D.matWorld, &sVars->scene3D.matWorld, &sVars->scene3D.matTemp);
            TransformVertices(&sVars->scene3D.matWorld, sVars->scene3D.vertexBuffer, offset, 17);

            sVars->tempTranslate.x = vertBuffer[1].x;
            sVars->tempTranslate.y = vertBuffer[1].y;
            sVars->tempTranslate.z = vertBuffer[1].z;
            sVars->worldRotate.x   = sVars->rotateTable[sVars->transformPos].x + sVars->rotateOffset.x;
            sVars->worldRotate.y   = sVars->rotateTable[sVars->transformPos].y + sVars->rotateOffset.y;
            sVars->worldRotate.z   = sVars->rotateTable[sVars->transformPos].z + sVars->rotateOffset.z;

            MatrixRotateXYZ(&sVars->scene3D.matWorld, sVars->worldRotate.x, sVars->worldRotate.y, sVars->worldRotate.z);
            MatrixTranslateXYZ(&sVars->scene3D.matTemp, sVars->tempTranslate.x, sVars->tempTranslate.y, sVars->tempTranslate.z);
            MatrixMultiply(&sVars->scene3D.matWorld, &sVars->scene3D.matWorld, &sVars->scene3D.matTemp);
            TransformVertices(&sVars->scene3D.matWorld, sVars->scene3D.vertexBuffer, offset + 17, vertID - (offset + 17));
            break;

        case 6:
            vertBuffer[0].x = 0;
            vertBuffer[0].y = 0;
            vertBuffer[0].z = 0x1400;
            vertBuffer[1].x = 0;
            vertBuffer[1].y = 0;
            vertBuffer[1].z = 0x2000;
            TransformVertices(&sVars->scene3D.matWorld, vertBuffer, 0, 2);

            sVars->tempTranslate.x = vertBuffer[0].x;
            sVars->tempTranslate.y = vertBuffer[0].y;
            sVars->tempTranslate.z = vertBuffer[0].z;
            sVars->worldRotate.x += (sVars->rotateOffset.x * 20) >> 5;
            sVars->worldRotate.y += (sVars->rotateOffset.y * 20) >> 5;
            sVars->worldRotate.z += (sVars->rotateOffset.z * 20) >> 5;

            MatrixRotateXYZ(&sVars->scene3D.matWorld, sVars->worldRotate.x, sVars->worldRotate.y, sVars->worldRotate.z);
            MatrixTranslateXYZ(&sVars->scene3D.matTemp, sVars->tempTranslate.x, sVars->tempTranslate.y, sVars->tempTranslate.z);
            MatrixMultiply(&sVars->scene3D.matWorld, &sVars->scene3D.matWorld, &sVars->scene3D.matTemp);
            TransformVertices(&sVars->scene3D.matWorld, sVars->scene3D.vertexBuffer, offset, 17);

            sVars->tempTranslate.x = vertBuffer[1].x;
            sVars->tempTranslate.y = vertBuffer[1].y;
            sVars->tempTranslate.z = vertBuffer[1].z;
            sVars->worldRotate.x   = sVars->rotateTable[sVars->transformPos].x + sVars->rotateOffset.x;
            sVars->worldRotate.y   = sVars->rotateTable[sVars->transformPos].y + sVars->rotateOffset.y;
            sVars->worldRotate.z   = sVars->rotateTable[sVars->transformPos].z + sVars->rotateOffset.z;

            MatrixRotateXYZ(&sVars->scene3D.matWorld, sVars->worldRotate.x, sVars->worldRotate.y, sVars->worldRotate.z);
            MatrixTranslateXYZ(&sVars->scene3D.matTemp, sVars->tempTranslate.x, sVars->tempTranslate.y, sVars->tempTranslate.z);
            MatrixMultiply(&sVars->scene3D.matWorld, &sVars->scene3D.matWorld, &sVars->scene3D.matTemp);
            TransformVertices(&sVars->scene3D.matWorld, sVars->scene3D.vertexBuffer, offset + 17, vertID - (offset + 17));
            break;
    }

    sVars->transformPos = (sVars->transformPos + 1) % 40;
    sVars->vertexCount  = vertID;
    if (sVars->vertexCount >= sVars->prevVertexCount)
        sVars->vertexCount = 0;
}

void HP_Halfpipe::DrawShadow(int32 x, int32 y, int32 z, int32 scaleX, int32 scaleY, RSDK::Animator *animator, bool32 transformVerts)
{
    int32 vertexID = sVars->scene3D.vertexCount;
    int32 faceID   = sVars->scene3D.faceCount;

    int32 startVertexID = vertexID;

    // [face->a].x = xpos
    // [face->a].y = ypos
    // [face->a].z = zpos
    // [face->a].u == spriteCenter.x
    // [face->a].v == spriteCenter.y
    // [face->b].u == scale.x
    // [face->b].v == scale.y
    // [face->c].u == spriteExtend.x
    // [face->c].v == spriteExtend.y
    // [face->d].u == unused
    // [face->d].u == unused

    SpriteFrame *frame = animator->GetFrame(sVars->shadowFrames);

    // Draw Shadow
    sVars->scene3D.faceBuffer[faceID].flag  = HP_Halfpipe::FaceTexturedC_Blend;
    sVars->scene3D.faceBuffer[faceID].a     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = x;
    sVars->scene3D.vertexBuffer[vertexID].y = y;
    sVars->scene3D.vertexBuffer[vertexID].z = z;
    sVars->scene3D.vertexBuffer[vertexID].u = frame->sprX + (frame->width >> 1);
    sVars->scene3D.vertexBuffer[vertexID].v = frame->sprY + (frame->height >> 1);
    if (transformVerts)
        TransformVertices(&sVars->scene3D.matWorld, sVars->scene3D.vertexBuffer, startVertexID, 1);
    vertexID++;

    sVars->scene3D.faceBuffer[faceID].b     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = sVars->scene3D.vertexBuffer[startVertexID].x;
    sVars->scene3D.vertexBuffer[vertexID].y = sVars->scene3D.vertexBuffer[startVertexID].y;
    sVars->scene3D.vertexBuffer[vertexID].z = sVars->scene3D.vertexBuffer[startVertexID].z;
    sVars->scene3D.vertexBuffer[vertexID].u = scaleX;
    sVars->scene3D.vertexBuffer[vertexID].v = scaleY;
    vertexID++;

    sVars->scene3D.faceBuffer[faceID].c     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = sVars->scene3D.vertexBuffer[startVertexID].x;
    sVars->scene3D.vertexBuffer[vertexID].y = sVars->scene3D.vertexBuffer[startVertexID].y;
    sVars->scene3D.vertexBuffer[vertexID].z = sVars->scene3D.vertexBuffer[startVertexID].z;
    sVars->scene3D.vertexBuffer[vertexID].u = frame->width >> 1;
    sVars->scene3D.vertexBuffer[vertexID].v = frame->height >> 1;
    vertexID++;

    sVars->scene3D.faceBuffer[faceID].d     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = sVars->scene3D.vertexBuffer[startVertexID].x;
    sVars->scene3D.vertexBuffer[vertexID].y = sVars->scene3D.vertexBuffer[startVertexID].y;
    sVars->scene3D.vertexBuffer[vertexID].z = sVars->scene3D.vertexBuffer[startVertexID].z;
    vertexID++;

    sVars->scene3D.vertexCount += 4;
    sVars->scene3D.faceCount++;
}

void HP_Halfpipe::DrawTexture(int32 x, int32 y, int32 z, int32 scaleX, int32 scaleY, RSDK::Animator *animator, RSDK::SpriteAnimation aniFrames,
                              bool32 transformVerts)
{
    int32 vertexID = sVars->scene3D.vertexCount;
    int32 faceID   = sVars->scene3D.faceCount;

    int32 startVertexID = vertexID;

    // [face->a].x = xpos
    // [face->a].y = ypos
    // [face->a].z = zpos
    // [face->a].u == spriteCenter.x
    // [face->a].v == spriteCenter.y
    // [face->b].u == scale.x
    // [face->b].v == scale.y
    // [face->c].u == spriteExtend.x
    // [face->c].v == spriteExtend.y
    // [face->d].u == unused
    // [face->d].u == unused

    SpriteFrame *frame = animator->GetFrame(aniFrames);

    // Draw Shadow
    sVars->scene3D.faceBuffer[faceID].flag  = HP_Halfpipe::FaceTexturedC;
    sVars->scene3D.faceBuffer[faceID].a     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = x;
    sVars->scene3D.vertexBuffer[vertexID].y = y;
    sVars->scene3D.vertexBuffer[vertexID].z = z;
    sVars->scene3D.vertexBuffer[vertexID].u = frame->sprX + (frame->width >> 1);
    sVars->scene3D.vertexBuffer[vertexID].v = frame->sprY + (frame->height >> 1);
    if (transformVerts)
        TransformVertices(&sVars->scene3D.matWorld, sVars->scene3D.vertexBuffer, startVertexID, 1);
    vertexID++;

    sVars->scene3D.faceBuffer[faceID].b     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = sVars->scene3D.vertexBuffer[startVertexID].x;
    sVars->scene3D.vertexBuffer[vertexID].y = sVars->scene3D.vertexBuffer[startVertexID].y;
    sVars->scene3D.vertexBuffer[vertexID].z = sVars->scene3D.vertexBuffer[startVertexID].z;
    sVars->scene3D.vertexBuffer[vertexID].u = scaleX;
    sVars->scene3D.vertexBuffer[vertexID].v = scaleY;
    vertexID++;

    sVars->scene3D.faceBuffer[faceID].c     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = sVars->scene3D.vertexBuffer[startVertexID].x;
    sVars->scene3D.vertexBuffer[vertexID].y = sVars->scene3D.vertexBuffer[startVertexID].y;
    sVars->scene3D.vertexBuffer[vertexID].z = sVars->scene3D.vertexBuffer[startVertexID].z;
    sVars->scene3D.vertexBuffer[vertexID].u = frame->width >> 1;
    sVars->scene3D.vertexBuffer[vertexID].v = frame->height >> 1;
    vertexID++;

    sVars->scene3D.faceBuffer[faceID].d     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = sVars->scene3D.vertexBuffer[startVertexID].x;
    sVars->scene3D.vertexBuffer[vertexID].y = sVars->scene3D.vertexBuffer[startVertexID].y;
    sVars->scene3D.vertexBuffer[vertexID].z = sVars->scene3D.vertexBuffer[startVertexID].z;
    vertexID++;

    sVars->scene3D.vertexCount += 4;
    sVars->scene3D.faceCount++;
}

void HP_Halfpipe::DrawSprite(int32 x, int32 y, int32 z, uint8 drawFX, int32 scaleX, int32 scaleY, int16 rotation, RSDK::Animator *animator,
                             RSDK::SpriteAnimation aniFrames, bool32 transformVerts)
{
    int32 vertexID = sVars->scene3D.vertexCount;
    int32 faceID   = sVars->scene3D.faceCount;

    int32 startVertexID = vertexID;

    // [face->a].x = xpos
    // [face->a].y = ypos
    // [face->a].z = stageZPos
    // [face->a].u = aniFrames
    // [face->a].v = drawFX
    // [face->b].u == frameID
    // [face->b].v == animID
    // [face->c].u == unused
    // [face->c].v == rotation
    // [face->d].u == scale.x
    // [face->d].v == scale.y

    // Draw Shadow
    sVars->scene3D.faceBuffer[faceID].flag  = HP_Halfpipe::Face3DSprite;
    sVars->scene3D.faceBuffer[faceID].a     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = x;
    sVars->scene3D.vertexBuffer[vertexID].y = y;
    sVars->scene3D.vertexBuffer[vertexID].z = z;
    sVars->scene3D.vertexBuffer[vertexID].u = aniFrames.aniFrames;
    sVars->scene3D.vertexBuffer[vertexID].v = drawFX;
    if (transformVerts)
        TransformVertices(&sVars->scene3D.matWorld, sVars->scene3D.vertexBuffer, startVertexID, 1);
    vertexID++;

    sVars->scene3D.faceBuffer[faceID].b     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = sVars->scene3D.vertexBuffer[startVertexID].x;
    sVars->scene3D.vertexBuffer[vertexID].y = sVars->scene3D.vertexBuffer[startVertexID].y;
    sVars->scene3D.vertexBuffer[vertexID].z = sVars->scene3D.vertexBuffer[startVertexID].z;
    sVars->scene3D.vertexBuffer[vertexID].u = animator->frameID;
    sVars->scene3D.vertexBuffer[vertexID].v = animator->animationID;
    vertexID++;

    sVars->scene3D.faceBuffer[faceID].c     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = sVars->scene3D.vertexBuffer[startVertexID].x;
    sVars->scene3D.vertexBuffer[vertexID].y = sVars->scene3D.vertexBuffer[startVertexID].y;
    sVars->scene3D.vertexBuffer[vertexID].z = sVars->scene3D.vertexBuffer[startVertexID].z;
    sVars->scene3D.vertexBuffer[vertexID].u = 0;
    sVars->scene3D.vertexBuffer[vertexID].v = rotation;
    vertexID++;

    sVars->scene3D.faceBuffer[faceID].d     = vertexID;
    sVars->scene3D.vertexBuffer[vertexID].x = sVars->scene3D.vertexBuffer[startVertexID].x;
    sVars->scene3D.vertexBuffer[vertexID].y = sVars->scene3D.vertexBuffer[startVertexID].y;
    sVars->scene3D.vertexBuffer[vertexID].z = sVars->scene3D.vertexBuffer[startVertexID].z;
    sVars->scene3D.vertexBuffer[vertexID].u = scaleX;
    sVars->scene3D.vertexBuffer[vertexID].v = scaleY;
    vertexID++;

    sVars->scene3D.vertexCount += 4;
    sVars->scene3D.faceCount++;
}

void HP_Halfpipe::ProcessScanEdge(int32 x1, int32 y1, int32 x2, int32 y2)
{
    ScreenInfo *currentScreen = &screenInfo[sceneInfo->currentScreenID];

    int32 ix1 = FROM_FIXED(x1);
    int32 iy1 = FROM_FIXED(y1);
    int32 ix2 = FROM_FIXED(x2);
    int32 iy2 = FROM_FIXED(y2);

    int32 top = FROM_FIXED(y1);
    if (iy1 != iy2) {
        if (iy1 > iy2) {
            top = FROM_FIXED(y2);
            ix1 = FROM_FIXED(x2);
            ix2 = FROM_FIXED(x1);
            iy1 = FROM_FIXED(y2);
            iy2 = FROM_FIXED(y1);
        }

        int32 bottom = iy2 + 1;
        if (top < currentScreen->clipBound_Y2 && bottom >= currentScreen->clipBound_Y1) {
            if (bottom > currentScreen->clipBound_Y2)
                bottom = currentScreen->clipBound_Y2;
            int32 scanPos = TO_FIXED(ix1);
            int32 delta   = TO_FIXED(ix2 - ix1) / (iy2 - iy1);
            if (top < 0) {
                scanPos -= top * delta;
                top = 0;
            }

            ScanEdge *edge = &sVars->scanEdgeBuffer[top];
            for (int32 i = top; i < bottom; ++i) {
                int32 scanX = scanPos >> 16;
                if (scanX < edge->start)
                    edge->start = scanX;
                if (scanX > edge->end)
                    edge->end = scanX;
                scanPos += delta;
                ++edge;
            }
        }
    }
}

void HP_Halfpipe::ProcessScanEdgeUV(int32 x1, int32 y1, int32 u1, int32 v1, int32 x2, int32 y2, int32 u2, int32 v2)
{
    ScreenInfo *currentScreen = &screenInfo[sceneInfo->currentScreenID];

    int32 ix1 = FROM_FIXED(x1);
    int32 iy1 = FROM_FIXED(y1);
    int32 ix2 = FROM_FIXED(x2);
    int32 iy2 = FROM_FIXED(y2);
    int32 iu1 = u1;
    int32 iu2 = u2;
    int32 iv2 = v2 + 1; // may be temp but this works best for rn
    int32 iv1 = v1;

    if (iy1 != iy2) {
        if (iy1 > iy2) {
            ix1 = FROM_FIXED(x2);
            ix2 = FROM_FIXED(x1);
            iy1 = FROM_FIXED(y2);
            iy2 = FROM_FIXED(y1);
            iu1 = u2;
            iu2 = u1;
            iv1 = v2;
            iv2 = v1 + 1; // see above
        }

        int32 top    = iy1;
        int32 bottom = iy2 + 1;
        if (iy1 < currentScreen->clipBound_Y2 && bottom >= currentScreen->clipBound_Y1) {
            if (bottom > currentScreen->clipBound_Y2)
                bottom = currentScreen->clipBound_Y2;

            int32 scanPosX = ix1 << 16;
            int32 scanPosU = iu1 << 16;
            int32 scanPosV = iv1 << 16;
            int32 deltaX   = ((ix2 - ix1) << 16) / (iy2 - iy1);
            int32 deltaU   = ((iu2 - iu1) << 16) / (iy2 - iy1);
            int32 deltaV   = ((iv2 - iv1) << 16) / (iy2 - iy1);
            if (top < 0) {
                scanPosX -= top * deltaX;
                scanPosU -= top * deltaU;
                scanPosV -= top * deltaV;
                top = 0;
            }

            ScanEdge *edge = &sVars->scanEdgeBuffer[top];
            for (int32 i = top; i < bottom; ++i) {
                int32 scanX = scanPosX >> 16;
                if (scanX < edge->start) {
                    edge->start  = scanX;
                    edge->startU = scanPosU;
                    edge->startV = scanPosV;
                }
                if (scanX > edge->end) {
                    edge->end  = scanX;
                    edge->endU = scanPosU;
                    edge->endV = scanPosV;
                }
                scanPosX += deltaX;
                scanPosU += deltaU;
                scanPosV += deltaV;
                ++edge;
            }
        }
    }
}

void HP_Halfpipe::DrawFadedFace(RSDK::Vector2 *vertices, int32 vertCount, int32 r, int32 g, int32 b, int32 fogAlpha, int32 alpha, int32 inkEffect)
{
    ScreenInfo *currentScreen = &screenInfo[sceneInfo->currentScreenID];

    uint16 *tintLookupTable = Mod::Engine::GetTintLookupTable();
    uint16 maskColor        = Mod::Engine::GetMaskColor();

    switch (inkEffect) {
        default: break;
        case INK_ALPHA:
            if (alpha > 0xFF)
                inkEffect = INK_NONE;
            else if (alpha <= 0)
                return;
            break;

        case INK_ADD:
        case INK_SUB:
            if (alpha > 0xFF)
                alpha = 0xFF;
            else if (alpha <= 0)
                return;
            break;

        case INK_TINT:
            if (!tintLookupTable)
                return;
            break;
    }

    int32 top    = 0x7FFFFFFF;
    int32 bottom = -0x10000;
    for (int32 v = 0; v < vertCount; ++v) {
        if (vertices[v].y < top)
            top = vertices[v].y;
        if (vertices[v].y > bottom)
            bottom = vertices[v].y;
    }

    int32 topScreen    = FROM_FIXED(top);
    int32 bottomScreen = FROM_FIXED(bottom);

    if (topScreen < currentScreen->clipBound_Y1)
        topScreen = currentScreen->clipBound_Y1;
    if (topScreen > currentScreen->clipBound_Y2)
        topScreen = currentScreen->clipBound_Y2;

    if (bottomScreen < currentScreen->clipBound_Y1)
        bottomScreen = currentScreen->clipBound_Y1;
    if (bottomScreen > currentScreen->clipBound_Y2)
        bottomScreen = currentScreen->clipBound_Y2;

    if (topScreen != bottomScreen) {
        ScanEdge *edge = &sVars->scanEdgeBuffer[topScreen];
        for (int32 s = topScreen; s <= bottomScreen; ++s) {
            edge->start = 0x7FFF;
            edge->end   = -1;
            ++edge;
        }

        for (int32 v = 0; v < vertCount - 1; ++v) {
            ProcessScanEdge(vertices[v + 0].x, vertices[v + 0].y, vertices[v + 1].x, vertices[v + 1].y);
        }
        ProcessScanEdge(vertices[0].x, vertices[0].y, vertices[vertCount - 1].x, vertices[vertCount - 1].y);

        uint16 *frameBuffer = &currentScreen->frameBuffer[topScreen * currentScreen->pitch];

        color fogColor = sVars->scene3D.fogColor;

        uint16 color16 = sVars->rgb32To16_B[b] | sVars->rgb32To16_G[g] | sVars->rgb32To16_R[r];
        uint16 fogColor16 =
            sVars->rgb32To16_B[(fogColor >> 0) & 0xFF] | sVars->rgb32To16_G[(fogColor >> 8) & 0xFF] | sVars->rgb32To16_R[(fogColor >> 16) & 0xFF];

        edge = &sVars->scanEdgeBuffer[topScreen];

        uint16 *fog_fbufferBlend = &sVars->blendLookupTable[0x20 * (0xFF - fogAlpha)];
        uint16 *fog_pixelBlend   = &sVars->blendLookupTable[0x20 * fogAlpha];

        uint16 fadedColor = 0;
        setPixelFaded(color16, fogColor16, fadedColor, fogAlpha);

        switch (inkEffect) {
            default: break;

            case INK_NONE:
                for (int32 s = topScreen; s <= bottomScreen; ++s) {
                    if (edge->start < currentScreen->clipBound_X1)
                        edge->start = currentScreen->clipBound_X1;
                    if (edge->start > currentScreen->clipBound_X2)
                        edge->start = currentScreen->clipBound_X2;

                    if (edge->end < currentScreen->clipBound_X1)
                        edge->end = currentScreen->clipBound_X1;
                    if (edge->end > currentScreen->clipBound_X2)
                        edge->end = currentScreen->clipBound_X2;

                    int32 count = edge->end - edge->start;
                    for (int32 x = 0; x < count; ++x) {
                        frameBuffer[edge->start + x] = fadedColor;
                    }
                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;

            case INK_BLEND:
                for (int32 s = topScreen; s <= bottomScreen; ++s) {
                    if (edge->start < currentScreen->clipBound_X1)
                        edge->start = currentScreen->clipBound_X1;
                    if (edge->start > currentScreen->clipBound_X2)
                        edge->start = currentScreen->clipBound_X2;

                    if (edge->end < currentScreen->clipBound_X1)
                        edge->end = currentScreen->clipBound_X1;
                    if (edge->end > currentScreen->clipBound_X2)
                        edge->end = currentScreen->clipBound_X2;

                    int32 count = edge->end - edge->start;
                    for (int32 x = 0; x < count; ++x) {
                        setPixelBlend(fadedColor, frameBuffer[edge->start + x]);
                    }
                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;

            case INK_ALPHA: {
                uint16 *fbufferBlend = &sVars->blendLookupTable[0x20 * (0xFF - alpha)];
                uint16 *pixelBlend   = &sVars->blendLookupTable[0x20 * alpha];

                for (int32 s = topScreen; s <= bottomScreen; ++s) {
                    if (edge->start < currentScreen->clipBound_X1)
                        edge->start = currentScreen->clipBound_X1;
                    if (edge->start > currentScreen->clipBound_X2)
                        edge->start = currentScreen->clipBound_X2;

                    if (edge->end < currentScreen->clipBound_X1)
                        edge->end = currentScreen->clipBound_X1;
                    if (edge->end > currentScreen->clipBound_X2)
                        edge->end = currentScreen->clipBound_X2;

                    int32 count = edge->end - edge->start;
                    for (int32 x = 0; x < count; ++x) {
                        setPixelAlpha(fadedColor, frameBuffer[edge->start + x], alpha);
                    }
                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;
            }

            case INK_ADD: {
                uint16 *blendTablePtr = &sVars->blendLookupTable[0x20 * alpha];

                for (int32 s = topScreen; s <= bottomScreen; ++s) {
                    if (edge->start < currentScreen->clipBound_X1)
                        edge->start = currentScreen->clipBound_X1;
                    if (edge->start > currentScreen->clipBound_X2)
                        edge->start = currentScreen->clipBound_X2;

                    if (edge->end < currentScreen->clipBound_X1)
                        edge->end = currentScreen->clipBound_X1;
                    if (edge->end > currentScreen->clipBound_X2)
                        edge->end = currentScreen->clipBound_X2;

                    int32 count = edge->end - edge->start;
                    for (int32 x = 0; x < count; ++x) {
                        setPixelAdditive(fadedColor, frameBuffer[edge->start + x]);
                    }
                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;
            }

            case INK_SUB: {
                uint16 *subBlendTable = &sVars->subtractLookupTable[0x20 * alpha];

                for (int32 s = topScreen; s <= bottomScreen; ++s) {
                    if (edge->start < currentScreen->clipBound_X1)
                        edge->start = currentScreen->clipBound_X1;
                    if (edge->start > currentScreen->clipBound_X2)
                        edge->start = currentScreen->clipBound_X2;

                    if (edge->end < currentScreen->clipBound_X1)
                        edge->end = currentScreen->clipBound_X1;
                    if (edge->end > currentScreen->clipBound_X2)
                        edge->end = currentScreen->clipBound_X2;

                    int32 count = edge->end - edge->start;
                    for (int32 x = 0; x < count; ++x) {
                        setPixelSubtractive(fadedColor, frameBuffer[edge->start + x]);
                    }
                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;
            }

            case INK_TINT:
                for (int32 s = topScreen; s <= bottomScreen; ++s) {
                    if (edge->start < currentScreen->clipBound_X1)
                        edge->start = currentScreen->clipBound_X1;
                    if (edge->start > currentScreen->clipBound_X2)
                        edge->start = currentScreen->clipBound_X2;

                    if (edge->end < currentScreen->clipBound_X1)
                        edge->end = currentScreen->clipBound_X1;
                    if (edge->end > currentScreen->clipBound_X2)
                        edge->end = currentScreen->clipBound_X2;

                    int32 count = edge->end - edge->start;
                    for (int32 x = 0; x < count; ++x) {
                        frameBuffer[edge->start + x] = tintLookupTable[fadedColor];
                    }
                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;

            case INK_MASKED:
                for (int32 s = topScreen; s <= bottomScreen; ++s) {
                    if (edge->start < currentScreen->clipBound_X1)
                        edge->start = currentScreen->clipBound_X1;
                    if (edge->start > currentScreen->clipBound_X2)
                        edge->start = currentScreen->clipBound_X2;

                    if (edge->end < currentScreen->clipBound_X1)
                        edge->end = currentScreen->clipBound_X1;
                    if (edge->end > currentScreen->clipBound_X2)
                        edge->end = currentScreen->clipBound_X2;

                    int32 count = edge->end - edge->start;
                    for (int32 x = 0; x < count; ++x) {
                        if (frameBuffer[edge->start + x] == maskColor)
                            frameBuffer[edge->start + x] = fadedColor;
                    }
                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;

            case INK_UNMASKED:
                for (int32 s = topScreen; s <= bottomScreen; ++s) {
                    if (edge->start < currentScreen->clipBound_X1)
                        edge->start = currentScreen->clipBound_X1;
                    if (edge->start > currentScreen->clipBound_X2)
                        edge->start = currentScreen->clipBound_X2;

                    if (edge->end < currentScreen->clipBound_X1)
                        edge->end = currentScreen->clipBound_X1;
                    if (edge->end > currentScreen->clipBound_X2)
                        edge->end = currentScreen->clipBound_X2;

                    int32 count = edge->end - edge->start;
                    for (int32 x = 0; x < count; ++x) {
                        if (frameBuffer[edge->start + x] != maskColor)
                            frameBuffer[edge->start + x] = fadedColor;
                    }
                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;
        }
    }
}

void HP_Halfpipe::DrawTexturedFace(RSDK::Vector2 *vertices, RSDK::Vector2 *vertexUVs, int32 vertCount, RSDK::Animator *animator, int32 alpha,
                                   int32 inkEffect)
{
    ScreenInfo *currentScreen = &screenInfo[sceneInfo->currentScreenID];

    uint16 *tintLookupTable = Mod::Engine::GetTintLookupTable();
    uint16 maskColor        = Mod::Engine::GetMaskColor();

    switch (inkEffect) {
        default: break;
        case INK_ALPHA:
            if (alpha > 0xFF)
                inkEffect = INK_NONE;
            else if (alpha <= 0)
                return;
            break;

        case INK_ADD:
        case INK_SUB:
            if (alpha > 0xFF)
                alpha = 0xFF;
            else if (alpha <= 0)
                return;
            break;

        case INK_TINT:
            if (!tintLookupTable)
                return;
            break;
    }

    SpriteFrame *frame = &animator->frames[animator->frameID];
    GFXSurface *sheet  = (GFXSurface *)Mod::Engine::GetSpriteSurface(frame->sheetID);

    int32 top    = 0x7FFFFFFF;
    int32 bottom = -0x10000;
    for (int32 v = 0; v < vertCount; ++v) {
        if (vertices[v].y < top)
            top = vertices[v].y;
        if (vertices[v].y > bottom)
            bottom = vertices[v].y;
    }

    int32 topScreen    = FROM_FIXED(top);
    int32 bottomScreen = FROM_FIXED(bottom);

    if (topScreen < currentScreen->clipBound_Y1)
        topScreen = currentScreen->clipBound_Y1;
    if (topScreen > currentScreen->clipBound_Y2)
        topScreen = currentScreen->clipBound_Y2;

    if (bottomScreen < currentScreen->clipBound_Y1)
        bottomScreen = currentScreen->clipBound_Y1;
    if (bottomScreen > currentScreen->clipBound_Y2)
        bottomScreen = currentScreen->clipBound_Y2;

    if (topScreen != bottomScreen) {
        ScanEdge *edge = &sVars->scanEdgeBuffer[topScreen];
        for (int32 s = topScreen; s <= bottomScreen; ++s) {
            edge->start = 0x7FFF;
            edge->end   = -1;
            ++edge;
        }

        for (int32 v = 0; v < vertCount - 1; ++v) {
            ProcessScanEdgeUV(vertices[v + 0].x, vertices[v + 0].y, vertexUVs[v + 0].x, vertexUVs[v + 0].y, vertices[v + 1].x, vertices[v + 1].y,
                              vertexUVs[v + 1].x, vertexUVs[v + 1].y);
        }
        ProcessScanEdgeUV(vertices[0].x, vertices[0].y, vertexUVs[0].x, vertexUVs[0].y, vertices[vertCount - 1].x, vertices[vertCount - 1].y,
                          vertexUVs[vertCount - 1].x, vertexUVs[vertCount - 1].y);

        uint16 *frameBuffer = &currentScreen->frameBuffer[topScreen * currentScreen->pitch];

        edge = &sVars->scanEdgeBuffer[topScreen];

        uint16 *fbufferBlend = &sVars->blendLookupTable[0x20 * (0xFF - alpha)];
        uint16 *pixelBlend   = &sVars->blendLookupTable[0x20 * alpha];

        uint8 *gfxLineBuffer = Mod::Engine::GetActivePaletteBuffer();

        switch (inkEffect) {
            default: break;

            case INK_NONE:
                for (int32 s = topScreen; s < bottomScreen; ++s) {
                    uint16 *palettePtr = Mod::Engine::GetPaletteBank(gfxLineBuffer[s]);

                    if (edge->start < currentScreen->clipBound_X2 && edge->end > currentScreen->clipBound_X1) {
                        int32 count = edge->end - edge->start;

                        int32 deltaU = 0;
                        int32 deltaV = 0;
                        if (count) {
                            deltaU = (edge->endU - edge->startU) / count;
                            deltaV = (edge->endV - edge->startV) / count;
                        }

                        int32 u = edge->startU;
                        int32 v = edge->startV;
                        if (edge->end > currentScreen->clipBound_X2)
                            count = currentScreen->clipBound_X2 - edge->start;

                        int32 start = edge->start;
                        if (start < currentScreen->clipBound_X1) {
                            count += start;
                            u -= start * deltaU;
                            v -= start * deltaV;
                            start = currentScreen->clipBound_X1;
                        }

                        int32 x = 0;
                        while (x < count) {
                            if (u < 0)
                                u = 0;
                            if (v < 0)
                                v = 0;

                            uint16 index = sheet->pixels[((v >> 16) << sheet->lineSize) + (u >> 16)];
                            if (index)
                                frameBuffer[start + x] = palettePtr[index];
                            ++x;
                            u += deltaU;
                            v += deltaV;
                        }
                    }

                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;

            case INK_BLEND:
                for (int32 s = topScreen; s < bottomScreen; ++s) {
                    uint16 *palettePtr = Mod::Engine::GetPaletteBank(gfxLineBuffer[s]);

                    if (edge->start < currentScreen->clipBound_X2 && edge->end > currentScreen->clipBound_X1) {
                        int32 count = edge->end - edge->start;

                        int32 deltaU = 0;
                        int32 deltaV = 0;
                        if (count) {
                            deltaU = (edge->endU - edge->startU) / count;
                            deltaV = (edge->endV - edge->startV) / count;
                        }

                        int32 u = edge->startU;
                        int32 v = edge->startV;
                        if (edge->end > currentScreen->clipBound_X2)
                            count = currentScreen->clipBound_X2 - edge->start;

                        int32 start = edge->start;
                        if (start < currentScreen->clipBound_X1) {
                            count += start;
                            u -= start * deltaU;
                            v -= start * deltaV;
                            start = currentScreen->clipBound_X1;
                        }

                        int32 x = 0;
                        while (x < count) {
                            if (u < 0)
                                u = 0;
                            if (v < 0)
                                v = 0;

                            uint16 index = sheet->pixels[((v >> 16) << sheet->lineSize) + (u >> 16)];
                            if (index)
                                setPixelBlend(palettePtr[index], frameBuffer[start + x]);
                            ++x;
                            u += deltaU;
                            v += deltaV;
                        }
                    }

                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;

            case INK_ALPHA: {
                uint16 *fbufferBlend = &sVars->blendLookupTable[0x20 * (0xFF - alpha)];
                uint16 *pixelBlend   = &sVars->blendLookupTable[0x20 * alpha];

                for (int32 s = topScreen; s < bottomScreen; ++s) {
                    uint16 *palettePtr = Mod::Engine::GetPaletteBank(gfxLineBuffer[s]);

                    if (edge->start < currentScreen->clipBound_X2 && edge->end > currentScreen->clipBound_X1) {
                        int32 count = edge->end - edge->start;

                        int32 deltaU = 0;
                        int32 deltaV = 0;
                        if (count) {
                            deltaU = (edge->endU - edge->startU) / count;
                            deltaV = (edge->endV - edge->startV) / count;
                        }

                        int32 u = edge->startU;
                        int32 v = edge->startV;
                        if (edge->end > currentScreen->clipBound_X2)
                            count = currentScreen->clipBound_X2 - edge->start;

                        int32 start = edge->start;
                        if (start < currentScreen->clipBound_X1) {
                            count += start;
                            u -= start * deltaU;
                            v -= start * deltaV;
                            start = currentScreen->clipBound_X1;
                        }

                        int32 x = 0;
                        while (x < count) {
                            if (u < 0)
                                u = 0;
                            if (v < 0)
                                v = 0;

                            uint16 index = sheet->pixels[((v >> 16) << sheet->lineSize) + (u >> 16)];
                            if (index) {
                                setPixelAlpha(palettePtr[index], frameBuffer[start + x], alpha);
                            }
                            ++x;
                            u += deltaU;
                            v += deltaV;
                        }
                    }

                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;
            }

            case INK_ADD: {
                uint16 *blendTablePtr = &sVars->blendLookupTable[0x20 * alpha];

                for (int32 s = topScreen; s < bottomScreen; ++s) {
                    uint16 *palettePtr = Mod::Engine::GetPaletteBank(gfxLineBuffer[s]);

                    if (edge->start < currentScreen->clipBound_X2 && edge->end > currentScreen->clipBound_X1) {
                        int32 count = edge->end - edge->start;

                        int32 deltaU = 0;
                        int32 deltaV = 0;
                        if (count) {
                            deltaU = (edge->endU - edge->startU) / count;
                            deltaV = (edge->endV - edge->startV) / count;
                        }

                        int32 u = edge->startU;
                        int32 v = edge->startV;
                        if (edge->end > currentScreen->clipBound_X2)
                            count = currentScreen->clipBound_X2 - edge->start;

                        int32 start = edge->start;
                        if (start < currentScreen->clipBound_X1) {
                            count += start;
                            u -= start * deltaU;
                            v -= start * deltaV;
                            start = currentScreen->clipBound_X1;
                        }

                        int32 x = 0;
                        while (x < count) {
                            if (u < 0)
                                u = 0;
                            if (v < 0)
                                v = 0;

                            uint16 index = sheet->pixels[((v >> 16) << sheet->lineSize) + (u >> 16)];
                            if (index) {
                                setPixelAdditive(palettePtr[index], frameBuffer[start + x]);
                            }
                            ++x;
                            u += deltaU;
                            v += deltaV;
                        }
                    }

                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;
            }

            case INK_SUB: {
                uint16 *subBlendTable = &sVars->subtractLookupTable[0x20 * alpha];

                for (int32 s = topScreen; s < bottomScreen; ++s) {
                    uint16 *palettePtr = Mod::Engine::GetPaletteBank(gfxLineBuffer[s]);

                    if (edge->start < currentScreen->clipBound_X2 && edge->end > currentScreen->clipBound_X1) {
                        int32 count = edge->end - edge->start;

                        int32 deltaU = 0;
                        int32 deltaV = 0;
                        if (count) {
                            deltaU = (edge->endU - edge->startU) / count;
                            deltaV = (edge->endV - edge->startV) / count;
                        }

                        int32 u = edge->startU;
                        int32 v = edge->startV;
                        if (edge->end > currentScreen->clipBound_X2)
                            count = currentScreen->clipBound_X2 - edge->start;

                        int32 start = edge->start;
                        if (start < currentScreen->clipBound_X1) {
                            count += start;
                            u -= start * deltaU;
                            v -= start * deltaV;
                            start = currentScreen->clipBound_X1;
                        }

                        int32 x = 0;
                        while (x < count) {
                            if (u < 0)
                                u = 0;
                            if (v < 0)
                                v = 0;

                            uint16 index = sheet->pixels[((v >> 16) << sheet->lineSize) + (u >> 16)];
                            if (index) {
                                setPixelSubtractive(palettePtr[index], frameBuffer[start + x]);
                            }
                            ++x;
                            u += deltaU;
                            v += deltaV;
                        }
                    }

                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;
            }

            case INK_TINT:
                for (int32 s = topScreen; s < bottomScreen; ++s) {
                    uint16 *palettePtr = Mod::Engine::GetPaletteBank(gfxLineBuffer[s]);

                    if (edge->start < currentScreen->clipBound_X2 && edge->end > currentScreen->clipBound_X1) {
                        int32 count = edge->end - edge->start;

                        int32 deltaU = 0;
                        int32 deltaV = 0;
                        if (count) {
                            deltaU = (edge->endU - edge->startU) / count;
                            deltaV = (edge->endV - edge->startV) / count;
                        }

                        int32 u = edge->startU;
                        int32 v = edge->startV;
                        if (edge->end > currentScreen->clipBound_X2)
                            count = currentScreen->clipBound_X2 - edge->start;

                        int32 start = edge->start;
                        if (start < currentScreen->clipBound_X1) {
                            count += start;
                            u -= start * deltaU;
                            v -= start * deltaV;
                            start = currentScreen->clipBound_X1;
                        }

                        int32 x = 0;
                        while (x < count) {
                            if (u < 0)
                                u = 0;
                            if (v < 0)
                                v = 0;

                            uint16 index = sheet->pixels[((v >> 16) << sheet->lineSize) + (u >> 16)];
                            if (index)
                                frameBuffer[start + x] = tintLookupTable[frameBuffer[start + x]];
                            ++x;
                            u += deltaU;
                            v += deltaV;
                        }
                    }

                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;

            case INK_MASKED:
                for (int32 s = topScreen; s < bottomScreen; ++s) {
                    uint16 *palettePtr = Mod::Engine::GetPaletteBank(gfxLineBuffer[s]);

                    if (edge->start < currentScreen->clipBound_X2 && edge->end > currentScreen->clipBound_X1) {
                        int32 count = edge->end - edge->start;

                        int32 deltaU = 0;
                        int32 deltaV = 0;
                        if (count) {
                            deltaU = (edge->endU - edge->startU) / count;
                            deltaV = (edge->endV - edge->startV) / count;
                        }

                        int32 u = edge->startU;
                        int32 v = edge->startV;
                        if (edge->end > currentScreen->clipBound_X2)
                            count = currentScreen->clipBound_X2 - edge->start;

                        int32 start = edge->start;
                        if (start < currentScreen->clipBound_X1) {
                            count += start;
                            u -= start * deltaU;
                            v -= start * deltaV;
                            start = currentScreen->clipBound_X1;
                        }

                        int32 x = 0;
                        while (x < count) {
                            if (u < 0)
                                u = 0;
                            if (v < 0)
                                v = 0;

                            uint16 index = sheet->pixels[((v >> 16) << sheet->lineSize) + (u >> 16)];
                            if (index && frameBuffer[edge->start + x] == maskColor)
                                frameBuffer[start + x] = palettePtr[index];
                            ++x;
                            u += deltaU;
                            v += deltaV;
                        }
                    }

                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;

            case INK_UNMASKED:
                for (int32 s = topScreen; s < bottomScreen; ++s) {
                    uint16 *palettePtr = Mod::Engine::GetPaletteBank(gfxLineBuffer[s]);

                    if (edge->start < currentScreen->clipBound_X2 && edge->end > currentScreen->clipBound_X1) {
                        int32 count = edge->end - edge->start;

                        int32 deltaU = 0;
                        int32 deltaV = 0;
                        if (count) {
                            deltaU = (edge->endU - edge->startU) / count;
                            deltaV = (edge->endV - edge->startV) / count;
                        }

                        int32 u = edge->startU;
                        int32 v = edge->startV;
                        if (edge->end > currentScreen->clipBound_X2)
                            count = currentScreen->clipBound_X2 - edge->start;

                        int32 start = edge->start;
                        if (start < currentScreen->clipBound_X1) {
                            count += start;
                            u -= start * deltaU;
                            v -= start * deltaV;
                            start = currentScreen->clipBound_X1;
                        }

                        int32 x = 0;
                        while (x < count) {
                            if (u < 0)
                                u = 0;
                            if (v < 0)
                                v = 0;

                            uint16 index = sheet->pixels[((v >> 16) << sheet->lineSize) + (u >> 16)];
                            if (index && frameBuffer[edge->start + x] != maskColor)
                                frameBuffer[start + x] = palettePtr[index];
                            ++x;
                            u += deltaU;
                            v += deltaV;
                        }
                    }

                    ++edge;
                    frameBuffer += currentScreen->pitch;
                }
                break;
        }
    }
}

void HP_Halfpipe::TransformVertices(RSDK::Matrix *matrix, Vertex *vertices, int32 startIndex, int32 count)
{
    RSDK::Matrix *m = matrix;
    for (int32 v = startIndex; v < startIndex + count; ++v) {
        int32 vx = vertices[v].x;
        int32 vy = vertices[v].y;
        int32 vz = vertices[v].z;

        vertices[v].x = (vx * m->values[0][0] >> 8) + (vy * m->values[1][0] >> 8) + (vz * m->values[2][0] >> 8) + m->values[3][0];
        vertices[v].y = (vx * m->values[0][1] >> 8) + (vy * m->values[1][1] >> 8) + (vz * m->values[2][1] >> 8) + m->values[3][1];
        vertices[v].z = (vx * m->values[0][2] >> 8) + (vy * m->values[1][2] >> 8) + (vz * m->values[2][2] >> 8) + m->values[3][2];
    }
}

void HP_Halfpipe::TransformVertexBuffer()
{
    Matrix matFinal;
    memcpy(&matFinal, &sVars->scene3D.matWorld, sizeof(matFinal));

    MatrixMultiply(&matFinal, &matFinal, &sVars->scene3D.matView);

    RSDK::Matrix *m = &matFinal;
    for (int32 v = 0; v < sVars->scene3D.vertexCount; ++v) {
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

    Vertex *vertexBufferT = sVars->scene3D.vertexBufferT;
    Vertex *vertexBuffer  = sVars->scene3D.vertexBuffer;

    for (int32 i = 0; i < sVars->scene3D.faceCount; ++i) {
        Face *face = &sVars->scene3D.faceBuffer[i];

        sVars->scene3D.drawList[i].depth =
            (vertexBufferT[face->d].z + vertexBufferT[face->c].z + vertexBufferT[face->b].z + vertexBufferT[face->a].z) >> 2;
        sVars->scene3D.drawList[i].index = i;
    }

    // slow & clunky but works ig LOL
    for (int32 i = 0; i < sVars->scene3D.faceCount; ++i) {
        for (int32 j = sVars->scene3D.faceCount - 1; j > i; --j) {
            if (sVars->scene3D.drawList[j].depth > sVars->scene3D.drawList[j - 1].depth) {
                int32 index                           = sVars->scene3D.drawList[j].index;
                int32 depth                           = sVars->scene3D.drawList[j].depth;
                sVars->scene3D.drawList[j].index      = sVars->scene3D.drawList[j - 1].index;
                sVars->scene3D.drawList[j].depth      = sVars->scene3D.drawList[j - 1].depth;
                sVars->scene3D.drawList[j - 1].index  = index;
                sVars->scene3D.drawList[j - 1].depth  = depth;
            }
        }
    }

    // faster but doesn't work as well :(
    // SortDrawList(0, sVars->scene3D.faceCount - 1);

    RSDK::Vector2 faceVerts[4];
    RSDK::Vector2 faceUVs[4];
    for (int32 i = 0; i < sVars->scene3D.faceCount; ++i) {
        Face *face = &sVars->scene3D.faceBuffer[sVars->scene3D.drawList[i].index];
        memset(faceVerts, 0, sizeof(faceVerts));
        memset(faceUVs, 0, sizeof(faceUVs));

        if (sVars->scene3D.drawList[i].index == 70 || sVars->scene3D.drawList[i].index == 71)
            printf("");

        switch (face->flag) {
            default: break;

            case HP_Halfpipe::FaceTextured3D:
                if (vertexBufferT[face->a].z > 0 && vertexBufferT[face->b].z > 0 && vertexBufferT[face->c].z > 0 && vertexBufferT[face->d].z > 0) {
                    faceVerts[0].x = TO_FIXED(screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->a].x / vertexBufferT[face->a].z);
                    faceVerts[0].y = TO_FIXED(screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->a].y / vertexBufferT[face->a].z);
                    faceVerts[1].x = TO_FIXED(screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->b].x / vertexBufferT[face->b].z);
                    faceVerts[1].y = TO_FIXED(screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->b].y / vertexBufferT[face->b].z);
                    faceVerts[3].x = TO_FIXED(screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->c].x / vertexBufferT[face->c].z);
                    faceVerts[3].y = TO_FIXED(screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->c].y / vertexBufferT[face->c].z);
                    faceVerts[2].x = TO_FIXED(screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->d].x / vertexBufferT[face->d].z);
                    faceVerts[2].y = TO_FIXED(screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->d].y / vertexBufferT[face->d].z);

                    faceUVs[0].x = vertexBuffer[face->a].u;
                    faceUVs[0].y = vertexBuffer[face->a].v;
                    faceUVs[1].x = vertexBuffer[face->b].u;
                    faceUVs[1].y = vertexBuffer[face->b].v;
                    faceUVs[3].x = vertexBuffer[face->c].u;
                    faceUVs[3].y = vertexBuffer[face->c].v;
                    faceUVs[2].x = vertexBuffer[face->d].u;
                    faceUVs[2].y = vertexBuffer[face->d].v;

                    this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
                    this->DrawTexturedFace(faceVerts, faceUVs, 4, &this->animator, 0xFF, INK_NONE);
                }
                break;

            case HP_Halfpipe::FaceTextured2D:
                if (vertexBufferT[face->a].z >= 0 && vertexBufferT[face->b].z >= 0 && vertexBufferT[face->c].z >= 0
                    && vertexBufferT[face->d].z >= 0) {
                    faceVerts[0].x = TO_FIXED(vertexBufferT[face->a].x);
                    faceVerts[0].y = TO_FIXED(vertexBufferT[face->a].y);
                    faceVerts[1].x = TO_FIXED(vertexBufferT[face->b].x);
                    faceVerts[1].y = TO_FIXED(vertexBufferT[face->b].y);
                    faceVerts[3].x = TO_FIXED(vertexBufferT[face->c].x);
                    faceVerts[3].y = TO_FIXED(vertexBufferT[face->c].y);
                    faceVerts[2].x = TO_FIXED(vertexBufferT[face->d].x);
                    faceVerts[2].y = TO_FIXED(vertexBufferT[face->d].y);

                    faceUVs[0].x = vertexBuffer[face->a].u;
                    faceUVs[0].y = vertexBuffer[face->a].v;
                    faceUVs[1].x = vertexBuffer[face->b].u;
                    faceUVs[1].y = vertexBuffer[face->b].v;
                    faceUVs[3].x = vertexBuffer[face->c].u;
                    faceUVs[3].y = vertexBuffer[face->c].v;
                    faceUVs[2].x = vertexBuffer[face->d].u;
                    faceUVs[2].y = vertexBuffer[face->d].v;

                    this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
                    this->DrawTexturedFace(faceVerts, faceUVs, 4, &this->animator, 0xFF, INK_NONE);
                }
                break;

            case HP_Halfpipe::FaceColored3D:
                if (vertexBufferT[face->a].z > 0 && vertexBufferT[face->b].z > 0 && vertexBufferT[face->c].z > 0 && vertexBufferT[face->d].z > 0) {
                    faceVerts[0].x = TO_FIXED(screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->a].x / vertexBufferT[face->a].z);
                    faceVerts[0].y = TO_FIXED(screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->a].y / vertexBufferT[face->a].z);
                    faceVerts[1].x = TO_FIXED(screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->b].x / vertexBufferT[face->b].z);
                    faceVerts[1].y = TO_FIXED(screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->b].y / vertexBufferT[face->b].z);
                    faceVerts[2].x = TO_FIXED(screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->c].x / vertexBufferT[face->c].z);
                    faceVerts[2].y = TO_FIXED(screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->c].y / vertexBufferT[face->c].z);
                    faceVerts[3].x = TO_FIXED(screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->d].x / vertexBufferT[face->d].z);
                    faceVerts[3].y = TO_FIXED(screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->d].y / vertexBufferT[face->d].z);

                    Graphics::DrawFace(faceVerts, 4, (face->color >> 16) & 0xFF, (face->color >> 8) & 0xFF, (face->color >> 0) & 0xFF,
                                       (face->color >> 24) & 0xFF, INK_ALPHA);
                }
                break;

            case HP_Halfpipe::FaceColored2D:
                if (vertexBufferT[face->a].z >= 0 && vertexBufferT[face->b].z >= 0 && vertexBufferT[face->c].z >= 0
                    && vertexBufferT[face->d].z >= 0) {
                    faceVerts[0].x = TO_FIXED(vertexBufferT[face->a].x);
                    faceVerts[0].y = TO_FIXED(vertexBufferT[face->a].y);
                    faceVerts[1].x = TO_FIXED(vertexBufferT[face->b].x);
                    faceVerts[1].y = TO_FIXED(vertexBufferT[face->b].y);
                    faceVerts[2].x = TO_FIXED(vertexBufferT[face->c].x);
                    faceVerts[2].y = TO_FIXED(vertexBufferT[face->c].y);
                    faceVerts[3].x = TO_FIXED(vertexBufferT[face->d].x);
                    faceVerts[3].y = TO_FIXED(vertexBufferT[face->d].y);

                    Graphics::DrawFace(faceVerts, 4, (face->color >> 16) & 0xFF, (face->color >> 8) & 0xFF, (face->color >> 0) & 0xFF,
                                       (face->color >> 24) & 0xFF, INK_ALPHA);
                }
                break;

            case HP_Halfpipe::FaceFaded:
                if (vertexBufferT[face->a].z > 0 && vertexBufferT[face->b].z > 0 && vertexBufferT[face->c].z > 0 && vertexBufferT[face->d].z > 0) {
                    faceVerts[0].x = screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->a].x / vertexBufferT[face->a].z;
                    faceVerts[0].y = screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->a].y / vertexBufferT[face->a].z;
                    faceVerts[1].x = screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->b].x / vertexBufferT[face->b].z;
                    faceVerts[1].y = screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->b].y / vertexBufferT[face->b].z;
                    faceVerts[2].x = screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->c].x / vertexBufferT[face->c].z;
                    faceVerts[2].y = screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->c].y / vertexBufferT[face->c].z;
                    faceVerts[3].x = screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->d].x / vertexBufferT[face->d].z;
                    faceVerts[3].y = screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->d].y / vertexBufferT[face->d].z;

                    // gotta do a few ugly checks here to get rid of some invalid stuff that'd show up otherwise
                    if (faceVerts[0].x < screen->clipBound_X1 && faceVerts[1].x < screen->clipBound_X1 && faceVerts[2].x < screen->clipBound_X1
                        && faceVerts[3].x < screen->clipBound_X1)
                        continue;

                    if (faceVerts[0].x > screen->clipBound_X2 && faceVerts[1].x > screen->clipBound_X2 && faceVerts[2].x > screen->clipBound_X2
                        && faceVerts[3].x > screen->clipBound_X2)
                        continue;

                    if (faceVerts[0].y < screen->clipBound_Y1 && faceVerts[1].y < screen->clipBound_Y1 && faceVerts[2].y < screen->clipBound_Y1
                        && faceVerts[3].y < screen->clipBound_Y1)
                        continue;

                    if (faceVerts[0].y > screen->clipBound_Y2 && faceVerts[1].y > screen->clipBound_Y2 && faceVerts[2].y > screen->clipBound_Y2
                        && faceVerts[3].y > screen->clipBound_Y2)
                        continue;

                    if (faceVerts[0].x == faceVerts[1].x && faceVerts[1].x == faceVerts[2].x && faceVerts[2].x == faceVerts[3].x)
                        continue;

                    if (faceVerts[0].y == faceVerts[1].y && faceVerts[1].y == faceVerts[2].y && faceVerts[2].y == faceVerts[3].y)
                        continue;

                    // convert to fixed point and go wild
                    faceVerts[0].x = TO_FIXED(faceVerts[0].x);
                    faceVerts[0].y = TO_FIXED(faceVerts[0].y);
                    faceVerts[1].x = TO_FIXED(faceVerts[1].x);
                    faceVerts[1].y = TO_FIXED(faceVerts[1].y);
                    faceVerts[2].x = TO_FIXED(faceVerts[2].x);
                    faceVerts[2].y = TO_FIXED(faceVerts[2].y);
                    faceVerts[3].x = TO_FIXED(faceVerts[3].x);
                    faceVerts[3].y = TO_FIXED(faceVerts[3].y);

                    int32 fogStrength = CLAMP((sVars->scene3D.drawList[i].depth - 0x8000) >> 8, 0, sVars->scene3D.fogStrength);

                    this->DrawFadedFace(faceVerts, 4, (face->color >> 16) & 0xFF, (face->color >> 8) & 0xFF, (face->color >> 0) & 0xFF,
                                        0xFF - fogStrength, 0xFF, INK_NONE);
                }
                break;

            case HP_Halfpipe::FaceTexturedC:
                if (vertexBufferT[face->a].z > 0) {
                    // [face->a].uv == sprite center
                    // [face->b].uv == ???
                    // [face->c].uv == sprite extend (how far to each edge X & Y)
                    // [face->d].uv == unused

                    faceVerts[0].x = screen->center.x
                                     + sVars->scene3D.projectionX * (vertexBufferT[face->a].x - vertexBuffer[face->b].u) / vertexBufferT[face->a].z;
                    faceVerts[0].y = screen->center.y
                                     - sVars->scene3D.projectionY * (vertexBufferT[face->a].y + vertexBuffer[face->b].v) / vertexBufferT[face->a].z;
                    faceVerts[1].x = screen->center.x
                                     + sVars->scene3D.projectionX * (vertexBufferT[face->a].x + vertexBuffer[face->b].u) / vertexBufferT[face->a].z;
                    faceVerts[1].y = screen->center.y
                                     - sVars->scene3D.projectionY * (vertexBufferT[face->a].y + vertexBuffer[face->b].v) / vertexBufferT[face->a].z;
                    faceVerts[3].x = screen->center.x
                                     + sVars->scene3D.projectionX * (vertexBufferT[face->a].x - vertexBuffer[face->b].u) / vertexBufferT[face->a].z;
                    faceVerts[3].y = screen->center.y
                                     - sVars->scene3D.projectionY * (vertexBufferT[face->a].y - vertexBuffer[face->b].v) / vertexBufferT[face->a].z;
                    faceVerts[2].x = screen->center.x
                                     + sVars->scene3D.projectionX * (vertexBufferT[face->a].x + vertexBuffer[face->b].u) / vertexBufferT[face->a].z;
                    faceVerts[2].y = screen->center.y
                                     - sVars->scene3D.projectionY * (vertexBufferT[face->a].y - vertexBuffer[face->b].v) / vertexBufferT[face->a].z;

                    faceUVs[0].x = vertexBuffer[face->a].u - vertexBuffer[face->c].u;
                    faceUVs[0].y = vertexBuffer[face->a].v - vertexBuffer[face->c].v;
                    faceUVs[1].x = vertexBuffer[face->a].u + vertexBuffer[face->c].u;
                    faceUVs[1].y = vertexBuffer[face->a].v - vertexBuffer[face->c].v;
                    faceUVs[3].x = vertexBuffer[face->a].u - vertexBuffer[face->c].u;
                    faceUVs[3].y = vertexBuffer[face->a].v + vertexBuffer[face->c].v;
                    faceUVs[2].x = vertexBuffer[face->a].u + vertexBuffer[face->c].u;
                    faceUVs[2].y = vertexBuffer[face->a].v + vertexBuffer[face->c].v;

                    // gotta do a few ugly checks here to get rid of some invalid stuff that'd show up otherwise
                    if (faceVerts[0].x < screen->clipBound_X1 && faceVerts[1].x < screen->clipBound_X1 && faceVerts[2].x < screen->clipBound_X1
                        && faceVerts[3].x < screen->clipBound_X1)
                        continue;

                    if (faceVerts[0].x > screen->clipBound_X2 && faceVerts[1].x > screen->clipBound_X2 && faceVerts[2].x > screen->clipBound_X2
                        && faceVerts[3].x > screen->clipBound_X2)
                        continue;

                    if (faceVerts[0].y < screen->clipBound_Y1 && faceVerts[1].y < screen->clipBound_Y1 && faceVerts[2].y < screen->clipBound_Y1
                        && faceVerts[3].y < screen->clipBound_Y1)
                        continue;

                    if (faceVerts[0].y > screen->clipBound_Y2 && faceVerts[1].y > screen->clipBound_Y2 && faceVerts[2].y > screen->clipBound_Y2
                        && faceVerts[3].y > screen->clipBound_Y2)
                        continue;

                    if (faceVerts[0].x == faceVerts[1].x && faceVerts[1].x == faceVerts[2].x && faceVerts[2].x == faceVerts[3].x)
                        continue;

                    if (faceVerts[0].y == faceVerts[1].y && faceVerts[1].y == faceVerts[2].y && faceVerts[2].y == faceVerts[3].y)
                        continue;

                    // convert to fixed point and go wild
                    faceVerts[0].x = TO_FIXED(faceVerts[0].x);
                    faceVerts[0].y = TO_FIXED(faceVerts[0].y);
                    faceVerts[1].x = TO_FIXED(faceVerts[1].x);
                    faceVerts[1].y = TO_FIXED(faceVerts[1].y);
                    faceVerts[2].x = TO_FIXED(faceVerts[2].x);
                    faceVerts[2].y = TO_FIXED(faceVerts[2].y);
                    faceVerts[3].x = TO_FIXED(faceVerts[3].x);
                    faceVerts[3].y = TO_FIXED(faceVerts[3].y);

                    this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
                    this->DrawTexturedFace(faceVerts, faceUVs, 4, &this->animator, 0xFF, INK_NONE);
                }
                break;

            case HP_Halfpipe::FaceTexturedC_Blend:
                if (vertexBufferT[face->a].z > 0) {
                    // See above, its the same just blended

                    faceVerts[0].x = screen->center.x
                                     + sVars->scene3D.projectionX * (vertexBufferT[face->a].x - vertexBuffer[face->b].u) / vertexBufferT[face->a].z;
                    faceVerts[0].y = screen->center.y
                                     - sVars->scene3D.projectionY * (vertexBufferT[face->a].y + vertexBuffer[face->b].v) / vertexBufferT[face->a].z;
                    faceVerts[1].x = screen->center.x
                                     + sVars->scene3D.projectionX * (vertexBufferT[face->a].x + vertexBuffer[face->b].u) / vertexBufferT[face->a].z;
                    faceVerts[1].y = screen->center.y
                                     - sVars->scene3D.projectionY * (vertexBufferT[face->a].y + vertexBuffer[face->b].v) / vertexBufferT[face->a].z;
                    faceVerts[3].x = screen->center.x
                                     + sVars->scene3D.projectionX * (vertexBufferT[face->a].x - vertexBuffer[face->b].u) / vertexBufferT[face->a].z;
                    faceVerts[3].y = screen->center.y
                                     - sVars->scene3D.projectionY * (vertexBufferT[face->a].y - vertexBuffer[face->b].v) / vertexBufferT[face->a].z;
                    faceVerts[2].x = screen->center.x
                                     + sVars->scene3D.projectionX * (vertexBufferT[face->a].x + vertexBuffer[face->b].u) / vertexBufferT[face->a].z;
                    faceVerts[2].y = screen->center.y
                                     - sVars->scene3D.projectionY * (vertexBufferT[face->a].y - vertexBuffer[face->b].v) / vertexBufferT[face->a].z;

                    faceUVs[0].x = vertexBuffer[face->a].u - vertexBuffer[face->c].u;
                    faceUVs[0].y = vertexBuffer[face->a].v - vertexBuffer[face->c].v;
                    faceUVs[1].x = vertexBuffer[face->a].u + vertexBuffer[face->c].u;
                    faceUVs[1].y = vertexBuffer[face->a].v - vertexBuffer[face->c].v;
                    faceUVs[3].x = vertexBuffer[face->a].u - vertexBuffer[face->c].u;
                    faceUVs[3].y = vertexBuffer[face->a].v + vertexBuffer[face->c].v;
                    faceUVs[2].x = vertexBuffer[face->a].u + vertexBuffer[face->c].u;
                    faceUVs[2].y = vertexBuffer[face->a].v + vertexBuffer[face->c].v;

                    // gotta do a few ugly checks here to get rid of some invalid stuff that'd show up otherwise
                    if (faceVerts[0].x < screen->clipBound_X1 && faceVerts[1].x < screen->clipBound_X1 && faceVerts[2].x < screen->clipBound_X1
                        && faceVerts[3].x < screen->clipBound_X1)
                        continue;

                    if (faceVerts[0].x > screen->clipBound_X2 && faceVerts[1].x > screen->clipBound_X2 && faceVerts[2].x > screen->clipBound_X2
                        && faceVerts[3].x > screen->clipBound_X2)
                        continue;

                    if (faceVerts[0].y < screen->clipBound_Y1 && faceVerts[1].y < screen->clipBound_Y1 && faceVerts[2].y < screen->clipBound_Y1
                        && faceVerts[3].y < screen->clipBound_Y1)
                        continue;

                    if (faceVerts[0].y > screen->clipBound_Y2 && faceVerts[1].y > screen->clipBound_Y2 && faceVerts[2].y > screen->clipBound_Y2
                        && faceVerts[3].y > screen->clipBound_Y2)
                        continue;

                    if (faceVerts[0].x == faceVerts[1].x && faceVerts[1].x == faceVerts[2].x && faceVerts[2].x == faceVerts[3].x)
                        continue;

                    if (faceVerts[0].y == faceVerts[1].y && faceVerts[1].y == faceVerts[2].y && faceVerts[2].y == faceVerts[3].y)
                        continue;

                    // convert to fixed point and go wild
                    faceVerts[0].x = TO_FIXED(faceVerts[0].x);
                    faceVerts[0].y = TO_FIXED(faceVerts[0].y);
                    faceVerts[1].x = TO_FIXED(faceVerts[1].x);
                    faceVerts[1].y = TO_FIXED(faceVerts[1].y);
                    faceVerts[2].x = TO_FIXED(faceVerts[2].x);
                    faceVerts[2].y = TO_FIXED(faceVerts[2].y);
                    faceVerts[3].x = TO_FIXED(faceVerts[3].x);
                    faceVerts[3].y = TO_FIXED(faceVerts[3].y);

                    this->animator.SetAnimation(sVars->aniFrames, 0, false, 0);
                    this->DrawTexturedFace(faceVerts, faceUVs, 4, &this->animator, 0xFF, INK_BLEND);
                }
                break;

            case HP_Halfpipe::Face3DSprite:
                if (vertexBufferT[face->a].z > 0) {
                    // [face->a].x = xpos
                    // [face->a].y = ypos
                    // [face->a].z = stageZPos
                    // [face->a].u = aniFrames
                    // [face->a].v = drawFX
                    // [face->b].u == frameID
                    // [face->b].v == animID
                    // [face->c].u == unused
                    // [face->c].v == rotation
                    // [face->d].u == scale.x
                    // [face->d].v == scale.y

                    Vector2 drawPos;
                    drawPos.x = TO_FIXED(screen->center.x + sVars->scene3D.projectionX * vertexBufferT[face->a].x / vertexBufferT[face->a].z);
                    drawPos.y = TO_FIXED(screen->center.y - sVars->scene3D.projectionY * vertexBufferT[face->a].y / vertexBufferT[face->a].z);

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

void HP_Halfpipe::MatrixTranslateXYZ(RSDK::Matrix *matrix, int32 x, int32 y, int32 z)
{
    matrix->values[0][0] = 0x100;
    matrix->values[0][1] = 0;
    matrix->values[0][2] = 0;
    matrix->values[0][3] = 0;

    matrix->values[1][0] = 0;
    matrix->values[1][1] = 0x100;
    matrix->values[1][2] = 0;
    matrix->values[1][3] = 0;

    matrix->values[2][0] = 0;
    matrix->values[2][1] = 0;
    matrix->values[2][2] = 0x100;
    matrix->values[2][3] = 0;

    matrix->values[3][0] = x;
    matrix->values[3][1] = y;
    matrix->values[3][2] = z;
    matrix->values[3][3] = 0x100;
}
void HP_Halfpipe::MatrixRotateXYZ(RSDK::Matrix *matrix, int16 x, int16 y, int16 z)
{
    int32 sinX = Math::Sin512(x) >> 1;
    int32 cosX = Math::Cos512(x) >> 1;
    int32 sinY = Math::Sin512(y) >> 1;
    int32 cosY = Math::Cos512(y) >> 1;
    int32 sinZ = Math::Sin512(z) >> 1;
    int32 cosZ = Math::Cos512(z) >> 1;

    matrix->values[0][0] = (cosZ * cosY >> 8) + (sinZ * (sinY * sinX >> 8) >> 8);
    matrix->values[0][1] = (sinZ * cosY >> 8) - (cosZ * (sinY * sinX >> 8) >> 8);
    matrix->values[0][2] = sinY * cosX >> 8;
    matrix->values[0][3] = 0;

    matrix->values[1][0] = sinZ * -cosX >> 8;
    matrix->values[1][1] = cosZ * cosX >> 8;
    matrix->values[1][2] = sinX;
    matrix->values[1][3] = 0;

    matrix->values[2][0] = (sinZ * (cosY * sinX >> 8) >> 8) - (cosZ * sinY >> 8);
    matrix->values[2][1] = (sinZ * -sinY >> 8) - (cosZ * (cosY * sinX >> 8) >> 8);
    matrix->values[2][2] = cosY * cosX >> 8;
    matrix->values[2][3] = 0;

    matrix->values[3][0] = 0;
    matrix->values[3][1] = 0;
    matrix->values[3][2] = 0;
    matrix->values[3][3] = 0x100;
}

void HP_Halfpipe::MatrixMultiply(RSDK::Matrix *dest, RSDK::Matrix *matrixA, RSDK::Matrix *matrixB)
{
    int32 output[16];

    for (int32 i = 0; i < 0x10; ++i) {
        uint32 rowA = i / 4;
        uint32 rowB = i % 4;
        output[i]   = (matrixA->values[rowA][3] * matrixB->values[3][rowB] >> 8) + (matrixA->values[rowA][2] * matrixB->values[2][rowB] >> 8)
                    + (matrixA->values[rowA][1] * matrixB->values[1][rowB] >> 8) + (matrixA->values[rowA][0] * matrixB->values[0][rowB] >> 8);
    }

    for (int32 i = 0; i < 0x10; ++i) dest->values[i / 4][i % 4] = output[i];
}

#if RETRO_INCLUDE_EDITOR
void HP_Halfpipe::EditorDraw() {}

void HP_Halfpipe::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_Halfpipe::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(HP_Halfpipe);

    sVars->aniFrames.Init();
    sVars->shadowFrames.Init();

    sVars->vertexTablePtrs[0] = (VertexTable *)sVars->vertexTable1;
    sVars->vertexTablePtrs[1] = (VertexTable *)sVars->vertexTable1;
    sVars->vertexTablePtrs[2] = (VertexTable *)sVars->vertexTable1;
    sVars->vertexTablePtrs[3] = (VertexTable *)sVars->vertexTable1;
    sVars->vertexTablePtrs[4] = (VertexTable *)sVars->vertexTable2;
    sVars->vertexTablePtrs[5] = (VertexTable *)sVars->vertexTable3;
    sVars->vertexTablePtrs[6] = (VertexTable *)sVars->vertexTable4;
    sVars->vertexTablePtrs[7] = (VertexTable *)sVars->vertexTable5;

    int32 vertexOffsetTable[] = { 19, 19, 19, 19, 17, 17, 45, 17 };
    memcpy(sVars->vertexOffsetTable, vertexOffsetTable, sizeof(vertexOffsetTable));

    int32 matTransformTable[] = { 0, 2, 4, 6, 8, 10, 12, 14, 16, -2, -4, -6, -8, -10, -12, -14, -16 };
    memcpy(sVars->matTransformTable, matTransformTable, sizeof(matTransformTable));

    // clang-format off
    int32 vertexTable1[] = {
        38,
        -0x4000,  0x1100,  0x0000,
        -0x4400,   0xD00,  0x0000,
        -0x3C00,   0xD00,  0x0000,
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
         0x4000,  0x1100,  0x0000,
         0x3C00,   0xD00,  0x0000,
         0x4400,   0xD00,  0x0000,
        -0x4000,  0x1100,  0x0000,
        -0x4400,   0xD00,  0x0000,
        -0x3C00,   0xD00,  0x0000,
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
         0x4000,  0x1100,  0x0000,
         0x3C00,   0xD00,  0x0000,
         0x4400,   0xD00,  0x0000,
    };
    // clang-format on
    memcpy(sVars->vertexTable1, vertexTable1, sizeof(vertexTable1));

    // clang-format off
    int32 vertexTable2[] = {
        34,
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x2059, -0x36DE,  0x0000,
        -0x1449, -0x3C5D,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x1449, -0x3C5D,  0x0000,
         0x2059, -0x36DE,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x1ED7, -0x378F,  0x0000,
        -0x15CB, -0x3BAF,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x15CB, -0x3BAF,  0x0000,
         0x1ED7, -0x378F,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
    };
    // clang-format on
    memcpy(sVars->vertexTable2, vertexTable2, sizeof(vertexTable2));

    // clang-format off
    int32 vertexTable3[] = { 
        51, 
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x1ED7, -0x378F,  0x0000,
        -0x15CB, -0x3BAF,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x15CB, -0x3BAF,  0x0000,
         0x1ED7, -0x378F,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x1E46, -0x37D1,  0x0000,
        -0x165C, -0x3B6D,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x165C, -0x3B6D,  0x0000,
         0x1E46, -0x37D1,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x1D55, -0x383F,  0x0000,
        -0x174D, -0x3AFF,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x174D, -0x3AFF,  0x0000,
         0x1D55, -0x383F,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
    };
    // clang-format on
    memcpy(sVars->vertexTable3, vertexTable3, sizeof(vertexTable3));

    // clang-format off
    int32 vertexTable4[] = {
        79,
        -0x3800,   0xA00,  0x0000,
        -0x3800,   0xA00,  0x0000,
        -0x3800,   0xA00,  0x0000,
        -0x3800,   0xA00,  0x0000,
        -0x2E00,  0x2400,  0x0000,
        -0x2E00,  0x2400,  0x0000,
        -0x2E00,  0x2400,  0x0000,
        -0x2E00,  0x2400,  0x0000,
        -0x1A00,  0x3600,  0x0000,
        -0x1A00,  0x3600,  0x0000,
        -0x1A00,  0x3600,  0x0000,
        -0x1A00,  0x3600,  0x0000,
         0x0000,  0x4000,  0x0000,
         0x0000,  0x4000,  0x0000,
         0x0000,  0x4000,  0x0000,
         0x0000,  0x4000,  0x0000,
         0x1A00,  0x3600,  0x0000,
         0x1A00,  0x3600,  0x0000,
         0x1A00,  0x3600,  0x0000,
         0x1A00,  0x3600,  0x0000,
         0x2E00,  0x2400,  0x0000,
         0x2E00,  0x2400,  0x0000,
         0x2E00,  0x2400,  0x0000,
         0x2E00,  0x2400,  0x0000,
         0x3800,   0xA00,  0x0000,
         0x3800,   0xA00,  0x0000,
         0x3800,   0xA00,  0x0000,
         0x3800,   0xA00,  0x0000,
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x1D55, -0x383F,  0x0000,
        -0x174D, -0x3AFF,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x174D, -0x3AFF,  0x0000,
         0x1D55, -0x383F,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x1C64, -0x38AD,  0x0000,
        -0x183E, -0x3A91,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x183E, -0x3A91,  0x0000,
         0x1C64, -0x38AD,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x1BD3, -0x38EF,  0x0000,
        -0x18CF, -0x3A50,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x18CF, -0x3A50,  0x0000,
         0x1BD3, -0x38EF,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
    };
    // clang-format on
    memcpy(sVars->vertexTable4, vertexTable4, sizeof(vertexTable4));

    // clang-format off
    int32 vertexTable5[] = { 
        34,
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x1BD3, -0x38EF,  0x0000,
        -0x18CF, -0x3A50,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x18CF, -0x3A50,  0x0000,
         0x1BD3, -0x38EF,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
        -0x4000,   0x900,  0x0000,
        -0x4000,  -0x91C,  0x0000,
        -0x3A37, -0x1A96,  0x0000,
        -0x305E, -0x29E9,  0x0000,
        -0x2298, -0x35D7,  0x0000,
        -0x1A51, -0x39A0,  0x0000,
        -0x1A51, -0x39A0,  0x0000,
        -0x1208, -0x3D68,  0x0000,
         0x0000, -0x4000,  0x0000,
         0x1208, -0x3D68,  0x0000,
         0x1A51, -0x39A0,  0x0000,
         0x1A51, -0x39A0,  0x0000,
         0x2298, -0x35D7,  0x0000,
         0x305E, -0x29E9,  0x0000,
         0x3A37, -0x1A96,  0x0000,
         0x4000,  -0x91C,  0x0000,
         0x4000,   0x900,  0x0000,
    };
    // clang-format on
    memcpy(sVars->vertexTable5, vertexTable5, sizeof(vertexTable5));

    // clang-format off
    int32 faceTable1[] = { 
        22, 
        0,  1,  3,  2,  FaceTable::Entry::RailEnd,  
        0,  1,  20, 19, FaceTable::Entry::Arrow,  
        0,  2,  21, 19, FaceTable::Entry::Arrow,  
        1,  20, 22, 3,  FaceTable::Entry::Arrow,  
        2,  21, 22, 3,  FaceTable::Entry::Arrow,  
        3,  22, 23, 4,  FaceTable::Entry::StageColor2,  
        4,  23, 24, 5,  FaceTable::Entry::StageColor1,  
        5,  24, 25, 6,  FaceTable::Entry::StageColor1,  
        6,  25, 26, 7,  FaceTable::Entry::StageColor1,  
        7,  26, 27, 8,  FaceTable::Entry::StageColor2,  
        8,  27, 28, 9,  FaceTable::Entry::StageColor1,
        9,  28, 29, 10, FaceTable::Entry::StageColor1,  
        10, 29, 30, 11, FaceTable::Entry::StageColor2,  
        11, 30, 31, 12, FaceTable::Entry::StageColor1,  
        12, 31, 32, 13, FaceTable::Entry::StageColor1,  
        13, 32, 33, 14, FaceTable::Entry::StageColor1,
        14, 33, 34, 15, FaceTable::Entry::StageColor2,  
        16, 17, 15, 18, FaceTable::Entry::RailEnd, 
        16, 35, 36, 17, FaceTable::Entry::Arrow,  
        17, 36, 15, 34, FaceTable::Entry::Arrow,  
        16, 35, 37, 18, FaceTable::Entry::Arrow,  
        18, 37, 34, 15, FaceTable::Entry::Arrow, 
    };
    // clang-format on
    memcpy(sVars->faceTable1, faceTable1, sizeof(faceTable1));

    // clang-format off
    int32 faceTable2[] = { 
        20, 
        0,  1,  20, 19, FaceTable::Entry::Arrow,  
        0,  2,  21, 19, FaceTable::Entry::Arrow,  
        1,  20, 22, 3,  FaceTable::Entry::Arrow,  
        2,  21, 22, 3,  FaceTable::Entry::Arrow,  
        3,  22, 23, 4,  FaceTable::Entry::StageColor2, 
        4,  23, 24, 5,  FaceTable::Entry::StageColor1,  
        5,  24, 25, 6,  FaceTable::Entry::StageColor1,  
        6,  25, 26, 7,  FaceTable::Entry::StageColor1,  
        7,  26, 27, 8,  FaceTable::Entry::StageColor2,  
        8,  27, 28, 9,  FaceTable::Entry::StageColor1,  
        9,  28, 29, 10, FaceTable::Entry::StageColor1,  
        10, 29, 30, 11, FaceTable::Entry::StageColor2,  
        11, 30, 31, 12, FaceTable::Entry::StageColor1,  
        12, 31, 32, 13, FaceTable::Entry::StageColor1,  
        13, 32, 33, 14, FaceTable::Entry::StageColor1, 
        14, 33, 34, 15, FaceTable::Entry::StageColor2,  
        16, 35, 36, 17, FaceTable::Entry::Arrow,  
        17, 36, 15, 34, FaceTable::Entry::Arrow,  
        16, 35, 37, 18, FaceTable::Entry::Arrow,  
        18, 37, 34, 15, FaceTable::Entry::Arrow,
    };
    // clang-format on
    memcpy(sVars->faceTable2, faceTable2, sizeof(faceTable2));

    // clang-format off
    int32 faceTable3[] = { 
        16, 
        0,  17, 18, 1,  FaceTable::Entry::StageColor2,  
        1,  18, 19, 2,  FaceTable::Entry::StageColor1,  
        2,  19, 20, 3,  FaceTable::Entry::StageColor1,  
        3,  20, 21, 4,  FaceTable::Entry::StageColor1,  
        4,  21, 22, 5,  FaceTable::Entry::StageColor2,  
        5,  22, 23, 6,  FaceTable::Entry::Arrow,  
        6,  23, 24, 7,  FaceTable::Entry::StageColor2,  
        7,  24, 25, 8,  FaceTable::Entry::StageColor1,  
        8,  25, 26, 9,  FaceTable::Entry::StageColor1,  
        9,  26, 27, 10, FaceTable::Entry::StageColor2,  
        10, 27, 28, 11, FaceTable::Entry::Arrow,  
        11, 28, 29, 12, FaceTable::Entry::StageColor2,  
        12, 29, 30, 13, FaceTable::Entry::StageColor1,  
        13, 30, 31, 14, FaceTable::Entry::StageColor1,  
        14, 31, 32, 15, FaceTable::Entry::StageColor1,  
        15, 32, 33, 16, FaceTable::Entry::StageColor2,
    };
    // clang-format on
    memcpy(sVars->faceTable3, faceTable3, sizeof(faceTable3));

    // clang-format off
    int32 faceTable4[] = {
        32, 
        0,  17, 18, 1,  FaceTable::Entry::StageColor2,  
        1,  18, 19, 2,  FaceTable::Entry::StageColor1,  
        2,  19, 20, 3,  FaceTable::Entry::StageColor1,  
        3,  20, 21, 4,  FaceTable::Entry::StageColor1,  
        4,  21, 22, 5,  FaceTable::Entry::StageColor2,  
        5,  22, 23, 6,  FaceTable::Entry::Arrow,  
        6,  23, 24, 7,  FaceTable::Entry::StageColor2,  
        7,  24, 25, 8,  FaceTable::Entry::StageColor1,  
        8,  25, 26, 9,  FaceTable::Entry::StageColor1,  
        9,  26, 27, 10, FaceTable::Entry::StageColor2,  
        10, 27, 28, 11, FaceTable::Entry::Arrow,  
        11, 28, 29, 12, FaceTable::Entry::StageColor2,  
        12, 29, 30, 13, FaceTable::Entry::StageColor1,
        13, 30, 31, 14, FaceTable::Entry::StageColor1,  
        14, 31, 32, 15, FaceTable::Entry::StageColor1,  
        15, 32, 33, 16, FaceTable::Entry::StageColor2,  
        17, 34, 35, 18, FaceTable::Entry::StageColor2,  
        18, 35, 36, 19, FaceTable::Entry::StageColor2,  
        19, 36, 37, 20, FaceTable::Entry::StageColor2,  
        20, 37, 38, 21, FaceTable::Entry::StageColor2, 
        21, 38, 39, 22, FaceTable::Entry::StageColor2, 
        22, 39, 40, 23, FaceTable::Entry::Arrow,  
        23, 40, 41, 24, FaceTable::Entry::StageColor2,  
        24, 41, 42, 25, FaceTable::Entry::StageColor2,  
        25, 42, 43, 26, FaceTable::Entry::StageColor2, 
        26, 43, 44, 27, FaceTable::Entry::StageColor2, 
        27, 44, 45, 28, FaceTable::Entry::Arrow,  
        28, 45, 46, 29, FaceTable::Entry::StageColor2, 
        29, 46, 47, 30, FaceTable::Entry::StageColor2,  
        30, 47, 48, 31, FaceTable::Entry::StageColor2,  
        31, 48, 49, 32, FaceTable::Entry::StageColor2,  
        32, 49, 50, 33, FaceTable::Entry::StageColor2,
    };
    // clang-format on
    memcpy(sVars->faceTable4, faceTable4, sizeof(faceTable4));

    // clang-format off
    int32 faceTable5[] = {
        39, 
        0,  1,  2,  3,  FaceTable::Entry::LoopRing,  
        4,  5,  6,  7,  FaceTable::Entry::LoopRing,  
        8,  9,  10, 11, FaceTable::Entry::LoopRing,  
        12, 13, 14, 15, FaceTable::Entry::LoopRing,  
        16, 17, 18, 19, FaceTable::Entry::LoopRing,  
        20, 21, 22, 23, FaceTable::Entry::LoopRing,  
        24, 25, 26, 27, FaceTable::Entry::LoopRing,  
        28, 45, 46, 29, FaceTable::Entry::StageColor2,  
        29, 46, 47, 30, FaceTable::Entry::StageColor2,  
        30, 47, 48, 31, FaceTable::Entry::StageColor2,  
        31, 48, 49, 32, FaceTable::Entry::StageColor2,  
        32, 49, 50, 33, FaceTable::Entry::StageColor2,  
        33, 50, 51, 34, FaceTable::Entry::Arrow,
        34, 51, 52, 35, FaceTable::Entry::StageColor2,  
        35, 52, 53, 36, FaceTable::Entry::StageColor2,  
        36, 53, 54, 37, FaceTable::Entry::StageColor2,  
        37, 54, 55, 38, FaceTable::Entry::StageColor2,  
        38, 55, 56, 39, FaceTable::Entry::Arrow,  
        39, 56, 57, 40, FaceTable::Entry::StageColor2,  
        40, 57, 58, 41, FaceTable::Entry::StageColor2,  
        41, 58, 59, 42, FaceTable::Entry::StageColor2,  
        42, 59, 60, 43, FaceTable::Entry::StageColor2,  
        43, 60, 61, 44, FaceTable::Entry::StageColor2,  
        45, 62, 63, 46, FaceTable::Entry::StageColor2,  
        46, 63, 64, 47, FaceTable::Entry::StageColor1,  
        47, 64, 65, 48, FaceTable::Entry::StageColor1,  
        48, 65, 66, 49, FaceTable::Entry::StageColor1,
        49, 66, 67, 50, FaceTable::Entry::StageColor2, 
        50, 67, 68, 51, FaceTable::Entry::Arrow, 
        51, 68, 69, 52, FaceTable::Entry::StageColor2, 
        52, 69, 70, 53, FaceTable::Entry::StageColor1, 
        53, 70, 71, 54, FaceTable::Entry::StageColor1, 
        54, 71, 72, 55, FaceTable::Entry::StageColor2,  
        55, 72, 73, 56, FaceTable::Entry::Arrow,  
        56, 73, 74, 57, FaceTable::Entry::StageColor2,  
        57, 74, 75, 58, FaceTable::Entry::StageColor1,  
        58, 75, 76, 59, FaceTable::Entry::StageColor1,  
        59, 76, 77, 60, FaceTable::Entry::StageColor1,  
        60, 77, 78, 61, FaceTable::Entry::StageColor2,
    };
    // clang-format on
    memcpy(sVars->faceTable5, faceTable5, sizeof(faceTable5));

    // clang-format off
    int32 faceTable6[] = { 
        16, 
        0,  17, 18, 1,  FaceTable::Entry::StageColor2,
        1,  18, 19, 2,  FaceTable::Entry::StageColor1,
        2,  19, 20, 3,  FaceTable::Entry::StageColor1,
        3,  20, 21, 4,  FaceTable::Entry::StageColor1, 
        4,  21, 22, 5,  FaceTable::Entry::StageColor2, 
        5,  22, 23, 6,  FaceTable::Entry::Arrow,  
        6,  23, 24, 7,  FaceTable::Entry::StageColor2,  
        7,  24, 25, 8,  FaceTable::Entry::StageColor1,  
        8,  25, 26, 9,  FaceTable::Entry::StageColor1,  
        9,  26, 27, 10, FaceTable::Entry::StageColor2, 
        10, 27, 28, 11, FaceTable::Entry::Arrow, 
        11, 28, 29, 12, FaceTable::Entry::StageColor2,  
        12, 29, 30, 13, FaceTable::Entry::StageColor1,  
        13, 30, 31, 14, FaceTable::Entry::StageColor1,  
        14, 31, 32, 15, FaceTable::Entry::StageColor1,  
        15, 32, 33, 16, FaceTable::Entry::StageColor2,
    };
    // clang-format on
    memcpy(sVars->faceTable6, faceTable6, sizeof(faceTable6));
}
#endif

void HP_Halfpipe::Serialize() {}

} // namespace GameLogic