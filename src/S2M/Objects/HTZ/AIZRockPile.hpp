#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct AIZRockPile : RSDK::GameObject::Entity {

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
        int32 rockPositions_small[8];   
        int32 rockSpeedsT_small[8];
        int32 rockSpeedsL_small[8];
        int32 rockSpeedsR_small[8];     
        RSDK::SpriteAnimation aniFrames;
        RSDK::SoundFX sfxBreak;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    bool32 smashTop;
    bool32 smashSides;
    bool32 onlyKnux;
    int32 *rockPositions;
    int32 *rockSpeedsT;
    int32 *rockSpeedsL;
    int32 *rockSpeedsR;
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

    void DebugDraw();
    void DebugSpawn();
    void SpawnRocks(int32 *speeds);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(AIZRockPile);
};
} // namespace GameLogic
