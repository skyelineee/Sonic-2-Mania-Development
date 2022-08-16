// ---------------------------------------------------------------------
// RSDK Project: Sonic 2 Mania
// Object Description: BadnikHelpers Object
// Object Author: Ducky
// ---------------------------------------------------------------------

#include "BadnikHelpers.hpp"
#include "Global/Zone.hpp"
#include "Global/Animals.hpp"
#include "Global/Explosion.hpp"

using namespace RSDK;

namespace GameLogic
{
RSDK_REGISTER_OBJECT(BadnikHelpers);

void BadnikHelpers::BadnikBreak(RSDK::GameObject::Entity *badnik, bool32 destroy, bool32 spawnAnimals)
{
    if (spawnAnimals) {
        int32 type = Math::RandSeeded(0, 32, &Zone::sVars->randSeed);
        GameObject::Create<Animals>(Animals::sVars->animalTypes[type >> 4] + 1, badnik->position.x, badnik->position.y);
    }

    GameObject::Create<Explosion>(Explosion::Type1, badnik->position.x, badnik->position.y)->drawGroup = Zone::sVars->objectDrawGroup[1];
    Explosion::sVars->sfxDestroy.Play();

    if (destroy)
        badnik->Destroy();
}
void BadnikHelpers::BadnikBreakUnseeded(RSDK::GameObject::Entity *badnik, bool32 destroy, bool32 spawnAnimals)
{
    if (spawnAnimals) {
        int32 type = Math::Rand(0, 32);
        GameObject::Create<Animals>(Animals::sVars->animalTypes[type >> 4] + 1, badnik->position.x, badnik->position.y);
    }

    GameObject::Create<Explosion>(Explosion::Type1, badnik->position.x, badnik->position.y)->drawGroup = Zone::sVars->objectDrawGroup[1];
    Explosion::sVars->sfxDestroy.Play();

    if (destroy)
        badnik->Destroy();
}
int32 BadnikHelpers::Oscillate(RSDK::GameObject::Entity *self, int32 origin, int32 speed, int32 amplitude)
{
    self->angle = (self->angle + speed) & 0xFF;
    return ((Math::Sin256(self->angle) << amplitude) + origin) & 0xFFFF0000;
}

#if RETRO_REV0U
void BadnikHelpers::StaticLoad(Static *sVars) { RSDK_INIT_STATIC_VARS(BadnikHelpers); }
#endif

} // namespace GameLogic