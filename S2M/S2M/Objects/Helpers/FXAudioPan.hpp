#pragma once
#include "S2M.hpp"

#include "Soundboard.hpp"

namespace GameLogic
{

struct FXAudioPan : RSDK::GameObject::Entity {

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
        int32 activeCount1;
        int32 activeCount2;
        int32 field_C;
        int32 field_10;
        int32 field_14;
        int32 field_18;
        RSDK::SpriteAnimation aniFrames;
        uint16 field_1E;
        int32 field_20;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::Vector2 size;
    RSDK::Vector2 sfxPos;
    bool32 sfxActive;
    RSDK::Animator animator;
    RSDK::SoundFX sfxID;
    uint32 loopPos;
    RSDK::String soundName;

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

    static Soundboard::SoundInfo CheckCB();
    static void UpdateCB(int32 sfxID);

    uint8 PlayDistancedSfx(RSDK::SoundFX sfx, uint32 loopPoint, uint32 priority, RSDK::Vector2 position);
    void HandleAudioDistance(uint32 channelID, RSDK::Vector2 position);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(FXAudioPan);
};
} // namespace GameLogic