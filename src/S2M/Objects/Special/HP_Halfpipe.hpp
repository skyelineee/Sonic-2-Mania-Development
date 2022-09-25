#pragma once
#include "S2M.hpp"

namespace GameLogic
{

#define HP_VERTEXBUFFER_SIZE (0x1000)
#define HP_FACEBUFFER_SIZE   (0x400)

struct HP_Halfpipe : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum FaceFlags {
        FaceTextured3D,
        FaceTextured2D,
        FaceColored3D,
        FaceColored2D,
        FaceFaded,
        FaceTexturedC,
        FaceTexturedC_Blend,
        Face3DSprite,
    };

    // ==============================
    // STRUCTS
    // ==============================
    struct GFXSurface {
        uint32 hash[4];
        uint8 *pixels;
        int32 height;
        int32 width;
        int32 lineSize;
        uint8 scope;
    };

    struct Vertex {
        int32 x;
        int32 y;
        int32 z;
        int32 u;
        int32 v;
    };

    struct Face {
        int32 a;
        int32 b;
        int32 c;
        int32 d;
        color color;
        uint8 flag;
    };

    struct DrawListEntry {
        int32 index;
        int32 depth;
    };

    struct Scene3D {
        int32 vertexCount;
        int32 faceCount;

        RSDK::Matrix matFinal;
        RSDK::Matrix matWorld;
        RSDK::Matrix matView;
        RSDK::Matrix matTemp;

        Face faceBuffer[HP_FACEBUFFER_SIZE];
        Vertex vertexBuffer[HP_VERTEXBUFFER_SIZE];
        Vertex vertexBufferT[HP_VERTEXBUFFER_SIZE];

        DrawListEntry drawList[HP_FACEBUFFER_SIZE];

        int32 projectionX;
        int32 projectionY;
        int32 fogColor;
        int32 fogStrength;
    };

    struct ScanEdge {
        int32 start;
        int32 end;

        int32 startU;
        int32 startV;
        int32 endU;
        int32 endV;
    };

    struct VertexTable {
        int32 count;
        Vector3 vertices[1];
    };

    struct FaceTable {
        struct Entry {
            enum Types {
                StageColor1,
                StageColor2,
                Arrow,
                RailEnd,
                Unused,
                LoopRing,
            };

            int32 a;
            int32 b;
            int32 c;
            int32 d;
            int32 type;
        };

        int32 count;
        Entry faces[1];
    };

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SpriteAnimation shadowFrames;
        int32 stageSize;
        int32 faceCount;
        int32 vertexCount;
        int32 prevVertexCount;
        uint8 vertexTableID;
        VertexTable *vertexTablePtrs[8];
        int32 sortSlot;
        Vector3 rotateOffset;
        Vector3 tempTranslate;
        Vector3 worldRotate;
        int32 transformPos;
        int32 checkpointTexCount;
        Vector3 translateTable[40];
        Vector3 rotateTable[40];
        int32 checkpointFaceTable[4];
        int32 vertexOffsetTable[8];
        int32 matTransformTable[17];
        int32 vertexTable1[115];
        int32 vertexTable2[103];
        int32 vertexTable3[154];
        int32 vertexTable4[238];
        int32 vertexTable5[103];
        int32 faceTable1[111];
        int32 faceTable2[101];
        int32 faceTable3[81];
        int32 faceTable4[161];
        int32 faceTable5[196];
        int32 faceTable6[81];
        Scene3D scene3D;
        ScanEdge scanEdgeBuffer[SCREEN_YSIZE * 2];
        uint16 *rgb32To16_R;
        uint16 *rgb32To16_G;
        uint16 *rgb32To16_B;
        uint16 *blendLookupTable;
        uint16 *subtractLookupTable;
        bool32 initialized;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::Animator animator;
    int32 moveStep;
    int32 moveSpeed;
    int32 transformIDHalfpipe;
    int32 nextTransformIDHalfpipe;
    int32 transformIDWorld;
    int32 nextTransformIDWorld;
    Vector3 halfpipeTranslation;
    Vector3 halfpipeRotation;
    Vector3 newHalfpipeRotation;
    Vector3 worldTranslation;
    Vector3 worldRotation;
    int32 playerZPos;
    int32 stageZPos;

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
    static void StageLoad();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    static void SetupFaces(int32 *faceTable);
    static void SetupVertices();

    static void DrawShadow(int32 x, int32 y, int32 z, int32 scaleX, int32 scaleY, RSDK::Animator *animator, bool32 transformVerts = false);
    static void DrawTexture(int32 x, int32 y, int32 z, int32 scaleX, int32 scaleY, RSDK::Animator *animator, RSDK::SpriteAnimation aniFrames,
                            bool32 transformVerts = false);
    static void DrawSprite(int32 x, int32 y, int32 z, uint8 drawFX, int32 scaleX, int32 scaleY, int16 rotation, RSDK::Animator *animator,
                           RSDK::SpriteAnimation aniFrames, bool32 transformVerts = false);

    void ProcessScanEdge(int32 x1, int32 y1, int32 x2, int32 y2);
    void ProcessScanEdgeUV(int32 x1, int32 y1, int32 u1, int32 v1, int32 x2, int32 y2, int32 u2, int32 v2);

    void DrawFadedFace(RSDK::Vector2 *vertices, int32 vertCount, int32 r, int32 g, int32 b, int32 fogAlpha, int32 alpha, int32 inkEffect);
    void DrawTexturedFace(RSDK::Vector2 *vertices, RSDK::Vector2 *vertexUVs, int32 vertCount, RSDK::Animator *animator, int32 alpha, int32 inkEffect);

    static void TransformVertices(RSDK::Matrix *matrix, Vertex* vertices, int32 startIndex, int32 endIndex);
    static void TransformVertexBuffer();
    static void SortDrawList(int32 first, int32 last);
    void Draw3DScene();

    static void MatrixTranslateXYZ(RSDK::Matrix *matrix, int32 x, int32 y, int32 z);
    static void MatrixRotateXYZ(RSDK::Matrix *matrix, int16 x, int16 y, int16 z);
    static void MatrixMultiply(RSDK::Matrix *dest, RSDK::Matrix *matrixA, RSDK::Matrix *matrixB);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(HP_Halfpipe);
};
} // namespace GameLogic