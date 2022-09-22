#pragma once
#include "S2M.hpp"

#include "Platform.hpp"
#include "PlatformNode.hpp"
#include "Button.hpp"

namespace GameLogic
{

struct PlatformControl : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Types {
        Circuit,
        Reverse,
        Teleport,
        Stop,
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
    int32 nodeCount;
    int32 childCount;
    int32 speed;
    bool32 setActive;
    uint8 type;
    bool32 isActive;
    int32 buttonTag;
    Button *taggedButton;
    bool32 hasStopped;
    bool32 resetOffScreen;
    bool32 playingPulleySfx;

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

    void ManagePlatformVelocity(Platform *platform, PlatformNode *node);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(PlatformControl);
};
} // namespace GameLogic