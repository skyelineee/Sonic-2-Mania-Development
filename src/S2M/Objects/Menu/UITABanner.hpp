#pragma once
#include "S2M.hpp"
#include "UIControl.hpp"

namespace GameLogic
{

struct UITABanner : RSDK::GameObject::Entity {

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
    };

    // ==============================
    // INSTANCE VARS
    // ==============================

    RSDK::StateMachine<UITABanner> state;
    RSDK::Vector2 startPos;
    UIControl *parent;
    int32 timer;
    uint8 characterID;
    uint8 zoneID;
    uint8 actID;
    RSDK::Animator bannerAnimator;
    RSDK::Animator textAnimator;
    RSDK::Animator zoneNameAnimator;

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

    static void SetupDetails(UITABanner *banner, uint8 zoneID, uint8 actID, uint8 characterID);
    void DrawStageInfo(int32 drawX, int32 drawY, uint8 actID, uint8 characterID);
    void DrawBannerInfo(int32 drawX, int32 drawY, uint8 zoneID, uint8 actID, uint8 characterID);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(UITABanner);
};
} // namespace GameLogic