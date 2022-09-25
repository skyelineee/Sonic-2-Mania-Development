// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: HP_HUD Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "HP_HUD.hpp"
#include "HP_Player.hpp"
#include "Global/Zone.hpp"
#include "Global/HUD.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(HP_HUD);

void HP_HUD::Update() {}
void HP_HUD::LateUpdate() {}
void HP_HUD::StaticUpdate() {}
void HP_HUD::Draw()
{
    Vector2 drawPos;
    ScreenInfo *screen = &screenInfo[sceneInfo->currentScreenID];

    HP_Player *player1 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER1);
    HP_Player *player2 = GameObject::Get<HP_Player>(SLOT_HP_PLAYER2);

    if (globals->gameMode != MODE_COMPETITION) {
        if (!GET_CHARACTER_ID(2)) {
            drawPos.x = TO_FIXED(screen->center.x - 24);
            DrawRingCount(SLOT_HP_PLAYER1, &drawPos);
        }
        else {
            drawPos.x = TO_FIXED(screen->center.x - 104);
            DrawRingCount(SLOT_HP_PLAYER1, &drawPos);

            drawPos.x = TO_FIXED(screen->center.x + 48);
            DrawRingCount(SLOT_HP_PLAYER2, &drawPos);

            int32 totalRings = player1->rings + player2->rings;

            drawPos.x                         = TO_FIXED(screen->center.x);
            drawPos.y                         = TO_FIXED(21);
            this->hudElementsAnimator.frameID = 3;
            this->hudElementsAnimator.DrawSprite(&drawPos, true);

            drawPos.x = TO_FIXED(screen->center.x);
            drawPos.y = TO_FIXED(29);
            if (totalRings > 99) {
                drawPos.x -= TO_FIXED(12);
                this->numbersAnimator.frameID = totalRings / 100;
                this->numbersAnimator.DrawSprite(&drawPos, true);

                drawPos.x += TO_FIXED(8);
                this->numbersAnimator.frameID = totalRings % 100 / 10;
                this->numbersAnimator.DrawSprite(&drawPos, true);

                drawPos.x += TO_FIXED(8);
                this->numbersAnimator.frameID = totalRings % 10;
                this->numbersAnimator.DrawSprite(&drawPos, true);
            }
            else if (totalRings > 9) {
                drawPos.x -= TO_FIXED(8);
                this->numbersAnimator.frameID = totalRings / 10;
                this->numbersAnimator.DrawSprite(&drawPos, true);

                drawPos.x += TO_FIXED(8);
                this->numbersAnimator.frameID = totalRings % 10;
                this->numbersAnimator.DrawSprite(&drawPos, true);
            }
            else {
                drawPos.x -= TO_FIXED(4);

                this->numbersAnimator.frameID = totalRings;
                this->numbersAnimator.DrawSprite(&drawPos, true);
            }
        }
    }
    else {
        drawPos.x = TO_FIXED(screen->center.x - 104);
        DrawRingCount(SLOT_HP_PLAYER1, &drawPos);

        drawPos.x                         = TO_FIXED(screen->center.x - 96);
        drawPos.y                         = TO_FIXED(45);
        this->hudElementsAnimator.frameID = 4;
        for (int32 c = 0; c < player1->vsCheckpointID; ++c) {
            this->hudElementsAnimator.DrawSprite(nullptr, true);
            drawPos.x += TO_FIXED(16);
        }

        drawPos.x = TO_FIXED(screen->center.x + 48);
        DrawRingCount(SLOT_HP_PLAYER2, &drawPos);

        drawPos.x = TO_FIXED(screen->center.x + 56);
        drawPos.y = TO_FIXED(45);
        for (int32 c = 0; c < player1->vsCheckpointID; ++c) {
            this->hudElementsAnimator.DrawSprite(nullptr, true);
            drawPos.x += TO_FIXED(16);
        }
    }
}

void HP_HUD::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_NORMAL;
        this->visible   = true;
        this->drawGroup = Zone::sVars->hudDrawGroup;

        this->hudElementsAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->numbersAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
        this->tagAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);

        if (!sVars->setupDebugFlags)
            Dev::AddViewableVariable("Show HP HUD", &this->visible, Dev::VIEWVAR_UINT8, false, true);
        sVars->setupDebugFlags = true;
    }
}

void HP_HUD::StageLoad()
{
    sVars->aniFrames.Load("Special/HUD.bin", SCOPE_STAGE);

    sVars->setupDebugFlags = false;

    // Destroy any hud entities placed in the scene
    for (auto hud : GameObject::GetEntities<HP_HUD>(FOR_ALL_ENTITIES)) hud->Destroy();

    // ... and ensure we have a hud entity in the correct reserved slot
    GameObject::Reset(SLOT_HP_HUD, sVars->classID, nullptr);
}

void HP_HUD::DrawRingCount(uint16 playerSlot, RSDK::Vector2 *drawPos)
{
    HP_Player *player = GameObject::Get<HP_Player>(playerSlot);

    drawPos->y                        = TO_FIXED(20);
    this->hudElementsAnimator.frameID = HUD::CharacterIndexFromID(player->characterID);

    if (player->characterID == ID_TAILS && (globals->secrets & SECRET_REGIONSWAP))
        this->hudElementsAnimator.frameID = this->hudElementsAnimator.frameCount - 1;

    this->hudElementsAnimator.DrawSprite(drawPos, true);

    drawPos->x += TO_FIXED(44);
    drawPos->y = TO_FIXED(29);

    if (player->rings > 99) {
        this->numbersAnimator.frameID = player->rings / 100;
        this->numbersAnimator.DrawSprite(drawPos, true);

        drawPos->x += TO_FIXED(8);
        this->numbersAnimator.frameID = player->rings % 100 / 10;
        this->numbersAnimator.DrawSprite(drawPos, true);

        drawPos->x += TO_FIXED(8);
        this->numbersAnimator.frameID = player->rings % 10;
        this->numbersAnimator.DrawSprite(drawPos, true);
    }
    else if (player->rings > 9) {
        this->numbersAnimator.frameID = player->rings / 10;
        this->numbersAnimator.DrawSprite(drawPos, true);

        drawPos->x += TO_FIXED(8);
        this->numbersAnimator.frameID = player->rings % 10;
        this->numbersAnimator.DrawSprite(drawPos, true);
    }
    else {
        this->numbersAnimator.frameID = player->rings % 10;
        this->numbersAnimator.DrawSprite(drawPos, true);
    }
}

#if RETRO_INCLUDE_EDITOR
void HP_HUD::EditorDraw() {}

void HP_HUD::EditorLoad() {}
#endif

#if RETRO_REV0U
void HP_HUD::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(HP_HUD);

    sVars->aniFrames.Init();
}
#endif

void HP_HUD::Serialize() {}

} // namespace GameLogic