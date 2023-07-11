#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct TitleSetup : RSDK::GameObject::Entity {

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
        bool32 useAltIntroMusic;
        RSDK::SoundFX sfxMenuAccept;
        RSDK::SoundFX sfxRing;
        RSDK::TileLayer *background;
        RSDK::TileLayer *backgroundIsland;
        RSDK::TileLayer *backgroundRipple;
        int32 deformation[64];
        int32 deformationTimer;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<TitleSetup> state;
    RSDK::StateMachine<TitleSetup> stateDraw;
    int32 timer;
    RSDK::Vector2 drawPos;
    int32 touched;

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif
    static void StageLoad();
    static void Serialize();
#if RETRO_INCLUDE_EDITOR
    static void EditorLoad();
    void EditorDraw();
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    static bool32 VideoSkipCB();

    void State_Wait();
    void State_TwinklesFadingIn();
    void State_WhiteFlash();
    void State_ClearFlash();
    void State_SetupStartText();
    void State_WaitForEnter();
    void State_FadeToMenu();
    void State_FadeToVideo();

    void Draw_FadeBlack();
    void Draw_Flash();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(TitleSetup);
};
} // namespace GameLogic