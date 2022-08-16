// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: FXAniTiles Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "FXAniTiles.hpp"

#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(FXAniTiles);

void FXAniTiles::Update()
{
    if (this->animate)
        this->animator.Process();

    if (!this->disable)
        this->animator.DrawAniTiles(this->tileID);
}
void FXAniTiles::LateUpdate() {}
void FXAniTiles::StaticUpdate() {}
void FXAniTiles::Draw() { this->animator.DrawSprite(&this->position, false); }

void FXAniTiles::Create(void *data)
{
    if (sceneInfo->inEditor) {
        this->animate = true;
    }
    else {
        this->active = ACTIVE_NORMAL;
        this->animator.SetAnimation(sVars->aniFrames, this->aniID, true, this->frameID);
    }
}

void FXAniTiles::StageLoad()
{
    RSDK_DYNAMIC_PATH_ACTID("AniTiles");

    sVars->aniFrames.Load(dynamicPath, SCOPE_STAGE);
}

#if RETRO_INCLUDE_EDITOR
void FXAniTiles::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 10);
    this->animator.DrawSprite(&this->position, false);
}

void FXAniTiles::EditorLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void FXAniTiles::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(FXAniTiles);

    sVars->aniFrames.Init();
}
#endif

void FXAniTiles::Serialize()
{
    RSDK_EDITABLE_VAR(FXAniTiles, VAR_UINT32, aniID);
    RSDK_EDITABLE_VAR(FXAniTiles, VAR_UINT32, frameID);
    RSDK_EDITABLE_VAR(FXAniTiles, VAR_UINT32, tileID);
    RSDK_EDITABLE_VAR(FXAniTiles, VAR_BOOL, animate);
    RSDK_EDITABLE_VAR(FXAniTiles, VAR_BOOL, disable);
    RSDK_EDITABLE_VAR(FXAniTiles, VAR_INT32, controlFlag);
}

} // namespace GameLogic