// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: CutsceneSeq Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "CutsceneSeq.hpp"
#include "Global/Zone.hpp"
#include "Global/Music.hpp"

#include "DrawHelpers.hpp"
#include "LogHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(CutsceneSeq);

void CutsceneSeq::Update() { CheckSkip(this->skipType, this->skipCallback); }
void CutsceneSeq::LateUpdate()
{
    this->currentState = this->cutsceneStates[this->stateID];
    if (this->currentState) {
        sceneInfo->entity    = this->activeEntity;
        bool32 finishedState = this->currentState(this);
        sceneInfo->entity    = (void *)this;

        ++this->timer;
        if (finishedState) {
            LogHelpers::Print("State completed");
            NewState(this->stateID + 1);
        }

        if (!this->cutsceneStates[this->stateID]) {
            LogHelpers::Print("Sequence completed");
            this->Destroy();
        }
    }

    this->visible = false;
    for (int32 p = 0; p < 8; ++p) {
        if (this->points[p].x && this->points[p].y)
            this->visible = true;
    }

    if (this->fadeWhite > 0 || this->fadeBlack > 0)
        this->visible = true;
}
void CutsceneSeq::StaticUpdate() {}
void CutsceneSeq::Draw()
{
    color colors[8];
    colors[0] = 0x00FF00;
    colors[1] = 0xFF0000;
    colors[2] = 0x0000FF;
    colors[3] = 0xFF00FF;
    colors[4] = 0xFFFF00;
    colors[5] = 0x00FFFF;
    colors[6] = 0x9933FF;
    colors[7] = 0xFF9900;

    for (int32 p = 0; p < 8; ++p) {
        Vector2 *point = &this->points[p];
        if (point->x || point->y) {
            LogHelpers::PrintVector2("Draw poi ", *point);
            DrawHelpers::DrawCross(point->x, point->y, 0x200000, 0x200000, colors[p]);
        }
    }

    if (this->fadeWhite > 0)
        Graphics::FillScreen(0xFFF0F0, this->fadeWhite, this->fadeWhite - 256, this->fadeWhite - 256);

    if (this->fadeBlack > 0)
        Graphics::FillScreen(0x000000, this->fadeBlack, this->fadeBlack - 128, this->fadeBlack - 256);
}

void CutsceneSeq::Create(void *data)
{
    this->active    = ACTIVE_NORMAL;
    this->visible   = false;
    this->fadeWhite = 0;
    this->fadeBlack = 0;
    this->drawGroup = Zone::sVars->hudDrawGroup + 1;

    CheckSkip(this->skipType, this->skipCallback);
}

void CutsceneSeq::StageLoad() {}

// Initializes a new state with ID of `nextState`
void CutsceneSeq::NewState(int32 nextState)
{
    this->stateID = nextState;
    this->timer   = 0;

    for (int32 p = 0; p < 8; ++p) {
        this->values[p]   = 0;
        this->points[p].x = 0;
        this->points[p].y = 0;
    }
}
// Sets the cutscene's skip type (and callback if applicable)
void CutsceneSeq::SetSkipType(uint8 type, void (*callback)())
{
    CutsceneSeq *seq = GameObject::Get<CutsceneSeq>(SLOT_CUTSCENESEQ);
    if (seq->classID != TYPE_NONE) {
        seq->skipType     = type;
        seq->skipCallback = callback;
    }
}
// Checks if the cutscene was skipped
void CutsceneSeq::CheckSkip(uint8 skipType, void (*skipCallback)())
{
    bool32 skipPress = controllerInfo->keyStart.press;

    if (skipType && skipPress && (sceneInfo->state & ENGINESTATE_REGULAR)) {
        bool32 loadNewScene = false;

        if (skipType == CutsceneSeq::SkipNext) {
            ++sceneInfo->listPos;
            loadNewScene = true;
        }
        else {
            if (skipType == CutsceneSeq::SkipCallback)
                skipCallback();

            loadNewScene = this->skipType == CutsceneSeq::SkipCallback || this->skipType == CutsceneSeq::SkipReload;
        }

        if (loadNewScene) {
            globals->suppressTitlecard = false;
            globals->suppressAutoMusic = false;
            globals->enableIntro       = false;

            Stage::SetEngineState(ENGINESTATE_FROZEN);

            Zone::StartFadeOut(20, 0x000000);
            Music::FadeOut(0.035f);
        }
    }
}
// Does a foreach loop for the entity of type `type`
RSDK::GameObject::Entity *CutsceneSeq::GetEntity(int32 type)
{
    for (auto entity : GameObject::GetEntities(FOR_ALL_ENTITIES, type)) {
        return entity;
    }

    return nullptr;
}
// Locks control of the selected player
void CutsceneSeq::LockPlayerControl(Player *player)
{
    player->up        = false;
    player->down      = false;
    player->left      = false;
    player->right     = false;
    player->jumpHold  = false;
    player->jumpPress = false;

    Player::sVars->upState        = 0;
    Player::sVars->downState      = 0;
    Player::sVars->leftState      = 0;
    Player::sVars->rightState     = 0;
    Player::sVars->jumpPressState = 0;
    Player::sVars->jumpHoldState  = 0;
}
// Locks Control of all players
void CutsceneSeq::LockAllPlayerControl()
{
    for (int32 p = 0; p < Player::sVars->playerCount; ++p) {
        LockPlayerControl(GameObject::Get<Player>(p));
    }
}
// Sets up a cutscene sequence, the cutscene object should be passed as 'manager', then the cutscene states should be passed in order, make sure
// to end the states with StateMachine_None to tell it when to stop reading states
void CutsceneSeq::StartSequence(RSDK::GameObject::Entity *manager, ...)
{
    if (GameObject::Get<CutsceneSeq>(SLOT_CUTSCENESEQ)->classID)
        return;

    GameObject::Reset(SLOT_CUTSCENESEQ, sVars->classID, nullptr);
    CutsceneSeq *cutsceneSeq = GameObject::Get<CutsceneSeq>(SLOT_CUTSCENESEQ);

    cutsceneSeq->position.x    = 0;
    cutsceneSeq->position.y    = 0;
    cutsceneSeq->activeEntity  = (GameObject::Entity *)sceneInfo->entity;
    cutsceneSeq->managerEntity = manager;

    va_list list;
    va_start(list, manager);

    int32 count = 0;
    for (count = 0;; ++count) {
        void *state = va_arg(list, void *);
        if (!state)
            break;

        LogHelpers::Print("state = %x", state);
        cutsceneSeq->cutsceneStates[count] = (bool32(*)(CutsceneSeq *))state;
    }
    va_end(list);

    LogHelpers::Print("Starting sequence with %d states", count);
}

#if RETRO_INCLUDE_EDITOR
void CutsceneSeq::EditorDraw() {}

void CutsceneSeq::EditorLoad() {}
#endif

#if RETRO_REV0U
void CutsceneSeq::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(CutsceneSeq); }
#endif

void CutsceneSeq::Serialize() {}

} // namespace GameLogic