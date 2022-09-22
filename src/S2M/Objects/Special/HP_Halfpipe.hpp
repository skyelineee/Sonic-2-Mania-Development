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
        FACE_FLAG_TEXTURED_3D      = 0,
        FACE_FLAG_TEXTURED_2D      = 1,
        FACE_FLAG_COLORED_3D       = 2,
        FACE_FLAG_COLORED_2D       = 3,
        FACE_FLAG_FADED            = 4,
        FACE_FLAG_TEXTURED_C       = 5,
        FACE_FLAG_TEXTURED_C_BLEND = 6,
        FACE_FLAG_3DSPRITE         = 7
    };

    // ==============================
    // STRUCTS
    // ==============================
    struct Vector3 {
        int32 x;
        int32 y;
        int32 z;
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

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        int32 faceCount;
        int32 vertexCount;
        int32 prevVertexCount;
        Vector3 rotateOffset;
        Vector3 tempTranslate;
        Vector3 worldRotate;
        int32 tranformPos;
        int32 checkpointTexCount;
        Vector3 translateTable[40];
        Vector3 rotateTable[40];
        int32 checkpointFaceTable[4];
        int32 vertexOffsetTable[8];
        int32 matTransformTable[17];
        int32 vertexTablePtrs[8];
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

    static void SetupFaces(int32 vertexTableID, int32 *faceTable);
    static void SetupVertices(int32 *faceTable);

    static void TransformVertexBuffer();
    static void SortDrawList(int32 first, int32 last);
    void Draw3DScene();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(HP_Halfpipe);
};
} // namespace GameLogic