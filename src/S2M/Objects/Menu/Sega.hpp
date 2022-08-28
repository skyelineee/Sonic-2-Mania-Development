#pragma once

#include "Game.hpp"
#include "S2M.hpp"

namespace GameLogic
{

// Object Class
struct Sega : RSDK::GameObject::Entity {

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
    };

    int32 listID;
    int32 frameID;
    int32 fadeValue;
    int32 timer;
    int32 direction;
    RSDK::Vector2 trailPos;
    RSDK::StateMachine<Sega> state;
    RSDK::Animator animator;

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Update();
    void LateUpdate();
    void Draw();
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

    //States
    void State_Setup();
    void State_FadeIn();
    void State_SonicRun_Left();
    void State_SonicRun_Right();
    void State_FadeOutTrail_Left();
    void State_FadeOutTrail_Right();
    void State_ShowSegaLogo();
    void State_FadeToTitle();

    RSDK_DECLARE(Sega);
};
} // namespace GameLogic