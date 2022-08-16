#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Explosion : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum Types {
        Type0,
        Type1,
        Type2,
        Type3,
        Type4,
        Type5,
        Type6,
        Type7,
        Type8,
        Type9,
        Type10,
    };

    enum SoundIDs {
        SoundNone,
        SoundDestroy,
        SoundExplosion,
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
        RSDK::SoundFX sfxExplosion;
        RSDK::SoundFX sfxDestroy;
        int32 explosionSfxTimer;
        RSDK::Vector2 offsets[7];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::Animator animator;
    int32 planeFilter;
    uint8 type;
    uint8 childType;
    RSDK::GameObject::Entity *parent;
    uint16 field_90;
    int16 spawnRangeX;
    uint16 field_94;
    int16 spawnRangeY;
    RSDK::Vector2 origin;
    RSDK::Vector2 *offset;
    uint16 offsetTablePos;
    int16 timer;
    bool32 harmful;
    bool32 disableExplosionSfx;
    int32 parameter;
    int32 aniID;
    int32 soundID;
    RSDK::Vector2 acceleration;
    uint8 screenRelative;
    int32 field_CC;

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
    Explosion *Spawn(RSDK::GameObject::Entity *parent, RSDK::Vector2 position, uint8 drawOrder, int16 rangeX, int16 rangeY, int16 timer,
                                int16 tablePos, uint8 childType);
    void SpawnGroup(RSDK::GameObject::Entity *parent, RSDK::Vector2 *offset, uint16 count, RSDK::Vector2 *origin, RSDK::SpriteAnimation *aniFrames,
                          uint16 *aniIDs, uint16 *timers, uint8 drawOrder, bool32 isHarmful);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Explosion);
};
} // namespace GameLogic