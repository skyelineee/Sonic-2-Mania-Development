// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SaveMenuIconBG Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "SaveMenuIconBG.hpp"
#include "UISaveSlot.hpp"
#include "UIControl.hpp"
#include "ManiaModeMenu.hpp"
#include "Global/SaveGame.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SaveMenuIconBG);

void SaveMenuIconBG::Update(){ this->staticAnimator.Process(); }
void SaveMenuIconBG::LateUpdate() {}
void SaveMenuIconBG::StaticUpdate() {}
void SaveMenuIconBG::Draw() 
{
    Vector2 drawPos;

    bool32 changeIcon = this->lastZoneIcon != this->zoneIconAnimator.frameID;

    drawPos.x = TO_FIXED(0);
    drawPos.y = TO_FIXED(16);

    UIControl *control   = ManiaModeMenu::sVars->saveSelectMenu;
    UISaveSlot *saveSlot = (UISaveSlot *)control->buttons[control->lastButtonID];
    SaveGame::SaveRAM *saveRAM = (SaveGame::SaveRAM *)SaveGame::GetSaveDataPtr(saveSlot->slotID);
    int32 saveState            = saveRAM->saveState;

    if (saveSlot->disabled)
        this->timer = 0;

    if (saveSlot->type == UISaveSlot::UISAVESLOT_REGULAR) {
        this->zoneIconAnimator.DrawSprite(&drawPos, true);
        this->zoneIconAnimator.frameID = saveSlot->zoneNameAnimator.frameID;
        if (changeIcon)
            this->timer = 0;
        if (this->timer <= 12) {
            this->timer++;
            this->staticAnimator.DrawSprite(&drawPos, true);
            this->lastZoneIcon = this->zoneIconAnimator.frameID;
        }
        if (saveState == SaveGame::SaveCompleted) {
            if (saveSlot->saveZoneID == 0xFF) 
               this->characterIconAnimator.DrawSprite(&drawPos, true);
               switch (saveSlot->fileAnimator.frameID) {
                   case 0: this->characterIconAnimator.frameID = 0; break;
                   case 1: this->characterIconAnimator.frameID = 1; break;
                   case 2: this->characterIconAnimator.frameID = 2; break;
                   case 3: this->characterIconAnimator.frameID = 3; break;
               }
        }
        if (saveSlot->isNewSave) {
            this->staticAnimator.DrawSprite(&drawPos, true);
        }
    }
    else {
        this->staticAnimator.DrawSprite(&drawPos, true);
    }
}

void SaveMenuIconBG::Create(void *data)
{
    this->staticAnimator.SetAnimation(&UISaveSlot::sVars->aniFrames, 22, true, 0);
    this->zoneIconAnimator.SetAnimation(&UISaveSlot::sVars->aniFrames, 23, true, 0);
    this->characterIconAnimator.SetAnimation(&UISaveSlot::sVars->aniFrames, 24, true, 0);

    this->lastZoneIcon = -1;
    this->active    = ACTIVE_BOUNDS;
    this->visible   = true;
    this->drawGroup = 1;
    this->timer     = 0;
}

void SaveMenuIconBG::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void SaveMenuIconBG::EditorDraw() {}

void SaveMenuIconBG::EditorLoad() {}
#endif

void SaveMenuIconBG::Serialize() {}

} // namespace GameLogic