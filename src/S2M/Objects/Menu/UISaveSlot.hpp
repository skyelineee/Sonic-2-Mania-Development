#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct UISaveSlot : RSDK::GameObject::Entity {

    // ==============================
    // ENUMS
    // ==============================

    enum UISaveSlotTypes {
        UISAVESLOT_REGULAR,
        UISAVESLOT_NOSAVE,
    };

    // ==============================
    // STRUCTS
    // ==============================

    // ==============================
    // STATIC VARS
    // ==============================

    struct Static : RSDK::GameObject::Static {
        RSDK::SpriteAnimation aniFrames;
    };

    S2M_UI_ITEM_BASE(UISaveSlot);
    bool32 isNewSave;
    RSDK::StateMachine<UISaveSlot> stateInput;
    int32 listID;
    int32 frameID;
    int32 saveZoneID;
    int32 saveLives;
    int32 saveContinues;
    int32 saveEmeralds;
    UISaveSlotTypes type;
    int32 slotID;
    int32 saveSlotPlacement;
    bool32 currentlySelected;
    int32 zoneIconSprX;
    int32 buttonBounceOffset;
    int32 buttonBounceVelocity;
    int32 fxRadius;
    RSDK::Animator uiAnimator;
    RSDK::Animator fileAnimator;
    RSDK::Animator livesAnimator;
    RSDK::Animator continuesAnimator;
    RSDK::Animator emeraldsAnimator;
    RSDK::Animator zoneNameAnimator;
    RSDK::Animator saveStatusAnimator;
    RSDK::Animator numbersAnimator;
    RSDK::Animator slotNumAnimator;
    RSDK::SpriteAnimation textFrames;

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

    static uint8 GetPlayerIDFromID(uint8 id);
    static uint8 GetIDFromPlayerID(uint8 playerID);
    void DrawFileIcons(int32 drawX, int32 drawY);
    void DrawPlayerInfo(int32 drawX, int32 drawY);
    void SetupButtonElements();
    void SetupAnimators();
    void LoadSaveInfo();
    void HandleSaveIcons();

    static void DeleteDLG_CB();
    static void DeleteSaveCB(bool32 success);
    void ProcessButtonCB();
    void SelectedCB();

    void NextCharacter();
    void PrevCharacter();
    void NextZone();
    void PrevZone();

    bool32 CheckButtonEnterCB();
    bool32 CheckSelectedCB();
    void ButtonEnterCB();
    void ButtonLeaveCB();
    void HandleSaveIconChange();

    // ==============================
    // STATES
    // ==============================

    void State_NotSelected();
    void State_OtherWasSelected();
    void State_NewSave();
    void State_ActiveSave();
    void StateInput_NewSave();
    void State_CompletedSave();
    void State_Selected();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UISaveSlot);
};
} // namespace GameLogic