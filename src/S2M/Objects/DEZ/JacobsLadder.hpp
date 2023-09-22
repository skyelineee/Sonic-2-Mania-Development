#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct JacobsLadder : RSDK::GameObject::Entity {

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
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxLadder;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 height;
    int32 interval;
    int32 intervalOffset;
    int32 duration;
    bool32 flip;
    int32 unused1;
    RSDK::Vector2 startPos;
    int32 unused2;
    int32 electricPos;
    bool32 showElectricity;
    bool32 electricFadeOut;
    uint8 activePlayers;
    uint8 playerTimers[PLAYER_COUNT];
    RSDK::Vector2 playerPositions[PLAYER_COUNT];
    RSDK::Hitbox hitbox;
    RSDK::Animator sideAnimator;
    RSDK::Animator electricAnimator;

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

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(JacobsLadder);
};
} // namespace GameLogic
