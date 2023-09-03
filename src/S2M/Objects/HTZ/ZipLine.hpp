#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct Player;

struct ZipLine : RSDK::GameObject::Entity {

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
        RSDK::Hitbox hitboxHandle;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<ZipLine> state;
    uint8 activePlayers;
    RSDK::Animator animator;
    int32 length;
    RSDK::Vector2 startPos;
    RSDK::Vector2 endPos;
    RSDK::Vector2 handlePos;
    uint8 grabDelay[PLAYER_COUNT];
    RSDK::Vector2 joinPos;
    int32 unused1;

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

    void VSSwap_CheckBusy();
    void GrabHandle(Player *player, int32 playerID, RSDK::Hitbox *playerHitbox);
    void ForceReleasePlayers();
    RSDK::Vector2 GetJoinPos();
    void State_Moving();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ZipLine);
};
} // namespace GameLogic
