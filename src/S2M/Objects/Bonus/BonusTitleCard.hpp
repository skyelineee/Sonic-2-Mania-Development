#pragma once
#include "S2M.hpp"

#include "Helpers/FXFade.hpp"

namespace GameLogic
{

struct BonusTitleCard : RSDK::GameObject::Entity {

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
    RSDK::Vector2 bonusTextPos;
    RSDK::String zoneName;
    RSDK::Animator zoneNameAnimator;
    int32 speed;
    int32 fadeOutTimer;
    bool32 hasPressed;

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
    void SetupTitleWords();

    // States
    void Draw_SlidingIn();

    // Draw States

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(BonusTitleCard);
};
} // namespace GameLogic