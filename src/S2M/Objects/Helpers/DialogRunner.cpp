// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: DialogRunner Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "DialogRunner.hpp"
#include "Global/SaveGame.hpp"
#include "Options.hpp"
#include "TimeAttackData.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(DialogRunner);

void DialogRunner::Update() { this->state.Run(this); }
void DialogRunner::Create(void *data)
{
    this->active         = ACTIVE_ALWAYS;
    this->visible        = 0;
    this->callback       = (void (*)())data;
    this->timer          = 0;
    this->useGenericText = false;
}

void DialogRunner::StageLoad()
{
    sVars->authForbidden = false;
    sVars->signedOut     = false;
    sVars->unused2       = 0;
    sVars->unused1       = 0;
    sVars->isAutoSaving  = false;

    sVars->activeCallback = NULL;

    SaveGame::LoadSaveData();

    TimeAttackData::sVars->loaded          = false;
    TimeAttackData::sVars->uuid            = 0;
    TimeAttackData::sVars->rowID           = -1;
    TimeAttackData::sVars->personalRank    = 0;
    TimeAttackData::sVars->leaderboardRank = 0;
    TimeAttackData::sVars->isMigratingData = false;

    Options::sVars->changed = false;
    if (SKU->platform && SKU->platform != PLATFORM_DEV) {
        Options *options         = Options::GetOptionsRAM();
        options->vSync           = false;
        options->windowed        = false;
        options->windowBorder    = false;
        options->tripleBuffering = false;
    }
    else {
        Options::Reload();
    }
}

#if RETRO_REV0U
void DialogRunner::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(DialogRunner); }
#endif

} // namespace GameLogic