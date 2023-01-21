#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct DialogRunner : RSDK::GameObject::Entity {

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
        int32 authForbidden;
        bool32 signedOut;
        int32 unused1;
        int32 unused2;
        bool32 isAutoSaving;
        void *activeCallback;
        int32 unused3;
        int32 unused4;
    };

    // ==============================
    // INSTANCE VARS
    // ==============================
    RSDK::StateMachine<DialogRunner> state;
    int32 timer;
    int32 unused[20];
    bool32 useGenericText;
    RSDK::Action<void> callback;
    int32 status;

    // ==============================
    // EVENTS
    // ==============================

    void Create(void *data);
    void Update();

    static void StageLoad();
#if RETRO_REV0U
    static void StaticLoad(Static *sVars);
#endif

    // ==============================
    // FUNCTIONS
    // ==============================

    void HandleCallback();
    static void NotifyAutoSave_CB();
    void NotifyAutoSave();
    static void SetNoSaveDisabled();
    static void SetNoSaveEnabled();
    void PromptSavePreference_CB();
    void CheckUserAuth_CB();
    void ManageNotifs();
    static void TrackGameProgressCB(bool32 success);
    static void GetNextNotif();
    static bool32 CheckUnreadNotifs();
    static bool32 NotifyAutosave();
    static void GetUserAuthStatus();
    static void PromptSavePreference(int32 id);
    static void CheckUserAuth_OK();

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(DialogRunner);
};
} // namespace GameLogic