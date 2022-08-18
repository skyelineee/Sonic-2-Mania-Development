// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Debris Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Debris.hpp"
#include "Player.hpp"
#include "Zone.hpp"
// #include "Common/ScreenWrap.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Debris);

void Debris::Update()
{
    bool32 hidden = false;
    if (!this->state.Matches(&Debris::State_Init)) {
        if (this->hiddenDuration) {
            if (!--this->hiddenDuration) {
                if (this->rotation != this->targetRotation)
                    this->rotation = this->targetRotation;
                this->visible = true;
            }
            else {
                hidden = true;
            }
        }
    }

    if (!hidden)
        this->state.Run(this);

    if (!this->screenRelative) {
        CheckPlayerCollisions();
        if (this->active == ACTIVE_NORMAL && this->updateRange.x >= 0 && this->updateRange.y >= 0) {
            // if (ScreenWrap::CheckCompetitionWrap()) {
            //     TileLayer *layer = Zone::sVars->fgLayer[1].GetTileLayer();
            //     if ((this->position.y >> 16) >= 16 * layer->height)
            //         this->Destroy();
            // }
            // else {
                 if (!CheckOnScreen(&this->updateRange))
                     this->Destroy();
            // }
        }
    }
}
void Debris::LateUpdate() {}
void Debris::StaticUpdate() {}
void Debris::Draw()
{
    this->stateDraw.Run(this);
    // ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Debris::Draw), true);
}

void Debris::Create(void *data)
{
    this->stateDraw.Set(&Debris::StateDraw_Default);

    this->active        = ACTIVE_BOUNDS;
    this->visible       = true;
    this->updateRange.x = 0x800000;
    this->updateRange.y = 0x800000;
    this->drawFX        = FX_ROTATE | FX_FLIP;
    this->drawGroup     = Zone::sVars->objectDrawGroup[1];

    if (data)
        this->type = (uint8)VOID_TO_INT(data);

    if (!sceneInfo->inEditor) {
        this->startPos.x = this->position.x;
        this->startPos.y = this->position.y;
        this->animate    = true;
        this->initDelay  = 0;
        this->harmful    = false;
        this->field_110  = 0;

        this->animator2.SetAnimation(nullptr, 0, true, 0);
        this->animator3.SetAnimation(nullptr, 0, true, 0);

        switch (this->type) {
            case Debris::Spawner: this->state.Set(&Debris::State_Init); break;

            case Debris::Move:
                this->state.Set(&Debris::State_Move);
                this->active = ACTIVE_NORMAL;
                break;

            case Debris::Fall:
                this->state.Set(&Debris::State_Fall);
                this->active = ACTIVE_NORMAL;
                break;

            case Debris::Idle:
                this->state.Set(nullptr);
                this->active = ACTIVE_NORMAL;
                break;

            default: break;
        }
        this->screenRelative = false;

        // if (ScreenWrap::CheckCompetitionWrap())
        //     this->active = ACTIVE_NORMAL;
    }
}

void Debris::StageLoad()
{
    RSDK_DYNAMIC_PATH_ACTID("Debris");

    sVars->aniFrames.Load(dynamicPath, SCOPE_STAGE);
    sVars->aniFrames2 = sVars->aniFrames;
}

void Debris::StateDraw_Default()
{
    SET_CURRENT_STATE();

    if (this->zdepth)
        this->zdepth--;

    if (!this->screenRelative || !this->zdepth || this->zdepth == sceneInfo->currentScreenID) {
        if (this->animator1.frames)
            this->animator1.DrawSprite(nullptr, this->screenRelative);

        if (this->animator2.frames)
            this->animator2.DrawSprite(nullptr, this->screenRelative);

        if (this->animator3.frames)
            this->animator3.DrawSprite(nullptr, this->screenRelative);
    }
}

void Debris::State_Init()
{
    SET_CURRENT_STATE();

    if (--this->initDelay <= 0) {
        for (int32 i = 0; i < this->entryCount; ++i) {
            Info *entry = &this->entries[i];

            int32 offsetX   = entry->xOffset;
            int32 offsetY   = entry->yOffset;
            int32 velocityX = entry->xVel;
            int32 velocityY = entry->yVel;

            if (this->direction & FLIP_X) {
                offsetX   = -offsetX;
                velocityX = -velocityX;
            }

            if (this->direction & FLIP_Y) {
                offsetY   = -offsetY;
                velocityY = -velocityY;
            }

            Debris *child = GameObject::Create<Debris>(Debris::Fall, this->position.x, this->position.y);

            switch (this->spawnOffsetMode) {
                default:
                case OffsetFixedPoint16:
                    child->position.x += offsetX;
                    child->position.y += offsetY;
                    break;

                case OffsetWhole:
                    child->position.x += offsetX << 24 >> 8;
                    child->position.y += offsetY << 24 >> 8;
                    break;

                case OffsetFixedPoint8:
                    child->position.x += offsetX << 16 >> 8;
                    child->position.y += offsetY << 16 >> 8;
                    break;
            }

            switch (this->spawnVelocityMode) {
                default:
                case VelocityFixedPoint16:
                    child->velocity.x = velocityX;
                    child->velocity.y = velocityY;
                    break;

                case VelocityWhole:
                    child->velocity.x = velocityX << 24 >> 8;
                    child->velocity.y = velocityY << 24 >> 8;
                    break;

                case VelocityFixedPoint8:
                    child->velocity.x = velocityX << 16 >> 8;
                    child->velocity.y = velocityY << 16 >> 8;
                    break;
            }

            child->hiddenDuration  = this->hiddenDuration + this->field_110 * i;
            child->direction       = this->direction;
            child->gravityStrength = this->spawnGravityStrength;
            child->flicker         = this->spawnFlickerMode;

            switch (this->spawnFlickerMode) {
                default:
                case FlickerInactive: child->visible = false; break;
                case FlickerInvisible: child->visible = false; break;
                case FlickerNone: child->visible = true; break;
                case FlickerNormal: child->visible = i & 1; break;
                case FlickerRand: child->visible = Math::RandSeeded(0, 2, &Zone::sVars->randSeed); break;
            }

            child->active      = ACTIVE_NORMAL;
            child->updateRange = this->updateRange;
            child->drawGroup   = this->drawGroup;
            child->animator1.SetAnimation(sVars->aniFrames2, entry->listID, false, 0);
            child->animate = this->spawnAnimationMode != Debris::AnimateNone;
            if (this->spawnAnimationMode == Debris::AnimateRandFrame)
                child->animator1.frameID = Math::RandSeeded(0, child->animator1.frameCount, &Zone::sVars->randSeed);
            else
                child->animator1.frameID = entry->frame;

            child->State_Move();
        }

        this->state.Set(&Debris::State_Destroy);
        sVars->aniFrames2 = sVars->aniFrames;
    }
}

void Debris::State_Move()
{
    SET_CURRENT_STATE();

    if (this->flicker)
        this->visible = !this->visible;
    else
        this->visible = true;

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->rotation += this->rotSpeed;

    if (this->animate) {
        this->animator1.Process();

        if (this->animator1.GetFrameID() == '9')
            this->Destroy();
    }

    if (this->duration) {
        if (!--this->duration)
            this->Destroy();
    }
}

void Debris::State_Fall()
{
    SET_CURRENT_STATE();

    if (this->flicker)
        this->visible = !this->visible;
    else
        this->visible = true;

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
    this->velocity.y += this->gravityStrength;
    this->rotation += this->rotSpeed;

    if (this->animate) {
        this->animator1.Process();

        if (this->animator1.GetFrameID() == '9')
            this->Destroy();
    }

    if (this->duration) {
        if (!--this->duration)
            this->Destroy();
    }
}

void Debris::CheckPlayerCollisions()
{
    if (this->harmful) {
        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
            Hitbox *hitbox = this->animator1.GetHitbox(0);
            if (player->CheckBadnikTouch(this, hitbox))
                player->ProjectileHurt(this);
        }
    }

    // ScreenWrap::HandleHWrap(RSDK::ToGenericPtr(&Debris::CheckPlayerCollisions), true);
}

Debris *Debris::CreateFromEntries(Debris::Info *entries, uint16 entryCount, RSDK::Vector2 pos, int32 gravityStrength, uint16 offsetMode,
                                  uint16 velocityMode, int32 animationMode, int32 flickerMode)
{
    Debris *debris               = GameObject::Create<Debris>(Debris::Spawner, pos.x, pos.y);
    debris->entries              = entries;
    debris->entryCount           = entryCount;
    debris->spawnGravityStrength = gravityStrength;
    debris->spawnOffsetMode      = offsetMode;
    debris->spawnVelocityMode    = velocityMode;
    debris->spawnAnimationMode   = animationMode;
    debris->spawnFlickerMode     = flickerMode;
    return debris;
}

void Debris::State_Destroy()
{
    SET_CURRENT_STATE();

    this->Destroy();
}

void Debris::VelocityFromID(uint8 id)
{
    id %= 38;

    this->velocity.x = sVars->velocities[id].x;
    this->velocity.y = sVars->velocities[id].y;
    if (this->direction & FLIP_X)
        this->velocity.x = -this->velocity.x;
}

#if RETRO_INCLUDE_EDITOR
void Debris::EditorDraw() { this->animator1.DrawSprite(nullptr, false); }

void Debris::EditorLoad()
{
    RSDK_DYNAMIC_PATH_ACTID("Debris");

    sVars->aniFrames.Load(dynamicPath, SCOPE_STAGE);
}
#endif

#if RETRO_REV0U
void Debris::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Debris);

    sVars->aniFrames.Init();
    sVars->aniFrames2.Init();

    sVars->info[0].listID  = 0;
    sVars->info[0].frame   = 0;
    sVars->info[0].xOffset = 0;
    sVars->info[0].yOffset = 0;
    sVars->info[0].xVel    = -0x400;
    sVars->info[0].yVel    = -0x400;
    sVars->info[0].unknown = 0;

    sVars->info[1].listID  = 0;
    sVars->info[1].frame   = 1;
    sVars->info[1].xOffset = 0;
    sVars->info[1].yOffset = 0;
    sVars->info[1].xVel    = 0x400;
    sVars->info[1].yVel    = -0x400;
    sVars->info[1].unknown = 0;

    sVars->info[2].listID  = 0;
    sVars->info[2].frame   = 2;
    sVars->info[2].xOffset = 0;
    sVars->info[2].yOffset = 0;
    sVars->info[2].xVel    = -0x80;
    sVars->info[2].yVel    = -0x400;
    sVars->info[2].unknown = 0;

    sVars->info[3].listID  = 0;
    sVars->info[3].frame   = 0;
    sVars->info[3].xOffset = 0;
    sVars->info[3].yOffset = 0;
    sVars->info[3].xVel    = 0x80;
    sVars->info[3].yVel    = -0x400;
    sVars->info[3].unknown = 0;

    sVars->info[4].listID  = 0;
    sVars->info[4].frame   = 0;
    sVars->info[4].xOffset = 0;
    sVars->info[4].yOffset = 0;
    sVars->info[4].xVel    = -0x300;
    sVars->info[4].yVel    = -0x200;
    sVars->info[4].unknown = 0;

    sVars->info[5].listID  = 0;
    sVars->info[5].frame   = 1;
    sVars->info[5].xOffset = 0;
    sVars->info[5].yOffset = 0;
    sVars->info[5].xVel    = 0x300;
    sVars->info[5].yVel    = -0x200;
    sVars->info[5].unknown = 0;

    sVars->info[6].listID  = 0;
    sVars->info[6].frame   = 0;
    sVars->info[6].xOffset = 0;
    sVars->info[6].yOffset = 0;
    sVars->info[6].xVel    = -0x200;
    sVars->info[6].yVel    = -0x300;
    sVars->info[6].unknown = 0;

    sVars->info[7].listID  = 0;
    sVars->info[7].frame   = 2;
    sVars->info[7].xOffset = 0;
    sVars->info[7].yOffset = 0;
    sVars->info[7].xVel    = 0x200;
    sVars->info[7].yVel    = -0x300;
    sVars->info[7].unknown = 0;

    sVars->info[8].listID  = 0;
    sVars->info[8].frame   = 0;
    sVars->info[8].xOffset = 0;
    sVars->info[8].yOffset = 0;
    sVars->info[8].xVel    = -0x80;
    sVars->info[8].yVel    = -0x200;
    sVars->info[8].unknown = 0;

    sVars->info[9].listID  = 0;
    sVars->info[9].frame   = 1;
    sVars->info[9].xOffset = 0;
    sVars->info[9].yOffset = 0;
    sVars->info[9].xVel    = -0x80;
    sVars->info[9].yVel    = -0x200;
    sVars->info[9].unknown = 0;

    sVars->info[10].listID  = 0;
    sVars->info[10].frame   = 0;
    sVars->info[10].xOffset = 0;
    sVars->info[10].yOffset = 0;
    sVars->info[10].xVel    = -0x400;
    sVars->info[10].yVel    = -0x400;
    sVars->info[10].unknown = 0;

    sVars->info[11].listID  = 0;
    sVars->info[11].frame   = 1;
    sVars->info[11].xOffset = 0;
    sVars->info[11].yOffset = 0;
    sVars->info[11].xVel    = 0x400;
    sVars->info[11].yVel    = -0x400;
    sVars->info[11].unknown = 0;

    sVars->info[12].listID  = 0;
    sVars->info[12].frame   = 2;
    sVars->info[12].xOffset = 0;
    sVars->info[12].yOffset = 0;
    sVars->info[12].xVel    = -0x80;
    sVars->info[12].yVel    = -0x400;
    sVars->info[12].unknown = 0;

    sVars->info[13].listID  = 0;
    sVars->info[13].frame   = 0;
    sVars->info[13].xOffset = 0;
    sVars->info[13].yOffset = 0;
    sVars->info[13].xVel    = 0x80;
    sVars->info[13].yVel    = -0x400;
    sVars->info[13].unknown = 0;

    sVars->info[14].listID  = 0;
    sVars->info[14].frame   = 0;
    sVars->info[14].xOffset = 0;
    sVars->info[14].yOffset = 0;
    sVars->info[14].xVel    = -0x300;
    sVars->info[14].yVel    = -0x200;
    sVars->info[14].unknown = 0;

    sVars->info[15].listID  = 0;
    sVars->info[15].frame   = 1;
    sVars->info[15].xOffset = 0;
    sVars->info[15].yOffset = 0;
    sVars->info[15].xVel    = 0x300;
    sVars->info[15].yVel    = -0x200;
    sVars->info[15].unknown = 0;

    sVars->info[16].listID  = 0;
    sVars->info[16].frame   = 0;
    sVars->info[16].xOffset = 0;
    sVars->info[16].yOffset = 0;
    sVars->info[16].xVel    = -0x200;
    sVars->info[16].yVel    = -0x300;
    sVars->info[16].unknown = 0;

    sVars->info[17].listID  = 0;
    sVars->info[17].frame   = 2;
    sVars->info[17].xOffset = 0;
    sVars->info[17].yOffset = 0;
    sVars->info[17].xVel    = 0x200;
    sVars->info[17].yVel    = -0x300;
    sVars->info[17].unknown = 0;

    sVars->info[18].listID  = 0;
    sVars->info[18].frame   = 0;
    sVars->info[18].xOffset = 0;
    sVars->info[18].yOffset = 0;
    sVars->info[18].xVel    = -0x80;
    sVars->info[18].yVel    = -0x200;
    sVars->info[18].unknown = 0;

    sVars->info[19].listID  = 0;
    sVars->info[19].frame   = 1;
    sVars->info[19].xOffset = 0;
    sVars->info[19].yOffset = 0;
    sVars->info[19].xVel    = 0x80;
    sVars->info[19].yVel    = -0x200;
    sVars->info[19].unknown = 0;

    sVars->velocities[0].x  = -0x10000;
    sVars->velocities[0].y  = -0x10000;
    sVars->velocities[1].x  = 0x10000;
    sVars->velocities[1].y  = -0x10000;
    sVars->velocities[2].x  = -0x20000;
    sVars->velocities[2].y  = -0x20000;
    sVars->velocities[3].x  = 0x20000;
    sVars->velocities[3].y  = -0x20000;
    sVars->velocities[4].x  = -0x30000;
    sVars->velocities[4].y  = -0x20000;
    sVars->velocities[5].x  = 0x30000;
    sVars->velocities[5].y  = -0x20000;
    sVars->velocities[6].x  = -0x20000;
    sVars->velocities[6].y  = -0x20000;
    sVars->velocities[7].x  = 0x000000;
    sVars->velocities[7].y  = -0x20000;
    sVars->velocities[8].x  = -0x40000;
    sVars->velocities[8].y  = -0x30000;
    sVars->velocities[9].x  = 0x40000;
    sVars->velocities[9].y  = -0x30000;
    sVars->velocities[10].x = 0x30000;
    sVars->velocities[10].y = -0x30000;
    sVars->velocities[11].x = -0x40000;
    sVars->velocities[11].y = -0x30000;
    sVars->velocities[12].x = 0x40000;
    sVars->velocities[12].y = -0x30000;
    sVars->velocities[13].x = -0x20000;
    sVars->velocities[13].y = -0x20000;
    sVars->velocities[14].x = 0x20000;
    sVars->velocities[14].y = -0x20000;
    sVars->velocities[15].x = 0x000000;
    sVars->velocities[15].y = -0x10000;
    sVars->velocities[16].x = -0x4000;
    sVars->velocities[16].y = -0x7000;
    sVars->velocities[17].x = -0x8000;
    sVars->velocities[17].y = -0x7000;
    sVars->velocities[18].x = -0x18000;
    sVars->velocities[18].y = -0x7000;
    sVars->velocities[19].x = -0x10000;
    sVars->velocities[19].y = -0x7000;
    sVars->velocities[20].x = -0x20000;
    sVars->velocities[20].y = -0x7000;
    sVars->velocities[21].x = -0x28000;
    sVars->velocities[21].y = -0x7000;
    sVars->velocities[22].x = -0x30000;
    sVars->velocities[22].y = -0x7000;
    sVars->velocities[23].x = 0x000000;
    sVars->velocities[23].y = -0x10000;
    sVars->velocities[24].x = -0x10000;
    sVars->velocities[24].y = -0x10000;
    sVars->velocities[25].x = 0x10000;
    sVars->velocities[25].y = -0x10000;
    sVars->velocities[26].x = -0x20000;
    sVars->velocities[26].y = -0x10000;
    sVars->velocities[27].x = 0x20000;
    sVars->velocities[27].y = -0x10000;
    sVars->velocities[28].x = -0x20000;
    sVars->velocities[28].y = -0x20000;
    sVars->velocities[29].x = 0x20000;
    sVars->velocities[29].y = -0x20000;
    sVars->velocities[30].x = -0x30000;
    sVars->velocities[30].y = -0x20000;
    sVars->velocities[31].x = 0x30000;
    sVars->velocities[31].y = -0x20000;
    sVars->velocities[32].x = -0x30000;
    sVars->velocities[32].y = -0x30000;
    sVars->velocities[33].x = 0x30000;
    sVars->velocities[33].y = -0x30000;
    sVars->velocities[34].x = -0x40000;
    sVars->velocities[34].y = -0x30000;
    sVars->velocities[35].x = 0x40000;
    sVars->velocities[35].y = -0x30000;
    sVars->velocities[36].x = -0x20000;
    sVars->velocities[36].y = -0x30000;
    sVars->velocities[37].x = 0x20000;
    sVars->velocities[37].y = -0x30000;
}
#endif

void Debris::Serialize() {}

} // namespace GameLogic