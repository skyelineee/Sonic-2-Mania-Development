// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: ItemBox Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "ItemBox.hpp"
#include "Zone.hpp"
#include "DebugMode.hpp"
#include "Music.hpp"
#include "Shield.hpp"
#include "ImageTrail.hpp"
#include "InvincibleStars.hpp"
#include "Explosion.hpp"
#include "Debris.hpp"
#include "Dust.hpp"
#include "HUD.hpp"
#include "Common/TilePlatform.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(ItemBox);

void ItemBox::Update()
{
    this->state.Run(this);

    if (this->type == ItemBox::Stock) {
        if (this->contentsAnimator.animationID == 2 || this->contentsAnimator.animationID == 7 || this->contentsAnimator.animationID == 8) {
            this->contentsAnimator.SetAnimation(sVars->aniFrames, 7, false, 0);
        }
    }
}
void ItemBox::LateUpdate() {}
void ItemBox::StaticUpdate() {}
void ItemBox::Draw()
{
    if (!this->hidden) {
        if (this->isContents) {
            if (sceneInfo->currentDrawGroup == Zone::sVars->playerDrawGroup[1]) {
                this->drawFX = FX_NONE;
                this->contentsAnimator.DrawSprite(&this->contentsPos, false);
            }
            else {
                this->drawFX    = FX_FLIP;
                this->inkEffect = INK_NONE;
                this->boxAnimator.DrawSprite(nullptr, false);

                Graphics::AddDrawListRef(Zone::sVars->playerDrawGroup[1], sceneInfo->entitySlot);
            }
        }
        else {
            this->inkEffect = INK_NONE;
            this->boxAnimator.DrawSprite(nullptr, false);
            this->contentsAnimator.DrawSprite(&this->contentsPos, false);

            if (globals->useManiaBehavior) {
                this->inkEffect = INK_ADD;
                this->overlayAnimator.DrawSprite(nullptr, false);
                this->inkEffect = INK_NONE;
            }

            this->debrisAnimator.DrawSprite(nullptr, false);
        }
    }
}

void ItemBox::Create(void *data)
{
    if (data)
        this->type = VOID_TO_INT(data);

    if (!this->state.Matches(&ItemBox::State_Done)) {
        this->boxAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
        this->contentsAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
        this->overlayAnimator.SetAnimation(sVars->aniFrames, 3, true, 0);
        this->debrisAnimator.SetAnimation(sVars->aniFrames, 4, true, 0);

        Player *player = GameObject::Get<Player>(SLOT_PLAYER1);
        switch (this->type) {
            case ItemBox::ExtraLife_Sonic:
            case ItemBox::ExtraLife_Tails:
            case ItemBox::ExtraLife_Knux:
                if (globals->gameMode == MODE_TIMEATTACK) {
                    this->type = ItemBox::Ring;
                }
                else {
                    switch (player->characterID) {
                        case ID_SONIC: this->type = ItemBox::ExtraLife_Sonic; break;
                        case ID_TAILS: this->type = ItemBox::ExtraLife_Tails; break;
                        case ID_KNUCKLES: this->type = ItemBox::ExtraLife_Knux; break;
                        default: break;
                    }
                }
                this->contentsAnimator.frameID = this->type;
                break;

            case ItemBox::Swap:
                this->Destroy();
                break;

            case ItemBox::Random:
                if (globals->secrets & SECRET_RANDOMITEMS) 
                    this->contentsAnimator.frameID = ItemBox::Random;
                else
                    this->Destroy();
                break;

            default: this->contentsAnimator.frameID = this->type; break;
        }
    }

    this->drawFX = FX_FLIP;
    if (!sceneInfo->inEditor) {
        this->direction *= FLIP_Y;
        this->active        = ACTIVE_BOUNDS;
        this->updateRange.x = 0x400000;
        this->updateRange.y = 0x400000;
        this->visible       = true;
        if (this->planeFilter > 0 && ((uint8)this->planeFilter - 1) & 2)
            this->drawGroup = Zone::sVars->objectDrawGroup[1];
        else
            this->drawGroup = Zone::sVars->objectDrawGroup[0];

        this->alpha = 0xFF;
        if (this->state.Matches(&ItemBox::State_Done)) {
            this->boxAnimator.SetAnimation(sVars->aniFrames, 1, true, this->boxAnimator.frameID);
        }
        else if (this->hidden) {
            this->state.Set(nullptr);
        }
        else if (this->isFalling) {
            this->active = ACTIVE_NORMAL;
            this->state.Set(&ItemBox::State_Falling);
        }
        else {
            this->state.Set(&ItemBox::State_Idle);
        }
    }
}

void ItemBox::StageLoad()
{
    
    sVars->aniFrames.Load("Global/ItemBox.bin", SCOPE_STAGE);

    sVars->hitboxItemBox.left   = -15;
    sVars->hitboxItemBox.top    = -14;
    sVars->hitboxItemBox.right  = 15;
    sVars->hitboxItemBox.bottom = 16;

    sVars->hitboxHidden.left   = -15;
    sVars->hitboxHidden.top    = -22;
    sVars->hitboxHidden.right  = 15;
    sVars->hitboxHidden.bottom = 16;

    DebugMode::AddObject(sVars->classID, &ItemBox::DebugSpawn, &ItemBox::DebugDraw);

    if (globals->secrets & SECRET_RANDOMITEMS) {
        for (auto itemBox : GameObject::GetEntities<ItemBox>(FOR_ALL_ENTITIES)) {
            itemBox->type = ItemBox::Random;
        }
    }
    else if (globals->secrets & SECRET_BLUESHIELDMODE) {
        for (auto itemBox : GameObject::GetEntities<ItemBox>(FOR_ALL_ENTITIES)) {
            if (itemBox->type == ItemBox::BubbleShield || itemBox->type == ItemBox::FireShield || itemBox->type == ItemBox::LightningShield)
            itemBox->type = ItemBox::BlueShield;
        }
    }

    if (globals->secrets & SECRET_NOITEMS) {
        for (auto itemBox : GameObject::GetEntities<ItemBox>(FOR_ALL_ENTITIES)) {
            itemBox->Destroy();
        }
    }

    sVars->sfxDestroy.Get("Global/Destroy.wav");
    sVars->sfxTeleport.Get("Global/Teleport.wav");
    sVars->sfxHyperRing.Get("Global/HyperRing.wav");
    sVars->sfxPowerDown.Get("Stage/PowerDown.wav");
    sVars->sfxRecovery.Get("Global/Recovery.wav");
}

void ItemBox::DebugDraw()
{
    DebugMode::sVars->itemTypeCount = 16;

    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.DrawSprite(nullptr, false);

    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 2, true, DebugMode::sVars->itemType);

    Vector2 drawPos;
    drawPos.x = this->position.x;
    drawPos.y = this->position.y - 0x30000;
    DebugMode::sVars->animator.DrawSprite(&drawPos, false);
}
void ItemBox::DebugSpawn()
{
    ItemBox *itemBox                  = GameObject::Create<ItemBox>(nullptr, this->position.x, this->position.y);
    itemBox->type                     = DebugMode::sVars->itemType;
    itemBox->contentsAnimator.frameID = DebugMode::sVars->itemType;
}

void ItemBox::State_Done()
{
    SET_CURRENT_STATE();

    ItemBox::HandleFallingCollision();
}
void ItemBox::State_Break()
{
    SET_CURRENT_STATE();

    ItemBox::HandleFallingCollision();

    if (this->contentsSpeed < 0) {
        this->contentsPos.y += this->contentsSpeed;
        this->contentsSpeed += 0x1800;
    }

    if (this->contentsSpeed >= 0) {
        this->contentsSpeed = 0;

        GivePowerup();

        this->contentsAnimator.SetAnimation(sVars->aniFrames, 5, true, 0);
        this->state.Set(&ItemBox::State_IconFinish);
    }
}

void ItemBox::State_IconFinish()
{
    SET_CURRENT_STATE();

    ItemBox::HandleFallingCollision();

    this->contentsAnimator.Process();

    if (this->contentsAnimator.frameID == this->contentsAnimator.frameCount - 1) {
        this->contentsAnimator.SetAnimation(nullptr, 0, true, 0);
        this->state.Set(&ItemBox::State_Done);
    }
}

void ItemBox::State_Idle()
{
    SET_CURRENT_STATE();

    this->contentsPos.x = this->position.x;

    if (this->direction == FLIP_NONE)
        this->contentsPos.y = this->position.y - 0x30000;
    else
        this->contentsPos.y = this->position.y + 0x30000;

    ItemBox::HandleObjectCollisions();
    ItemBox::CheckHit();

    this->overlayAnimator.Process();

    if (this->type == ItemBox::Stock) {
        this->contentsAnimator.Process();
    }

    if (this->timer) {
        this->timer--;
    }
    else {
        this->debrisAnimator.Process();

        if (!this->debrisAnimator.frameID) {
            this->timer                        = Math::Rand(1, 15);
            this->debrisAnimator.frameDuration = Math::Rand(1, 32);
        }
    }
}
void ItemBox::State_Falling()
{
    SET_CURRENT_STATE();

    if (ItemBox::HandleFallingCollision())
        this->state.Set(&ItemBox::State_Idle);

    this->contentsPos.x = this->position.x;

    if (this->direction == FLIP_NONE)
        this->contentsPos.y = this->position.y - 0x30000;
    else
        this->contentsPos.y = this->position.y + 0x30000;


    ItemBox::CheckHit();

    this->overlayAnimator.Process();

    if (this->type == ItemBox::Stock) {
        this->contentsAnimator.Process();
    }

    if (this->timer) {
        this->timer--;
    }
    else {
        this->debrisAnimator.Process();

        if (!this->debrisAnimator.frameID) {
            this->timer                        = Math::Rand(1, 15);
            this->debrisAnimator.frameDuration = Math::Rand(1, 32);
        }
    }
}

void ItemBox::CheckHit()
{
    for (auto player : GameObject::GetEntities<Player>(FOR_ACTIVE_ENTITIES))
    {
        if (this->planeFilter <= 0 || player->collisionPlane == (((uint8)this->planeFilter - 1) & 1)) {

            int32 anim = player->animator.animationID;
            bool32 attacking = false;

            switch (globals->gravityDir) {
                default: break;
                case CMODE_FLOOR: attacking |= player->velocity.y >= 0; break;
                case CMODE_ROOF: attacking |= player->velocity.y <= 0; break;
            }

            attacking |= player->onGround;
            attacking |= this->direction;

            attacking &= anim == Player::ANI_JUMP;
            switch (player->characterID) {
                case ID_SONIC: attacking |= anim == Player::ANI_DROPDASH; break;
                case ID_KNUCKLES: attacking |= anim == Player::ANI_GLIDE || anim == Player::ANI_GLIDE_SLIDE; break;
            }

            if (attacking && !player->sidekick) {
                if (player->CheckBadnikTouch(this, &sVars->hitboxItemBox)) {
                    this->Break(player);
                    break;
                }
            }
            else {
                this->position.x -= this->moveOffset.x;
                this->position.y -= this->moveOffset.y;
                int32 px = player->position.x;
                int32 py = player->position.y;

                uint8 side = player->CheckCollisionBox(this, &sVars->hitboxItemBox);

                player->position.x = px;
                player->position.y = py;
                this->position.x += this->moveOffset.x;
                this->position.y += this->moveOffset.y;

                if (side == C_BOTTOM) {
                    this->active = ACTIVE_ALWAYS;
                    this->state.Set(&ItemBox::State_Falling);
                    this->velocity.y = -0x20000;

                    if (!player->onGround)
                        player->velocity.y = 0x20000;
                }
                else if (side == C_TOP) {
                    player->position.x += this->moveOffset.x;
                    player->position.y += this->moveOffset.y;
                }

                if (player->CheckCollisionBox(this, &sVars->hitboxItemBox) == C_BOTTOM) {
                    if (player->onGround) {
                        player->position.x = px;
                        player->position.y = py;
                    }
                }
            }
        }
    }
}
void ItemBox::GivePowerup()
{
    Player *player = (Player*)this->storedEntity;

    switch (this->type) {
        case ItemBox::Ring: player->GiveRings(10, true); break;

        case ItemBox::BlueShield:
            player->shield = Shield::Blue;
            player->ApplyShield();
            Shield::sVars->sfxBlueShield.Play();
            break;

        case ItemBox::BubbleShield:
            player->shield = Shield::Bubble;
            player->ApplyShield();
            Shield::sVars->sfxBubbleShield.Play();
            player->drownTimer = 0;
            Music::JingleFadeOut(Music::TRACK_DROWNING, false);

            NotifyCallback(NOTIFY_STATS_PARAM_1, 0, 0, 1);
            break;

        case ItemBox::FireShield:
            player->shield = Shield::Fire;
            player->ApplyShield();
            Shield::sVars->sfxFireShield.Play();

            NotifyCallback(NOTIFY_STATS_PARAM_1, 1, 0, 0);
            break;

        case ItemBox::LightningShield:
            player->shield = Shield::Lightning;
            player->ApplyShield();
            Shield::sVars->sfxLightningShield.Play();

            NotifyCallback(NOTIFY_STATS_PARAM_1, 0, 1, 0);
            break;

        case ItemBox::Invincible:
            if (player->superState == Player::SuperStateNone) {
                InvincibleStars *invincibleStars = GameObject::Get<InvincibleStars>(Player::sVars->maxPlayerCount + player->Slot());
                invincibleStars->Reset(InvincibleStars::sVars->classID, player);
                player->invincibleTimer = 1260;
                Music::PlayJingle(Music::TRACK_INVINCIBLE);
            }
            break;

        case ItemBox::Sneaker:
            player->speedShoesTimer = 1320;
            player->UpdatePhysicsState();

            if (player->superState == Player::SuperStateNone) {
                if (globals->ostStyle == GAME_SM)
                    Music::PlayJingle(Music::TRACK_SNEAKERS);
                else
                    Music::sVars->musicSpeedUp.Run(nullptr);

                if (globals->useManiaBehavior) {
                    ImageTrail *powerup = GameObject::Get<ImageTrail>(2 * Player::sVars->maxPlayerCount + player->Slot());
                    powerup->Reset(ImageTrail::sVars->classID, player);
                }
            }
            break;

        case ItemBox::ExtraLife_Sonic:
        case ItemBox::ExtraLife_Tails:
        case ItemBox::ExtraLife_Knux:
            player->GiveLife();
            break;

        case ItemBox::Eggman: 
            player->Hurt(this); 
            break;

        case ItemBox::HyperRing:
            sVars->sfxHyperRing.Play();
            player->hyperRing = true;
            break;

        case ItemBox::Swap:       
            Player::sVars->sfxSwapFail.Play();
            break;

        case ItemBox::Random: {
            uint8 playerIDs[5]    = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
            uint8 newPlayerIDs[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

            if (player->animator.animationID == Player::ANI_TRANSFORM) {
                Player::sVars->sfxSwapFail.Play();
            }
            else {
                Player *player1 = GameObject::Get<Player>(SLOT_PLAYER1);
                Player *player2 = GameObject::Get<Player>(SLOT_PLAYER2);

                int32 charID = -1;
                for (int32 i = player1->characterID; i > 0; ++charID) i >>= 1;
                playerIDs[0] = charID;

                charID = -1;
                for (int32 i = player2->characterID; i > 0; ++charID) i >>= 1;
                playerIDs[1] = charID;

                if (playerIDs[1] == 0xFF) {
                    Player::sVars->sfxSwapFail.Play();
                }
                else {
                    for (int32 i = 0; i < 3; ++i) {
                        if (globals->stock & (0xFF << (8 * i))) {
                            int32 characterID = (globals->stock >> (8 * i)) & 0xFF;

                            playerIDs[i + 2] = -1;
                            for (int32 c = characterID; c > 0; ++playerIDs[i + 2]) c >>= 1;
                        }
                    }

                    globals->stock = 0;

                    int32 tempStock = 0;
                    int32 p         = 0;
                    for (; p < 5;) {
                        bool32 inc = true;
                        if (playerIDs[p] == 0xFF)
                            break;

                        newPlayerIDs[p] = Math::Rand(0, 5);
                        if ((1 << newPlayerIDs[p]) & globals->characterFlags) {
                            while (true) {
                                if (!((1 << newPlayerIDs[p]) & tempStock)) {
                                    if (newPlayerIDs[p] != playerIDs[p]) {
                                        tempStock |= 1 << newPlayerIDs[p];
                                        break;
                                    }
                                    else if (p == 4 || playerIDs[p + 1] == 0xFF) {
                                        int32 slot         = Math::Rand(0, p);
                                        int32 id           = newPlayerIDs[slot];
                                        newPlayerIDs[slot] = newPlayerIDs[p];
                                        newPlayerIDs[p]    = id;
                                        tempStock |= 1 << newPlayerIDs[slot];
                                        tempStock |= 1 << newPlayerIDs[p];
                                        break;
                                    }
                                    else {
                                        newPlayerIDs[p] = Math::Rand(0, 5);
                                        if (!((1 << newPlayerIDs[p]) & globals->characterFlags)) {
                                            inc = false;
                                            break;
                                        }
                                    }
                                }
                                else {
                                    newPlayerIDs[p] = Math::Rand(0, 5);
                                    if (!((1 << newPlayerIDs[p]) & globals->characterFlags)) {
                                        inc = false;
                                        break;
                                    }
                                }
                            }

                            if (inc)
                                ++p;
                        }
                    }

                    for (int32 i = 0; i < p; ++i) {
                        switch (i) {
                            case 0: player1->ChangeCharacter(1 << newPlayerIDs[0]); break;
                            case 1: player2->ChangeCharacter(1 << newPlayerIDs[1]); break;
                            default:
                                globals->stock <<= 8;
                                globals->stock |= 1 << newPlayerIDs[i];
                                break;
                        }
                    }

                    Explosion *explosion            = GameObject::Create<Explosion>(0, player1->position.x, player1->position.y);
                    explosion->drawGroup = Zone::sVars->objectDrawGroup[1];

                    explosion            = GameObject::Create<Explosion>(0, player2->position.x, player2->position.y);
                    explosion->drawGroup = Zone::sVars->objectDrawGroup[1];

                    sVars->sfxPowerDown.Play();
                }
            }
            break;
        }

        case ItemBox::Super:
            player->GiveRings(50, false);
            player->TryTransform(false, Player::TransformSuper);
            break;

        case ItemBox::Hyper:
            player->GiveRings(50, false);
            player->TryTransform(false, Player::TransformHyper);
        break;

        case ItemBox::Stock: {
            if (this->contentsAnimator.animationID == 7) {      
                switch (this->contentsAnimator.frameID) {
                    case 0: player->ChangeCharacter(ID_SONIC); break;
                    case 1: player->ChangeCharacter(ID_TAILS); break;
                    case 2: player->ChangeCharacter(ID_KNUCKLES); break;
                    default: break;
                }

                Explosion *explosion = GameObject::Create<Explosion>(0, player->position.x, player->position.y);
                explosion->drawGroup = Zone::sVars->objectDrawGroup[1];
                sVars->sfxPowerDown.Play();
            }
            else {
                switch (this->contentsAnimator.frameID) {
                    case 1: this->type = ItemBox::BlueShield; break;
                    case 2: this->type = ItemBox::BubbleShield; break;
                    case 3: this->type = ItemBox::FireShield; break;
                    case 4: this->type = ItemBox::LightningShield; break;
                    case 5: this->type = ItemBox::HyperRing; break;
                    case 6: this->type = ItemBox::Swap; break;
                    case 7: this->type = ItemBox::Random; break;
                    default: this->type = ItemBox::Ring; break;
                }

                player = (Player*)this->parent;
                if ((uint32)this->type <= ItemBox::Stock)
                    ItemBox::GivePowerup();
            }
            break;
        }
        default: break;
    }
}
void ItemBox::Break(Player *player)
{
    GameObject::Create(nullptr, this->position.x, this->position.y);

    switch (globals->gravityDir) {
        default: break;

        case CMODE_FLOOR:
                player->velocity.y = -(player->velocity.y + 2 * player->gravityStrength);
            break;

        case CMODE_ROOF:
                player->velocity.y = player->velocity.y + 2 * player->gravityStrength;
            break;
    }

    this->storedEntity  = (Entity *)player;
    this->alpha         = 0x100;
    this->contentsSpeed = -0x30000;
    this->active        = ACTIVE_NORMAL;

    switch (globals->gravityDir) {
        default: break;
        case CMODE_FLOOR: this->velocity.y = -0x20000; break;
        case CMODE_ROOF: this->velocity.y = -0x20000; break;
    }
    this->isContents    = true;
    this->state         .Set(&ItemBox::State_Break);
    this->boxAnimator.SetAnimation(sVars->aniFrames, 1, true, 0);
    this->boxAnimator.frameID = sVars->brokenFrame++;
    sVars->brokenFrame %= 3;
    this->overlayAnimator.SetAnimation(nullptr, 0, true, 0);
    this->debrisAnimator.SetAnimation(nullptr, 0, true, 0);

    int32 y = this->position.y - 0x100000;
    Explosion *explosion = GameObject::Create<Explosion>(0, this->position.x, y);
    explosion->drawGroup       = Zone::sVars->objectDrawGroup[1];

    for (int32 d = 0; d < 6; ++d) {
        Debris *debris = GameObject::Create<Debris>(Debris::Fall, this->position.x + Math::Rand(-0x80000, 0x80000),
                                                    this->position.y + Math::Rand(-0x80000, 0x80000));

        switch (globals->gravityDir) {
            default: break;

            case CMODE_FLOOR:
                debris->gravityStrength = 0x4000;
                debris->velocity.x      = Math::Rand(0, 0x20000);
                if (debris->position.x < this->position.x)
                    debris->velocity.x = -debris->velocity.x;
                debris->velocity.y = Math::Rand(-0x40000, -0x10000);
                break;

            case CMODE_ROOF:
                debris->gravityStrength = -0x4000;
                debris->velocity.x      = Math::Rand(0, 0x20000);
                if (debris->position.x < this->position.x)
                    debris->velocity.x = -debris->velocity.x;
                debris->velocity.y = -Math::Rand(-0x40000, -0x10000);
                break;
        }

        debris->drawFX     = FX_FLIP;
        debris->direction  = d & 3;
        debris->drawGroup  = Zone::sVars->objectDrawGroup[1];
        debris->animator1.SetAnimation(sVars->aniFrames, 6, true, Math::Rand(0, 4));
    }

    sVars->sfxDestroy.Play();

    this->active = ACTIVE_NORMAL;
    if (this->type == ItemBox::Random) {
        while (true) {
            this->type = Math::Rand(0, 13);
            switch (this->type) {
                case ItemBox::BubbleShield:
                case ItemBox::FireShield:
                case ItemBox::LightningShield:
                    if (globals->secrets & SECRET_BLUESHIELDMODE)
                        this->type = ItemBox::BlueShield;
                    break;

                case ItemBox::ExtraLife_Sonic:
                    if (globals->gameMode == MODE_TIMEATTACK)
                        continue;

                    switch (player->characterID) {
                        case ID_SONIC: this->type = ItemBox::ExtraLife_Sonic; break;
                        case ID_TAILS: this->type = ItemBox::ExtraLife_Tails; break;
                        case ID_KNUCKLES: this->type = ItemBox::ExtraLife_Knux; break;
                        default: break;
                    }
                    this->contentsAnimator.frameID = this->type;
                    break;

                case ItemBox::ExtraLife_Tails:
                case ItemBox::ExtraLife_Knux:
                    continue;

                case ItemBox::Swap:
                    break;

                default: this->contentsAnimator.frameID = this->type; break;
            }
            break;
        }
    }
}
bool32 ItemBox::HandleFallingCollision()
{
    if (this->direction)
        return false;

    this->moveOffset.x = -this->position.x;
    this->moveOffset.y = -this->position.y;
    if (this->velocity.y)
        this->parent = nullptr;

    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    switch (globals->gravityDir) {
        default: break;
        case CMODE_FLOOR: this->velocity.y += 0x3800; break;
        case CMODE_ROOF: this->velocity.y -= 0x3800; break;
    }

    ItemBox::HandleObjectCollisions();

    switch (globals->gravityDir) {
        default: break;
        case CMODE_FLOOR:
            if (this->velocity.y >= 0
                && (this->direction == FLIP_Y && this->boxAnimator.animationID == 1
                        ? this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0, true)
                        : this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, 0, 0, 0x100000, true))) {
                this->velocity.y = 0;
                if (!this->state.Matches(&ItemBox::State_IconFinish) && !this->state.Matches(&ItemBox::State_Break))
                    this->active = ACTIVE_BOUNDS;
                this->moveOffset.x += this->position.x;
                this->moveOffset.y += this->position.y;
                return true;
            }
            break;

        case CMODE_ROOF:
            if (this->velocity.y <= 0
                && (this->direction == FLIP_Y && this->boxAnimator.animationID == 1
                        ? this->TileCollision(Zone::sVars->collisionLayers, CMODE_ROOF, 0, 0, 0, true)
                        : this->TileCollision(Zone::sVars->collisionLayers, CMODE_ROOF, 0, 0, -0x100000, true))) {
                this->velocity.y = 0;
                if (!this->state.Matches(&ItemBox::State_IconFinish) && !this->state.Matches(&ItemBox::State_Break))
                    this->active = ACTIVE_BOUNDS;
                this->moveOffset.x += this->position.x;
                this->moveOffset.y += this->position.y;
                return true;
            }
            break;
    }

    this->moveOffset.x += this->position.x;
    this->moveOffset.y += this->position.y;
    return false;
}
bool32 ItemBox::HandlePlatformCollision(Platform *platform)
{    
    bool32 collided = false;
    if (!platform->state.Matches(&Platform::State_Falling2) && !platform->state.Matches(&Platform::State_Hold)) {
        platform->position.x = platform->drawPos.x - platform->collisionOffset.x;
        platform->position.y = platform->drawPos.y - platform->collisionOffset.y;

        switch (platform->collision) {
            default: break;

            case Platform::C_Solid:
            case Platform::C_SolidHurtSides:
            case Platform::C_SolidHurtBottom:
            case Platform::C_SolidHurtTop:
            case Platform::C_SolidHold:
            case Platform::C_SolidSticky:
            case Platform::C_StickyTop:
            case Platform::C_StickyLeft:
            case Platform::C_StickyRight:
            case Platform::C_StickyBottom:
            case Platform::C_SolidBarrel:
            case Platform::C_SolidNoCrush:
            case Platform::C_SolidHurtAll:
            case Platform::C_SolidHurtNoCrush:
                collided = platform->CheckCollisionBox(platform->animator.GetHitbox(1), this, &sVars->hitboxItemBox, true);
                break;

            case Platform::C_Platform:
                collided = platform->CheckCollisionPlatform(platform->animator.GetHitbox(0), this, &sVars->hitboxItemBox, true);
                break;

            case Platform::C_Tiled:
                if (platform->CheckCollisionTouchBox(&platform->hitbox, this, &sVars->hitboxItemBox)) {
                    if (this->collisionLayers & Zone::sVars->moveLayerMask) {
                        TileLayer *move  = Zone::sVars->moveLayer.GetTileLayer();
                        move->position.x = -(platform->drawPos.x + platform->tileOrigin.x) >> 16;
                        move->position.y = -(platform->drawPos.y + platform->tileOrigin.y) >> 16;
                    }

                    if (!this->state.Matches(&ItemBox::State_Idle)) {
                        switch (globals->gravityDir) {
                            default: break;

                            case CMODE_FLOOR:
                                if (this->velocity.y >= 0x3800)
                                    collided = true;
                                break;

                            case CMODE_ROOF:
                                if (this->velocity.y <= 0x3800)
                                    collided = true;
                                break;
                        }
                    }
                }
                break;
        }

        if (!collided) {
            platform->position.x = platform->centerPos.x;
            platform->position.y = platform->centerPos.y;
            return false;
        }

        this->parent = platform;

        this->scale.x = (this->position.x - platform->drawPos.x) & 0xFFFF0000;
        this->scale.y = (this->position.y - platform->drawPos.y) & 0xFFFF0000;

        this->updateRange.x = platform->updateRange.x;
        this->updateRange.y = platform->updateRange.y;

        if (this->state.Matches(&ItemBox::State_Falling))
            this->state.Set(&ItemBox::State_Idle);

        if (platform->state.Matches(&Platform::State_Fall) && !platform->timer)
            platform->timer = 30;

        this->velocity.y     = 0;
        platform->stood      = true;
        platform->position.x = platform->centerPos.x;
        platform->position.y = platform->centerPos.y;
    }

    return collided;
}
void ItemBox::HandleObjectCollisions()
{
    bool32 platformCollided = false;

    if (Platform::sVars) {
        if (this->parent) {
            Platform *platform = (Platform *)this->parent;
    
            if (platform->classID == Platform::sVars->classID) {
                platform->stood    = true;
                this->position.x   = this->scale.x + platform->drawPos.x;
                this->position.y   = (this->scale.y + platform->drawPos.y) & 0xFFFF0000;
                this->moveOffset.x = platform->collisionOffset.x & 0xFFFF0000;
                this->moveOffset.y = platform->collisionOffset.y & 0xFFFF0000;
                this->contentsPos.x += platform->collisionOffset.x;
                this->contentsPos.y += platform->collisionOffset.y;
                this->velocity.y = 0;
                platformCollided = true;
            }
        }
        else {
            for (auto platform : GameObject::GetEntities<Platform>(FOR_ACTIVE_ENTITIES)) {
                if (ItemBox::HandlePlatformCollision(platform))
                    platformCollided = true;
            }
        }
    }

    if (TilePlatform::sVars && this->parent) {
        TilePlatform *platform = (TilePlatform *)this->parent;

        if (platform->classID == TilePlatform::sVars->classID) {
            platform->stood    = true;
            this->position.x   = this->scale.x + platform->drawPos.x;
            this->position.y   = (this->scale.y + platform->drawPos.y) & 0xFFFF0000;
            this->moveOffset.x = platform->collisionOffset.x & 0xFFFF0000;
            this->moveOffset.y = platform->collisionOffset.y & 0xFFFF0000;
            this->contentsPos.x += platform->collisionOffset.x;
            this->contentsPos.y += platform->collisionOffset.y;
            this->velocity.y = 0;
            platformCollided = true;
        }
    }

    if (!platformCollided)
        this->parent = nullptr;

}

#if RETRO_INCLUDE_EDITOR
void ItemBox::EditorDraw()
{
    this->direction *= FLIP_Y;

    this->boxAnimator.SetAnimation(sVars->aniFrames, 0, true, 0);
    this->boxAnimator.DrawSprite(nullptr, false);

    this->contentsPos.x = this->position.x;
    if (this->direction)
        this->contentsPos.y = this->position.y + 0x50000;
    else
        this->contentsPos.y = this->position.y + -0x50000;

    this->contentsAnimator.SetAnimation(sVars->aniFrames, 2, true, 0);
    this->contentsAnimator.frameID = this->type - this->type / this->contentsAnimator.frameCount * this->contentsAnimator.frameCount;
    this->contentsAnimator.DrawSprite(&this->contentsPos, false);

    this->direction >>= FLIP_X;
}

void ItemBox::EditorLoad()
{
    sVars->aniFrames.Load("Global/ItemBox.bin", SCOPE_STAGE);

    RSDK_ACTIVE_VAR(sVars, type);
    RSDK_ENUM_VAR("Super Ring");
    RSDK_ENUM_VAR("Blue Shield");
    RSDK_ENUM_VAR("Bubble Shield");
    RSDK_ENUM_VAR("Fire Shield");
    RSDK_ENUM_VAR("Lightning Shield");
    RSDK_ENUM_VAR("Invincible");
    RSDK_ENUM_VAR("Sneakers");
    RSDK_ENUM_VAR("1UP Sonic");
    RSDK_ENUM_VAR("1UP Tails");
    RSDK_ENUM_VAR("1UP Knuckles");
    RSDK_ENUM_VAR("Eggman");
    RSDK_ENUM_VAR("Hyper Ring");
    RSDK_ENUM_VAR("Swap");
    RSDK_ENUM_VAR("Random");
    RSDK_ENUM_VAR("Super");
    RSDK_ENUM_VAR("Change");

    RSDK_ACTIVE_VAR(sVars, isFalling);
    RSDK_ENUM_VAR("false");
    RSDK_ENUM_VAR("true");

    RSDK_ACTIVE_VAR(sVars, direction);
    RSDK_ENUM_VAR("Normal");
    RSDK_ENUM_VAR("Upside-Down");

    RSDK_ACTIVE_VAR(sVars, planeFilter);
    RSDK_ENUM_VAR("All");
    RSDK_ENUM_VAR("AL");
    RSDK_ENUM_VAR("BL");
    RSDK_ENUM_VAR("AH");
    RSDK_ENUM_VAR("BH");
}
#endif

#if RETRO_REV0U
void ItemBox::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(ItemBox);

    sVars->aniFrames.Init();

    sVars->sfxDestroy.Init();
    sVars->sfxTeleport.Init();
    sVars->sfxHyperRing.Init();
    sVars->sfxPowerDown.Init();
    sVars->sfxRecovery.Init();
}
#endif

void ItemBox::Serialize()
{
    RSDK_EDITABLE_VAR(ItemBox, VAR_INT32, type);
    RSDK_EDITABLE_VAR(ItemBox, VAR_BOOL, isFalling);
    RSDK_EDITABLE_VAR(ItemBox, VAR_BOOL, hidden);
    RSDK_EDITABLE_VAR(ItemBox, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(ItemBox, VAR_INT32, planeFilter);
}

} // namespace GameLogic
