#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct ParallaxSprite : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Attributes {
        AttrStandard,
        AttrEmitter,
        AttrParticle,
        AttrStatic,
    };

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
    RSDK::StateMachine<ParallaxSprite> state;
    RSDK::Animator animator;
    uint8 aniID;
    uint8 attribute;
    RSDK::Vector2 parallaxFactor;
    RSDK::Vector2 scrollSpeed;
    RSDK::Vector2 loopPoint;
    bool32 hiddenAtStart;
    uint8 overrideDrawGroup;
    RSDK::Vector2 acceleration;
    RSDK::Vector2 scrollPos;
    int32 screenOffsetX;
    int32 screenOffsetY;

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

    void State_Emitter();
    void State_Particle();
    void State_Static();
    void State_Normal();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ParallaxSprite);
};
} // namespace GameLogic