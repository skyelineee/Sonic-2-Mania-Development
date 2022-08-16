#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct FXTileModifier : RSDK::GameObject::Entity {

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
        FXTileModifier *targetModifier;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::Animator animator;
    RSDK::String src1;
    RSDK::String src2;
    RSDK::String dst1;
    RSDK::String dst2;
    RSDK::Vector2 copyOrigin;
    RSDK::Vector2 copyRange;
    bool32 activated;
    int32 activationTime;
    int32 activationCount;
    RSDK::Vector2 activationAddOffset;
    int32 timer;
    int32 parameter;

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

    static FXTileModifier *GetNearest(RSDK::GameObject::Entity *self);
    static void Find(uint8 type);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(FXTileModifier);
};
} // namespace GameLogic