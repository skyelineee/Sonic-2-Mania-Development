// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Localization Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Localization.hpp"
#include "Helpers/Options.hpp"
#include "Helpers/LogHelpers.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Localization);

void Localization::StageLoad()
{
    Options *options = Options::GetOptionsRAM();
    if (globals->optionsLoaded >= STATUS_OK && options->overrideLanguage) 
        sVars->language = options->language;
    else 
        sVars->language = SKU->language;

    LoadStrings();
}

void Localization::LoadStrings()
{
    sVars->text = "";

    switch (sVars->language) {
        case LANGUAGE_EN:
        default:
            LogHelpers::Print("Loading EN strings...");
            sVars->text.LoadStrings("StringsEN.txt");
            break;
    }

    sVars->text.Split(sVars->strings, 0, Localization::StringCount);
    sVars->loaded = true;
}

void Localization::GetString(RSDK::String *string, uint8 id)
{
    *string = "";
    String::Copy(string, &sVars->strings[id]);

    for (int32 c = 0; c < string->length; ++c) {
        if (string->chars[c] == '\\')
            string->chars[c] = '\n';
    }
}

#if RETRO_REV0U
void Localization::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(Localization); }
#endif

} // namespace GameLogic