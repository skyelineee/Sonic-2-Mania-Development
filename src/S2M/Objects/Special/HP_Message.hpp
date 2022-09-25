#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct HP_Message : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================
    enum Types {
        Generic,
        StartMessage,
        RingReminder,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        uint16 numFrames[10];
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<HP_Message> state;
    RSDK::StateMachine<HP_Message> stateDraw;
    RSDK::Animator charAnimator;
    RSDK::String message[4];
    uint8 type;
    Vector3 localPos;
    Vector3 localShadowPos;
    Vector3 worldPos;
    Vector3 worldShadowPos;
    int32 timer;
    int32 duration;
    bool32 resetRings;
    int32 charPos;
    int32 number;
    int32 angleOffset;
    int32 scaleSpeed;
    int32 rotateSpeed;
    int32 spinDirection;
    color fadeColor;

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

    void SetMessage(void (HP_Message::*state)(), int32 number, int32 duration, bool32 resetRings, ...);
    void SetupAniChars(uint8 id);
    void DrawMessage(uint8 id);

    // States
    void State_SingleMessage();
    void State_NotEnough();
    void State_GetRings();
    void State_RingReminder();
    void State_AniChar();

    // Start Message
    void State_InitStartMessage();
    void State_StartMessageDelay();
    void State_StartMessageSendDelay();
    void State_StartMessageStartFadeIn();
    void State_StartMessageFadeIn();
    void State_StartMessageEnterMessage();
    void State_StartMessageShowMessage();
    void State_StartMessageSetupNextMsg();

    // Ring Reminder
    void State_RingReminderTrigger();

    // Draw States
    void Draw_Message();
    void Draw_NotEnough();
    void Draw_GetRings();
    void Draw_RingReminder();
    void Draw_AniChar();
    void Draw_StartMessage();
    void Draw_Fade();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(HP_Message);
};
} // namespace GameLogic