// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: Zone Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "Zone.hpp"
#include "SaveGame.hpp"
#include "Music.hpp"
#include "ActClear.hpp"
#include "Player.hpp"

#include "ItemBox.hpp"
#include "Ring.hpp"
#include "SignPost.hpp"
#include "EggPrison.hpp"
#include "Shield.hpp"
#include "InvincibleStars.hpp"
#include "ImageTrail.hpp"
#include "SuperSparkle.hpp"
#include "Animals.hpp"
#include "Common/Decoration.hpp"

using namespace RSDK;

// dont have anywhere else to put it, sorry lol
char GameLogic::dynamicPath[0x40];
int32 GameLogic::dynamicPathActID;
int32 GameLogic::dynamicPathUnknown;

// stageFolder, spriteFolder, zoneID, actID, noActID, isSavable, listPos
GameLogic::StageFolderInfo GameLogic::stageList[] = {
    { "LSelect", "LSelect", -1, 0, false, false, false }, { "OWZ", "OWZ", 0, 0, true, false, false },   { "EHZ", "EHZ", 1, 0, false, false, false },
    { "EHZ", "EHZ", 1, 1, false, true, false },           { "CPZ", "CPZ", 2, 0, false, false, false },  { "CPZ", "CPZ", 2, 1, false, true, false },
    { "ARZ", "ARZ", 3, 0, false, false, false },          { "ARZ", "ARZ", 3, 1, false, true, false },   { "SWZ", "SWZ", 4, 0, false, false, false },
    { "SWZ", "SWZ", 4, 1, false, true, false },           { "CNZ", "CNZ", 5, 0, false, false, false },  { "CNZ", "CNZ", 5, 1, false, true, false },
    { "HTZ", "HTZ", 6, 0, false, false, false },          { "HTZ", "HTZ", 6, 1, false, true, false },   { "MCZ", "MCZ", 7, 0, false, false, false },
    { "MCZ", "MCZ", 7, 1, false, true, false },           { "HPZ", "HPZ", 8, 0, true, true, false },    { "SSZ", "SSZ", 9, 0, false, false, false },
    { "SSZ", "SSZ", 9, 1, false, true, false },           { "OOZ", "OOZ", 10, 0, false, false, false }, { "OOZ", "OOZ", 10, 1, false, true, false },
    { "MTZ", "MTZ", 11, 0, false, false, false },         { "MTZ", "MTZ", 11, 1, false, true, false },  { "CCZ", "CCZ", 12, 0, false, false, false },
    { "CCZ", "CCZ", 12, 1, false, true, false },          { "SCZ", "SCZ", 13, 0, true, true, false },   { "WFZ", "WFZ", 14, 0, true, true, false },
    { "DEZ", "DEZ", 15, 0, false, false, false },         { "DEZ", "DEZ", 15, 1, false, true, false }, { "PPZ", "PPZ", 16, 0, true, false, false }
};

void GameLogic::StrCopy(char *dest, uint32 destSize, const char *src)
{
    uint32 c = 0;
    for (; src[c] && c < destSize - 1; ++c) dest[c] = src[c];
    dest[c] = 0;
}
void GameLogic::StrAppend(char *dest, uint32 bufferSize, const char *src)
{
    uint32 destStrPos = 0;
    uint32 srcStrPos  = 0;
    while (dest[destStrPos]) ++destStrPos;

    while (destStrPos < bufferSize - 1) {
        if (!src[srcStrPos]) {
            break;
        }
        dest[destStrPos++] = src[srcStrPos++];
    }
    dest[destStrPos] = 0;
}

namespace GameLogic
{
RSDK_REGISTER_OBJECT(Zone);

void Zone::Update() { this->state.Run(this); }
void Zone::LateUpdate()
{
    if (sceneInfo->entitySlot != SLOT_ZONE) {
        this->state.Run(this);
        return;
    }

    HandlePlayerBounds();

    this->state.Run(this);

    if (sceneInfo->timeEnabled && sceneInfo->minutes == 10 && !(globals->medalMods & MEDAL_NOTIMEOVER)) {
        sceneInfo->minutes      = 9;
        sceneInfo->seconds      = 59;
        sceneInfo->milliseconds = 99;
        sceneInfo->timeEnabled  = false;

        if (Player::sVars) {
            Player::sVars->sfxHurt.Play();

            for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
                if (!player->sidekick)
                    player->deathType = Player::DeathDie_Sfx;
            }

            sVars->gotTimeOver = true;
            sVars->timeOverCallback.Run(this);
        }
    }

    // You took an hour to beat the stage... no time bonus for you!
    if (ActClear::sVars && (sceneInfo->minutes == 59 && sceneInfo->seconds == 59))
        ActClear::sVars->disableTimeBonus = true;

    // Player Draw order sorting
    // Ensure P1 is always on top
    if (Player::sVars && Player::sVars->playerCount > 0) {
        Player *sidekick = GameObject::Get<Player>(SLOT_PLAYER2);
        if ((!sidekick->state.Matches(&Player::State_FlyToPlayer) && !sidekick->state.Matches(&Player::State_ReturnToPlayer))
            || sidekick->characterID == ID_TAILS || sidekick->scale.x == 0x200) {
            Player *leader = GameObject::Get<Player>(SLOT_PLAYER1);
            Graphics::SwapDrawListEntries(leader->drawGroup, SLOT_PLAYER1, SLOT_PLAYER2, Player::sVars->playerCount);
        }
    }
}
void Zone::StaticUpdate()
{
    sVars->timer             = (sVars->timer + 1) & 0x7FFF;
    globals->persistentTimer = (globals->persistentTimer + 1) & 0x7FFF;

    if (globals->useManiaBehavior) {
        if (!(sVars->timer & 1))
            sVars->ringFrame = (sVars->ringFrame + 1) & 0xF;
    }
    else {
        if (!(sVars->timer & 3))
            sVars->ringFrame = (sVars->ringFrame + 1) & 7;
    }

    // Handle times for the summary screen
    int32 zone = Zone::CurrentID();

    // if (zone >= ZONE_AIZ)
    //     zone = ZONE_AIZ;
    if (zone == Zone::Invalid)
        return;

    int32 act = Zone::sVars->actID;
    if (act >= 3)
        act = 0;

    int32 pos = act + 2 * zone;
    if (pos >= 0 && sceneInfo->timeEnabled && globals->gameMode < MODE_TIMEATTACK)
        ++SaveGame::sVars->saveRAM->zoneTimes[pos];
}
void Zone::Draw()
{
    if (this->screenID >= CAMERA_COUNT || this->screenID == sceneInfo->currentScreenID)
        this->stateDraw.Run(this);
}

void Zone::Create(void *data)
{
    this->active = ACTIVE_ALWAYS;

    if (this->stateDraw.Matches(nullptr)) {
        this->visible   = false;
        this->drawGroup = -1;
    }
}

void Zone::StageLoad()
{
    int32 stageID = 0;
    if (!sceneInfo->inEditor)
        stageID = sceneInfo->listData[sceneInfo->listPos].id[0];

    sVars->folderListPos = 0;
    sVars->actID         = stageList[sVars->folderListPos].actID;
    sVars->useFolderIDs  = stageList[sVars->folderListPos].useFolderIDs;

    for (int32 i = 0; i < 0x80; ++i) {
        StageFolderInfo *stage = &stageList[i];

        if (Stage::CheckSceneFolder(stage->stageFolder)) {
            if (sceneInfo->inEditor || stage->noActID) {
                sVars->folderListPos = i;
                sVars->actID         = stage->actID;
                sVars->useFolderIDs  = stage->useFolderIDs;
                i                    = 0x80;
                break;
            }
            else {
                for (; i < 0x80; ++i) {
                    stage = &stageList[i];
                    if ('1' + stage->actID == stageID) {
                        sVars->folderListPos = i;
                        sVars->actID         = stage->actID;
                        sVars->useFolderIDs  = stage->useFolderIDs;
                        i                    = 0x80;
                        break;
                    }
                }
            }
        }
    }

    sVars->timer           = 0;
    sVars->autoScrollSpeed = 0;
    sVars->ringFrame       = 0;
    sVars->gotTimeOver     = false;
    sVars->vsSwapCBCount   = 0;

    // Setup draw group ids (shouldn't be changed after this, but can be if needed)
    sVars->fgDrawGroup[0]     = 0;
    sVars->objectDrawGroup[0] = 2;
    sVars->playerDrawGroup[0] = 4;
    sVars->fgDrawGroup[1]     = 6;
    sVars->objectDrawGroup[1] = 8;
    sVars->playerDrawGroup[1] = 12;
    sVars->hudDrawGroup       = 14;

    // Layer IDs
    sVars->fgLayer[0].Get("FG Low");
    sVars->fgLayer[1].Get("FG High");
    sVars->moveLayer.Get("Move");
    sVars->scratchLayer.Get("Scratch");

    // Layer Masks

    if (sVars->fgLayer[0].Loaded())
        sVars->fgLayerMask[0] = 1 << sVars->fgLayer[0].id;

    if (sVars->fgLayer[1].Loaded())
        sVars->fgLayerMask[1] = 1 << sVars->fgLayer[1].id;

    if (sVars->moveLayer.Loaded())
        sVars->moveLayerMask = 1 << sVars->moveLayer.id;

    sVars->collisionLayers = (1 << sVars->fgLayer[0].id) | (1 << sVars->fgLayer[1].id);

    // Get Layer size and setup default bounds
    Vector2 layerSize;
    sVars->fgLayer[0].Size(&layerSize, true);

    if (!sVars->swapGameMode) {
        for (int32 s = 0; s < PLAYER_COUNT; ++s) {
            sVars->cameraBoundsL[s] = 0;
            sVars->cameraBoundsR[s] = layerSize.x;
            sVars->cameraBoundsT[s] = 0;
            sVars->cameraBoundsB[s] = layerSize.y;

            sVars->playerBoundsL[s] = sVars->cameraBoundsL[s] << 0x10;
            sVars->playerBoundsR[s] = sVars->cameraBoundsR[s] << 0x10;
            sVars->playerBoundsT[s] = sVars->cameraBoundsT[s] << 0x10;
            sVars->playerBoundsB[s] = sVars->cameraBoundsB[s] << 0x10;

            sVars->deathBoundary[s]      = sVars->cameraBoundsB[s] << 0x10;
            sVars->playerBoundActiveL[s] = true;
            sVars->playerBoundActiveB[s] = false;
        }
    }

    // Setup cool bonus
    if (!globals->initCoolBonus) {
        globals->coolBonus[0]  = 10000;
        globals->coolBonus[1]  = 10000;
        globals->coolBonus[2]  = 10000;
        globals->coolBonus[3]  = 10000;
        globals->initCoolBonus = true;
    }

    // Destroy any zone entities placed in the scene
    for (auto zone : GameObject::GetEntities<Zone>(FOR_ALL_ENTITIES)) zone->Destroy();
    // ... and ensure we have a zone entity in the correct reserved slot
    GameObject::Reset(SLOT_ZONE, sVars->classID, nullptr);

    sVars->sfxFail.Get("Stage/Fail.wav");
}

int32 Zone::GetZoneID()
{
    if (Stage::CheckSceneFolder("OWZ"))
        return OWZ;
    if (Stage::CheckSceneFolder("EHZ"))
        return EHZ;
    if (Stage::CheckSceneFolder("CPZ"))
        return CPZ;
    if (Stage::CheckSceneFolder("ARZ"))
        return ARZ;
    if (Stage::CheckSceneFolder("SWZ"))
        return SWZ;
    if (Stage::CheckSceneFolder("CNZ"))
        return CNZ;
    if (Stage::CheckSceneFolder("HTZ"))
        return HTZ;
    if (Stage::CheckSceneFolder("MCZ"))
        return MCZ;
    if (Stage::CheckSceneFolder("SSZ"))
        return SSZ;
    if (Stage::CheckSceneFolder("OOZ"))
        return OOZ;
    if (Stage::CheckSceneFolder("MTZ"))
        return MTZ;
    if (Stage::CheckSceneFolder("CCZ"))
        return CCZ;
    if (Stage::CheckSceneFolder("SFZ"))
        return SFZ;
    if (Stage::CheckSceneFolder("DEZ"))
        return DEZ;
    if (Stage::CheckSceneFolder("HPZ"))
        return HPZ;
    return Invalid;
}

void Zone::HandlePlayerBounds()
{
    if (!Player::sVars)
        return;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        int32 playerID = SLOT_PLAYER1;
        if (!player->sidekick)
            playerID = player->Slot();

        Hitbox *playerHitbox = player->GetHitbox();

        // Left Boundary
        if (sVars->playerBoundActiveL[playerID]) {
            int32 offset = -0x10000 * playerHitbox->left;
            if (player->position.x - offset <= sVars->playerBoundsL[playerID]) {
                player->position.x = sVars->playerBoundsL[playerID] + offset;

                if (player->onGround) {
                    if (player->groundVel < sVars->autoScrollSpeed) {
                        player->velocity.x = sVars->autoScrollSpeed;
                        player->groundVel  = sVars->autoScrollSpeed;
                        player->pushing    = false;
                    }
                }
                else if (player->velocity.x < sVars->autoScrollSpeed) {
                    player->velocity.x = sVars->autoScrollSpeed;
                    player->groundVel  = 0;
                }
            }
        }

        // Right Boundary
        if (sVars->playerBoundActiveR[playerID]) {
            int32 offset = playerHitbox->right << 16;
            if (player->position.x + offset >= sVars->playerBoundsR[playerID]) {
                player->position.x = sVars->playerBoundsR[playerID] - offset;

                if (player->onGround) {
                    if (player->groundVel > sVars->autoScrollSpeed) {
                        player->velocity.x = sVars->autoScrollSpeed;
                        player->groundVel  = sVars->autoScrollSpeed;
                        player->pushing    = false;
                    }
                }
                else {
                    if (player->velocity.x > sVars->autoScrollSpeed) {
                        player->velocity.x = sVars->autoScrollSpeed;
                        player->groundVel  = 0;
                    }
                }
            }
        }

        // Top Boundary
        if (sVars->playerBoundActiveT[playerID]) {
            if (player->position.y - 0x140000 < sVars->playerBoundsT[playerID]) {
                player->position.y = sVars->playerBoundsT[playerID] + 0x140000;
                player->velocity.y = 0;
            }
        }

        // Death Boundary
        if (!player->state.Matches(&Player::State_Death)
            && !player->state.Matches(&Player::State_DeathHold)
            /*&& !player->state.Matches(Unknown_PlayerState_Tubing)*/
            && !player->deathType) {
            if (sVars->playerBoundsB[playerID] <= sVars->deathBoundary[playerID]) {
                if (player->position.y > sVars->deathBoundary[playerID]) {
                    player->deathType                   = Player::DeathDie_NoSfx;
                    sVars->playerBoundActiveB[playerID] = false;
                }
            }
            else if (player->position.y > sVars->playerBoundsB[playerID]) {
                player->deathType                   = Player::DeathDie_NoSfx;
                sVars->playerBoundActiveB[playerID] = false;
            }
        }

        // Bottom Boundary
        if (sVars->playerBoundActiveB[playerID]) {
            if (player->position.y + 0x140000 > sVars->playerBoundsB[playerID]) {
                player->position.y = sVars->playerBoundsB[playerID] - 0x140000;
                player->velocity.y = 0;
                player->onGround   = true;
            }
        }
    }
}

void Zone::AddToHyperList(uint16 classID, bool32 hyperDashTarget, bool32 hyperSlamTarget, bool32 superFlickyTarget)
{
    for (int32 i = 0; i < 0x80; ++i) {
        if (!sVars->hyperList[i].classID) {
            sVars->hyperList[i].classID           = classID;
            sVars->hyperList[i].hyperDashTarget   = hyperDashTarget;
            sVars->hyperList[i].hyperSlamTarget   = hyperSlamTarget;
            sVars->hyperList[i].superFlickyTarget = superFlickyTarget;
            ++sVars->hyperListCount;
            break;
        }
    }
}

void Zone::StoreEntities(RSDK::Vector2 offset)
{
    // "Normalize" the positions of players, signposts & itemboxes when we store them
    // (this is important for later)

    globals->atlOffset.x = offset.x;
    globals->atlOffset.y = offset.y;

    int32 count   = 0;
    int32 dataPos = 0;

    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES)) {
        player->position.x -= offset.x;
        player->position.y -= offset.y;
        globals->atlEntitySlot[count] = player->Slot();
        GameObject::Copy(&globals->atlEntityData[dataPos], player, false);
        count++;
        dataPos += 0x400;

        globals->atlCameraBoundsL[0] = screenInfo->position.x;
        globals->atlCameraBoundsR[0] = screenInfo->position.x + screenInfo->size.x;
        globals->atlCameraBoundsT[0] = screenInfo->position.y;
        globals->atlCameraBoundsB[0] = screenInfo->position.y + screenInfo->size.y;

        globals->atlCameraBoundsL[0] -= offset.x >> 16;
        globals->atlCameraBoundsR[0] -= offset.x >> 16;
        globals->atlCameraBoundsT[0] -= offset.y >> 16;
        globals->atlCameraBoundsB[0] -= offset.y >> 16;

        if (player->camera) {
            globals->atlCameraPos[0].x = player->camera->position.x;
            globals->atlCameraPos[0].y = player->camera->position.y;

            globals->atlCameraPos[0].x -= offset.x >> 16;
            globals->atlCameraPos[0].y -= offset.y >> 16;
        }
    }

    for (auto shield : GameObject::GetEntities<Shield>(FOR_ACTIVE_ENTITIES)) {
        shield->position.x -= offset.x;
        shield->position.y -= offset.y;
        globals->atlEntitySlot[count] = shield->Slot();
        GameObject::Copy(&globals->atlEntityData[dataPos], shield, false);
        count++;
        dataPos += 0x400;
    }

    for (auto invincibleStars : GameObject::GetEntities<InvincibleStars>(FOR_ACTIVE_ENTITIES)) {
        invincibleStars->position.x -= offset.x;
        invincibleStars->position.y -= offset.y;
        globals->atlEntitySlot[count] = invincibleStars->Slot();
        GameObject::Copy(&globals->atlEntityData[dataPos], invincibleStars, false);
        count++;
        dataPos += 0x400;
    }

    for (auto signPost : GameObject::GetEntities<SignPost>(FOR_ACTIVE_ENTITIES)) {
        signPost->position.x -= offset.x;
        signPost->position.y -= offset.y;
        globals->atlEntitySlot[count] = signPost->Slot();
        GameObject::Copy(&globals->atlEntityData[dataPos], signPost, false);
        count++;
        dataPos += 0x400;
    }

    for (auto itemBox : GameObject::GetEntities<ItemBox>(FOR_ACTIVE_ENTITIES)) {
        itemBox->position.x -= offset.x;
        itemBox->position.y -= offset.y;
        globals->atlEntitySlot[count] = itemBox->Slot();
        GameObject::Copy(&globals->atlEntityData[dataPos], itemBox, false);
        count++;
        dataPos += 0x400;
    }

    for (auto capsule : GameObject::GetEntities<EggPrison>(FOR_ACTIVE_ENTITIES)) {
        capsule->position.x -= offset.x;
        capsule->position.y -= offset.y;
        globals->atlEntitySlot[count] = capsule->Slot();
        GameObject::Copy(&globals->atlEntityData[dataPos], capsule, false);
        count++;
        dataPos += 0x400;
    }

    for (auto animal : GameObject::GetEntities<Animals>(FOR_ACTIVE_ENTITIES)) {
        animal->position.x -= offset.x;
        animal->position.y -= offset.y;
        globals->atlEntitySlot[count] = animal->Slot();
        GameObject::Copy(&globals->atlEntityData[dataPos], animal, false);
        count++;
        dataPos += 0x400;
    }

    for (auto sparkle : GameObject::GetEntities<SuperSparkle>(FOR_ACTIVE_ENTITIES)) {
        sparkle->position.x -= offset.x;
        sparkle->position.y -= offset.y;
        globals->atlEntitySlot[count] = sparkle->Slot();
        GameObject::Copy(&globals->atlEntityData[dataPos], sparkle, false);
        count++;
        dataPos += 0x400;
    }

    for (auto trail : GameObject::GetEntities<ImageTrail>(FOR_ACTIVE_ENTITIES)) {
        trail->position.x -= offset.x;
        trail->position.y -= offset.y;

        for (int32 i = IMAGETRAIL_TRACK_COUNT - 1; i > 0; --i) {
            trail->statePos[i].x -= offset.x;
            trail->statePos[i].y -= offset.y;
        }

        globals->atlEntitySlot[count] = trail->Slot();
        GameObject::Copy(&globals->atlEntityData[dataPos], trail, false);
        count++;
        dataPos += 0x400;
    }

    // store any relevant info about the player
    Player *player1          = GameObject::Get<Player>(SLOT_PLAYER1);
    globals->restartLives[0] = player1->lives;
    globals->restartScore    = player1->score;
    globals->restartPowerups = player1->shield;
    globals->atlEntityCount  = count;
    globals->atlEnabled      = true;
}
void Zone::ReloadEntities(RSDK::Vector2 offset, bool32 setATLBounds)
{
    // reload any stored entities we have
    for (int32 e = 0; e < globals->atlEntityCount; ++e) {
        Entity *storedEntity = (Entity *)&globals->atlEntityData[e << 10];
        Entity *entity       = nullptr;

        // only players & powerups get to be overridden, everything else is just added to the temp area
        if (globals->atlEntitySlot[e] >= 28)
            entity = GameObject::Create(0, 0, 0);
        else
            entity = GameObject::Get(globals->atlEntitySlot[e]);

        if (storedEntity->classID == Player::sVars->classID) {
            Player *storedPlayer = (Player *)storedEntity;
            Player *player       = (Player *)entity;

            player->shield = storedPlayer->shield;
            player->ApplyShield();
        }
        else {
            GameObject::Copy(entity, storedEntity, false);
        }

        entity->position.x = storedEntity->position.x + offset.x;
        entity->position.y = storedEntity->position.y + offset.y;

    }

    // clear ATL data, we dont wanna do it again
    memset(globals->atlEntityData, 0, globals->atlEntityCount << 10);

    // if we're allowing the new boundary, update our camera to use ATL bounds instead of the default ones
    sVars->setATLBounds = setATLBounds;
    if (setATLBounds) {
        Player *player         = GameObject::Get<Player>(SLOT_PLAYER1);
        player->camera         = NULL;
        Camera *camera         = GameObject::Get<Camera>(SLOT_CAMERA1);
        camera->position.x     = offset.x;
        camera->position.y     = offset.y;
        camera->state.Set(nullptr);
        camera->target         = nullptr;
        camera->boundsL        = (offset.x >> 16) - screenInfo->center.x;
        camera->boundsR        = (offset.x >> 16) + screenInfo->center.x;
        camera->boundsT        = (offset.y >> 16) - screenInfo->size.y;
        camera->boundsB        = offset.y >> 16;
        Camera::sVars->centerBounds.x = TO_FIXED(8);
        Camera::sVars->centerBounds.y = TO_FIXED(4);
    }

    Player::sVars->savedLives = globals->restartLives[0];
    Player::sVars->savedScore = globals->restartScore;
    Player::sVars->powerups   = globals->restartPowerups;
    globals->atlEntityCount   = 0;

    for (auto player : GameObject::GetEntities<Player>(FOR_ALL_ENTITIES)) {
        player->onGround      = true;
        player->groundedStore = true;
        player->state.Set(&Player::State_Ground);
    }
}

void Zone::StartFadeOut(int32 fadeSpeed, int32 fadeColor)
{
    Zone *zone = GameObject::Get<Zone>(SLOT_ZONE);

    zone->fadeColor = fadeColor;
    zone->fadeSpeed = fadeSpeed;
    zone->screenID  = CAMERA_COUNT;
    zone->timer     = 0;
    zone->state.Set(&Zone::State_Fade);
    zone->stateDraw.Set(&Zone::Draw_Fade);
    zone->visible   = true;
    zone->drawGroup = 15;
}

void Zone::StartFadeOut_MusicFade(int32 fadeSpeed, int32 fadeColor)
{
    StartFadeOut(fadeSpeed, fadeColor);

    Music::FadeOut(0.035f);
}

void Zone::GoBonus_Pinball()
{
    SaveGame::sVars->saveRAM->storedStageID = sceneInfo->listPos;
    Stage::SetScene("Bonus Stages", "Gachapon Bonus");
    StartFadeOut(10, 0xF0F0F0);
}

void Zone::StartTeleportAction() {}

void Zone::RotateOnPivot(RSDK::Vector2 *position, RSDK::Vector2 *pivot, int32 angle)
{
    int32 x     = (position->x - pivot->x) >> 8;
    int32 y     = (position->y - pivot->y) >> 8;
    position->x = pivot->x + (y * Math::Sin256(angle)) + x * Math::Cos256(angle);
    position->y = pivot->y + (y * Math::Cos256(angle)) - x * Math::Sin256(angle);
}

void Zone::Draw_Fade()
{
    SET_CURRENT_STATE();

    Graphics::FillScreen(this->fadeColor, this->timer, this->timer - 0x80, this->timer - 0x100);
}

void Zone::State_Fade()
{
    SET_CURRENT_STATE();

    this->timer += this->fadeSpeed;

    if (this->timer > 1024) {
        Stage::LoadScene();
    }
}



void Zone::ApplyWorldBounds()
{
    if (sVars->setATLBounds) {
        Camera *camera = GameObject::Get<Camera>(SLOT_CAMERA1);

        for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
        {
            int32 camWorldL = camera->boundsL << 16;
            if (player->position.x - TO_FIXED(10) <= camWorldL) {
                player->position.x = camWorldL + TO_FIXED(10);
                if (player->onGround) {
                    if (player->groundVel < 0) {
                        player->velocity.x = 0;
                        player->groundVel  = 0;
                        player->pushing    = false;
                    }
                }
                else if (player->velocity.x < 0) {
                    player->velocity.x = 0;
                    player->groundVel  = 0;
                }
            }

            int32 camWorldR = camera->boundsR << 16;
            if (player->position.x + TO_FIXED(10) >= camWorldR) {
                player->position.x = camWorldR - TO_FIXED(10);
                if (player->onGround) {
                    if (player->groundVel > 0) {
                        player->velocity.x = 0;
                        player->groundVel  = 0;
                        player->pushing    = false;
                    }
                }
                else if (player->velocity.x > 0) {
                    player->velocity.x = 0;
                    player->groundVel  = 0;
                }
            }
        }
    }
}

int32 Zone::CurrentID()
{
    SET_CURRENT_STATE();

    for (int32 i = 0; i < 0x80; ++i) {
        StageFolderInfo *stage = &stageList[i];

        if (Stage::CheckSceneFolder(stage->stageFolder))
            return stage->zoneID;
    }
    return -1;
}

bool32 Zone::CurrentStageSaveable()
{
    SET_CURRENT_STATE();

    int32 stageID = sceneInfo->listData[sceneInfo->listPos].id[0];

    for (int32 i = 0; i < 0x80; ++i) {
        StageFolderInfo *stage = &stageList[i];

        if (Stage::CheckSceneFolder(stage->stageFolder)) {
            if (stage->noActID)
                return stage->isSavable;

            for (i = 0; i < 0x80; ++i) {
                stage = &stageList[i];

                if (stageID == '1' + stage->actID)
                    return stage->isSavable;
            }
        }
    }
    return false;
}

int32 Zone::GetListPos(uint8 zone, uint8 act)
{
    SET_CURRENT_STATE();

    for (int32 i = 0; i < 0x80; ++i) {
        StageFolderInfo *stage = &stageList[i];

        if (stage->zoneID == zone && stage->actID == act) {
            SceneListInfo *category = &sceneInfo->listCategory[sceneInfo->activeCategory];
            SceneListEntry *scene   = &sceneInfo->listData[category->sceneOffsetStart];

            for (int32 i = 0; i + (category->sceneOffsetStart - 1) < category->sceneOffsetEnd; ++i) {
                if (strncmp(scene->folder, stage->stageFolder, sizeof(scene->folder)) == 0
                    && (stage->noActID || scene->id[0] - '1' == stage->actID)) {
                    return i;
                }
            }
        }
    }

    return 0;
}

void Zone::GetTileInfo(int32 x, int32 y, int32 moveOffsetX, int32 moveOffsetY, int32 cPlane, RSDK::Tile *tile, uint8 *flags)
{
    RSDK::Tile tileLow  = sVars->fgLayer[0].GetTile(x >> 20, y >> 20);
    RSDK::Tile tileHigh = sVars->fgLayer[1].GetTile(x >> 20, y >> 20);

    int32 flagsLow  = tileLow.GetFlags(cPlane);
    int32 flagsHigh = tileHigh.GetFlags(cPlane);

    RSDK::Tile tileMove = 0;
    int32 flagsMove     = 0;
    if (sVars->moveLayer.Loaded()) {
        tileMove  = sVars->moveLayer.GetTile((moveOffsetX + x) >> 20, (moveOffsetY + y) >> 20);
        flagsMove = tileMove.GetFlags(cPlane);
    }

    int32 tileSolidLow  = 0;
    int32 tileSolidHigh = 0;
    int32 tileSolidMove = 0;
    if (cPlane) {
        tileSolidHigh = tileHigh.SolidB();
        tileSolidLow  = tileLow.SolidB();
    }
    else {
        tileSolidHigh = tileHigh.SolidA();
        tileSolidLow  = tileLow.SolidA();
    }

    if (sVars->moveLayer.Loaded())
        tileSolidMove = tileMove.SolidA();

    *tile  = 0;
    *flags = 0;
    if (flagsMove && tileSolidMove) {
        *tile  = tileMove;
        *flags = flagsMove;
    }
    else if (flagsHigh && tileSolidHigh) {
        *tile  = tileHigh;
        *flags = flagsHigh;
    }
    else if (flagsLow && tileSolidLow) {
        *tile  = tileLow;
        *flags = flagsLow;
    }
}

#if RETRO_INCLUDE_EDITOR
void Zone::EditorDraw() {}

void Zone::EditorLoad()
{
    int32 stageID = 0;
    if (!sceneInfo->inEditor)
        stageID = sceneInfo->listData[sceneInfo->listPos].id[0];

    sVars->folderListPos = 0;
    sVars->actID         = stageList[sVars->folderListPos].actID;
    sVars->useFolderIDs  = stageList[sVars->folderListPos].useFolderIDs;

    for (int32 i = 0; i < 0x80; ++i) {
        StageFolderInfo *stage = &stageList[i];

        if (Stage::CheckSceneFolder(stage->stageFolder)) {
            if (sceneInfo->inEditor || stage->noActID) {
                sVars->folderListPos = i;
                sVars->actID         = stage->actID;
                sVars->useFolderIDs  = stage->useFolderIDs;
                break;
            }
            else {
                for (; i < 0x80; ++i) {
                    stage = &stageList[i];
                    if ('1' + stage->actID == stageID) {
                        sVars->folderListPos = i;
                        sVars->actID         = stage->actID;
                        sVars->useFolderIDs  = stage->useFolderIDs;
                        break;
                    }
                }
            }
        }
    }
}
#endif

#if RETRO_REV0U
void Zone::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(Zone);

    sVars->sfxFail.Init();
}
#endif

void Zone::Serialize() {}

} // namespace GameLogic
