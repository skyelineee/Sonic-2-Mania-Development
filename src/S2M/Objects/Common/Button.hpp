#pragma once
#include "Game.hpp"

namespace GameLogic
{

struct Button : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum Types {
        Floor,
        Roof,
        RWall,
        LWall,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::Hitbox hitboxV;
        RSDK::Hitbox hitboxH;
        int32 activatePos;
        int32 buttonOffset;
        int32 hitboxOffset;
        int32 unused1;
        RSDK::SoundFX sfxButton;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    int32 type;
    bool32 walkOnto;
    uint8 tag;
    bool32 down;
    bool32 toggled;
    bool32 currentlyActive;
    bool32 activated;
    bool32 wasActivated;
    int32 pressPos;
    RSDK::Hitbox hitboxButton;
    RSDK::Animator buttonAnimator;
    RSDK::Animator baseAnimator;
    bool32 silent;

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

    void HandleFloor();
    void HandleRoof();
    void HandleRWall();
    void HandleLWall();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Button);
};
} // namespace GameLogic