#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Springboard : RSDK::GameObject::Entity {

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
        int32 forces[3];
        int32 springPower[40];                    
        int32 heightsReady[28];
        int32 heightsFlat[28];
              //
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxSpring;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    int32 force;
    int32 bounceDelay; // bounceDelay goes unused here, though it was used in S2 '13
    uint8 activePlayers;
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

    void DebugSpawn();
    void DebugDraw();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Springboard);
};
} // namespace GameLogic
