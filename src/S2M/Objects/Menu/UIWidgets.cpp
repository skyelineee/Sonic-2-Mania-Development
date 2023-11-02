// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: UIWidgets Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "UIWidgets.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Global/Localization.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(UIWidgets);

void UIWidgets::Update() {}
void UIWidgets::LateUpdate() {}
void UIWidgets::StaticUpdate()
{
    ++sVars->timer;
    sVars->timer &= 0x7FFF;

    sVars->buttonColor = sVars->buttonColors[(sVars->timer >> 1) & 0xF];

    sVars->arrowRightAnimator.Process();
    sVars->arrowLeftAnimator.Process();
    sVars->arrowDownAnimator.Process();
}
void UIWidgets::Draw() {}

void UIWidgets::Create(void *data) {}

void UIWidgets::StageLoad()
{
    sVars->active = ACTIVE_ALWAYS;

    // these should all load globally
    sVars->uiFrames.Load("UI/UIElements.bin", SCOPE_STAGE);
    sVars->fontFrames.Load("UI/SmallFont.bin", SCOPE_STAGE);
    // buttonframes is set to pausemenu.bin for every stage that isnt the main menu, gets replaced with uibuttons.bin if it is the main menu
    sVars->buttonFrames.Load("UI/PauseMenu.bin", SCOPE_STAGE);
    // with how buttons are setup, the description is always there and just has to get set to an empty frame so it can be not visible, works great for
    // the main menu but can lead to some issues here if i dont load anything for descframes, it gets set to titlecard.bin by default so i just have
    // to make it load pausemenu.bin again so i can give it an empty frame
    sVars->descFrames.Load("UI/PauseMenu.bin", SCOPE_STAGE);

    // these should only load in the main menu (this saves on stage space so we dont have a bunch of unneccessary textures loaded which could
    // negatively affect stuff later maybe idk)
    if (Stage::CheckSceneFolder("Menu")) {
        sVars->buttonFrames.Load("UI/UIButtons.bin", SCOPE_STAGE);
        sVars->descFrames.Load("UI/DescriptionText.bin", SCOPE_STAGE);
        sVars->timeAttackFrames.Load("UI/TimeAttack.bin", SCOPE_STAGE);
    }

    UIWidgets::ApplyLanguage();
    sVars->arrowRightAnimator.SetAnimation(&sVars->uiFrames, 0, true, 0);
    sVars->arrowLeftAnimator.SetAnimation(&sVars->uiFrames, 1, true, 0);
    sVars->arrowDownAnimator.SetAnimation(&sVars->uiFrames, 2, true, 0);

    sVars->sfxBleep.Get("Global/MenuBleep.wav");
    sVars->sfxAccept.Get("Global/MenuAccept.wav");
    sVars->sfxWarp.Get("Global/SpecialWarp.wav");
    sVars->sfxEvent.Get("Special/Event.wav");
    sVars->sfxWoosh.Get("Global/MenuWoosh.wav");
    sVars->sfxFail.Get("Stage/Fail.wav");

    sVars->buttonColors[0]  = 0xB00000;
    sVars->buttonColors[1]  = 0xB81820;
    sVars->buttonColors[2]  = 0xBC3440;
    sVars->buttonColors[3]  = 0xC44C60;
    sVars->buttonColors[4]  = 0xCC6480;
    sVars->buttonColors[5]  = 0xD07CA0;
    sVars->buttonColors[6]  = 0xD898C0;
    sVars->buttonColors[7]  = 0xE0B0E0;
    sVars->buttonColors[8]  = 0xE0B0E0;
    sVars->buttonColors[9]  = 0xD898C0;
    sVars->buttonColors[10] = 0xD07CA0;
    sVars->buttonColors[11] = 0xCC6480;
    sVars->buttonColors[12] = 0xC44C60;
    sVars->buttonColors[13] = 0xBC3440;
    sVars->buttonColors[14] = 0xB81820;
    sVars->buttonColors[15] = 0xB00000;
}

void UIWidgets::ApplyLanguage()
{
    LogHelpers::Print("Apply Language %d", Localization::sVars->language);

    switch (Localization::sVars->language) {
        case LANGUAGE_EN: sVars->textFrames.Load("UI/TextEN.bin", SCOPE_STAGE); break;
        default: break;
    }
}

void UIWidgets::DrawRectOutline_Black(int32 x, int32 y, int32 width, int32 height)
{
    int32 w = width << 16 >> 1;
    int32 h = height << 16 >> 1;

    Graphics::DrawRect(x - w, y - h, width << 16, TO_FIXED(3), 0x000000, 0xFF, INK_NONE, false);
    Graphics::DrawRect(x - w, y - h, TO_FIXED(3), height << 16, 0x000000, 0xFF, INK_NONE, false);
    Graphics::DrawRect(x - w, h + y - TO_FIXED(3), width << 16, TO_FIXED(3), 0x000000, 0xFF, INK_NONE, false);
    Graphics::DrawRect(x + w - TO_FIXED(3), y - h, TO_FIXED(3), height << 16, 0x000000, 0xFF, INK_NONE, false);
}

void UIWidgets::DrawRectOutline_Blended(int32 x, int32 y, int32 width, int32 height)
{
    int32 w = width << 16 >> 1;
    int32 h = height << 16 >> 1;

    Graphics::DrawRect(x - w + TO_FIXED(3), y - h, (width << 16) - TO_FIXED(6), TO_FIXED(3), 0x000000, 0xFF, INK_BLEND, false);
    Graphics::DrawRect(x - w, y - h, TO_FIXED(3), height << 16, 0x000000, 0xFF, INK_BLEND, false);
    Graphics::DrawRect(x - w + TO_FIXED(3), h + y - TO_FIXED(3), (width << 16) - TO_FIXED(6), TO_FIXED(3), 0x000000, 0xFF, INK_BLEND, false);
    Graphics::DrawRect(x + w - TO_FIXED(3), y - h, TO_FIXED(3), height << 16, 0x000000, 0xFF, INK_BLEND, false);
}

void UIWidgets::DrawRectOutline_Flash(int32 x, int32 y, int32 width, int32 height)
{
    int32 w     = width << 16 >> 1;
    int32 h     = height << 16 >> 1;
    color color = paletteBank[3].GetEntry((UIWidgets::sVars->timer >> 1) & 0xF);

    Graphics::DrawRect(x - w, y - h, width << 16, TO_FIXED(3), color, 0xFF, INK_NONE, false);
    Graphics::DrawRect(x - w, y - h, TO_FIXED(3), height << 16, color, 0xFF, INK_NONE, false);
    Graphics::DrawRect(x - w, h + y - TO_FIXED(3), width << 16, TO_FIXED(3), color, 0xFF, INK_NONE, false);
    Graphics::DrawRect(x + w - TO_FIXED(3), y - h, TO_FIXED(3), height << 16, color, 0xFF, INK_NONE, false);
}

void UIWidgets::DrawRightTriangle(int32 x, int32 y, int32 size, int32 red, int32 green, int32 blue)
{
    Vector2 verts[3];

    if (size) {
        verts[0].x = x;
        verts[0].y = y;
        verts[1].x = x;
        verts[1].y = y;
        verts[2].x = x;
        verts[2].y = y;

        if (size < 0) {
            verts[2].x = (size << 16) + x;
            verts[0].y = (size << 16) + y;
        }
        else {
            verts[1].x = (size << 16) + x;
            verts[2].y = (size << 16) + y;
        }

        if (sceneInfo->inEditor) {
            color clr = (blue << 0) | (green << 8) | (red << 16);

            Graphics::DrawLine(verts[0].x, verts[0].y, verts[1].x, verts[1].y, clr, 0xFF, INK_NONE, false);
            Graphics::DrawLine(verts[1].x, verts[1].y, verts[2].x, verts[2].y, clr, 0xFF, INK_NONE, false);
            Graphics::DrawLine(verts[2].x, verts[2].y, verts[0].x, verts[0].y, clr, 0xFF, INK_NONE, false);
        }
        else {
            int32 sx = screenInfo->position.x << 16;
            int32 sy = screenInfo->position.y << 16;
            verts[0].x -= sx;
            verts[0].y -= sy;
            verts[1].x -= sx;
            verts[1].y -= sy;
            verts[2].x -= sx;
            verts[2].y -= sy;

            Graphics::DrawFace(verts, 3, red, green, blue, 0xFF, INK_NONE);
        }
    }
}

void UIWidgets::DrawEquilateralTriangle(int32 x, int32 y, int32 size, uint8 sizeMode, int32 red, int32 green, int32 blue, InkEffects ink)
{
    Vector2 verts[3];

    if (sizeMode) {
        verts[0].x = x;
        verts[0].y = y;
        verts[1].x = x;
        verts[1].y = y;
        verts[2].x = x;
        verts[2].y = y;

        if (sizeMode == 1) {
            verts[0].x = x - (size << 16);
            verts[1].x = x + (size << 16);
            verts[2].y = y + (size << 16);
        }

        if (sceneInfo->inEditor) {
            color clr = blue | (green << 8) | (red << 16);
            Graphics::DrawLine(verts[0].x, verts[0].y, verts[1].x, verts[1].y, clr, 0xFF, ink, false);
            Graphics::DrawLine(verts[1].x, verts[1].y, verts[2].x, verts[2].y, clr, 0xFF, ink, false);
            Graphics::DrawLine(verts[2].x, verts[2].y, verts[0].x, verts[0].y, clr, 0xFF, ink, false);
        }
        else {
            int32 sx = screenInfo->position.x << 16;
            int32 sy = screenInfo->position.y << 16;
            verts[1].y -= sy;
            verts[2].x -= sx;
            verts[0].x -= sx;
            verts[0].y -= sy;
            verts[1].x -= sx;
            verts[2].y -= sy;

            Graphics::DrawFace(verts, 3, red, green, blue, 0xFF, ink);
        }
    }
}

void UIWidgets::DrawParallelogram(int32 x, int32 y, int32 width, int32 height, int32 edgeSize, int32 red, int32 green, int32 blue)
{
    Vector2 verts[4];

    verts[0].x = x - (width << 15);
    verts[0].y = y - (height << 15);
    verts[1].x = x + (width << 15);
    verts[1].y = y - (height << 15);
    verts[2].x = verts[1].x;
    verts[2].y = y + (height << 15);
    verts[3].x = verts[0].x;
    verts[3].y = y + (height << 15);

    if (edgeSize << 16 <= 0) {
        verts[0].x -= edgeSize << 16;
        verts[2].x += edgeSize << 16;
    }
    else {
        verts[1].x += edgeSize << 16;
        verts[3].x -= edgeSize << 16;
    }

    if (sceneInfo->inEditor) {
        color clr = (blue << 0) | (green << 8) | (red << 16);

        Graphics::DrawLine(verts[0].x, verts[0].y, verts[1].x, verts[1].y, clr, 0xFF, INK_NONE, false);
        Graphics::DrawLine(verts[1].x, verts[1].y, verts[2].x, verts[2].y, clr, 0xFF, INK_NONE, false);
        Graphics::DrawLine(verts[2].x, verts[2].y, verts[3].x, verts[3].y, clr, 0xFF, INK_NONE, false);
        Graphics::DrawLine(verts[3].x, verts[3].y, verts[0].x, verts[0].y, clr, 0xFF, INK_NONE, false);
    }
    else {
        int32 sx = screenInfo->position.x << 16;
        int32 sy = screenInfo->position.y << 16;
        verts[1].y -= sy;
        verts[2].y -= sy;
        verts[3].y -= sy;
        verts[0].y -= sy;
        verts[0].x -= sx;
        verts[1].x -= sx;
        verts[2].x -= sx;
        verts[3].x -= sx;

        Graphics::DrawFace(verts, 4, red, green, blue, 0xFF, INK_NONE);
    }
}

void UIWidgets::DrawLeftRightArrows(int32 x, int32 y, int32 arrowDist)
{
    Vector2 drawPos;

    drawPos.x = x;
    drawPos.y = y;

    drawPos.x -= arrowDist >> 1;
    sVars->arrowLeftAnimator.DrawSprite(&drawPos, false);

    drawPos.x += arrowDist;
    sVars->arrowRightAnimator.DrawSprite(&drawPos, false);
}

Vector2 UIWidgets::DrawTriJoinRect(int32 x, int32 y, color leftColor, color rightColor)
{
    Vector2 newPos;
    newPos.x = x + TO_FIXED(14);
    newPos.y = y;

    UIWidgets::DrawRightTriangle(x, newPos.y, 13, (leftColor >> 16) & 0xFF, (leftColor >> 8) & 0xFF, leftColor & 0xFF);
    UIWidgets::DrawRightTriangle(newPos.x, newPos.y + TO_FIXED(12), -13, (rightColor >> 16) & 0xFF, (rightColor >> 8) & 0xFF, rightColor & 0xFF);

    return newPos;
}

void UIWidgets::DrawTime(int32 x, int32 y, int32 minutes, int32 seconds, int32 milliseconds)
{
    Vector2 drawPos;
    Animator animator;
    Animator arrowsAnimator;
    char strBuf[16];

    memset(&animator, 0, sizeof(Animator));
    memset(&arrowsAnimator, 0, sizeof(Animator));

    drawPos.x = x;
    drawPos.y = y + TO_FIXED(2);

    arrowsAnimator.SetAnimation(&sVars->timeAttackFrames, 10, true, 0);
    arrowsAnimator.DrawSprite(&drawPos, false);

    drawPos.x += TO_FIXED(16);
    drawPos.y -= TO_FIXED(2);
    if (minutes) {
        if (minutes != 99 || seconds != 99 || milliseconds != 99)
            sprintf_s(strBuf, (int32)sizeof(strBuf), "%02d:%02d;%02d", minutes, seconds, milliseconds);
        else
            sprintf_s(strBuf, (int32)sizeof(strBuf), "<<:<<;<<");
    }
    else {
        if (!seconds && !milliseconds)
            sprintf_s(strBuf, (int32)sizeof(strBuf), "<<:<<;<<");
        else
            sprintf_s(strBuf, (int32)sizeof(strBuf), "%02d:%02d;%02d", minutes, seconds, milliseconds);
    }

    for (int32 i = 0; i < 8; ++i) {
        if (!strBuf[i])
            break;

        animator.SetAnimation(&sVars->timeAttackFrames, 9, true, (uint8)(strBuf[i] - '0'));
        animator.DrawSprite(&drawPos, false);

        drawPos.x += TO_FIXED(8);
    }
}

#if RETRO_INCLUDE_EDITOR
void UIWidgets::EditorDraw() {}

void UIWidgets::EditorLoad()
{
    sVars->uiFrames.Load("UI/UIElements.bin", SCOPE_STAGE);
    sVars->buttonFrames.Load("UI/UIButtons.bin", SCOPE_STAGE);
    sVars->fontFrames.Load("UI/SmallFont.bin", SCOPE_STAGE);
    sVars->descFrames.Load("UI/DescriptionText.bin", SCOPE_STAGE);
    sVars->timeAttackFrames.Load("UI/TimeAttack.bin", SCOPE_STAGE);

    sVars->arrowRightAnimator.SetAnimation(&sVars->uiFrames, 0, true, 0);
    sVars->arrowLeftAnimator.SetAnimation(&sVars->uiFrames, 1, true, 0);
    sVars->arrowDownAnimator.SetAnimation(&sVars->uiFrames, 2, true, 0);

    sVars->buttonColor = 0xF0F0F0;
}
#endif

void UIWidgets::Serialize() {}

} // namespace GameLogic