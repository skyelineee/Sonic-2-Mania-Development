#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SlotPrize : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum SlotPrizeAnimations {
        Rings,
        Spike,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::Hitbox hitbox;
        int32 spikeSFXTimer;
        RSDK::SoundFX sfxSpike;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    bool32 winner;
    RSDK::Animator animator;
    RSDK::Vector2 originPos;
    int32 timer;
    int32 listID;

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
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SlotPrize);
};
} // namespace GameLogic
