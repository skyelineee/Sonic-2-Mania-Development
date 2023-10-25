// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: DASetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "DASetup.hpp"
#include "UIControl.hpp"
#include "UIInfoLabel.hpp"
#include "UIWidgets.hpp"
#include "UIPicture.hpp"
#include "Global/Localization.hpp"
#include "Global/Music.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Helpers/RPCHelpers.hpp"

using namespace RSDK;

// changed the fading system here bc the mania method wasnt working here, i think the fxfade object just works differently here compared to mania so
namespace GameLogic
{
RSDK_REGISTER_OBJECT(DASetup);

void DASetup::Update()
{
    this->state.Run(this);

    if (sVars->returnToMenu) {
        Music::FadeOut(0.02);
        if (this->timer >= 1024) {
            Stage::SetScene("Presentation", "Menu");
            Stage::LoadScene();
        }
        else {
            this->timer += 16;
        }
    }
}

void DASetup::LateUpdate() {}

void DASetup::StaticUpdate()
{
    if (!sVars->initialized) {
        DASetup::SetupUI();
        sVars->initialized = true;
    }

    UIControl *control = sVars->control;
    if (!control->childHasFocus)
        control->childHasFocus = true;

    // added!!!
    // just cycles through the picture entities, finds sonic, and changes his animation depending on if a track is playing or not
    for (auto picture : GameObject::GetEntities<UIPicture>(FOR_ALL_ENTITIES)) {
        if (picture->listID == 0) {
            if (sVars->activeTrack != Music::TRACK_NONE) {
                picture->animator.SetAnimation(UIPicture::sVars->aniFrames, 1, false, 0);
            }
            else {
                picture->animator.SetAnimation(UIPicture::sVars->aniFrames, 0, false, 0);
            }
        }
    }
}

void DASetup::Draw() { this->stateDraw.Run(this); }

void DASetup::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_ALWAYS;
        this->visible   = true;
        this->drawGroup = 12;
        this->drawFX    = FX_FLIP;
        this->state.Set(&DASetup::State_Idle);
        this->stateDraw.Set(&DASetup::Draw_Fade);

        this->timer = 0x300;
    }
}

void DASetup::StageLoad()
{
    for (auto control : GameObject::GetEntities<UIControl>(FOR_ALL_ENTITIES))
    {
        sVars->control              = control;
        control->processButtonInputCB.Set(&DASetup::State_ManageControl);
    }

    for (auto label : GameObject::GetEntities<UIInfoLabel>(FOR_ALL_ENTITIES))
    {
        if (sVars->trackTitleLabel) {
            if (!sVars->trackSelLabel) {
                sVars->trackSelLabel = label;
                break;
            }
        }
        else {
            sVars->trackTitleLabel = label;
        }
    }

    int32 trackCount = 0;
    for (auto track : GameObject::GetEntities<Music>(FOR_ALL_ENTITIES)) { sVars->trackList[trackCount++] = track; }

    sVars->trackCount  = trackCount;
    sVars->activeTrack = Music::TRACK_NONE;

    sVars->sfxEmerald.Get("Special/Emerald.wav");
    sVars->sfxMedal.Get("Special/Medal.wav");
    sVars->sfxSSExit.Get("Special/SSExit.wav");
    sVars->sfxScoreTotal.Get("Global/ScoreTotal.wav");

    SetPresence("", "In Cabaret", "doggy", "doggy", "", "");
}

void DASetup::State_Idle()
{
    if (this->timer <= 0) {
        this->timer = 0;
        this->stateDraw.Set(nullptr);
    }
    else {
        this->timer -= 16;
    }

    if (!sVars->returnToMenu && UIControl::sVars->anyBackPress) {
        sVars->returnToMenu = true;

        this->state.Set(nullptr);
        this->stateDraw.Set(&DASetup::Draw_Fade);
    }
}

void DASetup::Draw_Fade() { Graphics::FillScreen(0x000000, this->timer, this->timer - 128, this->timer - 256); }

void DASetup::DisplayTrack(int32 trackID)
{
    char buffer[0x10];
    String text;
    text = {};

    UIInfoLabel *trackTitleLabel = sVars->trackTitleLabel;
    Music *trackCountTrack       = sVars->trackList[trackID];

    memset(buffer, 0, 0x10 * sizeof(char));
    strcpy(&buffer[2], " - ");
    buffer[0] = (trackID / 10) + '0';
    buffer[1] = trackID - 10 * (trackID / 10) + '0';
    text.Set(buffer);
    RSDKTable->AppendString(&text, &trackCountTrack->soundTestTitle);
    UIInfoLabel::SetString(trackTitleLabel, &text);
}

void DASetup::SetupUI()
{
    String buffer;
    buffer = {};

    DASetup::DisplayTrack(0);
    UIInfoLabel *trackSelLabel = sVars->trackSelLabel;
    Localization::GetString(&buffer, Localization::SelectATrack);
    LogHelpers::PrintString(&buffer);
    UIInfoLabel::SetString(trackSelLabel, &buffer);
}

void DASetup::State_ManageControl()
{
    int32 prevTrack = sVars->trackID;
    if (UIControl::sVars->anyRightPress)
        sVars->trackID++;
    else if (UIControl::sVars->anyLeftPress)
        sVars->trackID--;
    else if (UIControl::sVars->anyUpPress)
        sVars->trackID += 10;
    else if (UIControl::sVars->anyDownPress)
        sVars->trackID -= 10;

    if (sVars->trackID < 0)
        sVars->trackID += sVars->trackCount;
    if (sVars->trackID >= sVars->trackCount)
        sVars->trackID -= sVars->trackCount;

    if (prevTrack != sVars->trackID) {
        UIWidgets::sVars->sfxBleep.Play(false, 255);
        DASetup::DisplayTrack(sVars->trackID);
    }

    if (UIControl::sVars->anyConfirmPress) {
        if (sVars->activeTrack == sVars->trackID) {
            Music::Stop();
            sVars->activeTrack = Music::TRACK_NONE;
        }
        else {
            Music *track = sVars->trackList[sVars->trackID];
            //if (!DASetup::HandleMedallionDebug()) {
                if (track->trackFile.length) {
                    sVars->activeTrack = sVars->trackID;
                    track->Play();
                }
                else {
                    sVars->activeTrack = Music::TRACK_NONE;
                    Music::Stop();
                }
            //}
        }
    }
}

#if RETRO_INCLUDE_EDITOR
void DASetup::EditorDraw() {}

void DASetup::EditorLoad() {}
#endif

void DASetup::Serialize() {}
} // namespace GameLogic