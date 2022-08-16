#pragma once
#include "S2M.hpp"

#include "Player.hpp"

namespace GameLogic
{

struct Spikes : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum MoveTypes {
        MoveStatic,
        MoveHidden,
        MoveAppear,
        MoveShown,
        MoveDisappear,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::StateMachine<Spikes> stateDraw;
        RSDK::Animator verticalAnimator;
        RSDK::Animator horizontalAnimator;
        RSDK::SpriteAnimation aniFrames;
        int32 unused1; // both set on StageLoad but then never used
        int32 unused2; // both set on StageLoad but then never used
        RSDK::SoundFX sfxMove;
        RSDK::SoundFX sfxSpike;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    int32 type;
    bool32 moving;
    uint8 count;
    uint8 stagger;
    int16 timer;
    int32 planeFilter;
    int32 stateMove;
    int32 moveOffset;
    RSDK::Vector2 collisionOffset;
    int16 glintTimer;
    uint8 shatterTimer;
    RSDK::Hitbox hitbox;
    RSDK::Animator animator;

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

    void Draw_Normal();

    void CheckHit(Player *player, int32 playerVelX, int32 playerVelY);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Spikes);
};
} // namespace GameLogic