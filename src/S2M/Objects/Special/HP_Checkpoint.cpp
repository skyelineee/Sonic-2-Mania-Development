// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_Checkpoint Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_Checkpoint.hpp"
#include "HP_Setup.hpp"
#include "HP_Halfpipe.hpp"
#include "HP_Player.hpp"
#include "HP_Message.hpp"
#include "HP_SpecialClear.hpp"
#include "HP_Collectable.hpp"
#include "Global/Zone.hpp"
#include "Global/Music.hpp"
#include "Global/HUD.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_Checkpoint);

void HP_Checkpoint::Update() { this->state.Run(this); }
void HP_Checkpoint::LateUpdate() {}
void HP_Checkpoint::StaticUpdate() {}
void HP_Checkpoint::Draw() { this->stateDraw.Run(this); }

void HP_Checkpoint::Create(void *data)
{
    this->active  = ACTIVE_NEVER;
    this->visible = true;

    this->iconAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->emblemAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    this->ringAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);

    this->state.Set(&HP_Checkpoint::State_Init);
    this->stateDraw.Set(nullptr);
}

void HP_Checkpoint::StageLoad()
{
    sVars->aniFrames.Load("Special/Checkpoint.bin", SCOPE_STAGE);

    sVars->sfxStarPost.Get("Global/StarPost.wav");
    sVars->sfxFail.Get("Stage/Fail.wav");

    if (globals->gameMode == MODE_COMPETITION) {
        for (auto collectable : GameObject::GetEntities<HP_Collectable>(FOR_ALL_ENTITIES)) {
            if (collectable->type == HP_Collectable::Emerald)
                collectable->Reset(sVars->classID, 0);
        }

        for (auto message : GameObject::GetEntities<HP_Message>(FOR_ALL_ENTITIES)) {
            if (message->type == HP_Message::RingReminder)
                message->Destroy();
        }
    }
}

// States
void HP_Checkpoint::State_Init()
{
    SET_CURRENT_STATE();

    if (this->timer++ >= 16) {
        this->localPos.z = ((this->localPos.z >> 22) << 22) + 0x300000;

        // Destroy any existing text messages
        GameObject::Reset(SLOT_HP_MESSAGE, 0, 0);
        this->timer = 0;
        this->state.Set(&HP_Checkpoint::State_AwaitPlayer);
    }
}
void HP_Checkpoint::State_AwaitPlayer()
{
    SET_CURRENT_STATE();

    this->ringAnimator.Process();
    DrawRings();

    HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
    HP_Player *player2 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER2);

    for (auto player : GameObject::GetEntities<HP_Player>(FOR_ACTIVE_ENTITIES)) {
        if (player->localPos.z > this->localPos.z - 0x20000 && player->localPos.z < this->localPos.z + 0x20000) {
            this->thumbPos  = 80;
            this->drawGroup = Zone::sVars->hudDrawGroup;

            if (globals->gameMode == MODE_COMPETITION) {
                HP_Message *message = GameObject::Create<HP_Message>(0, 0, 0);

                if (player1->rings == player2->rings) {
                    this->emblemAnimator.frameID = -1;

                    message->position.y = TO_FIXED(68);
                    message->SetMessage(&HP_Message::State_SingleMessage, 0, 90, true, "TIE!", nullptr);
                }
                else {
                    message->position.y = TO_FIXED(116);

                    if (player1->rings > player2->rings) {
                        player1->vsCheckpointID++;
                        this->emblemAnimator.frameID = HUD::CharacterIndexFromID(player1->characterID);

                        // if (!vs.playerID)
                        //     message->SetMessage(&HP_Message::State_SingleMessage, 0, 90, true, "PLAYER ONE WINS!", nullptr);
                        // else
                        //     message->SetMessage(&HP_Message::State_SingleMessage, 0, 90, true, "PLAYER TWO WINS!", nullptr);
                    }
                    else {
                        //  if (ReceiveValue == -1)
                        //  	player2->vsCheckpointID++;
                        player2->vsCheckpointID++;

                        this->emblemAnimator.frameID = HUD::CharacterIndexFromID(player2->characterID);
                        // if (!vs.playerID)
                        //     message->SetMessage(&HP_Message::State_SingleMessage, 0, 90, true, "PLAYER TWO WINS!", nullptr);
                        // else
                        //     message->SetMessage(&HP_Message::State_SingleMessage, 0, 90, true, "PLAYER ONE WINS!", nullptr);
                    }
                }

                player1->vsCheckpointRings[HP_Setup::sVars->checkpointID] = player1->rings;
                player2->vsCheckpointRings[HP_Setup::sVars->checkpointID] = player2->rings;

                sVars->sfxStarPost.Play();
                this->iconAnimator.frameID = 0;

                this->state.Set(&HP_Checkpoint::State_ShowResults);
                this->stateDraw.Set(&HP_Checkpoint::Draw_Results);
                break;
            }
            else {
                this->emblemAnimator.frameID = HUD::CharacterIndexFromID(GET_CHARACTER_ID(1));

                HP_Message *message = GameObject::Get<HP_Message>(SLOT_HP_MESSAGE);
                message->Reset(HP_Message::sVars->classID, 0);

                int32 totalRings = player1->rings + (GET_CHARACTER_ID(2) ? player2->rings : 0);
                if (totalRings >= HP_Setup::sVars->ringCounts[HP_Setup::sVars->checkpointID]) {
                    this->failed = false;

                    message->position.y = TO_FIXED(116);
                    message->SetMessage(&HP_Message::State_SingleMessage, 0, 120, false, "COOL!", nullptr);

                    sVars->sfxStarPost.Play();
                }
                else {
                    this->failed           = true;

                    message->position.y = TO_FIXED(116);
                    message->SetMessage(&HP_Message::State_NotEnough, 0, 120, false, "NOT ENOUGH", "RINGS ... ", nullptr);

                    sVars->sfxFail.Play();
                }

                this->iconAnimator.frameID = this->failed;
                if (GET_CHARACTER_ID(1) == ID_KNUCKLES)
                    this->iconAnimator.frameID += 2;

                this->state.Set(&HP_Checkpoint::State_ShowResults);
                this->stateDraw.Set(&HP_Checkpoint::Draw_Results);
            }
        }
    }
}
void HP_Checkpoint::State_ShowResults()
{
    SET_CURRENT_STATE();

    this->thumbPos = 80 + (Math::Sin512(this->angle) >> 7);
    this->angle    = (this->angle + 24) & 0x1FF;

    if (this->timer < 120) {
        if (this->timer < 20)
            DrawRings();
        this->timer++;
    }
    else {
        HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
        HP_Player *player2 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER2);

        if (globals->gameMode == MODE_COMPETITION) {
            if (++HP_Setup::sVars->checkpointID == 3) {
                this->state.Set(&HP_Checkpoint::State_FadeOut);
                this->stateDraw.Set(&HP_Checkpoint::Draw_Fade);
                player1->stateInput.Set(nullptr);
                player2->stateInput.Set(nullptr);
            }
            else {
                this->state.Set(&HP_Checkpoint::State_ShowNewRingTarget);
            }
        }
        else {
            this->timer = 0;
            this->state.Set(&HP_Checkpoint::State_ExitMessage);
            this->scale.x = 0x200;
            this->scale.y = 0x200;
            this->drawFX |= FX_SCALE;
        }
    }
}
void HP_Checkpoint::State_ExitMessage()
{
    SET_CURRENT_STATE();

    if (this->scale.x > 0) {
        this->scale.x -= 16;
    }
    else {
        if (!this->failed) {
            this->state.Set(&HP_Checkpoint::State_ShowNewRingTarget);
        }
        else {
            this->Destroy();
            GameObject::Reset(SLOT_HP_SPECIALCLEAR, HP_SpecialClear::sVars->classID, 0);
        }
    }
    this->scale.y = this->scale.x;
}
void HP_Checkpoint::State_ShowNewRingTarget()
{
    SET_CURRENT_STATE();

    if (this->timer++ >= 10) {
        HP_Message *message = GameObject::Get<HP_Message>(SLOT_HP_MESSAGE);
        message->Reset(HP_Message::sVars->classID, 0);

        if (globals->gameMode == MODE_COMPETITION) {
            message->position.y = TO_FIXED(116);
            message->SetMessage(&HP_Message::State_SingleMessage, 0, 90, false, "MOST RINGS WINS!", nullptr);
        }
        else {
            HP_Setup::sVars->checkpointID++;

            message->position.y = TO_FIXED(116);
            message->SetMessage(&HP_Message::State_SingleMessage, HP_Setup::sVars->ringCounts[HP_Setup::sVars->checkpointID], 90, false,
                                "GET % RINGS!", "GET $% RINGS!", "GET #$% RINGS!", nullptr);
            message->stateDraw.Set(&HP_Message::Draw_GetRings);

        }
        this->Destroy();
    }
}
void HP_Checkpoint::State_FadeOut()
{
    SET_CURRENT_STATE();

    if (this->timer < 320) {
        this->timer += 8;
    }
    else {
        this->timer = 0x100;
        Music::FadeOut(0.03125f); // 8/256
        this->state.Set(&HP_Checkpoint::State_GoToResults);
        this->stateDraw.Set(&HP_Checkpoint::Draw_Exit);
    }
}
void HP_Checkpoint::State_GoToResults()
{
    SET_CURRENT_STATE();

    if (this->timer > 0) {
        this->timer -= 8;
    }
    else {
        HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
        HP_Player *player2 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER2);

        // if (vs.playerID == 0) {
        //     vs.checkpoint1P = player1->vsCheckpointRings[0] | (player1->vsCheckpointRings[1] << 8) | (player1->vsCheckpointRings[2] << 16);
        //     vs.checkpoint2P = player2->vsCheckpointRings[0] | (player2->vsCheckpointRings[1] << 8) | (player2->vsCheckpointRings[2] << 16);
        // }
        // else {
        //     vs.checkpoint2P = player1->vsCheckpointRings[0] | (player1->vsCheckpointRings[1] << 8) | (player1->vsCheckpointRings[2] << 16);
        //     vs.checkpoint1P = player2->vsCheckpointRings[0] | (player2->vsCheckpointRings[1] << 8) | (player2->vsCheckpointRings[2] << 16);
        // }

        Music::Stop();
        Stage::SetScene("Presentation", "Menu");
        Stage::LoadScene();
    }
}

// Draw Stuff
void HP_Checkpoint::DrawRings()
{
    int32 index = HP_Halfpipe::sVars->checkpointFaceTable[(this->localPos.z >> 22) & 3];

    // Draw flashing rings
    for (int32 s = 0; s < 7; ++s) {
        Vector3 drawPos;
        drawPos.x = HP_Halfpipe::sVars->scene3D.vertexBuffer[index].x;
        drawPos.y = HP_Halfpipe::sVars->scene3D.vertexBuffer[index].y;
        drawPos.z = HP_Halfpipe::sVars->scene3D.vertexBuffer[index].z;
        index += 4;

        HP_Halfpipe::DrawTexture(drawPos.x, drawPos.y, drawPos.z, 0x800, 0x800, &this->ringAnimator, sVars->aniFrames, false);
    }
}

void HP_Checkpoint::Draw_Results()
{
    SET_CURRENT_STATE();

    if (this->emblemAnimator.frameID >= 0) {
        Vector2 drawPos;
        drawPos.x = TO_FIXED(screenInfo[sceneInfo->currentScreenID].center.x);
        drawPos.y = TO_FIXED(80);
        this->emblemAnimator.DrawSprite(&drawPos, true);

        drawPos.y = TO_FIXED(this->thumbPos);
        this->iconAnimator.DrawSprite(&drawPos, true);
    }
}

void HP_Checkpoint::Draw_Fade()
{
    SET_CURRENT_STATE();

    Draw_Results();

    Graphics::FillScreen(0xF8F8F8, this->timer, this->timer, this->timer);
}

void HP_Checkpoint::Draw_Exit()
{
    SET_CURRENT_STATE();

    uint8 c = CLAMP(this->timer, 0, 255);
    Graphics::FillScreen((c << 16) | (c << 8) | (c << 0), 0xFF, 0xFF, 0xFF);
}

#if RETRO_INCLUDE_EDITOR
void HP_Checkpoint::EditorDraw()
{
    this->iconAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
    this->iconAnimator.DrawSprite(nullptr, false);
}

void HP_Checkpoint::EditorLoad() { sVars->aniFrames.Load("Special/Checkpoint.bin", SCOPE_STAGE); }
#endif

#if RETRO_REV0U
void HP_Checkpoint::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(HP_Checkpoint);

    sVars->aniFrames.Init();
}
#endif

void HP_Checkpoint::Serialize()
{
    RSDK_EDITABLE_VAR(HP_Checkpoint, VAR_INT32, ringCountSonic);
    RSDK_EDITABLE_VAR(HP_Checkpoint, VAR_INT32, ringCountKnux);
    RSDK_EDITABLE_VAR(HP_Checkpoint, VAR_INT32, ringCount2P);
}

} // namespace GameLogic