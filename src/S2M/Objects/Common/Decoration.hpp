#pragma once
#include "Game.hpp"

namespace GameLogic
{

struct Decoration : RSDK::GameObject::Entity {

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
    RSDK::Animator animator;
    uint8 type;
    uint8 subtype;
    int32 rotSpeed;
    RSDK::Vector2 repeatTimes;
    RSDK::Vector2 repeatSpacing;
    bool32 reverseX;
    bool32 reverseY;
    int32 startRot;
    int32 rangeType;

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

    void DrawSprites();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Decoration);
};
} // namespace GameLogic