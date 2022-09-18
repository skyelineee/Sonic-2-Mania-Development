#pragma once
#include "S2M.hpp"

namespace GameLogic
{
struct Bridge : RSDK::GameObject::Entity {

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
    };

    uint8 length;
    bool32 burnable;
    uint8 burnOffset;
    uint8 stoodEntityCount;
    uint8 timer;
    int32 stoodPos;
    int32 bridgeDepth;
    int32 depression;
    void *stoodEntity;
    int32 startPos;
    int32 endPos;
    RSDK::Animator animator;

    // EVENTS //

    void Create(void *data);
    void Draw();
    void Update();
    void LateUpdate();

    static void StaticUpdate();
    static void StageLoad();
    static void Serialize();
#if RETRO_INCLUDE_EDITOR
    void EditorDraw();
    static void EditorLoad();
#endif

    // FUNCTIONS //

    void DebugDraw();
    void DebugSpawn();

    void Burn(int32 offset);
    bool32 HandleCollisions(void *e, Bridge *self, RSDK::Hitbox *entityHitbox, bool32 updateVars, bool32 isPlayer);

    RSDK_DECLARE(Bridge);
};
} // namespace GameLogic