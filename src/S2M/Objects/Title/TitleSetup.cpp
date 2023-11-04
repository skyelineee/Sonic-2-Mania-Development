// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: TitleSetup Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "TitleSetup.hpp"
#include "TitleLogo.hpp"
#include "TitleSonic.hpp"
#include "Twinkle.hpp"
#include "Global/Localization.hpp"
#include "Global/Music.hpp"
#include "Global/Zone.hpp"
#include "Helpers/TimeAttackData.hpp"
#include "Helpers/RPCHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(TitleSetup);

void TitleSetup::Update()
{
    this->state.Run(this);

    screenInfo->position.x = 0x100 - screenInfo->center.x;
}

void TitleSetup::LateUpdate() {}

void TitleSetup::StaticUpdate()
{
    sVars->deformationTimer =
        (sVars->deformationTimer + 1) & 0x7FFF; // need this here as i cant use the one in zone like other setups, bc zone is not an object in title
    if (!(sVars->deformationTimer & 7)) {       // every 7th frame the deformation offset gets added to by 1
        ++sVars->background->deformationOffset;
        ++sVars->backgroundIsland->deformationOffset;
        ++sVars->backgroundRipple->deformationOffset;
    }
}

void TitleSetup::Draw() { this->stateDraw.Run(this); }

void TitleSetup::Create(void *data)
{
    // i dont actually know how it gets here lmao, these are entity events but title setup is not actually placed in hte stage, so i would think itd
    // only do static events and functions copying manias scene directly just has these happen anyway tho so idk
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_ALWAYS;
        this->visible   = true;
        this->drawGroup = 12;
        this->drawFX    = FX_FLIP;
        this->state.Set(&TitleSetup::State_Wait);
        this->stateDraw.Set(&TitleSetup::Draw_FadeBlack); // state draw is set to fade black so the stage can fade in at the start
        this->timer     = 1024;
        this->drawPos.x = 256 << 16;
        this->drawPos.y = 108 << 16;
    }
}

void TitleSetup::StageLoad()
{
    // String presence;
    // Localization::GetString(&presence, Localization::RPC_Title);
    // API::RichPresence::Set(PRESENCE_TITLE, &presence);
    SetPresence("", "In Title Screen", "title", "Title Screen", "", "");

    API::Storage::SetNoSave(false);

    TimeAttackData::Clear();

    API::Auth::ClearPrerollErrors();
    API::Storage::ClearSaveStatus();

    globals->saveLoaded = false;
    memset(globals->saveRAM, 0, sizeof(globals->saveRAM));

    globals->optionsLoaded = false;
    memset(globals->optionsRAM, 0, sizeof(globals->optionsRAM));

    APITable->ClearUserDB(globals->replayTableID);
    APITable->ClearUserDB(globals->taTableID);

    globals->replayTableID     = (uint16)-1;
    globals->replayTableLoaded = STATUS_NONE;
    globals->taTableID         = (uint16)-1;
    globals->taTableLoaded     = STATUS_NONE;

    sVars->sfxMenuAccept.Get("Global/MenuAccept.wav");
    sVars->sfxRing.Get("Global/Ring.wav");
    sVars->deformationTimer = 0;

    RSDKTable->ResetEntitySlot(0, sVars->classID, nullptr);

    // need to do this multiple times as multiple layers need the rippling lol
    sVars->background       = SceneLayer::GetTileLayer(2);
    sVars->backgroundIsland = SceneLayer::GetTileLayer(3);
    sVars->backgroundRipple = SceneLayer::GetTileLayer(4);
    for (int32 i = 0; i < 1024; ++i) {
        // sets the layers' deformation data to this objects deformation data
        sVars->background->deformationData[i]       = sVars->deformation[i & 63];
        sVars->backgroundIsland->deformationData[i] = sVars->deformation[i & 63];
        sVars->backgroundRipple->deformationData[i] = sVars->deformation[i & 63];
    }
}

void TitleSetup::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(TitleSetup);

    // sets the deformation table, the size of which being how many numbers are inside
    int32 deformation[] = { 1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3,
                            1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0, 2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3 };

    // copies the deformation table here to sVars->deformation so it can be used elsewhere
    memcpy(sVars->deformation, deformation, sizeof(deformation));
}

// skips the video while playing one
bool32 TitleSetup::VideoSkipCB()
{
    if (controllerInfo->keyA.press || controllerInfo->keyB.press || controllerInfo->keyStart.press) {
        Music::Stop();
        return true;
    }

    return false;
}

// title startup
void TitleSetup::State_Wait()
{
    if (this->timer <= -300) {
        this->timer = 0;
        this->state.Set(&TitleSetup::State_TwinklesFadingIn);
        this->stateDraw.Set(nullptr); // nothing to change it to, but it needs to stop drawing the fading rectangle, so nullptr
    }
    else {
        this->timer -= 16;
    }
}

void TitleSetup::State_TwinklesFadingIn()
{
    this->timer++;
    for (auto titleLogo : GameObject::GetEntities<TitleLogo>(FOR_ALL_ENTITIES)) { // gets all title logos
        if (titleLogo->type
            == TitleLogo::TITLELOGO_EMBLEM) { // if its the emblem type (there should only be one) then create stars based on its position
            // this looks ugly im aware but its the best way i know of creating a good amount of twinkles in quick succession all in different
            // positions
            if (this->timer == 15) { // creates stars
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x - TO_FIXED(64), titleLogo->position.y + TO_FIXED(8));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 30) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x + TO_FIXED(64), titleLogo->position.y - TO_FIXED(16));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 45) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x + TO_FIXED(32), titleLogo->position.y - TO_FIXED(64));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 60) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x - TO_FIXED(96), titleLogo->position.y - TO_FIXED(56));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 75) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x + TO_FIXED(50), titleLogo->position.y + TO_FIXED(80));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 90) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x - TO_FIXED(80), titleLogo->position.y + TO_FIXED(2));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 105) {
                Music::PlayTrack(Music::TRACK_STAGE); // need this here so its only called on one frame and played on this time
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x + TO_FIXED(8), titleLogo->position.y + TO_FIXED(60));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 120) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x + TO_FIXED(80), titleLogo->position.y - TO_FIXED(70));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 135) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x - TO_FIXED(16), titleLogo->position.y + TO_FIXED(70));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 150) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x + TO_FIXED(32), titleLogo->position.y - TO_FIXED(32));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 165) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x - TO_FIXED(64), titleLogo->position.y + TO_FIXED(70));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 180) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x + TO_FIXED(96), titleLogo->position.y + TO_FIXED(47));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 195) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x - TO_FIXED(46), titleLogo->position.y - TO_FIXED(40));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer == 210) {
                Twinkle *twinkle = GameObject::Create<Twinkle>(nullptr, titleLogo->position.x + TO_FIXED(76), titleLogo->position.y + TO_FIXED(16));
                twinkle->type    = Twinkle::TwinkleStar;
            }
            if (this->timer >= 225) {
                for (auto twinkle :
                     GameObject::GetEntities<Twinkle>(FOR_ALL_ENTITIES)) { // gets all of the twinkles currently in the stage to destroy them all
                    twinkle->Destroy();
                }
            }
        }
        if (this->timer >= 100) {
            if (titleLogo->type == TitleLogo::TITLELOGO_EMBLEM) { // if its emblem start increasing the alpha immediately and set the visible to true
                if (titleLogo->alpha < 255) {
                    titleLogo->alpha += 8;
                }
                titleLogo->active  = ACTIVE_NORMAL;
                titleLogo->visible = true;
            }
            else if (titleLogo->type
                     == TitleLogo::TITLELOGO_RIBBON) { // if its ribbon wait 50 extra frames and then start increasing alpha and make it visible
                if (this->timer >= 150) {
                    if (titleLogo->alpha < 255) {
                        titleLogo->alpha += 8;
                    }
                    titleLogo->active  = ACTIVE_NORMAL;
                    titleLogo->visible = true;
                }
            }
        }
    }
    if (this->timer >= 180) { // when sonic and tails start coming in
        for (auto titleSonic : GameObject::GetEntities<TitleSonic>(FOR_ALL_ENTITIES)) {
            if (titleSonic->type == TitleSonic::Sonic) {
                titleSonic->active  = ACTIVE_NORMAL;
                titleSonic->visible = true;
            }
            if (this->timer == 240) {
                if (titleSonic->type == TitleSonic::Tails) {
                    titleSonic->active  = ACTIVE_NORMAL;
                    titleSonic->visible = true;
                }
            }
        }
    }
    if (this->timer == 240) {
        this->state.Set(&TitleSetup::State_WhiteFlash);
    }
}

void TitleSetup::State_WhiteFlash()
{
    this->timer++;
    if (this->timer == 320) {
        for (auto titleLogo : GameObject::GetEntities<TitleLogo>(
                 FOR_ALL_ENTITIES)) // gets all title logos and sets them ALL to active and visible when the screen flashes white
        {
            titleLogo->active  = ACTIVE_NORMAL;
            titleLogo->visible = true;
            if (titleLogo->type == TitleLogo::TITLELOGO_RIBBON) {
                titleLogo->showRibbonCenter = true;
                titleLogo->mainAnimator.SetAnimation(TitleLogo::sVars->aniFrames, 2, true, 0);
            }
        }
        // changes the bg during the flash by changing draw orders
        SceneLayer::GetTileLayer(0)->drawGroup[0] = DRAWGROUP_COUNT;
        this->timer                               = 768; // high value so the screen flash is instant
        this->state.Set(&TitleSetup::State_ClearFlash);  // state that slowly fades away the white screen
        this->stateDraw.Set(&TitleSetup::Draw_Flash);
    }
}

void TitleSetup::State_ClearFlash()
{
    if (this->timer <= 0) {
        this->timer = 0;
        this->stateDraw.Set(nullptr);                       // set to nullptr when its done so the white screen is no longer drawing
        this->state.Set(&TitleSetup::State_SetupStartText); // sets up the press any button text alongside the "mania" animation
    }
    else {
        this->timer -= 16;
    }
}

void TitleSetup::State_SetupStartText()
{
    if (this->timer >= 20) {
        for (auto titleLogo : GameObject::GetEntities<TitleLogo>(FOR_ALL_ENTITIES)) {
            if (titleLogo->type == TitleLogo::TITLELOGO_GAMETITLE) {
                titleLogo->mainAnimator.Process();
            }
        }
    }

    if (++this->timer == 120) {
        for (auto titleLogo : GameObject::GetEntities<TitleLogo>(FOR_ALL_ENTITIES)) { // gets all titlelogos but only uses the press start type
            if (titleLogo->type == TitleLogo::TITLELOGO_PRESSSTART) {
                sceneInfo->entity = titleLogo;
                titleLogo->SetupPressStart(); // used to be for setting the sprite for whichever language the game is on, but now its always just set
                                              // on the english text
            }
        }

        this->timer = 0;
        this->state.Set(&TitleSetup::State_WaitForEnter);
    }
}

void TitleSetup::State_WaitForEnter()
{
    bool32 anyButton = controllerInfo->keyA.press || controllerInfo->keyB.press || controllerInfo->keyC.press || controllerInfo->keyX.press
                       || controllerInfo->keyY.press || controllerInfo->keyZ.press || controllerInfo->keyStart.press
                       || controllerInfo->keySelect.press;

    bool32 anyClick = (!touchInfo->count && this->touched) || Unknown_anyKeyPress;
    this->touched   = touchInfo->count > 0;

    if (this->timer == 15) {
        Twinkle *twinkle =
            GameObject::Create<Twinkle>(INT_TO_VOID(true), 0, 0); // position doesnt matter, shooting star is drawn relative to the screen
        // the type is only set AFTER the twinkle is created which is really annoying honestly, so instead on create im setting int_to_void true and
        // using that in the twinkle's create event
        twinkle->type = Twinkle::ShootingStar;
    }

    if (anyClick || anyButton) {
        sVars->sfxMenuAccept.Play(false, 255);
        this->timer = 0;

        const char *nextScene = "Menu"; // "nextScene" is used so it can change depending on whether you did a cheat or not, but no cheats are here so
                                        // its always just menu
        Stage::SetScene("Presentation", nextScene);

        int32 id = Input::GetFilteredInputDeviceID(false, false, 5); // gets the id of the whichever controller was used
        Input::ResetInputSlotAssignments();                          // resets assignments
        Input::AssignInputSlotToDevice(Input::CONT_P1, id);          // assigns p1 to whichever controller was used to pass the title screen

        Music::Stop();
        this->state.Set(&TitleSetup::State_FadeToMenu);
        this->stateDraw.Set(&TitleSetup::Draw_FadeBlack);
    }
    this->timer++;
    /*else if (++this->timer == 1024) { // timer until video is about to play
        this->timer     = 0;
        this->state.Set(&TitleSetup::State_FadeToVideo);
        this->stateDraw.Set(&TitleSetup::Draw_FadeBlack);
    }//*/
    // stop video from even attempting to play
    // chuli you NUMBSKULL this made the timer never count up and as a result the shooting star never started
}

void TitleSetup::State_FadeToMenu()
{
    if (this->timer >= 1248)
        Stage::LoadScene();
    else
        this->timer += 8;
}

void TitleSetup::State_FadeToVideo()
{
    if (this->timer >= 1248) { // timer for when the video actually starts
        Stage::LoadScene();
        Music::Stop();

        if (sVars->useAltIntroMusic) {
            RSDKTable->PlayStream("IntroTee.ogg", Music::sVars->channelID, 0, 0, false);
            RSDKTable->LoadVideo("Mania.ogv", 1.8, TitleSetup::VideoSkipCB);
            sVars->useAltIntroMusic = false;
        }
        else {
            RSDKTable->PlayStream("IntroHP.ogg", Music::sVars->channelID, 0, 0, false);
            RSDKTable->LoadVideo("Mania.ogv", 0, TitleSetup::VideoSkipCB);
            sVars->useAltIntroMusic = true;
        }
    }
    else {
        this->timer += 8;
    }
}

void TitleSetup::Draw_FadeBlack()
{
    Graphics::FillScreen(0x000000, this->timer, this->timer - 128,
                         this->timer - 256); // timer should be 0 at start and goes down, but once it reaches -300 it resets to 0 and the draw state
                                             // changes, making this rectangle fade away
}

void TitleSetup::Draw_Flash() { Graphics::FillScreen(0xF0F0F0, this->timer, this->timer - 128, this->timer - 256); }

#if RETRO_INCLUDE_EDITOR
void TitleSetup::EditorDraw() {}

void TitleSetup::EditorLoad() {}
#endif

void TitleSetup::Serialize() {}

} // namespace GameLogic