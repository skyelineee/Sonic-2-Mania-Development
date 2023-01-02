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
}
void UIWidgets::Draw() {}

void UIWidgets::Create(void *data) {}

void UIWidgets::StageLoad()
{
    sVars->active = ACTIVE_ALWAYS;

    sVars->uiFrames.Load("UI/UIElements.bin", SCOPE_STAGE);
    sVars->saveSelFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE);
    sVars->fontFrames.Load("UI/SmallFont.bin", SCOPE_STAGE);

    UIWidgets::ApplyLanguage();
    sVars->frameAnimator.SetAnimation(&sVars->uiFrames, 1, true, 0);
    sVars->arrowsAnimator.SetAnimation(&sVars->uiFrames, 2, true, 0);

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
        case LANGUAGE_FR: sVars->textFrames.Load("UI/TextFR.bin", SCOPE_STAGE); break;
        case LANGUAGE_IT: sVars->textFrames.Load("UI/TextIT.bin", SCOPE_STAGE); break;
        case LANGUAGE_GE: sVars->textFrames.Load("UI/TextGE.bin", SCOPE_STAGE); break;
        case LANGUAGE_SP: sVars->textFrames.Load("UI/TextSP.bin", SCOPE_STAGE); break;
        case LANGUAGE_JP: sVars->textFrames.Load("UI/TextJP.bin", SCOPE_STAGE); break;
        default: break;
    }
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

    arrowsAnimator.SetAnimation(&sVars->saveSelFrames, 9, true, 9);
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

        animator.SetAnimation(&sVars->saveSelFrames, 8, true, (uint8)(strBuf[i] - '0'));
        animator.DrawSprite(&drawPos, false);

        drawPos.x += TO_FIXED(8);
    }
}

#if RETRO_INCLUDE_EDITOR
void UIWidgets::EditorDraw() {}

void UIWidgets::EditorLoad()
{
    sVars->uiFrames.Load("UI/UIElements.bin", SCOPE_STAGE);
    sVars->saveSelFrames.Load("UI/SaveSelect.bin", SCOPE_STAGE);
    sVars->fontFrames.Load("UI/SmallFont.bin", SCOPE_STAGE);
    sVars->textFrames.Load("UI/TextEN.bin", SCOPE_STAGE);

    sVars->frameAnimator.SetAnimation(&sVars->uiFrames, 1, true, 0);
    sVars->arrowsAnimator.SetAnimation(&sVars->uiFrames, 2, true, 0);

    sVars->buttonColor = 0xF0F0F0;
}
#endif

void UIWidgets::Serialize() {}

} // namespace GameLogic