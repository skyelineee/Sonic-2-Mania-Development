#pragma once

#include "S2M.hpp"
#include "S2M.hpp"

namespace GameLogic
{

// Object Class
struct LogoSetup : RSDK::GameObject::Entity {

    struct Static : RSDK::GameObject::Static {
        int32 timer;
        RSDK::SoundFX sfxSega;
        RSDK::SpriteAnimation aniFrames;
    };

    RSDK::StateMachine<LogoSetup> state;
    RSDK::StateMachine<LogoSetup> stateDraw;
    int32 timer;

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
    void EditorDraw();
    static void EditorLoad();
#endif

// ==============================
// FUNCTIONS
// ==============================

    static bool32 ImageCallback();
    void Draw_Fade();

    //States
    void State_CESAScreen();
    void State_ShowLogos();
    void State_FadeToNextLogos();
    void State_NextLogos();

    RSDK_DECLARE(LogoSetup);
};
} // namespace GameLogic