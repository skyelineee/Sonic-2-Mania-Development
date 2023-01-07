#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIWidgets : RSDK::GameObject::Entity {

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
        int32 buttonColors[16];
        int32 timer;
        RSDK::Animator unusedAnimator1;
        RSDK::Animator unusedAnimator2;
        RSDK::Animator frameAnimator;
        RSDK::Animator arrowsAnimator;
        RSDK::SpriteAnimation uiFrames;
        RSDK::SpriteAnimation saveSelFrames;
        RSDK::SpriteAnimation textFrames;
        RSDK::SpriteAnimation fontFrames;
        RSDK::SoundFX sfxBleep;
        RSDK::SoundFX sfxAccept;
        RSDK::SoundFX sfxWarp;
        RSDK::SoundFX sfxEvent;
        RSDK::SoundFX sfxWoosh;
        RSDK::SoundFX sfxFail;
        color buttonColor;
    };

    RSDK::StateMachine<UIWidgets> state;

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

    static void ApplyLanguage();

    static void DrawRectOutline_Black(int32 x, int32 y, int32 width, int32 height);
    static void DrawRectOutline_Blended(int32 x, int32 y, int32 width, int32 height);
    static void DrawRectOutline_Flash(int32 x, int32 y, int32 width, int32 height);
    static void DrawRightTriangle(int32 x, int32 y, int32 size, int32 red, int32 green, int32 blue);
    static void DrawEquilateralTriangle(int32 x, int32 y, int32 size, uint8 sizeMode, int32 red, int32 green, int32 blue, RSDK::InkEffects ink);
    static void DrawParallelogram(int32 x, int32 y, int32 width, int32 height, int32 edgeSize, int32 red, int32 green, int32 blue);
    static void DrawUpDownArrows(int32 x, int32 y, int32 arrowDist);
    static void DrawLeftRightArrows(int32 x, int32 y, int32 arrowDist);
    static RSDK::Vector2 DrawTriJoinRect(int32 x, int32 y, color leftColor, color rightColor);

    static void DrawTime(int32 x, int32 y, int32 minutes, int32 seconds, int32 milliseconds);

    // ==============================
    // STATES
    // ==============================

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIWidgets);
};
} // namespace GameLogic
