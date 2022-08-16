#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct FXAniTiles : RSDK::GameObject::Entity {

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
    uint32 aniID;
    uint32 frameID;
    uint32 tileID;
    bool32 animate;
    bool32 disable;
    bool32 controlFlag;

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

    void State_Init();
    void State_Idle();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(FXAniTiles);
};
} // namespace GameLogic