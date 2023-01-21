// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Music Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Music.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Music);

void Music::Update() { this->state.Run(this); }
void Music::LateUpdate() {}
void Music::StaticUpdate() {}
void Music::Draw() {}

void Music::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        // if frozen, ACTIVE_ALWAYS, else ACTIVE_NORMAL
        this->active = ACTIVE_ALWAYS + ((sceneInfo->state & 3) != ENGINESTATE_FROZEN);

        if (this->trackFile.length) {
            if (this->trackID != TRACK_SOUNDTEST) {
                char trackName[0x20];
                this->trackFile.CStr(trackName);
                SetMusicTrack(this->trackID, trackName, this->trackLoop);

                if (this->playOnLoad)
                    this->state.Set(&Music::State_PlayOnLoad);
                else
                    this->Destroy();
            }
        }
    }
}

void Music::StageLoad()
{
    // Slot 0: stage music
    SetMusicTrack(TRACK_INVINCIBLE, "Invincible.ogg", 139263);
    SetMusicTrack(TRACK_SNEAKERS, "Sneakers.ogg", 120960);
    SetMusicTrack(TRACK_MINIBOSS, "BossMini.ogg", 276105);
    SetMusicTrack(TRACK_HBHBOSS, "BossHBH.ogg", 70560);
    SetMusicTrack(TRACK_EGGMAN1, "BossEggman1.ogg", 282240);
    SetMusicTrack(TRACK_EGGMAN2, "BossEggman2.ogg", 264600);
    SetMusicTrack(TRACK_ACTCLEAR, "ActClear.ogg", false);
    SetMusicTrack(TRACK_DROWNING, "Drowning.ogg", false);
    SetMusicTrack(TRACK_GAMEOVER, "GameOver.ogg", false);
    SetMusicTrack(TRACK_SUPER, "Super.ogg", 165375);
    SetMusicTrack(TRACK_HBHMISCHIEF, "HBHMischief.ogg", 381405);
    // Slot 12 (slot 11 in pre-plus): "no load"
    SetMusicTrack(TRACK_1UP, "1up.ogg", false);

    if (globals->suppressAutoMusic) {
        if (globals->restartMusicID > TRACK_STAGE)
            sVars->activeTrack = globals->restartMusicID;
    }
    else {
        sVars->activeTrack = TRACK_STAGE;
        sVars->prevTrack   = Music::TRACK_STAGE;
    }

    globals->restartMusicID = TRACK_STAGE;
    if (!sVars->disableNextTrack)
        sVars->nextTrack = TRACK_NONE;

    sVars->restartTrackID = -1;
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);
    sVars->musicSpeedUp.Set(nullptr);
    sVars->musicSlowDown.Set(nullptr);
    sVars->isSpedUp = false;
}

void Music::SetMusicTrack(uint8 track, const char *path, uint32 loopPoint)
{
    track &= 0xF;

    int32 charID = 0;
    for (; charID < 0x20; ++charID) {
        if (!path[charID])
            break;
        else
            sVars->trackNames[track][charID] = path[charID];
    }
    sVars->trackNames[track][charID] = 0;
    sVars->trackLoops[track]         = loopPoint;
}

void Music::SwapMusicTrack(uint8 track, const char *swapTrack, uint32 loopPoint, float ratio)
{
    track &= 0xF;

    SetMusicTrack(track, swapTrack, loopPoint);

    if (sVars->activeTrack == track) {
        uint32 startPos  = (int32)(channels[sVars->channelID].AudioPos() * ratio);
        sVars->channelID = channels[0].PlayStream(sVars->trackNames[sVars->activeTrack], startPos, sVars->trackLoops[sVars->activeTrack], true);
    }
}

void Music::Stop() { channels[sVars->channelID].Stop(); }
void Music::Pause() { channels[sVars->channelID].Pause(); }
void Music::Resume() { channels[sVars->channelID].Resume(); }

void Music::State_PlayOnLoad()
{
    SET_CURRENT_STATE();

    if (globals->suppressAutoMusic) {
        globals->suppressAutoMusic = false;

        switch (sVars->activeTrack) {
            case TRACK_INVINCIBLE:
            case TRACK_SNEAKERS:
            case TRACK_1UP: Music::PlayOnFade(this->trackID, 0.025f); break;
            case TRACK_SUPER: Music::PlayAutoMusicQueuedTrack(sVars->activeTrack); break;
            default: break;
        }
    }
    else {
        Music::PlayTrack(this->trackID);
    }

    this->Destroy();
}

void Music::PlayJingle(uint8 trackID)
{
    trackID &= 0xF;

    // remove any existing vers of this on the stack
    for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) {
        Music *music = GameObject::Get<Music>(slot);
        if (music->classID == sVars->classID && music->trackID == trackID) {
            music->Destroy();
        }
    }

    Music *entity = nullptr;
    // get the first empty stack slot
    for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) {
        entity = GameObject::Get<Music>(slot);
        if (entity->classID != sVars->classID) {
            entity->Reset(sVars->classID, nullptr);
            break;
        }
    }

    entity->trackID = trackID;
    entity->state.Set(&Music::State_Jingle);
    entity->trackPriority = PriorityNone;
    entity->fadeSpeed     = 0.08f;

    switch (trackID) {
        case TRACK_INVINCIBLE:
        case TRACK_SNEAKERS:
            entity->HandleMusicStack_Powerups();
            entity->trackPriority = PriorityPowerup;
            entity->restartTrack  = true;
            break;

        case TRACK_DROWNING:
            entity->timer         = 960;
            entity->trackPriority = PriorityDrown;
            entity->restartTrack  = true;
            break;

        case TRACK_SUPER:
            entity->trackPriority = PrioritySuper;
            entity->restartTrack  = true;
            break;

        case TRACK_1UP:
            entity->timer         = 300;
            entity->trackPriority = Priority1UP;
            break;

        default: break;
    }

    entity->GetNextTrackStartPos();

    for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) {
        Music *music = GameObject::Get<Music>(slot);
        if (music->classID == sVars->classID && music != entity) {
            if (music->trackPriority > entity->trackPriority) {
                entity->volume = 0.0;
                return;
            }
        }
    }

    Stop();
    sVars->activeTrack = trackID;
    sVars->channelID   = channels[0].PlayStream(sVars->trackNames[sVars->activeTrack], 0, sVars->trackLoops[sVars->activeTrack], true);

    if (globals->vapeMode)
        channels[sVars->channelID].SetAttributes(1.0, 0.0, 0.75);

    entity->volume = 1.0;
}

void Music::PlayTrack(uint8 trackID)
{
    trackID &= 0xF;

    if (trackID != TRACK_ACTCLEAR && Music::CheckMusicStack_Active()) {
        sVars->nextTrack = trackID;
    }
    else {
        if (trackID == TRACK_ACTCLEAR)
            Music::ClearMusicStack();

        GameObject::Get<Music>(SLOT_MUSIC)->Destroy();
        Stop();

        sVars->activeTrack   = trackID;
        sVars->trackStartPos = 0;
        sVars->channelID     = channels[0].PlayStream(sVars->trackNames[sVars->activeTrack], 0, sVars->trackLoops[sVars->activeTrack], true);

        if (globals->vapeMode)
            channels[sVars->channelID].SetAttributes(1.0, 0.0, 0.75);
    }
}

void Music::Play()
{
    this->trackFile.CStr(sVars->trackNames[0]);
    sVars->trackLoops[0] = this->trackLoop;

    GameObject::Get<Music>(SLOT_MUSIC)->Destroy();
    Stop();

    sVars->activeTrack   = 0;
    sVars->trackStartPos = 0;
    sVars->channelID     = channels[0].PlayStream(sVars->trackNames[0], 0, sVars->trackLoops[0], true);

    if (globals->vapeMode)
        channels[sVars->channelID].SetAttributes(1.0, 0.0, 0.75);
}

void Music::PlayAutoMusicQueuedTrack(uint8 trackID)
{
    trackID &= 0xF;
    sVars->nextTrack = TRACK_STAGE;

    // remove any existing vers of this on the stack
    for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) {
        Music *music = GameObject::Get<Music>(slot);
        if (music->classID == sVars->classID && music->trackID == trackID) {
            music->Destroy();
        }
    }

    Music *entity = nullptr;
    // get the first empty stack slot
    for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) {
        entity = GameObject::Get<Music>(slot);
        if (entity->classID != sVars->classID) {
            entity->Destroy();
            break;
        }
    }

    entity->trackID = trackID;
    entity->state.Set(&Music::State_Jingle);
    entity->trackPriority = PriorityNone;
    entity->volume        = 1.0;
    entity->fadeSpeed     = 0.08f;

    switch (trackID) {
        case TRACK_INVINCIBLE:
        case TRACK_SNEAKERS:
            entity->HandleMusicStack_Powerups();
            entity->trackPriority = PriorityPowerup;
            entity->restartTrack  = true;
            break;

        case TRACK_DROWNING:
            entity->timer         = 960;
            entity->trackPriority = PriorityDrown;
            entity->restartTrack  = true;
            break;

        case TRACK_SUPER:
            entity->trackPriority = PrioritySuper;
            entity->restartTrack  = true;
            break;

        case TRACK_1UP:
            entity->timer         = 224;
            entity->trackPriority = Priority1UP;
            break;

        default: break;
    }
}

void Music::HandleMusicStack_Powerups()
{
    for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) {
        Music *music = GameObject::Get<Music>(slot);
        if (music->classID == sVars->classID && music->trackPriority == PriorityPowerup && music->trackID != this->trackID && music != this)
            music->trackPriority = PriorityAny;
    }
}

bool32 Music::CheckMusicStack_Active()
{
    bool32 active = false;

    for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) {
        Music *music = GameObject::Get<Music>(slot);
        if (music->classID == sVars->classID && music->trackPriority > PriorityNone)
            active = true;
    }

    return active;
}

bool32 Music::IsPlaying() { return RSDKTable->ChannelActive(Music::sVars->channelID); }

void Music::GetNextTrackStartPos()
{
    int32 stackCount = 0;

    for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) {
        Music *music = GameObject::Get<Music>(slot);
        if (music->classID == sVars->classID && this != music) {
            if (music->trackID == sVars->activeTrack) {
                this->trackStartPos = channels[sVars->channelID].AudioPos();
                this->volume        = 0.0;
            }
            stackCount++;
        }
    }

    if (!stackCount) {
        if (sVars->activeTrack > TRACK_NONE) {
            if (sVars->nextTrack == TRACK_NONE)
                sVars->nextTrack = sVars->activeTrack;

            sVars->trackStartPos = channels[sVars->channelID].AudioPos();
        }
    }
}

void Music::JingleFadeOut(uint8 trackID, bool32 transitionFade)
{
    trackID &= 0xF;

    for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) {
        Music *music = GameObject::Get<Music>(slot);
        if (music->classID == sVars->classID && music->trackID == trackID) {
            if (transitionFade) {
                music->state.Set(&Music::State_JingleFade);
                music->fadeSpeed = 0.05f;
            }
            else {
                music->timer = 1;
            }
        }
    }
}

void Music::FinishJingle()
{
    Music *music = GameObject::Get<Music>(SLOT_MUSIC);
    if (music->classID != sVars->classID || !music->state.Matches(&Music::State_PlayOnFade)) {
        music->Destroy();

        // remove all of these buggers that have higher priority and thus wont be played
        for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) {
            Music *stack = GameObject::Get<Music>(slot);
            if (stack->classID == sVars->classID && stack->trackPriority > this->trackPriority) {
                this->Destroy();
                return;
            }
        }

        bool32 restartTrack = this->restartTrack;
        this->Destroy();
        int32 priority = 0;

        Music *trackPtr = nullptr;
        // the next track to be played will be the track with the highest priority on the stack (may be none)
        for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) {
            Music *stack = GameObject::Get<Music>(slot);
            if (stack->classID == sVars->classID && stack->trackPriority > priority) {
                trackPtr = stack;
                priority = stack->trackPriority;
            }
        }

        Stop();
        if (trackPtr) { // another track is on the music stack still
            if (trackPtr->trackID == sVars->activeTrack) {
                trackPtr->trackStartPos = 0;
            }
            else {
                sVars->activeTrack = trackPtr->trackID;
                if (restartTrack)
                    trackPtr->trackStartPos = 0;
                sVars->channelID = channels[sVars->channelID].PlayStream(sVars->trackNames[sVars->activeTrack], trackPtr->trackStartPos,
                                                                         sVars->trackLoops[sVars->activeTrack], true);
                if (trackPtr->trackStartPos) {
                    channels[sVars->channelID].SetAttributes(1.0, 0.0, globals->vapeMode ? 0.75f : 1.0f);
                }
                else if (globals->vapeMode) {
                    channels[sVars->channelID].SetAttributes(1.0, 0.0, 0.75f);
                }
            }
        }
        else if (sVars->nextTrack > TRACK_NONE) { // next track is queued
            sVars->activeTrack = sVars->nextTrack;
            sVars->nextTrack   = TRACK_NONE;

            if (restartTrack)
                sVars->trackStartPos = 0;

            sVars->channelID = channels[sVars->channelID].PlayStream(sVars->trackNames[sVars->activeTrack], sVars->trackStartPos,
                                                                     sVars->trackLoops[sVars->activeTrack], true);

            if (sVars->trackStartPos) {
                channels[sVars->channelID].SetAttributes(1.0, 0.0, globals->vapeMode ? 0.75f : 1.0f);
                music = GameObject::Get<Music>(SLOT_MUSIC);
                music->Reset(sVars->classID, nullptr);
                music->state.Set(&Music::State_FadeTrackIn);
                music->volume    = 0.0;
                music->fadeSpeed = 0.08f;
            }
            else if (globals->vapeMode) {
                channels[sVars->channelID].SetAttributes(1.0, 0.0, 0.75f);
            }
        }
    }
    else {
        this->Destroy();
    }
}

void Music::ClearMusicStack()
{
    for (int32 slot = SLOT_MUSICSTACK_START; slot < SLOT_MUSICSTACK_END; ++slot) GameObject::Get<Music>(slot)->Destroy();
}

void Music::PlayOnFade(uint8 trackID, float fadeSpeed)
{
    trackID &= 0xF;

    Music *music = nullptr;
    switch (trackID) {
        case TRACK_SUPER: Music::ClearMusicStack(); break;

        case TRACK_ACTCLEAR:
            Music::ClearMusicStack();
            // [Fallthrough]

        case TRACK_MINIBOSS:
        case TRACK_HBHBOSS:
        case TRACK_EGGMAN1:
        case TRACK_EGGMAN2:
        case TRACK_HBHMISCHIEF:
            music            = GameObject::Get<Music>(SLOT_MUSIC);
            sVars->nextTrack = trackID;
            break;

        default:
            if (Music::CheckMusicStack_Active()) {
                sVars->nextTrack = trackID;
            }
            else {
                music            = GameObject::Get<Music>(SLOT_MUSIC);
                sVars->nextTrack = trackID;
            }
            break;
    }

    if (music && (music->classID != sVars->classID || !music->state.Matches(&Music::State_PlayOnFade))) {
        music->Reset(sVars->classID, nullptr);
        music->state.Set(&Music::State_PlayOnFade);
        music->volume    = 1.0;
        music->fadeSpeed = fadeSpeed;
    }
}

void Music::FadeOut(float fadeSpeed)
{
    if (sVars->activeTrack != TRACK_DROWNING) {
        Music *music = GameObject::Get<Music>(SLOT_MUSIC);
        music->Reset(sVars->classID, nullptr);
        music->state.Set(&Music::State_StopOnFade);
        music->volume    = 1.0;
        music->fadeSpeed = fadeSpeed;
    }
}

void Music::State_Jingle()
{
    SET_CURRENT_STATE();

    if (sVars->activeTrack == this->trackID && channels[sVars->channelID].AudioPos() > this->trackStartPos) {
        this->trackStartPos = 0;
        if (this->volume < 1.0) {
            this->volume += this->fadeSpeed;
            channels[sVars->channelID].SetAttributes(this->volume, 0.0, 1.0);
            if (this->volume >= 1.0)
                this->volume = 1.0;
        }
    }

    if (this->timer > 0) {
        if (!--this->timer)
            this->FinishJingle();
    }
}
void Music::State_JingleFade()
{
    SET_CURRENT_STATE();

    if (this->volume > -0.5) {
        this->volume -= this->fadeSpeed;

        if (sVars->activeTrack == this->trackID)
            channels[sVars->channelID].SetAttributes(this->volume, 0.0, 1.0);

        if (this->volume <= -0.5)
            this->FinishJingle();
    }
}

void Music::State_FadeTrackIn()
{
    SET_CURRENT_STATE();

    if (channels[sVars->channelID].AudioPos() > sVars->trackStartPos) {
        sVars->trackStartPos = 0;
        this->volume += this->fadeSpeed;

        channels[sVars->channelID].SetAttributes(this->volume, 0.0, 1.0);
        if (this->volume >= 1.0) {
            this->volume = 1.0;
            this->Destroy();
        }
    }
}
void Music::State_StopOnFade()
{
    SET_CURRENT_STATE();

    this->volume -= this->fadeSpeed;
    channels[sVars->channelID].SetAttributes(this->volume, 0.0, 1.0);

    if (this->volume < -0.5) {
        Stop();
        this->Destroy();
    }
}

void Music::State_PlayOnFade()
{
    SET_CURRENT_STATE();

    this->volume -= this->fadeSpeed;
    channels[sVars->channelID].SetAttributes(this->volume, 0.0, 1.0);

    if (this->volume < -0.5) {
        Stop();

        sVars->activeTrack   = sVars->nextTrack;
        sVars->trackStartPos = 0;
        sVars->channelID =
            channels[sVars->channelID].PlayStream(sVars->trackNames[sVars->activeTrack], 0, sVars->trackLoops[sVars->activeTrack], true);

        if (globals->vapeMode)
            channels[sVars->channelID].SetAttributes(1.0, 0.0, 0.75);

        sVars->nextTrack = TRACK_NONE;
        this->Destroy();
    }
}

#if RETRO_INCLUDE_EDITOR
void Music::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, 0, true, 1);
    this->animator.DrawSprite(nullptr, false);
}

void Music::EditorLoad() { sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void Music::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Music);

    sVars->aniFrames.Init();
}
#endif

void Music::Serialize()
{
    RSDK_EDITABLE_VAR(Music, VAR_STRING, trackFile);
    RSDK_EDITABLE_VAR(Music, VAR_STRING, soundTestTitle);
    RSDK_EDITABLE_VAR(Music, VAR_ENUM, trackID);
    RSDK_EDITABLE_VAR(Music, VAR_UINT32, trackLoop);
    RSDK_EDITABLE_VAR(Music, VAR_BOOL, playOnLoad);
}

} // namespace GameLogic