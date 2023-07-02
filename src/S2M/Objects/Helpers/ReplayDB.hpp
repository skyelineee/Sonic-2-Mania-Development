#pragma once
#include "S2M.hpp"

namespace GameLogic
{

struct ReplayDB : RSDK::GameObject::Entity {

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
        Entity *loadEntity;
        void (*loadCallback)(bool32 success);
        Entity *saveEntity;
        void (*saveCallback)(bool32 success);
        Entity *deleteEntity;
        void (*deleteCallback)(bool32 success);
    };

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

    static void CreateDB();
    static void LoadDB(void (*callback)(bool32 success));
    static void SaveDB(void (*callback)(bool32 success));
    static uint32 AddReplay(uint8 zoneID, uint8 act, uint8 characterID, int32 score);
    static void DeleteReplay(int32 row, void (*callback)(bool32 success), bool32 useAltCB);
    static void DeleteReplay_CB(int32 status);
    static void DeleteReplaySave_CB(int32 status);
    static void DeleteReplaySave2_CB(int32 status);
    static void LoadDBCallback(int32 status);
    static void SaveDBCallback(int32 status);
    static void LoadCallback(bool32 success);

    static int32 Buffer_PackEntry(uint8 *compressed, void *uncompressed);
    static int32 Buffer_UnpackEntry(void *uncompressed, uint8 *compressed);

    // ==============================
    // DECLARATION
    // ==============================

    RSDK_DECLARE(ReplayDB);
};
} // namespace GameLogic