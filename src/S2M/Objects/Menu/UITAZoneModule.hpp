#pragma once
#include "S2M.hpp"
#include "UIButtonPrompt.hpp"

namespace GameLogic
{

struct UITAZoneModule : RSDK::GameObject::Entity {

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
    };

    S2M_UI_ITEM_BASE(UITAZoneModule);
    uint8 zoneID;
    bool32 wasDisabled;
    RSDK::Vector2 drawPos;
    uint8 characterID;
    uint8 actID;
    RSDK::Animator buttonAnimator;
    RSDK::Animator buttonZoneAnimator;
    RSDK::Animator descriptionAnimator;
    int32 descriptionListID;
    int32 descriptionFrameID;
    int32 buttonBounceOffset;
    int32 buttonBounceVelocity;
    bool32 reverseVelocity;
    bool32 nameVisible;
    bool32 currentlySelected;
    int32 descAngle;
    RSDK::Vector2 descOffset;
    int32 startDescriptionListID;
    int32 startDescriptionFrameID;

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

    void Setup();
    void FailCB();
    void SelectedCB();
    bool32 CheckButtonEnterCB();
    bool32 CheckSelectedCB();
    void ButtonEnterCB();
    void ButtonLeaveCB();
    void ProcessButtonCB();

    // ==============================
    // STATES
    // ==============================

    void State_Init();
    void State_NotSelected();
    void State_Selected();
    void State_HasBeenSelected();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UITAZoneModule);
};
} // namespace GameLogic