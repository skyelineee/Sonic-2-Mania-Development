// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ParallaxSprite Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "ParallaxSprite.hpp"
#include "Global/Zone.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ParallaxSprite);

void ParallaxSprite::Update() { this->state.Run(this); }
void ParallaxSprite::LateUpdate() {}
void ParallaxSprite::StaticUpdate() {}
void ParallaxSprite::Draw()
{
    if (this->zdepth && sceneInfo->currentScreenID != this->overrideDrawGroup)
        return;

    Vector2 drawPos;
    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    switch (this->attribute) {
        default:
        case ParallaxSprite::AttrStandard:
        case ParallaxSprite::AttrEmitter:
        case ParallaxSprite::AttrParticle: {
            int32 scrollPosX = ((this->scrollPos.x + this->parallaxFactor.x * screen->position.x) & 0x7FFF0000) % this->loopPoint.x;
            drawPos.x        = this->position.x - scrollPosX;

            int32 scrollPosY = ((this->scrollPos.y + this->parallaxFactor.y * screen->position.y) & 0x7FFF0000) % this->loopPoint.y;
            drawPos.y        = this->position.y - scrollPosY;

            int32 loopX = -this->loopPoint.x >> 2;
            if (drawPos.x < loopX)
                drawPos.x += this->loopPoint.x;

            int32 loopY = -this->loopPoint.y >> 2;
            if (drawPos.y < loopY)
                drawPos.y += this->loopPoint.y;

            this->animator.DrawSprite(&drawPos, true);
            break;
        }

        case ParallaxSprite::AttrStatic: {
            drawPos = this->position;

            if (this->parallaxFactor.x)
                drawPos.x += ((int64)(this->position.x - ((screen->position.x + screen->center.x) << 16)) << 16) / this->parallaxFactor.x;

            if (this->parallaxFactor.y)
                drawPos.y += ((int64)(this->position.y - ((screen->position.y + screen->center.y) << 16)) << 16) / this->parallaxFactor.y;

            this->animator.DrawSprite(&drawPos, false);
            break;
        }
    }
}

void ParallaxSprite::Create(void *data)
{
    this->active    = ACTIVE_NORMAL;
    this->drawGroup = Zone::sVars->fgDrawGroup[0] + 1;

    if (data)
        this->attribute = (uint8)VOID_TO_INT(data);

    switch (this->attribute) {
        default:
        case ParallaxSprite::AttrStandard:
            this->parallaxFactor.x >>= 8;
            this->parallaxFactor.y >>= 8;
            this->drawFX  = FX_ROTATE;
            this->visible = !this->hiddenAtStart;
            this->state.Set(&ParallaxSprite::State_Normal);
            break;

        case ParallaxSprite::AttrEmitter:
            this->parallaxFactor.x >>= 8;
            this->parallaxFactor.y >>= 8;
            this->state.Set(&ParallaxSprite::State_Emitter);
            break;

        case ParallaxSprite::AttrParticle:
            this->inkEffect = INK_ALPHA;
            this->visible   = true;
            this->alpha     = 0x100;
            this->state.Set(&ParallaxSprite::State_Particle);
            break;

        case ParallaxSprite::AttrStatic:
            this->visible   = true;
            this->drawGroup = Zone::sVars->hudDrawGroup - 2;
            this->parallaxFactor.x >>= 8;
            this->parallaxFactor.y >>= 8;
            this->state.Set(&ParallaxSprite::State_Static);
            break;
    }

    if (this->overrideDrawGroup)
        this->drawGroup = this->overrideDrawGroup;

    this->animator.SetAnimation(sVars->aniFrames, this->aniID, true, 0);
}

void ParallaxSprite::StageLoad()
{
    RSDK_DYNAMIC_PATH_ACTID("ParallaxSprite");

    sVars->aniFrames.Load(dynamicPath, SCOPE_STAGE);
}

void ParallaxSprite::State_Emitter()
{
    SET_CURRENT_STATE();

    if (!(Zone::sVars->timer & 3) ) {
        ParallaxSprite *sprite = GameObject::Create<ParallaxSprite>(ParallaxSprite::AttrParticle, this->position.x, this->position.y);
        sprite->parallaxFactor = this->parallaxFactor;
        sprite->loopPoint      = this->loopPoint;
        sprite->scrollSpeed.y  = Math::Rand(0x8000, 0x20000);
        sprite->acceleration.x = Math::Rand(-0x100, 0x100);
    }
}

void ParallaxSprite::State_Particle()
{
    SET_CURRENT_STATE();

    this->animator.Process();

    this->scrollSpeed.x += this->acceleration.x;
    this->scrollSpeed.y += this->acceleration.y;

    this->scrollPos.x += this->scrollPos.x;
    this->scrollPos.y += this->scrollPos.x;

    this->alpha -= 2;
    if (!this->alpha)
        this->Destroy();
}

void ParallaxSprite::State_Static()
{
    SET_CURRENT_STATE();

    this->animator.Process();

    this->scrollPos.x += this->scrollSpeed.x;
    this->scrollPos.y += this->scrollSpeed.y;
}

void ParallaxSprite::State_Normal()
{
    SET_CURRENT_STATE();

    this->animator.Process();

    this->rotation     = (this->rotation + 2) & 0x1FF;

    this->scrollPos.x += this->scrollSpeed.x;
    this->scrollPos.y += this->scrollSpeed.y;
}

#if RETRO_INCLUDE_EDITOR
void ParallaxSprite::EditorDraw()
{
    this->animator.SetAnimation(sVars->aniFrames, this->aniID, false, 0);
    this->animator.DrawSprite(&this->position, false);
}

void ParallaxSprite::EditorLoad()
{
    RSDK_DYNAMIC_PATH_ACTID("ParallaxSprite");

    sVars->aniFrames.Load(dynamicPath, SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, attribute);
    RSDK_ENUM_VAR("Standard");
    RSDK_ENUM_VAR("Emitter");
    RSDK_ENUM_VAR("Particle");
    RSDK_ENUM_VAR("Static");
}
#endif

#if RETRO_REV0U
void ParallaxSprite::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(ParallaxSprite);

    sVars->aniFrames.Init();
}
#endif

void ParallaxSprite::Serialize()
{
    RSDK_EDITABLE_VAR(ParallaxSprite, VAR_UINT8, aniID);
    RSDK_EDITABLE_VAR(ParallaxSprite, VAR_UINT8, attribute);
    RSDK_EDITABLE_VAR(ParallaxSprite, VAR_VECTOR2, parallaxFactor);
    RSDK_EDITABLE_VAR(ParallaxSprite, VAR_VECTOR2, scrollSpeed);
    RSDK_EDITABLE_VAR(ParallaxSprite, VAR_VECTOR2, loopPoint);
    RSDK_EDITABLE_VAR(ParallaxSprite, VAR_BOOL, hiddenAtStart);
    RSDK_EDITABLE_VAR(ParallaxSprite, VAR_UINT8, overrideDrawGroup);
}

} // namespace GameLogic