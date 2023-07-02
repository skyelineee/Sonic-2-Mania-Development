#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"

namespace GameLogic
{

struct UIDiorama : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum UIDioramaIDs {
        UIDIORAMA_ADVENTURE,
        UIDIORAMA_TIMEATTACK,
        UIDIORAMA_EXTRAS,
        UIDIORAMA_OPTIONS,
        UIDIORAMA_EXIT,
        UIDIORAMA_BOSSRUSH,
        UIDIORAMA_MUSICPLAYER,
        UIDIORAMA_LEVELSELECT,
        UIDIORAMA_EXTRALEVELS,
        UIDIORAMA_CREDITS,
        UIDIORAMA_VIDEO,
        UIDIORAMA_SOUND,
        UIDIORAMA_CONTROLS,
        UIDIORAMA_DATA,

    } ;

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
    };

    int32 dioramaID;
    int32 lastDioramaID;
    int32 timer;
    uint8 parentActivity;
    bool32 needsSetup;
    UIControl *parent;
    RSDK::Vector2 dioramaPos;
    RSDK::Vector2 dioramaSize;
    RSDK::Animator staticAnimator;
    RSDK::Animator dioramaAnimator;

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

    void ChangeDiorama(uint8 dioramaID);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIDiorama);
};
} // namespace GameLogic