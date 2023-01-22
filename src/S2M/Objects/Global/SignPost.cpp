// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: SignPost Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "SignPost.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "DebugMode.hpp"
#include "Music.hpp"
#include "ActClear.hpp"
#include "Ring.hpp"
#include "ItemBox.hpp"
#include "ScoreBonus.hpp"
#include "GameOver.hpp"

#include "Helpers/DrawHelpers.hpp"
#include "Helpers/MathHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(SignPost);

void SignPost::Update()
{
    if (!this->state.Matches(nullptr))
        this->state.Run(this);
}
void SignPost::LateUpdate() {}
void SignPost::StaticUpdate() {}
void SignPost::Draw()
{
    if (!this->state.Matches(nullptr)) {
        if (globals->useManiaBehavior) {
            Vector2 drawPos;

            this->drawFX = FX_SCALE;
            drawPos.y    = this->position.y;

            Animator *face = &this->facePlateAnimator;
            if (this->rotation <= 128 || this->rotation >= 384)
                face = &this->eggPlateAnimator;

            this->scale.x = abs(Math::Cos512(this->rotation));
            int32 scale   = abs(Math::Sin512(this->rotation));

            switch (this->rotation >> 7) {
                case 0:
                case 2:
                    drawPos.x = this->position.x + (scale << 9);
                    face->DrawSprite(&drawPos, false);
                    drawPos.x += -0xC00 * this->scale.x - (scale << 9);
                    break;

                case 1:
                case 3:
                    drawPos.x = this->position.x - (scale << 9);
                    face->DrawSprite(&drawPos, false);
                    drawPos.x += (scale + 2 * (3 * this->scale.x - 32)) << 9;
                    break;

                default: break;
            }
            this->scale.x = scale;
            this->sidebarAnimator.DrawSprite(&drawPos, false);

            this->drawFX = FX_NONE;
            this->postTopAnimator.DrawSprite(nullptr, false);
            this->standAnimator.DrawSprite(nullptr, false);
        }
        else {
            this->drawFX = FX_NONE;
            this->postTopAnimator.DrawSprite(nullptr, false);

            Animator *face = &this->eggPlateAnimator;
            if (this->eggPlateAnimator.frameID == 4)
                face = &this->facePlateAnimator;
            face->DrawSprite(nullptr, false);
            this->standAnimator.DrawSprite(nullptr, false);
        }
    }
}

void SignPost::Create(void *data)
{
    if (!this->vsBoundsSize.x)
        this->vsBoundsSize.x = 512 << 16;
    if (!this->vsBoundsSize.y)
        this->vsBoundsSize.y = 240 << 16;
    if (!this->vsExtendTop)
        this->vsExtendTop = 120;
    if (!this->vsExtendBottom)
        this->vsExtendBottom = 120;

    if (!sceneInfo->inEditor) {
        if (globals->gameMode != MODE_TIMEATTACK) {
            this->eggPlateAnimator.SetAnimation(sVars->aniFrames, globals->useManiaBehavior ? SignPost::AniEggman : SignPost::AniSpin, true, 0);
            switch (GET_CHARACTER_ID(1)) {
                default:
                case ID_SONIC: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniSonic, true, 0); break;
                case ID_TAILS: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniTails, true, 0); break;
                case ID_KNUCKLES: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniKnux, true, 0); break;
            }
            this->postTopAnimator.SetAnimation(sVars->aniFrames, SignPost::AniPost, true, 0);
            this->sidebarAnimator.SetAnimation(sVars->aniFrames, SignPost::AniPost, true, 1);
            this->standAnimator.SetAnimation(sVars->aniFrames, SignPost::AniPost, true, 2);
            this->updateRange.x = 0x400000;
            this->updateRange.y = 0x400000;
            for (int32 p = 0; p < Player::sVars->playerCount; ++p) {
                Zone::sVars->playerBoundActiveR[p] = true;
            }

            if (globals->gameMode == MODE_COMPETITION) {
                int32 left   = this->vsBoundsOffset.x - (this->vsBoundsSize.x >> 1);
                int32 top    = this->vsBoundsOffset.y - (this->vsBoundsSize.y >> 1);
                int32 right  = this->vsBoundsOffset.x + (this->vsBoundsSize.x >> 1);
                int32 bottom = this->vsBoundsOffset.y + (this->vsBoundsSize.y >> 1);

                int32 extendTop    = -(this->vsExtendTop << 16);
                int32 extendBottom = (this->vsExtendBottom << 16);

                if (extendTop < top)
                    top = extendTop;

                if (extendBottom < bottom)
                    bottom = extendBottom;

                if (abs(left) > right)
                    this->updateRange.x = abs(left);
                else
                    this->updateRange.x = right + 0x400000;

                if (abs(top) > bottom)
                    this->updateRange.y = abs(top) + 0x400000;
                else
                    this->updateRange.y = bottom + 0x400000;
            }
            this->visible   = true;
            this->drawGroup = Zone::sVars->objectDrawGroup[0];
            this->spinSpeed = 0x3000;
            this->spinCount = 8;
            this->maxAngle  = 0x10000;
            this->scale.y   = 0x200;

            bool32 destroy = true;
            switch (this->type) {
                default: break;
                case SignPost::RunPast: // Normal (Main Game Only)
                    if (globals->gameMode != MODE_COMPETITION) {
                        this->active = ACTIVE_BOUNDS;
                        this->state.Set(&SignPost::State_InitDelay);
                        destroy = false;
                    }
                    break;

                case SignPost::Drop: // Hidden (Until Dropped)
                    if (globals->gameMode != MODE_COMPETITION) {
                        this->active = ACTIVE_XBOUNDS;
                        this->state.Set(nullptr);
                        destroy = false;
                    }
                    break;

                case SignPost::Competition: // Normal (Competition Only)
                    if (globals->gameMode == MODE_COMPETITION) {
                        this->active = ACTIVE_BOUNDS;
                        this->state.Set(&SignPost::State_InitDelay);
                        destroy = false;
                    }
                    break;

                case SignPost::Decoration: // Decoration
                    if (globals->gameMode != MODE_COMPETITION) {
                        this->active = ACTIVE_BOUNDS;
                        this->state.Set(&SignPost::State_Done);
                        destroy = false;
                    }
                    break;
            }

            if (destroy)
                this->Destroy();
        }
    }
}

void SignPost::StageLoad()
{
    sVars->aniFrames.Load("Global/SignPost.bin", SCOPE_STAGE);

    sVars->hitboxSignPost.left   = -24;
    sVars->hitboxSignPost.top    = -22;
    sVars->hitboxSignPost.right  = 24;
    sVars->hitboxSignPost.bottom = 8;

    sVars->hitboxItemBox.left   = -8;
    sVars->hitboxItemBox.top    = 20;
    sVars->hitboxItemBox.right  = 8;
    sVars->hitboxItemBox.bottom = 24;

    sVars->maxPlayerCount = ~(-1 << Player::sVars->playerCount);

    DebugMode::AddObject(sVars->classID, &SignPost::DebugSpawn, &SignPost::DebugDraw);

    sVars->sfxSignPost.Get("Global/SignPost.wav");
    sVars->sfxSignPost2P.Get("Global/SignPost2p.wav");
    sVars->sfxTwinkle.Get("Global/Twinkle.wav");
    sVars->sfxBubbleBounce.Get("Global/BubbleBounce.wav");
    sVars->sfxSlide.Get("Global/Slide.wav");
}

void SignPost::DebugSpawn()
{
    SignPost *signpost           = GameObject::Create<SignPost>(0, this->position.x, this->position.y);
    signpost->spawnedByDebugMode = true;
}

void SignPost::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 4, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);

    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 3, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);

    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 4, true, 2);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void SignPost::State_Done()
{
    SET_CURRENT_STATE();

    this->facePlateAnimator.Process();

    if (this->spawnedByDebugMode) {
        Zone::sVars->stageFinishCallback.Set(nullptr);

        globals->atlEnabled        = false;
        globals->enableIntro       = false;
        globals->suppressTitlecard = false;
        globals->suppressAutoMusic = false;
    }

    if (!this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x180000, true)
        && !this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 1, 0, 0x180000, true)) {
        this->state.Set(&SignPost::State_FakeFalling);
    }
}

void SignPost::State_InitDelay()
{
    SET_CURRENT_STATE();

    this->state.Set(&SignPost::State_Wait);
}

void SignPost::Spawn(int32 x)
{
    SignPost *signpost = GameObject::Create<SignPost>(SignPost::Drop, 0, 0);

    SpriteFrame *frame = signpost->standAnimator.GetFrame(sVars->aniFrames);
    int32 standHeight  = ((frame->height + (frame->height >> 15)) >> 1);

    frame                 = signpost->facePlateAnimator.GetFrame(sVars->aniFrames);
    int32 facePlateHeight = ((frame->height + (frame->height >> 15)) >> 1);

    signpost->position.x = x;
    signpost->position.y = (screenInfo->position.y - standHeight - facePlateHeight) << 16;
    signpost->state.Set(globals->useManiaBehavior ? &SignPost::State_Falling : &SignPost::State_Fall_Classic);
    signpost->active = ACTIVE_NORMAL;
    sVars->sfxTwinkle.Play();
    signpost->drawGroup = GameObject::Get<Player>(SLOT_PLAYER1)->drawGroup - 1;
}

void SignPost::State_Fall_Classic()
{
    SET_CURRENT_STATE();

    HandleSparkles();

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (this->velocity.y >= 0) {
            if (player->velocity.y < 0 && player->animator.animationID == Player::ANI_JUMP && !player->onGround) {
                if (player->CheckCollisionTouchDefault(this, &sVars->hitboxSignPost)) {
                    this->velocity.x = (this->position.x - player->position.x) >> 4;
                    this->velocity.y = -0x20000;
                    sVars->sfxTwinkle.Play();

                    ScoreBonus *scoreBonus       = GameObject::Create<ScoreBonus>(0, this->position.x, this->position.y);
                    scoreBonus->drawGroup        = Zone::sVars->objectDrawGroup[1];
                    scoreBonus->animator.frameID = 0;

                    player->GiveScore(100);
                }
            }
        }
    }

    this->velocity.y += 0xC00;

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    this->eggPlateAnimator.Process();

    if (this->velocity.x >= 0) {
        if (this->position.x > (screenInfo->position.x + screenInfo->size.x - 32) << 16) {
            this->velocity.x = -this->velocity.x;
        }
        else if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_LWALL, 0, 0x180000, 0, true)) {
            this->velocity.x = -this->velocity.x;
        }
    }
    else {
        if (this->position.x < (screenInfo->position.x + 32) << 16) {
            this->velocity.x = -this->velocity.x;
        }
        else if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_RWALL, 0, -0x180000, 0, true)) {
            this->velocity.x = -this->velocity.x;
        }
    }

    if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x180000, true)
        || this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 1, 0, 0x180000, true)) {
        for (auto itemBox : GameObject::GetEntities<ItemBox>(FOR_ACTIVE_ENTITIES)) {
            if (itemBox->hidden) {
                if (itemBox->CheckCollisionBox(&ItemBox::sVars->hitboxHidden, this, &sVars->hitboxItemBox)) {
                    sVars->sfxBubbleBounce.Play();
                    itemBox->velocity.y = -0x50000;
                    itemBox->hidden     = false;
                    itemBox->isFalling  = true;
                    itemBox->drawGroup  = this->drawGroup + 1;
                    itemBox->state.Set(&ItemBox::State_Falling);
                    this->itemBounceCount++;
                    this->velocity.y = -0x20000;
                }
            }
        }

        if (this->velocity.y >= 0) {
            sVars->sfxSlide.Play();
            this->velocity.y = 0;
            Music::FadeOut(0.00875f);
            this->state.Set(&SignPost::State_Landed_Classic);
        }
    }
}

void SignPost::HandleSpin()
{
    this->angle += this->spinSpeed;

    if (this->angle >= this->maxAngle) {
        this->maxAngle += 0x20000;
        this->spinSpeed = MIN(0x600 * this->spinCount, 0x3000);
        if (!--this->spinCount) {
            this->spinSpeed               = 0;
            this->angle                   = 0x10000;
            this->facePlateAnimator.speed = 1;
        }
    }
    this->rotation = (this->angle >> 8) & 0x1FF;
}

void SignPost::HandleSparkles()
{
    if (!(Zone::sVars->timer & 3)) {
        Ring *sparkle = GameObject::Create<Ring>(0, this->position.x + Math::Rand(-0x180000, 0x180000),
                                                   (this->position.y + Math::Rand(-0x200000, 0x80000)));
        sparkle->state.Set(&Ring::State_Sparkle);
        sparkle->stateDraw.Set(&Ring::Draw_Sparkle);
        sparkle->active  = ACTIVE_NORMAL;
        sparkle->visible = false;
        sparkle->animator.SetAnimation(Ring::sVars->aniFrames, 2 + this->sparkleType, true, 0);
        if (sparkle->animator.animationID == 2) {
            sparkle->alpha         = 0xE0;
            sparkle->maxFrameCount = (sparkle->animator.frameCount >> 1) - 1;
        }
        else {
            sparkle->maxFrameCount = sparkle->animator.frameCount - 1;
        }
        sparkle->animator.speed = 6;
        this->sparkleType       = (this->sparkleType + 1) % 3;
    }
}

void SignPost::State_Sparkle_Classic()
{
    SET_CURRENT_STATE();

    Ring *self = (Ring *)this;

    if (self->position.x < self->drawPos.x)
        self->velocity.x += 0x40000;
    else
        self->velocity.x += -0x40000;

    if (self->velocity.x <= 0) {
        if (self->sparkleType + 1 < Zone::sVars->objectDrawGroup[1])
            self->drawGroup = self->sparkleType + 1;
        else
            self->drawGroup = Zone::sVars->objectDrawGroup[1];
    }
    else {
        if (self->sparkleType <= 0)
            self->drawGroup = 0;
        else
            self->drawGroup = self->sparkleType - 1;
    }
    self->position.x += self->velocity.x;
    self->position.y += self->velocity.y;

    self->animator.Process();

    self->timer--;
    if (self->timer <= 0)
        self->Destroy();
}

void SignPost::HandleCamBounds()
{
    int32 x = this->vsBoundsOffset.x + this->position.x;
    int32 y = this->vsBoundsOffset.y + this->position.y;

    Hitbox hitbox;
    hitbox.left   = -this->vsBoundsSize.x >> 17;
    hitbox.top    = -this->vsBoundsSize.y >> 17;
    hitbox.right  = this->vsBoundsSize.x >> 17;
    hitbox.bottom = this->vsBoundsSize.y >> 17;

    for (int32 p = 0; p < Player::sVars->playerCount; ++p) {
        Player *player = GameObject::Get<Player>(p);
        if (player->classID == Player::sVars->classID && !player->sidekick) {
            if (globals->gameMode == MODE_COMPETITION) {
                int32 storeX     = this->position.x;
                int32 storeY     = this->position.y;
                this->position.x = x;
                this->position.y = y;
                if (player->CheckCollisionTouch(this, &hitbox)) {
                    this->position.x                   = storeX;
                    this->position.y                   = storeY;
                    Zone::sVars->cameraBoundsL[p]      = (this->position.x >> 0x10) - screenInfo[p].center.x;
                    Zone::sVars->cameraBoundsR[p]      = screenInfo[p].center.x + (this->position.x >> 0x10);
                    Zone::sVars->playerBoundActiveR[p] = true;
                }
                else {
                    this->position.x = storeX;
                    this->position.y = storeY;
                }
            }
            else {
                if (this->position.x - player->position.x < 0x1000000 || this->position.x - (Zone::sVars->cameraBoundsR[p] << 16) < 0x1000000) {
                    Zone::sVars->cameraBoundsL[p] = (this->position.x >> 0x10) - screenInfo[p].center.x;
                    Zone::sVars->cameraBoundsR[p] = screenInfo[p].center.x + (this->position.x >> 0x10);
                }
            }
        }
    }
}

void SignPost::CheckTouch()
{

    for (int32 p = 0; p < Player::sVars->playerCount; ++p) {
        Player *player = GameObject::Get<Player>(p);
        if (this->activePlayers && GameObject::Get<Player>(p + Player::sVars->playerCount)->classID == GameOver::sVars->classID) {
            this->activePlayers |= 1 << p;
        }
        else {
             if (!p || globals->gameMode == MODE_COMPETITION) {
                 if (!((1 << p) & this->activePlayers)) {
                     bool32 passedSignpost = false;
                     if (globals->gameMode != MODE_COMPETITION) {
                         passedSignpost = player->position.x > this->position.x;
                     }
                     else if (this->playerPosStore[p].x && this->playerPosStore[p].y) {
                         passedSignpost = MathHelpers::CheckPositionOverlap(
                             player->position.x, player->position.y, this->playerPosStore[p].x, this->playerPosStore[p].y, this->position.x,
                             this->position.y - (this->vsExtendTop << 16), this->position.x, this->position.y + (this->vsExtendBottom << 16));
                     }

                     if (passedSignpost) {
                         sVars->sfxSignPost.Play();
                         this->active = ACTIVE_NORMAL;
                         if (player->superState == Player::SuperStateSuper)
                             player->superState = Player::SuperStateFadeOut;

                         int32 vel = 0;
                         if (player->onGround)
                             vel = player->groundVel;
                         else
                             vel = player->velocity.x;

                         this->velocity.y      = -(vel >> 1);
                         this->gravityStrength = vel / 96;
                         if (globals->gameMode == MODE_COMPETITION) {
                             this->active = ACTIVE_NORMAL;
                             if (!this->activePlayers) {
                                 switch (player->characterID) {
                                     default:
                                     case ID_SONIC: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniSonic, true, 0); break;
                                     case ID_TAILS: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniTails, true, 0); break;
                                     case ID_KNUCKLES: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniKnux, true, 0); break;
                                 }
                                 sVars->sfxSignPost2P.Play();
                             }

                             this->activePlayers |= 1 << p;
                             if (this->activePlayers == sVars->maxPlayerCount)
                                 Music::FadeOut(0.025f);
                             this->state.Set(&SignPost::State_SpinVS);
                         }
                         else {
                             if (globals->gameMode == MODE_ENCORE) {
                                 switch (GET_CHARACTER_ID(1)) {
                                     default:
                                     case ID_SONIC: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniSonic, true, 0); break;
                                     case ID_TAILS: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniTails, true, 0); break;
                                     case ID_KNUCKLES: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniKnux, true, 0); break;
                                 }
                             }

                             sceneInfo->timeEnabled = false;
                             if (vel >= 0x40000 && globals->useManiaBehavior) {
                                 this->state.Set(&SignPost::State_FlyUp);
                             }
                             else {
                                 Music::FadeOut(0.025f);
                                 this->state.Set(&SignPost::State_Spin);
                             }
                         }
                     }

                     this->playerPosStore[p].x = player->position.x;
                     this->playerPosStore[p].y = player->position.y;
                 }
             }
         }
    }
}

void SignPost::State_SpinVS()
{
    SET_CURRENT_STATE();

    HandleSpin();
    HandleSparkles();
    this->facePlateAnimator.Process();
    CheckTouch();

    if (!this->spinCount) {
        if (this->activePlayers >= sVars->maxPlayerCount) {
            this->type = SignPost::Decoration;
            this->state.Set(&SignPost::State_Done);
            sceneInfo->timeEnabled = false;
        }
        else {
            this->spinSpeed = 0x3000;
            this->spinCount = 8;
            this->maxAngle  = 0x10000;
            this->state.Set(&SignPost::State_Wait);
        }
    }
}

void SignPost::State_Spin()
{
    SET_CURRENT_STATE();

    HandleSpin();
    HandleSparkles();
    this->facePlateAnimator.Process();
    this->eggPlateAnimator.Process();

    if (!this->spinCount) {
        this->velocity.y = 0;
        this->eggPlateAnimator.SetAnimation(sVars->aniFrames, this->facePlateAnimator.animationID, true, this->facePlateAnimator.frameID);
        this->type = SignPost::Decoration;
        this->state.Set(&SignPost::State_Done);
        GameObject::Reset(SLOT_ACTCLEAR, ActClear::sVars->classID, nullptr);
    }
}

void SignPost::State_FlyUp()
{
    SET_CURRENT_STATE();

    HandleSpin();
    HandleSparkles();
    this->facePlateAnimator.Process();

    this->spinCount = 16;
    this->position.y += this->velocity.y;
    this->velocity.y += this->gravityStrength;
    if (this->velocity.y >= 0)
        this->state.Set(&SignPost::State_Falling);
}

void SignPost::State_Wait()
{
    SET_CURRENT_STATE();

    HandleCamBounds();
    CheckTouch();
}

void SignPost::State_Falling()
{
    SET_CURRENT_STATE();

    this->active           = ACTIVE_NORMAL;
    sceneInfo->timeEnabled = false;

    if (this->type == SignPost::Drop) {
        this->type = SignPost::RunPast;
        if (globals->gameMode < MODE_COMPETITION) {
            switch (GET_CHARACTER_ID(1)) {
                default:
                case ID_SONIC: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniSonic, true, 0); break;
                case ID_TAILS: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniTails, true, 0); break;
                case ID_KNUCKLES: this->facePlateAnimator.SetAnimation(sVars->aniFrames, SignPost::AniKnux, true, 0); break;
            }
        }
    }

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        if (this->velocity.y >= 0) {
            if (player->velocity.y < 0 && player->animator.animationID == Player::ANI_JUMP && !player->onGround) {
                if (player->CheckCollisionTouchDefault(this, &sVars->hitboxSignPost)) {
                    this->velocity.x = (this->position.x - player->position.x) >> 4;
                    this->velocity.y = -0x20000;
                    sVars->sfxTwinkle.Play();

                    ScoreBonus *scoreBonus       = GameObject::Create<ScoreBonus>(0, this->position.x, this->position.y);
                    scoreBonus->drawGroup        = Zone::sVars->objectDrawGroup[1];
                    scoreBonus->animator.frameID = 0;

                    player->GiveScore(100);
                }
            }
        }
    }

    HandleSpin();
    HandleSparkles();
    this->facePlateAnimator.Process();

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->spinCount = 16;

    if (this->velocity.x >= 0) {
        if (this->position.x > (screenInfo->position.x + screenInfo->size.x - 32) << 16) {
            this->velocity.x = -this->velocity.x;
        }
        else if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_LWALL, 0, 0x180000, 0, true)) {
            this->velocity.x = -this->velocity.x;
        }
    }
    else {
        if (this->position.x < (screenInfo->position.x + 32) << 16) {
            this->velocity.x = -this->velocity.x;
        }
        else if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_RWALL, 0, -0x180000, 0, true)) {
            this->velocity.x = -this->velocity.x;
        }
    }

    this->velocity.y += 0xC00;

    if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x180000, true)) {
        for (auto itemBox : GameObject::GetEntities<ItemBox>(FOR_ACTIVE_ENTITIES)) {
            if (itemBox->hidden) {
                if (itemBox->CheckCollisionBox(&ItemBox::sVars->hitboxHidden, this, &sVars->hitboxItemBox)) {
                    sVars->sfxBubbleBounce.Play();
                    itemBox->velocity.y = -0x50000;
                    itemBox->hidden     = false;
                    itemBox->state.Set(&ItemBox::State_Falling);
                    this->itemBounceCount++;
                    this->velocity.y = -0x20000;
                }
            }
        }

        if (this->velocity.y >= 0) {
            sVars->sfxSlide.Play();
            this->spinCount  = 4;
            this->velocity.y = 0;
            Music::FadeOut(0.025f);
            this->state.Set(&SignPost::State_Spin);
        }
    }
}

void SignPost::State_FakeFalling()
{
    SET_CURRENT_STATE();

    this->active = ACTIVE_NORMAL;

    this->position.y += this->velocity.y;
    this->velocity.y += 0x3800;

    if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x180000, true) && this->velocity.y >= 0) {
        this->velocity.y = 0;
        this->state.Set(&SignPost::State_Done);
    }

    if (!this->CheckOnScreen(&this->updateRange))
        this->Destroy();
}

void SignPost::State_Landed_Classic()
{
    SET_CURRENT_STATE();

    this->timer++;
    if (this->timer >= 64) {
        if (this->zdepth) {
            if (this->timer >= 107) {
                this->state.Set(&SignPost::State_Done);
                GameObject::Reset(SLOT_ACTCLEAR, ActClear::sVars->classID, nullptr);
            }
        }
        else {
            this->zdepth                    = 1;
            this->eggPlateAnimator.frameID = 4;
            this->eggPlateAnimator.speed   = 0;
            this->type                     = SignPost::Decoration;
        }

        if (!ActClear::sVars->disableVictory)
            ActClear::CheckPlayerVictory();
    }

    this->eggPlateAnimator.Process();
}

#if RETRO_INCLUDE_EDITOR
void SignPost::EditorDraw()
{
    this->eggPlateAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
    this->postTopAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);
    this->standAnimator.SetAnimation(sVars->aniFrames, 4, true, 2);

    this->postTopAnimator.DrawSprite(nullptr, false);
    this->eggPlateAnimator.DrawSprite(nullptr, false);
    this->standAnimator.DrawSprite(nullptr, false);

    if (this->type == SignPost::Competition && showGizmos()) {
        RSDK_DRAWING_OVERLAY(true);

        Vector2 drawPos;
        drawPos.x = this->vsBoundsOffset.x + this->position.x;
        drawPos.y = this->vsBoundsOffset.y + this->position.y;
        Hitbox hitbox;
        hitbox.left   = -(this->vsBoundsSize.x >> 17);
        hitbox.top    = -(this->vsBoundsSize.y >> 17);
        hitbox.right  = this->vsBoundsSize.x >> 17;
        hitbox.bottom = this->vsBoundsSize.y >> 17;
        DrawHelpers::DrawHitboxOutline(drawPos.x, drawPos.y, &hitbox, FLIP_NONE, 0xFFFFFF);

        Vector2 drawPosB, drawPosT;
        drawPosB.x = this->position.x;
        drawPosB.y = this->position.y + (this->vsExtendBottom << 16);
        drawPosT.x = this->position.x;
        drawPosT.y = this->position.y - (this->vsExtendTop << 16);
        DrawHelpers::DrawLine(drawPosB, drawPosT, 0xFF0000);

        RSDK_DRAWING_OVERLAY(false);
    }
}

void SignPost::EditorLoad()
{
    sVars->aniFrames.Load("Global/SignPost.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Run Past");
    RSDK_ENUM_VAR("Drop");
    RSDK_ENUM_VAR("Competition");
}
#endif

#if RETRO_REV0U
void SignPost::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(SignPost);

    sVars->aniFrames.Init();

    sVars->sfxSignPost.Init();
    sVars->sfxSignPost2P.Init();
    sVars->sfxTwinkle.Init();
    sVars->sfxBubbleBounce.Init();
    sVars->sfxSlide.Init();
}
#endif

void SignPost::Serialize()
{
    RSDK_EDITABLE_VAR(SignPost, VAR_UINT8, type);
    RSDK_EDITABLE_VAR(SignPost, VAR_VECTOR2, vsBoundsSize);
    RSDK_EDITABLE_VAR(SignPost, VAR_VECTOR2, vsBoundsOffset);
    RSDK_EDITABLE_VAR(SignPost, VAR_INT32, vsExtendTop);
    RSDK_EDITABLE_VAR(SignPost, VAR_INT32, vsExtendBottom);
}

} // namespace GameLogic