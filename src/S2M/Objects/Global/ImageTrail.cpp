// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ImageTrail Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "ImageTrail.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ImageTrail);

void ImageTrail::Update() {}
void ImageTrail::LateUpdate()
{
    Player *player = this->player;

    // Check for fadeouts/destroy triggers
    if (player->superState == Player::SuperStateSuper) {
        this->baseAlpha = 0x100;
    }
    else {
        if (player->state.Matches(&Player::State_DrillKick)) {
            this->fadeoutTimer = 0x10;
        }
        else {
            if (this->fadeoutTimer <= 0) {
                if (player->speedShoesTimer < 32) {
                    this->baseAlpha = player->speedShoesTimer;
                    this->baseAlpha *= 8;
                    if (this->baseAlpha == 0)
                        this->Destroy();
                }
            }
            else {
                this->fadeoutTimer--;
                this->baseAlpha = 0x10;
                this->baseAlpha *= this->fadeoutTimer;
                if (this->baseAlpha == 0)
                    this->Destroy();
            }
        }
    }

    // Update recordings
    for (int32 i = IMAGETRAIL_TRACK_COUNT - 1; i > 0; --i) {
        this->statePos[i].x     = this->statePos[i - 1].x;
        this->statePos[i].y     = this->statePos[i - 1].y;
        this->stateRotation[i]  = this->stateRotation[i - 1];
        this->stateScale[i]     = this->stateScale[i - 1];
        this->stateDirection[i] = this->stateDirection[i - 1];
        this->stateVisible[i]   = this->stateVisible[i - 1];
        memcpy(&this->stateAnimator[i], &this->stateAnimator[i - 1], sizeof(Animator));
    }

    this->statePos[0].x     = this->currentPos.x;
    this->statePos[0].y     = this->currentPos.y;
    this->stateRotation[0]  = this->currentRotation;
    this->stateDirection[0] = this->currentDirection;
    this->stateScale[0]     = this->currentScale;
    this->stateVisible[0]   = this->currentVisible;
    memcpy(&this->stateAnimator[0], &this->currentAnimator, sizeof(Animator));

    // Record Player
    this->drawGroup        = player->drawGroup - 1;
    this->currentPos.x     = player->position.x;
    this->currentPos.y     = player->position.y;
    this->currentRotation  = player->rotation;
    this->currentDirection = player->direction;
    memcpy(&this->currentAnimator, &player->animator, sizeof(Animator));

    if (player->isChibi || !(player->drawFX & FX_SCALE))
        this->currentScale = 0x200;
    else
        this->currentScale = player->scale.x;

    // Check if we have enough speed to be visible
    if (abs(player->velocity.x) >= 0x10000 || abs(player->velocity.y) >= 0x10000)
        this->currentVisible = player->visible;
    else
        this->currentVisible = false;
}
void ImageTrail::StaticUpdate() {}
void ImageTrail::Draw()
{
    color colorStorage[32];
    for (int32 c = 0; c < 32; ++c) colorStorage[c] = paletteBank[0].GetEntry(c);

    // int32 alpha[3] = { 0xA0 * this->baseAlpha >> 8, this->baseAlpha >> 1, 0x60 * this->baseAlpha >> 8 };
    int32 alpha = 0x60 * this->baseAlpha >> 8;
    int32 inc   = 0x40 / (IMAGETRAIL_TRACK_COUNT / 3);

    switch (player->characterID) {
        default: break;
        case ID_SONIC: player->HandleSuperColors_Sonic(true); break;
        case ID_TAILS: player->HandleSuperColors_Tails(true); break;
        case ID_KNUCKLES: player->HandleSuperColors_Knux(true); break;
    }

    for (int32 i = (IMAGETRAIL_TRACK_COUNT / 3); i >= 0; --i) {
        int32 id = (i * 3) - (i - 1);

        Vector2 posStore = this->position;
        if (this->stateVisible[id]) {
            if (this->stateScale[id] != 0x200) {
                this->drawFX |= FX_SCALE;
                this->scale.x = this->stateScale[id];
                this->scale.y = this->stateScale[id];
            }
            this->alpha = alpha;
            alpha += inc;
            this->rotation  = this->stateRotation[id];
            this->direction = this->stateDirection[id];
            this->position  = this->statePos[id];
            player->DrawSprite((Player*)this, &this->stateAnimator[id]);
            this->drawFX &= ~FX_SCALE;
        }
        this->position = posStore;
    }

    if (this->playerClassID == player->classID) {
        for (int32 c = 0; c < 32; ++c) paletteBank[0].SetEntry(c, colorStorage[c]);
    }
}

void ImageTrail::Create(void *data)
{

    if (!sceneInfo->inEditor) {
        Player *player      = (Player *)data;
        this->active        = ACTIVE_ALWAYS;
        this->visible       = true;
        this->player        = player;
        this->playerClassID = player->classID; // dunno what this is for, maybe a v4 leftover where frames were per-object?
        this->baseAlpha     = 0x100;
        this->drawFX        = FX_FLIP | FX_SCALE | FX_ROTATE;
        this->inkEffect     = INK_ALPHA;

        for (int32 i = IMAGETRAIL_TRACK_COUNT - 1; i >= 0; --i) {
            this->statePos[i].x     = player->position.x;
            this->statePos[i].y     = player->position.y;
            this->stateRotation[i]  = player->rotation;
            this->stateDirection[i] = player->direction;
            this->stateVisible[i]   = false;
        }
        this->lastPos = player->position;
    }
}

void ImageTrail::StageLoad() {}

#if RETRO_INCLUDE_EDITOR
void ImageTrail::EditorDraw() {}

void ImageTrail::EditorLoad() {}
#endif

#if RETRO_REV0U
void ImageTrail::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(ImageTrail); }
#endif

void ImageTrail::Serialize() {}

} // namespace GameLogic