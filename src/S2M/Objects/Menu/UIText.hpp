#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIText : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum UITextAlignments {
        UITEXT_ALIGN_LEFT,
        UITEXT_ALIGN_CENTER,
        UITEXT_ALIGN_RIGHT,
    };

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

    RSDK::StateMachine<UIText> state;
    RSDK::StateMachine<UIText> stateDraw;
    RSDK::String text;
    RSDK::String tag;
    bool32 selectable;
    uint16 listID;
    int32 align;
    int32 data0;
    int32 data1;
    int32 data2;
    int32 data3;
    bool32 highlighted;
    RSDK::Animator animator;

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

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIText);
};
} // namespace GameLogic
