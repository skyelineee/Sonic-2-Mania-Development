#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct SpecialRing : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitbox;
        RSDK::SoundFX sfxSpecialRing;
        RSDK::SoundFX sfxSpecialWarp;
        RSDK::Graphics::Mesh modelIndex;
        RSDK::Graphics::Scene3D sceneIndex;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<SpecialRing> state;
    int32 id;
    int32 planeFilter;
    int32 warpTimer;
    int32 sparkleRadius;
    RSDK::Animator warpAnimator;
    int32 angleZ;
    int32 angleY;
    bool32 enabled;
    RSDK::Matrix matTempRot;
    RSDK::Matrix matTransform;
    RSDK::Matrix matWorld;
    RSDK::Matrix matNormal;

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

    // States
    void State_Idle();
    void State_Flash();
    void State_Warp();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(SpecialRing);
};
} // namespace GameLogic