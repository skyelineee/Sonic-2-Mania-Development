// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: BonusTitleCard Object
// Object Author: AChickMcNuggie
// ---------------------------------------------------------------------

#include "BonusTitleCard.hpp"
#include "Global/Zone.hpp"
#include "Global/Music.hpp"
#include "Global/SaveGame.hpp"
#include "Helpers/RPCHelpers.hpp"
#include "Helpers/FXFade.hpp"

using namespace RSDK;

// simple object, just shows a basic title card then sends the player back to the normal stage after a few seconds
namespace GameLogic
{
RSDK_REGISTER_OBJECT(BonusTitleCard);

void BonusTitleCard::Update()
{
    // waits for any of these keys and sets hasPressed to true, so then the fading out can occur
    // also prevents the timer from restarting if you press the key again since hasPressed will always be true
    if (controllerInfo->keyA.press || controllerInfo->keyStart.press) {
        this->hasPressed = true;
    }

    if (this->hasPressed) {
        Music::FadeOut(0.75);
        FXFade *fade    = GameObject::Create<FXFade>(0x000000, this->position.x, this->position.y);
        fade->drawGroup = Zone::sVars->hudDrawGroup + 1;
        fade->active    = ACTIVE_ALWAYS;
        fade->state.Set(&FXFade::State_FadeOut);
        fade->speedIn = 12;
        if (++this->fadeOutTimer >= 60) {
            Stage::SetScene("Adventure Mode", ""); // sets category
            sceneInfo->listPos = SaveGame::sVars->saveRAM->storedStageID; // sets the listpos to the storedstageid, which is stored when you enter the bonus stage
            Stage::LoadScene();
        }
    }
}

void BonusTitleCard::LateUpdate() {}
void BonusTitleCard::StaticUpdate() {}
void BonusTitleCard::Draw() 
{ 
    Graphics::FillScreen(0x000000, 255, 255, 255);

    // decreases speed every frame if its above 0
    if (this->speed > 0) {
        this->speed -= TO_FIXED(1);
    }

    if (this->bonusTextPos.y < TO_FIXED(110)) {
        this->bonusTextPos.y += this->speed;
    }
    
    this->zoneNameAnimator.DrawString(&this->bonusTextPos, &zoneName, 0, 0, 2, -1, nullptr, true);
}

void BonusTitleCard::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active      = ACTIVE_ALWAYS;
        this->visible     = true;
        this->hasPressed  = false;
        this->drawGroup   = Zone::sVars->hudDrawGroup + 1;

        this->speed          = TO_FIXED(21);
        this->bonusTextPos.x = TO_FIXED(300);
        this->bonusTextPos.y = TO_FIXED(-100);
        this->zoneNameAnimator.SetAnimation(&sVars->aniFrames, 1, false, 0);

        SetupTitleWords();
    }
}

void BonusTitleCard::StageLoad()
{
    sVars->aniFrames.Load("Global/TitleCard.bin", SCOPE_STAGE);

    const char *playingAsText  = "";
    const char *characterImage = "";
    const char *characterText  = "";
    switch (GET_CHARACTER_ID(1)) {
        case ID_SONIC:
            playingAsText  = "Playing as Sonic";
            characterImage = "sonic";
            characterText  = "Sonic";
            break;
        case ID_TAILS:
            playingAsText  = "Playing as Tails";
            characterImage = "tails";
            characterText  = "Tails";
            break;
        case ID_KNUCKLES:
            playingAsText  = "Playing as Knuckles";
            characterImage = "knuckles";
            characterText  = "Knuckles";
            break;
    }
    SetPresence(playingAsText, "In a Bonus Stage", "bonus", "Bonus Stage", characterImage, characterText);
}

void BonusTitleCard::SetupTitleWords()
{
    this->zoneName = "BONUS STAGE";

    this->zoneName.SetSpriteString(sVars->aniFrames, 1);
}

#if RETRO_INCLUDE_EDITOR
void BonusTitleCard::EditorDraw() {}

void BonusTitleCard::EditorLoad()
{
    sVars->aniFrames.Load("Editor/EditorIcons.bin", SCOPE_STAGE);
}
#endif

void BonusTitleCard::Serialize() {}

} // namespace GameLogic