#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct InvisibleBlock : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum HurtTypes {
        HurtNone,
        HurtAll,
        HurtAqua,
        HurtFlame,
        HurtThunder,
        HurtKill,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::Animator animator;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    uint8 width;
    uint8 height;
    bool32 planeFilter;
    bool32 noCrush;
    bool32 activeNormal;
    bool32 timeAttackOnly;
    bool32 noChibi;
    RSDK::Hitbox hitbox;
    uint8 hurtType;
    bool32 ignoreTop;
    bool32 ignoreBottom;
    bool32 ignoreLeft;
    bool32 ignoreRight;
    bool32 noPush;

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

    void DrawSprites();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(InvisibleBlock);
};
} // namespace GameLogic