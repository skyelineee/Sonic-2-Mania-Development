#pragma once
#include "S2M.hpp"
#include "Helpers/FXFade.hpp"

namespace GameLogic
{

struct UITransition : RSDK::GameObject::Entity {

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
        Entity *activeTransition;
        char *newTag;
    };

    RSDK::StateMachine<UITransition> state;
    RSDK::Action<void> callback;
    Entity *prevEntity;
    int32 timer;
    int32 delay;
    bool32 isTransitioning;
    int32 fadeColor;
    bool32 back;

    // ==============================
    // INSTANCE VARS
    // ==============================

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

    static void StartTransition(RSDK::Action<void> callback, int32 delay, bool32 back = false);
    static inline void StartTransition(void (*callback)(), int32 delay, bool32 back = false)
    {
        RSDK::Action<void> cb;
        cb.Set(callback);
        StartTransition(cb, delay, back);
    }
    static void MatchNewTag();
    static void SetNewTag(const char *text);

    void DrawFade();

    void State_Init();
    void State_TransitionIn();
    void State_TransitionOut();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UITransition);
};
} // namespace GameLogic