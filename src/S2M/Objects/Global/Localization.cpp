// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Localization Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Localization.hpp"
#include "Helpers/Options.hpp"
#include "Helpers/LogHelpers.hpp"
#include "Zone.hpp"

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

void Localization::GetZoneName(String *string, uint8 zone)
{
    switch (zone) {
        case Zone::OWZ: string->Init("OCEAN WIND", 0); break;
        case Zone::EHZ: string->Init("EMERALD HILL", 0); break;
        case Zone::CPZ: string->Init("CHEMICAL PLANT", 0); break;
        case Zone::ARZ: string->Init("AQUATIC RUIN", 0); break;
        case Zone::SWZ: string->Init("SECRET WOODS", 0); break;
        case Zone::CNZ: string->Init("CASINO NIGHT", 0); break;
        case Zone::HTZ: string->Init("HILL TOP", 0); break;
        case Zone::MCZ: string->Init("MYSTIC CAVE", 0); break;
        case Zone::SSZ: string->Init("SAND SHOWER", 0); break;
        case Zone::OOZ: string->Init("OIL OCEAN", 0); break;
        case Zone::MTZ: string->Init("METROPOLIS", 0); break;
        case Zone::CCZ: string->Init("CYBER CITY", 0); break;
        case Zone::SFZ: string->Init("SKY FORTRESS", 0); break;
        case Zone::DEZ: string->Init("DEATH EGG", 0); break;
        case Zone::HPZ: string->Init("HIDDEN PALACE", 0); break;
        default: break;
    }
}

#if RETRO_REV0U
void Localization::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(Localization); }
#endif

} // namespace GameLogic