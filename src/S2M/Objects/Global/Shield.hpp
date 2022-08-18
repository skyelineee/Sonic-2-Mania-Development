#pragma once
#include "S2M.hpp"

#include "Player.hpp"

namespace GameLogic
{

struct Shield : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Types {
        None,
        Blue,
        Bubble,
        Fire,
        Lightning,
        Unknown,
    };

    enum AniIDs {
        AniBlue,
        AniFire,
        AniFireAttack,
        AniLightning,
        AniLightningSpark,
        AniBubbleAdd,
        AniBubble,
        AniBubbleAttackDAdd,
        AniBubbleAttackD,
        AniBubbleAttackUpAdd,
        AniBubbleAttackUp,
        AniInsta,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxBlueShield;
        RSDK::SoundFX sfxBubbleShield;
        RSDK::SoundFX sfxFireShield;
        RSDK::SoundFX sfxLightningShield;
        RSDK::SoundFX sfxInstaShield;
        RSDK::SoundFX sfxBubbleBounce;
        RSDK::SoundFX sfxFireDash;
        RSDK::SoundFX sfxLightningJump;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    Player *player;
    RSDK::StateMachine<Shield> state;
    int32 type;
    int32 timer;
    int32 frameFlags;
    bool32 forceVisible;
    RSDK::Animator shieldAnimator;
    RSDK::Animator fxAnimator;

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
    static void SpawnLightningSparks(RSDK::GameObject::Entity *parent, RSDK::SpriteAnimation &aniFrames, uint32 listID);

    void State_Default();
    void State_Insta();
    void State_BubbleDrop();
    void State_BubbleBounce();
    void State_FireDash();
    void State_LightningSparks();
    void State_LightningFlash();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Shield);
};
} // namespace GameLogic