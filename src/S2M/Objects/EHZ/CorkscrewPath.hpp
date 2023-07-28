#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct CorkscrewPath : RSDK::GameObject::Entity {

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
        int32 frameTable[24];
        RSDK::SpriteAnimation aniFrames; // probably editor-only
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 period;
    int32 xSize; // length to the (left/right) edges from the center
    int32 amplitude;
    int32 activePlayers;
    RSDK::Animator animator; // also probably editor-only


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

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(CorkscrewPath);
};
} // namespace GameLogic