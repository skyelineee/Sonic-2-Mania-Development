#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct TitleSonic : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum TitleSonicTypes {
        Sonic,
        Tails,
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

    int32 type;
    RSDK::Animator animatorSonic;
    RSDK::Animator animatorTails;

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

    RSDK_DECLARE(TitleSonic);
};
} // namespace GameLogic