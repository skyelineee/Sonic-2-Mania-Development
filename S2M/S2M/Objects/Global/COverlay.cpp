// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: COverlay Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "COverlay.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "DebugMode.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(COverlay);

void COverlay::Update() { }
void COverlay::LateUpdate() {}
void COverlay::StaticUpdate() {}
void COverlay::Draw()
{
    int32 tileX = 0;
    for (this->position.x = (screenInfo->position.x & 0xFFFFFFF0) << 16; tileX < (screenInfo->size.x >> 4) + 2; ++tileX) {
        int32 tileY = 0;
        for (this->position.y = (screenInfo->position.y & 0xFFFFFFF0) << 16; tileY < (screenInfo->size.y >> 4) + 2; ++tileY) {
            DrawTile();
            this->position.y += 16 << 0x10;
        }
        this->position.x += 16 << 0x10;
    }
}

void COverlay::Create(void *data)
{
    if (!sceneInfo->inEditor) {
        this->active    = ACTIVE_ALWAYS;
        this->visible   = true;
        this->drawGroup = Zone::sVars->objectDrawGroup[1];
    }
}

void COverlay::StageLoad()
{
    sVars->aniFrames.Load("Global/PlaneSwitch.bin", SCOPE_STAGE);
    
    DebugMode::AddObject(sVars->classID, &COverlay::DebugSpawn, &COverlay::DebugDraw);
}

void COverlay::DebugDraw()
{
    DebugMode::sVars->animator.SetAnimation(sVars->aniFrames, 0, true, 0);
    DebugMode::sVars->animator.frameID = 4;

    DebugMode::sVars->animator.DrawSprite(nullptr, false);
}

void COverlay::DebugSpawn()
{
    int32 count = 0;
    for (auto overlay : GameObject::GetEntities<COverlay>(FOR_ALL_ENTITIES)) {
        overlay->Destroy();
        ++count;
    }

    if (!count)
        GameObject::Create<COverlay>(nullptr, this->position.x, this->position.y);
}

void COverlay::DrawTile()
{
    Player *player = GameObject::Get<Player>(SLOT_PLAYER1);
    int32 tx             = 0;
    for (int32 x = 0; x < 0x10; ++x) {
        uint8 ty    = -1;
        uint8 th2   = -1;
        uint8 ty2   = -1;
        uint8 th    = -1;
        uint8 solid = 0;
        for (int32 y = 0; y < 0x10; ++y) {
            if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_FLOOR, player->collisionPlane, x << 0x10, y << 0x10, false)) {
                solid |= 1;
                th2 = y + 1;
                if (ty == 0xFF)
                    ty = y;
            }
            
            if (this->TileCollision(Zone::sVars->collisionLayers, CMODE_ROOF, player->collisionPlane, x << 0x10, y << 0x10, false)) {
                solid |= 2;
                th = y + 1;
                if (ty2 == 0xFF)
                    ty2 = y - 1;
            }
        }

        if ((ty <= ty2 || ty == 0xFF) && ty2 != 0xFF)
            ty = ty2;
        if (th2 < th)
            th = th2;

        if (ty != 0xFF) {
            switch (solid) {
                default: break;
            
                case 1:
                    Graphics::DrawLine(this->position.x + tx, this->position.y + (ty << 16), this->position.x + tx, this->position.y + (th << 16),
                                  0xE0E000, 0xFF, INK_NONE, false);
                    break;
            
                case 2:
                    Graphics::DrawLine(this->position.x + tx, this->position.y + (ty << 16), this->position.x + tx, this->position.y + (th << 16),
                                  0xE00000, 0xFF, INK_NONE, false);
                    break;
            
                case 3:
                    Graphics::DrawLine(this->position.x + tx, this->position.y + (ty << 16), this->position.x + tx, this->position.y + (th << 16),
                                  0xE0E0E0, 0xFF, INK_NONE, false);
                    break;
            }
        }

        tx += 0x10000;
    }
}

#if RETRO_INCLUDE_EDITOR
void COverlay::EditorDraw() { this->animator.DrawSprite(&this->position, false); }

void COverlay::EditorLoad() {  }
#endif

#if RETRO_REV0U
void COverlay::StaticLoad(Static *sVars)
{
    RSDK_INIT_STATIC_VARS(COverlay);
    
    sVars->aniFrames.Init();
}
#endif

void COverlay::Serialize() {}

} // namespace GameLogic