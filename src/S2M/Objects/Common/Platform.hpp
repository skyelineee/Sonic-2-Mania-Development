#pragma once
#include "Game.hpp"


#include "Global/Player.hpp"

namespace GameLogic
{

struct Platform : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum Types {
        Fixed,
        Fall,
        Linear,
        Circular,
        Swing,
        Path,
        Push,
        Track,
        React,
        HoverReact,
        DoorSlide,
        PathReact,
        SwingReact,
        TrackReact,
        Clacker,
        Child,
        Null,
    };

    enum CollisionTypes {
        C_Platform,
        C_Solid,
        C_Tiled,
        C_Hurt,
        C_None,
        C_SolidHurtSides,
        C_SolidHurtBottom,
        C_SolidHurtTop,
        C_SolidHold,
        C_SolidSticky,
        C_StickyTop,
        C_StickyLeft,
        C_StickyRight,
        C_StickyBottom,
        C_SolidBarrel,
        C_SolidNoCrush,
        C_SolidHurtAll,
        C_SolidHurtNoCrush,
        C_Null,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::Vector2 stoodPos[4];
        RSDK::SoundFX sfxClacker;
        RSDK::SoundFX sfxClang;
        RSDK::SoundFX sfxPush;
        RSDK::SoundFX sfxClack;
        bool32 useClack;
        bool32 playingPushSfx;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    // Platform Core
    RSDK::StateMachine<Platform> state;
    RSDK::StateMachine<Platform> stateCollide;
    int32 type;
    RSDK::Vector2 amplitude;
    int32 speed;
    bool32 hasTension;
    int8 frameID;
    uint8 collision;
    RSDK::Vector2 tileOrigin;
    RSDK::Vector2 centerPos;
    RSDK::Vector2 drawPos;
    RSDK::Vector2 collisionOffset;
    int32 stood;
    int32 timer;
    int32 stoodAngle;
    uint8 stoodPlayers;
    uint8 pushPlayersL;
    uint8 pushPlayersR;
    RSDK::Hitbox hitbox;
    RSDK::Animator animator;
    int32 childCount;

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

    // States
    void State_Fixed();
    void State_Fall();
    void State_Falling();
    void State_Falling2();
    void State_Hold();
    void State_Linear();
    void State_Circular();
    void State_Swing();
    void State_Clacker();
    void State_Push_Init();
    void State_Push();
    void State_Push_SlideOffL();
    void State_Push_SlideOffR();
    void State_Push_Fall();
    void State_Path();
    void State_PathStop();
    void State_Track();
    void State_React();
    void State_Hover_React();
    void State_ReactMove();
    void State_ReactSlow();
    void State_Hover();
    void State_ReactWait();
    void State_DoorSlide();
    void State_PathReact();
    void State_Child();
    void State_SwingReact();
    void State_Swing2();
    void State_SwingWait();
    void State_SwingReturn();
    void State_TrackReact();
    void State_Track2();
    void State_TrackWait();
    void State_TrackReturn();

    // Collision States
    void Collision_Platform();
    void Collision_Solid();
    void Collision_Solid_NoCrush();
    void Collision_Solid_Hurt_Sides();
    void Collision_Solid_Hurt_Bottom();
    void Collision_Solid_Hurt_Top();
    void Collision_Solid_Hold();
    void Collision_Solid_Barrel();
    void Collision_Sticky();
    void Collision_Tiles();
    void Collision_Hurt();
    void Collision_None();
    void Collision_Solid_Hurt_All();
    void Collision_Solid_Hurt_NoCrush();

    // Collision Helpers
    void HandleStood(Player *player, int32 playerID, int32 stoodPlayers);
    void HandleStood_Tiles(Player *player, int32 playerID);
    void HandleStood_Hold(Player *player, int32 playerID, int32 stoodPlayers);
    void HandleStood_Barrel(Player *player, int32 playerID, int32 stoodPlayers);
    void HandleStood_Sticky(Player *player, int32 playerID, uint8 cSide);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(Platform);
};
} // namespace GameLogic