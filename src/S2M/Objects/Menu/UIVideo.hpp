#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UIVideo : RSDK::GameObject::Entity {

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
        bool32 playing;
        RSDK::SpriteAnimation aniFrames; // prolly used in-editor
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<UIVideo> state;
    RSDK::String videoFile1;
    RSDK::String videoFile2;
    RSDK::String audioFile;
    bool32 gotoStage;
    RSDK::String stageListCategory;
    RSDK::String stageListName;
    int32 timer;
    RSDK::Animator unusedAnimator;

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

    static bool32 SkipCB();

    void State_PlayVideo1();
    void State_PlayVideo2();
    void State_FinishPlayback();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIVideo);
};
} // namespace GameLogic