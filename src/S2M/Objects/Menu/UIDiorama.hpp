#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"

namespace GameLogic
{

struct UIDiorama : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum UIDioramaIDs {
        UIDIORAMA_MANIAMODE,
        UIDIORAMA_PLUSUPSELL,
        UIDIORAMA_ENCOREMODE,
        UIDIORAMA_TIMEATTACK,
        UIDIORAMA_COMPETITION,
        UIDIORAMA_OPTIONS,
        UIDIORAMA_EXTRAS,
        UIDIORAMA_EXIT,
    } ;

    enum UIDioramaAlts {
        UIDIORAMA_ALT_RUN,
        UIDIORAMA_ALT_JOG,
        UIDIORAMA_ALT_LOOP,
    };

    // ==============================
    // STRUCTS
    // ==============================

    struct UIDiorama_StateInfo_ManiaMode {
        int32 scrollPos;
        int32 clouds1ScrollPos;
        int32 clouds2ScrollPos;
        int32 clouds3ScrollPos;
        int32 valuesPadding[12];

        RSDK::Vector2 playerPos;
        RSDK::Vector2 vectorsPadding[15];
        
        RSDK::Animator terrainAnimator;
        RSDK::Animator clouds1Animator;
        RSDK::Animator clouds2Animator;
        RSDK::Animator clouds3Animator;
        RSDK::Animator mountainsAnimator;
        RSDK::Animator backgroundAnimator;
        RSDK::Animator sonicAnimator;
        RSDK::Animator tailsAnimator;
        RSDK::Animator animatorsPadding[8];
    };

    struct UIDiorama_StateInfo_PlusUpsell {
        bool32 showFlash;
        int32 flashAlpha;
        int32 unused;
        int32 delay;
        int32 lineCount;
        int32 linePos[4];
        int32 valuesPadding[7];

        RSDK::Vector2 plusPos;
        RSDK::Vector2 plusVelocity;
        RSDK::Vector2 vectorsPadding[14];
        
        RSDK::Animator dioramaAnimator;
        RSDK::Animator flashAnimator;
        RSDK::Animator logoAnimator;
        RSDK::Animator plusAnimator;
        RSDK::Animator arrowAnimator;
        RSDK::Animator lightningAnimator;
        RSDK::Animator textAnimator;
        RSDK::Animator animatorsPadding[9];
    };

    struct UIDiorama_StateInfo_EncoreMode {
        int32 valuesPadding[16];

        RSDK::Vector2 vectorsPadding[16];
        
        RSDK::Animator dioramaAnimator;
        RSDK::Animator capsuleAnimator;
        RSDK::Animator buttonAnimator;
        RSDK::Animator glassAnimator;
        RSDK::Animator mightyAnimator;
        RSDK::Animator rayAnimator;
        RSDK::Animator animatorsPadding[10];
    };

    struct UIDiorama_StateInfo_TimeAttack {
        int32 valuesPadding[16];

        RSDK::Vector2 vectorsPadding[16];
        
        RSDK::Animator dioramaAnimator;
        RSDK::Animator sonicAnimator;
        RSDK::Animator ringAnimator;
        RSDK::Animator gateBaseAnimator;
        RSDK::Animator gateTopAnimator;
        RSDK::Animator gateFinsAnimator;
        RSDK::Animator animatorsPadding[10];
    };

    struct UIDiorama_StateInfo_Competition {
        int32 tailsAngle;
        int32 rayAngle;
        int32 scrollPos[9];
        int32 valuesPadding[5];

        RSDK::Vector2 terrainPos;
        RSDK::Vector2 platformPos;
        RSDK::Vector2 tailsPos;
        RSDK::Vector2 knuxPos;
        RSDK::Vector2 rayPos;
        RSDK::Vector2 mightyPos;
        RSDK::Vector2 vectorsPadding[10];
        
        RSDK::Animator dioramaAnimator;
        RSDK::Animator platformAnimator;
        RSDK::Animator ringAnimator;
        RSDK::Animator tailsAnimator;
        RSDK::Animator knuxAnimator;
        RSDK::Animator rayAnimator;
        RSDK::Animator mightyAnimator;
        RSDK::Animator animatorsPadding[9];
    };

    struct UIDiorama_StateInfo_Options {
        int32 itemAlpha;
        int32 contAlpha;
        int32 audioAlpha;
        int32 itemAngle;
        int32 contAngle;
        int32 audioAngle;
        int32 valuesPadding[10];

        RSDK::Vector2 itemPos;
        RSDK::Vector2 contPos;
        RSDK::Vector2 audioPos;
        RSDK::Vector2 itemOffset;
        RSDK::Vector2 contOffset;
        RSDK::Vector2 audioOffset;
        RSDK::Vector2 vectorsPadding[10];
        
        RSDK::Animator dioramaAnimator;
        RSDK::Animator sonicAnimator;
        RSDK::Animator tailsAnimator;
        RSDK::Animator knuxAnimator;
        RSDK::Animator itemConstellationAnimator;
        RSDK::Animator contConstellationAnimator;
        RSDK::Animator audioConstellationAnimator;
        RSDK::Animator animatorsPadding[9];
    };

    struct UIDiorama_StateInfo_Extras {
        int32 valuesPadding[16];

        RSDK::Vector2 vectorsPadding[16];
        
        RSDK::Animator dioramaAnimator;
        RSDK::Animator medalAnimator;
        RSDK::Animator sonicAnimator;
        RSDK::Animator animatorsPadding[13];
    };

    struct UIDiorama_StateInfo_Exit {
        int32 animDelay;
        bool32 processVelocity;
        bool32 isOffScreen;
        int32 valuesPadding[13];

        RSDK::Vector2 sonicPos;
        RSDK::Vector2 sonicVelocity;
        RSDK::Vector2 vectorsPadding[14];
     
        RSDK::Animator dioramaAnimator;
        RSDK::Animator sonicAnimator;
        RSDK::Animator animatorsPadding[14];
    };

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
        RSDK::SpriteAnimation capsuleFrames;
        RSDK::SpriteAnimation sonicFrames;
        RSDK::SpriteAnimation tailsFrames;
        RSDK::SpriteAnimation knuxFramesAIZ;
        RSDK::SpriteAnimation knuxFramesHCZ;
        RSDK::SpriteAnimation mightyFrames;
        RSDK::SpriteAnimation rayFrames;
        RSDK::SpriteAnimation ringFrames;
        RSDK::SpriteAnimation speedGateFrames;
        RSDK::SpriteAnimation bssSonicFrames;
        RSDK::SpriteAnimation bssFrames;
        uint8 dioramaAlt;
    };

    int32 dioramaID;
    int32 lastDioramaID;
    int32 timer;
    uint8 parentActivity;
    bool32 needsSetup;
    RSDK::StateMachine<UIDiorama> state;
    RSDK::StateMachine<UIDiorama> stateDraw;
    UIControl *parent;
    int32 maskColor;
    RSDK::Vector2 dioramaPos;
    RSDK::Vector2 dioramaSize;
    RSDK::Animator maskAnimator;
    RSDK::Animator staticAnimator;
    int32 values[16];
    RSDK::Vector2 vectors[16];
    RSDK::Animator animators[16];
    RSDK::String texts[16];

    // ==============================
    // INSTANCE VARS
    // ==============================

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

    void ChangeDiorama(uint8 dioramaID);
    void SetText(RSDK::String *string);

    // ==============================
    // STATES
    // ==============================

    void State_ManiaMode_Alt_Run();
    void State_ManiaMode_Alt_Jog();
    void State_ManiaMode_Alt_Loop();
    void State_PlusUpsell();
    void State_EncoreMode();
    void State_TimeAttack();
    void State_Competition();
    void State_Options();
    void State_Extras();
    void State_Exit();

    // ==============================
    // DRAW STATES
    // ==============================

    void Draw_ManiaMode();
    void Draw_PlusUpsell();
    void Draw_EncoreMode();
    void Draw_TimeAttack();
    void Draw_Competition();
    void Draw_Options();
    void Draw_Extras();
    void Draw_Exit();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UIDiorama);
};
} // namespace GameLogic