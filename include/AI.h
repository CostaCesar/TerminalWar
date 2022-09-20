#include "const.h"
#include "unit.h"
#include "map.h"
#include "combat.h"

int get_BestMatchup(B_Unit *unit, B_Unit *foes, int nFoes, B_Map *map)
{
    int bestAdvtg = -100, posibAdvtg = -100, index = NO_UNIT;
    B_Pos foe_Pos;
    bool ranged = false;
    // Checking if units exists
    if((unit && nFoes < 1) || (unit->position.X < 0 || unit->position.Y < 0))
        return FUNCTION_FAIL;

    for(int i = 0; i < nFoes; i++)
    {
        if(foes[i].position.X < 0 || foes[i].position.Y < 0)
            continue;

        if(autoMove(map, 
            &map->tiles[unit->position.Y][unit->position.X],
            &map->tiles[foes[i].position.Y][foes[i].position.X]) == NULL)
            continue;

        if(unit->ammo > 0 && unit->range > 0)
        {
            posibAdvtg = get_BonusByClass(unit->type, foes[i].type, true)
            // + get_BonusByHeight(map->tiles[unit->position.Y][unit->position.X].elevation -
            // map->tiles[foes[i].position.Y][foes[i].position.X].elevation);
            + get_BonusByVeget(map->tiles[unit->position.Y][unit->position.X].vegetation);
        }
        posibAdvtg = get_BonusByClass(unit->type, foes[i].type, false);
        // + get_BonusByHeight(map->tiles[unit->position.Y][unit->position.X].elevation -
        // map->tiles[foes[i].position.Y][foes[i].position.X].elevation);


        if(posibAdvtg > bestAdvtg)
        {
            index = foes[i].ID, bestAdvtg = posibAdvtg;
            if(unit->ammo > 0 && unit->range > 0) ranged = true;
        }
    }
    return index;
}