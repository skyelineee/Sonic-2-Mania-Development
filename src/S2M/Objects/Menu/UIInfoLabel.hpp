#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIInfoLabel : RSDK::GameObject::Entity {

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

    RSDK::Animator unusedAnimator;
    int32 unused1;
    int32 unused2;
    RSDK::Vector2 size;
    RSDK::String text;
    int32 width;
    RSDK::Animator textAnimator;

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
    static void StageLoad();
    static void Serialize();

#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    static void SetText(UIInfoLabel *label, char *text);
    static void SetString(UIInfoLabel *self, RSDK::String *text);
    void DrawSprites();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIInfoLabel);
};
} // namespace GameLogic