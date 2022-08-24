// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: BGSwitch Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "BGSwitch.hpp"
#include "Global/Zone.hpp"
#include "Global/Camera.hpp"

#include "Helpers/DrawHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(BGSwitch);

void BGSwitch::Update()
{
    if (Zone::sVars->timer != 1) {
        for (sVars->screenID = 0; sVars->screenID < CAMERA_COUNT; sVars->screenID++) {
            Camera *camera = GameObject::Get<Camera>(SLOT_CAMERA1 + sVars->screenID);
            if (!camera->classID)
                break;

            if (abs(camera->position.x - this->position.x) < this->size.x && abs(camera->position.y - this->position.y) < this->size.y) {
                if (this->bgID < 8)
                    sVars->layerIDs[sVars->screenID].Set(this->bgID);
            }

            if (!sVars->layerIDs[sVars->screenID].Matches(sVars->layerIDs[sVars->screenID + CAMERA_COUNT])) {
                sVars->layerIDs[sVars->screenID + CAMERA_COUNT] = sVars->layerIDs[sVars->screenID];
                sVars->switchCallback[sVars->layerIDs[sVars->screenID].id].Run(this);
            }
        }
    }
}
void BGSwitch::LateUpdate() {}
void BGSwitch::StaticUpdate() {}
void BGSwitch::Draw() { this->animator.DrawSprite(&this->position, false); }

void BGSwitch::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->updateRange.x = this->size.x;
        this->updateRange.y = this->size.y;
        this->active        = ACTIVE_BOUNDS;
    }
}

void BGSwitch::StageLoad()
{
    sVars->layerIDs[4].Init();
    sVars->layerIDs[5].Init();
    sVars->layerIDs[6].Init();
    sVars->layerIDs[7].Init();
}

#if RETRO_INCLUDE_EDITOR
void BGSwitch::EditorDraw()
{
    if (showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);
        DrawHelpers::DrawRectOutline(this->position.x, this->position.y, this->size.x, this->size.y, 0xFFFF00);
        RSDK_DRAWING_OVERLAY(false);
    }

    Animator animator;
    animator.SetAnimation(sVars->aniFrames, 0, true, 5);
    animator.DrawSprite(nullptr, false);
}

void BGSwitch::EditorLoad()
{
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, bgID);
    RSDK_ENUM_VAR("Switch to BG0");
    RSDK_ENUM_VAR("Switch to BG1");
    RSDK_ENUM_VAR("Switch to BG2");
    RSDK_ENUM_VAR("Switch to BG3");
    RSDK_ENUM_VAR("Switch to BG4");
    RSDK_ENUM_VAR("Switch to BG5");
    RSDK_ENUM_VAR("Switch to BG6");
    RSDK_ENUM_VAR("Switch to BG7");
    RSDK_ENUM_VAR("Don't switch");
}
#endif

#if RETRO_REV0U
void BGSwitch::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(BGSwitch);

    sVars->aniFrames.Init();
}
#endif

void BGSwitch::Serialize()
{
    RSDK_EDITABLE_VAR(BGSwitch, VAR_VECTOR2, size);
    RSDK_EDITABLE_VAR(BGSwitch, VAR_UINT8, bgID);
}

} // namespace GameLogic