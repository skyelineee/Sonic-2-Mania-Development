#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct CNZSpotlight : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum SSZSpotlightColors {
        CNZSPOTLIGHT_CLR_CYAN,
        CNZSPOTLIGHT_CLR_PINK,
    };

    enum SSZSpotlightSizes {
        CNZSPOTLIGHT_SIZE_SMALL,
        CNZSPOTLIGHT_SIZE_MED,
        CNZSPOTLIGHT_SIZE_LARGE,
    };

    enum SSZSpotlightDrawFlags {
        CNZSPOTLIGHT_DRAW_LOW,
        CNZSPOTLIGHT_DRAW_HIGH,
        CNZSPOTLIGHT_DRAW_LOWER,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        color cyanSpotlightColors[8];
        color pinkSpotlightColors[8];
        RSDK::SpriteAnimation aniFrames;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    uint8 spotlightColor;
    uint8 size;
    uint8 drawFlag;
    uint8 speed;
    uint8 offset;
    uint8 flashSpeed;
    RSDK::Vector2 originPos;
    RSDK::Vector2 spotlightVertices[8];
    RSDK::Vector2 drawVertices[8];
    color *colorTable;
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

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(CNZSpotlight);
};
} // namespace GameLogic
