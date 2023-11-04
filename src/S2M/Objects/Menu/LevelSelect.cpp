// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: LevelSelect Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "LevelSelect.hpp"
#include "UIText.hpp"
#include "UIPicture.hpp"
#include "Global/HUD.hpp"
#include "Global/Music.hpp"
#include "Helpers/RPCHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(LevelSelect);

void LevelSelect::Update()
{
    this->state.Run(this);

    screenInfo->position.x = 0x100 - screenInfo->center.x;
}

void LevelSelect::LateUpdate() {}

void LevelSelect::StaticUpdate()
{
    if (--sVars->bgAniDuration <= 0) {
        ++sVars->bgAniFrame;

        sVars->bgAniFrame &= 3;
        sVars->bgAniDuration = sVars->bgAniDurationTable[sVars->bgAniFrame];

        TileLayer *background = RSDKTable->GetTileLayer(0);
        background->scrollPos = (background->scrollPos + 0x1000000) & 0x7FF0000;
    }
}

void LevelSelect::Draw() { this->stateDraw.Run(this); }

void LevelSelect::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_ALWAYS;
        this->visible   = true;
        this->drawGroup = 12;
        this->state.Set(&LevelSelect::State_Init);
        this->stateDraw.Set(&LevelSelect::Draw_Fade);
        this->timer     = 640;
    }
}

void LevelSelect::StageLoad()
{
    sVars->sfxFail.Get("Stage/Fail.wav");
    sVars->sfxRing.Get("Global/Ring.wav");
    sVars->sfxEmerald.Get("Special/Emerald.wav");
    sVars->sfxContinue.Get("Special/Continue.wav");
    sVars->sfxMedalGot.Get("Special/MedalCaught.wav");

    GameObject::Reset(0, sVars->classID, nullptr);

    UIPicture::sVars->aniFrames.Load("LSelect/Icons.bin", SCOPE_STAGE);
    UIText::sVars->aniFrames.Load("LSelect/Text.bin", SCOPE_STAGE);

    SetPresence("", "In Level Select", "levelselect", "Level Select", "", "");

    globals->medalMods |= MEDAL_DEBUGMODE;
}

void LevelSelect::Draw_Fade()
{
    Graphics::FillScreen(0x000000, this->timer, this->timer - 128, this->timer - 256);
}

void LevelSelect::State_Init()
{
    int32 labelPos[32];
    int32 lastY = 0;
    for (auto labelPosPtrL : GameObject::GetEntities<UIText>(FOR_ALL_ENTITIES))
    {
        if (labelPosPtrL->position.x < 0x1000000 && labelPosPtrL->position.y > lastY) {
            lastY                        = labelPosPtrL->position.y;
            labelPos[this->labelCount++] = labelPosPtrL->position.y;
        }
    }

    for (auto labelL : GameObject::GetEntities<UIText>(FOR_ALL_ENTITIES))
    {
        if (labelL->position.x < 0x1000000) {
            for (int32 i = 0; i < this->labelCount; ++i) {
                if (labelL->position.y == labelPos[i]) {
                    switch (labelL->align) {
                        case UIText::UITEXT_ALIGN_LEFT: this->zoneNameLabels[i] = labelL; break;

                        default:
                        case UIText::UITEXT_ALIGN_CENTER: break;

                        case UIText::UITEXT_ALIGN_RIGHT: this->stageIDLabels[i] = labelL; break;
                    }
                }
            }
        }
    }

    if (this->labelCount > 0)
        memset(labelPos, 0, sizeof(int32) * this->labelCount);

    lastY = 0;

    for (auto labelPosPtrR : GameObject::GetEntities<UIText>(FOR_ALL_ENTITIES))
    {
        if (labelPosPtrR->position.x > 0x1000000 && labelPosPtrR->position.y > lastY) {
            lastY                        = labelPosPtrR->position.y;
            labelPos[this->labelCount++] = labelPosPtrR->position.y;
        }
    }

    for (auto labelR : GameObject::GetEntities<UIText>(FOR_ALL_ENTITIES))
    {
        if (labelR->position.x > 0x1000000 && this->labelCount > 0) {
            for (int32 i = 0; i < this->labelCount; ++i) {
                if (labelR->position.y == labelPos[i]) {
                    switch (labelR->align) {
                        case UIText::UITEXT_ALIGN_LEFT: this->zoneNameLabels[i] = labelR; break;

                        default:
                        case UIText::UITEXT_ALIGN_CENTER: break;

                        case UIText::UITEXT_ALIGN_RIGHT: this->stageIDLabels[i] = labelR; break;
                    }
                }
            }
        }
    }

    for (int32 i = 0; i < this->labelCount; ++i) {
        if (!this->zoneNameLabels[i]) {
            for (int32 v = i; v >= 0; --v) {
                if (this->zoneNameLabels[v]) {
                    this->zoneNameLabels[i] = this->zoneNameLabels[v];
                    break;
                }
            }
        }
    }

    LevelSelect::SetLabelHighlighted(true);

    this->leaderCharacterID   = HUD::CharacterIndexFromID(GET_CHARACTER_ID(1)) + 1;
    this->sidekickCharacterID = HUD::CharacterIndexFromID(GET_CHARACTER_ID(2)) + 1;

    for (auto picture : GameObject::GetEntities<UIPicture>(FOR_ALL_ENTITIES))
    {
        if (picture->listID == 1) {
            this->zoneIcon = picture;

            // Bug Details(?):
            // frameID is equal to... playerID...?
            // this feels like a slight oversight, though idk what it is meant to be
            // picture->animator.frameID = this->leaderCharacterID;
            // changed this to instead be whatever data1 is on the selected uitext at init, stops the wrong frame from being used when the stage is fading in
            picture->animator.SetAnimation(UIPicture::sVars->aniFrames, 1, true, this->stageIDLabels[this->labelID]->data1);
        }
        else if (picture->listID == 3) {
            if (picture->frameID)
                this->player2Icon = picture;
            else
                this->player1Icon = picture;
        }
    }

    LevelSelect::ManagePlayerIcon();

    this->state.Set(&LevelSelect::State_FadeIn);
}

void LevelSelect::State_FadeIn()
{
    if (this->timer <= 0) {
        this->timer     = 0;
        this->state.Set(&LevelSelect::State_Navigate);
        this->stateDraw.Set(nullptr);
    }
    else {
        this->timer -= 16;
    }
}

void LevelSelect::State_Navigate()
{
    bool32 confirmPress = API::GetConfirmButtonFlip() ? controllerInfo->keyB.press : controllerInfo->keyA.press;

    if (controllerInfo->keyUp.down || analogStickInfoL->keyUp.down) {
        this->timer = (this->timer + 1) & 0xF;

        if (this->timer == 1) {
            LevelSelect::SetLabelHighlighted(false);
            if (--this->labelID == 28)
                --this->labelID;

            if (this->labelID < 0)
                this->labelID = this->labelCount - 1;

            LevelSelect::SetLabelHighlighted(true);
        }
    }
    else if (controllerInfo->keyDown.down || analogStickInfoL->keyDown.down) {
        this->timer = (this->timer + 1) & 0xF;
        if (this->timer == 1) {
            LevelSelect::SetLabelHighlighted(false);

            if (++this->labelID == 28)
                ++this->labelID;

            if (this->labelID == this->labelCount)
                this->labelID = 0;

            LevelSelect::SetLabelHighlighted(true);
        }
    }
    else if (analogStickInfoL->keyLeft.press || controllerInfo->keyLeft.press || controllerInfo->keyRight.press || analogStickInfoL->keyRight.press) {
        this->timer = 0;

        LevelSelect::HandleColumnChange();
    }
    else if (confirmPress || controllerInfo->keyStart.press) {
        LevelSelect::HandleNewStagePos();
    }
    else {
        this->timer = 0;
    }

    if (controllerInfo->keyX.press) {
        ++this->leaderCharacterID;
        LevelSelect::ManagePlayerIcon();
    }

    if (controllerInfo->keyY.press) {
        ++this->sidekickCharacterID;
        LevelSelect::ManagePlayerIcon();
    }

    UIPicture *zoneIcon = this->zoneIcon;
    zoneIcon->animator.SetAnimation(UIPicture::sVars->aniFrames, 1, true, this->stageIDLabels[this->labelID]->data1);
}

void LevelSelect::State_FadeOut()
{
    if (this->timer >= 1024)
        Stage::LoadScene();
    else
        this->timer += 16;
}

void LevelSelect::ManagePlayerIcon()
{
    UIPicture *player1 = this->player1Icon;
    UIPicture *player2 = this->player2Icon;

    switch (this->leaderCharacterID) {
        case LSELECT_PLAYER_SONIC:
        case LSELECT_PLAYER_TAILS:
            // Bug Details(?):
            if (this->sidekickCharacterID == LSELECT_PLAYER_TAILS) // if leader is sonic or tails, and the sidekick is tails... change to knux...?
                this->leaderCharacterID = LSELECT_PLAYER_SONIC;
            // playerID 3 may have been meant to be "S&T" before it was rearranged?
            // v4 support this, with the player ids being: Sonic, Tails, Knux, Sonic & Tails

            player1->animator.frameID = this->leaderCharacterID;
            break;

        case LSELECT_PLAYER_KNUCKLES: player1->animator.frameID = this->leaderCharacterID; break;

        default:
            this->leaderCharacterID   = LSELECT_PLAYER_SONIC;
            player1->animator.frameID = LSELECT_PLAYER_SONIC;
            break;
    }

    switch (this->sidekickCharacterID) {
        case LSELECT_PLAYER_TAILS:
            player2->animator.frameID = this->sidekickCharacterID;

            // if leader is sonic & sidekick is tails, show ST icon. otherwise remove sidekick
            if (this->leaderCharacterID != LSELECT_PLAYER_SONIC) {
                this->sidekickCharacterID = LSELECT_PLAYER_NONE;
                player2->animator.frameID = LSELECT_PLAYER_NONE;
            }
            break;

        default:
            this->sidekickCharacterID = LSELECT_PLAYER_NONE;
            player2->animator.frameID = LSELECT_PLAYER_NONE;
            break;

        // if P2 is sonic, no he's not thats tails actually
        case LSELECT_PLAYER_SONIC:
            this->sidekickCharacterID = LSELECT_PLAYER_TAILS;
            player2->animator.frameID = LSELECT_PLAYER_TAILS;

            // if leader is sonic & sidekick is tails, show ST icon. otherwise remove sidekick
            if (this->leaderCharacterID != LSELECT_PLAYER_SONIC) {
                this->sidekickCharacterID = LSELECT_PLAYER_NONE;
                player2->animator.frameID = LSELECT_PLAYER_NONE;
            }
            break;
    }
}

void LevelSelect::SetLabelHighlighted(bool32 highlight)
{
    UIText *zoneName = this->zoneNameLabels[this->labelID];
    if (zoneName)
        zoneName->highlighted = highlight;

    UIText *stageID = this->stageIDLabels[this->labelID];
    if (stageID)
        stageID->highlighted = highlight;
}

void LevelSelect::HandleColumnChange()
{
    UIText *curLabel = this->stageIDLabels[this->labelID];
    if (!curLabel)
        curLabel = this->zoneNameLabels[this->labelID];

    int32 distance         = 0x1000000;
    UIText *labelPtr = NULL;
    if (curLabel->position.x < 0x1000000) {
        for (auto label : GameObject::GetEntities<UIText>(FOR_ACTIVE_ENTITIES))
        {
            if (label->position.x > 0x1000000) {
                int32 pos = abs(label->position.y - curLabel->position.y);
                if (pos < distance) {
                    distance = pos;
                    labelPtr = label;
                }
            }
        }
    }
    else {
        for (auto label : GameObject::GetEntities<UIText>(FOR_ACTIVE_ENTITIES))
        {
            if (label->position.x < 0x1000000) {
                int32 dist = abs(label->position.y - curLabel->position.y);
                if (dist < distance) {
                    distance = dist;
                    labelPtr = label;
                }
            }
        }
    }

    int32 labelID = this->labelID;
    for (int32 i = 0; i < this->labelCount; ++i) {
        if (this->stageIDLabels[i] == labelPtr || this->zoneNameLabels[i] == labelPtr) {
            labelID = i;
            break;
        }
    }

    if (this->labelID != labelID) {
        LevelSelect::SetLabelHighlighted(false);

        this->labelID = labelID;
        LevelSelect::SetLabelHighlighted(true);
    }
}

void LevelSelect::HandleNewStagePos()
{
    UIText *curLabel = this->stageIDLabels[this->labelID];
    if (!curLabel)
        curLabel = this->zoneNameLabels[this->labelID];

    if (curLabel->selectable) {
        char buffer[32];
        curLabel->tag.CStr(buffer);
        Stage::SetScene(buffer, "");
        sceneInfo->listPos += curLabel->data0;

        int32 leaderID = 0;
        if (this->leaderCharacterID > 0)
            leaderID = 1 << (this->leaderCharacterID - 1);

        int32 sidekickID = 0;
        if (this->sidekickCharacterID > 0)
            sidekickID = 1 << (this->sidekickCharacterID - 1);

        globals->playerID = leaderID | (sidekickID << 8);

        this->timer     = 0;
        this->state.Set(&LevelSelect::State_FadeOut);
        this->stateDraw.Set(&LevelSelect::Draw_Fade);
        Music::FadeOut(0.1);
    }
    else {
        sVars->sfxFail.Play(false, 255);
    }
}

#if RETRO_REV0U
void LevelSelect::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(LevelSelect);

    int32 bgAniDurationTable[] = { 240, 3, 3, 3 };

    memcpy(sVars->bgAniDurationTable, bgAniDurationTable, sizeof(bgAniDurationTable));

    sVars->bgAniDuration = 240;
}
#endif

#if RETRO_INCLUDE_EDITOR
void LevelSelect::EditorDraw() {}

void LevelSelect::EditorLoad()
{
    if (UIPicture::sVars)
        UIPicture::sVars->aniFrames.Load("LSelect/Icons.bin", SCOPE_STAGE);

    if (UIText::sVars)
        UIText::sVars->aniFrames.Load("LSelect/Text.bin", SCOPE_STAGE);
}
#endif

void LevelSelect::Serialize() {}

} // namespace GameLogic