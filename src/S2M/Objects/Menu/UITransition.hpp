#pragma once
#include "S2M.hpp"

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
    RSDK::Vector2 drawPos[3];

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

    static void StartTransition(void (*callback)(), int32 delay);
    static void StartTransition(RSDK::Action<void> callback, int32 delay);
    static void MatchNewTag();
    static void SetNewTag(const char *text);

    void DrawShapes();

    void State_Init();
    void State_TransitionIn();
    void State_TransitionOut();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UITransition);
};
} // namespace GameLogic