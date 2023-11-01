// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIVideo Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIVideo.hpp"
#include "Global/Music.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIVideo);

void UIVideo::Update() { this->state.Run(this); }

void UIVideo::LateUpdate() {}

void UIVideo::StaticUpdate() {}

void UIVideo::Draw() {}

void UIVideo::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawGroup = 0;

        sVars->playing = false;
        this->state.Set(&UIVideo::State_PlayVideo1);
    }
}

void UIVideo::StageLoad() {}

bool32 UIVideo::SkipCB()
{
    if (controllerInfo->keyStart.press || controllerInfo->keyA.press || controllerInfo->keyB.press) {
        Music::FadeOut(0.0125);

        sVars->playing = true;
        return true;
    }

    return false;
}

void UIVideo::State_PlayVideo1()
{
    if (!this->timer)
        Music::Stop();

    if (++this->timer >= 16) {
        char videoFile1[64];
        char audioFile[64];
        RSDKTable->GetCString(audioFile, &this->audioFile);
        RSDKTable->GetCString(videoFile1, &this->videoFile1);

        int32 len = this->videoFile1.length;
        if (videoFile1[len - 3] == 'p' && videoFile1[len - 2] == 'n' && videoFile1[len - 1] == 'g')
            RSDKTable->LoadImage(videoFile1, 32.0, 1.0, UIVideo::SkipCB);
        else
            RSDKTable->LoadVideo(videoFile1, 0.0, UIVideo::SkipCB);

        this->state.Set(&UIVideo::State_PlayVideo2);
        RSDKTable->PlayStream(audioFile, Music::sVars->channelID, 0, false, false);

        this->timer = 0;
    }
}
void UIVideo::State_PlayVideo2()
{
    if (this->videoFile2.length && !sVars->playing) {
        char videoFile2[64];
        RSDKTable->GetCString(videoFile2, &this->videoFile2);
        RSDKTable->LoadVideo(videoFile2, 14.0, nullptr);
    }

    this->state.Set(&UIVideo::State_FinishPlayback);
}
void UIVideo::State_FinishPlayback()
{
    if (++this->timer == 120) {
        if (this->gotoStage) {
            char categoryName[64];
            char sceneName[64];
            RSDKTable->GetCString(categoryName, &this->stageListCategory);
            RSDKTable->GetCString(sceneName, &this->stageListName);

            Stage::SetScene(categoryName, sceneName);

            if (!Stage::CheckValidScene())
                Stage::SetScene("Presentation", "Title Screen");

            Stage::LoadScene();
        }

        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void UIVideo::EditorDraw() {}

void UIVideo::EditorLoad() {}
#endif

void UIVideo::Serialize()
{
    RSDK_EDITABLE_VAR(UIVideo, VAR_STRING, videoFile1);
    RSDK_EDITABLE_VAR(UIVideo, VAR_STRING, videoFile2);
    RSDK_EDITABLE_VAR(UIVideo, VAR_STRING, audioFile);
    RSDK_EDITABLE_VAR(UIVideo, VAR_BOOL, gotoStage);
    RSDK_EDITABLE_VAR(UIVideo, VAR_STRING, stageListCategory);
    RSDK_EDITABLE_VAR(UIVideo, VAR_STRING, stageListName);
}

} // namespace GameLogic