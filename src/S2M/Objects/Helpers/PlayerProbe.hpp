#pragma once
#include "S2M.hpp"

#include "Global/Player.hpp"

namespace GameLogic
{

struct PlayerProbe : RSDK::GameObject::Entity {

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
    int32 size;
    uint8 activePlayers;
    int32 negAngle;
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
    void Print(Player *player);
    void DrawSprites();
    void DrawArrow(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(PlayerProbe);
};
} // namespace GameLogic